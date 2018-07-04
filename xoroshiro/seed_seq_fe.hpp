/* boost random/seed_seq_fe.hpp header file
 *
 * Copyright Melissa E. O'Neill 2015
 * Copyright degski 2017
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 *
 */

/*
 * Summary and Usage:
 * ------------------
 *
 *   Fixed-Entropy Seed Sequence
 *
 *   Provides a replacement for std::seed_seq that avoids problems with bias,
 *   performs better in empirical statistical tests, and executes faster in
 *   normal-sized use cases. The values of the generated seed sequence differ
 *   (by design) from the ones produced by std::seed_seq.
 *
 *   In normal use, it's accessed via one of the following type aliases
 *
 *       boost::random::seed_seq_fe128
 *       boost::random::seed_seq_fe256
 *
 *   It's discussed in detail at
 *       http://www.pcg-random.org/posts/developing-a-seed_seq-alternative.html
 *   and the motivation for its creation (what's wrong with std::seed_seq) here
 *       http://www.pcg-random.org/posts/cpp-seeding-surprises.html
 *
 *
 * Original Distribution License:
 * ------------------------------
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Melissa E. O'Neill
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 * Re-Licensing:
 * -------------
 *
 * Permission granted by Melissa E. O'Neill for Fixed-Entropy Seed Sequence code
 * to be distributed under the Boost Software License, Version 1.0 for the
 * purpose of inclusion in the Boost library collection.
 *
 *
 * Modifications/Changes by degski:
 * --------------------------------
 *
 * Added a default constructor, to be compliant with the standard. In order to
 * fully mimic std::seed_seq and boost::random::seed_seq in case of default
 * initialization and to provide static allocation as per the original
 * implementation, member std::array<IntRep, count> _mixer was replaced with
 * boost::container::static_vector<IntRep, count> _mixer. This involved a change to
 * mix_entropy(). Furthermore there were some minor changes like boostifications,
 * privatization of some public functions and some re-formatting.
 *
 */

#ifndef BOOST_RANDOM_SEED_SEQ_FE_HPP
#define BOOST_RANDOM_SEED_SEQ_FE_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <iterator>

#include <boost/config.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
#include <initializer_list>
#endif

#if __cpp_constexpr >= 201304L
    #define SEED_SEQ_FE_GENERALIZED_CONSTEXPR constexpr
#else
    #define SEED_SEQ_FE_GENERALIZED_CONSTEXPR
#endif


namespace boost {
namespace random {

/**
 * seed_seq_fe implements a fixed-entropy seed sequence; it conforms to all
 * the requirements of a Seed Sequence concept.
 *
 * seed_seq_fe<N> implements a seed sequence which seeds based on a store of
 * N * 32 bits of entropy.  Typically, it would be initialized with N or more
 * integers.
 *
 * seed_seq_fe128 and seed_seq_fe256 are provided as convenience typedefs for
 * 128- and 256-bit entropy stores respectively.  These variants outperform
 * std::seed_seq, while being better mixing the bits it is provided as entropy.
 * In almost all common use cases, they serve as better drop-in replacements
 * for seed_seq.
 *
 * Technical details
 *
 * Assuming it constructed with M seed integers as input, it exhibits the
 * following properties
 *
 * * Diffusion/Avalanche:  A single-bit change in any of the M inputs has a
 *   50% chance of flipping every bit in the bitstream produced by generate.
 *   Initializing the N-word entropy store with M words requires O(N * M)
 *   time precisely because of the avalanche requirements.  Once constructed,
 *   calls to generate are linear in the number of words generated.
 *
 * * Bias freedom/Bijection: If M == N, the state of the entropy store is a
 *   bijection from the M inputs (i.e., no states occur twice, none are
 *   omitted). If M > N the number of times each state can occur is the same
 *   (each state occurs 2**(32*(M-N)) times, where ** is the power function).
 *   If M < N, some states cannot occur (bias) but no state occurs more
 *   than once (it's impossible to avoid bias if M < N; ideally N should not
 *   be chosen so that it is more than M).
 *
 *   Likewise, the generate function has similar properties (with the entropy
 *   store as the input data).  If more outputs are requested than there is
 *   entropy, some outputs cannot occur.  For example, the Mersenne Twister
 *   will request 624 outputs, to initialize it's 19937-bit state, which is
 *   much larger than a 128-bit or 256-bit entropy pool.  But in practice,
 *   limiting the Mersenne Twister to 2**128 possible initializations gives
 *   us enough initializations to give a unique initialization to trillions
 *   of computers for billions of years.  If you really have 624 words of
 *   *real* high-quality entropy you want to use, you probably don't need
 *   an entropy mixer like this class at all.  But if you *really* want to,
 *   nothing is stopping you from creating a boost::random::seed_seq_fe<624>.
 *
 * * As a consequence of the above properties, if all parts of the provided
 *   seed data are kept constant except one, and the remaining part is varied
 *   through K different states, K different output sequences will be produced.
 *
 * * Also, because the amount of entropy stored is fixed, this class never
 *   performs dynamic allocation and is free of the possibility of generating
 *   an exception.
 *
 * Ideas used to implement this code include hashing, a simple PCG generator
 * based on an MCG base with an XorShift output function and permutation
 * functions on tuples.
 *
 * More detail at
 *     http://www.pcg-random.org/posts/developing-a-seed_seq-alternative.html
 *
 */

template <std::size_t count = 4, std::size_t mix_rounds = 1 + (count <= 2)>
class seed_seq_fe {
public:
    typedef std::uint_least32_t result_type;
private:
    typedef std::uint32_t IntRep;

