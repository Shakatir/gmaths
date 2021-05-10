#ifndef GMATHS_INTEGERS_LIMB_SPAN_BITWISE_HPP_INCLUDED
#define GMATHS_INTEGERS_LIMB_SPAN_BITWISE_HPP_INCLUDED

#include <gmaths/integers/limb_span/limb_span_base.hpp>

namespace gmaths::integers
{

namespace _detail_limb_span_bitwise
{

std::size_t ar;

struct unary_one
{
    constexpr limb_type operator()(limb_type) const noexcept { return static_cast<limb_type>(-1); }
};

struct unary_zero
{
    constexpr limb_type operator()(limb_type) const noexcept { return 0; }
};

struct unary_neutral
{
    constexpr limb_type operator()(limb_type a) const noexcept { return a; }
};

struct unary_not
{
    constexpr limb_type operator()(limb_type a) const noexcept { return ~a; }
};

struct binary_and
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return l & r; }
    using bind_one = unary_neutral;
    using bind_zero = unary_zero;
    using flip = binary_and;
};

struct binary_nand
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return ~(l & r); }
    using bind_one = unary_not;
    using bind_zero = unary_one;
    using flip = binary_nand;
};

struct binary_or
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return l | r; }
    using bind_one = unary_one;
    using bind_zero = unary_neutral;
    using flip = binary_or;
};

struct binary_nor
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return ~(l | r); }
    using bind_one = unary_zero;
    using bind_zero = unary_not;
    using flip = binary_nor;
};

struct binary_xor
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return l ^ r; }
    using bind_one = unary_not;
    using bind_zero = unary_neutral;
    using flip = binary_xor;
};

struct binary_xnor
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return ~(l ^ r); }
    using bind_one = unary_neutral;
    using bind_zero = unary_not;
    using flip = binary_xnor;
};

struct binary_greater;
struct binary_less
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return ~l & r; }
    using bind_one = unary_not;
    using bind_zero = unary_zero;
    using flip = binary_greater;
};

struct binary_greater
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return l & ~r; }
    using bind_one = unary_zero;
    using bind_zero = unary_neutral;
    using flip = binary_greater;
};

struct binary_geq;
struct binary_leq
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return ~l | r; }
    using bind_one = unary_one;
    using bind_zero = unary_not;
    using flip = binary_geq;
};

struct binary_geq
{
    constexpr limb_type operator()(limb_type l, limb_type r) const noexcept { return l | ~r; }
    using bind_one = unary_neutral;
    using bind_zero = unary_one;
    using flip = binary_leq;
};

template<int N, typename DIt, typename RIt, typename Func>
constexpr void unary_unroll_helper(DIt d, RIt r, Func f, int n = N) noexcept
{
    limb_type arr[N]{ };
    std::copy_n(r, n, arr);
    for (int i = 0; i < n; ++i) {
        arr[i] = f(arr[i]);
    }
    std::copy_n(arr, n, d);
}

template<int N, typename DIt, typename LIt, typename RIt, typename Func>
constexpr void binary_unroll_helper(DIt d, LIt l, RIt r, Func f, int n = N) noexcept
{
    limb_type l_arr[N]{ };
    std::copy_n(l, n, l_arr);
    limb_type r_arr[N]{ };
    std::copy_n(r, n, r_arr);
    for (int i = 0; i < n; ++i) {
        l_arr[i] = f(l_arr[i], r_arr[i]);
    }
    std::copy_n(l_arr, n, d);
}

template<int N, typename DIt, typename LIt, typename Func>
constexpr void binary_unroll_helper(DIt d, LIt l, limb_type r, Func f, int n = N) noexcept
{
    limb_type l_arr[N]{ };
    std::copy_n(l, n, l_arr);
    for (int i = 0; i < n; ++i) {
        l_arr[i] = f(l_arr[i], r);
    }
    std::copy_n(l_arr, n, d);
}

constexpr int unroll_large = 16;
constexpr int unroll_small = 4;

template<std::size_t N, typename DIt, typename Func>
constexpr void unary_inplace_unroll(DIt d, Func f, std::size_t count) noexcept
{
    if constexpr (N != std::dynamic_extent) {
        count = N;
    }

    for (auto i = count / unroll_large; i > 0; --i, d += unroll_large)
        unary_unroll_helper<unroll_large>(d, d, f);
    for (auto i = (count % unroll_large) / unroll_small; i > 0; --i, d += unroll_small)
        unary_unroll_helper<unroll_small>(d, d, f);
    unary_unroll_helper<unroll_small>(d, d, f, (count % unroll_large) % unroll_small);
}

