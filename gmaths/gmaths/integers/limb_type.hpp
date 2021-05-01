#ifndef GMATHS_INTEGERS_LIMB_TYPE_HPP_INCLUDED
#define GMATHS_INTEGERS_LIMB_TYPE_HPP_INCLUDED

#include <bit>
#include <cstdint>
#include <type_traits>

#ifndef GMATHS_NO_INTRINSICS
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif
#endif

namespace gmaths
{
namespace integers
{

using limb_type = std::uint64_t;
using signed_limb_type = std::int64_t;
using limb_half_type = std::uint32_t;
using signed_limb_half_type = std::int32_t;
constexpr int limb_bits = 64;

/**
 * @brief Counts the argument's leading zeroes.
 * @param arg value whose leading zeroes shall be counted.
 * @return the number of 0 bits above the highest order 1 bit in the argument or
 * gmaths::integers::limb_bits if the argument is zero.
 */
constexpr int limb_lzcount(limb_type arg) noexcept
{
    return std::countl_zero(arg);
}

/**
 * @brief Counts the argument's trailing zeroes.
 * @param arg the value whose trailing zeroes shall be counted.
 * @return the number of 0 bits below the lowest order 1 bit in the argument or
 * gmaths::integers::limb_bits if the argument is zero.
 */
constexpr int limb_tzcount(limb_type arg) noexcept
{
    return std::countr_zero(arg);
}

/**
 * @brief Counts the number of 1 bits in the argument.
 * @param arg the value whose 1 bits shall be counted.
 * @return the number of 1 bits in the argument.
 */
constexpr int limb_popcount(limb_type arg) noexcept
{
    return std::popcount(arg);
}

/**@{*/
/**
 * @brief Increments the argument by one or by the provided carry bit, stores
 * the result in the destination address and returns the carry bit of the
 * increment operation.
 *
 * The behavior is undefined if @p result is not a valid pointer to an object of
 * type gmaths::integer::limb_type.
 *
 * These functions can be used as iteration steps in incrementing a large
 * integer consisting of multiple limbs.
 *
 * @param carry the carry bit (defaults to 1).
 * @param arg the value to be incremented.
 * @param result address where the resulting value will be stored.
 * @return true iff the resulting value is zero (i. e. the increment operation
 * wraps around).
 */
constexpr bool limb_inc(limb_type arg, limb_type* result) noexcept
{
    return (*result = arg + 1) < arg;
}

constexpr bool limb_inc(bool carry, limb_type arg, limb_type* result) noexcept
{
    return (*result = arg + carry) < arg;
}
/**@}*/

/**@{*/
/**
 * @brief Decrements the argument by one or by the provided borrow bit, stores
 * the result in the destination address and returns the borrow bit of the
 * decrement operation.
 *
 * The behavior is undefined if @p result is not a valid pointer to an object of
 * type gmaths::integer::limb_type.
 *
 * These functions can be used as iteration steps in decrementing a large
 * integer consisting of multiple limbs.
 *
 * @param borrow the borrow bit (defaults to 1).
 * @param arg the value to be decremented.
 * @param result address where the resulting value will be stored.
 * @return true iff @p arg is zero (i. e. the decrement operation wraps around).
 */
constexpr bool limb_dec(limb_type arg, limb_type* result) noexcept
{
    return (*result = arg - 1) > arg;
}

constexpr bool limb_dec(bool borrow, limb_type arg, limb_type* result) noexcept
{
    return (*result = arg - borrow) > arg;
}
/**@}*/

/**@{*/
/**
 * @brief Negates the argument or flips its bits iff a carry bit with value 0 is
 * provided, stores the result in the destination address and returns true iff
 * the argument was negated and it was zero.
 *
 * The behavior is undefined if @p result is not a valid pointer to an object of
 * type gmaths::integer::limb_type.
 *
 * The rationale behind these functions is that negating and integer in two's
 * complement is the same as flipping its bits and then adding one.
 *
 *     0 - arg == ~arg + 1
 *
 * So the functions allow for simple negation of integers consisting of
 * multiple limbs by propagating the carry bit of the "addition" correctly.
 *
 * @param carry the carry bit (defaults to 1).
 * @param arg the value to be negated.
 * @param result address where the result will be stored.
 * @return true iff @p arg is zero.
 */
constexpr bool limb_neg(limb_type arg, limb_type* result) noexcept
{
    return !(*result = (0 - arg));
}

constexpr bool limb_neg(bool carry, limb_type arg, limb_type* result) noexcept
{
    return limb_inc(carry, ~arg, result);
}
/**@}*/

/**@{*/
/**
 * @brief Performs addition of two limbs and one optional carry bit, stores the
 * result in the destination address and returns the carry bit of the addition.
 *
 * The behavior is undefined if @p result is not a valid pointer to an object of
 * type gmaths::integer::limb_type.
 *
 * @param carry the carry bit (defaults to 0).
 * @param l the first summand.
 * @param r the second summand.
 * @param result address where the sum will be stored.
 * @return the carry bit of the addition.
*/
constexpr bool limb_add(limb_type l, limb_type r, limb_type* result) noexcept
{
    return (*result = l + r) < l;
}

constexpr bool limb_add(bool carry, limb_type l, limb_type r, limb_type* result) noexcept
{
#ifndef GMATHS_NO_INTRINSICS
#if (defined(_MSC_VER) && defined(_M_X64)) || (defined(__GNUC__) && defined(__x86_64__))
    if (!std::is_constant_evaluated()) {
        unsigned long long tmp = 0;
        bool ret = static_cast<bool>(_addcarry_u64(carry, l, r, &tmp));
        *result = tmp;
        return ret;
    }
#elif (defined(_MSC_VER) && defined(_M_IX86)) || (defined(__GNUC__) && defined(__i386__))
    if (!std::is_constant_evaluated()) {
        limb_half_type ld[]{
            static_cast<limb_half_type>(l),
            static_cast<limb_half_type>(l >> 32)
        };
        limb_half_type rd[]{
            static_cast<limb_half_type>(r),
            static_cast<limb_half_type>(r >> 32)
        };
        carry = static_cast<bool>(_addcarry_u32(carry, ld[0], rd[0], &ld[0]));
        carry = static_cast<bool>(_addcarry_u32(carry, ld[1], rd[1], &ld[1]));
        *result = static_cast<limb_type>(ld[1]) << 32 | ld[0];
        return carry;
    }
#endif
#endif
    limb_type tmp = 0;
    bool b1 = limb_add(l, r, &tmp);
    bool b2 = limb_inc(carry, tmp, result);
    return b1 || b2;
}
/**@}*/

/**@{*/
/**
 * @brief Performs subtraction of two limbs and one optional borrow bit, stores
 * the result in the destination address and returns the carry bit of the
 * addition.
 *
 * The behavior is undefined if @p result is not a valid pointer to an object of
 * type gmaths::integer::limb_type.
 *
 * @param borrow the borrow bit (defaults to 0).
 * @param l the subtrahend.
 * @param r the subtractor.
 * @param result address where the sum will be stored.
 * @return the borrow bit of the subtraction.
*/
constexpr bool limb_sub(limb_type l, limb_type r, limb_type* result) noexcept
{
    return (*result = l - r) > l;
}

constexpr bool limb_sub(bool borrow, limb_type l, limb_type r, limb_type* result) noexcept
{
#ifndef GMATHS_NO_INTRINSICS
#if (defined(_MSC_VER) && defined(_M_X64)) || (defined(__GNUC__) && defined(__x86_64__))
    if (!std::is_constant_evaluated()) {
        unsigned long long tmp = 0;
        bool ret = static_cast<bool>(_subborrow_u64(borrow, l, r, &tmp));
        *result = tmp;
        return ret;
    }
#elif (defined(_MSC_VER) && defined(_M_IX86)) || (defined(__GNUC__) && defined(__i386__))
    if (!std::is_constant_evaluated()) {
        limb_half_type ld[]{
            static_cast<limb_half_type>(l),
            static_cast<limb_half_type>(l >> 32)
        };
        limb_half_type rd[]{
            static_cast<limb_half_type>(r),
            static_cast<limb_half_type>(r >> 32)
        };
        borrow = static_cast<bool>(_subborrow_u32(borrow, ld[0], rd[0], &ld[0]));
        borrow = static_cast<bool>(_subborrow_u32(borrow, ld[1], rd[1], &ld[1]));
        *result = static_cast<limb_type>(ld[1]) << 32 | ld[0];
        return borrow;
    }
#endif
#endif
    limb_type tmp = 0;
    bool b1 = limb_sub(l, r, &tmp);
    bool b2 = limb_dec(borrow, tmp, result);
    return b1 || b2;
}
/**@}*/

/**@{*/
/**
 * @brief Multiplies two limbs and adds up to two more limbs, stores the high
 * half of the result in the address @p high_value and returns the low part of
 * the result.
 *
 * The behavior is undefined if @p high_result is not a valid pointer to an
 * object of type gmaths::integer::limb_type.
 *
 * @param l first multiplicand.
 * @param r second multiplicand.
 * @param c first additional summand (defaults to 0).
 * @param d second additional summand (defaults to 0).
 * @param high_result address where the high part of the result of the
 * multiplication (plus addition) will be stored.
 * @return low part of the result of the multiplication (plus addition).
*/
constexpr limb_type limb_mul(limb_type l, limb_type r, limb_type* high_result) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__SIZEOF_INT128__)
    unsigned __int128 tmp = static_cast<unsigned __int128>(l) * r;
    *high_result = static_cast<limb_type>(tmp >> 64);
    return static_cast<limb_type>(tmp);
#else
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_X64)
    if (!std::is_constant_evaluated()) {
        return _umul128(l, r, high_result);
    }
