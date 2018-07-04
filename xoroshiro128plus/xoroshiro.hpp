/* boost random/xoroshiro.hpp header file
 *
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

#ifndef BOOST_RANDOM_XOROSHIRO_HPP
#define BOOST_RANDOM_XOROSHIRO_HPP

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <algorithm>

#include <boost/config.hpp>
#include <boost/random/detail/config.hpp>
#include <boost/random/detail/seed.hpp>
#include <boost/random/detail/seed_impl.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/random/detail/disable_warnings.hpp>

namespace boost {
namespace random {

#if !defined(BOOST_NO_INT64_T) && !defined(BOOST_NO_INTEGRAL_INT64_T)

	namespace detail {

		inline std::uint64_t xoroshiro_integer_hash ( std::uint64_t x );
	}

	/**
	* This is a fixed-increment version of Java 8's SplittableRandom generator
	* See http://dx.doi.org/10.1145/2714064.2660195 and
	* http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
	*
	* It is a very fast generator passing BigCrush, and it can be useful if
	* for some reason one absolutely want 64 bits of state;
	*
	* c-code by Sebastiano Vigna: http://xoroshiro.di.unimi.it/splitmix64.c
	*/
	class splitmix64 {
		friend class xoroshiro128plus;
		friend class xoroshiro128plusshixo;
		friend class xoroshiro128plusshixostar;
		friend class xoroshiro128plusshixostarshixo;
		friend class xorshift128plus;
		friend class xorshift1024star;
	public:
		typedef std::uint64_t result_type;

		// Required for old Boost.Random concept.
		static const bool has_fixed_range = true;
		static const std::uint64_t default_seed = std::uint64_t { 0x9E3779B97F4A7C15 };

		/**
		* Constructs a @c splitmix64, using the default seed.
		*/
		splitmix64 ( )
		{
			seed ( );
		}

		/**
		* Constructs a @c splitmix64, seeding it with @c value.
		*/
		BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR ( splitmix64,
			std::uint64_t, value )
		{
			seed ( value );
		}

		/**
		* Constructs a @c splitmix64, seeding it with values
		* produced by a call to @c seq.generate().
		*/
		BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR ( splitmix64,
			SeedSeq, seq )
		{
			seed ( seq );
		}

		/**
		* Constructs a @c splitmix64 and seeds it with values taken
		* from the iterator range [first, last) and adjusts first to
		* point to the element after the last one used. If there are
		* not enough elements, throws @c std::invalid_argument.
		*
		* first and last must be input iterators.
		*/
		template<class It>
		splitmix64 ( It& first, It last )
		{
			seed ( first, last );
		}

		// compiler-generated copy constructor and assignment operator are fine.

		/**
		* Calls seed(default_seed).
		*/
		void seed ( )
		{
			seed ( default_seed );
		}

		/**
		* Seeds a @c splitmix64 using the supplied value. 'Hashes' @c value
		* using the bijection described by:
		*
		* std::uint64_t integer_hash(std::uint64_t x) {
		*
		*     x = ((x >> 32) ^ x) * 0xDE17C195AA959A81;
		*	   x = ((x >> 32) ^ x) * 0xDE17C195AA959A81;
		*     x = ((x >> 32) ^ x);
		*
		*	   return x;
		* }
		*/
		BOOST_RANDOM_DETAIL_ARITHMETIC_SEED ( splitmix64, std::uint64_t, value )
		{
			_s [ 0 ] = detail::xoroshiro_integer_hash ( value );
		}

		/**
		* Seeds a @c splitmix64 using values from a SeedSeq.
		*/
		BOOST_RANDOM_DETAIL_SEED_SEQ_SEED ( splitmix64, SeedSeq, seq )
		{
			detail::seed_array_int<64, 1, SeedSeq, std::uint64_t> ( seq, _s );
		}

		/**
		* seeds a @c splitmix64 with values taken from the iterator
		* range [first, last) and adjusts @c first to point to the
		* element after the last one used.  If there are not enough
		* elements, throws @c std::invalid_argument.
		*
		* @c first and @c last must be input iterators.
		*/
		template<class It>
		void seed ( It& first, It last )
		{
			detail::fill_array_int<64, 1, It, std::uint64_t> ( first, last, _s );
		}

		/**
		* Returns the smallest value that the @c splitmix64
		* can produce.
		*/
		static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ( )
		{
			return 0;
		}

		/**
		* Returns the largest value that the @c splitmix64
		* can produce.
		*/
		static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ( )
		{
			return UINT64_MAX;
		}

		/** Returns the next value of the @c splitmix64. */
		std::uint64_t operator()( )
		{
			return hash ( next ( ) );
		}

		/** Fills a range with random values. */
		template<class Iter>
		void generate ( Iter first, Iter last )
		{
			detail::generate_from_int ( *this, first, last );
		}

		/** Advances the state of the generator by @c z. */
		void discard ( boost::uintmax_t z )
		{
			// This seems to be the fastest way (release),
			// as opposed to anything more fancy.
			while ( z-- ) {
				next ( );
			}
		}

		friend bool operator==( const splitmix64& x,
			const splitmix64& y )
		{
			return x._s [ 0 ] == y._s [ 0 ];
		}

		friend bool operator!=( const splitmix64& x,
			const splitmix64& y )
		{
			return !( x == y );
		}

		/** Writes a @c splitmix64 to a @c std::ostream. */
		template<class CharT, class Traits>
		friend std::basic_ostream<CharT, Traits>&
			operator<<( std::basic_ostream<CharT, Traits>& os,
				const splitmix64& sm64 )
		{
			os << sm64._s [ 0 ];
			return os;
		}

		/** Reads a @c splitmix64 from a @c std::istream. */
		template<class CharT, class Traits>
		friend std::basic_istream<CharT, Traits>&
			operator >> ( std::basic_istream<CharT, Traits>& is,
				splitmix64& sm64 )
		{
			is >> sm64._s [ 0 ];
			return is;
		}

	private:

		/// \cond show_private

		inline std::uint64_t next ( )
		{
			return ( _s [ 0 ] += std::uint64_t { 0x9E3779B97F4A7C15 } );
		}

		static inline std::uint64_t hash ( std::uint64_t z )
		{
			z = ( z ^ ( z >> 30 ) ) * std::uint64_t { 0xBF58476D1CE4E5B9 };
			z = ( z ^ ( z >> 27 ) ) * std::uint64_t { 0x94D049BB133111EB };
			return z ^ ( z >> 31 );
		}

		/// \endcond

		std::uint64_t _s [ 1 ];
	};

