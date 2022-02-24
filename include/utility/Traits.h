#pragma once

#include <type_traits>
#include <utility>

/// A helper macro that reuses a const accessor method for a non-const accessor method, avoiding
/// code duplication.  For example:
/// \code{cpp}
/// struct Foo
/// {
///     bool b = false;
///
///     // Const accessor by value
///     const bool get_b_value() const { return b; }
///     // Example for accessor without any parameters
///     auto get_b_ref() { REUSE_CONST_ACCESSOR(get_b_ref) }
///
///     // Const-ref accessor that takes no parameters
///     const bool& get_b_ref() const { return b; }
///     // Example for ref accessor without any parameters (Note: auto&)
///     auto& get_b_ref() { REUSE_CONST_ACCESSOR(get_b_ref) }
///
///     // Const-ref accessor that takes a parameter
///     const bool& get_b_ref(int a) const { return b; }
///     // Example for ref accessor with parameters (Note: auto&)
///     auto& get_b_ref(int a) { REUSE_CONST_ACCESSOR(get_b_ref, a) }
///
/// };
/// \endcode
/// Note: REUSE_CONST_ACCESSOR(__func__) would not be portable, since __func__ means different
/// string literals on different systems.
///
/// There is a lot to unpack here.
/// - const_return_type takes on the return type of the const method
/// - return_type has a conditional statement to remove const for return types by value or by
///   reference (note: r-values are not handled)
/// - The return statement calls the const method and removes the const-ness (this is one of the
///   very few instances where this is acceptable, see Scott Meyers Effective Modern C++, page 23 in
///   3rd edition)
#define REUSE_CONST_ACCESSOR(method_name, ...)                                         \
    using const_return_type = decltype(std::as_const(*this).method_name(__VA_ARGS__)); \
    using return_type = typename                                                       \
      std::conditional<std::is_reference<const_return_type>::value,                    \
                       typename std::add_lvalue_reference<typename std::remove_const<           \
                         typename std::remove_reference<const_return_type>::type>::type>::type, \
                       typename std::remove_const<const_return_type>::type                      \
                       >::type;             \
    return const_cast<typename return_type>(std::as_const(*this).method_name(__VA_ARGS__));

/// Used with REUSE_CONST_ACCESSOR for templated methods 
#define COMMA ,