# Folly::fiber


## Example 1

FibersTestApp, the case is coming from folly's unit test: [FibersTestApp](https://github.com/facebook/folly/blob/99e8fc1fc40779f98a16e52fd852b7b6a6845c3a/folly/fibers/test/FibersTestApp.cpp#L27)
- It demos using a loop() to generate requests and hung the program until requests has been full filled

```C++
// [perry] SimpleLoopController() works similar as while(true)
void loop() {
    // [perry] When there are certain number of elements in pendingRequests then start fulfill data
    //         each round pop 1 request 
    if (pendingRequests.size() == maxOutstanding || toSend == 0) {
      // [perry] full fill data from pendingRequests
    } else {
      // [perry]generate fiber
      fiberManager.addTask([this, id]() {
          // ..
          auto result1 = await([this](Promise<int> fiber) { ... }
          auto result2 = await([this](Promise<int> fiber) { ... }
        });
      });

      // [perry] toSend has been constructed with 20
      if (--toSend == 0) {
        auto& loopController =
            dynamic_cast<SimpleLoopController&>(fiberManager.loopController());
        loopController.stop();
      }
    }
  }
```

- For the generator part, the logic in `else`, it is controlled by the parameter of `toSend`.  For each round will create a fiber by `fiberManager.addTask`, and inside each fiber, will put two requests into `pendingRequests`.
```C++
	fiberManager.addTask([this, id]() {
        // #1 
        auto result1 = await([this](Promise<int> fiber) {
          pendingRequests.push(std::move(fiber));
        });
        // [perry] function inside await() will be executed immediately
        //         but then will suspend until promise is fulfilled
        //         Promise<int> fiber is passed as value

        ...
        // #2
        auto result2 = await([this](Promise<int> fiber) {
          pendingRequests.push(std::move(fiber));
        });
    });
```
Callback function passed to fibers::await is executed immediately and then fiber-task is suspended until fibers::Promise is fulfilled.  Which means code will be blocked here, it will come into loop() again for next iteration.  Until `fibers::Promise<int> fiber` be set in
```C++
	pendingRequests.front().setValue(value);
```
Then `auto result1` be set to the value of `value` which is generated randomly.  Then logic will enter the second fibers::await, it will block there until the value of fibers::Promise be set.

- The running result of this program could be found [here](../resource/fibers_app_test_result.txt), the execution is similar to
```
Add request id = 1 into pendingRequests, result1 for id = 1 is waiting
Add request id = 2 into pendingRequests, result1 for id = 2 is waiting
Add request id = 3 into pendingRequests, result1 for id = 3 is waiting
Add request id = 4 into pendingRequests, result1 for id = 4 is waiting
Add request id = 5 into pendingRequests, result1 for id = 5 is waiting
// when there are 5 elements in pending pendingRequests
Go to if(...), result1 for id = 1's value has been set, reset stack, go to next line of auto result1 = await...
Add request id = 1 into pendingRequests, result2 for id = 1 is waiting 
Go to if(...), result1 for id = 2's value has been set, reset stack, go to next line of auto result1 = await...
...
```



