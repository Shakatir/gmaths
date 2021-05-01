#ifndef GMATHS_INTEGERS_LIMB_TYPE_HPP_INCLUDED
#define GMATHS_INTEGERS_LIMB_TYPE_HPP_INCLUDED

/**
 * @file gmaths/integers/limb_type.hpp
 * @brief Introduces a type gmaths::integers::limb_type together with a number
 * of functions that are useful when dealing with arbitrary precision integers.
 * 
 * The types introduced here are native integer types that can be used just the
 * same as `unsigned`, `long` etc. The functions handle things like addition
 * with carry, or getting the high result of a multiplication, which is
 * something that most computers are capable of quite easily, but that do not
 * have a corresponding method to be expressed in standard C++.
 * 
 * All functions can be used in constant expressions, but if possible will make
 * use of intrinsics that cannot be constant evaluated when possible. The
 * intrinsics and platform specific code may not be standard compliant, but by
 * defining `GMATHS_NO_INTRINSICS` all platform specific code is removed and the
 * remaining code is fully standard compliant (i. e. should run anywhere). The
 * only exception is that the file may rely on the presence of certain types
 * like std::uint64_t for its implementation.
 */

#include <bit>
#include <cassert>
#include <cstdint>
#include <type_traits>

/**
 * @def GMATHS_NO_INTRINSICS
 * @brief Define this macro to disable all platform specific optimizations and
 * make the implementation conforming to the C++ standard.
 */

#ifndef GMATHS_NO_INTRINSICS
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif
#endif

