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

        void push(exception e)
        {
            _errors.emplace_back(std::move(e));

            if (_errors.back().level() >= _error_level)
            {
                ++_error_count;
            }

            else if (_errors.back().level() == warning)
            {
                ++_warning_count;
            }

            if (_error_count == _max_errors)
            {
                _errors.emplace_back(exception(crash) << "too many errors emitted: " << _max_errors << ".");

                // uh... I'm sorry, officer, I can't explain how this happened
                throw std::move(*this);
            }
        }

        void set_error_level(::level l)
        {
            _error_level = l;
        }

        void set_error_limit(uint64_t i)
        {
            _max_errors = i;

            if (_error_count >= _max_errors)
            {
                _errors.emplace_back(exception(crash) << "too many errors emitted: " << _max_errors << ".");

                throw std::move(*this);
            }
        }

        virtual void print(reaver::logger::logger & l) const noexcept
        {
            for (const auto & e : _errors)
            {
                e.print(l);
            }

            (exception(always) << _error_count << " errors and " << _warning_count << " warnings generated.").print(l);
        }

        template<typename T>
        exception & operator<<(const T & rhs) = delete;

    private:
        std::vector<exception> _errors;
        uint64_t _error_count = 0;
        uint64_t _warning_count = 0;
        uint64_t _max_errors;
        logger::level _error_level;
    };
}
