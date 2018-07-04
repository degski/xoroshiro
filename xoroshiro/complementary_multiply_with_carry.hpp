
#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <algorithm>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/random/detail/config.hpp>
#include <boost/random/detail/seed.hpp>
#include <boost/random/detail/seed_impl.hpp>
#include <boost/detail/workaround.hpp>
#include "xoroshiro.hpp"
#include <boost/random/detail/disable_warnings.hpp>


namespace boost {
namespace random {

#if !defined(BOOST_NO_INT64_T) && !defined(BOOST_NO_INTEGRAL_INT64_T)

template<typename IntType, std::size_t w, std::size_t CMWC_CYCLE, std::uint32_t CMWC_C_MAX, std::uint64_t A, bool is_complementary>
class complementary_multiply_with_carry_engine {
	typedef typename std::conditional<is_complementary, std::true_type, std::false_type>::type complementary;
public:
	typedef IntType result_type;

	// Required for old Boost.Random concept.
	static const bool has_fixed_range = true;
	static const IntType default_seed = 1;

	/**
	* Constructs a @c complementary_multiply_with_carry_engine, using the default seed.
	*/
	complementary_multiply_with_carry_engine()
	{
		seed();
	}

	/**
	* Constructs a @c complementary_multiply_with_carry_engine, seeding it with @c value.
	*/
	template<typename IT>
	BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR(complementary_multiply_with_carry_engine,
		IT, value)
	{
		seed(value);
	}

	/**
	* Constructs a @c complementary_multiply_with_carry_engine, seeding it with values
	* produced by a call to @c seq.generate().
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR(complementary_multiply_with_carry_engine,
		SeedSeq, seq)
	{
		seed(seq);
	}

	/**
	* Constructs a @c complementary_multiply_with_carry_engine and seeds it with values
	* taken from the iterator range [first, last) and adjusts
	* first to point to the element after the last one used.
	* If there are not enough elements, throws @c std::invalid_argument.
	*
	* first and last must be input iterators.
	*/
	template<class It>
	complementary_multiply_with_carry_engine(It& first, It last)
	{
		seed(first, last);
	}

	// compiler-generated copy constructor and assignment operator are fine.

	/**
	* Calls seed(default_seed)
	*/
	void seed()
	{
		seed(default_seed);
	}

	/**
	* seeds a @c complementary_multiply_with_carry_engine with splitmix64.
	*/
	template<typename IT>
	BOOST_RANDOM_DETAIL_ARITHMETIC_SEED(complementary_multiply_with_carry_engine, IT, value)
	{
		BOOST_STATIC_ASSERT(std::numeric_limits<IT>::is_integer);

		boost::random::splitmix64 intgen(static_cast<std::uint64_t>(value));
		detail::generator_seed_seq<boost::random::splitmix64> gen(intgen);
		seed(gen);
	}

	/**
	* Seeds a @c complementary_multiply_with_carry_engine using values from a SeedSeq.
	*/
	BOOST_RANDOM_DETAIL_SEED_SEQ_SEED(complementary_multiply_with_carry_engine, SeedSeq, seq)
	{
		detail::seed_array_int<w>(seq, _Q);
		_carry = detail::seed_one_int<IntType, CMWC_C_MAX>(seq);
		_i = CMWC_CYCLE - 1;
		warmup();
	}

	/**
	* Seeds a @c complementary_multiply_with_carry_engine with values taken from the
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
		detail::fill_array_int<w>(first, last, _Q );
		_carry = detail::seed_one_int<IntType, CMWC_C_MAX>(first, last);
		_i = CMWC_CYCLE - 1;
		warmup();
	}

	/**
	* Returns the smallest value that the @c complementary_multiply_with_carry_engine
	* can produce.
	*/
	static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{ return 0; }

	/**
	* Returns the largest value that the @c complementary_multiply_with_carry_engine
	* can produce.
	*/
	static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ( )
	{ return boost::low_bits_mask_t<w>::sig_bits; }

