/**
 * Reaver Library
 * 
 * Copyright (C) 2012 Reaver Project Team:
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

#include <functional>
#include <ostream>
#include <vector>
#include <mutex>
#include <memory>
#include <iostream>

namespace reaver
{
    namespace logger
    {    
        enum
        {
            trace,
            debug,
            info,
            warning,
            error,
            crash,
            always
        };
        
        class stream_wrapper
        {
        public:
            std::ostream & get()
            {
                return std::cout;
            }
        };
        
        class logger
        {
        public:
    #ifdef DEBUG
    # define def debug
    #else
    # define def info
    #endif
            logger(int level = def) : _level(level), _writer([](const std::string & rhs) { return rhs; }), _ref_streams({std::cout}) {}
            template<typename T>
            logger(const typename std::enable_if<!std::is_integral<T>::value>::type & writer) : _level(def), _writer(writer), _ref_streams({std::cout}) {}
            template<typename T>
            logger(int level, const T & writer) : _level(level), _writer(writer), _ref_streams({std::cout}) {}
    #undef def
            ~logger()
            {
                flush();
            }
            
            int level(int level = -1)
            {
                if (level != -1)
                {
                    _level = level;
                }
                
                return _level;
            }
            
            void add_stream(std::ostream & o)
            {
                _ref_streams.push_back(o);
            }
            
            void add_stream(std::shared_ptr<std::ostream> o)
            {
                _streams.push_back(o);
            }
            
            void add_stream(std::shared_ptr<reaver::logger::stream_wrapper> o)
            {
                _special_streams.push_back(o);
            }
            
            void flush()
            {
                for (auto s : _streams)
                {
                    *s << std::flush;
                }
                
                for (auto s : _ref_streams)
                {
                    s.get() << std::flush;
                }
                
                for (auto s : _special_streams)
                {
                    s->get() << std::flush;
                }
            }
            
            class action
            {
            public:
                action(logger & parent, int level) : _level(level), _log(parent) {}
                ~action()
                {
                    _log._write("\n", _level);
                    _log._lock.unlock();
                }
                
                template<typename T>
                action & operator << (const T & rhs)
                {
                    _log._write(rhs, _level);
                    
                    return *this;
                }
                
            private:
                int _level;
                logger & _log;
            };
            
            friend class action;
            
            action operator()(int level = always)
            {
                _lock.lock();
                
                std::string prefix;
                
                switch (level)
                {
                    case debug:
                        _write("Debug: ", debug);
                        break;
                    case info:
                        _write("Info: ", info);
                        break;
                    case warning:
                        _write("Warning: ", warning);
                        break;
                    case error:
                        _write("Error: ", error);
                        break;
                    case crash:
                        _write("Crash: ", crash);
                        break;
                    case always:
                        ;
                }
                
                return action(*this, level);
            }
            
            action operator()(std::string cstr)
            {
                if (cstr == "debug")
                {
                    return operator()(debug);
                }
                
                if (cstr == "info")
                {
                    return operator()(info);
                }
                
                if (cstr == "warning")
                {
                    return operator()(warning);
                }
                
                if (cstr == "error")
                {
                    return operator()(error);
                }
                
                if (cstr == "crash")
                {
                    return operator()(crash);
                }
                
                return operator()(always);
            }
            
        private:
            template<typename T>
            void _write(const T & rhs, int level)
            {
                if (level < _level)
                {
                    return;
                }
                
                for (auto s : _streams)
                {
                    *s << _writer(rhs);
                }
                
                for (auto s : _ref_streams)
                {
                    s.get() << _writer(rhs);
                }
                
                for (auto s : _special_streams)
                {
                    s->get() << _writer(rhs);
                }
            }
            
            int _level;
            std::function<std::string (std::string)> _writer;
            std::vector<std::reference_wrapper<std::ostream>> _ref_streams;
            std::vector<std::shared_ptr<reaver::logger::stream_wrapper>> _special_streams;
            std::vector<std::shared_ptr<std::ostream>> _streams;
            std::mutex _lock;
        };
    }
}
