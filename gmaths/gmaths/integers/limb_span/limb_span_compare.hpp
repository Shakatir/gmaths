#ifndef GMATHS_INTEGERS_LIMB_SPAN_COMPARE_HPP_INCLUDED
#define GMATHS_INTEGERS_LIMB_SPAN_COMPARE_HPP_INCLUDED

/**
 * @file gmaths/integers/limb_span/limb_span_compare.hpp
 * @brief Provides functions for comparison of the numeric values stored in
 * two limb_spans.
 */

#include <gmaths/integers/limb_span/limb_span_base.hpp>

namespace gmaths::integers
{
namespace _detail_limb_span_compare
{

template<bool LSigned, bool RSigned, input_limb_span L, input_limb_span R>
constexpr std::strong_ordering compare_promoted(L l, R r) noexcept
{
    static_assert (std::max(L::extent, R::extent) == std::dynamic_extent || L::extent >= R::extent,
        "expecting left range to be larger (in number of limbs) than right range");
    assert(l.size() >= r.size());

    if (l.empty()) {
        return std::strong_ordering::equal;
    }

    auto li = l.rbegin();
    auto ri = r.rbegin();

    if (l.size() > r.size()) {
        limb_type rext = limb_span_sign_extension<RSigned>(r);
        if constexpr (LSigned) {
            if (*li != rext)
                return static_cast<signed_limb_type>(*li) <=> static_cast<signed_limb_type>(rext);
            ++li;
        }

        auto lm = l.rend() - r.size();
        for (; li != lm; ++li) {
            if (*li != rext)
                return *li <=> rext;
        }
    } else if constexpr (LSigned && RSigned) {
        if (*li != *ri)
            return static_cast<signed_limb_type>(*li) <=> static_cast<signed_limb_type>(*ri);
        ++li, ++ri;
    }

    auto le = l.rend();
    for (; li != le; ++li, ++ri) {
        if (*li != *ri)
            return *li <=> *ri;
    }

    return std::strong_ordering::equal;
}

template<bool LSigned, bool RSigned, input_limb_span L, input_limb_span R>
constexpr std::strong_ordering compare_infinite(L l, R r) noexcept
{
    constexpr std::size_t LN = L::extent;
    constexpr std::size_t RN = R::extent;
    static_assert (std::max(LN, RN) == std::dynamic_extent || LN >= RN,
        "expecting left range to be larger (in number of limbs) than right range");
    assert(l.size() >= r.size());

    // Infinite comparison is equivalent to promoted comparison except when a signed type is promoted to an unsigned type
    // So we simply make sure that in those cases the signs are compared first, then continue with promoted comparison.
    if constexpr (LSigned != RSigned) {
        if constexpr ((LN >= RN && RSigned) || (RN >= LN && LSigned) || std::max(LN, RN) == std::dynamic_extent) {
            signed_limb_type lext = static_cast<signed_limb_type>(limb_span_sign_extension<LSigned>(l));
            signed_limb_type rext = static_cast<signed_limb_type>(limb_span_sign_extension<RSigned>(r));
            if (lext != rext)
                return lext <=> rext;
        }
    }

    return compare_promoted<LSigned, RSigned>(l, r);
}

}

/**
 * @brief Compares two integer values according to the standard promotion rules.
 * 
 * If the larger of the two spans is marked as unsigned, the smaller span will
 * be promoted to an unsigned value as well which creates a discrepancy between
 * the result of this function and the actual numeric values represented by the
 * spans. Use ::limb_span_compare_infinite() if no promotion shall be performed.
 * 
 * @tparam test for ::left_signed_option and ::right_signed_option, all other
 * options are ignored
 * @param l left hand side of the comparison
 * @param r right hand side of the comparsion
 * @return result of promoted comparison
 */
template<limb_span_option Opt, input_limb_span L, input_limb_span R>
constexpr std::strong_ordering limb_span_compare_promoted(L l, R r) noexcept
{
    std::span<const limb_type, L::extent> l2 = l;
    std::span<const limb_type, R::extent> r2 = r;
    constexpr bool LSigned = static_cast<bool>(Opt & left_signed_option);
    constexpr bool RSigned = static_cast<bool>(Opt & right_signed_option);

    if constexpr (std::max(L::extent, R::extent) == std::dynamic_extent || L::extent >= R::extent) {
        if (l.size() >= r.size())
            return _detail_limb_span_compare::compare_promoted<LSigned, RSigned>(l2, r2);
    }

    if constexpr (std::max(L::extent, R::extent) == std::dynamic_extent || L::extent < R::extent) {
        if (l.size() < r.size())
            return 0 <=> _detail_limb_span_compare::compare_promoted<RSigned, LSigned>(r2, l2);
    }

    // unreachable
    return std::strong_ordering::equal;
}

/**
 * @brief Compares two integer values without performing any promotions.
 * 
 * The resulting value is guaranteed to be equivalent to the actual ordering of
 * unbounded integers.
 * 
 * @param l left hand side of the comparison
 * @param r right hand side of the comparsion
 * @return result of infinite comparison
 */
template<limb_span_option Opt, input_limb_span L, input_limb_span R>
constexpr std::strong_ordering limb_span_compare_infinite(L l, R r) noexcept
{
    std::span<const limb_type, L::extent> l2 = l;
    std::span<const limb_type, R::extent> r2 = r;
    constexpr bool LSigned = static_cast<bool>(Opt & left_signed_option);
    constexpr bool RSigned = static_cast<bool>(Opt & right_signed_option);

    if constexpr (std::max(L::extent, R::extent) == std::dynamic_extent || L::extent >= R::extent) {
        if (l.size() >= r.size())
            return _detail_limb_span_compare::compare_infinite<LSigned, RSigned>(l2, r2);
    }

    if constexpr (std::max(L::extent, R::extent) == std::dynamic_extent || L::extent < R::extent) {
        if (l.size() < r.size())
            return 0 <=> _detail_limb_span_compare::compare_infinite<RSigned, LSigned>(r2, l2);
    }

    // unreachable
    return std::strong_ordering::equal;
}

}

#endif