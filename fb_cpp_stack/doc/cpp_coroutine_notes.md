# Coroutine notes

## Why coroutine
If you are a python user, you already experience similar functionality by `yield`.  Here is a super hot topic from stackoverflow help you understand more on `yield`: [What does the “yield” keyword do?](https://stackoverflow.com/questions/231767/what-does-the-yield-keyword-do).  

Basically, I want to have such ability
```C
int function(void) {
    int i;
    for (i = 0; i < 10; i++)
        return i;   
}
// The first time I call function(), it could return me 0   
// The second time I call function(), it could return me 1   
// ...   
// The 10th time I call function(), it could return me 9  
```
 
But this dream is too beautiful to be true with basic C++.  But with goto's help, we can transfer control to an arbitrary point in the function:
```C
int function(void) {
    static int i, state = 0;
    switch (state) {
        case 0: goto LABEL0;
        case 1: goto LABEL1;
    }
    LABEL0: /* start of function */
    for (i = 0; i < 10; i++) {
        state = 1; /* so we will come back to LABEL1 */
        return i;
        LABEL1:; /* resume control straight after the return */
    }
}
```
The problem is, if we want write a function with two `return`, we need to add `case 2: goto LABEL2;`.  The set of labels must be maintained manually, and must be consistent between the function body and the initial switch statement.
The following code use the trick of [Duff's device](https://en.wikipedia.org/wiki/Duff%27s_device) by using the switch statement to perform the jump itself:
```C
int function(void) {
    static int i, state = 0;
    switch (state) {
        case 0: /* start of function */
        for (i = 0; i < 10; i++) {
            state = 1; /* so we will come back to "case 1" */
            return i;
            case 1:; /* resume control straight after the return */
        }
    }
}
```
By some chosen macros, we could hide the gory details:
```C
#define crBegin static int state=0; switch(state) { case 0:
#define crReturn(i,x) do { state=i; return x; case i:; } while (0)
#define crFinish }
int function(void) {
    static int i;
    crBegin;
    for (i = 0; i < 10; i++)
        crReturn(1, i);
    crFinish;
}
``` 
Still not perfect, but it helps to get a feeling about what is coroutine.

In The Art of Computer Programming, Donald Knuth presents a solution to this sort of problem. His answer is to throw away the stack concept completely. Stop thinking of one process as the caller and the other as the callee, and start thinking of them as cooperating equals.  In practical terms: replace the traditional "call" primitive with a slightly different one. The new "call" will save the return value somewhere other than on the stack, and will then jump to a location specified in another saved return value.   Languages like C depend utterly on their stack-based structure, so whenever control passes from any function to any other, one must be the caller and the other must be the callee.  Let's see how we could modify C++'s compiler to achieve this.  

## Simple Compiler Impl

let’s say we have a function (or coroutine), f() that calls a coroutine, x(int a).

```
STACK                     REGISTERS               HEAP

                          +------+
+---------------+ <------ | rsp  |
|  f()          |         +------+
+---------------+
| ...           |
|               |
```
Then when x(42) is called, it first creates a stack frame for x(), as with normal functions.

```
STACK                     REGISTERS               HEAP
+----------------+ <-+
|  x()           |   |
| a  = 42        |   |
| ret= f()+0x123 |   |    +------+
+----------------+   +--- | rsp  |
|  f()           |        +------+
+----------------+
| ...            |
|                |
```
Then, once the coroutine x() has allocated memory for the coroutine frame on the heap and copied/moved parameter values into the coroutine frame we’ll end up with something that looks like the next diagram. Note that the compiler will typically hold the address of the coroutine frame in a separate register to the stack pointer (eg. MSVC stores this in the rbp register).

```
STACK                     REGISTERS               HEAP
+----------------+ <-+
|  x()           |   |
| a  = 42        |   |                   +-->  +-----------+
| ret= f()+0x123 |   |    +------+       |     |  x()      |
+----------------+   +--- | rsp  |       |     | a =  42   |
|  f()           |        +------+       |     +-----------+
+----------------+        | rbp  | ------+
| ...            |        +------+
|                |
```
If the coroutine x() then calls another normal function g() it will look something like this.

```
STACK                     REGISTERS               HEAP
+----------------+ <-+
|  g()           |   |
| ret= x()+0x45  |   |
+----------------+   |
|  x()           |   |
| coroframe      | --|-------------------+
| a  = 42        |   |                   +-->  +-----------+
| ret= f()+0x123 |   |    +------+             |  x()      |
+----------------+   +--- | rsp  |             | a =  42   |
|  f()           |        +------+             +-----------+
+----------------+        | rbp  |
| ...            |        +------+
|                |
```
When g() returns it will destroy its activation frame and restore x()’s activation frame. Let’s say we save g()’s return value in a local variable b which is stored in the coroutine frame.
```
STACK                     REGISTERS               HEAP
+----------------+ <-+
|  x()           |   |
| a  = 42        |   |                   +-->  +-----------+
| ret= f()+0x123 |   |    +------+       |     |  x()      |
+----------------+   +--- | rsp  |       |     | a =  42   |
|  f()           |        +------+       |     | b = 789   |
+----------------+        | rbp  | ------+     +-----------+
| ...            |        +------+
|                |
```

If x() now hits a suspend-point and suspends execution without destroying its activation frame then execution returns to f().  

This results in the stack-frame part of x() being popped off the stack while leaving the coroutine-frame on the heap. When the coroutine suspends for the first time, a return-value is returned to the caller. This return value often holds a handle to the coroutine-frame that suspended that can be used to later resume it. When x() suspends it also stores the address of the resumption-point of x() in the coroutine frame (call it RP for resume-point).

```
STACK                     REGISTERS               HEAP
                                        +----> +-----------+
                          +------+      |      |  x()      |
+----------------+ <----- | rsp  |      |      | a =  42   |
|  f()           |        +------+      |      | b = 789   |
| handle     ----|---+    | rbp  |      |      | RP=x()+99 |
| ...            |   |    +------+      |      +-----------+
|                |   |                  |
|                |   +------------------+
```
The function that resumes the coroutine calls a void resume(handle) function to resume execution of the coroutine. To the caller, this looks just like any other normal call to a void-returning function with a single argument.  

This creates a new stack-frame that records the return-address of the caller to resume(), activates the coroutine-frame by loading its address into a register and resumes execution of x() at the resume-point stored in the coroutine-frame.

```
STACK                     REGISTERS               HEAP
+----------------+ <-+
|  x()           |   |                   +-->  +-----------+
| ret= h()+0x87  |   |    +------+       |     |  x()      |
+----------------+   +--- | rsp  |       |     | a =  42   |
|  h()           |        +------+       |     | b = 789   |
| handle         |        | rbp  | ------+     +-----------+
+----------------+        +------+
| ...            |
|                |
```

## Where should consider coroutine

Couroutine always act as generators, for a class provide a set of data, consumers always have different requirements:
Maybe we want to limit the maximum number of iterations  
Or maybe we want to print after every iteration  
Or maybe we want to return a tuple of { fib(n+1), fib(n) }  
Or maybe there is a use-case we haven’t thought of yet, but someone might have down the line  

Sometimes, when we write a function, we are not sure whether this function will be a caller or a callee.