#endif
    limb_half_type ld[]{
        static_cast<limb_half_type>(l),
        static_cast<limb_half_type>(l >> 32)
    };
    limb_half_type rd[]{
        static_cast<limb_half_type>(r),
        static_cast<limb_half_type>(r >> 32)
    };
    limb_type lo = static_cast<limb_type>(ld[0]) * rd[0];
    limb_type m1 = static_cast<limb_type>(ld[0]) * rd[1] + (lo >> 32);
    limb_type m2 = static_cast<limb_type>(ld[1]) * rd[0] + static_cast<limb_half_type>(m1);
    limb_type hi = static_cast<limb_type>(ld[1]) * rd[1] + (m2 >> 32) + (m1 >> 32);
    lo = static_cast<limb_half_type>(lo) | m2 << 32;
    *high_result = hi;
    return lo;
#endif
}

constexpr limb_type limb_mul(limb_type l, limb_type r, limb_type c, limb_type* high_result) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__SIZEOF_INT128__)
    unsigned __int128 tmp = static_cast<unsigned __int128>(l) * r + c;
    *high_result = static_cast<limb_type>(tmp >> 64);
    return static_cast<limb_type>(tmp);
#else
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_X64)
    if (!std::is_constant_evaluated()) {
        l = _umul128(l, r, &r);
        r += limb_add(l, c, &l);
        *high_result = r;
        return l;
    }