namespace gmaths::integers
{

/**
 * @brief Integer type that is used as the limb of big integer types
 * in the gmaths library. The type is guaranteed to be a native unsigned
 * integer type with an even number of bits.
 * 
 * While the width is currently fixed at 64 bits, this may be subject to
 * changes.
 */
using limb_type = std::uint64_t;

/**
 * @brief Signed counterpart to ::limb_type. It is guaranteed to be a native
 * signed integer type with the same number of bits as ::limb_type.
 */
using signed_limb_type = std::int64_t;

/**
 * @brief The number of bits of ::limb_type and ::signed_limb_type.
 */
constexpr int limb_bits = 64;

/**
 * @brief Integer type that is half as wide as ::limb_type. It is
 * guaranteed to be a native unsigned integer type with exactly half as many
 * bits as ::limb_type.
 */
using limb_half_type = std::uint32_t;

/**
 * @brief Signed counterpart to ::limb_half_type. It is
 * guaranteed to be a native signed integer type with exactly half as many
 * bits as ::limb_type.
 */
using signed_limb_half_type = std::int32_t;

/**
 * @brief The number of bits of ::limb_half_type and ::signed_limb_half_type.
 * Guaranteed to be exactly half of ::limb_bits.
 */
constexpr int limb_half_bits = 32;

/**
 * @brief Counts the argument's leading zeroes.
 * @param arg value whose leading zeroes shall be counted.
 * @return the number of 0 bits above the highest order 1 bit in the argument or
 * ::limb_bits if the argument is zero.
 */
constexpr int limb_lzcount(limb_type arg) noexcept
{
    // This used to make a lot more trouble before the <bit> header was introduced :)
    // TODO : see if manual implementation of lzcount is desirable
    return std::countl_zero(arg);
}

/**
 * @brief Counts the argument's trailing zeroes.
 * @param arg the value whose trailing zeroes shall be counted.
 * @return the number of 0 bits below the lowest order 1 bit in the argument or
 * ::limb_bits if the argument is zero.
 */
constexpr int limb_tzcount(limb_type arg) noexcept
{
    // TODO : see if manual implementation of tzcount is desirable
    return std::countr_zero(arg);
}

/**
 * @brief Counts the number of 1 bits in the argument.
 * @param arg the value whose 1 bits shall be counted.
 * @return the number of 1 bits in the argument.
 */
constexpr int limb_popcount(limb_type arg) noexcept
{
    // TODO : see if manual implementation of popcount is desirable
    return std::popcount(arg);
}

/**@{*/
/**
 * @brief Increments the argument by one or by the provided carry bit, stores
 * the result in the destination address and returns the carry bit of the
 * increment operation.
 *
 * The behavior is undefined if @p result is not a valid pointer to an object of
 * type ::limb_type.
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
 * type ::limb_type.
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
 * type ::limb_type.
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
 * type ::limb_type.
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
        // TODO : find a cross-platform way to determine the correct type to pass to intrinsic functions
        // unsigned long and unsigned long long are both inconsistent
        unsigned long long tmp = 0;
        bool ret = static_cast<bool>(_addcarry_u64(carry, l, r, &tmp));
        *result = tmp;
        return ret;
    }
#elif (defined(_MSC_VER) && defined(_M_IX86)) || (defined(__GNUC__) && defined(__i386__))
    if (!std::is_constant_evaluated()) {
        limb_half_type ld[]{
            static_cast<limb_half_type>(l),
            static_cast<limb_half_type>(l >> limb_half_bits)
        };
        limb_half_type rd[]{
            static_cast<limb_half_type>(r),
            static_cast<limb_half_type>(r >> limb_half_bits)
        };
        carry = static_cast<bool>(_addcarry_u32(carry, ld[0], rd[0], &ld[0]));
        carry = static_cast<bool>(_addcarry_u32(carry, ld[1], rd[1], &ld[1]));
        *result = static_cast<limb_type>(ld[1]) << limb_half_bits | ld[0];
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
 * type ::limb_type.
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
        // TODO : find a cross-platform way to determine the correct type to pass to intrinsic functions
        // unsigned long and unsigned long long are both inconsistent
        unsigned long long tmp = 0;
        bool ret = static_cast<bool>(_subborrow_u64(borrow, l, r, &tmp));
        *result = tmp;
        return ret;
    }
#elif (defined(_MSC_VER) && defined(_M_IX86)) || (defined(__GNUC__) && defined(__i386__))
    if (!std::is_constant_evaluated()) {
        limb_half_type ld[]{
            static_cast<limb_half_type>(l),
            static_cast<limb_half_type>(l >> limb_half_bits)
        };
        limb_half_type rd[]{
            static_cast<limb_half_type>(r),
            static_cast<limb_half_type>(r >> limb_half_bits)
        };
        borrow = static_cast<bool>(_subborrow_u32(borrow, ld[0], rd[0], &ld[0]));
        borrow = static_cast<bool>(_subborrow_u32(borrow, ld[1], rd[1], &ld[1]));
        *result = static_cast<limb_type>(ld[1]) << limb_half_bits | ld[0];
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
 * object of ::limb_type.
 * 
 * The option to add two limbs is more for convenience than for technical
 * reasons. Since `l * r + c + d` precisely fits into two limbs and adding one
 * or two extra limbs frequently occurs in long multiplication algorithms, it is
 * a nice thing to have. The performance advantage however is likely to be
 * negligible.
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
    // int128 is constexpr compatible and compiles to optimal assembly already
    unsigned __int128 tmp = static_cast<unsigned __int128>(l) * r;
    *high_result = static_cast<limb_type>(tmp >> limb_bits);
    return static_cast<limb_type>(tmp);
#else
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_X64)
    // MSVC offers an intrinsic that exactly matches this function
    if (!std::is_constant_evaluated()) {
        return _umul128(l, r, high_result);
    }
#endif
    limb_half_type ld[]{
        static_cast<limb_half_type>(l),
        static_cast<limb_half_type>(l >> limb_half_bits)
    };
    limb_half_type rd[]{
        static_cast<limb_half_type>(r),
        static_cast<limb_half_type>(r >> limb_half_bits)
    };

    /*
     * Simple long multiplication algorithm.
     * The calculations are arranged such that the number of carry propagations
     * on x86 platforms stays minimal.
     */