template<std::size_t N, typename DIt, typename RIt, typename Func>
constexpr void unary_unroll(DIt d, RIt r, Func f, std::size_t count) noexcept
{
    if constexpr (N != std::dynamic_extent) {
        count = N;
    }

    for (auto i = count / unroll_large; i > 0; --i, d += unroll_large, r += unroll_large)
        unary_unroll_helper<unroll_large>(d, r, f);
    for (auto i = (count % unroll_large) / unroll_small; i > 0; --i, d += unroll_small, r += unroll_small)
        unary_unroll_helper<unroll_small>(d, r, f);
    unary_unroll_helper<unroll_small>(d, r, f, (count % unroll_large) % unroll_small);
}

template<std::size_t N, typename DIt, typename Func>
constexpr void binary_inplace_unroll(DIt d, limb_type r, Func f, std::size_t count) noexcept
{
    if constexpr (N != std::dynamic_extent) {
        count = N;
    }

    for (auto i = count / unroll_large; i > 0; --i, d += unroll_large)
        binary_unroll_helper<unroll_large>(d, d, r, f);
    for (auto i = (count % unroll_large) / unroll_small; i > 0; --i, d += unroll_small)
        binary_unroll_helper<unroll_small>(d, d, r, f);
    binary_unroll_helper<unroll_small>(d, d, r, f, (count % unroll_large) % unroll_small);
}

template<std::size_t N, typename DIt, typename LIt, typename Func>
constexpr void binary_unroll(DIt d, LIt l, limb_type r, Func f, std::size_t count) noexcept
{
    if constexpr (N != std::dynamic_extent) {
        count = N;
    }

    for (auto i = count / unroll_large; i > 0; --i, d += unroll_large, l += unroll_large)
        binary_unroll_helper<unroll_large>(d, l, r, f);
    for (auto i = (count % unroll_large) / unroll_small; i > 0; --i, d += unroll_small, l += unroll_small)
        binary_unroll_helper<unroll_small>(d, l, r, f);
    binary_unroll_helper<unroll_small>(d, l, r, f, (count % unroll_large) % unroll_small);
}

template<std::size_t N, typename DIt, typename RIt, typename Func>
constexpr void binary_inplace_unroll(DIt d, RIt r, Func f, std::size_t count) noexcept
{
    if constexpr (N != std::dynamic_extent) {
        count = N;
    }

    for (auto i = count / unroll_large; i > 0; --i, d += unroll_large, r += unroll_large)
        binary_unroll_helper<unroll_large>(d, d, r, f);
    for (auto i = (count % unroll_large) / unroll_small; i > 0; --i, d += unroll_small, r += unroll_small)
        binary_unroll_helper<unroll_small>(d, d, r, f);
    binary_unroll_helper<unroll_small>(d, d, r, f, (count % unroll_large) % unroll_small);
}

template<std::size_t N, typename DIt, typename LIt, typename RIt, typename Func>
constexpr void binary_unroll(DIt d, LIt l, RIt r, Func f, std::size_t count) noexcept
{
    if constexpr (N != std::dynamic_extent) {
        count = N;
    }

    for (auto i = count / unroll_large; i > 0; --i, d += unroll_large, l += unroll_large, r += unroll_large)
        binary_unroll_helper<unroll_large>(d, l, r, f);
    for (auto i = (count % unroll_large) / unroll_small; i > 0; --i, d += unroll_small, l += unroll_small, r += unroll_small)
        binary_unroll_helper<unroll_small>(d, l, r, f);
    binary_unroll_helper<unroll_small>(d, l, r, f, (count % unroll_large) % unroll_small);
}

template<output_limb_span D, typename Func>
constexpr void unary_inplace(D d, Func f) noexcept
{
    if constexpr (std::is_same_v<Func, unary_one> || std::is_same_v<Func, unary_zero>) {
        std::fill(d.begin(), d.end(), f(0));
    } else if constexpr (std::is_same_v<Func, unary_neutral>) {
        return;
    } else {
        unary_inplace_unroll<D::extent>(d.begin(), f, d.size());
    }
}

