# xoroshiro
Boost Version of Xoroshiro Pseudo Random Number Generator (WIP)

* cmwc, C99 Complementary Multiply With Carry generator;
* practrand, build a generator for use with practrand;
* xoroshiro, and amongst others a modified version of `xoroshiro128plus` for lack of better ideas I've called it `xoroshiro128plusshixo`, which reflects what it does, it's an ordinary `xoroshiro128plus` with a final mixer added of the form `r = ( r >> 32 ) ^ r`. Testing with practrand shows that this generator performs better than the original. It still fails, but less and later. The really good news in this respect is that testing shows that this modified generator is appoximately 10% faster than `xoroshiro128plus`.
* All code to verify the above claim is available in this repo.