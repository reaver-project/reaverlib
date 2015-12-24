/**
 * Reaver Library Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 **/

#pragma once

#include <type_traits>
#include <functional>

#include "logic.h"
#include "prelude/fold.h"
#include "tpl/vector.h"
#include "tpl/nth.h"
#include "exception.h"
#include "tpl/unique.h"
#include "tpl/rebind.h"
#include "tpl/index_of.h"
#include "invoke.h"

namespace reaver { inline namespace _v1
{
    namespace _detail
    {
        template<typename... Ts>
        class _variant;

        template<typename T>
        struct _dereference_wrapper
        {
            using type = T;
        };

        template<typename T>
        struct _dereference_wrapper<std::reference_wrapper<T>>
        {
            using type = T &;
        };

        template<typename T>
        using _dereference_wrapper_t = typename _dereference_wrapper<T>::type;

        template<typename T>
        _dereference_wrapper_t<T> _dereference(T && t)
        {
            return std::forward<T>(t);
        }
    }

    template<std::size_t N, typename... Ts>
    _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(_detail::_variant<Ts...> &);

    template<std::size_t N, typename... Ts>
    const _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(const _detail::_variant<Ts...> &);

    template<std::size_t N, typename... Ts>
    _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> get(_detail::_variant<Ts...> &&);

    namespace _detail
    {
        struct _max
        {
            template<typename T, typename U>
            constexpr decltype(auto) operator()(T && t, U && u) const
            {
                return t > u ? std::forward<T>(t) : std::forward<U>(u);
            }
        };

        template<typename T>
        struct _replace_reference
        {
            using type = T;
        };

        template<typename T>
        struct _replace_reference<T &>
        {
            using type = std::reference_wrapper<T>;
        };

        template<typename T>
        using _replace_reference_t = typename _replace_reference<T>::type;

        template<typename... Args>
        class _variant
        {
        public:
            static_assert(sizeof...(Args) != 0, "A nullary variant is invalid.");

            static_assert(all_of<
                    !std::is_rvalue_reference<Args>::value...
                >::value,
                "Rvalue-reference types in the variant are not supported.");

            static_assert(all_of<
                    std::is_nothrow_move_constructible<Args>::value...
                >::value,
                "All types in variant must be noexcept movable (that is, not absurdly insane).");

            static_assert(all_of<
                    std::is_nothrow_destructible<Args>::value...
                >::value,
                "All types in variant must be noexcept destructible.");

            template<std::size_t N, typename... Ts>
            friend _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & reaver::get(_variant<Ts...> &);

            template<std::size_t N, typename... Ts>
            friend const _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & reaver::get(const _variant<Ts...> &);

            template<std::size_t N, typename... Ts>
            friend _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> reaver::get(_variant<Ts...> &&);

            template<typename T, typename std::enable_if<
                any_of<std::is_same<std::remove_reference_t<T>, Args>::value...>::value,
                int>::type = 0>
            _variant(T && t) noexcept(noexcept(std::remove_reference_t<T>(std::forward<T>(t)))) : _tag(tpl::index_of<tpl::vector<Args...>, std::remove_reference_t<T>>())
            {
                new (&_storage) std::remove_reference_t<T>(std::forward<T>(t));
            }

            template<typename T, typename std::enable_if<
                any_of<std::is_same<std::reference_wrapper<T>, Args>::value...>::value,
                int>::type = 0>
            _variant(T & t) noexcept : _tag(tpl::index_of<tpl::vector<Args...>, std::reference_wrapper<T>>())
            {
                new (&_storage) auto(std::ref(t));
            }

            template<typename T, typename std::enable_if<
                !any_of<std::is_same<std::remove_reference_t<T>, Args>::value...>::value
                    && any_of<std::is_constructible<Args, T &&>::value...>::value,
                int>::type = 0>
            _variant(T && t)
            {
                auto constructor = make_overload_set(
                    [&](auto && t) -> typename std::enable_if<std::is_constructible<Args, decltype(t)>::value>::type {
                        _tag = tpl::index_of<tpl::vector<Args...>, Args>();
                        new (&_storage) Args(std::forward<decltype(t)>(t));
                    }...
                );

                constructor(std::forward<T>(t));
            }