#endif
    limb_half_type ld[]{
        static_cast<limb_half_type>(l),
        static_cast<limb_half_type>(l >> 32)
    };
    limb_half_type rd[]{
        static_cast<limb_half_type>(r),
        static_cast<limb_half_type>(r >> 32)
    };
    limb_half_type cd[]{
        static_cast<limb_half_type>(c),
        static_cast<limb_half_type>(c >> 32)
    };
    limb_type lo = static_cast<limb_type>(ld[0]) * rd[0] + cd[0];
    limb_type m1 = static_cast<limb_type>(ld[0]) * rd[1] + cd[1] + (lo >> 32);
    limb_type m2 = static_cast<limb_type>(ld[1]) * rd[0] + static_cast<limb_half_type>(m1);
    limb_type hi = static_cast<limb_type>(ld[1]) * rd[1] + (m2 >> 32) + (m1 >> 32);
    lo = static_cast<limb_half_type>(lo) | m2 << 32;
    *high_result = hi;
    return lo;
#endif
}

/**  */
constexpr limb_type limb_mul(limb_type l, limb_type r, limb_type c, limb_type d, limb_type* high_result) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__SIZEOF_INT128__)
    unsigned __int128 tmp = static_cast<unsigned __int128>(l) * r + c + d;
    *high_result = static_cast<limb_type>(tmp >> 64);
    return static_cast<limb_type>(tmp);