    limb_type lo = static_cast<limb_type>(ld[0]) * rd[0];
    limb_type m1 = static_cast<limb_type>(ld[0]) * rd[1] + (lo >> limb_half_bits);
    limb_type m2 = static_cast<limb_type>(ld[1]) * rd[0] + static_cast<limb_half_type>(m1);
    limb_type hi = static_cast<limb_type>(ld[1]) * rd[1] + (m2 >> limb_half_bits) + (m1 >> limb_half_bits);
    lo = static_cast<limb_half_type>(lo) | m2 << limb_half_bits;
    *high_result = hi;
    return lo;
#endif
}

constexpr limb_type limb_mul(limb_type l, limb_type r, limb_type c, limb_type* high_result) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__SIZEOF_INT128__)
    unsigned __int128 tmp = static_cast<unsigned __int128>(l) * r + c;
    *high_result = static_cast<limb_type>(tmp >> limb_bits);
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
        static_cast<limb_half_type>(l >> limb_half_bits)
    };
    limb_half_type rd[]{
        static_cast<limb_half_type>(r),
        static_cast<limb_half_type>(r >> limb_half_bits)
    };
    limb_half_type cd[]{
        static_cast<limb_half_type>(c),
        static_cast<limb_half_type>(c >> limb_half_bits)
    };
    limb_type lo = static_cast<limb_type>(ld[0]) * rd[0] + cd[0];
    limb_type m1 = static_cast<limb_type>(ld[0]) * rd[1] + cd[1] + (lo >> limb_half_bits);
    limb_type m2 = static_cast<limb_type>(ld[1]) * rd[0] + static_cast<limb_half_type>(m1);
    limb_type hi = static_cast<limb_type>(ld[1]) * rd[1] + (m2 >> 32) + (m1 >> limb_half_bits);
    lo = static_cast<limb_half_type>(lo) | m2 << limb_half_bits;
    *high_result = hi;
    return lo;
#endif
}

constexpr limb_type limb_mul(limb_type l, limb_type r, limb_type c, limb_type d, limb_type* high_result) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__SIZEOF_INT128__)
    unsigned __int128 tmp = static_cast<unsigned __int128>(l) * r + c + d;
    *high_result = static_cast<limb_type>(tmp >> limb_bits);
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
        static_cast<limb_half_type>(l >> limb_half_bits)
    };
    limb_half_type rd[]{
        static_cast<limb_half_type>(r),
        static_cast<limb_half_type>(r >> limb_half_bits)
    };
    limb_half_type cd[]{
        static_cast<limb_half_type>(c),
        static_cast<limb_half_type>(c >> limb_half_bits)
    };
    limb_half_type dd[]{
        static_cast<limb_half_type>(d),
        static_cast<limb_half_type>(d >> limb_half_bits)
    };
    limb_type lo = static_cast<limb_type>(ld[0]) * rd[0] + cd[0] + dd[0];
    limb_type m1 = static_cast<limb_type>(ld[0]) * rd[1] + cd[1] + dd[1];
    limb_type m2 = static_cast<limb_type>(ld[1]) * rd[0] + (lo >> limb_half_bits) + static_cast<limb_half_type>(m1);
    limb_type hi = static_cast<limb_type>(ld[1]) * rd[1] + (m2 >> limb_half_bits) + (m1 >> limb_half_bits);
    lo = static_cast<limb_half_type>(lo) | m2 << limb_half_bits;
    *high_result = hi;
    return lo;
#endif
}
/**@}*/

