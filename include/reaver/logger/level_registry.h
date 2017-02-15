/**
 * Reaver Library Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <map>
#include <vector>

#include "../configuration.h"
#include "../style.h"
#include "streamable.h"

namespace reaver
{
namespace logger
{
    inline namespace _v1
    {
        enum class base_level
        {
            trace,
            debug,
            note,
            info,
            success,
            warning,
            error,
            fatal,
            crash,
            always
        };

        struct hasher
        {
            auto operator()(const base_level & l) const
            {
                return std::hash<std::underlying_type_t<base_level>>()(static_cast<std::underlying_type_t<base_level>>(l));
            }
        };

        struct level_type
        {
            using type = std::vector<streamable>;
        };

#define REAVER_LEVEL_TYPE(x)                                                                                                                                   \
    static constexpr struct x##_type : level_type                                                                                                              \
    {                                                                                                                                                          \
        constexpr static base_level level = base_level::x;                                                                                                     \
        constexpr operator base_level() const                                                                                                                  \
        {                                                                                                                                                      \
            return level;                                                                                                                                      \
        }                                                                                                                                                      \
    } x                                                                                                                                                        \
    {                                                                                                                                                          \
    }

        REAVER_LEVEL_TYPE(always);
        REAVER_LEVEL_TYPE(trace);
        REAVER_LEVEL_TYPE(debug);
        REAVER_LEVEL_TYPE(note);
        REAVER_LEVEL_TYPE(info);
        REAVER_LEVEL_TYPE(success);
        REAVER_LEVEL_TYPE(warning);
        REAVER_LEVEL_TYPE(error);
        REAVER_LEVEL_TYPE(fatal);
        REAVER_LEVEL_TYPE(crash);

#undef REAVER_LEVEL_TYPE

        template<typename T>
        struct is_logger_level : std::is_base_of<level_type, T>
        {
        };

        class invalid_logger_level : public std::runtime_error
        {
        public:
            invalid_logger_level() : std::runtime_error{ "unregistered logger level used" }
            {
            }
        };

        class duplicate_logger_level : public std::runtime_error
        {
        public:
            duplicate_logger_level()
                : std::runtime_error{ "attempted to register a logger level "
                                      "that has already been registered" }
            {
            }
        };

        class level_registry
        {
        public:
            template<typename T, typename... Args>
            void register_level(T, Args &&... args)
            {
                _config.set<T>(std::forward<Args>(args)...);
            }

            template<typename T>
            std::vector<streamable> operator[](T) const
            {
                return _config.get<T>();
            }

        private:
            configuration _config;
        };

        inline level_registry & default_level_registry()
        {
            static level_registry default_registry = [] {
                using style::styles;
                using style::colors;
                using style::style;

                level_registry reg;

                reg.register_level(always, style());
                reg.register_level(trace, style(colors::bgray), "Trace", style(), ": ");
                reg.register_level(debug, style(colors::gray), "Debug", style(), ": ");
                reg.register_level(note, style(colors::gray, colors::def, styles::bold), "Note", style(), ": ");
                reg.register_level(info, style(colors::gray, colors::def, styles::bold), "Info", style(), ": ");
                reg.register_level(success, style(colors::green, colors::def, styles::bold), "Success", style(colors::bgray, colors::def, styles::bold), ": ");
                reg.register_level(warning, style(colors::bbrown, colors::def, styles::bold), "Warning", style(colors::bgray, colors::def, styles::bold), ": ");
                reg.register_level(error, style(colors::bred, colors::def, styles::bold), "Error", style(colors::bgray, colors::def, styles::bold), ": ");
                reg.register_level(fatal, style(colors::bred, colors::def, styles::bold), "Fatal error", style(colors::bgray, colors::def, styles::bold), ": ");
                reg.register_level(
                    crash, style(colors::bred, colors::def, styles::bold), "Internal error", style(colors::bgray, colors::def, styles::bold), ": ");

                return reg;
            }();

            return default_registry;
        }
    }
}
}