namespace detail {

	// const std::uint64_t v = 0x1AEC805299990163, y = 0xCDFB859A3DD0884B;

    inline std::uint64_t xoroshiro_integer_hash(std::uint64_t x)
    {
        x = ((x >> 32) ^ x) * std::uint64_t { 0x1AEC805299990163 };
        x = ((x >> 32) ^ x);
        return x;
    }

    template<class SeedSeq, std::size_t n>
    void seed_array_non_zero_int(SeedSeq &seq, std::uint64_t (&x)[n])
    {
        std::uint_least32_t storage[2 * n];
        seq.generate(std::begin(storage), std::end(storage));

        std::size_t j = 0;
        for (; j < n; ++j) {
            x[j] = (static_cast<std::uint64_t>(storage[2 * j + 1]) << 32)
                + static_cast<std::uint64_t>(storage [2 * j]);
            if (x[j]) {
                // non-zero seed detected, carry on, without checking...
                ++j;
                for (; j < n; ++j) {
                    x[j] = (static_cast<std::uint64_t>(storage[2 * j + 1]) << 32)
                        + static_cast<std::uint64_t>(storage[2 * j]);
                }
                return;
            }
        }

        // Fix zeros, generating some kind of seed from the
        // SeedSeq, subsequently use this seed for the seeding
        // of boost::random::splitmix64.
        std::uint64_t seed = 0;

        std::vector<int> v;
        v.reserve(seq.size());
        seq.param(std::back_inserter(v));
        int shift = 32;

        for (auto i : v) {
            seed ^= detail::xoroshiro_integer_hash((static_cast<std::uint64_t>(i) << (shift ^= int { 32 })) ^ seed);
        }

        boost::random::splitmix64 gen(seed);
		std::generate(std::begin(x), std::end(x), gen);
    }

    template<class It, std::size_t n>
    void fill_array_non_zero_int(It& first, It last, std::uint64_t (&x)[n])
    {
        if (std::distance(first, last) < (2 * n)) {
            throw(std::invalid_argument("Not enough elements in call to seed."));
        }

        std::size_t j = 0;

        for (; j < n; ++j) {
            x[j] = static_cast<std::uint64_t>(*first);
            ++first;
            x[j] |= static_cast<std::uint64_t>(*first) << 32;
            ++first;
            if (x[j]) {
                ++j;
                for (; j < n; ++j) {
                     x[j] = static_cast<std::uint64_t>(*first);
                     ++first;
                     x[j] |= static_cast<std::uint64_t>(*first) << 32;
                     ++first;
                }
                return;
            }
        }

        // Fix zeros.
        boost::random::splitmix64 gen;
		std::generate(std::begin(x), std::end(x), gen);
    }

} // namespace detail


/**
 * xoroshiro128+
 *
 * xoroshiro128+ (XOR/rotate/shift/rotate) is the successor to xorshift128+.
 *
 * Instead of perpetuating  Marsaglia's tradition  of xorshift as a  basic
 * operation, xoroshiro128+ uses a carefully handcrafted shift/rotate-based
 * linear transformation designed by Sebastiano Vigna in collaboration with
 * David Blackman.
 *
 * It is the fastest full-period generator passing BigCrush without systematic
 * failures, but due to the relatively short period it is acceptable only for
 * applications with a mild amount of parallelism; otherwise, use a
 * xorshift1024* generator.
 *
 * Beside passing BigCrush, this generator passes the PractRand test suite
 * up to (and included)  16TB, with  the exception of  binary rank  tests,
 * which fail due to the lowest bit being an LFSR; all other bits pass all
 * tests. Use a sign test to extract a random Boolean value.
 *
 * The state must be seeded so that it is not everywhere zero.
 *
 * Web-site: http://xoroshiro.di.unimi.it/
 */
class xoroshiro128plus
{
public:
    typedef std::uint64_t result_type;

    // Required for old Boost.Random concept.
    static const bool has_fixed_range = true;
    static const std::uint64_t default_seed = 1;

    /**
     * Constructs a @c xoroshiro128plus, using the default seed.
     */
    xoroshiro128plus()
    { seed(); }

    /**
     * Constructs a @c xoroshiro128plus, seeding it with @c value.
     */
    BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR(xoroshiro128plus,
                                               std::uint64_t, value)
    { seed(value); }

    /**
     * Constructs a @c xoroshiro128plus, seeding it with values
     * produced by a call to @c seq.generate().
     */
    BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR(xoroshiro128plus,
                                             SeedSeq, seq)
    { seed(seq); }

    /**
     * Constructs a @c xoroshiro128plus and seeds it with values
     * taken from the iterator range [first, last) and adjusts
     * first to point to the element after the last one used.
     * If there are not enough elements, throws @c std::invalid_argument.
     *
     * first and last must be input iterators.
     */
    template<class It>
    xoroshiro128plus(It& first, It last)
    { seed(first, last); }

    // compiler-generated copy constructor and assignment operator are fine.

    /**
     * Calls seed(default_seed)
     */
    void seed()
    { seed(default_seed); }

    /**
     * seeds a @c xoroshiro128plus with splitmix64, as per Sebastiano
     * Vigna's recommendation.
     */
    BOOST_RANDOM_DETAIL_ARITHMETIC_SEED(xoroshiro128plus, std::uint64_t, value)
    {
        std::uint64_t s = value + std::uint64_t { 0x9E3779B97F4A7C15 };
        _s[0] = splitmix64::hash(s);
        _s[1] = splitmix64::hash((s += std::uint64_t { 0x9E3779B97F4A7C15 }));
    }

    /**
     * Seeds a @c xoroshiro128plus using values from a SeedSeq. If a
     * valid seed cannot be generated throws @c std::runtime_error.
     */
    BOOST_RANDOM_DETAIL_SEED_SEQ_SEED(xoroshiro128plus, SeedSeq, seq)
    {
		detail::seed_array_non_zero_int(seq, _s);
		warmup();
	}

    /**
     * Seeds a @c xoroshiro128plus with values taken from the
     * iterator range [first, last) and adjusts @c first to
     * point to the element after the last one used. If there are
     * not enough elements or all the whole input range is zero,
     * throws @c std::invalid_argument.
     *
     * @c first and @c last must be input iterators.
     */
    template<class It>
    void seed(It& first, It last)
    {
        detail::fill_array_non_zero_int(first, last, _s);
        warmup();
    }

