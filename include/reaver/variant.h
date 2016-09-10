/**
 * Reaver Library Licence
 *
 * Copyright © 2015-2016 Michał "Griwes" Dominiak
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

/*
 * The "GCC dumbness" I'm refering to throughout this file is https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47226.
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
#include "traits.h"
#include "tpl/replace.h"
#include "overloads.h"
#include "tpl/concat.h"
#include "tpl/map.h"

namespace reaver { inline namespace _v1
{
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

    template<typename CRTP, typename... Ts>
    class _variant;

    template<typename T>
    struct recursive_wrapper
    {
        recursive_wrapper() : _storage{ std::make_unique<T>() }
        {
        }

        recursive_wrapper(const recursive_wrapper & other) : _storage{ std::make_unique<T>(*other._storage) }
        {
        }
        recursive_wrapper(recursive_wrapper &&) noexcept = default;

        recursive_wrapper & operator=(const recursive_wrapper & rhs)
        {
            _storage = std::make_unique<T>(*rhs._storage);
            return *this;
        }
        recursive_wrapper & operator=(recursive_wrapper &&) noexcept = default;

        recursive_wrapper & operator=(const T & t)
        {
            _storage = std::make_unique<T>(t);
            return *this;
        }

        recursive_wrapper & operator=(T && t)
        {
            _storage = std::make_unique<T>(std::move(t));
            return *this;
        }

        recursive_wrapper(T t) : _storage{ std::make_unique<T>(std::move(t)) }
        {
        }

        operator T &()
        {
            return *_storage;
        }

        operator const T &() const
        {
            return *_storage;
        }

        T & operator*()
        {
            return *_storage;
        }

        const T & operator*() const
        {
            return *_storage;
        }

        T * operator->()
        {
            return &*_storage;
        }

        const T * operator->() const
        {
            return &*_storage;
        }

        template<typename U = T, typename = decltype(std::declval<U>().index())>
        auto index() const
        {
            return _storage->index();
        }

        template<typename U = T, typename = decltype(std::declval<U>() == std::declval<U>())>
        bool operator==(const recursive_wrapper & rhs) const
        {
            return *_storage == *rhs;
        }

        template<typename U = T, typename = decltype(std::declval<U>() != std::declval<U>())>
        bool operator!=(const recursive_wrapper & rhs) const
        {
            return *_storage != *rhs;
        }

        template<typename U = T, typename = decltype(std::declval<U>() < std::declval<U>())>
        bool operator<(const recursive_wrapper & rhs) const
        {
            return *_storage < *rhs;
        }

    private:
        std::unique_ptr<T> _storage;
    };

    namespace _detail
    {
        template<typename T>
        struct _dereference_wrapper
        {
            using type = T;
        };

        template<typename T>
        struct _dereference_wrapper<std::reference_wrapper<T>>
        {
            using type = typename _dereference_wrapper<T>::type &;
        };

        template<typename T>
        struct _dereference_wrapper<recursive_wrapper<T>>
        {
            using type = typename _dereference_wrapper<T>::type;
        };

        template<typename T>
        using _dereference_wrapper_t = typename _dereference_wrapper<T>::type;

        template<typename T>
        _dereference_wrapper_t<T> _dereference(T && t)
        {
            return std::forward<T>(t);
        }

        struct _max
        {
            template<typename T, typename U>
            constexpr decltype(auto) operator()(T && t, U && u) const
            {
                return t > u ? std::forward<T>(t) : std::forward<U>(u);
            }
        };

        template<typename CRTP, typename... Args>
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

            template<std::size_t N, typename CRTP_, typename... Ts>
            friend _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(_variant<CRTP_, Ts...> &);

            template<std::size_t N, typename CRTP_, typename... Ts>
            friend const _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(const _variant<CRTP_, Ts...> &);

            template<std::size_t N, typename CRTP_, typename... Ts>
            friend _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> get(_variant<CRTP_, Ts...> &&);

            template<typename T, typename std::enable_if<
                any_of<std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, Args>::value...>::value,
                int>::type = 0>
            _variant(T && t) noexcept(noexcept(std::remove_reference_t<T>(std::forward<T>(t)))) : _tag(tpl::index_of<tpl::vector<Args...>, std::remove_cv_t<std::remove_reference_t<T>>>())
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

            // GCC is mightly dumb
            // and fails hard at type deduction
            // so we need some additional boilerplate for the next constructor
        private:
            struct _generator
            {
                // needed for this to compile under GCC 5.1
                _generator() {}

                template<typename T>
                static void conversion_context(T);

                template<typename Arg, typename T, typename = decltype(conversion_context<Arg>({ std::declval<T>() }))>
                static auto generate(choice<0>)
                {
                    return [](_variant & v, T && t)
                    {
                        v._tag = tpl::index_of<tpl::vector<Args...>, Arg>();
                        new (&v._storage) Arg{ std::forward<T>(t) };
                    };
                }

                // workaround for an ICE in GCC 6.1
                // the last argument *must be* a template for overload resolution to work properly in this case
                // (favoring working braced-init over ())
                // but if that happens with the lambda-style of the 0th choice... it segfaults
                template<typename Arg, typename T>
                struct to_generate
                {
                    // needed for this to compile under GCC 5.1
                    to_generate() {}

                    template<typename U>
                    void operator()(_variant & v, U && t) const
                    {
                        v._tag = tpl::index_of<tpl::vector<Args...>, Arg>();
                        new (&v._storage) Arg(std::forward<T>(t));
                    }
                };

                template<typename Arg, typename T, typename = decltype(conversion_context<Arg>(std::declval<T>()))>
                static auto generate(choice<1>)
                {
                    return to_generate<Arg, T>();
                }

                template<typename...>
                static auto generate(choice<2>)
                {
                    return [](auto &&... args)
                    {
                        static_assert(sizeof...(args) != 0, "no viable constructor for variant");
                    };
                };
            };

        public:
            template<typename T, typename std::enable_if<
                !any_of<std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, Args>::value...>::value
                    && any_of<std::is_constructible<Args, T &&>::value...>::value,
                int>::type = 0>
            _variant(T && t)
            {
                auto constructor = reaver::make_overload_set(
                    _generator::template generate<Args, T>(select_overload{})...
                );

                constructor(*this, std::forward<T>(t));
            }

            template<typename CRTP_, typename... Ts, typename std::enable_if<is_subset<tpl::vector<Ts...>, tpl::vector<Args...>>::value, int>::type = 0>
            _variant(const _variant<CRTP_, Ts...> & other)
            {
                fmap(other, [&](auto && value) {
                    using Other = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;

                    _tag = tpl::index_of<tpl::vector<Args...>, Other>();
                    new (&_storage) Other(std::forward<decltype(value)>(value));

                    return unit{};
                });
            }

            template<typename CRTP_, typename... Ts, typename std::enable_if<is_subset<tpl::vector<Ts...>, tpl::vector<Args...>>::value, int>::type = 0>
            _variant(_variant<CRTP_, Ts...> & other)
            {
                fmap(other, [&](auto && value) {
                    using Other = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;

                    _tag = tpl::index_of<tpl::vector<Args...>, Other>();
                    new (&_storage) Other(std::forward<decltype(value)>(value));

                    return unit{};
                });
            }

            template<typename CRTP_, typename... Ts, typename std::enable_if<is_subset<tpl::vector<Ts...>, tpl::vector<Args...>>::value, int>::type = 0>
            _variant(_variant<CRTP_, Ts...> && other)
            {
                fmap(std::move(other), [&](auto && value) {
                    using Other = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;

                    _tag = tpl::index_of<tpl::vector<Args...>, Other>();
                    new (&_storage) Other(std::forward<decltype(value)>(value));

                    return unit{};
                });
            }

            template<typename T, typename std::enable_if<
                is_container<T>::value
                    && any_of<(is_container<Args>::value && std::is_same<tpl::replace<T, CRTP, recursive_wrapper<CRTP>>, Args>::value)...>::value
                    && !std::is_same<tpl::replace<T, CRTP, recursive_wrapper<CRTP>>, T>::value,
                int>::type = 0>
            _variant(const T & t) : _variant{ tpl::replace<T, CRTP, recursive_wrapper<CRTP>>(std::begin(t), std::end(t)) }
            {
            }

            template<typename T, typename std::enable_if<
                is_container<T>::value
                    && any_of<(is_container<Args>::value && std::is_same<tpl::replace<T, CRTP, recursive_wrapper<CRTP>>, Args>::value)...>::value
                    && !std::is_same<tpl::replace<T, CRTP, recursive_wrapper<CRTP>>, T>::value,
                int>::type = 0>
            _variant(T && t) : _variant{ tpl::replace<T, CRTP, recursive_wrapper<CRTP>>(std::make_move_iterator(std::begin(t)), std::make_move_iterator(std::end(t))) }
            {
            }

            _variant(const _variant & other) noexcept(all_of<std::is_nothrow_copy_constructible<Args>::value...>::value) : _tag(other._tag)
            {
                using visitor_type = void(*)(_variant & self, const _variant & other);

                // hack for GCC dumbness
                auto generator = [](auto type) {
                    using Arg = typename decltype(type)::type;
                    return [](_variant & self, const _variant & other) {
                        new (&self._storage) Arg(*reinterpret_cast<const Arg *>(&other._storage));
                    };
                };

                static visitor_type copy_ctors[] = {
                    generator(id<Args>())...
                };

                copy_ctors[_tag](*this, other);
            }

            _variant(_variant && other) noexcept : _tag(other._tag)
            {
                using visitor_type = void (*)(_variant & self, _variant && other);

                // hack for GCC dumbness
                auto generator = [](auto type) {
                    using Arg = typename decltype(type)::type;
                    return [](_variant & self, _variant && other) {
                        new (&self._storage) Arg(std::move(*reinterpret_cast<Arg *>(&other._storage)));
                    };
                };

                static visitor_type move_ctors[] = {
                    generator(id<Args>())...
                };

                move_ctors[_tag](*this, std::move(other));
            }

            _variant & operator=(const _variant & other) noexcept(all_of<std::is_nothrow_copy_constructible<Args>::value...>::value)
            {
                using visitor_type = void (*)(_variant & self, const _variant & other);

                // hack for GCC dumbness
                auto generator = [](auto type) {
                    using T = typename decltype(type)::type;

                    return [](_variant & self, const _variant & other) {
                        // hack for GCC dumbness
                        auto generator = [](auto type) {
                            using Arg = typename decltype(type)::type;

                            return [](_variant & self, const _variant & other) {
                                auto old_ptr = reinterpret_cast<T *>(&self._storage);
                                if (std::is_nothrow_copy_constructible<Arg>())
                                {
                                    old_ptr->~T();
                                    new (&self._storage) Arg(*reinterpret_cast<const Arg *>(&other._storage));
                                    self._tag = other._tag;

                                    return;
                                }

                                auto old_value = std::move(*old_ptr);
                                old_ptr->~T();
                                try
                                {
                                    new (&self._storage) Arg(*reinterpret_cast<const Arg *>(&other._storage));
                                    self._tag = other._tag;
                                }
                                catch (...)
                                {
                                    new (&self._storage) T(std::move(old_value));
                                    throw;
                                }
                            };
                        };

                        static visitor_type assignment_helpers[] = {
                            generator(id<Args>())...
                        };

                        assignment_helpers[other._tag](self, other);
                    };
                };

                static visitor_type copy_assignments[] = {
                    generator(id<Args>())...
                };

                copy_assignments[_tag](*this, other);
                return *this;
            }

            _variant & operator=(_variant && other) noexcept
            {
                using visitor_type = void (*)(_variant & self, _variant && other);

                // hack for GCC dumbness
                auto generator = [](auto type) {
                    using T = typename decltype(type)::type;

                    return [](_variant & self, _variant && other) {
                        // hack for GCC dumbness
                        auto generator = [](auto type) {
                            using Arg = typename decltype(type)::type;

                            return [](_variant & self, _variant && other) {
                                reinterpret_cast<T *>(&self._storage)->~T();
                                new (&self._storage) Arg(std::move(*reinterpret_cast<Arg *>(&other._storage)));
                                self._tag = other._tag;

                                return;
                            };
                        };

                        static visitor_type assignment_helpers[] = {
                            generator(id<Args>())...
                        };

                        assignment_helpers[other._tag](self, std::move(other));
                    };
                };

                static visitor_type move_assignments[] = {
                    generator(id<Args>())...
                };

                move_assignments[_tag](*this, std::move(other));
                return *this;
            }

            ~_variant()
            {
                using dtor_type = void (*)(_variant &);

                // hack for GCC dumbness
                auto generator = [](auto type) {
                    using Arg = typename decltype(type)::type;

                    return [](_variant & v) { reinterpret_cast<Arg *>(&v._storage)->~Arg(); };
                };

                static dtor_type dtors[] = {
                    generator(id<Args>())...
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

        template<std::size_t N, typename CRTP, typename... Ts>
        _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(_detail::_variant<CRTP, Ts...> & variant)
        {
            if (variant._tag != N)
            {
                throw invalid_variant_get(N, variant._tag);
            }

            return _detail::_dereference(*reinterpret_cast<tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage));
        }

        template<std::size_t N, typename CRTP, typename... Ts>
        const _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> & get(const _detail::_variant<CRTP, Ts...> & variant)
        {
            if (variant._tag != N)
            {
                throw invalid_variant_get(N, variant._tag);
            }

            return _detail::_dereference(*reinterpret_cast<const tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage));
        }

        template<std::size_t N, typename CRTP, typename... Ts>
        _detail::_dereference_wrapper_t<tpl::nth<tpl::vector<Ts...>, N>> get(_detail::_variant<CRTP, Ts...> && variant)
        {
            if (variant._tag != N)
            {
                throw invalid_variant_get(N, variant._tag);
            }

            return _detail::_dereference(std::move(*reinterpret_cast<tpl::nth<tpl::vector<Ts...>, N> *>(&variant._storage)));
        }
    }

    template<typename T>
    struct replace_reference
    {
        using type = T;
    };

    template<typename T>
    struct replace_reference<T &>
    {
        using type = std::reference_wrapper<T>;
    };

    template<typename T>
    using replace_reference_t = typename replace_reference<T>::type;

    using _detail::get;

    template<typename... Ts>
    class variant : public _detail::_variant<variant<Ts...>, replace_reference_t<Ts>...>
    {
        using _base = _detail::_variant<variant, replace_reference_t<Ts>...>;

    public:
        using _base::_base;
    };

    template<typename T, typename CRTP, typename... Ts>
    T get(_detail::_variant<CRTP, Ts...> && variant)
    {
        return get<tpl::index_of<tpl::vector<Ts...>, T>::value>(std::move(variant));
    }

    template<typename T, typename CRTP, typename... Ts>
    const T & get(const _detail::_variant<CRTP, Ts...> & variant)
    {
        return get<tpl::index_of<tpl::vector<Ts...>, T>::value>(variant);
    }

    template<typename T, typename CRTP, typename... Ts>
    T & get(_detail::_variant<CRTP, Ts...> & variant)
    {
        return get<tpl::index_of<tpl::vector<Ts...>, T>::value>(variant);
    }

    template<typename CRTP, typename... Ts, typename F>
    auto fmap(_detail::_variant<CRTP, Ts...> && var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(invoke(std::forward<F>(f), std::declval<_detail::_dereference_wrapper_t<Ts> &&>()))...>, variant>;
        using visitor_type = result_type (*)(_detail::_variant<CRTP, Ts...> &&, F &&);

        // hack for GCC dumbness
        auto generator = [](auto type) {
            using T = typename decltype(type)::type;
            return [](_detail::_variant<CRTP, Ts...> && v, F && f) -> result_type { return invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, T>::value>(std::move(v))); };
        };

        static visitor_type visitors[] = {
            generator(id<Ts>())...
        };

        auto index = var.index();
        return visitors[index](std::move(var), std::forward<F>(f));
    }

    template<typename CRTP, typename... Ts, typename F>
    auto fmap(const _detail::_variant<CRTP, Ts...> & var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(invoke(std::forward<F>(f), std::declval<const _detail::_dereference_wrapper_t<Ts> &>()))...>, variant>;
        using visitor_type = result_type (*)(const _detail::_variant<CRTP, Ts...> &, F &&);

        // hack for GCC dumbness
        auto generator = [](auto type) {
            using T = typename decltype(type)::type;
            return [](const _detail::_variant<CRTP, Ts...> & v, F && f) -> result_type { return invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, T>::value>(v)); };
        };

        static visitor_type visitors[] = {
            generator(id<Ts>())...
        };

        auto index = var.index();
        return visitors[index](var, std::forward<F>(f));
    }

    template<typename CRTP, typename... Ts, typename F>
    auto fmap(_detail::_variant<CRTP, Ts...> & var, F && f)
    {
        using result_type = tpl::rebind<tpl::unique<decltype(invoke(std::forward<F>(f), std::declval<_detail::_dereference_wrapper_t<Ts> &>()))...>, variant>;
        using visitor_type = result_type (*)(_detail::_variant<CRTP, Ts...> &, F &&);

        // hack for GCC dumbness
        auto generator = [](auto type) {
            using T = typename decltype(type)::type;
            return [](_detail::_variant<CRTP, Ts...> & v, F && f) -> result_type { return invoke(std::forward<F>(f), get<tpl::index_of<tpl::vector<Ts...>, T>::value>(v)); };
        };

        static visitor_type visitors[] = {
            generator(id<Ts>())...
        };

        auto index = var.index();
        return visitors[index](var, std::forward<F>(f));
    }

    template<typename F, typename CRTP, typename... Ts,
        typename = decltype(get<0>(fmap(std::declval<const _detail::_variant<CRTP, Ts...> &>(), std::declval<F>())))>
    auto mbind(const _detail::_variant<CRTP, Ts...> & v, F && f)
    {
        using return_type = tpl::rebind<
            tpl::rebind<
                tpl::rebind<
                    tpl::map<
                        tpl::unbind<decltype(fmap(v, std::forward<F>(f)))>,
                        tpl::unbind
                    >,
                    tpl::concat
                >,
                tpl::unique
            >,
            variant
        >;

        return get<0>(fmap(v, [&](auto && value) -> return_type {
            return invoke(std::forward<F>(f), std::forward<decltype(value)>(value));
        }));
    }

    template<typename F, typename CRTP, typename... Ts,
        typename = decltype(get<0>(fmap(std::declval<_detail::_variant<CRTP, Ts...> &>(), std::declval<F>())))>
    auto mbind(_detail::_variant<CRTP, Ts...> & v, F && f)
    {
        using return_type = tpl::rebind<
            tpl::rebind<
                tpl::rebind<
                    tpl::map<
                        tpl::unbind<decltype(fmap(v, std::forward<F>(f)))>,
                        tpl::unbind
                    >,
                    tpl::concat
                >,
                tpl::unique
            >,
            variant
        >;

        return get<0>(fmap(v, [&](auto && value) -> return_type {
            return invoke(std::forward<F>(f), std::forward<decltype(value)>(value));
        }));
    }

    template<typename F, typename CRTP, typename... Ts,
        typename = decltype(get<0>(fmap(std::declval<_detail::_variant<CRTP, Ts...>>(), std::declval<F>())))>
    auto mbind(_detail::_variant<CRTP, Ts...> && v, F && f)
    {
        using return_type = tpl::rebind<
            tpl::rebind<
                tpl::rebind<
                    tpl::map<
                        tpl::unbind<decltype(fmap(std::move(v), std::forward<F>(f)))>,
                        tpl::unbind
                    >,
                    tpl::concat
                >,
                tpl::unique
            >,
            variant
        >;

        return get<0>(fmap(std::move(v), [&](auto && value) -> return_type {
            return invoke(std::forward<F>(f), std::forward<decltype(value)>(value));
        }));
    }

    namespace _detail
    {
        template<typename T>
        auto _make_variant(T && t)
        {
            return variant<T>{ std::forward<T>(t) };
        };

        template<typename F, std::size_t... Is, typename... TupleTs>
        auto _visit(F && f, std::index_sequence<Is...>, std::tuple<TupleTs...> tuple)
        {
            return _make_variant(std::forward<F>(f)(std::forward<TupleTs>(std::get<Is>(tuple))...));
        }

        template<typename F, typename... TupleTs>
        auto _visit(F && f, std::tuple<TupleTs...> tuple)
        {
            return _visit(std::forward<F>(f), std::make_index_sequence<sizeof...(TupleTs)>(), std::move(tuple));
        }

        template<typename F, typename CRTP, typename... TupleTs, typename... Ts, typename... Variants>
        auto _visit(F && f, std::tuple<TupleTs...> tuple, const _detail::_variant<CRTP, Ts...> & head, Variants &&... tail)
        {
            return mbind(head, [&](auto && elem) {
                return _visit(std::forward<F>(f), std::tuple_cat(std::move(tuple), std::make_tuple(std::forward<decltype(elem)>(elem))), std::forward<Variants>(tail)...);
            });
        }

        template<typename F, typename CRTP,  typename... TupleTs, typename... Ts, typename... Variants>
        auto _visit(F && f, std::tuple<TupleTs...> tuple, _detail::_variant<CRTP, Ts...> & head, Variants &&... tail)
        {
            return mbind(head, [&](auto && elem) {
                return _visit(std::forward<F>(f), std::tuple_cat(std::move(tuple), std::make_tuple(std::forward<decltype(elem)>(elem))), std::forward<Variants>(tail)...);
            });
        }

        template<typename F, typename CRTP, typename... TupleTs, typename... Ts, typename... Variants>
        auto _visit(F && f, std::tuple<TupleTs...> tuple, _detail::_variant<CRTP, Ts...> && head, Variants &&... tail)
        {
            return mbind(std::move(head), [&](auto && elem) {
                return _visit(std::forward<F>(f), std::tuple_cat(std::move(tuple), std::make_tuple(std::forward<decltype(elem)>(elem))), std::forward<Variants>(tail)...);
            });
        }
    }

    template<typename F, typename... Variants>
    auto visit(F && f, Variants &&... variants)
    {
        return _detail::_visit(std::forward<F>(f), std::make_tuple(), std::forward<Variants>(variants)...);
    }

    template<typename CRTP, typename... Ts>
    bool operator==(const _detail::_variant<CRTP, Ts...> & lhs, const _detail::_variant<CRTP, Ts...> & rhs)
    {
        using comparator_type = bool (*)(const _detail::_variant<CRTP, Ts...> &, const _detail::_variant<CRTP, Ts...> &);

        // hack for GCC dumbness
        auto generator = [](auto type) {
            using T = typename decltype(type)::type;
            return [](const _detail::_variant<CRTP, Ts...> & lhs, const _detail::_variant<CRTP, Ts...> & rhs) {
                return get<tpl::index_of<tpl::vector<Ts...>, T>::value>(lhs) == get<tpl::index_of<tpl::vector<Ts...>, T>::value>(rhs);
            };
        };

        static comparator_type comparators[] = {
            generator(id<Ts>())...
        };

        return lhs.index() == rhs.index() && comparators[lhs.index()](lhs, rhs);
    }

    template<typename CRTP, typename... Ts>
    bool operator!=(const _detail::_variant<CRTP, Ts...> & lhs, const _detail::_variant<CRTP, Ts...> & rhs)
    {
        return !(lhs == rhs);
    }

    template<typename CRTP, typename... Ts>
    bool operator<(const _detail::_variant<CRTP, Ts...> & lhs, const _detail::_variant<CRTP, Ts...> & rhs)
    {
        using comparator_type = bool (*)(const _detail::_variant<CRTP, Ts...> &, const _detail::_variant<CRTP, Ts...> &);

        // hack for GCC dumbness
        auto generator = [](auto type) {
            using T = typename decltype(type)::type;
            return [](const _detail::_variant<CRTP, Ts...> & lhs, const _detail::_variant<CRTP, Ts...> & rhs) {
                return get<tpl::index_of<tpl::vector<Ts...>, T>::value>(lhs) < get<tpl::index_of<tpl::vector<Ts...>, T>::value>(rhs);
            };
        };

        static comparator_type comparators[] = {
            generator(id<Ts>())...
        };

        return lhs.index() < rhs.index() || (lhs.index() == rhs.index() && comparators[lhs.index()](lhs, rhs));
    }

    struct recursive_variant_tag{};
    using rvt = recursive_variant_tag;

    namespace _detail
    {
        template<typename... Ts>
        struct _make_recursive_variant
        {
            class type;

            class type : public _variant<type, replace_reference_t<tpl::replace<Ts, recursive_variant_tag, recursive_wrapper<type>>>...>
            {
                using _base = _variant<type, replace_reference_t<tpl::replace<Ts, recursive_variant_tag, recursive_wrapper<type>>>...>;

            public:
                using _base::_base;
                using _base::operator=;
            };
        };
    }

    template<typename... Ts>
    using recursive_variant = typename _detail::_make_recursive_variant<Ts...>::type;

    template<std::size_t N, typename Variant>
    decltype(auto) get(recursive_wrapper<Variant> & variant)
    {
        return get<N>(*variant);
    }

    template<std::size_t N, typename Variant>
    decltype(auto) get(const recursive_wrapper<Variant> & variant)
    {
        return get<N>(*variant);
    }

    template<std::size_t N, typename Variant>
    decltype(auto) get(recursive_wrapper<Variant> && variant)
    {
        return get<N>(std::move(*variant));
    }

    template<typename Variant, typename F>
    decltype(auto) fmap(recursive_wrapper<Variant> & variant, F && f)
    {
        return fmap(*variant, std::forward<F>(f));
    }

    template<typename Variant, typename F>
    decltype(auto) fmap(const recursive_wrapper<Variant> & variant, F && f)
    {
        return fmap(*variant, std::forward<F>(f));
    }

    template<typename Variant, typename F>
    decltype(auto) fmap(recursive_wrapper<Variant> && variant, F && f)
    {
        return fmap(std::move(*variant), std::forward<F>(f));
    }

    template<typename T, typename U>
    bool operator==(const recursive_wrapper<T> & lhs, const U & rhs)
    {
        return *lhs == rhs;
    }

    template<typename T, typename U>
    bool operator==(const T & lhs, const recursive_wrapper<U> & rhs)
    {
        return lhs == *rhs;
    }

    template<typename T, typename U>
    bool operator!=(const recursive_wrapper<T> & lhs, const U & rhs)
    {
        return *lhs != rhs;
    }

    template<typename T, typename U>
    bool operator!=(const T & lhs, const recursive_wrapper<U> & rhs)
    {
        return lhs != *rhs;
    }

    template<typename T, typename U>
    bool operator<(const recursive_wrapper<T> & lhs, const U & rhs)
    {
        return *lhs < rhs;
    }

    template<typename T, typename U>
    bool operator<(const T & lhs, const recursive_wrapper<U> & rhs)
    {
        return lhs < *rhs;
    }
}}