template<bool RSigned, output_limb_span D, input_limb_span R, typename Func>
constexpr void unary(D d, R r, Func f) noexcept
{
    constexpr std::size_t DN = D::extent;
    constexpr std::size_t RN = R::extent;

    if constexpr (std::is_same_v<Func, unary_one> || std::is_same_v<Func, unary_zero>) {
        std::fill(d.begin(), d.end(), f(0));
    } else if constexpr (std::is_same_v<Func, unary_neutral>) {
        std::copy_n(r.begin(), std::min(d.size(), r.size()), d.begin());
    } else {
        constexpr std::size_t N = span_utils::min_extent({DN, RN});
        unary_unroll<N>(d.begin(), r.begin(), f, std::min(d.size(), r.size()));

        if constexpr (std::max(DN, RN) == std::dynamic_extent || DN > RN) {
            if (d.size() <= r.size()) {
                return;
            }

            std::fill(d.begin() + r.size(), d.end(), f(limb_span_sign_extension<RSigned>(r)));
        }
    }
}

template<bool Branchless, bool RSigned, output_limb_span D, typename Func>
constexpr void binary_inplace(D d, limb_type r, Func f) noexcept
{
    if constexpr (!RSigned) {
        unary_inplace(d, typename Func::bind_zero{ });
    } else if constexpr (!Branchless) {
        if (r) {
            unary_inplace(d, typename Func::bind_one{ });
        } else {
            unary_inplace(d, typename Func::bind_zero{ });
        }
    } else {
        binary_inplace_unroll<D::extent>(d.begin(), r, f, d.size());
    }
}

template<bool Branchless, bool LSigned, bool RSigned, output_limb_span D, input_limb_span L, typename Func>
constexpr void binary(D d, L l, limb_type r, Func f) noexcept
{
    constexpr std::size_t DN = D::extent;
    constexpr std::size_t LN = L::extent;

    if constexpr (!RSigned) {
        unary<LSigned>(d, l, typename Func::bind_zero{ });
    } else if constexpr (!Branchless) {
        if (r) {
            unary<LSigned>(d, l, typename Func::bind_one{ });
        } else {
            unary<LSigned>(d, l, typename Func::bind_zero{ });
        }
    } else {
        constexpr std::size_t N = span_utils::min_extent({DN, LN});
        binary_unroll<N>(d.begin(), l.begin(), r, f, std::min(d.size(), l.size()));

        if constexpr (std::max(DN, LN) == std::dynamic_extent || DN > LN) {
            if (d.size() <= l.size()) {
                return;
            }

            std::fill(d.begin() + l.size(), d.end(), f(limb_span_sign_extension<LSigned>(l), r));
        }
    }
}


template<bool Branchless, bool RSigned, output_limb_span D, input_limb_span R, typename Func>
constexpr void binary_inplace(D d, R r, Func f) noexcept
{
    constexpr std::size_t DN = D::extent;
    constexpr std::size_t RN = R::extent;

    constexpr std::size_t N = span_utils::min_extent({DN, RN});
    binary_inplace_unroll<N>(d.begin(), r.begin(), f, std::min(d.size(), r.size()));
    if constexpr (std::max(DN, RN) == std::dynamic_extent || DN > RN) {
        if (d.size() <= r.size()) {
            return;
        }

        auto dtail = span_utils::skip<RN>(d, r.size());
        binary_inplace<Branchless, RSigned>(dtail, limb_span_sign_extension<RSigned>(r), f);
    }
}

