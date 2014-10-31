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

#include <vector>
#include <atomic>

#include "level_registry.h"
#include "streamable.h"
#include "logger_friend.h"
#include "../style.h"

namespace reaver
{
    namespace logger { inline namespace _v1
    {
        template<typename Level = always_type>
        class action : public logger_friend
        {
        public:
            action(logger & log = reaver::logger::default_logger()) : _logger(log), _streamables()
            {
                _streamables.emplace_back(style::style());
            }

            ~action()
            {
                if (Level{} < logger_friend::_level(_logger))
                {
                    return;
                }

                _streamables.emplace_back('\n');
                logger_friend::_write(_logger, _streamables);
            }

            template<typename T>
            action & operator<<(T && rhs)
            {
                _streamables.emplace_back(std::forward<T>(rhs));
                return *this;
            }

            template<typename T>
            action & operator<<(const std::atomic<T> & atomic)
            {
                return *this << atomic.load();
            }

            friend class logger;

        private:
            action(logger & log, std::vector<streamable> vec) : _logger{ log }
            {
                _streamables.swap(vec);
            }

            logger & _logger;

            std::vector<streamable> _streamables;
        };
    }}
}
