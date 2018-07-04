/* boost random/xor_combine.hpp header file
 *
 * Copyright Jens Maurer 2002
 * Copyright Steven Watanabe 2010
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

#ifndef BOOST_RANDOM_XOR_COMBINE_HPP
#define BOOST_RANDOM_XOR_COMBINE_HPP

#include <istream>
#include <iosfwd>
#include <cassert>
#include <vector>
#include <algorithm> // for std::min and std::max
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/cstdint.hpp>     // uint32_t

#include <boost/type_traits/add_lvalue_reference.hpp>
#include <boost/type_traits/conditional.hpp>
#include <boost/type_traits/declval.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/type_traits/is_copy_assignable.hpp>
#include <boost/type_traits/is_copy_constructible.hpp>
#include <boost/type_traits/is_empty.hpp>
#include <boost/type_traits/is_rvalue_reference.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/make_void.hpp>

#include <boost/random/detail/config.hpp>
#include <boost/random/detail/seed.hpp>
#include <boost/random/detail/seed_impl.hpp>
#include <boost/random/detail/operators.hpp>


namespace boost {
namespace random {

	class seed_seq;

namespace traits {

struct oops;

template<class B>
struct negation : boost::integral_constant<bool, !bool ( B::value )> {};

#define BOOST_RANDOM_HAS_GENERIC(trait_name, what_type)                                                        \
template<typename T, typename return_type, typename... args>                                                   \
struct trait_name                                                                                              \
{                                                                                                              \
private:                                                                                                       \
	template<typename arg>                                                                                     \
	static typename boost::conditional<                                                                        \
		boost::is_rvalue_reference<arg>::value,                                                                \
		arg,                                                                                                   \
		typename boost::add_lvalue_reference<arg>::type>::type                                                 \
		refval();                                                                                              \
	template<typename check_type, typename... check_args>                                                      \
	static what_type check(int);                                                                               \
	template<typename check_type, typename... check_args>                                                      \
	static boost::random::traits::oops check(...);                                                             \
public:                                                                                                        \
	static constexpr bool value = boost::is_same<decltype(check<T, args...>(0)), return_type>::value;          \
    typedef typename boost::conditional<value, return_type, boost::false_type>::type type;                     \
};

#define BOOST_RANDOM_HAS_METHOD(function_name) \
BOOST_RANDOM_HAS_GENERIC(has_##function_name, decltype(boost::declval<check_type>().function_name(refval<check_args>()...)))
#define BOOST_RANDOM_HAS_OPERATOR(operator_name, operator_symbol) \
BOOST_RANDOM_HAS_GENERIC(has_##operator_name##_operator, decltype(boost::declval<check_type>().operator_symbol(refval<check_args>()...)))
#define BOOST_RANDOM_HAS_TYPE_(type_name) \
BOOST_RANDOM_HAS_GENERIC(has_type_##type_name, typename check_type::type_name*)

#define BOOST_RANDOM_HAS_STATIC(function_name)                                                                            \
template <typename T, typename return_type, typename... args>                                                  \
class has_static_##function_name                                                                               \
{                                                                                                              \
private:                                                                                                       \
	template<typename U, U> struct helper;                                                                     \
	template<typename check_type>                                                                              \
	static decltype(boost::declval<helper<return_type(*)(args...), &check_type::function_name>>()) check(int); \
    template<typename check_type>                                                                              \
    static boost::random::traits::oops check(...);                                                             \
public:                                                                                                        \
	static constexpr bool value = negation<boost::is_same<                                                     \
		decltype(check<T>(0)), boost::random::traits::oops>                                                    \
	>::value;                                                                                                  \
};

namespace detail {

template<typename... Ts>
using void_t = typename boost::make_void<Ts...>::type;

struct nonesuch {
	nonesuch() = delete;
	~nonesuch() = delete;
	nonesuch(nonesuch const&) = delete;
	void operator=(nonesuch const&) = delete;
};

template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
struct detector {
	using value_t = boost::false_type;
	using type = Default;
};

template<class Default, template<class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
	using value_t = boost::true_type;
	using type = Op<Args...>;
};

template<template<class...> class Op, class... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;
template<template<class...> class Op, class... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

} // namespace detail

#define BOOST_RANDOM_HAS_TYPE(type) \
namespace detail { template<typename T> using type##_type = typename T::type; } \
template<typename T, class... Args> \
using has_type_##type = detail::is_detected<detail::type##_type, T, Args...>; \
template<typename T, class... Args> \
using type##_t = detail::detected_t<detail::type##_type, T, Args...>;


// Member Types
BOOST_RANDOM_HAS_TYPE(result_type)
// Member Functions
BOOST_RANDOM_HAS_METHOD(seed)
BOOST_RANDOM_HAS_METHOD(discard)
BOOST_RANDOM_HAS_METHOD(entropy)
BOOST_RANDOM_HAS_METHOD(generate)
BOOST_RANDOM_HAS_METHOD(size)
BOOST_RANDOM_HAS_METHOD(param)
BOOST_RANDOM_HAS_METHOD(max)
// Operators
BOOST_RANDOM_HAS_OPERATOR(call, operator())
// Static Member Functions
BOOST_RANDOM_HAS_STATIC(min)
BOOST_RANDOM_HAS_STATIC(max)

// Generator
template<typename G>
struct is_generator
{
	static constexpr bool value =
		negation<boost::is_empty<G>>::value
		&& has_type_result_type<G>::value
		&& has_seed<G, void>::value
		&& has_call_operator<G, result_type_t<G>>::value
		&& has_discard<G, void, unsigned long long>::value
		&& has_static_min<G, result_type_t<G>>::value
		&& has_static_max<G, result_type_t<G>>::value;
};

// Device
template<typename D>
struct is_random_device
{
	static constexpr bool value =
		boost::is_class<D>::value
		&& has_type_result_type<D>::value
		&& boost::is_same<result_type_t<D>, unsigned int>::value
		&& negation<boost::is_copy_constructible<D>>::value
		&& negation<boost::is_copy_assignable<D>>::value
		&& has_call_operator<D, result_type_t<D>>::value
		&& has_entropy<D, double>::value
		&& has_static_min<D, result_type_t<D>>::value
		&& has_static_max<D, result_type_t<D>>::value;
};

// Sequence
template<typename S>
struct is_seed_seq
{
	static constexpr bool value =
		negation<boost::is_empty<S>>::value
		&& has_type_result_type<S>::value
		&& sizeof(result_type_t<S>) >= sizeof(std::uint32_t)
		// && negation<boost::is_copy_constructible<S>>::value // boost::random::seed_seq does not pass this.
		// && negation<boost::is_copy_assignable<S>>::value // boost::random::seed_seq does not pass this.
		&& has_generate<S, void, typename std::vector<result_type_t<S>>::iterator, typename std::vector<result_type_t<S>>::iterator>::value
		&& has_size<S, std::size_t>::value
		&& has_param<S, void, std::ostreambuf_iterator<char>>::value;
};

}

namespace detail {

	inline std::uint32_t xor_combine_integer_hash(std::uint32_t x)
	{
		x = ((x >> 16) ^ x) * std::uint32_t { 0x45D9F3B };
		return (x >> 16) ^ x;
	}

	inline std::uint64_t xor_combine_integer_hash(std::uint64_t x)
	{
		x = ((x >> 32) ^ x) * std::uint64_t { 0x1AEC805299990163 };
		return (x >> 32) ^ x;
	}
}


/**
 * Instantiations of @c xor_combine_engine model a
 * \pseudo_random_number_generator.  To produce its output it
 * invokes each of the base generators, shifts their results
 * and xors them together.
 */
