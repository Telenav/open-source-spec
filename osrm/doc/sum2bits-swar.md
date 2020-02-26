# sum2bits - variable-precision SWAR algorithm
When I read [.osrm.names](./osrm-toolchain-files/map.osrm.names.md) processing related codes, I found a function named [sum2bits()](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/include/util/indexed_data.hpp#L94) that looks a little strange to me. It's used for "summation of 16 2-bit values using SWAR" according to comment, but what I can see in codes are a lot of bitwise operations with some magic numbers. It looks no sense to me. What does it exactly do? Let's try to understand it step-by-step.          

```c++
    /// Summation of 16 2-bit values using SWAR
    inline std::uint32_t sum2bits(std::uint32_t value) const
    {
        value = (value >> 2 & 0x33333333) + (value & 0x33333333);
        value = (value >> 4 & 0x0f0f0f0f) + (value & 0x0f0f0f0f);
        value = (value >> 8 & 0x00ff00ff) + (value & 0x00ff00ff);
        return (value >> 16 & 0x0000ffff) + (value & 0x0000ffff);
    }
```

## Background 
When processing [.osrm.names](./osrm-toolchain-files/map.osrm.names.md), OSRM internally uses a `uint32_t` to store `16` seperate values for saving memory. In another word, every `2`-bits represents a independent value in the `uint32_t`, and each independent value will be `[0,3]` of course.     
The `2`-bits value is used to record how many bytes will be sufficient to store a length of `char`s array, and OSRM need to get the summarization of them for calcuating something like `offset` later.    
Here's a example:      
- If we have a `char` array `s`: `char *s = "abcedfg";    // length: 7 bytes `;      
- `s`'s length is `7`, so `1` byte should be enough to store the length value `7`;    
- If we use the last `2`-bits of `uint32_t d` to store how many bytes for the length value `7`, the `2`-bits will be `0b01`(means `1` byte);      
- There'll be a lot of `char` arrays with very different lengths, some of them might bigger than `255`(beyond `1` byte's range) but shorter than `65536`, then at least `2` bytes are necessary to store the length, and we could fill in the `2`-bits as `0b10`; same thing can be applied on array length range `[65536, 2^24]`, the `2`-bits have to be `0b11`.     
  - The [`log256()`](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/include/util/indexed_data.hpp#L60) implements the idea. The `log` is [mathematics logarithm](https://en.wikipedia.org/wiki/Logarithm).          
  ```c++
      /// Returns ceiling(log_256(value + 1))
      inline std::uint32_t log256(std::uint32_t value) const
      {
          BOOST_ASSERT(value < 0x1000000);
          return value == 0 ? 0 : value < 0x100 ? 1 : value < 0x10000 ? 2 : 3;
      }
  ```
- Once the `uint32_t d` have stored `16` independent values, OSRM wants to know totally how many bytes will be necessary for store these `16` lengths, that's why the [sum2bits()](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/include/util/indexed_data.hpp#L94) comes.        

## What sum2bits achieves
Assume we have `16` arrays with lengths `1,10,20,30,40,50,100,200,250,300,500,1000,5000,10000,50000,100000`, we'll need `1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3` bytes to store these lengths(refer to below table for the mapping).       

|  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
| - | - | - | - | - | - | - | - | - | - | - | - | - | - | - | - | - |
| array length | 1 | 10 | 20 | 30 | 40 | 50 | 100 | 200 | 250 | 300 | 500 | 1000 | 5000 | 10000 | 50000 |100000|
| bytes to store length | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 2 | 2 | 2 | 2 | 2 | 2 | 3 | 

If we use `uint32_t d` to store the "bytes to store length", we'll get     
`d = 0b01010101010101010110101010101011 // 0x55556AAB`       
Totally "bytes to store length" of `d` is `1+1+1+1+1+1+1+1+1+2+2+2+2+2+2+3=24`.      
The [sum2bits()](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/include/util/indexed_data.hpp#L94) is designed to calculate `sum2bits(d) = 24`.     
The [unittests](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/unit_tests/util/indexed_data.cpp#L22) show more examples.  

```c++
    BOOST_CHECK_EQUAL(variable_group_block.sum2bits(0xe4), 6);
    BOOST_CHECK_EQUAL(variable_group_block.sum2bits(0x11111111), 8);
    BOOST_CHECK_EQUAL(variable_group_block.sum2bits(0x55555555), 16);
    BOOST_CHECK_EQUAL(variable_group_block.sum2bits(0xffffffff), 48);
```

### sum2bits_normal
Normally we may implement the [sum2bits()](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/include/util/indexed_data.hpp#L94) as below. It requires loop `16` times to get result, then the performance may be not good enough due to the loops if we have many `uint32_t`s to calculate.    
The [sum2bits()](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/include/util/indexed_data.hpp#L94) implementation has no loop, looks much more efficient.           

```c++
    std::uint32_t sum2bits_normal(std::uint32_t value)
    {
        std::uint32_t sum = 0;
        for (int i = 0; i < 16; ++i) {
            sum += (value >> (i*2)) & 0x3;
        }
        return sum;
    }
```

## How does sum2bits work
Now it's time to understand the magic.     

```c++
    /// Summation of 16 2-bit values using SWAR
    inline std::uint32_t sum2bits(std::uint32_t value) const
    {
        value = (value >> 2 & 0x33333333) + (value & 0x33333333);   // (A)
        value = (value >> 4 & 0x0f0f0f0f) + (value & 0x0f0f0f0f);   // (B)
        value = (value >> 8 & 0x00ff00ff) + (value & 0x00ff00ff);   // (C)
        return (value >> 16 & 0x0000ffff) + (value & 0x0000ffff);   // (D)
    }
```

### Line (A)
There're 4 lines in the `sum2bits()` function. Let's take a look on the line `(A)` first.     
First of all, the significance of the constant `0x33333333` is that, written using the `Java`/`GCC` style [binary literal notation](https://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html),     
`0x33333333 = 0b00110011001100110011001100110011`     
That is, 8 repeated `0011` block(per **4 bits**). Well, let's see what would happen if the `value` is only 1 **4 bits** block. 

| value_before | left(`v >> 2 & 0x3`) | right(`v & 0x3`) | value_after(left+right) | 
| - | - | - | - |
|`0001`|`0000`|`0001`|`0001` = 1|
|`0010`|`0000`|`0010`|`0010` = 2|
|`0011`|`0000`|`0011`|`0011` = 3|
|`0100`|`0001`|`0000`|`0001` = 1|
|`0101`|`0001`|`0001`|`0010` = 2| 
|`0110`|`0001`|`0010`|`0011` = 3|
|`0111`|`0001`|`0011`|`0100` = 4|
|`1000`|`0010`|`0000`|`0010` = 2|
|`1001`|`0010`|`0001`|`0011` = 3|
|`1010`|`0010`|`0010`|`0100` = 4|
|`1011`|`0010`|`0011`|`0101` = 5|
|`1100`|`0011`|`0000`|`0011` = 3|
|`1101`|`0011`|`0001`|`0100` = 4|
|`1110`|`0011`|`0010`|`0101` = 5|
|`1111`|`0011`|`0011`|`1010` = 6|

We exactly get sum of the two `2`-bits(e.g. `01+10=11=0011`)! And the result still in the original 4 bits!      
Other 7 repeated 4-bits block will do the same thing, but please be aware that all the 8 repeated 4-bits block calculate at the same time, that's why this algorithm also known as "parallel". The algorithm calculates 8 blocks in parallel without any loop.          

### Line (B),(C),(D) 
After [Line (A)](#line-a) done, the problem becomes to sum 8 values(each one stores in `4`-bits) instead of orginal 16 values(each `2`-bits).     
The significance of the other constants are        
```
0x0f0f0f0f=0b00001111000011110000111100001111
0x00ff00ff=0b00000000111111110000000011111111
0x0000ffff=0b00000000000000001111111111111111
```      
Very similar style, right?  
The Line `(B),(C),(D)` do exactly the same as the previous line `(A)`, except they adds the adjacent `4/8/16`-bit bitcounts together to give the bitcounts of each `8/16/32`-bit block.     


## Conclusion
The **variable-precision SWAR algorithm** in this binary case is also knowns as **population count**, **popcount**, **sideways sum**, or **bit summation**. The concept comes from [Hamming weight](https://en.wikipedia.org/wiki/Hamming_weight), and there're a lot of CPUs provide built-in instructions to do it. This algorithm is the best solution if your processors lacking those features. Read more details in [How does this algorithm to count the number of set bits in a 32-bit integer work?](https://stackoverflow.com/questions/22081738/how-does-this-algorithm-to-count-the-number-of-set-bits-in-a-32-bit-integer-work), [How to count the number of set bits in a 32-bit integer?](https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer), [Hamming weight](https://en.wikipedia.org/wiki/Hamming_weight).    


## References
- [How does this algorithm to count the number of set bits in a 32-bit integer work?](https://stackoverflow.com/questions/22081738/how-does-this-algorithm-to-count-the-number-of-set-bits-in-a-32-bit-integer-work)
- [How to count the number of set bits in a 32-bit integer?](https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer)
- [Hamming weight - Efficient implementation](https://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation)
- [Telenav/osrm-backend - sum2bits()](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/include/util/indexed_data.hpp#L94)
- [Telenav/osrm-backend - unittest for sum2bits()](https://github.com/Telenav/osrm-backend/blob/b24b8a085dc10bea279ffb352049330beae23791/unit_tests/util/indexed_data.cpp#L22)
- [binary literal notation](https://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html)