    /**
     * Returns the smallest value that the @c xoroshiro128plus
     * can produce.
     */
    static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ()
    { return 0; }

    /**
     * Returns the largest value that the @c xoroshiro128plus
     * can produce.
     */
    static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ()
    { return UINT64_MAX; }

    /** Returns the next value of the @c xoroshiro128plus. */
    std::uint64_t operator()()
    {
        std::uint64_t r = _s[0] + _s[1];
        next();
        return r;
    }

    /** Fills a range with random values. */
    template<class Iter>
    void generate(Iter first, Iter last)
    { detail::generate_from_int(*this, first, last); }

    /** Advances the state of the generator by @c z. */
    void discard(std::uintmax_t z)
    {
        while (z--) {
            next();
        }
    }

    /**
     * This is a jump function for the generator. It is equivalent
     * to calling @c discard(2^64) @c z times; it can be used to
     * generate 2^64 non-overlapping subsequences for parallel
     * computations.
     */
    void jump(std::uintmax_t z = 1)
    {
        while(z--) {
            std::uint64_t s0 = 0, s1 = 0;
            for (std::size_t b = 0; b < 64; ++b ) {
                if (std::uint64_t { 0xBEAC0467EBA5FACB } & std::uint64_t { 1 } << b) {
                    s0 ^= _s[0], s1 ^= _s[1];
                }
                next();
            }
            for ( std::size_t b = 0; b < 64; ++b ) {
                if (std::uint64_t { 0xD86B048B86AA9922 } & std::uint64_t { 1 } << b) {
                    s0 ^= _s[0], s1 ^= _s[1];
                }
                next();
            }
            _s[0] = s0, _s[1] = s1;
        }
    }

    friend bool operator==(const xoroshiro128plus& x,
                           const xoroshiro128plus& y)
    { return x._s[0] == y._s[0] && x._s[1] == y._s[1]; }

    friend bool operator!=(const xoroshiro128plus& x,
                           const xoroshiro128plus& y)
    { return !(x == y); }

    /** Writes a @c xoroshiro128plus to a @c std::ostream. */
    template<class CharT, class Traits>
    friend std::basic_ostream<CharT,Traits>&
    operator<<(std::basic_ostream<CharT,Traits>& os,
               const xoroshiro128plus& xoro)
    {
        os << xoro._s[0] << ' ' << xoro._s[1];
        return os;
    }

    /** Reads a @c xoroshiro128plus from a @c std::istream. */
    template<class CharT, class Traits>
    friend std::basic_istream<CharT,Traits>&
    operator>>(std::basic_istream<CharT,Traits>& is,
               xoroshiro128plus& xoro)
    {
        is >> xoro._s[0] >> std::ws >> xoro._s[1];
        return is;
    }

private:

    /// \cond show_private

    // Rotate left, use of intrinsic shows no speed-up. */
    static inline std::uint64_t rotl(const std::uint64_t x, const int k)
    { return (x << k) | (x >> (64 - k)); }

    /** Advance the state by 1 step. */
    inline void next()
    {
        _s[1] ^= _s [0];
        _s[0] = rotl(_s[0], 55);
        _s[0] ^= _s[1];
        _s[0] ^= _s[1] << 14;
        _s[1] = rotl(_s[1], 36);
    }

	// As per http://www0.cs.ucl.ac.uk/staff/D.Jones/GoodPracticeRNG.pdf
    inline void warmup()
    {
		discard(8);
    }

    /// \endcond

    std::uint64_t _s[2];
};

class xoroshiro128plusshixo
{
public:
	typedef std::uint64_t result_type;

	// Required for old Boost.Random concept.
	static const bool has_fixed_range = true;
	static const std::uint64_t default_seed = 1;

	/**
	* Constructs a @c xoroshiro128plusshixo, using the default seed.
	*/
	xoroshiro128plusshixo ( )
	{
		seed ( );
	}

	/**
	* Constructs a @c xoroshiro128plusshixo, seeding it with @c value.
	*/
	BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR ( xoroshiro128plusshixo,
		std::uint64_t, value )
	{
		seed ( value );
	}

	/**
	* Constructs a @c xoroshiro128plusshixo, seeding it with values
	* produced by a call to @c seq.generate().
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR ( xoroshiro128plusshixo,
		SeedSeq, seq )
	{
		seed ( seq );
	}

	/**
	* Constructs a @c xoroshiro128plusshixo and seeds it with values
	* taken from the iterator range [first, last) and adjusts
	* first to point to the element after the last one used.
	* If there are not enough elements, throws @c std::invalid_argument.
	*
	* first and last must be input iterators.
	*/
	template<class It>
	xoroshiro128plusshixo ( It& first, It last )
	{
		seed ( first, last );
	}

	// compiler-generated copy constructor and assignment operator are fine.

	/**
	* Calls seed(default_seed)
	*/
	void seed ( )
	{
		seed ( default_seed );
	}

	/**
	* seeds a @c xoroshiro128plusshixo with splitmix64, as per Sebastiano
	* Vigna's recommendation.
	*/
	BOOST_RANDOM_DETAIL_ARITHMETIC_SEED ( xoroshiro128plusshixo, std::uint64_t, value )
	{
		std::uint64_t s = value + std::uint64_t { 0x9E3779B97F4A7C15 };
		_s [ 0 ] = splitmix64::hash ( s );
		_s [ 1 ] = splitmix64::hash ( ( s += std::uint64_t { 0x9E3779B97F4A7C15 } ) );
	}

	/**
	* Seeds a @c xoroshiro128plusshixo using values from a SeedSeq. If a
	* valid seed cannot be generated throws @c std::runtime_error.
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_SEED ( xoroshiro128plusshixo, SeedSeq, seq )
	{
		detail::seed_array_non_zero_int ( seq, _s );
		warmup ( );
	}

	/**
	* Seeds a @c xoroshiro128plusshixo with values taken from the
	* iterator range [first, last) and adjusts @c first to
	* point to the element after the last one used. If there are
	* not enough elements or all the whole input range is zero,
	* throws @c std::invalid_argument.
	*
	* @c first and @c last must be input iterators.
	*/
	template<class It>
	void seed ( It& first, It last )
	{
		detail::fill_array_non_zero_int ( first, last, _s );
		warmup ( );
	}

	/**
	* Returns the smallest value that the @c xoroshiro128plusshixo
	* can produce.
	*/
	static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{
		return 0;
	}

