
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <iostream>
#include <sstream>
#include <random>
#include <cstdint>

#include <functional>
#include "taus88.hpp"
// #include <boost/random/taus88.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/seed_seq.hpp>
#include <boost/random/random_device.hpp>
#include "seed_seq_fe.hpp"
#include "xoroshiro.hpp"
#include "complementary_multiply_with_carry.hpp"

#include <pcg/pcg_random.hpp>


#define PCG_USE_ZEROCHECK_ROTATE_IDIOM 1

#include "autotimer.hpp"

std::uint64_t integer_hash ( std::uint64_t x )
{
    x = ( ( x >> 32 ) ^ x ) * std::uint64_t { 0xDE17C195AA959A81 };
    x = ( ( x >> 32 ) ^ x ) * std::uint64_t { 0xDE17C195AA959A81 };
    x = ( ( x >> 32 ) ^ x ) * std::uint64_t { 0xDE17C195AA959A81 };
    return ( x >> 32 ) ^ x;
}


#define VOLATILE volatile

template<typename Generator>
struct GeneratorCache : private Generator {

    using result_type = typename Generator::result_type;
    using size_type = std::size_t;

    GeneratorCache ( ) : Generator ( ) { }
    GeneratorCache ( result_type v_ ) : Generator ( v_ ) { }
    template<typename SeedSeq>
    GeneratorCache ( const SeedSeq & seq_ ) : Generator ( seq_ ) { }
    template<typename It>
    GeneratorCache ( It & first_, It last_ ) : Generator ( first_, last_ ) { }

    result_type operator ( ) ( ) noexcept {

        if ( m_index == data_size ( ) ) {

            Generator::generate ( std::begin ( m_data ), std::end ( m_data ) );
            // std::generate ( std::begin ( m_data ), std::end ( m_data ), [ this ] ( ) { return Generator::operator ( ) ( ); } );
            m_index = 1;

            return m_data [ 0 ];
        }

        return m_data [ m_index++ ];
    }

    static constexpr result_type min ( ) noexcept { return Generator::min ( ); }
    static constexpr result_type max ( ) noexcept { return Generator::max ( ); }
    static constexpr std::size_t data_size ( ) noexcept { return 64 / sizeof ( result_type ); }

    result_type m_data [ data_size ( ) ];
    size_type m_index = data_size ( );
};


