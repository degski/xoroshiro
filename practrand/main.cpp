/**
* Copyright degski 2018
* Distributed under the Boost Software License, Version 1.0. (See
* accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*
* See http://www.boost.org for most recent version including documentation.
*/

#ifdef _WIN32 // needed to allow binary stdout on windows
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <cstdint>

#include <random>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/seed_seq.hpp>
#include <boost/random/random_device.hpp>
#include "../xoroshiro/seed_seq_fe.hpp"
#include "../xoroshiro/xoroshiro.hpp"

#include "../xoroshiro/taus88.hpp"

#include <pcg/pcg_random.hpp>
#include "../xoroshiro/xoroshiro_meo.hpp"

#define __AVX2__ 1

#include <integer_utils.hpp>

auto main ( ) -> int {

    #ifdef _WIN32 // Needed to allow binary stdout on Windhoze...
    _setmode ( _fileno ( stdout ), _O_BINARY );
    #endif

    using Generator = xoroshiro128plus64; // boost::random::xoroshiro128plusshixo;
    // using Generator = pcg64;
    using result_type = typename Generator::result_type;

    const boost::random::seed_seq_fe256 seq { 11, 21, 31, 41, 51, 61, 71, 81, 91, 101 };
    Generator rng( 0xBEAC0467EBA5FACB );

    const std::size_t page_size = 524'288, buffer_size = page_size / sizeof(result_type);
    result_type buffer[buffer_size];

    while(true) {
        for(std::size_t i = 0; i < buffer_size; ++i) {
            buffer[i] = rng();
        }
        std::cout.write(reinterpret_cast<char*>(buffer), page_size);
    }
}

/*

xoroshiro128plus
================

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


xoroshiro128plusshixo
=====================

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


xoroshiro128plusshixostar
=========================

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


xoroshiro128plusshixostarshixo
==============================

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


pcg64
=====

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


*/