	/**
	* Returns the largest value that the @c xoroshiro128plusshixo
	* can produce.
	*/
	static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{
		return UINT64_MAX;
	}

	/** Returns the next value of the @c xoroshiro128plusshixo. */
	std::uint64_t operator()( )
	{
		std::uint64_t r = _s [ 0 ] + _s [ 1 ];
		next ( );
		return ( r >> 32 ) ^ r;
	}

	/** Fills a range with random values. */
	template<class Iter>
	void generate ( Iter first, Iter last ) {

		detail::generate_from_int ( *this, first, last );

		/*

		while ( first != last ) {

			std::uint64_t tmp = _s [ 0 ] + _s [ 1 ];
			tmp ^= tmp >> 32;

			next ( );

			*first = tmp;
			++first;
		}

		*/
	}

	/** Advances the state of the generator by @c z. */
	void discard ( std::uintmax_t z )
	{
		while ( z-- ) {
			next ( );
		}
	}

	/**
	* This is a jump function for the generator. It is equivalent
	* to calling @c discard(2^64) @c z times; it can be used to
	* generate 2^64 non-overlapping subsequences for parallel
	* computations.
	*/
	void jump ( std::uintmax_t z = 1 )
	{
		while ( z-- ) {
			std::uint64_t s0 = 0, s1 = 0;
			for ( std::size_t b = 0; b < 64; ++b ) {
				if ( std::uint64_t { 0xBEAC0467EBA5FACB } &std::uint64_t { 1 } << b ) {
					s0 ^= _s [ 0 ], s1 ^= _s [ 1 ];
				}
				next ( );
			}
			for ( std::size_t b = 0; b < 64; ++b ) {
				if ( std::uint64_t { 0xD86B048B86AA9922 } &std::uint64_t { 1 } << b ) {
					s0 ^= _s [ 0 ], s1 ^= _s [ 1 ];
				}
				next ( );
			}
			_s [ 0 ] = s0, _s [ 1 ] = s1;
		}
	}

	friend bool operator==( const xoroshiro128plusshixo& x,
		const xoroshiro128plusshixo& y )
	{
		return x._s [ 0 ] == y._s [ 0 ] && x._s [ 1 ] == y._s [ 1 ];
	}

	friend bool operator!=( const xoroshiro128plusshixo& x,
		const xoroshiro128plusshixo& y )
	{
		return !( x == y );
	}

	/** Writes a @c xoroshiro128plusshixo to a @c std::ostream. */
	template<class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>&
		operator<<( std::basic_ostream<CharT, Traits>& os,
			const xoroshiro128plusshixo& xoro )
	{
		os << xoro._s [ 0 ] << ' ' << xoro._s [ 1 ];
		return os;
	}

	/** Reads a @c xoroshiro128plusshixo from a @c std::istream. */
	template<class CharT, class Traits>
	friend std::basic_istream<CharT, Traits>&
		operator>>( std::basic_istream<CharT, Traits>& is,
			xoroshiro128plusshixo& xoro )
	{
		is >> xoro._s [ 0 ] >> std::ws >> xoro._s [ 1 ];
		return is;
	}

private:

	/// \cond show_private

	// Rotate left, use of intrinsic shows no speed-up. */
	static inline std::uint64_t rotl ( const std::uint64_t x, const int k )
	{
		return ( x << k ) | ( x >> ( 64 - k ) );
	}

	/** Advance the state by 1 step. */
	inline void next ( )
	{
		_s [ 1 ] ^= _s [ 0 ];
		_s [ 0 ] = rotl ( _s [ 0 ], 55 );
		_s [ 0 ] ^= _s [ 1 ];
		_s [ 0 ] ^= _s [ 1 ] << 14;
		_s [ 1 ] = rotl ( _s [ 1 ], 36 );
	}

	// As per http://www0.cs.ucl.ac.uk/staff/D.Jones/GoodPracticeRNG.pdf
	inline void warmup ( )
	{
		discard ( 8 );
	}

	/// \endcond

	std::uint64_t _s [ 2 ];
};

class xoroshiro128plusshixostar
{
public:
	typedef std::uint64_t result_type;

	// Required for old Boost.Random concept.
	static const bool has_fixed_range = true;
	static const std::uint64_t default_seed = 1;

	/**
	* Constructs a @c xoroshiro128plusshixostar, using the default seed.
	*/
	xoroshiro128plusshixostar ( )
	{
		seed ( );
	}

	/**
	* Constructs a @c xoroshiro128plusshixostar, seeding it with @c value.
	*/
	BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR ( xoroshiro128plusshixostar,
		std::uint64_t, value )
	{
		seed ( value );
	}

	/**
	* Constructs a @c xoroshiro128plusshixostar, seeding it with values
	* produced by a call to @c seq.generate().
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR ( xoroshiro128plusshixostar,
		SeedSeq, seq )
	{
		seed ( seq );
	}

	/**
	* Constructs a @c xoroshiro128plusshixostar and seeds it with values
	* taken from the iterator range [first, last) and adjusts
	* first to point to the element after the last one used.
	* If there are not enough elements, throws @c std::invalid_argument.
	*
	* first and last must be input iterators.
	*/
	template<class It>
	xoroshiro128plusshixostar ( It& first, It last )
	{
		seed ( first, last );
	}

	// compiler-generated copy constructor and assignment operator are fine.

	/**
	* Calls seed(default_seed)
	*/
	void seed ( )
	{
		seed ( default_seed );
	}

	/**
	* seeds a @c xoroshiro128plusshixostar with splitmix64, as per Sebastiano
	* Vigna's recommendation.
	*/
	BOOST_RANDOM_DETAIL_ARITHMETIC_SEED ( xoroshiro128plusshixostar, std::uint64_t, value )
	{
		std::uint64_t s = value + std::uint64_t { 0x9E3779B97F4A7C15 };
		_s [ 0 ] = splitmix64::hash ( s );
		_s [ 1 ] = splitmix64::hash ( ( s += std::uint64_t { 0x9E3779B97F4A7C15 } ) );
	}