int main ( ) {

    boost::random::seed_seq_fe256 seq_fe { INT32_MAX, 3466, 456454, 23423 };

    std::uint64_t cnt = 1;

    GeneratorCache<boost::random::xoroshiro128plus> gen1 ( seq_fe );
    GeneratorCache<boost::random::xoshiro256starstar> gen2 ( seq_fe );
    GeneratorCache<boost::random::xoshiro256plus> gen3 ( seq_fe );
    GeneratorCache<boost::random::xoroshiro128plusshixo> gen4 ( seq_fe );

    pcg64 gen5 ( seq_fe );

    while ( cnt-- ) {

        std::int64_t cnt1 = 1'000'000'000, cnt2 = cnt1, cnt3 = cnt1, cnt4 = cnt1, cnt5 = cnt1;

        VOLATILE std::uint64_t acc1 = 0;

        {
            degski::AutoTimer t ( degski::milliseconds );

            while ( cnt1-- )
                acc1 += std::uniform_int_distribution<std::uint64_t> ( ) ( gen1 );
        }

        VOLATILE std::uint64_t acc2 = 0;

        {
            degski::AutoTimer t ( degski::milliseconds );

            while ( cnt2-- )
                acc2 += std::uniform_int_distribution<std::uint64_t> ( ) ( gen2 );
        }

        VOLATILE std::uint64_t acc3 = 0;

        {
            degski::AutoTimer t ( degski::milliseconds );

            while ( cnt3-- )
                acc3 += std::uniform_int_distribution<std::uint64_t> ( ) ( gen3 );
        }

        VOLATILE std::uint64_t acc4 = 0;

        {
            degski::AutoTimer t ( degski::milliseconds );

            while ( cnt4-- )
                acc4 += std::uniform_int_distribution<std::uint64_t> ( ) ( gen4 );
        }

        VOLATILE std::uint64_t acc5 = 0;

        {
            degski::AutoTimer t ( degski::milliseconds );

            while ( cnt5-- )
                acc5 += std::uniform_int_distribution<std::uint64_t> ( ) ( gen5 );
        }

        std::cout << std::boolalpha << !( ( acc2 == acc1 ) && ( acc3 == acc2 ) && ( acc4 == acc3 ) && ( acc5 == acc4 ) ) << '\n';
    }

    exit ( 0 );

#if 0

    std::cout << std::boolalpha << ( gen1 == gen2 ) << "\n";

    std::stringstream s;

    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );
    gen1 ( );

    s << gen1;

    std::cout << gen1 << '\n';
    std::cout << gen2 << '\n';

    s >> gen2;

    std::cout << gen2 << '\n';

    std::cout << std::boolalpha << ( gen1 == gen2 ) << "\n";

    std::seed_seq seq { INT32_MAX,0,0,0,0,0,0,346,456454,23423 };

    boost::random::xoroshiro128plus gen4 ( seq );

    std::vector<int> inp { 0, 5, 0, 0, 1, 3,4,43,3,4,4,4,45,43,463,63,63,356,36,25,141,144,0, 5, 0, 0, 1, 3,4,43,3,4,4,4,45,43,463,63,63,356,36,25,141,144 };

    auto start = std::begin ( inp );

    boost::random::xoroshiro128plus gen5 ( start, std::end(inp) );

    std::uint64_t seed = 0;

    std::vector<seq::result_type> v;
    v.reserve ( seq.size ( ) );
    seq.param ( std::back_inserter ( v ) );

    int shift = 32;

    for ( auto i : v ) {
        // std::cout << seed << '\n';
        seed ^= boost::random::detail::xoroshiro_integer_hash ( ( static_cast<std::uint64_t>( i ) << ( shift ^= int { 32 } ) ) );
    }

    seed = 0;

    for ( std::uint32_t i = 1; i < 1025; ++i ) {
        seed ^= boost::random::detail::xoroshiro_integer_hash ( ( static_cast<std::uint64_t>( i ) << ( shift ^= int { 32 } ) ) ^ seed );
        std::cout << seed << '\n';
    }

#endif

}


std::uint32_t Q1 [ 256 ], c1 = 362436;

std::uint32_t MWC256 ( ) {

    std::uint64_t t, a = 1540315826LL;
    static std::size_t i = 255;
    unsigned long x;
    i = ( i + 1 ) & 255;
    t = a * Q1 [ i ] + c1;
    c1 = ( t >> 32 );
    x = t + c1;
    if (x < c1) { x++; c1++; }
    return ( Q1 [ i ] = x );
}

std::uint32_t Q2 [ 4096 ], c2 = 362436;

std::uint32_t CMWC4096 ( ) {

    std::uint64_t t, a = 18782LL;
    static std::size_t i = 4095;
    std::uint32_t x, r = 0xfffffffe;
    i = ( i + 1 ) & 4095;
    t = a*Q2 [ i ] + c2;
    c2 = ( t >> 32 );
    x = t + c2;
    if ( x < c2 ) { x++; c2++; }
    return ( Q2 [ i ] = r - x );
}

int main436463466 ( ) {

    std::seed_seq seq { INT32_MAX, 3466, 456454, 23423 };

    boost::random::taus88 gen ( seq );

    std::cout << gen << "\n\n";

    return 0;
}



