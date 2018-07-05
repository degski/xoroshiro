# xoroshiro
Boost Version of Xoroshiro Pseudo Random Number Generator (WIP)

* cmwc, C99 Complementary Multiply With Carry generator;
* practrand, build a generator for use with `practrand`;
* xoroshiro, `splitmix64`, `xoroshiro128plus`, `xorshift128plus`, `xorshift1024star`, [`xoshi256starstar`](http://xoshiro.di.unimi.it/xoshiro256starstar.c), [xoshiro256plus](http://xoshiro.di.unimi.it/xoshiro256plus.c) and some 'mods' of `xoroshiro128plus`. The most interesting one amongst those 'mods', for lack of better ideas (and to not obfuscate its origins), I've called it `xoroshiro128plusshixo`, which reflects what it does, it's an ordinary `xoroshiro128plus` with a final mixer added of the form `r = ( r >> 32 ) ^ r`. From my layman's perspective I would describe it as that the higher entropy bits from the middle get mixed-in with the lower entropy low bits, hence quality improves. Testing with `practrand` shows that this generator performs better than the original. It still fails (consistently, with different seeds, reporting `BRank(12):12K(1)` at the 64 gigabyte mark);
* Testing shows that `xoroshiro128plusshixo` is **approximately 7.5% faster** than `xoroshiro128plus`;
* All code to verify the above claim is available in this repo;
* I did not yet make any effort to avoid code duplication, surely this can be done far more cleverly, but for now it's just copy a generator, change some lines and go, wash, rinse, repeat;
* `xoroshiro128plusshixo` 'found' by me (while fiddling), I am not aware of prior-art;
* TODO1: Test the idea of `xoroshiro128plusshixo` with `r = ( ( r << 32 ) | ( r >> 32 ) ) ^ r`, or just `r = ( r << 32 ) | ( r >> 32 )`, i.e. **moving the bad bits to the middle**;
* TODO2: Implement `xoroshiro128plus` or `xoroshiro128plusshixo` in AVX2, 4 parallel states and generators, **but with a twist**, which could make quite the difference. I've implemented and published [lane-crossing shift and rotation in AVX2](https://gist.github.com/degski/b5fbac1ec6c8200d1d8ad102f89df89f). In my mind this would solve the low-bits problem in `xoroshiro`, as we will be shifting or rotating the entire 256 bits around, hence they actually move between the individual states of the 4 parallel generators. This means that the low bits don't get stuck, but are mixed by neighbouring generators and [the generators] will be mutually improving each other. I don't expect a lot of speed improvement (it's not cheap, I've indicated the expected latencies in the gist), if any, but, in qualitative terms, the result should be better.

## Some test results
### Speedtest (showing relative difference, the absolute number is not relevant)

    xoroshiro128plus                  3500 ms
    xoshiro256starstar                4550 ms
    xoshiro256plus                    4360 ms
    xoroshiro128plusshixo             3240 ms
    pcg64                             3800 ms

The test were conducted on a `Intel Ci3-5005U` cpu, `Windows 10 1803 x64`, using [`Clang/LLVM-7.0.0-r333363-win64`](https://llvm.org/builds/) () (has been updated since).


#### xoroshiro128plus

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0xa813cb76
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0xa813cb76
    length= 128 megabytes (2^27 bytes), time= 2.7 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
    [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
    ...and 146 test result(s) without anomalies

    rng=RNG_stdin64, seed=0xa813cb76
    length= 256 megabytes (2^28 bytes), time= 6.0 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
    [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
    [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
    [Low1/64]BRank(12):512(1)         R= +8161  p~=  1e-2457    FAIL !!!!!!!!
    ...and 155 test result(s) without anomalies

    rng=RNG_stdin64, seed=0xa813cb76
    length= 512 megabytes (2^29 bytes), time= 11.8 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
    [Low4/64]BRank(12):1K(1)          R= +2650  p~=  9.8e-799   FAIL !!!!!!!
    [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
    [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
    [Low1/64]BRank(12):512(1)         R= +8161  p~=  1e-2457    FAIL !!!!!!!!
    ...and 164 test result(s) without anomalies

    rng=RNG_stdin64, seed=0xa813cb76
    length= 1 gigabyte (2^30 bytes), time= 22.9 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
    [Low4/64]BRank(12):1K(1)          R= +2650  p~=  9.8e-799   FAIL !!!!!!!
    [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
    [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
    [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
    [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
    ...and 174 test result(s) without anomalies

    rng=RNG_stdin64, seed=0xa813cb76
    length= 2 gigabytes (2^31 bytes), time= 44.1 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
    [Low4/64]BRank(12):1K(2)          R= +3748  p~=  3e-1129    FAIL !!!!!!!!
    [Low4/64]BRank(12):1536(1)        R= +5405  p~=  3e-1628    FAIL !!!!!!!!
    [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
    [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
    [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
    [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
    [Low1/64]BRank(12):1K(1)          R=+19183  p~=  1e-5775    FAIL !!!!!!!!
    ...and 183 test result(s) without anomalies


#### xoshiro256starstar


#### xoshiro256plus



#### xoroshiro128plusshixo

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0xc24c83c5
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 128 megabytes (2^27 bytes), time= 2.3 seconds
    no anomalies in 148 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 256 megabytes (2^28 bytes), time= 5.4 seconds
    no anomalies in 159 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 512 megabytes (2^29 bytes), time= 10.6 seconds
    no anomalies in 169 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 1 gigabyte (2^30 bytes), time= 20.5 seconds
    no anomalies in 180 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 2 gigabytes (2^31 bytes), time= 39.1 seconds
    no anomalies in 191 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 4 gigabytes (2^32 bytes), time= 74.4 seconds
    no anomalies in 201 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 8 gigabytes (2^33 bytes), time= 146 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low1/64]DC6-9x1Bytes-1           R=  +5.3  p =  8.4e-3   unusual
    ...and 211 test result(s) without anomalies

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 16 gigabytes (2^34 bytes), time= 289 seconds
    no anomalies in 223 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 32 gigabytes (2^35 bytes), time= 579 seconds
    no anomalies in 233 test result(s)

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 64 gigabytes (2^36 bytes), time= 1155 seconds
    Test Name                         Raw       Processed     Evaluation
    BRank(12):12K(1)                  R= +1272  p~=  5.4e-384   FAIL !!!!!!!
    ...and 243 test result(s) without anomalies

    rng=RNG_stdin64, seed=0xc24c83c5
    length= 128 gigabytes (2^37 bytes), time= 2293 seconds
    Test Name                         Raw       Processed     Evaluation
    BRank(12):12K(1)                  R= +1272  p~=  5.4e-384   FAIL !!!!!!!
    BRank(12):16K(1)                  R= +2650  p~=  9.8e-799   FAIL !!!!!!!
    ...and 253 test result(s) without anomalies


#### xoroshiro128plusshixostar

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0x833d33c6
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 128 megabytes (2^27 bytes), time= 2.4 seconds
    no anomalies in 148 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 256 megabytes (2^28 bytes), time= 5.5 seconds
    no anomalies in 159 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 512 megabytes (2^29 bytes), time= 10.8 seconds
    no anomalies in 169 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 1 gigabyte (2^30 bytes), time= 20.6 seconds
    no anomalies in 180 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 2 gigabytes (2^31 bytes), time= 39.0 seconds
    no anomalies in 191 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 4 gigabytes (2^32 bytes), time= 74.1 seconds
    Test Name                         Raw       Processed     Evaluation
    BCFN(2+3,13-0,T)                  R=  -9.5  p =1-4.6e-5   mildly suspicious
    [Low16/64]DC6-9x1Bytes-1          R=  -3.8  p =1-8.9e-3   unusual
    ...and 199 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x833d33c6
    length= 8 gigabytes (2^33 bytes), time= 146 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low1/64]DC6-9x1Bytes-1           R=  +5.3  p =  8.4e-3   unusual
    ...and 211 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x833d33c6
    length= 16 gigabytes (2^34 bytes), time= 287 seconds
    no anomalies in 223 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 32 gigabytes (2^35 bytes), time= 561 seconds
    no anomalies in 233 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 64 gigabytes (2^36 bytes), time= 1124 seconds
    Test Name                         Raw       Processed     Evaluation
    FPF-14+6/16:all                   R=  +4.7  p =  7.4e-4   unusual
    ...and 243 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x833d33c6
    length= 128 gigabytes (2^37 bytes), time= 2236 seconds
    no anomalies in 255 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 256 gigabytes (2^38 bytes), time= 4408 seconds
    no anomalies in 265 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 512 gigabytes (2^39 bytes), time= 8914 seconds
    no anomalies in 276 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 1 terabyte (2^40 bytes), time= 17837 seconds
    no anomalies in 287 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 2 terabytes (2^41 bytes), time= 35848 seconds
    no anomalies in 297 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 4 terabytes (2^42 bytes), time= 75120 seconds
    no anomalies in 308 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 8 terabytes (2^43 bytes), time= 149979 seconds
    no anomalies in 319 test result(s)

    rng=RNG_stdin64, seed=0x833d33c6
    length= 16 terabytes (2^44 bytes), time= 298426 seconds
    no anomalies in 329 test result(s)


#### xoroshiro128plusshixostarshixo

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0x81da82c3
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 128 megabytes (2^27 bytes), time= 2.3 seconds
    no anomalies in 148 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 256 megabytes (2^28 bytes), time= 5.4 seconds
    no anomalies in 159 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 512 megabytes (2^29 bytes), time= 10.7 seconds
    no anomalies in 169 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 1 gigabyte (2^30 bytes), time= 20.9 seconds
    no anomalies in 180 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 2 gigabytes (2^31 bytes), time= 39.5 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low16/64]DC6-9x1Bytes-1          R=  +5.9  p =  3.0e-3   unusual
    ...and 190 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x81da82c3
    length= 4 gigabytes (2^32 bytes), time= 74.5 seconds
    no anomalies in 201 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 8 gigabytes (2^33 bytes), time= 146 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low4/64]Gap-16:A                 R=  -3.8  p =1-1.4e-3   unusual
    ...and 211 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x81da82c3
    length= 16 gigabytes (2^34 bytes), time= 286 seconds
    no anomalies in 223 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 32 gigabytes (2^35 bytes), time= 560 seconds
    no anomalies in 233 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 64 gigabytes (2^36 bytes), time= 1133 seconds
    no anomalies in 244 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 128 gigabytes (2^37 bytes), time= 2251 seconds
    no anomalies in 255 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 256 gigabytes (2^38 bytes), time= 4430 seconds
    no anomalies in 265 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 512 gigabytes (2^39 bytes), time= 8924 seconds
    no anomalies in 276 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 1 terabyte (2^40 bytes), time= 17820 seconds
    no anomalies in 287 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 2 terabytes (2^41 bytes), time= 35659 seconds
    no anomalies in 297 test result(s)

    rng=RNG_stdin64, seed=0x81da82c3
    length= 4 terabytes (2^42 bytes), time= 71536 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low16/64]BCFN(2+4,13-0,T)        R=  -9.1  p =1-8.1e-5   unusual
    ...and 307 test result(s) without anomalies


#### pcg64

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0xe90ca14c
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 128 megabytes (2^27 bytes), time= 2.3 seconds
    no anomalies in 148 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 256 megabytes (2^28 bytes), time= 5.4 seconds
    no anomalies in 159 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 512 megabytes (2^29 bytes), time= 10.7 seconds
    no anomalies in 169 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 1 gigabyte (2^30 bytes), time= 20.6 seconds
    no anomalies in 180 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 2 gigabytes (2^31 bytes), time= 39.3 seconds
    Test Name                         Raw       Processed     Evaluation
    [Low4/64]BCFN(2+2,13-3,T)         R=  +8.9  p =  5.4e-4   unusual
    ...and 190 test result(s) without anomalies

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 4 gigabytes (2^32 bytes), time= 74.7 seconds
    no anomalies in 201 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 8 gigabytes (2^33 bytes), time= 148 seconds
    no anomalies in 212 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 16 gigabytes (2^34 bytes), time= 291 seconds
    no anomalies in 223 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 32 gigabytes (2^35 bytes), time= 569 seconds
    no anomalies in 233 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 64 gigabytes (2^36 bytes), time= 1143 seconds
    no anomalies in 244 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 128 gigabytes (2^37 bytes), time= 2299 seconds
    no anomalies in 255 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 256 gigabytes (2^38 bytes), time= 4566 seconds
    no anomalies in 265 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 512 gigabytes (2^39 bytes), time= 9171 seconds
    no anomalies in 276 test result(s)

    rng=RNG_stdin64, seed=0xe90ca14c
    length= 1 terabyte (2^40 bytes), time= 18198 seconds
    no anomalies in 287 test result(s)