template<class URNG1, int s1, class URNG2, int s2>
class xor_combine_engine
{
public:
    typedef URNG1 base1_type;
    typedef URNG2 base2_type;
    typedef typename base1_type::result_type result_type;

    BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);
    BOOST_STATIC_CONSTANT(int, shift1 = s1);
    BOOST_STATIC_CONSTANT(int, shift2 = s2);

    /**
     * Constructors a @c xor_combine_engine by default constructing
     * both base generators.
     */
    xor_combine_engine() : _rng1(), _rng2() { }

    /** Constructs a @c xor_combine by copying two base generators. */
    xor_combine_engine(const base1_type & rng1, const base2_type & rng2)
      : _rng1(rng1), _rng2(rng2) {}

    /**
     * Constructs a @c xor_combine_engine, seeding both base generators
     * with @c v.
     *
     * @xmlwarning
     * The exact algorithm used by this function may change in the future.
     * @endxmlwarning
     */
	BOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR ( xor_combine_engine,
		result_type, v )
	{ seed(v); }

    /**
     * Constructs a @c xor_combine_engine, seeding both base generators
     * with values produced by @c seq.
     */
	BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR ( xor_combine_engine,
		SeedSeq, seq )
	{ seed(seq); }

    /**
     * Constructs a @c xor_combine_engine, seeding both base generators
     * with values from the iterator range [first, last) and changes
     * first to point to the element after the last one used.  If there
     * are not enough elements in the range to seed both generators,
     * throws @c std::invalid_argument.
     */
    template<class It> xor_combine_engine(It& first, It last)
      : _rng1(first, last), _rng2( /* advanced by other call */ first, last) { }

    /** Calls @c seed() for both base generators. */
    void seed() { _rng1.seed(); _rng2.seed(); }

    /**
     * @c seeds both base generators with a @c boost::random::seed_seq derived
     * from @c v.
     */
	BOOST_RANDOM_DETAIL_ARITHMETIC_SEED(xor_combine_engine, result_type, v)
	{
		const result_type s1 = detail::xor_combine_integer_hash( v + 1U); // +1 to avoid any modular dependencies...
		const result_type s2 = detail::xor_combine_integer_hash(s1 + 1U);
		_rng1.seed(detail::xor_combine_integer_hash(s1 + 1U));
		_rng2.seed(detail::xor_combine_integer_hash(s2 + 1U));
	}