#else
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_X64)
    if (!std::is_constant_evaluated()) {
        l = _umul128(l, r, &r);
        r += limb_add(l, c, &l);
        r += limb_add(l, d, &l);
        *high_result = r;
        return l;
    }
#endif
    limb_half_type ld[]{
        static_cast<limb_half_type>(l),
        static_cast<limb_half_type>(l >> 32)
    };
    limb_half_type rd[]{
        static_cast<limb_half_type>(r),
        static_cast<limb_half_type>(r >> 32)
    };
    limb_half_type cd[]{
        static_cast<limb_half_type>(c),
        static_cast<limb_half_type>(c >> 32)
    };
    limb_half_type dd[]{
        static_cast<limb_half_type>(d),
        static_cast<limb_half_type>(d >> 32)
    };
    limb_type lo = static_cast<limb_type>(ld[0]) * rd[0] + cd[0] + dd[0];
    limb_type m1 = static_cast<limb_type>(ld[0]) * rd[1] + cd[1] + dd[1];
    limb_type m2 = static_cast<limb_type>(ld[1]) * rd[0] + (lo >> 32) + static_cast<limb_half_type>(m1);
    limb_type hi = static_cast<limb_type>(ld[1]) * rd[1] + (m2 >> 32) + (m1 >> 32);
    lo = static_cast<limb_half_type>(lo) | m2 << 32;
    *high_result = hi;
    return lo;
#endif
}
/**@}*/

namespace _detail
{

constexpr void limb_div_64_by_32(limb_half_type* l, limb_half_type* r, limb_half_type* q) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_IX86)
    /*
     * Using _udiv64 neatly translates into one div instruction whereas
     * the standard C++ version may require multiple instructions for
     * the same effect
     */
    if (!std::is_constant_evaluated()) {
        limb_type tmp = static_cast<limb_type>(l[1]) << 32 | l[0];
        q[0] = _udiv64(tmp, r[0], l);
        return;
    }
#elif !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__i386__) && !defined(__x86_64__)
    /*
     * GCC & co. unfortunately don't have such a _udiv64 intrinsic. We're
     * resorting to inline assembly instead.
     */
    if (!std::is_constant_evaluated()) {
        asm("div %[d]"
            : "=a"(q[0]), "=d"(l[0])
            : "a"(l[0]), "d"(l[1]), [d]"r"(r[0])
            : "cc");
        return;
    }
#endif
    limb_type tmp = static_cast<limb_type>(l[1]) << 32 | l[0];
    q[0] = static_cast<limb_half_type>(tmp / r[0]);
    l[0] = static_cast<limb_half_type>(tmp % r[0]);
}

