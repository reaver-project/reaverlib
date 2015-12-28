/**
 * Reaver Library Licence
 *
 * Copyright © 2013 Michał "Griwes" Dominiak
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

// some code review done on: 29.12.2015
// TODO: further review of the functionality needed at a later time

#pragma once

#include "logger.h"
#include "exception.h"
#include "swallow.h"

namespace reaver { inline namespace _v1
{
    class error_engine : public exception
    {
    public:
        error_engine(std::size_t max_errors = 20, logger::base_level error_level = logger::base_level::error) : _max_errors{ max_errors },
            _error_level{ error_level }
        {
        }

        error_engine(const error_engine &) = delete;

        error_engine(error_engine && moved_out)
        {
            using std::swap;

            swap(_errors, moved_out._errors);
            swap(_error_count, moved_out._error_count);
            swap(_warning_count, moved_out._warning_count);
            swap(_max_errors, moved_out._max_errors);
            swap(_error_level, moved_out._error_level);

            _printed_in_handler = moved_out._printed_in_handler;
            moved_out._printed_in_handler = true;
        }

        // this is very ugly; I'm a very naughty child
        ~error_engine() noexcept(false)
        {
            if (_error_count && !_printed_in_handler)
            {
                throw std::move(*this);
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
                // uh... I'm sorry, officer, I can't explain how this happened
                throw std::move(*this);
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
                // uh... I'm sorry, officer, I can't explain how this happened
                throw std::move(*this);
            }
        }

        void set_error_level(logger::base_level l)
        {
            _error_level = l;
        }

        virtual const char * what() const noexcept
        {
            if (!_printed_in_handler && std::current_exception() != std::exception_ptr{})
            {
                _printed_in_handler = true;
            }

            return exception::what();
        }

        void set_error_limit(std::size_t i)
        {
            _max_errors = i;

            if (_error_count >= _max_errors)
            {
                _errors.emplace_back(exception(logger::fatal) << "too many errors emitted: " << _max_errors << ".");

                throw std::move(*this);
            }
        }

        virtual void print(reaver::logger::logger & l) const noexcept
        {
            if (std::current_exception())
            {
                _printed_in_handler = true;
            }

            for (const auto & e : _errors)
            {
                e.print(l);
            }

            l() << _error_count << " error" << (_error_count != 1 ? "s" : "") << " and " << _warning_count << " warning"
                << (_warning_count != 1 ? "s" : "") << " generated.";
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

        template<typename T>
        exception & operator<<(const T & rhs) = delete;

    private:
        std::vector<exception> _errors;
        std::size_t _error_count = 0;
        std::size_t _warning_count = 0;
        std::size_t _max_errors;
        logger::base_level _error_level;
        mutable bool _printed_in_handler = false;
    };
}}
