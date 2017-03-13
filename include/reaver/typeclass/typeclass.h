/**
 * Reaver Library Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#include "../function_traits.h"

namespace reaver
{
inline namespace _v1
{
    template<typename Typeclass, typename T, typename = void>
    struct typeclass_trait;

    template<typename Typeclass, typename T>
    struct typeclass_trait<Typeclass, T, typename std::enable_if<std::is_class<T>::value>::type>
    {
        using type = typename T::template instance<Typeclass, T>;
    };

    template<typename Typeclass, typename T>
    struct typeclass_trait<Typeclass, T, typename std::enable_if<!std::is_class<T>::value>::type>
    {
        using type = typename Typeclass::template instance<Typeclass, T>;
    };

    template<typename Typeclass, typename T>
    using tc_instance = typename typeclass_trait<Typeclass, T>::type;

#define TYPECLASS_INTERNAL_REAVER_NAMESPACE ::reaver

#define INSTANCE_TEMPLATE_HELPER                                                                                                                               \
    template<typename Typeclass, typename Class>                                                                                                               \
    struct typeclass_instance_helper

#define TYPECLASS_INSTANCE(...)                                                                                                                                \
    template<typename _typeclass, __VA_ARGS__>                                                                                                                 \
    struct instance

#define TYPECLASS_INSTANCE_TEMPLATE(template_decl, template_args, class)                                                                                       \
    template<typename _typeclass, ONLY template_decl>                                                                                                          \
    struct instance : typeclass_instance_helper<_typeclass, ONLY template_args>                                                                                \
    {                                                                                                                                                          \
    }

#define DEFAULT_INSTANCE(typeclass, ...)                                                                                                                       \
    template<typename __VA_ARGS__>                                                                                                                             \
    struct typeclass::instance<typeclass, __VA_ARGS__>

#define DEFAULT_INSTANCE_TEMPLATE(template_decl, template_args, typeclass, ...)                                                                                \
    INSTANCE_TEMPLATE_HELPER;                                                                                                                                  \
    template<ONLY template_decl>                                                                                                                               \
    template<typename __VA_ARGS__>                                                                                                                             \
    struct typeclass<ONLY template_args>::instance<typeclass<ONLY template_args>, __VA_ARGS__>

#define SPECIAL_INSTANCE(typeclass, class)                                                                                                                     \
    template<>                                                                                                                                                 \
    struct typeclass::instance<typeclass, class>

#define INSTANCE(typeclass, class)                                                                                                                             \
    template<>                                                                                                                                                 \
    struct class ::instance<typeclass, class> : public typeclass::instance<typeclass, class>

#define INSTANCE_TEMPLATE(template_decl, template_args, typeclass, class)                                                                                      \
    INSTANCE_TEMPLATE_HELPER;                                                                                                                                  \
    template<ONLY template_decl>                                                                                                                               \
    struct typeclass_instance_helper<typeclass<ONLY template_args>, class>                                                                                     \
        : public typeclass<ONLY template_args>::template instance<typeclass<ONLY template_args>, class>

    template<typename T>
    struct virtual_dtor
    {
        virtual ~virtual_dtor() = default;
    };

    template<typename T>
    struct typeclass_provide_data_member
    {
        T t = {};
    };

#define CONCAT(X, Y) X##Y
#define CONCAT3(X, Y, Z) X##Y##Z

#define TYPECLASS_PREPARE_MIXINS(x, typeclass_name, memfn_name)                                                                                                \
    template<typename ReturnType, typename... Args>                                                                                                            \
    struct CONCAT3(typeclass_name, _typeclass_base_provide_, memfn_name)                                                                                       \
        : protected TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<class typeclass_base_provide_>                                                           \
    {                                                                                                                                                          \
        virtual ReturnType memfn_name(Args...) = 0;                                                                                                            \
    };                                                                                                                                                         \
                                                                                                                                                               \
    template<typename Typeclass, typename T, typename ReturnType, typename... Args>                                                                            \
    struct CONCAT3(typeclass_name, _typeclass_impl_provide_, memfn_name)                                                                                       \
        : virtual CONCAT3(typeclass_name, _typeclass_base_provide_, memfn_name)<ReturnType, Args...>,                                                          \
          protected virtual TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<T>                                                              \
    {                                                                                                                                                          \
        CONCAT3(typeclass_name, _typeclass_impl_provide_, memfn_name)                                                                                          \
        (const T & t) : TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<T>{ t }                                                             \
        {                                                                                                                                                      \
        }                                                                                                                                                      \
                                                                                                                                                               \
        virtual ReturnType memfn_name(Args... args) override                                                                                                   \
        {                                                                                                                                                      \
            return Typeclass::memfn_name(TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<T>::t, std::forward<Args>(args)...);               \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
                                                                                                                                                               \
    template<typename Typeclass, typename ReturnType, typename... Args>                                                                                        \
    struct CONCAT3(typeclass_name, _typeclass_provide_, memfn_name) : protected TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<class typeclass_provide_>    \
    {                                                                                                                                                          \
        template<typename T>                                                                                                                                   \
        static ReturnType memfn_name(T & t, Args... args)                                                                                                      \
        {                                                                                                                                                      \
            return TYPECLASS_INTERNAL_REAVER_NAMESPACE::tc_instance<Typeclass, T>::memfn_name(t, std::forward<Args>(args)...);                                 \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
                                                                                                                                                               \
    /* workaround for Clang <4.0 */                                                                                                                            \
    template<typename... Ts>                                                                                                                                   \
    using CONCAT3(typeclass_name, _typeclass_provide_alias_, memfn_name) = CONCAT3(typeclass_name, _typeclass_provide_, memfn_name)<Ts...>;                    \
                                                                                                                                                               \
    template<typename Typeclass, typename ReturnType, typename... Args>                                                                                        \
    struct CONCAT3(typeclass_name, _typeclass_erased_provide_, memfn_name)                                                                                     \
        : protected TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<class typeclass_erased_provide_>,                                                        \
          protected virtual TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typename Typeclass::_base>>                     \
    {                                                                                                                                                          \
        ReturnType memfn_name(Args... args)                                                                                                                    \
        {                                                                                                                                                      \
            return TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typename Typeclass::_base>>::t->memfn_name(              \
                std::forward<Args>(args)...);                                                                                                                  \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
                                                                                                                                                               \
    /* also a workaround for Clang <4.0 */                                                                                                                     \
    template<typename... Ts>                                                                                                                                   \
    using CONCAT3(typeclass_name, _typeclass_erased_provide_alias_, memfn_name) = CONCAT3(typeclass_name, _typeclass_erased_provide_, memfn_name)<Ts...>;      \
                                                                                                                                                               \
    template<typename Typeclass, typename ReturnType, typename... Args>                                                                                        \
    struct CONCAT3(typeclass_name, _typeclass_erased_instance_provide_, memfn_name)                                                                            \
    {                                                                                                                                                          \
        static ReturnType memfn_name(typename Typeclass::erased_ref & t, Args... args)                                                                         \
        {                                                                                                                                                      \
            return t.memfn_name(std::forward<Args>(args)...);                                                                                                  \
        }                                                                                                                                                      \
    };

    template<typename T>
    struct strip_arguments;

    template<template<typename...> typename Template, typename... Args>
    struct strip_arguments<Template<Args...>>
    {
        template<typename... Ts>
        using type = Template<Ts...>;
    };

#define NOOP(...)
#define ONLY(...) __VA_ARGS__
#define FIRST(...) __VA_ARGS__ NOOP
#define SECOND(...) ONLY

#define TYPECLASS_FRIEND_MIXIN(x, typeclass_name, memfn_name)                                                                                                  \
    template<typename Typeclass, typename ReturnType, typename... Args>                                                                                        \
    friend struct CONCAT3(typeclass_name, _typeclass_erased_provide_, memfn_name);

#define TYPECLASS_TYPE_BASE_CLASS(x, typeclass_info, memfn_name) TYPECLASS_TYPE_BASE_CLASS_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_TYPE_BASE_CLASS_IMPL(typeclass_name, template_args, memfn_name)                                                                              \
private                                                                                                                                                        \
    TYPECLASS_INTERNAL_REAVER_NAMESPACE::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                                       \
        CONCAT3(typeclass_name, _typeclass_provide_, memfn_name),                                                                                              \
        typeclass_name template_args>,

#define TYPECLASS_TYPE_BASE_USING(x, typeclass_info, memfn_name) TYPECLASS_TYPE_BASE_USING_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_TYPE_BASE_USING_IMPL(typeclass_name, template_args, memfn_name)                                                                              \
    using TYPECLASS_INTERNAL_REAVER_NAMESPACE::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                                 \
        CONCAT3(typeclass_name, _typeclass_provide_alias_, memfn_name),                                                                                        \
        typeclass_name template_args>::memfn_name;

#define TYPECLASS_BASE_BASE_CLASS(x, typeclass_info, memfn_name) TYPECLASS_BASE_BASE_CLASS_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_BASE_BASE_CLASS_IMPL(typeclass_name, template_args, memfn_name)                                                                              \
public                                                                                                                                                         \
    virtual TYPECLASS_INTERNAL_REAVER_NAMESPACE::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                               \
        CONCAT3(typeclass_name, _typeclass_base_provide_, memfn_name)>,

#define TYPECLASS_IMPL_BASE_CLASS(x, typeclass_info, memfn_name) TYPECLASS_IMPL_BASE_CLASS_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_IMPL_BASE_CLASS_IMPL(typeclass_name, template_args, memfn_name)                                                                              \
public                                                                                                                                                         \
    TYPECLASS_INTERNAL_REAVER_NAMESPACE::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                                       \
        CONCAT3(typeclass_name, _typeclass_impl_provide_, memfn_name),                                                                                         \
        typeclass_name template_args,                                                                                                                          \
        TYPECLASS_T>,

#define TYPECLASS_IMPL_CTOR_INIT(x, typeclass_info, memfn_name) TYPECLASS_IMPL_CTOR_INIT_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_IMPL_CTOR_INIT_IMPL(typeclass_name, template_args, memfn_name)                                                                               \
    TYPECLASS_INTERNAL_REAVER_NAMESPACE::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                                       \
        CONCAT3(typeclass_name, _typeclass_impl_provide_, memfn_name),                                                                                         \
        typeclass_name template_args,                                                                                                                          \
        TYPECLASS_T>{ t },

#define TYPECLASS_ERASED_BASE_CLASS(x, typeclass_info, memfn_name) TYPECLASS_ERASED_BASE_CLASS_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_ERASED_BASE_CLASS_IMPL(typeclass_name, template_args, memfn_name)                                                                            \
protected                                                                                                                                                      \
    TYPECLASS_INTERNAL_REAVER_NAMESPACE::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                                       \
        CONCAT3(typeclass_name, _typeclass_erased_provide_, memfn_name),                                                                                       \
        typeclass_name template_args>,

#define TYPECLASS_ERASED_USING(x, typeclass_info, memfn_name) TYPECLASS_ERASED_USING_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_ERASED_USING_IMPL(typeclass_name, template_args, memfn_name)                                                                                 \
    using TYPECLASS_INTERNAL_REAVER_NAMESPACE::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                                 \
        CONCAT3(typeclass_name, _typeclass_erased_provide_alias_, memfn_name),                                                                                 \
        typeclass_name template_args>::memfn_name;

#define DEFINE_TYPECLASS_SEQ(template_decl, template_args, typeclass_name, member_list)                                                                        \
    BOOST_PP_SEQ_FOR_EACH(TYPECLASS_PREPARE_MIXINS, typeclass_name, member_list)                                                                               \
                                                                                                                                                               \
    template_decl struct typeclass_name : BOOST_PP_SEQ_FOR_EACH(TYPECLASS_TYPE_BASE_CLASS, (typeclass_name)(template_args), member_list)                       \
                                              TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<typeclass_name template_args>                                  \
    {                                                                                                                                                          \
        TYPECLASS_INSTANCE(typename TYPECLASS_INSTANCE_ARGUMENT);                                                                                              \
        class erased_ref;                                                                                                                                      \
        class erased;                                                                                                                                          \
                                                                                                                                                               \
        BOOST_PP_SEQ_FOR_EACH(TYPECLASS_FRIEND_MIXIN, typeclass_name, member_list)                                                                             \
        BOOST_PP_SEQ_FOR_EACH(TYPECLASS_TYPE_BASE_USING, (typeclass_name)(template_args), member_list)                                                         \
                                                                                                                                                               \
        struct _base : BOOST_PP_SEQ_FOR_EACH(TYPECLASS_BASE_BASE_CLASS,                                                                                        \
                           (typeclass_name)(template_args),                                                                                                    \
                           member_list) TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<_base>                                                               \
        {                                                                                                                                                      \
            virtual ~_base() = default;                                                                                                                        \
            virtual std::unique_ptr<_base> clone() const = 0;                                                                                                  \
        };                                                                                                                                                     \
                                                                                                                                                               \
    private:                                                                                                                                                   \
        template<typename TYPECLASS_T>                                                                                                                         \
        struct _impl : _base,                                                                                                                                  \
                       BOOST_PP_SEQ_FOR_EACH(TYPECLASS_IMPL_BASE_CLASS,                                                                                        \
                           (typeclass_name)(template_args),                                                                                                    \
                           member_list) TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<_impl<TYPECLASS_T>>                                                  \
        {                                                                                                                                                      \
            _impl(TYPECLASS_T t)                                                                                                                               \
                : TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<TYPECLASS_T>{ std::forward<TYPECLASS_T>(t) },                             \
                  BOOST_PP_SEQ_FOR_EACH(TYPECLASS_IMPL_CTOR_INIT,                                                                                              \
                      (typeclass_name)(template_args),                                                                                                         \
                      member_list) TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<_impl<TYPECLASS_T>>{}                                                     \
            {                                                                                                                                                  \
            }                                                                                                                                                  \
                                                                                                                                                               \
            virtual std::unique_ptr<_base> clone() const override                                                                                              \
            {                                                                                                                                                  \
                return std::make_unique<_impl>(TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<TYPECLASS_T>::t);                            \
            }                                                                                                                                                  \
        };                                                                                                                                                     \
    };                                                                                                                                                         \
                                                                                                                                                               \
    template_decl class typeclass_name template_args::erased_ref                                                                                               \
        : public typeclass_name,                                                                                                                               \
          BOOST_PP_SEQ_FOR_EACH(TYPECLASS_ERASED_BASE_CLASS, (typeclass_name)(template_args), member_list)                                                     \
              TYPECLASS_INTERNAL_REAVER_NAMESPACE::virtual_dtor<typeclass_name::erased>                                                                        \
    {                                                                                                                                                          \
    public:                                                                                                                                                    \
        template<typename TYPECLASS_T>                                                                                                                         \
        erased_ref(std::reference_wrapper<TYPECLASS_T> t)                                                                                                      \
            : TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typeclass_name::_base>>{                                      \
                  std::make_unique<_impl<TYPECLASS_T &>>(t.get())                                                                                              \
              }                                                                                                                                                \
        {                                                                                                                                                      \
        }                                                                                                                                                      \
                                                                                                                                                               \
        template<typename TYPECLASS_T>                                                                                                                         \
        erased_ref(TYPECLASS_T & t)                                                                                                                            \
            : TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typeclass_name::_base>>{                                      \
                  std::make_unique<_impl<TYPECLASS_T &>>(t)                                                                                                    \
              }                                                                                                                                                \
        {                                                                                                                                                      \
        }                                                                                                                                                      \
                                                                                                                                                               \
        erased_ref(const erased_ref & other)                                                                                                                   \
            : TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typeclass_name::_base>>{                                      \
                  other.TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typeclass_name::_base>>::t->clone()                 \
              }                                                                                                                                                \
        {                                                                                                                                                      \
        }                                                                                                                                                      \
                                                                                                                                                               \
    protected:                                                                                                                                                 \
        erased_ref() = default;                                                                                                                                \
                                                                                                                                                               \
    public:                                                                                                                                                    \
        BOOST_PP_SEQ_FOR_EACH(TYPECLASS_ERASED_USING, (typeclass_name)(template_args), member_list)                                                            \
    };                                                                                                                                                         \
                                                                                                                                                               \
    template_decl class typeclass_name template_args::erased : public typeclass_name::erased_ref                                                               \
    {                                                                                                                                                          \
    public:                                                                                                                                                    \
        template<typename TYPECLASS_T>                                                                                                                         \
        erased(TYPECLASS_T && t)                                                                                                                               \
            : TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typeclass_name::_base>>{                                      \
                  std::make_unique<_impl<std::remove_reference_t<TYPECLASS_T>>>(std::forward<TYPECLASS_T>(t))                                                  \
              }                                                                                                                                                \
        {                                                                                                                                                      \
        }                                                                                                                                                      \
                                                                                                                                                               \
        template<typename TYPECLASS_T>                                                                                                                         \
        erased(std::reference_wrapper<TYPECLASS_T> t)                                                                                                          \
            : TYPECLASS_INTERNAL_REAVER_NAMESPACE::typeclass_provide_data_member<std::unique_ptr<typeclass_name::_base>>{                                      \
                  std::make_unique<_impl<TYPECLASS_T &>>(t.get())                                                                                              \
              }                                                                                                                                                \
        {                                                                                                                                                      \
        }                                                                                                                                                      \
    };

#define TYPECLASS_ERASED_INSTANCE_BASE(x, typeclass_name, memfn_name)                                                                                          \
public                                                                                                                                                         \
    ::reaver::                                                                                                                                                 \
        explode<CONCAT(typeclass_name, _definition)::memfn_name, CONCAT3(typeclass_name, _typeclass_erased_instance_provide_, memfn_name), typeclass_name>,

#define TYPECLASS_ERASED_INSTANCE_TEMPLATE_BASE(x, typeclass_info, memfn_name)                                                                                 \
    TYPECLASS_ERASED_INSTANCE_TEMPLATE_BASE_IMPL(FIRST typeclass_info, SECOND typeclass_info, memfn_name)
#define TYPECLASS_ERASED_INSTANCE_TEMPLATE_BASE_IMPL(typeclass_name, template_args, memfn_name)                                                                \
public                                                                                                                                                         \
    ::reaver::explode<typename CONCAT(typeclass_name, _definition) template_args::memfn_name,                                                                  \
        CONCAT3(typeclass_name, _typeclass_erased_instance_provide_, memfn_name),                                                                              \
        typeclass_name template_args>,

#define DEFINE_TYPECLASS_IMPL(typeclass_name, member_list)                                                                                                     \
    DEFINE_TYPECLASS_SEQ(, , typeclass_name, member_list)                                                                                                      \
    template<>                                                                                                                                                 \
    struct typeclass_name::instance<typeclass_name, typeclass_name::erased_ref>                                                                                \
        : BOOST_PP_SEQ_FOR_EACH(TYPECLASS_ERASED_INSTANCE_BASE, typeclass_name, member_list)::reaver::unit                                                     \
    {                                                                                                                                                          \
    };                                                                                                                                                         \
    template<>                                                                                                                                                 \
    struct typeclass_name::instance<typeclass_name, typeclass_name::erased> : typeclass_name::instance<typeclass_name, typeclass_name::erased_ref>             \
    {                                                                                                                                                          \
    }

#define DEFINE_TYPECLASS(typeclass_name, ...) DEFINE_TYPECLASS_IMPL(typeclass_name, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define TYPECLASS_TEMPLATE_DECL(X) template<X>
#define TYPECLASS_TEMPLATE_ARGS(X) <X>

#define DEFINE_TYPECLASS_TEMPLATE_IMPL(template_decl, template_args, typeclass_name, member_list)                                                              \
    DEFINE_TYPECLASS_SEQ(TYPECLASS_TEMPLATE_DECL template_decl, TYPECLASS_TEMPLATE_ARGS template_args, typeclass_name, member_list)                            \
    TYPECLASS_TEMPLATE_DECL template_decl struct typeclass_instance_helper<typeclass_name TYPECLASS_TEMPLATE_ARGS template_args,                               \
        typename typeclass_name TYPECLASS_TEMPLATE_ARGS template_args::erased_ref>                                                                             \
        : BOOST_PP_SEQ_FOR_EACH(TYPECLASS_ERASED_INSTANCE_TEMPLATE_BASE, (typeclass_name)(TYPECLASS_TEMPLATE_ARGS template_args), member_list)::reaver::unit   \
    {                                                                                                                                                          \
    };                                                                                                                                                         \
    TYPECLASS_TEMPLATE_DECL template_decl struct typeclass_instance_helper<typeclass_name TYPECLASS_TEMPLATE_ARGS template_args,                               \
        typename typeclass_name TYPECLASS_TEMPLATE_ARGS template_args::erased>                                                                                 \
        : typeclass_instance_helper<typeclass_name TYPECLASS_TEMPLATE_ARGS template_args,                                                                      \
              typename typeclass_name TYPECLASS_TEMPLATE_ARGS template_args::erased_ref>                                                                       \
    {                                                                                                                                                          \
    };

#define DEFINE_TYPECLASS_TEMPLATE(template_decl, template_args, typeclass_name, ...)                                                                           \
    INSTANCE_TEMPLATE_HELPER;                                                                                                                                  \
    DEFINE_TYPECLASS_TEMPLATE_IMPL(template_decl, template_args, typeclass_name, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
}
}