            _variant(const _variant & other) noexcept(all_of<std::is_nothrow_copy_constructible<Args>::value...>::value) : _tag(other._tag)
            {
                using visitor_type = void(*)(_variant & self, const _variant & other);
                static visitor_type copy_ctors[] = {
                    [](_variant & self, const _variant & other) {
                        new (&self._storage) Args(*reinterpret_cast<const Args *>(&other._storage));
                    }...
                };

                copy_ctors[_tag](*this, other);
            }

            _variant(_variant && other) noexcept : _tag(other._tag)
            {
                using visitor_type = void (*)(_variant & self, _variant && other);
                static visitor_type move_ctors[] = {
                    [](_variant & self, _variant && other) {
                        new (&self._storage) Args(std::move(*reinterpret_cast<Args *>(&other._storage)));
                    }...
                };

                move_ctors[_tag](*this, std::move(other));
            }

            _variant & operator=(const _variant & other) noexcept(all_of<std::is_nothrow_copy_constructible<Args>::value...>::value)
            {
                using visitor_type = void (*)(_variant & self, const _variant & other);
                static visitor_type copy_assignments[] = {
                    [](_variant & self, const _variant & other) {
                        using T = Args;
                        static visitor_type assignment_helpers[] = {
                            [](_variant & self, const _variant & other) {
                                auto old_ptr = reinterpret_cast<T *>(&self._storage);
                                if (std::is_nothrow_copy_constructible<Args>())
                                {
                                    old_ptr->~T();
                                    new (&self._storage) Args(*reinterpret_cast<const Args *>(&other._storage));
                                    self._tag = other._tag;

                                    return;
                                }

                                auto old_value = std::move(*old_ptr);
                                old_ptr->~T();
                                try
                                {
                                    new (&self._storage) Args(*reinterpret_cast<const Args *>(&other._storage));
                                    self._tag = other._tag;
                                }
                                catch (...)
                                {
                                    new (&self._storage) T(std::move(old_value));
                                    throw;
                                }
                            }...
                        };

                        assignment_helpers[other._tag](self, other);
                    }...
                };

                copy_assignments[_tag](*this, other);
                return *this;
            }

            _variant & operator=(_variant && other) noexcept
            {
                using visitor_type = void (*)(_variant & self, _variant && other);
                static visitor_type move_assignments[] = {
                    [](_variant & self, _variant && other) {
                        using T = Args;
                        static visitor_type assignment_helpers[] = {
                            [](_variant & self, _variant && other) {
                                reinterpret_cast<T *>(&self._storage)->~T();
                                new (&self._storage) Args(std::move(*reinterpret_cast<Args *>(&other._storage)));
                                self._tag = other._tag;

                                return;
                            }...
                        };

                        assignment_helpers[other._tag](self, std::move(other));
                    }...
                };

                move_assignments[_tag](*this, std::move(other));
                return *this;
            }

            ~_variant()
            {
                using dtor_type = void (*)(_variant &);
                static dtor_type dtors[] = {
                    [](_variant & v) { reinterpret_cast<Args *>(&v._storage)->~Args(); }...
                };

                assert(_tag < sizeof...(Args));

                dtors[_tag](*this);
            }

            std::size_t index() const
            {
                return _tag;
            }

        private:
            using _storage_type = std::aligned_storage_t<
                foldl(_detail::_max{}, sizeof(Args)...),
                foldl(_detail::_max{}, alignof(Args)...)
            >;

            _storage_type _storage;
            std::size_t _tag = -1;
        };
    }

    template<typename... Ts>
    class variant : public _detail::_variant<_detail::_replace_reference_t<Ts>...>
    {
        using _base = _detail::_variant<_detail::_replace_reference_t<Ts>...>;

