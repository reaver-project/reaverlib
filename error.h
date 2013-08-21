/**
 * Reaver Library Licence
 *
 * Copyright (C) 2013 Reaver Project Team:
 * 1. Michał "Griwes" Dominiak
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
 * Michał "Griwes" Dominiak
 *
 **/

#pragma once

#include "logger.h"
#include "exception.h"

namespace reaver
{
    class error_engine : public exception
    {
    public:
        error_engine(uint64_t max_errors = 20, logger::level error_level = syntax) : _max_errors{ max_errors },
            _error_level{ error_level }
        {
        }

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

        ~error_engine()
        {
            if (_error_count && !_printed_in_handler)
            {
                throw std::move(*this);
            }
        }

        void push(exception e)
        {
            _errors.emplace_back(std::move(e));

            if (_errors.back().level() >= _error_level && _errors.back().level() != always)
            {
                ++_error_count;
            }

            else if (_errors.back().level() == warning)
            {
                ++_warning_count;
            }

            if (_error_count == _max_errors)
            {
                _errors.emplace_back(exception(fatal) << "too many errors emitted: " << _max_errors << ".");

                // uh... I'm sorry, officer, I can't explain how this happened
                throw std::move(*this);
            }

            if (_errors.back().level() == crash || _errors.back().level() == fatal)
            {
                throw std::move(*this);
            }
        }

        void set_error_level(::level l)
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

        void set_error_limit(uint64_t i)
        {
            _max_errors = i;

            if (_error_count >= _max_errors)
            {
                _errors.emplace_back(exception(fatal) << "too many errors emitted: " << _max_errors << ".");

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

        bool size() const
        {
            return _errors.size();
        }

        template<typename T>
        exception & operator<<(const T & rhs) = delete;

    private:
        std::vector<exception> _errors;
        uint64_t _error_count = 0;
        uint64_t _warning_count = 0;
        uint64_t _max_errors;
        logger::level _error_level;
        mutable bool _printed_in_handler = false;
    };
}