	/**
	* Seeds a @c xoroshiro128plusshixostar using values from a SeedSeq. If a
	* valid seed cannot be generated throws @c std::runtime_error.
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_SEED ( xoroshiro128plusshixostar, SeedSeq, seq )
	{
		detail::seed_array_non_zero_int ( seq, _s );
		warmup ( );
	}

	/**
	* Seeds a @c xoroshiro128plusshixostar with values taken from the
	* iterator range [first, last) and adjusts @c first to
	* point to the element after the last one used. If there are
	* not enough elements or all the whole input range is zero,
	* throws @c std::invalid_argument.
	*
	* @c first and @c last must be input iterators.
	*/
	template<class It>
	void seed ( It& first, It last )
	{
		detail::fill_array_non_zero_int ( first, last, _s );
		warmup ( );
	}

	/**
	* Returns the smallest value that the @c xoroshiro128plusshixostar
	* can produce.
	*/
	static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{
		return 0;
	}

	/**
	* Returns the largest value that the @c xoroshiro128plusshixostar
	* can produce.
	*/
	static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{
		return UINT64_MAX;
	}

	/** Returns the next value of the @c xoroshiro128plusshixostar. */
	std::uint64_t operator()( )
	{
		std::uint64_t r = _s [ 0 ] + _s [ 1 ];
		next ( );
		return ((r >> 32) ^ r) * std::uint64_t { 0x1AEC805299990163 };
	}

	/** Fills a range with random values. */
	template<class Iter>
	void generate ( Iter first, Iter last )
	{
		detail::generate_from_int ( *this, first, last );
	}

	/** Advances the state of the generator by @c z. */
	void discard ( std::uintmax_t z )
	{
		while ( z-- ) {
			next ( );
		}
	}

	/**
	* This is a jump function for the generator. It is equivalent
	* to calling @c discard(2^64) @c z times; it can be used to
	* generate 2^64 non-overlapping subsequences for parallel
	* computations.
	*/
	void jump ( std::uintmax_t z = 1 )
	{
		while ( z-- ) {
			std::uint64_t s0 = 0, s1 = 0;
			for ( std::size_t b = 0; b < 64; ++b ) {
				if ( std::uint64_t { 0xBEAC0467EBA5FACB } &std::uint64_t { 1 } << b ) {
					s0 ^= _s [ 0 ], s1 ^= _s [ 1 ];
				}
				next ( );
			}
			for ( std::size_t b = 0; b < 64; ++b ) {
				if ( std::uint64_t { 0xD86B048B86AA9922 } &std::uint64_t { 1 } << b ) {
					s0 ^= _s [ 0 ], s1 ^= _s [ 1 ];
				}
				next ( );
			}
			_s [ 0 ] = s0, _s [ 1 ] = s1;
		}
	}

	friend bool operator==( const xoroshiro128plusshixostar& x,
		const xoroshiro128plusshixostar& y )
	{
		return x._s [ 0 ] == y._s [ 0 ] && x._s [ 1 ] == y._s [ 1 ];
	}

	friend bool operator!=( const xoroshiro128plusshixostar& x,
		const xoroshiro128plusshixostar& y )
	{
		return !( x == y );
	}

	/** Writes a @c xoroshiro128plusshixostar to a @c std::ostream. */
	template<class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>&
		operator<<( std::basic_ostream<CharT, Traits>& os,
			const xoroshiro128plusshixostar& xoro )
	{
		os << xoro._s [ 0 ] << ' ' << xoro._s [ 1 ];
		return os;
	}

	/** Reads a @c xoroshiro128plusshixostar from a @c std::istream. */
	template<class CharT, class Traits>
	friend std::basic_istream<CharT, Traits>&
		operator>>( std::basic_istream<CharT, Traits>& is,
			xoroshiro128plusshixostar& xoro )
	{
		is >> xoro._s [ 0 ] >> std::ws >> xoro._s [ 1 ];
		return is;
	}

private:

	/// \cond show_private

	// Rotate left, use of intrinsic shows no speed-up. */
	static inline std::uint64_t rotl ( const std::uint64_t x, const int k )
	{
		return ( x << k ) | ( x >> ( 64 - k ) );
	}

	/** Advance the state by 1 step. */
	inline void next ( )
	{
		_s [ 1 ] ^= _s [ 0 ];
		_s [ 0 ] = rotl ( _s [ 0 ], 55 );
		_s [ 0 ] ^= _s [ 1 ];
		_s [ 0 ] ^= _s [ 1 ] << 14;
		_s [ 1 ] = rotl ( _s [ 1 ], 36 );
	}

	// As per http://www0.cs.ucl.ac.uk/staff/D.Jones/GoodPracticeRNG.pdf
	inline void warmup ( )
	{
		discard ( 8 );
	}

	/// \endcond

	std::uint64_t _s [2];
};


class xoroshiro128plusshixostarshixo
{
public:
	typedef std::uint64_t result_type;

	// Required for old Boost.Random concept.
	static const bool has_fixed_range = true;
	static const std::uint64_t default_seed = 1;

	/**
	* Constructs a @c xoroshiro128plusshixostarshixo, using the default seed.
	*/
	xoroshiro128plusshixostarshixo ( )
	{
		seed ( );
	}

	/**
	* Constructs a @c xoroshiro128plusshixostarshixo, seeding it with @c value.
	*/
	BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR ( xoroshiro128plusshixostarshixo,
		std::uint64_t, value )
	{
		seed ( value );
	}

	/**
	* Constructs a @c xoroshiro128plusshixostarshixo, seeding it with values
	* produced by a call to @c seq.generate().
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR ( xoroshiro128plusshixostarshixo,
		SeedSeq, seq )
	{
		seed ( seq );
	}

	/**
	* Constructs a @c xoroshiro128plusshixostarshixo and seeds it with values
	* taken from the iterator range [first, last) and adjusts
	* first to point to the element after the last one used.
	* If there are not enough elements, throws @c std::invalid_argument.
	*
	* first and last must be input iterators.
	*/
	template<class It>
	xoroshiro128plusshixostarshixo ( It& first, It last )
	{
		seed ( first, last );
	}

	// compiler-generated copy constructor and assignment operator are fine.

	/**
	* Calls seed(default_seed)
	*/
	void seed ( )
	{
		seed ( default_seed );
	}

	/**
	* seeds a @c xoroshiro128plusshixostarshixo with splitmix64, as per Sebastiano
	* Vigna's recommendation.
	*/
	BOOST_RANDOM_DETAIL_ARITHMETIC_SEED ( xoroshiro128plusshixostarshixo, std::uint64_t, value )
	{
		std::uint64_t s = value + std::uint64_t { 0x9E3779B97F4A7C15 };
		_s [ 0 ] = splitmix64::hash ( s );
		_s [ 1 ] = splitmix64::hash ( ( s += std::uint64_t { 0x9E3779B97F4A7C15 } ) );
	}

