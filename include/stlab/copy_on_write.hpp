/*
    Copyright 2013 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
/**************************************************************************************************/

/*!
    @file copy_on_write.hpp
    @brief Copy-on-write wrapper implementation

    This file contains the complete implementation of stlab::copy_on_write, a thread-safe
    copy-on-write wrapper for any type that models Regular. The implementation uses atomic
    reference counting and provides lazy copying semantics.
*/

#ifndef STLAB_COPY_ON_WRITE_HPP
#define STLAB_COPY_ON_WRITE_HPP

/**************************************************************************************************/

/// @defgroup member_types Member Types
/// @defgroup member_functions Member Functions
/// @defgroup observers Observers
/// @defgroup non_member_functions Non-Member Functions

/*!
    @mainpage

    [![Source
   Code](https://img.shields.io/badge/Source_Code-blue?logo=github)](https://github.com/stlab/copy-on-write)

    @section intro_sec Description

    The `stlab::copy_on_write<T>` class provides a copy-on-write wrapper for any type that models
    Regular. This implementation allows multiple instances to share the same underlying data until
    one of them needs to modify it, at which point a copy is made. The class is not intended to be
    exposed in a class interface, but rather used as a utility to construct a type where some or all
    of the members are copy_on_write.

    The `stlab::copy_on_write<T>::write()` operation optionally takes a transform and in-place
    transform function. For many mutable operations, there is a transform operation that can be
    applied to perform the copy with the mutation in a more efficient manner.

    Copy-on-write is most useful for types between 4K and 1M in size and expected to be copied
    frequently, such as part of a transaction system, i.e., implementing undo/redo. Larger objects
    can be decomposed into smaller copy-on-write objects or use a data structure such as a rope that
    has an internal copy-on-write structure. Smaller objects can be more efficient by avoiding heap
    allocations (such as with small object optimization) and always copying; however, if they always
    heap allocate, they may be more efficient by using copy-on-write.

    @section features_sec Key Features

    - **Thread-safe**: Uses atomic reference counting for safe concurrent access
    - **Header-only**: No compilation required, just include the header
    - **C++17**: Leverages modern C++ features for clean, efficient implementation

    @section copy_on_write_sec Class `stlab::copy_on_write<T>`
    @copydoc stlab::copy_on_write

    @subsection member_types_sec Member Types
    @ref member_types

    @subsection member_functions_sec Member Functions
    @ref member_functions

    @subsubsection observers_sec Observers
    @ref observers

    @subsection non_member_functions_sec Non-Member Functions
    @ref non_member_functions

    @section usage_sec Basic Usage

    This example demonstrates the core functionality including efficient copying,
    copy-on-write semantics, identity checking, and swap operations.

    @include basic_usage_test.cpp

    @section license_sec License

    Distributed under the Boost Software License, Version 1.0.
*/

/*!
    @example basic_usage_test.cpp

    This example demonstrates the core functionality of stlab::copy_on_write including:
    - Efficient copying through shared data
    - Copy-on-write semantics when modifying
    - Identity checking and uniqueness testing
    - Swap operations

    The example shows how multiple copy_on_write instances can share the same underlying
    data until one needs to be modified, at which point a copy is automatically made.
*/

/**************************************************************************************************/

#include <atomic>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

/**************************************************************************************************/

/*!
    The stlab namespace contains utilities and components for modern C++ development.
*/
namespace stlab {

/**************************************************************************************************/

/*!
    A copy-on-write wrapper for any type that models Regular.

    Copy-on-write semantics allow for an object to be lazily copied - only creating a copy when
    the value is modified and there is more than one reference to the value.

    This class is thread safe and supports types that model Moveable.
*/
template <typename T> // T models Regular
class copy_on_write {
    struct model {
        std::atomic<std::size_t> _count{1};

        model() noexcept(std::is_nothrow_constructible_v<T>) = default;

        template <class... Args>
        explicit model(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) :
            _value(std::forward<Args>(args)...) {}

        T _value;
    };

    model* _self;

    template <class U>
    using disable_copy = std::enable_if_t<!std::is_same_v<std::decay_t<U>, copy_on_write>>*;

    template <typename U>
    using disable_copy_assign =
        std::enable_if_t<!std::is_same_v<std::decay_t<U>, copy_on_write>, copy_on_write&>;

    auto default_model() noexcept(std::is_nothrow_constructible_v<T>) -> model* {
        static model default_s;
        return &default_s;
    }

public:
    /*! @addtogroup member_types
    @{ */
    /*!
        @deprecated Use element_type instead. The type of value stored.
    */
    /* [[deprecated]] */ using value_type = T;