    static constexpr std::uint32_t INIT_A = 0x43B0D7E5;
    static constexpr std::uint32_t MULT_A = 0x931E8875;

    static constexpr std::uint32_t INIT_B = 0x8B51F9DD;
    static constexpr std::uint32_t MULT_B = 0x58F38DED;

    static constexpr std::uint32_t MIX_MULT_L = 0xCA01F9DD;
    static constexpr std::uint32_t MIX_MULT_R = 0x4973F715;
    static constexpr std::uint32_t XSHIFT = sizeof(IntRep)*8/2;

    SEED_SEQ_FE_GENERALIZED_CONSTEXPR
    static IntRep fast_exp(IntRep x, IntRep power)
    {
        IntRep result = IntRep(1);
        IntRep multiplier = x;
        while (power != IntRep(0)) {
            IntRep thismult = power & IntRep(1) ? multiplier : IntRep(1);
            result *= thismult;
            power >>= 1;
            multiplier *= multiplier;
        }
        return result;
    }

    /** Initializes the sequence from an iterator range. */
    template <typename InputIter>
    void mix_entropy(InputIter begin, InputIter end);

public:
    seed_seq_fe(const seed_seq_fe&)     = delete;
    void operator=(const seed_seq_fe&)  = delete;

    /** Initializes a seed_seq to hold an empty sequence. */
    seed_seq_fe(){}

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    template <typename T>
    seed_seq_fe ( std::initializer_list<T> init )
    { seed(init.begin(), init.end()); }
#endif

    template <typename InputIter>
    seed_seq_fe(InputIter begin, InputIter end)
    { seed(begin, end); }

    /** Initializes the sequence from Boost.Range range. */
    template<class Range>
    explicit seed_seq_fe(const Range& range)
    { seed(boost::begin(range), boost::end(range)); }

    /**
     * Fills a range with 32-bit values based on the stored sequence.
     *
     * Requires: Iter must be a Random Access Iterator whose value type
     * is an integral type at least 32 bits wide.
     */
    template <typename RandomAccessIterator>
    void generate(RandomAccessIterator first, RandomAccessIterator last) const;

    static constexpr std::size_t size()
    { return count; }

    template <typename OutputIterator>
    void param(OutputIterator dest) const;

private:
    template <typename InputIter>
    void seed(InputIter begin, InputIter end)
    {
        mix_entropy(begin, end);
        // For very small sizes, we do some additional mixing.  For normal
        // sizes, this loop never performs any iterations.
        for (std::size_t i = 1; i < mix_rounds; ++i)
            stir();
    }

    seed_seq_fe& stir()
    {
        mix_entropy(_mixer.begin(), _mixer.end());
        return *this;
    }