int main34342 ( ) {

    typedef boost::random::taus88 Engine;
    // typedef std::ranlux48 Engine;

    boost::random::random_device rng1;
    Engine eng1 ( rng1 );

    std::cout << eng1 << '\n';

    //std::random_device rng2;
    //Engine eng2 ( rng2 );

    //std::cout << eng2 << '\n';

    boost::random::mt19937 rng3;
    Engine eng3 ( rng3 );

    std::cout << eng3 << '\n';

    //std::mt19937 rng4;
    //Engine eng4 ( rng4 );

    //std::cout << eng4 << '\n';

    boost::random::seed_seq seq5 { INT32_MAX, 3466, 456454, 23423 };
    Engine eng5 ( seq5 );

    std::cout << eng5 << '\n';

    boost::random::seed_seq_fe256 seq6 { INT32_MAX, 3466, 456454, 23423 };
    Engine eng6 ( seq6 );

    std::cout << eng6 << '\n';

    std::seed_seq seq7 { INT32_MAX, 3466, 456454, 23423 };
    Engine eng7 ( seq7 );

    std::cout << eng7 << '\n';

    Engine eng8 ( eng7 ); // Copy constructor...

    std::cout << eng8 << '\n';

    Engine eng9 ( 4563546 );

    std::cout << eng9 << '\n';

#if 0

    exit ( 0 );

    boost::random::seed_seq seq { INT32_MAX, 3466, 456454, 23423 };

    boost::random::cmwc4 gen ( 6568 );

    std::int64_t cnt = 1'000'000'000;

    std::uint64_t acc = 0;

    {
        degski::AutoTimer t ( degski::milliseconds );

        while ( cnt-- )
        acc += std::uniform_int_distribution<std::uint32_t> ( ) ( gen );
    }

    std::cout << gen << '\n';

#endif
    return 0;
}


// https://genix.wordpress.com/2014/07/29/detecting-the-presence-of-a-member-function/

namespace boost {
namespace random {

template<typename G, std::enable_if_t<boost::random::traits::is_generator<G>::value>* = nullptr>
using generator = G;
template<typename D, std::enable_if_t<boost::random::traits::is_random_device<D>::value>* = nullptr>
using device = D;
template<typename S, std::enable_if_t<boost::random::traits::is_seed_seq<S>::value>* = nullptr>
using sequence = S;

} // namespace random
} // namespace boost


auto main4543543 ( ) -> int {

    std::cout << std::boolalpha
        << '\n'
        << boost::random::traits::is_generator<int>::value
        << '\n'
        << boost::random::traits::is_generator<boost::random::taus88>::value
        << '\n'
        << boost::random::traits::is_random_device<std::random_device>::value
        << '\n'
        << boost::random::traits::is_random_device<boost::random::random_device>::value
        << '\n'
        << boost::random::traits::is_seed_seq<std::seed_seq>::value
        << '\n';

    return 0;
}


#if 0
template <typename...>
using void_t = void;

// We have our base case:

template<class T, class Index, typename = void>
struct has_subscript_operator : std::false_type { };

// And our expression SFINAE valid case:

template<class T, class Index>
struct has_subscript_operator<T, Index, void_t<
    decltype(std::declval<T>( )[std::declval<Index>( )])
    >> : std::true_type { };


namespace details {
    template<class...>struct voider { using type = void; };
    template<class...Ts>using void_t = typename voider<Ts...>::type;

    template<template<class...>class Z, class, class...Ts>
    struct can_apply :
        std::false_type
    {};
    template<template<class...>class Z, class...Ts>
    struct can_apply<Z, void_t<Z<Ts...>>, Ts...> :
        std::true_type
    {};
}
template<template<class...>class Z, class...Ts>
using can_apply = details::can_apply<Z, void, Ts...>;

// You can then simply write properties :

template <class T, class Index>
using subscript_t = decltype(std::declval<T>()[std::declval<Index> ( )]);

template <class T, class Index>
using has_subscript = can_apply<subscript_t, T, Index>;

template <class T>
using call_t = decltype(std::declval<T>()());

template <class T>
using has_call_operator = can_apply<call_t, T>;

template <class T>
using has_seed_t = decltype(std::declval<T>().seed());

template <class T>
using has_seed = can_apply<has_seed_t, T>;

template <class T>
using has_max_t = decltype(std::declval<T> ( ).max());

template <class T>
using has_max = can_apply<has_max_t, T>;


template<typename T, typename R, typename RT = typename T::result_type>
struct has_method_max : boost::random::traits::has_method_max<T, R> {};
#endif

struct Foo {

    typedef int result_type;

    result_type i;

#if 1
    static result_type max ( int ) { return INT_MAX; }
#else
    result_type max ( int ) { return INT_MAX; }
#endif

    Foo ( int i_ ) : i ( i_ ) {}

    result_type get ( ) { return i; }
    result_type get ( int ) { return i; }
};