    /*!
        @brief The type of value stored
    */
    using element_type = T;
    /*! @} */

    /*! @addtogroup member_functions
    @{ */
    /*!
        @brief Default constructs the wrapped value.
    */
    copy_on_write() noexcept(std::is_nothrow_constructible_v<T>) {
        _self = default_model();

        // coverity[useless_call]
        _self->_count.fetch_add(1, std::memory_order_relaxed);
    }

    /*!
        @brief Constructs a new instance by forwarding arguments to the wrapped value constructor.
    */
    template <class U>
    copy_on_write(U&& x, disable_copy<U> = nullptr) : _self(new model(std::forward<U>(x))) {}

    /*!
        @brief Constructs a new instance by forwarding multiple arguments to the wrapped value
       constructor.
    */
    template <class U, class V, class... Args>
    copy_on_write(U&& x, V&& y, Args&&... args) :
        _self(new model(std::forward<U>(x), std::forward<V>(y), std::forward<Args>(args)...)) {}

    /*!
        @brief Copy constructor that shares the underlying data with the source object.
    */
    copy_on_write(const copy_on_write& x) noexcept : _self(x._self) {
        assert(_self && "FATAL (sparent) : using a moved copy_on_write object");

        // coverity[useless_call]
        _self->_count.fetch_add(1, std::memory_order_relaxed);
    }

    /*!
        @brief Move constructor that takes ownership of the source object's data.
    */
    copy_on_write(copy_on_write&& x) noexcept : _self{std::exchange(x._self, nullptr)} {
        assert(_self && "WARNING (sparent) : using a moved copy_on_write object");
    }

    /*!
        @brief Destructor
    */
    ~copy_on_write() {
        assert(!_self || ((_self->_count > 0) && "FATAL (sparent) : double delete"));
        if (_self && (_self->_count.fetch_sub(1, std::memory_order_release) == 1)) {
            std::atomic_thread_fence(std::memory_order_acquire);
            if constexpr (std::is_default_constructible_v<element_type>) {
                assert(_self != default_model());
            }
            delete _self;
        }
    }

    /*!
        @brief Copy assignment operator that shares the underlying data with the source object.
    */
    auto operator=(const copy_on_write& x) noexcept -> copy_on_write& {
        // self-assignment is not allowed to disable cert-oop54-cpp warning (and is likely a bug)
        assert(this != &x && "self-assignment is not allowed");
        return *this = copy_on_write(x);
    }

    /*!
        @brief Move assignment operator that takes ownership of the source object's data.
    */
    auto operator=(copy_on_write&& x) noexcept -> copy_on_write& {
        auto tmp{std::move(x)};
        swap(*this, tmp);
        return *this;
    }

    /*!
        @brief Assigns a new value to the wrapped object, optimizing for in-place assignment when
       unique.
    */
    template <class U>
    auto operator=(U&& x) -> disable_copy_assign<U> {
        if (_self && unique()) {
            _self->_value = std::forward<U>(x);
            return *this;
        }

        return *this = copy_on_write(std::forward<U>(x));
    }

    /*! @addtogroup observers
    @{ */

    /*!
        @brief Obtains a non-const reference to the underlying value.

        This will copy the underlying value if necessary so changes to the value do not affect
        other copy_on_write objects sharing the same data.
    */
    auto write() -> element_type& {
        if (!unique()) *this = copy_on_write(read());

        return _self->_value;
    }

    /*!
        @brief If the object is not unique, the transform is applied to the underlying value to copy
       it and a reference to the new value is returned. If the object is unique, the inplace
       function is called with a reference to the underlying value and a reference to the value is
       returned.

        @param transform A function object that takes a const reference to the underlying value and
        returns a new value.
        @param inplace A function object that takes a reference to the underlying value and modifies
        it in place.

        @return A reference to the underlying value.
    */
    template <class Transform, class Inplace>
    auto write(Transform transform, Inplace inplace) -> element_type& {
        static_assert(std::is_invocable_r_v<T, Transform, const T&>,
                      "Transform must be invocable with const T&");
        static_assert(std::is_invocable_r_v<void, Inplace, T&>,
                      "Inplace must be invocable with T&");

        if (!unique()) {
            *this = copy_on_write(transform(read()));
        } else {
            inplace(_self->_value);
        }

        return _self->_value;
    }