    boost::container::static_vector<IntRep, count> _mixer;
};

template <std::size_t count, std::size_t r>
template <typename InputIter>
void seed_seq_fe<count, r>::mix_entropy(InputIter begin, InputIter end)
{
    auto hash_const = INIT_A;
    auto hash = [&](IntRep value) {
        value ^= hash_const;
        hash_const *= MULT_A;
        value *= hash_const;
        value ^= value >> XSHIFT;
        return value;
    };
    auto mix = [](IntRep x, IntRep y) {
        IntRep result = MIX_MULT_L*x - MIX_MULT_R*y;
        result ^= result >> XSHIFT;
        return result;
    };

    InputIter current = begin;
    if (_mixer.empty()) {
        while (current != end && _mixer.size() < _mixer.static_capacity)
            _mixer.emplace_back(hash(*current++));
        while (_mixer.size() < _mixer.static_capacity)
            _mixer.emplace_back(hash(0U));
    }
    else {
        for (auto& elem : _mixer) {
            if (current != end)
                elem = hash(*current++);
            else
                elem = hash(0U);
        }
    }

    for (auto& src : _mixer)
        for (auto& dest : _mixer)
            if (&src != &dest)
                dest = mix(dest,hash(src));
    for (; current != end; ++current)
        for (auto& dest : _mixer)
            dest = mix(dest,hash(*current));
}

template <std::size_t count, std::size_t mix_rounds>
template <typename OutputIterator>
void seed_seq_fe<count, mix_rounds>::param(OutputIterator dest) const
{
    const IntRep INV_A = fast_exp(MULT_A, IntRep(-1));
    const IntRep MIX_INV_L = fast_exp(MIX_MULT_L, IntRep(-1));

    boost::container::static_vector<IntRep, count> _mixercopy;
    std::copy(_mixer.begin(), _mixer.end(), std::back_inserter(_mixercopy));

    for (std::size_t round = 0; round < mix_rounds; ++round) {
        // Advance to the final value. We'll backtrack from that.
        auto hash_const = INIT_A*fast_exp(MULT_A, IntRep(count * count));

        for (auto src = _mixercopy.rbegin(); src != _mixercopy.rend(); ++src)
            for (auto dest = _mixercopy.rbegin(); dest != _mixercopy.rend();
                 ++dest)
                if (src != dest) {
                    IntRep revhashed = *src;
                    auto mult_const = hash_const;
                    hash_const *= INV_A;
                    revhashed ^= hash_const;
                    revhashed *= mult_const;
                    revhashed ^= revhashed >> XSHIFT;
                    IntRep unmixed = *dest;
                    unmixed ^= unmixed >> XSHIFT;
                    unmixed += MIX_MULT_R*revhashed;
                    unmixed *= MIX_INV_L;
                    *dest = unmixed;
                }
        for (auto i = _mixercopy.rbegin(); i != _mixercopy.rend(); ++i) {
            IntRep unhashed = *i;
            unhashed ^= unhashed >> XSHIFT;
            unhashed *= fast_exp(hash_const, IntRep(-1));
            hash_const *= INV_A;
            unhashed ^= hash_const;
            *i = unhashed;
        }
    }
    std::copy(_mixercopy.begin(), _mixercopy.end(), dest);
}


template <std::size_t count, std::size_t mix_rounds>
template <typename RandomAccessIterator>
void seed_seq_fe<count, mix_rounds>::generate(
        RandomAccessIterator dest_begin,
        RandomAccessIterator dest_end) const
{
    auto src_begin = _mixer.begin();
    auto src_end   = _mixer.end();
    auto src       = src_begin;
    auto hash_const = INIT_B;
    for (auto dest = dest_begin; dest != dest_end; ++dest) {
        auto dataval = *src;
        if (++src == src_end)
            src = src_begin;
        dataval ^= hash_const;
        hash_const *= MULT_B;
        dataval *= hash_const;
        dataval ^= dataval >> XSHIFT;
        *dest = dataval;
    }
}

using seed_seq_fe128 = seed_seq_fe<4>;
using seed_seq_fe256 = seed_seq_fe<8>;

} // namespace random
} // namespace boost

#endif // BOOST_RANDOM_SEED_SEQ_FE_HPP