	/**
	* Returns the next value of the @c
	* complementary_multiply_with_carry_engine,
	* complemented.
	*/
	template<typename U = complementary>
		typename std::enable_if<
			std::is_same<U, std::true_type>::value, result_type>::type
	operator()( )
	{
		_i = (_i + 1) & (CMWC_CYCLE - 1);
		const std::uint64_t t = A * _Q[_i] + _carry;
		_carry = t >> 32;
		std::uint32_t x = t + _carry;
		if (x < _carry) {
			++x, ++_carry;
		}
		return (_Q[_i] = std::uint32_t { 0xFFFFFFFE } - x); // m - x
	}

	/**
	 * Returns the next value of the @c
	 * complementary_multiply_with_carry_engine,
	 * not-complemented.
	 */
	template<typename U = complementary>
		typename std::enable_if<
			std::is_same<U, std::false_type>::value, result_type>::type
	operator()( )
	{
		_i = (_i + 1) & (CMWC_CYCLE - 1);
		const std::uint64_t t = A * _Q[_i] + _carry;
		_carry = t >> 32;
		std::uint32_t x = t + _carry;
		if (x < _carry) {
			++x, ++_carry;
		}
		return (_Q[_i] = x);
	}

	/** Fills a range with random values. */
	template<class Iter>
	void generate(Iter first, Iter last)
	{
		detail::generate_from_int(*this, first, last);
	}

	/** Advances the state of the generator by @c z. */
	void discard(std::uintmax_t z)
	{
		while (z--) {
			_i = (_i + 1) & (CMWC_CYCLE - 1);
			const std::uint64_t t = A * _Q[_i] + _carry;
			_carry = t >> 32;
			_carry += ((t + _carry) < _carry);
		}
	}

	/** Writes a @c complementary_multiply_with_carry_engine to a @c std::ostream. */
	template<class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>&
		operator<<( std::basic_ostream<CharT, Traits>& os,
			const complementary_multiply_with_carry_engine& cmwc)
	{
		std::size_t i = cmwc._i;
		for (; i < CMWC_CYCLE; ++i) {
			os << cmwc._Q[i] << ' ';
		}
		i = 0;
		for (; i < cmwc._i; ++i) {
			os << cmwc._Q[i] << ' ';
		}
		os << cmwc._carry;
		return os;
	}

	/** Reads a @c complementary_multiply_with_carry_engine from a @c std::istream. */
	template<class CharT, class Traits>
	friend std::basic_istream<CharT, Traits>&
		operator >> ( std::basic_istream<CharT, Traits>& is,
			complementary_multiply_with_carry_engine& cmwc)
	{
		for (std::size_t i = 0; i < CMWC_CYCLE; ++i) {
			is >> cmwc._Q [ i ] >> std::ws;
		}
		is >> cmwc._carry;
		cmwc._i = 0;
		return is;
	}

private:

	// As per http://www0.cs.ucl.ac.uk/staff/D.Jones/GoodPracticeRNG.pdf
	inline void warmup()
	{
		discard(4 * CMWC_CYCLE);
	}

	IntType _Q[CMWC_CYCLE];
	std::size_t _i;
	IntType _carry;
};


using mwc256 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 256, 809430660, 1540315826, false>;

/*
 * Generators below from:
 *
 * Marsaglia, George (2003) "Random Number Generators,"Journal of Modern Applied Statistical Methods: Vol. 2 : Iss. 1 , Article 2, Page 9.
 * DOI: 10.22237/jmasm/1051747320
 * Available at: http://digitalcommons.wayne.edu/jmasm/vol2/iss1/2
 */

using cmwc4 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 4, 123, 987651670, true>;
using cmwc8 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 8, 123, 987651386, true>;
using cmwc16 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 16, 123, 987651178, true>;
using cmwc32 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 32, 123, 987655670, true>;
using cmwc64 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 64, 123, 987651206, true>;
using cmwc128 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 128, 123, 987688302, true>;
using cmwc256 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 256, 123, 987662290, true>;
using cmwc512 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 512, 123, 123462658, true>;
using cmwc1024 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 1024, 123, 5555698, true>;
using cmwc2048 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 2048, 123, 1030770, true>;

using cmwc4096 = complementary_multiply_with_carry_engine<std::uint32_t, 32, 4096, 809430660, 18782, true>;

#endif /* !BOOST_NO_INT64_T && !BOOST_NO_INTEGRAL_INT64_T */

} // namespace random
} // namespace boost
