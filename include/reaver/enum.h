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

#pragma once

#include <string>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <unordered_map>
#include <vector>

#ifndef REAVER_ROOT_NAMESPACE
# define REAVER_ROOT_NAMESPACE ::reaver
#endif

namespace reaver { inline namespace _v1
{
    namespace _detail
    {
        template<typename Enum, typename Context>
        struct _enum_wrapper
        {
            static Enum next;

            _enum_wrapper() : value{ next }
            {
                next = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(next) + 1);
            }

            _enum_wrapper(std::underlying_type_t<Enum> current) : _enum_wrapper{ static_cast<Enum>(current) }
            {
            }

            _enum_wrapper(Enum current) : value{ current }
            {
                next = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(current) + 1);
            }

            operator Enum() const
            {
                return value;
            }

            template<typename T>
            _enum_wrapper & operator=(T)
            {
                return *this;
            }

            Enum value;
        };

        template<typename Enum, typename Context>
        Enum _enum_wrapper<Enum, Context>::next = {};
    }
}}

#define reflected_enum(name, ...) \
    template<typename T> const std::vector<T> & enum_values() ; \
    template<typename T> const std::vector<std::string> & enum_strings(); \
    template<typename T> T from_string(const std::string &); \
    enum class name { __VA_ARGS__ }; \
    \
    template<> \
    inline const std::vector<name> & enum_values<name>() \
    { \
        static auto values = []{ \
            REAVER_ROOT_NAMESPACE::_detail::_enum_wrapper<name, class context> __VA_ARGS__; \
            std::vector<name> vec{ __VA_ARGS__ }; \
            return vec; \
        }(); \
        return values; \
    } \
    \
    template<> \
    inline const std::vector<std::string> & enum_strings<name>() \
    { \
        static auto strings = []() { \
            std::string str{ #__VA_ARGS__ }; \
            str.erase(std::remove_if(str.begin(), str.end(), [](auto arg){ return std::isspace(arg); }), str.end()); \
            std::istringstream stream{ std::move(str) }; \
            std::vector<std::string> strings; \
            std::string buffer; \
            while (std::getline(stream, buffer, ',')) \
            { \
                strings.push_back(buffer.substr(0, buffer.find('='))); \
            } \
            return strings; \
        }(); \
        return strings; \
    } \
    \
    template<> \
    [[gnu::unused]] inline name from_string<name>(const std::string & arg) \
    { \
        static const auto mapping = [&]() { \
            auto it = enum_values<name>().begin(); \
            std::unordered_map<std::string, name> map; \
            for (auto && elem : enum_strings<name>()) \
            { \
                map.emplace(elem, *it++); \
            } \
            return map; \
        }(); \
        return mapping.at(arg); \
    } \
    \
    [[gnu::unused]] inline std::string to_string(name value) \
    { \
        static const auto mapping = [&]() { \
            auto it = enum_values<name>().begin(); \
            std::unordered_map<std::underlying_type_t<name>, std::string> map; \
            for (auto && elem : enum_strings<name>()) \
            { \
                map.emplace(static_cast<std::underlying_type_t<name>>(*it++), elem); \
            } \
            return map; \
        }(); \
        return mapping.at(static_cast<std::underlying_type_t<name>>(value)); \
    }

