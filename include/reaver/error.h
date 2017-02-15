/**
 * Reaver Library Licence
 *
 * Copyright © 2013, 2015 Michał "Griwes" Dominiak
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

#include "exception.h"
#include "logger.h"
#include "swallow.h"

namespace reaver
{
inline namespace _v1
{
    class error_engine_exception : public exception
    {
    public:
        error_engine_exception(std::vector<exception> errors, std::size_t error_count, std::size_t warning_count)
            : exception{ logger::fatal }, errors{ std::move(errors) }, error_count{ error_count }, warning_count{ warning_count }
        {
        }

        virtual void print(reaver::logger::logger & l) const noexcept override
        {
            for (const auto & e : errors)
            {
                e.print(l);
            }

            l() << error_count << " error" << (error_count != 1 ? "s" : "") << " and " << warning_count << " warning" << (warning_count != 1 ? "s" : "")
                << " generated.";
        }

        const std::vector<exception> errors;
        const std::size_t error_count;
        const std::size_t warning_count;
    };

    class error_engine
    {
    public:
        error_engine(std::size_t max_errors = 20, logger::base_level error_level = logger::base_level::error)
            : _max_errors{ max_errors }, _error_level{ error_level }
        {
        }

        error_engine(const error_engine &) = delete;

        error_engine(error_engine && moved_out) noexcept
        {
            using std::swap;

            swap(_errors, moved_out._errors);
            swap(_error_count, moved_out._error_count);
            swap(_warning_count, moved_out._warning_count);
            swap(_max_errors, moved_out._max_errors);
            swap(_error_level, moved_out._error_level);

            _reported = moved_out._reported;
            moved_out._reported = true;
        }

        ~error_engine()
        {
            if (_error_count && !_reported)
            {
                _throw();
            }
        }

        void validate()
        {
            if (_error_count && !_reported)
            {
                _throw();
            }
        }

        void push(exception e)
        {
            auto level = e.level();
            _errors.emplace_back(std::move(e));

            if (level >= _error_level && level != logger::base_level::always)
            {
                ++_error_count;
            }

            else if (level == logger::base_level::warning)
            {
                ++_warning_count;
            }

            if (_error_count == _max_errors)
            {
                _errors.emplace_back(exception(logger::fatal) << "too many errors emitted: " << _max_errors << ".");
                level = logger::fatal;
            }

            if (level == logger::base_level::crash || level == logger::base_level::fatal)
            {
                _throw();
            }
        }

        template<typename... Ts>
        void push(Ts &&... ts)
        {
            std::vector<exception> vec;
            vec.reserve(sizeof...(Ts));
            swallow{ (vec.push_back(std::forward<Ts>(ts)), 0)... };
            push(std::move(vec));
        }

        void push(std::vector<exception> ve)
        {
            for (auto && elem : ve)
            {
                auto level = elem.level();
                _errors.emplace_back(std::move(elem));

                if (level >= _error_level && level != logger::base_level::always)
                {
                    ++_error_count;
                }

                else if (level == logger::base_level::warning)
                {
                    ++_warning_count;
                }
            }

            if (_error_count >= _max_errors)
            {
                _errors.emplace_back(exception(logger::fatal) << "too many errors emitted: " << _max_errors << ".");
            }

            auto level = _errors.back().level();
            if (level == logger::base_level::crash || level == logger::base_level::fatal)
            {
                _throw();
            }
        }

        void set_error_level(logger::base_level l)
        {
            _error_level = l;
        }

        void set_error_limit(std::size_t i)
        {
            if (i == 0)
            {
                i = 1;
            }

            _max_errors = i;

            if (_error_count >= _max_errors)
            {
                _errors.emplace_back(exception(logger::fatal) << "too many errors emitted: " << _max_errors << ".");
                _throw();
            }
        }

        void print(reaver::logger::logger & l) const noexcept
        {
            if (std::current_exception())
            {
                _reported = true;
            }

            for (const auto & e : _errors)
            {
                e.print(l);
            }

            l() << _error_count << " error" << (_error_count != 1 ? "s" : "") << " and " << _warning_count << " warning" << (_warning_count != 1 ? "s" : "")
                << " generated.";
        }

        operator bool() const
        {
            return !_error_count;
        }

        bool operator!() const
        {
            return _error_count;
        }

        bool error_count() const
        {
            return _errors.size();
        }

    private:
        void _throw()
        {
            _reported = true;
            throw error_engine_exception{ std::move(_errors), _error_count, _warning_count };
        }

        std::vector<exception> _errors;
        std::size_t _error_count = 0;
        std::size_t _warning_count = 0;
        std::size_t _max_errors;
        logger::base_level _error_level;
        mutable bool _reported = false;
    };
}
}