	/**
	* Seeds a @c xoroshiro128plusshixostarshixo using values from a SeedSeq. If a
	* valid seed cannot be generated throws @c std::runtime_error.
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_SEED ( xoroshiro128plusshixostarshixo, SeedSeq, seq )
	{
		detail::seed_array_non_zero_int ( seq, _s );
		warmup ( );
	}

	/**
	* Seeds a @c xoroshiro128plusshixostarshixo with values taken from the
	* iterator range [first, last) and adjusts @c first to
	* point to the element after the last one used. If there are
	* not enough elements or all the whole input range is zero,
	* throws @c std::invalid_argument.
	*
	* @c first and @c last must be input iterators.
	*/
	template<class It>
	void seed ( It& first, It last )
	{
		detail::fill_array_non_zero_int ( first, last, _s );
		warmup ( );
	}

	/**
	* Returns the smallest value that the @c xoroshiro128plusshixostarshixo
	* can produce.
	*/
	static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{
		return 0;
	}

	/**
	* Returns the largest value that the @c xoroshiro128plusshixostarshixo
	* can produce.
	*/
	static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{
		return UINT64_MAX;
	}

	/** Returns the next value of the @c xoroshiro128plusshixostarshixo. */
	std::uint64_t operator()( )
	{
		std::uint64_t r = _s [ 0 ] + _s [ 1 ];
		next ( );
		r = ( ( r >> 32 ) ^ r ) * std::uint64_t { 0x1AEC805299990163 };
		return ( r >> 32 ) ^ r;
	}

	/** Fills a range with random values. */
	template<class Iter>
	void generate ( Iter first, Iter last )
	{
		detail::generate_from_int ( *this, first, last );
	}

	/** Advances the state of the generator by @c z. */
	void discard ( std::uintmax_t z )
	{
		while ( z-- ) {
			next ( );
		}
	}

	/**
	* This is a jump function for the generator. It is equivalent
	* to calling @c discard(2^64) @c z times; it can be used to
	* generate 2^64 non-overlapping subsequences for parallel
	* computations.
	*/
	void jump ( std::uintmax_t z = 1 )
	{
		while ( z-- ) {
			std::uint64_t s0 = 0, s1 = 0;
			for ( std::size_t b = 0; b < 64; ++b ) {
				if ( std::uint64_t { 0xBEAC0467EBA5FACB } &std::uint64_t { 1 } << b ) {
					s0 ^= _s [ 0 ], s1 ^= _s [ 1 ];
				}
				next ( );
			}
			for ( std::size_t b = 0; b < 64; ++b ) {
				if ( std::uint64_t { 0xD86B048B86AA9922 } &std::uint64_t { 1 } << b ) {
					s0 ^= _s [ 0 ], s1 ^= _s [ 1 ];
				}
				next ( );
			}
			_s [ 0 ] = s0, _s [ 1 ] = s1;
		}
	}

	friend bool operator==( const xoroshiro128plusshixostarshixo& x,
		const xoroshiro128plusshixostarshixo& y )
	{
		return x._s [ 0 ] == y._s [ 0 ] && x._s [ 1 ] == y._s [ 1 ];
	}

	friend bool operator!=( const xoroshiro128plusshixostarshixo& x,
		const xoroshiro128plusshixostarshixo& y )
	{
		return !( x == y );
	}

	/** Writes a @c xoroshiro128plusshixostarshixo to a @c std::ostream. */
	template<class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>&
		operator<<( std::basic_ostream<CharT, Traits>& os,
			const xoroshiro128plusshixostarshixo& xoro )
	{
		os << xoro._s [ 0 ] << ' ' << xoro._s [ 1 ];
		return os;
	}

	/** Reads a @c xoroshiro128plusshixostarshixo from a @c std::istream. */
	template<class CharT, class Traits>
	friend std::basic_istream<CharT, Traits>&
		operator>>( std::basic_istream<CharT, Traits>& is,
			xoroshiro128plusshixostarshixo& xoro )
	{
		is >> xoro._s [ 0 ] >> std::ws >> xoro._s [ 1 ];
		return is;
	}

private:

	/// \cond show_private

	// Rotate left, use of intrinsic shows no speed-up. */
	static inline std::uint64_t rotl ( const std::uint64_t x, const int k )
	{
		return ( x << k ) | ( x >> ( 64 - k ) );
	}

	/** Advance the state by 1 step. */
	inline void next ( )
	{
		_s [ 1 ] ^= _s [ 0 ];
		_s [ 0 ] = rotl ( _s [ 0 ], 55 );
		_s [ 0 ] ^= _s [ 1 ];
		_s [ 0 ] ^= _s [ 1 ] << 14;
		_s [ 1 ] = rotl ( _s [ 1 ], 36 );
	}

	// As per http://www0.cs.ucl.ac.uk/staff/D.Jones/GoodPracticeRNG.pdf
	inline void warmup ( )
	{
		discard ( 8 );
	}

	/// \endcond

	std::uint64_t _s [ 2 ];
};

/**
 * xorshift128+
 *
 * This generator has been replaced by xoroshiro128+, which is
 * significantly faster and has better statistical properties.
 *
 * Due to the relatively short period it is acceptable only for
 * applications with a mild amount of parallelism; otherwise, use a
 * xorshift1024* generator.
 *
 * The lowest bit of this generator is an LSFR, and thus it is
 * slightly less random than the other bits. Use a sign test to
 * extract a random Boolean value.
 *
 * The state must be seeded so that it is not everywhere zero.
 *
 * Web-site: http://xoroshiro.di.unimi.it/
 */
class xorshift128plus
{
public:
    typedef std::uint64_t result_type;

    // Required for old Boost.Random concept.
    static const bool has_fixed_range = true;
    static const std::uint64_t default_seed = 1;

    /**
     * Constructs a @c xorshift128plus, using the default seed.
     */
    xorshift128plus()
    { seed(); }

    /**
     * Constructs a @c xorshift128plus, seeding it with @c value.
     */
    BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR(xorshift128plus,
                                               std::uint64_t, value)
    { seed(value); }

    /**
     * Constructs a @c xorshift128plus, seeding it with values
     * produced by a call to @c seq.generate().
     */
    BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR(xorshift128plus,
                                             SeedSeq, seq)
    { seed(seq); }