    public:
        using _base::_base;
    };

    class invalid_variant_get : public exception
    {
    public:
        invalid_variant_get(std::size_t expected, std::size_t actual) : exception(logger::error), expected(expected), actual(actual)
        {
            *this << "Invalid variant get: expected type at index " << expected << " to be active, while type at index " << actual << " was actually active.";
        }

        std::size_t expected;
        std::size_t actual;
    };

    template<std::size_t N, typename... Ts>
    _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(_detail::_variant<Ts...> & variant)
    {
        if (variant._tag != N)
        {
            throw invalid_variant_get(N, variant._tag);
        }

        return _detail::_dereference(*reinterpret_cast<tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage));
    }

    template<std::size_t N, typename... Ts>
    const _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(const _detail::_variant<Ts...> & variant)
    {
        if (variant._tag != N)
        {
            throw invalid_variant_get(N, variant._tag);
        }

        return _detail::_dereference(*reinterpret_cast<const tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage));
    }

    template<std::size_t N, typename... Ts>
    _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> get(_detail::_variant<Ts...> && variant)
    {
        if (variant._tag != N)
        {
            throw invalid_variant_get(N, variant._tag);
        }

        return _detail::_dereference(std::move(*reinterpret_cast<tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage)));
    }

    template<typename... Ts, typename F>
    auto fmap(variant<Ts...> && var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(invoke(std::forward<F>(f), std::declval<Ts &&>()))...>, variant>;
        using visitor_type = result_type (*)(variant<Ts...> &&, F &&);
        static visitor_type visitors[] = {
            [](variant<Ts...> && v, F && f) -> result_type { return invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(std::move(v))); }...
        };

        auto index = var.index();
        return visitors[index](std::move(var), std::forward<F>(f));
    }

    template<typename... Ts, typename F>
    auto fmap(const variant<Ts...> & var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(invoke(std::forward<F>(f), std::declval<const Ts &>()))...>, variant>;
        using visitor_type = result_type (*)(const variant<Ts...> &, F &&);
        static visitor_type visitors[] = {
            [](const variant<Ts...> & v, F && f) -> result_type { return invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(v)); }...
        };

        auto index = var.index();
        return visitors[index](var, std::forward<F>(f));
    }

    template<typename... Ts, typename F>
    auto fmap(variant<Ts...> & var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(invoke(std::forward<F>(f), std::declval<Ts &>()))...>, variant>;
        using visitor_type = result_type (*)(variant<Ts...> &, F &&);
        static visitor_type visitors[] = {
            [](variant<Ts...> & v, F && f) -> result_type { return invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(v)); }...
        };

        auto index = var.index();
        return visitors[index](var, std::forward<F>(f));
    }

    template<typename... Ts>
    bool operator==(const variant<Ts...> & lhs, const variant<Ts...> & rhs)
    {
        using comparator_type = bool (*)(const variant<Ts...> &, const variant<Ts...> &);
        static comparator_type comparators[] = {
            [](const variant<Ts...> & lhs, const variant<Ts...> & rhs) {
                return get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(lhs) == get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(rhs);
            }...
        };

        return lhs.index() == rhs.index() && comparators[lhs.index()](lhs, rhs);
    }

    template<typename... Ts>
    bool operator!=(const variant<Ts...> & lhs, const variant<Ts...> & rhs)
    {
        return !(lhs == rhs);
    }

    template<typename... Ts>
    bool operator<(const variant<Ts...> & lhs, const variant<Ts...> & rhs)
    {
        using comparator_type = bool (*)(const variant<Ts...> &, const variant<Ts...> &);
        static comparator_type comparators[] = {
            [](const variant<Ts...> & lhs, const variant<Ts...> & rhs) {
                return get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(lhs) < get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(rhs);
            }...
        };

        return lhs.index() < rhs.index() || (lhs.index() == rhs.index() && comparators[lhs.index()](lhs, rhs));
    }
}}