#define DEFINE_HAS_STATIC(func_name)                                                                            \
template <typename U, typename return_type, typename... args>                                                   \
class has_trait_static_##func_name                                                                              \
{                                                                                                               \
private:                                                                                                        \
    template<typename T, T> struct helper;                                                                      \
    template<typename check_type, typename... check_args>                                                       \
    static decltype(std::declval<helper<return_type(*)(check_args...), &check_type::func_name>>()) check(int);  \
    template<typename check_type, typename... check_args>                                                       \
    static boost::random::traits::oops check(...);                                                              \
public:                                                                                                         \
    static constexpr bool value = !std::is_same_v<decltype(check<U, args...>(0)), boost::random::traits::oops>; \
};

DEFINE_HAS_STATIC ( max )

#include <boost/tti/has_type.hpp>
#include <boost/tti/has_static_member_function.hpp>

BOOST_TTI_HAS_TYPE(result_type)


template<typename... Ts> struct make_void { typedef void type; };
template<typename... Ts> using void_t = typename make_void<Ts...>::type;

struct nonesuch {
    nonesuch ( ) = delete;
    ~nonesuch ( ) = delete;
    nonesuch ( nonesuch const& ) = delete;
    void operator=( nonesuch const& ) = delete;
};

template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template<class Default, template<class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

template<template<class...> class Op, class... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;
template<template<class...> class Op, class... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;
template <class Default, template<class...> class Op, class... Args>
using detected_or = detector<Default, void, Op, Args...>;

template< template<class...> class Op, class... Args >
constexpr bool is_detected_v = is_detected<Op, Args...>::value;
template< class Default, template<class...> class Op, class... Args >
using detected_or_t = typename detected_or<Default, Op, Args...>::type;
template <class Expected, template<class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;
template <class Expected, template<class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;
template <class To, template<class...> class Op, class... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;
template <class To, template<class...> class Op, class... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;


template <typename T, template <typename> class Expression, typename AlwaysVoid = void_t<>>
struct compiles : std::false_type {};
template <typename T, template <typename> class Expression>
struct compiles<T, Expression, void_t<Expression<T>>> : std::true_type {};


template <typename ResultType, typename CheckType, template <typename> class... Values>
using requires = std::enable_if_t<std::conjunction<Values<CheckType>...>::value, ResultType>;

auto main3253523 ( ) -> int {

    std::cout << std::boolalpha
        << true
        << '\n'
        << true
        << '\n';

    return 0;
}



class A {};

template <typename T, typename = void>
struct is_iterable : std::false_type {};
template <typename T>
struct is_iterable<T, std::void_t<decltype( std::declval<T> ( ).begin ( ) ),
    decltype( std::declval<T> ( ).end ( ) )>>
    : std::true_type {};

int main34242 ( )
{
    std::cout << std::boolalpha;
    std::cout << is_iterable<std::vector<double>>::value << '\n';
    std::cout << is_iterable<double>::value << '\n';
    std::cout << is_iterable<A>::value << '\n';

    return 0;
}


//decltype(return_type(refval<check_args>()...))
//template<typename check_type, typename... check_args>
//static std::uint16_t check ( ... );


#include <iostream>
#include <string>

using namespace std;

template<typename ...Args> auto sum ( Args ...args )
{
    return ( args + ... + 0 );
}

template<typename ...Args> auto sum2 ( Args ...args )
{
    return ( args * ... );
}

template <auto ... vs> struct HeterogenousValueList {};

using MyList = HeterogenousValueList<'a', 100, 'b'>;

int main325245 ( )
{
    cout << sum ( 1, 2, 3, 4, 5, 6, 7 ) << "\n";
    cout << sum2 ( 1, 2, 3, 4, 5, 6, 7 ) << "\n";

    // std::pair<int, double> p ( 10, 0.0 );
    // same as
    std::pair p ( 10, 0.0 ); // deduced automatically!

    return 0;
}


template< typename s, typename t, typename u, typename = void >
struct is_product_serializable : std::false_type {};

template< typename s, typename t, typename u >
struct is_product_serializable< s, t, u, typename std::conditional< true, void,
    decltype( std::declval< s > ( ) << std::declval< t > ( ) * std::declval< u > ( ) )
>::type >
    : std::true_type {};