namespace _detail_limb_type
{
/*
 * Divides the integer l[1]:l[0] by r[0], stores the quotient in q[0] and the
 * remainder in l[0].
 */
constexpr void limb_div_2_by_1(limb_half_type* l, limb_half_type* r, limb_half_type* q) noexcept
{
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_IX86)
    // MSVC has a nice intrinsic for this
    if (!std::is_constant_evaluated()) {
        limb_type tmp = static_cast<limb_type>(l[1]) << limb_half_bits | l[0];
        q[0] = _udiv64(tmp, r[0], l);
        return;
    }
#elif !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__i386__) && !defined(__x86_64__)
    // GCC and clang require inline assembly
    if (!std::is_constant_evaluated()) {
        asm("div %[d]"
            : "=a"(q[0]), "=d"(l[0])
            : "a"(l[0]), "d"(l[1]), [d]"r"(r[0])
            : "cc");
        return;
    }
#endif
    limb_type tmp = static_cast<limb_type>(l[1]) << limb_half_bits | l[0];
    q[0] = static_cast<limb_half_type>(tmp / r[0]);
    l[0] = static_cast<limb_half_type>(tmp % r[0]);
}

/*
 * Divides the integer l[2]:l[1]:l[0] by the integer r[1]:r[0] and stores the
 * quotient in q[0] and the remainder in l[1]:l[0].
 * 
 * It represents a single iteration in Knuth's Algorithm D as referenced in
 * `limb_div()`.
 */
constexpr void limb_div_3_by_2(limb_half_type* l, limb_half_type* r, limb_half_type* q) noexcept
{
    // edge case where the quotient is guaranteed to be its max value with no validation needed
    if (l[2] >= r[1]) {
        q[0] = static_cast<limb_half_type>(-1);
        limb_type tmp = static_cast<limb_half_type>(-1) * (static_cast<limb_type>(r[1]) << limb_half_bits | r[0]);
        tmp = (static_cast<limb_type>(l[1]) << limb_half_bits | l[0]) - tmp;
        l[1] = static_cast<limb_half_type>(tmp >> limb_half_bits);
        l[0] = static_cast<limb_half_type>(tmp);
        return;
    }

    limb_div_2_by_1(l + 1, r + 1, q);

    // q[0] is not less than the desired quotient and at most 2 too large.


    // Rearranging values for convenience. On x86 platforms this should boil
    // down to just a few `mov` instructions that can easily be optimized.
    limb_type ll = static_cast<limb_type>(l[1]) << limb_half_bits | l[0];
    limb_half_type lh = l[2];

    // Calculate the product of q[0] and r[1]:r[0] to compare it with l[2]:l[1]:l[0]
    limb_type cl = 0;
    limb_half_type ch = 0;
    {
        cl = static_cast<limb_type>(q[0]) * r[0];
        limb_type tmp = (cl >> limb_half_bits) + static_cast<limb_type>(q[0]) * r[1];
        cl = static_cast<limb_half_type>(cl) | (tmp << limb_half_bits);
        ch = static_cast<limb_half_type>(tmp >> limb_half_bits);
    }

    limb_type rr = static_cast<limb_type>(r[1]) << limb_half_bits | r[0];

    // Validation. Reduce q[0] until it is not too large anymore. This happens at most 2 times.
    if (ch > lh || (ch == lh && cl > ll)) {
        --q[0];
        ch -= limb_sub(cl, rr, &cl);
        if (ch > lh || (ch == lh && cl > ll)) {
            --q[0];
            ch -= limb_sub(cl, rr, &cl);
        }
    }

    // calculate and store remainder
    ll -= cl;
    l[0] = static_cast<limb_half_type>(ll);
    l[1] = static_cast<limb_half_type>(ll >> limb_half_bits);
}

inline void constexpr_assert_fail(const char* arr) { }

}