    /*!
        @brief Returns a const reference to the underlying value for read-only access.
    */
    [[nodiscard]] auto read() const noexcept -> const element_type& {
        assert(_self && "FATAL (sparent) : using a moved copy_on_write object");

        return _self->_value;
    }

    /*!
        @brief Implicit conversion to const reference of the underlying value.
    */
    operator const element_type&() const noexcept { return read(); }

    /*!
        @brief Dereference operator that returns a const reference to the underlying value.
    */
    auto operator*() const noexcept -> const element_type& { return read(); }

    /*!
        @brief Arrow operator that returns a const pointer to the underlying value.
    */
    auto operator->() const noexcept -> const element_type* { return &read(); }

    /*!
        @brief Returns true if this is the only reference to the underlying object.

        This is useful to determine if calling write() will cause a copy.
    */
    [[nodiscard]] auto unique() const noexcept -> bool {
        assert(_self && "FATAL (sparent) : using a moved copy_on_write object");

        return _self->_count.load(std::memory_order_acquire) == 1;
    }

    /*!
        @deprecated Use unique() instead. Returns true if this is the only reference to the
       underlying object.
    */
    [[deprecated]] [[nodiscard]] auto unique_instance() const noexcept -> bool { return unique(); }

    /*!
        @brief Returns true if this object and the given object share the same underlying data.
    */
    [[nodiscard]] auto identity(const copy_on_write& x) const noexcept -> bool {
        assert((_self && x._self) && "FATAL (sparent) : using a moved copy_on_write object");

        return _self == x._self;
    }

    /*! @} */
    /*! @} */

    /*!
        @addtogroup non_member_functions
        @{ */
    /*!
        @brief Efficiently swaps the contents of two copy_on_write objects.
    */
    friend inline void swap(copy_on_write& x, copy_on_write& y) noexcept {
        std::swap(x._self, y._self);
    }

    /*! @{ */
    /*!
        @brief Comparisons can be done with the underlying value or the copy_on_write object.
    */
    friend inline auto operator<(const copy_on_write& x, const copy_on_write& y) noexcept -> bool {
        return !x.identity(y) && (*x < *y);
    }

    friend inline auto operator<(const copy_on_write& x, const element_type& y) noexcept -> bool {
        return *x < y;
    }

    friend inline auto operator<(const element_type& x, const copy_on_write& y) noexcept -> bool {
        return x < *y;
    }

    friend inline auto operator>(const copy_on_write& x, const copy_on_write& y) noexcept -> bool {
        return y < x;
    }

    friend inline auto operator>(const copy_on_write& x, const element_type& y) noexcept -> bool {
        return y < x;
    }

    friend inline auto operator>(const element_type& x, const copy_on_write& y) noexcept -> bool {
        return y < x;
    }

    friend inline auto operator<=(const copy_on_write& x, const copy_on_write& y) noexcept -> bool {
        return !(y < x);
    }

    friend inline auto operator<=(const copy_on_write& x, const element_type& y) noexcept -> bool {
        return !(y < x);
    }

    friend inline auto operator<=(const element_type& x, const copy_on_write& y) noexcept -> bool {
        return !(y < x);
    }

    friend inline auto operator>=(const copy_on_write& x, const copy_on_write& y) noexcept -> bool {
        return !(x < y);
    }

    friend inline auto operator>=(const copy_on_write& x, const element_type& y) noexcept -> bool {
        return !(x < y);
    }

    friend inline auto operator>=(const element_type& x, const copy_on_write& y) noexcept -> bool {
        return !(x < y);
    }

    friend inline auto operator==(const copy_on_write& x, const copy_on_write& y) noexcept -> bool {
        return x.identity(y) || (*x == *y);
    }

    friend inline auto operator==(const copy_on_write& x, const element_type& y) noexcept -> bool {
        return *x == y;
    }

    friend inline auto operator==(const element_type& x, const copy_on_write& y) noexcept -> bool {
        return x == *y;
    }

    friend inline auto operator!=(const copy_on_write& x, const copy_on_write& y) noexcept -> bool {
        return !(x == y);
    }

    friend inline auto operator!=(const copy_on_write& x, const element_type& y) noexcept -> bool {
        return !(x == y);
    }

    friend inline auto operator!=(const element_type& x, const copy_on_write& y) noexcept -> bool {
        return !(x == y);
    }
    /*! @} */
    /*! @} */
};
/**************************************************************************************************/

} // namespace stlab

/**************************************************************************************************/

#endif

/**************************************************************************************************/
