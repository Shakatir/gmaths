#ifndef GMATHS_INTEGERS_LIMB_SPAN_HPP_BASE_INCLUDED
#define GMATHS_INTEGERS_LIMB_SPAN_HPP_BASE_INCLUDED

/**
 * @file gmaths/integers/limb_span/limb_span_base.hpp
 * @brief Provides the basis for big integer arithmetic.
 * 
 * All big integer arithmetic functions operate on spans of limbs.
 * `std::span` offers a great deal of flexibility as it can be both static sized
 * and dynamic sized. Thus it is used as the backbone of all types.
 */

#include <gmaths/integers/limb_type.hpp>
#include <gmaths/utility/basic_option.hpp>

#include <algorithm>
#include <cassert>
#include <compare>
#include <initializer_list>
#include <span>
#include <type_traits>

namespace gmaths::integers
{

namespace span_utils
{
/**
 * @brief Type traits class that determines if `T` is a `std::span<Elem, N>`.
 */
template<typename Elem, typename T>
struct is_span
{
    static constexpr bool value = false;
};

template<typename T, std::size_t N>
struct is_span<T, std::span<T, N>>
{
    static constexpr bool value = true;
};

/**
 * @brief Returns a subspan of @p arg consisting of the first @p n elements.
 * 
 * This function complements the `std::span<T, Sz>::first()` member functions.
 * The difference between them and this function is that this function allows
 * for simultaneous passing of the desired size through template and function
 * argument.
 * 
 * The resulting span has extent @p N and size @p n.
 */
template<std::size_t N, typename T, std::size_t Sz>
requires(N == std::dynamic_extent || Sz == std::dynamic_extent || N <= Sz)
constexpr auto first(std::span<T, Sz> arg, std::size_t n)
{
    assert(N == std::dynamic_extent || N == n);
    assert(n <= arg.size());
    return std::span<T, N>(arg.begin(), n);
}

/**
 * @brief Returns a subspan consisting of all elements not covered by ::first().
 */
template<std::size_t N, typename T, std::size_t Sz>
requires(N == std::dynamic_extent || Sz == std::dynamic_extent || N <= Sz)
constexpr auto skip(std::span<T, Sz> arg, std::size_t n)
{
    assert(N == std::dynamic_extent || N == n);
    assert(n <= arg.size());
    if constexpr (N == std::dynamic_extent || Sz == std::dynamic_extent) {
        return std::span<T, std::dynamic_extent>(arg.begin() + n, arg.size() - n);
    } else {
        return std::span<T, Sz - N>(arg.begin() + n, arg.size() - n);
    }
}

/**
 * @brief Returns a subspan of @p arg consisting of the last @p n elements.
 *
 * This function complements the `std::span<T, Sz>::last()` member functions.
 * The difference between them and this function is that this function allows
 * for simultaneous passing of the desired size through template and function
 * argument.
 *
 * The resulting span has extent @p N and size @p n.
 */
template<std::size_t N, typename T, std::size_t Sz>
requires(N == std::dynamic_extent || Sz == std::dynamic_extent || N <= Sz)
constexpr auto last(std::span<T, Sz> arg, std::size_t n)
{
    assert(N == std::dynamic_extent || N == n);
    assert(n <= arg.size());
    return std::span<T, N>(arg.end() - n, n);
}

/**
 * @brief Returns a subspan consisting of all elements not covered by ::last().
 */
template<std::size_t N, typename T, std::size_t Sz>
requires(N == std::dynamic_extent || Sz == std::dynamic_extent || N <= Sz)
constexpr auto truncate(std::span<T, Sz> arg, std::size_t n)
{
    assert(N == std::dynamic_extent || N == n);
    assert(n <= arg.size());
    if constexpr (N == std::dynamic_extent || Sz == std::dynamic_extent) {
        return std::span<T, std::dynamic_extent>(arg.data(), arg.size() - n);
    } else {
        return std::span<T, Sz - N>(arg.data(), arg.size() - n);
    }
}

/**
 * @brief Returns the smallest of the arguments passed or `std::dynamic_extent`
 * if any of the arguments are equal to that value.
 */
constexpr std::size_t min_extent(std::initializer_list<std::size_t> ilist) noexcept
{
    if (std::max(ilist) == std::dynamic_extent) {
        return std::dynamic_extent;
    } else {
        return std::min(ilist);
    }
}

/**
 * @brief Returns the largest of the arguments passed or `std::dynamic_extent`
 * if any of the arguments are equal to that value.
 */
constexpr std::size_t max_extent(std::initializer_list<std::size_t> ilist) noexcept
{
    return std::max(ilist);
}
}

/**
 * @brief Placeholder for any `std::span<limb_type, N>`.
 */
template<typename T>
concept output_limb_span = span_utils::is_span<limb_type, T>::value;

/**
 * @brief Placeholder for any `std::span<T, N>` where `T` is either `limb_type`
 * or `const limb_type`.
 */
template<typename T>
concept input_limb_span = output_limb_span<T> || span_utils::is_span<const limb_type, T>::value;

/**
 * @brief Returns the bit pattern that continues beyond the highest limb of the
 * argument.
 * 
 * In signed two's complement a negative value continues with infinite 1-bits, a
 * non-negative value continues with 0-bits. The result is therefore the maximum
 * value of `limb_type` for negative values (or `-1` when cast to a signed type)
 * and 0 for non-negative values.
 * 
 * @param arg number whose sign extension shall be computed
 * @tparam Signed true, if the argument is a signed two_s complement integer
 * @tparam Span any limb_span type
 * @return the sign extension of the number provided
 */
template<bool Signed = true, input_limb_span Span>
constexpr limb_type limb_span_sign_extension(Span arg) noexcept
{
    return Signed && !arg.empty() ? static_cast<limb_type>(static_cast<signed_limb_type>(arg.back()) >> (limb_bits - 1)) : 0;
}

namespace _detail_limb_span_base
{
struct span_option_tag { };
}

/**
 * @brief 
 */
using limb_span_option = utility::basic_option<_detail_limb_span_base::span_option_tag>;

/**@{*/
/**
 * @brief Marks the corresponding argument in limb_span operations as a signed
 * two's complement integer instead of an unsigned one.
 */
constexpr limb_span_option left_signed_option(0x1);
constexpr limb_span_option right_signed_option(0x10);
constexpr limb_span_option arg_signed_option(0x10);
/**@}*/

/**@{*/
/**
 * @brief Marks the corresponding argument in limb_span operations as mutable.
 * 
 * This means that the caller allows the function to change the contents of the 
 * arguments during the computation. This does not transfer ownership over
 * allocated memory. Resource management is the caller's responsibility.
 * 
 * This only has an effect if the corresponding argument is a span of non-const
 * elements. The functions will not cast any const-qualifications away.
 * 
 * Use with restrict options for maximum impact.
 */
constexpr limb_span_option left_mutable_option(0x2);
constexpr limb_span_option right_mutable_option(0x20);
constexpr limb_span_option arg_mutable_option(0x20);
/**@}*/

/**@{*/
/**
 * @brief Marks the corresponding pair of arguments in limb_span operations as
 * non-overlapping.
 * 
 * This offers the optimization opportunity to write partial results into the
 * provided limb_span arguments before the computation is finished, thus
 * avoiding internal memory allocation.
 * 
 * Note that the output span(s) cannot arbitrarily overlap with other arguments
 * even when these options are not set. Overlap is only allowed if the
 * overlapping spans begin at the same address.
 * 
 * Use with mutable options for maximum impact.
 */
constexpr limb_span_option restrict_left_right_option(0x1000);
constexpr limb_span_option restrict_dest_left_option(0x2000);
constexpr limb_span_option restrict_dest_right_option(0x4000);
constexpr limb_span_option restrict_dest_arg_option(0x4000);
/**@}*/

/**
 * @brief Reduces the number of branches in the implementation of a limb_span
 * operation.
 * 
 * This can potentially reduce code size and increase performance when the spans
 * are particularly small because simply continuing a quick computation might be
 * cheaper than the test to determine if the computation is necessary.
 * 
 * However this optimization quickly becomes obsolete or may even be drastically
 * slower for larger spans and should be handled with care.
 */
constexpr limb_span_option branchless_option(0x100);

/**
 * @brief Promises that the output span passed to the limb_span operation is
 * large enough to fit the result without truncating.
 *
 * The output of said operations if an unsufficient output span is provided,
 * turns from truncated to undefined.
 */
constexpr limb_span_option no_overflow_option(0x200);

}

#endif // !GMATHS_INTEGERS_LIMB_SPAN_HPP_BASE_INCLUDED