    /**
     * Constructs a @c xorshift128plus and seeds it with values
     * taken from the iterator range [first, last) and adjusts
     * first to point to the element after the last one used.
     * If there are not enough elements, throws @c std::invalid_argument.
     *
     * first and last must be input iterators.
     */
    template<class It>
    xorshift128plus(It& first, It last)
    { seed(first, last); }

    // compiler-generated copy constructor and assignment operator are fine.

    /**
     * Calls seed(default_seed)
     */
    void seed()
    { seed(default_seed); }

    /**
     * seeds a @c xorshift128plus with splitmix64, as per Sebastiano
     * Vigna's recommendation.
     */
    BOOST_RANDOM_DETAIL_ARITHMETIC_SEED(xorshift128plus, std::uint64_t, value)
    {
        std::uint64_t s = value + std::uint64_t { 0x9E3779B97F4A7C15 };
        _s[0] = splitmix64::hash(s);
        _s[1] = splitmix64::hash((s += std::uint64_t { 0x9E3779B97F4A7C15 }));
    }

    /**
     * Seeds a @c xorshift128plus using values from a SeedSeq. If a
     * valid seed cannot be generated throws @c std::runtime_error.
     */
    BOOST_RANDOM_DETAIL_SEED_SEQ_SEED(xorshift128plus, SeedSeq, seq)
    {
		detail::seed_array_non_zero_int(seq, _s);
		warmup();
	}

    /**
     * Seeds a @c xorshift128plus with values taken from the
     * iterator range [first, last) and adjusts @c first to
     * point to the element after the last one used. If there are
     * not enough elements or all the whole input range is zero,
     * throws @c std::invalid_argument.
     *
     * @c first and @c last must be input iterators.
     */
    template<class It>
    void seed(It& first, It last)
    {
        detail::fill_array_non_zero_int(first, last, _s);
        warmup();
    }

    /**
     * Returns the smallest value that the @c xorshift128plus
     * can produce.
     */
    static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ()
    { return 0; }

    /**
     * Returns the largest value that the @c xorshift128plus
     * can produce.
     */
    static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ()
    { return UINT64_MAX; }

    /** Returns the next value of the @c xorshift128plus. */
    std::uint64_t operator()()
    {
        const std::uint64_t r = _s[0] + _s[1];
        next();
        return r;
    }

    /** Fills a range with random values. */
    template<class Iter>
    void generate(Iter first, Iter last)
    { detail::generate_from_int(*this, first, last); }

    /** Advances the state of the generator by @c z. */
    void discard(std::uintmax_t z)
    {
        while (z--) {
            next();
        }
    }

    /**
     * This is a jump function for the generator. It is equivalent
     * to calling @c discard(2^64) @c z times; it can be used to
     * generate 2^64 non-overlapping subsequences for parallel
     * computations.
     */
    void jump(std::uintmax_t z = 1)
    {
        static const std::uint64_t jmp [2] {
            0x8a5cd789635d2dff, 0x121fd2155c472f96
        };

        while(z--) {
            std::uint64_t s0 = 0, s1 = 0;
            for (std::size_t i = 0; i < 2; ++i) {
                for (std::size_t b = 0; b < 64; ++b) {
                    if (jmp[i] & std::uint64_t { 1 } << b) {
                        s0 ^= _s[0], s1 ^= _s[1];
                    }
                    next();
                }
            }
            _s[0] = s0, _s[1] = s1;
        }
    }

    friend bool operator==(const xorshift128plus& x,
                           const xorshift128plus& y)
    { return x._s[0] == y._s[0] && x._s[1] == y._s[1]; }

    friend bool operator!=(const xorshift128plus& x,
                           const xorshift128plus& y)
    { return !(x == y); }

    /** Writes a @c xorshift128plus to a @c std::ostream. */
    template<class CharT, class Traits>
    friend std::basic_ostream<CharT,Traits>&
    operator<<(std::basic_ostream<CharT,Traits>& os,
               const xorshift128plus& xosh)
    {
        os << xosh._s[0] << ' ' << xosh._s[1];
        return os;
    }

    /** Reads a @c xorshift128plus from a @c std::istream. */
    template<class CharT, class Traits>
    friend std::basic_istream<CharT,Traits>&
    operator>>(std::basic_istream<CharT,Traits>& is,
               xorshift128plus& xosh)
    {
        is >> xosh._s[0] >> std::ws >> xosh._s[1];
        return is;
    }

private:

    /// \cond show_private

    /** Advance the state by 1 step. */
    inline void next()
    {
        std::uint64_t s1 = _s[0];
        _s[0] = _s[1];
        s1 ^= s1 << 23;
        _s[1] = s1 ^ _s[0] ^ (s1 >> 18) ^ (_s[0] >> 5);
    }

	// As per http://www0.cs.ucl.ac.uk/staff/D.Jones/GoodPracticeRNG.pdf
	inline void warmup()
	{
		discard(8);
	}

    /// \endcond

    std::uint64_t _s[2];
};


/**
 * xorshift1024*.
 *
 * This is a fast, top-quality generator. If 1024 bits of state are too
 * much, a xoroshiro128+ generator can be used.
 *
 * The three lowest bits of this generator are LSFRs, and thus they are
 * slightly less random than the other bits. Use a sign test to extract
 * a random Boolean value.
 *
 * The state must be seeded so that it is not everywhere zero.
 *
 * Web-site: http://xoroshiro.di.unimi.it/
 *
 * There was an issue with an earlier version of this code, the corrected
 * version is implemented below.
 *
 * https://stackoverflow.com/questions/34574701/xorshift1024-jump-not-commutative
 */
class xorshift1024star
{
public:
    typedef std::uint64_t result_type;

    // Required for old Boost.Random concept.
    static const bool has_fixed_range = true;
    static const std::uint64_t default_seed = 1;

    /**
     * Constructs a @c xorshift1024star, using the default seed.
     */
    xorshift1024star()
    { seed(); }

    /**
     * Constructs a @c xorshift1024star, seeding it with @c value.
     */
    BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR(xorshift1024star,
                                               std::uint64_t, value)
    { seed(value); }

    /**
     * Constructs a @c xorshift1024star, seeding it with values
     * produced by a call to @c seq.generate().
     */
    BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR(xorshift1024star,
                                             SeedSeq, seq)
    { seed(seq); }