template<bool Branchless, bool LSigned, bool RSigned, output_limb_span D, input_limb_span L, input_limb_span R, typename Func>
constexpr void binary(D d, L l, R r, Func f) noexcept
{
    constexpr std::size_t DN = D::extent;
    constexpr std::size_t LN = L::extent;
    constexpr std::size_t RN = R::extent;

    constexpr std::size_t N = span_utils::min_extent({DN, LN, RN});

    std::size_t minSize = std::min({d.size(), l.size(), r.size()});
    binary_unroll<N>(d.begin(), l.begin(), r.begin(), f, minSize);
    if constexpr (std::max({DN, LN, RN}) == std::dynamic_extent || DN > LN || DN > RN) {
        if (d.size() <= minSize) {
            return;
        }

        if constexpr (RN == std::dynamic_extent || (LN > RN && DN > RN)) {
            if (l.size() > r.size()) {
                auto dtail = span_utils::skip<RN>(d, r.size());
                auto ltail = span_utils::skip<RN>(l, r.size());
                limb_type rext = limb_span_sign_extension<RSigned>(r);
                binary<Branchless, LSigned, RSigned>(dtail, ltail, rext, f);
            }
        }

        if constexpr (LN == std::dynamic_extent || (RN > LN && DN > LN)) {
            if (l.size() < r.size()) {
                auto dtail = span_utils::skip<LN>(d, l.size());
                limb_type lext = limb_span_sign_extension<LSigned>(l);
                auto rtail = span_utils::skip<LN>(r, l.size());
                typename Func::flip flip{ };
                binary<Branchless, RSigned, LSigned>(dtail, rtail, lext, flip);
            }
        }

        if constexpr (std::max(LN, RN) == std::dynamic_extent || LN == RN) {
            if (l.size() == r.size()) {
                limb_type lext = limb_span_sign_extension<LSigned>(l);
                limb_type rext = limb_span_sign_extension<RSigned>(r);
                std::fill(d.begin() + l.size(), d.end(), f(lext, rext));
            }
        }
    }
}

template<limb_span_option, output_limb_span D, typename Func>
constexpr void unary_inplace_dispatch(D d, Func f) noexcept
{
    _detail_limb_span_bitwise::unary_inplace(d, f);
}

template<limb_span_option Opt, output_limb_span D, input_limb_span R, typename Func>
constexpr void unary_dispatch(D d, R r, Func f) noexcept
{
    constexpr bool RSigned = static_cast<bool>(Opt & right_signed_option);
    std::span<const limb_type, R::extent> r2 = r;
    _detail_limb_span_bitwise::unary<RSigned>(d, r2, f);
}

template<limb_span_option Opt, output_limb_span D, input_limb_span R, typename Func>
constexpr void binary_inplace_dispatch(D d, R r, Func f) noexcept
{
    constexpr bool Branchless = static_cast<bool>(Opt & branchless_option);
    constexpr bool RSigned = static_cast<bool>(Opt & right_signed_option);
    std::span<const limb_type, R::extent> r2 = r;
    _detail_limb_span_bitwise::binary_inplace<Branchless, RSigned>(d, r2, f);
}

template<limb_span_option Opt, output_limb_span D, input_limb_span L, input_limb_span R, typename Func>
constexpr void binary_dispatch(D d, L l, R r, Func f) noexcept
{
    constexpr bool Branchless = static_cast<bool>(Opt & branchless_option);
    constexpr bool LSigned = static_cast<bool>(Opt & left_signed_option);
    constexpr bool RSigned = static_cast<bool>(Opt & right_signed_option);
    std::span<const limb_type, L::extent> l2 = l;
    std::span<const limb_type, R::extent> r2 = r;
    _detail_limb_span_bitwise::binary<Branchless, LSigned, RSigned>(d, l2, r2, f);
}

}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D>
constexpr void limb_span_bitnot_inplace(D d) noexcept
{
    _detail_limb_span_bitwise::unary_not func{ };
    _detail_limb_span_bitwise::unary_inplace_dispatch<Opt>(d, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitnot(D d, R r) noexcept
{
    _detail_limb_span_bitwise::unary_not func{ };
    _detail_limb_span_bitwise::unary_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitand_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_and func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitand(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_and func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitnand_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_nand func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitnand(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_nand func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitor_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_or func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitor(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_or func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitnor_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_nor func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitnor(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_nor func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitxor_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_xor func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitxor(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_xor func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitxnor_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_xnor func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitxnor(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_xnor func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitless_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_less func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitless(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_less func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitleq_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_leq func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitleq(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_leq func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitgreater_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_greater func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitgreater(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_greater func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span R>
constexpr void limb_span_bitgeq_inplace(D d, R r) noexcept
{
    _detail_limb_span_bitwise::binary_geq func{ };
    _detail_limb_span_bitwise::binary_inplace_dispatch<Opt>(d, r, func);
}

template<limb_span_option Opt = limb_span_option(0), output_limb_span D, input_limb_span L, input_limb_span R>
constexpr void limb_span_bitgeq(D d, L l, R r) noexcept
{
    _detail_limb_span_bitwise::binary_geq func{ };
    _detail_limb_span_bitwise::binary_dispatch<Opt>(d, l, r, func);
}

}

#endif // !GMATHS_INTEGERS_LIMB_SPAN_BITWISE_HPP_INCLUDED