constexpr void limb_div_96_by_64(limb_half_type* l, limb_half_type* r, limb_half_type* q) noexcept
{
    if (l[2] >= r[1]) {
        q[0] = static_cast<limb_half_type>(-1);
        limb_type tmp = static_cast<limb_half_type>(-1) * (static_cast<limb_type>(r[1]) << 32 | r[0]);
        tmp = (static_cast<limb_type>(l[1]) << 32 | l[0]) - tmp;
        l[1] = static_cast<limb_half_type>(tmp >> 32);
        l[0] = static_cast<limb_half_type>(tmp);
        return;
    }

    limb_half_type candidate = static_cast<limb_half_type>(0);
    limb_div_64_by_32(l + 1, r + 1, &candidate);

    limb_type ll = static_cast<limb_type>(l[1]) << 32 | l[0];
    limb_half_type lh = l[2];

    limb_type cl = 0;
    limb_half_type ch = 0;
    {
        cl = static_cast<limb_type>(candidate) * r[0];
        limb_type tmp = (cl >> 32) + static_cast<limb_type>(candidate) * r[1];
        cl = static_cast<limb_half_type>(cl) | (tmp << 32);
        ch = static_cast<limb_half_type>(tmp >> 32);
    }

    limb_type rr = static_cast<limb_type>(r[1]) << 32 | r[0];

    for (int i = 0; i < 2; ++i) {
        if (ch < lh || (ch == lh && cl <= ll))
            break;

        --candidate;
        ch -= limb_sub(cl, rr, &cl);
    }

    ll -= cl;
    l[0] = static_cast<limb_half_type>(ll);
    l[1] = static_cast<limb_half_type>(ll >> 32);
    q[0] = candidate;
}

}

/**
 * @brief Divides a two limbs wide operand by a one limb wide operand, stores
 * the remainder at address @p remainder and returns the quotient.
 *
 * The behavior is undefined, if @p l_high is not less than @p r or if
 * @p remainder is not a valid pointer to an object of type
 * gmaths::integers::limb_type.
 *
 * @param l_high high part of the dividend.
 * @param l_low low part of the dividend.
 * @param r divisor.
 * @param remainder address where the remainder of the division will be stored.
 * @return the quotient of the divison.
*/
constexpr limb_type limb_div(limb_type l_high, limb_type l_low, limb_type r, limb_type* remainder) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_X64)
    if (!std::is_constant_evaluated()) {
        return _udiv128(l_high, l_low, r, remainder);
    }
#elif !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__x86_64__)
    if (!std::is_constant_evaluated()) {
        limb_type quo = 0, rem = 0;
        asm("div %[d]"
            : "=a"(quo), "=d"(rem)
            : "a"(l_low), "d"(l_high), [d]"r"(r)
            : "cc");
        *remainder = rem;
        return quo;
    }
#endif

#if !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__SIZEOF_INT128__)
    unsigned __int128 tmp = static_cast<unsigned __int128>(l_high) << 64 | l_low;
    *remainder = static_cast<limb_type>(tmp % r);
    return static_cast<limb_type>(tmp / r);
#else
    limb_half_type ld[]{ 
        static_cast<limb_half_type>(l_low),
        static_cast<limb_half_type>(l_low >> 32),
        static_cast<limb_half_type>(l_high),
        static_cast<limb_half_type>(l_high >> 32)
    };
    limb_half_type rd[]{
        static_cast<limb_half_type>(r),
        static_cast<limb_half_type>(r >> 32)
    };
    limb_half_type qd[2]{ };

    if (!rd[1]) {
        _detail::limb_div_64_by_32(ld + 1, rd, qd + 1);
        _detail::limb_div_64_by_32(ld, rd, qd);
        *remainder = ld[0];
    } else {
        _detail::limb_div_96_by_64(ld + 1, rd, qd + 1);
        _detail::limb_div_96_by_64(ld, rd, qd);
        *remainder = static_cast<limb_type>(ld[1]) << 32 | ld[0];
    }
    return static_cast<limb_type>(qd[1]) << 32 | qd[0];
#endif
}

}
}

#endif