    /**
     * Constructs a @c xorshift1024star and seeds it with values
     * taken from the iterator range [first, last) and adjusts
     * first to point to the element after the last one used.
     * If there are not enough elements, throws @c std::invalid_argument.
     *
     * first and last must be input iterators.
     */
    template<class It>
    xorshift1024star(It& first, It last)
    { seed(first, last); }

    // compiler-generated copy constructor and assignment operator are fine.

    /**
     * Calls seed(default_seed)
     */
    void seed()
    { seed(default_seed); }

    /**
     * seeds a @c xorshift1024star with splitmix64, as per Sebastiano
     * Vigna's recommendation.
     */
    BOOST_RANDOM_DETAIL_ARITHMETIC_SEED(xorshift1024star, std::uint64_t, value)
    {
        std::uint64_t s = value + std::uint64_t { 0x9E3779B97F4A7C15 };
        _s[0] = splitmix64::hash(s);
        for (std::size_t i = 1; i < 16; ++i) {
            _s[i] = splitmix64::hash ((s += std::uint64_t { 0x9E3779B97F4A7C15 }));
        }
        _p = 0;
    }

    /**
     * Seeds a @c xorshift1024star using values from a SeedSeq. If a
     * valid seed cannot be generated throws @c std::runtime_error.
     */
    BOOST_RANDOM_DETAIL_SEED_SEQ_SEED(xorshift1024star, SeedSeq, seq)
    {
        detail::seed_array_non_zero_int(seq, _s);
        _p = 0;
		warmup();
    }

    /**
     * Seeds a @c xorshift1024star with values taken from the
     * iterator range [first, last) and adjusts @c first to
     * point to the element after the last one used. If there are
     * not enough elements or all the whole input range is zero,
     * throws @c std::invalid_argument.
     *
     * @c first and @c last must be input iterators.
     */
    template<class It>
    void seed(It& first, It last)
    {
        detail::fill_array_non_zero_int(first, last, _s);
        _p = 0;
        warmup();
    }

    /**
     * Returns the smallest value that the @c xorshift1024star
     * can produce.
     */
    static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ()
    { return 0; }

    /**
     * Returns the largest value that the @c xorshift1024star
     * can produce.
     */
    static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ()
    { return UINT64_MAX; }

    /** Returns the next value of the @c xorshift1024star. */
    std::uint64_t operator()()
    {
        next();
        return _s[_p] * std::uint64_t { 0x106689D45497FDB5 };
    }

    /** Fills a range with random values. */
    template<class Iter>
    void generate(Iter first, Iter last)
    { detail::generate_from_int(*this, first, last); }

    /** Advances the state of the generator by @c z. */
    void discard(std::uintmax_t z)
    {
        while (z--) {
            next();
        }
    }

    /**
     * This is a jump function for the generator. It is equivalent
     * to calling @c discard(2^512) @c z times; it can be used to
     * generate 2^512 non-overlapping subsequences for parallel
     * computations.
     */
    void jump(std::uintmax_t z = 1)
    {
        static const std::uint64_t jmp[16] {
            0x84242F96ECA9C41D, 0xA3C65B8776F96855,
            0x5B34A39F070B5837, 0x4489AFFCE4F31A1E,
            0x2FFEEB0A48316F40, 0xDC2D9891FE68C022,
            0x3659132BB12FEA70, 0xAAC17D8EFA43CAB8,
            0xC4CB815590989B13, 0x5EE975283D71C93B,
            0x691548C86C1BD540, 0x7910C41D10A1E6A5,
            0x0B5FC64563B3E2A8, 0x047F7684E9FC949D,
            0xB99181F2D8F685CA, 0x284600E3F30E38C3
        };

        while(z--) {
            std::uint64_t t[16];
            for (std::size_t i = 0; i < 16; ++i) {
                for (std::size_t b = 0; b < 64; ++b) {
                    if (jmp[i] & std::uint64_t { 1 } << b) {
                        for (std::size_t j = 0; j < 16; ++j) {
                            t [j] ^= _s[(j + _p) & 15];
                        }
                    }
                    next();
                }
            }
            for (std::size_t j = 0; j < 16; ++j) {
                _s [(j + _p) & 15] = t[j];
            }
        }
    }

    friend bool operator==(const xorshift1024star& x,
        const xorshift1024star& y)
	{
		std::size_t i = x._p, j = y._p;
		for (; i < 16; ++i, j = (j + 1) & 15) {
			if (x._s[i] != y._s[j]) {
				return false;
			}
		}
		i = 0;
		for ( ; i < x._p; ++i, j = (j + 1) & 15) {
			if (x._s[i] != y._s[j]) {
				return false;
			}
		}
		return true;
    }

    friend bool operator!=(const xorshift1024star& x,
                           const xorshift1024star& y)
    { return !(x == y); }

    /** Writes a @c xorshift1024star to a @c std::ostream. */
    template<class CharT, class Traits>
    friend std::basic_ostream<CharT,Traits>&
    operator<<(std::basic_ostream<CharT,Traits>& os,
               const xorshift1024star& xosh)
    {
		std::size_t i = xosh._p;
		for (; i < 16; ++i) {
			os << xosh._s[i] << ' ';
		}
		i = 0;
		for (; i < xosh._p; ++i) {
			os << xosh._s[i] << ' ';
		}
        return os;
    }

    /** Reads a @c xorshift1024star from a @c std::istream. */
    template<class CharT, class Traits>
    friend std::basic_istream<CharT,Traits>&
    operator>>(std::basic_istream<CharT,Traits>& is,
               xorshift1024star& xosh)
    {
		for (std::size_t i = 0; i < 16; ++i) {
			is >> xosh._s[i] >> std::ws;
		}
        xosh._p = 0;
        return is;
    }

private:

    /// \cond show_private

    /** Advance the state by 1 step. */
    inline void next()
    {
        const std::uint64_t s0 = _s[_p];
        std::uint64_t s1 = _s[(_p = (_p + 1) & 15)];
        s1 ^= s1 << 31;
        _s[_p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);
    }

	// As per http://www0.cs.ucl.ac.uk/staff/D.Jones/GoodPracticeRNG.pdf
	inline void warmup()
	{
		discard(64);
	}

    /// \endcond

    std::uint64_t _s[16];
    std::size_t _p;
};

#endif /* !BOOST_NO_INT64_T && !BOOST_NO_INTEGRAL_INT64_T */

} // namespace random
} // namespace boost

#include <boost/random/detail/enable_warnings.hpp>

#endif // BOOST_RANDOM_XOROSHIRO_HPP