public:
    /** @c seeds both base generators with values produced by @c gen. */
    template<typename RNG, typename SeedSeq = boost::random::seed_seq, std::size_t forwarded_size = 8>
	std::enable_if_t<
		(traits::is_generator<RNG>::value || traits::is_random_device<RNG>::value)
		&& traits::negation<std::is_same<RNG, xor_combine_engine>>::value
		&& traits::is_seed_seq<SeedSeq>::value
	>
    seed(RNG &gen)
    {
        std::uint_least32_t storage[2 * forwarded_size];
        std::generate(std::begin(storage), std::end(storage), [&gen]()->std::uint_least32_t { return gen(); });
        // Forward half the generated @c std::uint_least32_t array to each rng.
        forward_array_int<SeedSeq>(storage);
		//_rng1.seed(gen);
		//_rng2.seed(gen);
    }

    /** @c seeds both base generators with values produced by @c seq. */
	template<typename SeedSeq>
	std::enable_if_t<
		traits::is_seed_seq<SeedSeq>::value
	>
    seed(const SeedSeq &seq)
    {
        // Expand (double) the SeedSeq.
        std::vector<std::uint_least32_t> storage(2 * seq.size());
        seq.generate(std::begin(storage), std::end(storage));
        // Forward half the expanded SeedSeq to each rng.
        forward_array_int<SeedSeq>(storage);
    }

    /**
     * seeds both base generators with values from the iterator
     * range [first, last) and changes first to point to the element
     * after the last one used.  If there are not enough elements in
     * the range to seed both generators, throws @c std::invalid_argument.
     */
    template<class It> void seed(It& first, It last)
    {
        _rng1.seed(first, last);
        _rng2.seed(first, last);
    }

    /** Returns the first base generator. */
    const base1_type& base1() const { return _rng1; }

    /** Returns the second base generator. */
    const base2_type& base2() const { return _rng2; }

    /** Returns the next value of the generator. */
    result_type operator()()
    {
        return (_rng1() << s1) ^ (_rng2() << s2);
    }

    /** Fills a range with random values */
    template<class Iter>
    void generate(Iter first, Iter last)
    { detail::generate_from_int(*this, first, last); }

    /** Advances the state of the generator by @c z. */
    void discard(boost::uintmax_t z)
    {
        _rng1.discard(z);
        _rng2.discard(z);
    }

    /** Returns the smallest value that the generator can produce. */
    static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () { return (std::min)((URNG1::min)(), (URNG2::min)()); }
    /** Returns the largest value that the generator can produce. */
    static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () { return (std::max)((URNG1::min)(), (URNG2::max)()); }

    /**
     * Writes the textual representation of the generator to a @c std::ostream.
     */
    BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, xor_combine_engine, s)
    {
        os << s._rng1 << ' ' << s._rng2;
        return os;
    }

    /**
     * Reads the textual representation of the generator from a @c std::istream.
     */
    BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, xor_combine_engine, s)
    {
        is >> s._rng1 >> std::ws >> s._rng2;
        return is;
    }

    /** Returns true if the two generators will produce identical sequences. */
    BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(xor_combine_engine, x, y)
    { return x._rng1 == y._rng1 && x._rng2 == y._rng2; }

    /** Returns true if the two generators will produce different sequences. */
    BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(xor_combine_engine)

private:
	/// \cond show_private
    template<typename SeedSeq, typename Storage>
    void forward_array_int(const Storage &storage)
    {
        const auto middle = std::begin(storage) +
            std::distance(std::begin(storage), std::end(storage)) / 2;
        const SeedSeq seq1(std::begin(storage), middle);
        _rng1.seed(seq1);
        const SeedSeq seq2(middle, std::end(storage));
        _rng2.seed(seq2);
    }
	/// \endcond

    base1_type _rng1;
    base2_type _rng2;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//  A definition is required even for integral static constants
template<class URNG1, int s1, class URNG2, int s2>
const bool xor_combine_engine<URNG1, s1, URNG2, s2>::has_fixed_range;
template<class URNG1, int s1, class URNG2, int s2>
const int xor_combine_engine<URNG1, s1, URNG2, s2>::shift1;
template<class URNG1, int s1, class URNG2, int s2>
const int xor_combine_engine<URNG1, s1, URNG2, s2>::shift2;
#endif

/// \cond show_private

/** Provided for backwards compatibility. */
template<class URNG1, int s1, class URNG2, int s2,
    typename URNG1::result_type v = 0>
class xor_combine : public xor_combine_engine<URNG1, s1, URNG2, s2>
{
    typedef xor_combine_engine<URNG1, s1, URNG2, s2> base_type;
public:
    typedef typename base_type::result_type result_type;
    xor_combine() {}
    xor_combine(result_type val) : base_type(val) {}
    template<class It>
    xor_combine(It& first, It last) : base_type(first, last) {}
    xor_combine(const URNG1 & rng1, const URNG2 & rng2)
      : base_type(rng1, rng2) { }

    result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const { return (std::min)((this->base1().min)(), (this->base2().min)()); }
    result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return (std::max)((this->base1().min)(), (this->base2().max)()); }
};

/// \endcond

} // namespace random
} // namespace boost

#endif // BOOST_RANDOM_XOR_COMBINE_HPP