/**
 * @brief Divides a two limbs wide operand by a one limb wide operand, stores
 * the remainder at address @p remainder and returns the quotient.
 *
 * The behavior is undefined, if @p l_high is not less than @p r or if
 * @p remainder is not a valid pointer to an object of type
 * ::limb_type.
 *
 * @param l_high high part of the dividend.
 * @param l_low low part of the dividend.
 * @param r divisor.
 * @param remainder address where the remainder of the division will be stored.
 * @return the quotient of the divison.
*/
constexpr limb_type limb_div(limb_type l_high, limb_type l_low, limb_type r, limb_type* remainder) noexcept
{
    assert(l_high < r);
    /*
     * Constant evaluation with invalid arguments shall result in the program
     * being ill-formed rather than the behavior being undefined 
     */
    if (std::is_constant_evaluated() && l_high >= r)
        _detail_limb_type::constexpr_assert_fail("Calling limb_div with invalid arguments (l_high >= r).");

    /*
     * A div instruction is the fastest way, so we prefer it whenever possible.
     * It is unfortunately not constexpr compatible (yet).
     */
#if !defined(GMATHS_NO_INTRINSICS) && defined(_MSC_VER) && defined(_M_X64)
    // MSVC has an intrinsic for this
    if (!std::is_constant_evaluated()) {
        return _udiv128(l_high, l_low, r, remainder);
    }
#elif !defined(GMATHS_NO_INTRINSICS) && defined(__GNUC__) && defined(__x86_64__)
    // GCC and clang require inline assembly
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
    /*
     * While int128 is a valid implementation technique, its division is
     * significantly slower than a div instruction because of different
     * semantics in cases where `l_high >= r`.
     * 
     * We use it in cases where constant evaluation is desired or as a fallback
     * for other 64 bit CPUs than the ones supported above.
     */
    unsigned __int128 tmp = static_cast<unsigned __int128>(l_high) << 64 | l_low;
    *remainder = static_cast<limb_type>(tmp % r);
    return static_cast<limb_type>(tmp / r);
#else
    limb_half_type ld[4]{ };
    limb_half_type rd[2]{ };
    limb_half_type qd[2]{ };

    if (!(r >> limb_half_bits)) {
        /*
         * Fast version: 3 by 1 digit divison that does not require any
         * normalization and is in fact branchless.
         */
        ld[0] = static_cast<limb_half_type>(l_low);
        ld[1] = static_cast<limb_half_type>(l_low >> limb_half_bits);
        ld[2] = static_cast<limb_half_type>(l_high);
        rd[0] = static_cast<limb_half_type>(r);
        _detail_limb_type::limb_div_2_by_1(ld + 1, rd, qd + 1);
        _detail_limb_type::limb_div_2_by_1(ld, rd, qd);
        *remainder = ld[0];
    } else {
        /*
         * Slow version: Full 4 by 2 digit division.
         * It is loosely based on Algorithm D in Donald Knuth's The Art of
         * Computer Programming, Vol. 2, 4.3.1.
         * 
         * It consists of normalization steps and two iterations of a 3 by 2
         * digit division.
         */

        // Normalization
        int n = limb_lzcount(r);
        if (n) {
            l_high <<= n;
            l_high |= l_low >> (limb_bits - n);
            l_low <<= n;
            r <<= n;
        }

        ld[0] = static_cast<limb_half_type>(l_low);
        ld[1] = static_cast<limb_half_type>(l_low >> limb_half_bits);
        ld[2] = static_cast<limb_half_type>(l_high);
        ld[3] = static_cast<limb_half_type>(l_high >> limb_half_bits);
        rd[0] = static_cast<limb_half_type>(r);
        rd[0] = static_cast<limb_half_type>(r >> limb_half_bits);

        // two iterations calculating one digit of the quotient each.
        _detail_limb_type::limb_div_3_by_2(ld + 1, rd, qd + 1);
        _detail_limb_type::limb_div_3_by_2(ld, rd, qd);

        // denormalization of the remainder
        *remainder = (static_cast<limb_type>(ld[1]) << limb_half_bits | ld[0]) >> n;
    }
    return static_cast<limb_type>(qd[1]) << limb_half_bits | qd[0];
#endif
}

}

#endif
