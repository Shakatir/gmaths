#ifndef GMATHS_UTILITY_BASIC_OPTION_HPP_INCLUDED
#define GMATHS_UTILITY_BASIC_OPTION_HPP_INCLUDED

#include <compare>
#include <cstddef>
#include <limits>

namespace gmaths
{
namespace utility
{

/**
 * @brief Wrapper type around `unsigned long long` to treat it as a bitset.
 * 
 * When using an enum or enum class as a set of flags, you often encounter the
 * problem that neither option offers typesafe bitwise operators.
 * That makes their usage as flags quite cumbersome as you find yourself casting
 * between enum and integer types all the time. Or you have to create a type
 * or a bunch of operators and functions that resolve the issue.
 * 
 * This is where the ::basic_option comes in. It provides typesafe bitwise
 * operators and can be parametrized with any arbitrary type to distinguish
 * between different option types.
 * 
 * The type fulfills the requirements of a literal type, so its values can be
 * used as template parameters.
 * 
 * @tparam DomainTag arbitrary type to distinguish incompatible types
*/
template<typename DomainTag>
struct basic_option
{
    /**
     * @brief Initializes the object with a default value of 0.
     */
    constexpr basic_option() noexcept : _value{0} { }

    /**
     * @brief Initializes the object with the provided value.
     * @param v Value to be assigned to the object.
     */
    constexpr explicit basic_option(unsigned long long v) noexcept : _value{v} { }

    /**
     * @brief Casts the object to the underlying integer type.
     * @return Underlying integer.
     */
    constexpr explicit operator unsigned long long() const noexcept { return _value; }

    /**
     * @brief Tests if the value is non-zero.
     * @return true iff the value is non-zero.
     */
    constexpr explicit operator bool() const noexcept { return !!_value; }

    /**@{*/
    /**
     * @brief Compares the objects according to their underlying values.
     * @param o other object to be compared to 
     * @return result of the comparison of underlying integers.
     */
    constexpr auto operator<=>(const basic_option& o) const noexcept = default;
    constexpr bool operator==(const basic_option& o) const noexcept = default;
    /**@}*/

    /**@{*/
    /**
     * @brief Basic bitwise operations.
     * @param o right handside parameter.
     */
    constexpr basic_option operator~() const noexcept { return basic_option(~_value); }
    constexpr basic_option& operator&=(basic_option o) noexcept { _value &= o._value; return *this; }
    constexpr basic_option operator&(basic_option o) const noexcept { return r &= *this; }
    constexpr basic_option& operator|=(basic_option o) noexcept { _value |= o._value; return *this; }
    constexpr basic_option operator|(basic_option o) const noexcept { return r |= *this; }
    constexpr basic_option& operator^=(basic_option o) noexcept { _value ^= o._value; return *this; }
    constexpr basic_option operator^(basic_option o) const noexcept { return r ^= *this; }
    /**@}*/

    /**
     * @brief Underlying integer value. Visibility is public to allow for
     * treatment as literal type.
     */
    unsigned long long _value;
};

}
}

#endif