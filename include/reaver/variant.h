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

#include "logic.h"
#include "prelude/fold.h"
#include "tpl/vector.h"
#include "tpl/nth.h"
#include "exception.h"
#include "tpl/unique.h"
#include "tpl/rebind.h"
#include "tpl/index_of.h"

namespace reaver { inline namespace _v1
{
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
    }

    template<typename... Args>
    class variant
    {
    public:
        static_assert(sizeof...(Args) != 0, "A nullary variant is invalid.");

        static_assert(all_of<
                !std::is_reference<Args>::value...
            >::value,
            "Reference types in the variant are not yet supported.");

        static_assert(all_of<
                std::is_nothrow_move_constructible<Args>::value...
            >::value,
            "All types in variant must be noexcept movable (that is, not absurdly insane).");

        static_assert(all_of<
                std::is_nothrow_destructible<Args>::value...
            >::value,
            "All types in variant must be noexcept destructible.");

        template<std::size_t N, typename... Ts>
        friend tpl::nth<tpl::vector<Ts...>, N> & get(variant<Ts...> &);

        template<std::size_t N, typename... Ts>
        friend const tpl::nth<tpl::vector<Ts...>, N> & get(const variant<Ts...> &);

        template<std::size_t N, typename... Ts>
        friend tpl::nth<tpl::vector<Ts...>, N> get(variant<Ts...> &&);

        template<typename T, typename std::enable_if<
            any_of<std::is_same<std::remove_reference_t<T>, Args>::value...>::value,
            int>::type = 0>
        variant(T && t) noexcept(noexcept(std::remove_reference_t<T>(std::forward<T>(t)))) : _tag(tpl::index_of<tpl::vector<Args...>, std::remove_reference_t<T>>())
        {
            new (&_storage) std::remove_reference_t<T>(std::forward<T>(t));
        }

        variant(const variant & other) noexcept(all_of<std::is_nothrow_copy_constructible<Args>::value...>::value) : _tag(other._tag)
        {
            using visitor_type = void(*)(variant & self, const variant & other);
            static visitor_type copy_ctors[] = {
                [](variant & self, const variant & other) {
                    new (&self._storage) Args(*reinterpret_cast<const Args *>(&other._storage));
                }...
            };

            copy_ctors[_tag](*this, other);
        }

        variant(variant && other) noexcept : _tag(other._tag)
        {
            using visitor_type = void (*)(variant & self, variant && other);
            static visitor_type move_ctors[] = {
                [](variant & self, variant && other) {
                    new (&self._storage) Args(std::move(*reinterpret_cast<Args *>(&other._storage)));
                }...
            };

            move_ctors[_tag](*this, std::move(other));
        }

        variant & operator=(const variant & other) noexcept(all_of<std::is_nothrow_copy_constructible<Args>::value...>::value)
        {
            using visitor_type = void (*)(variant & self, const variant & other);
            static visitor_type copy_assignments[] = {
                [](variant & self, const variant & other) {
                    using T = Args;
                    static visitor_type assignment_helpers[] = {
                        [](variant & self, const variant & other) {
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

        variant & operator=(variant && other) noexcept
        {
            using visitor_type = void (*)(variant & self, variant && other);
            static visitor_type move_assignments[] = {
                [](variant & self, variant && other) {
                    using T = Args;
                    static visitor_type assignment_helpers[] = {
                        [](variant & self, variant && other) {
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

        ~variant()
        {
            using dtor_type = void (*)(variant &);
            static dtor_type dtors[] = {
                [](variant & v) { reinterpret_cast<Args *>(&v._storage)->~Args(); }...
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
        std::size_t _tag;
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
    tpl::nth<tpl::vector<Ts...>, N> & get(variant<Ts...> & variant)
    {
        if (variant._tag != N)
        {
            throw invalid_variant_get(N, variant._tag);
        }

        return *reinterpret_cast<tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage);
    }

    template<std::size_t N, typename... Ts>
    const tpl::nth<tpl::vector<Ts...>, N> & get(const variant<Ts...> & variant)
    {
        if (variant._tag != N)
        {
            throw invalid_variant_get(N, variant._tag);
        }

        return *reinterpret_cast<const tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage);
    }

    template<std::size_t N, typename... Ts>
    tpl::nth<tpl::vector<Ts...>, N> get(variant<Ts...> && variant)
    {
        if (variant._tag != N)
        {
            throw invalid_variant_get(N, variant._tag);
        }

        return std::move(*reinterpret_cast<tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage));
    }

    template<typename... Ts, typename F>
    auto fmap(variant<Ts...> && var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(std::invoke(std::forward<F>(f), std::declval<Ts &&>()))...>, variant>;
        using visitor_type = result_type (*)(variant<Ts...> &&, F &&);
        static visitor_type visitors[] = {
            [](variant<Ts...> && v, F && f) -> result_type
            {
                return std::invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(std::move(v)));
            }...
        };

        auto index = var.index();
        return visitors[index](std::move(var), std::forward<F>(f));
    }

    template<typename... Ts, typename F>
    auto fmap(const variant<Ts...> & var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(std::invoke(std::forward<F>(f), std::declval<const Ts &>()))...>, variant>;
        using visitor_type = result_type (*)(const variant<Ts...> &, F &&);
        static visitor_type visitors[] = {
            [](const variant<Ts...> & v, F && f) -> result_type
            {
                return std::invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(v));
            }...
        };

        auto index = var.index();
        return visitors[index](var, std::forward<F>(f));
    }

    template<typename... Ts, typename F>
    auto fmap(variant<Ts...> & var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(std::invoke(std::forward<F>(f), std::declval<Ts &>()))...>, variant>;
        using visitor_type = result_type (*)(variant<Ts...> &, F &&);
        static visitor_type visitors[] = {
            [](variant<Ts...> & v, F && f) -> result_type { return std::invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, Ts>::value>(v)); }...
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

        return lhs.index() < rhs.index() && comparators[lhs.index()](lhs, rhs);
    }
}}

