/**
 * Reaver Library Licence
 *
 * Copyright (C) 2012-2013 Reaver Project Team:
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

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <istream>
#include <regex>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include "exception.h"

namespace reaver
{
    namespace lexer
    {
        template<typename Out, typename In>
        Out convert(const In & in)
        {
            return boost::lexical_cast<Out>(in);
        }

        namespace _detail
        {
            template<typename CharType>
            struct _token
            {
                _token(const std::basic_string<CharType> & s) : literal{ s }
                {
                }

                virtual ~_token() {}

                std::basic_string<CharType> literal;
            };

            template<typename T, typename CharType>
            struct _token_impl : public _token<CharType>
            {
                _token_impl(const T & t, const std::basic_string<CharType> & s) : _token<CharType>{ s }, match{ t }
                {
                }

                T match;
            };

            template<typename CharType>
            struct _token_impl<std::basic_string<CharType>, CharType> : public _token<CharType>
            {
                _token_impl(const std::basic_string<CharType> & s) : _token<CharType>{ s }, match{ _token<CharType>::literal }
                {
                }

                std::basic_string<CharType> & match;
            };

            template<typename, typename>
            struct _as;

            template<typename CharType>
            class _iterator_wrapper
            {
            public:
                virtual ~_iterator_wrapper() {}

                virtual const CharType & operator*() = 0;
                virtual void operator++() = 0;
                virtual void operator+=(uint64_t i) = 0;
                virtual bool operator==(_iterator_wrapper *) = 0;
                virtual bool operator>(_iterator_wrapper *) = 0;
                virtual bool operator>=(_iterator_wrapper *) = 0;

                virtual std::shared_ptr<_iterator_wrapper<CharType>> clone() = 0;
            };

            template<typename CharType, typename Iterator>
            class _iterator_wrapper_impl : public _iterator_wrapper<CharType>
            {
            public:
                _iterator_wrapper_impl(Iterator it) : _it{ it }
                {
                }

                virtual ~_iterator_wrapper_impl() {}

                virtual const CharType & operator*()
                {
                    return *_it;
                }

                virtual void operator++()
                {
                    ++_it;
                }

                virtual void operator+=(uint64_t i)
                {
                    std::advance(_it, i);
                }

                virtual bool operator==(_iterator_wrapper<CharType> * rhs)
                {
                    auto orig = dynamic_cast<_iterator_wrapper_impl<CharType, Iterator> *>(rhs);

                    if (!orig)
                    {
                        throw std::bad_cast{};
                    }

                    return _it == orig->_it;
                }

                virtual bool operator>(_iterator_wrapper<CharType> * rhs)
                {
                    auto orig = dynamic_cast<_iterator_wrapper_impl<CharType, Iterator> *>(rhs);

                    if (!orig)
                    {
                        throw std::bad_cast{};
                    }

                    return _it > orig->_it;
                }

                virtual bool operator>=(_iterator_wrapper<CharType> * rhs)
                {
                    auto orig = dynamic_cast<_iterator_wrapper_impl<CharType, Iterator> *>(rhs);

                    if (!orig)
                    {
                        throw std::bad_cast{};
                    }

                    return _it >= orig->_it;
                }

                virtual std::shared_ptr<_iterator_wrapper<CharType>> clone()
                {
                    return std::make_shared<_iterator_wrapper_impl<CharType, Iterator>>(_it);
                }

            private:
                Iterator _it;
            };
        }

        template<typename CharType>
        class iterator_wrapper
        {
        public:
            using value_type = CharType;
            using reference_type = const CharType &;
            using pointer_type = const CharType *;
            using difference_type = std::ptrdiff_t;

            iterator_wrapper() = default;

            template<typename T, typename = typename std::enable_if<std::is_same<typename std::remove_cv<typename
                std::iterator_traits<T>::value_type>::type, CharType>::value>::type>
            iterator_wrapper(T iterator) : _it{ std::make_shared<_detail::_iterator_wrapper_impl<CharType, T>>(iterator) }
            {
            }

            template<typename T, typename = typename std::enable_if<std::is_same<typename std::remove_cv<typename
                std::iterator_traits<T>::value_type>::type, CharType>::value>::type>
            iterator_wrapper(T iterator, iterator_wrapper end) : _it{ std::make_shared<_detail::_iterator_wrapper_impl<
                CharType, T>>(iterator) }, _end{ end._it->clone() }
            {
            }

            iterator_wrapper(const iterator_wrapper & rhs) : _it{ rhs._it->clone() }
            {
            }

            const CharType & operator*()
            {
                return **_it;
            }

            iterator_wrapper & operator++()
            {
                if (!_end || !(*_it == &*_end))
                {
                    _it->operator++();
                }

                return *this;
            }

            iterator_wrapper operator++(int)
            {
                iterator_wrapper<CharType> tmp = *this;
                ++*this;
                return tmp;
            }

            iterator_wrapper & operator+=(uint64_t i)
            {
                if (!_end || !(*_it == &*_end))
                {
                    *_it += i;
                }

                if (_end && *_it >= &*_end)
                {
                    *_it = *_end;
                }

                return *this;
            }

            iterator_wrapper operator+(uint64_t i)
            {
                iterator_wrapper<CharType> tmp = *this;
                tmp += i;
                return tmp;
            }

            bool operator!=(iterator_wrapper rhs) const
            {
                return !(*this == rhs);
            }

            bool operator==(iterator_wrapper rhs) const
            {
                return *_it == &*rhs._it;
            }

        private:
            std::shared_ptr<_detail::_iterator_wrapper<CharType>> _it;
            std::shared_ptr<_detail::_iterator_wrapper<CharType>> _end;
        };
    }
}

template<typename CharType>
struct std::iterator_traits<reaver::lexer::iterator_wrapper<CharType>>
{
    using value_type = CharType;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
};

namespace reaver
{
    namespace lexer
    {
        template<typename CharType>
        class basic_token
        {
        public:
            using char_type = CharType;

            basic_token(uint64_t type = -1) : _type{ type }
            {
            }

            template<typename T>
            basic_token(uint64_t type, const T & t, std::basic_string<CharType> s) : _type{ type }, _token{ new _detail::_token_impl
                <T, CharType>{ t, std::move(s) } }
            {
            }

            basic_token(uint64_t type, std::basic_string<CharType> s, const std::basic_string<CharType> &) : _type{ type },
                _token{ new _detail::_token_impl<std::basic_string<CharType>, CharType>{ std::move(s) } }
            {
            }

            template<typename T>
            T as() const
            {
                return _detail::_as<T, CharType>::get(*this);
            }

            uint64_t type() const
            {
                return _type;
            }

            std::size_t position() const
            {
                return _pos;
            }

            std::size_t position(std::size_t pos)
            {
                auto ret = _pos;
                _pos = pos;
                return ret;
            }

        private:
            uint64_t _type;
            std::size_t _pos = 0;
            std::shared_ptr<_detail::_token<CharType>> _token;

            template<typename, typename>
            friend class _detail::_as;
        };

        template<typename>
        struct is_token : public std::false_type
        {
        };

        template<typename CharType>
        struct is_token<basic_token<CharType>> : public std::true_type
        {
        };

        namespace _detail
        {
            template<typename T, typename CharType>
            struct _as
            {
                static T get(const basic_token<CharType> & token)
                {
                    if (dynamic_cast<_detail::_token_impl<T, CharType> *>(&*token._token))
                    {
                        return dynamic_cast<_detail::_token_impl<T, CharType> *>(&*token._token)->match;
                    }

                    else
                    {
                        throw std::bad_cast{};
                    }
                }
            };

            template<typename CharType>
            struct _as<std::basic_string<CharType>, CharType>
            {
                static std::basic_string<CharType> get(const basic_token<CharType> & token)
                {
                    return token._token->literal;
                }
            };

            template<typename CharType>
            class _token_description
            {
            public:
                virtual ~_token_description() {}

                virtual basic_token<CharType> match(iterator_wrapper<CharType> &, iterator_wrapper<CharType>) = 0;
                virtual uint64_t type() = 0;
            };

            template<typename CharType, typename T>
            class _token_description_impl : public _token_description<CharType>
            {
            public:
                template<typename F>
                _token_description_impl(uint64_t type, std::basic_regex<CharType> regex, F converter) : _type{ type }, _regex{ regex },
                    _converter{ converter }
                {
                }

                virtual basic_token<CharType> match(iterator_wrapper<CharType> & begin, iterator_wrapper<CharType> end)
                {
                    std::match_results<iterator_wrapper<CharType>> match;

                    if (std::regex_search(begin, end, match, _regex, std::regex_constants::match_any | std::regex_constants::match_continuous))
                    {
                        if (match[0].first == begin)
                        {
                            begin += match[0].str().length();
                            return basic_token<CharType>{ _type, _converter(match[0].str()), match[0].str() };
                        }
                    }

                    return basic_token<CharType>{ -1 };
                }

                virtual uint64_t type()
                {
                    return _type;
                }

            private:
                uint64_t _type;
                std::basic_regex<CharType> _regex;
                std::function<T (const std::basic_string<CharType> &)> _converter;
            };
        }

        template<typename>
        class basic_token_description;

        template<typename CharType, typename T = std::basic_string<CharType>>
        class basic_token_definition
        {
        public:
            friend class basic_token_description<CharType>;

            using value_type = T;

            basic_token_definition(uint64_t type, std::basic_string<CharType> regex) : _desc{ new _detail::_token_description_impl
                <CharType, T>{ type, std::basic_regex<CharType>{ regex, std::regex::extended }, [](const std::basic_string
                <CharType> & str) { return convert<T>(str); } } }
            {
            }

            template<typename F>
            basic_token_definition(uint64_t type, std::basic_string<CharType> regex, F converter) : _desc
                { new _detail::_token_description_impl<CharType, T>{ type, std::basic_regex<CharType>{ regex,
                std::regex::extended }, converter } }
            {
            }

            basic_token<CharType> match(iterator_wrapper<CharType> & begin, iterator_wrapper<CharType> end) const
            {
                return _desc->match(begin, end);
            }

            uint64_t type() const
            {
                return _desc->type();
            }

        private:
            std::shared_ptr<_detail::_token_description_impl<CharType, T>> _desc;
        };

        template<typename T>
        struct match_type
        {
        };

        template<typename CharType>
        class basic_token_description
        {
        public:
            template<typename T>
            basic_token_description(const basic_token_definition<CharType, T> & def) : _desc{ def._desc }
            {
            }

            basic_token_description(uint64_t type, std::basic_string<CharType> regex) : _desc{ new _detail::_token_description_impl
                <CharType, std::basic_string<CharType>>{ type, std::basic_regex<CharType>{ regex, std::regex::extended },
                [](const std::basic_string<CharType> & str) { return str; } } }
            {
            }

            template<typename T>
            basic_token_description(uint64_t type, std::basic_string<CharType> regex, match_type<T>) : _desc{ new _detail::
                _token_description_impl<CharType, T>{ type, std::basic_regex<CharType>{ regex, std::regex::extended },
                [](const std::basic_string<CharType> & str) { return convert<T>(str); } } }
            {
            }

            template<typename T, typename F>
            basic_token_description(uint64_t type, std::basic_string<CharType> regex, match_type<T>, F converter) : _desc
                { new _detail::_token_description_impl<CharType, T>{ type, std::basic_regex<CharType>{ regex,
                std::regex::extended }, converter } }
            {
            }

            basic_token<CharType> match(iterator_wrapper<CharType> & begin, iterator_wrapper<CharType> end) const
            {
                return _desc->match(begin, end);
            }

            uint64_t type() const
            {
                return _desc->type();
            }

        private:
            std::shared_ptr<_detail::_token_description<CharType>> _desc;
        };

        template<typename CharType>
        class basic_tokens_description
        {
        public:
            basic_tokens_description() {}

            struct _inserter
            {
                _inserter(basic_tokens_description<CharType> & parent) : _parent{ parent }
                {
                }

                _inserter & operator()(const basic_token_description<CharType> & desc)
                {
                    _parent._descs.emplace(std::make_pair(desc.type(), desc));
                    return *this;
                }

                template<typename... Args>
                _inserter & operator()(Args &&... args)
                {
                    (*this)({ std::forward<Args>(args)... });
                    return *this;
                }

                _inserter & operator()(const std::string & alias, uint64_t type)
                {
                    _parent._aliases[alias] = type;
                    return *this;
                }

            private:
                basic_tokens_description<CharType> & _parent;
            };

            friend struct _inserter;

            _inserter add(const basic_token_description<CharType> & desc)
            {
                _descs.emplace(std::make_pair(desc.type(), desc));
                return { *this };
            }

            template<typename... Args>
            _inserter add(Args &&... args)
            {
                add({ std::forward<Args>(args)... });
                return { *this };
            }

            _inserter add(const std::string & alias, uint64_t type)
            {
                _aliases[alias] = type;
            }

            typename std::map<uint64_t, basic_token_description<CharType>>::const_iterator begin() const
            {
                return _descs.begin();
            }

            typename std::map<uint64_t, basic_token_description<CharType>>::const_iterator end() const
            {
                return _descs.end();
            }

            basic_token_description<CharType> operator[](uint64_t type)
            {
                return _descs.at(type);
            }

            basic_token_description<CharType> operator[](const std::string & alias)
            {
                return _descs.at(_aliases.at(alias));
            }

        private:
            std::map<uint64_t, basic_token_description<CharType>> _descs;
            std::map<std::string, uint64_t> _aliases;
        };

        class unexpected_characters : public exception
        {
        public:
            unexpected_characters() : exception{ crash }
            {
                *this << "unexpected characters in tokenized string.";
            }
        };

        namespace _detail
        {
            template<typename CharType>
            class _queue_chunk
            {
            public:
                _queue_chunk() : _array{{}}, _next_index{ 0 }, _last_chunk{ false }
                {
                }

                void push(basic_token<CharType> token)
                {
                    std::unique_lock<std::mutex> lock{ _mutex };

                    if (_next_index == 4096)
                    {
                        throw std::out_of_range{ "reaver::lexer::_detail::_queue_chunk::push" };
                    }

                    _array[_next_index++] = std::move(token);
                }

                basic_token<CharType> & get(std::size_t index)
                {
                    std::unique_lock<std::mutex> lock{ _mutex };

                    if (index >= _next_index)
                    {
                        throw std::out_of_range{ "reaver::lexer::_detail::_queue_chunk::get" };
                    }

                    return _array[index];
                }

                std::shared_ptr<_queue_chunk> next()
                {
                    std::unique_lock<std::mutex> lock{ _mutex };

                    if (_last_chunk)
                    {
                        return { nullptr };
                    }

                    if (!_next)
                    {
                        _next = std::make_shared<_queue_chunk>();
                    }

                    return _next;
                }

                void end()
                {
                    std::unique_lock<std::mutex> lock{ _mutex };
                    _last_chunk = true;
                }

                bool last() const
                {
                    return _last_chunk;
                }

                std::size_t size() const
                {
                    return _next_index;
                }

            private:
                std::array<basic_token<CharType>, 4096> _array;
                std::atomic<std::size_t> _next_index;
                std::atomic<bool> _last_chunk;
                std::shared_ptr<_queue_chunk> _next;
                std::mutex _mutex;
            };

            template<typename CharType>
            void _worker(iterator_wrapper<CharType> begin, iterator_wrapper<CharType> end, basic_tokens_description<CharType>
                def, std::shared_ptr<_queue_chunk<CharType>> chunk, std::shared_ptr<std::atomic<size_t>> index, std::shared_ptr<
                semaphore> sem, std::shared_ptr<boost::optional<unexpected_characters>> exception, std::shared_ptr<std::atomic<
                bool>> end_marker)
            {
                std::size_t position = 0;

                while (begin != end)
                {
                    for (auto e = begin + 100; ; e += 100)
                    {
                        auto defb = def.begin(), defe = def.end();

                        for (; defb != defe; ++defb)
                        {
                            if (*end_marker)
                            {
                                chunk->end();
                                sem->notify();
                                return;
                            }

                            auto _ = begin;
                            basic_token<CharType> matched = defb->second.match(_, e);

                            if (matched.type() != -1)
                            {
                                basic_token<CharType> matched = defb->second.match(begin, end);
                                matched.position(position);

                                position += matched.template as<std::string>().length();

                                chunk->push(std::move(matched));

                                ++*index;
                                if (!(*index % 4096) && *index != 0)
                                {
                                    chunk = chunk->next();
                                }

                                sem->notify();

                                goto after;
                            }
                        }

                        if (e == end)
                        {
                            *exception = unexpected_characters{};
                            sem->notify();
                            return;
                        }
                    }

                after:
                    if (*begin == '\0' && begin + 1 == end)
                    {
                        begin = end;
                    }
                }

                chunk->end();
                sem->notify();
            }

            struct _ender
            {
                _ender(std::shared_ptr<std::atomic<bool>> end, std::shared_ptr<std::thread> worker) : end{ std::move(end) },
                    worker{ std::move(worker) }
                {
                }

                ~_ender()
                {
                    *end = true;
                    worker->join();
                }

                std::shared_ptr<std::atomic<bool>> end;
                std::shared_ptr<std::thread> worker;
            };
        }

        class incompatible_iterators : public exception
        {
        public:
            incompatible_iterators() : exception{ crash }
            {
                *this << "incompatible lexer iterators compared.";
            }
        };

        namespace _detail
        {
            using std::begin;
            using std::end;

            template<typename T>
            auto _end(const T & t) -> iterator_wrapper<typename std::remove_cv<typename std::iterator_traits<decltype(begin(t))>
                ::value_type>::type>
            {
                return { end(t) };
            }

            template<typename T>
            auto _begin(const T & t) -> iterator_wrapper<typename std::remove_cv<typename std::iterator_traits<decltype(begin(t))>
                ::value_type>::type>
            {
                return { begin(t), _end(t) };
            }
        }

        template<typename CharType>
        class basic_iterator
        {
        public:
            template<typename T>
            basic_iterator(const T & t, const basic_tokens_description<CharType> & def)
                : basic_iterator{ _detail::_begin(t), _detail::_end(t), def }
            {
            }

            template<typename Iterator>
            basic_iterator(Iterator begin, Iterator end, const basic_tokens_description<CharType> & def) : _chunk{
                std::make_shared<_detail::_queue_chunk<CharType>>() }, _ready_semaphore{ std::make_shared<semaphore>() },
                _index{ 0 }, _max_index{ std::make_shared<std::atomic<std::size_t>>(-1) }, _exception{ std::make_shared<
                boost::optional<unexpected_characters>>() }, _end{ std::make_shared<std::atomic<bool>>(false) }
            {
                static_assert(std::is_same<CharType, typename std::remove_cv<typename std::iterator_traits<Iterator>::value_type>
                    ::type>::value,
                    "incompatible iterator type used in initialization of basic_lexer_iterator (value_type must be the "
                    "same as `basic_lexer_iterator`'s template parameter");

                _worker_thread = std::make_shared<std::thread>(&_detail::_worker<CharType>, begin, end, def, _chunk, _max_index,
                    _ready_semaphore, _exception, _end);
                _ender = std::make_shared<_detail::_ender>(_end, _worker_thread);
                _ready_semaphore->wait();

                if (*_exception)
                {
                    throw std::move(**_exception);
                }
            }

            basic_iterator() : _index{ -1 }
            {
            }

            basic_iterator & operator++()
            {
                if (_index != -1)
                {
                    if (_chunk->last() && _index == *_max_index)
                    {
                        return *this = basic_iterator{};
                    }

                    if (*_exception)
                    {
                        throw std::move(**_exception);
                    }

                    while (_index == *_max_index)
                    {
                        _ready_semaphore->wait();

                        if (_chunk->last() && _index == *_max_index)
                        {
                            return *this = basic_iterator{};
                        }

                        if (*_exception)
                        {
                            throw std::move(**_exception);
                        }
                    }

                    if (!(++_index % 4096))
                    {
                        _chunk = _chunk->next();
                    }
                }

                return *this;
            }

            basic_iterator operator++(int)
            {
                basic_iterator tmp{ *this };
                ++*this;
                return std::move(tmp);
            }

            basic_iterator & operator+=(std::size_t count)
            {
                if (_index != -1)
                {
                    while (count--)
                    {
                        ++*this;
                    }
                }

                return *this;
            }

            basic_iterator operator+(std::size_t count)
            {
                basic_iterator tmp{ *this };
                tmp += count;
                return std::move(tmp);
            }

            basic_token<CharType> & operator*()
            {
                return _chunk->get(_index % 4096);
            }

            basic_token<CharType> * operator->()
            {
                return &**this;
            }

            bool operator<(basic_iterator rhs)
            {
                if (rhs._index == -1)
                {
                    return _index != -1;
                }

                if (_worker_thread != rhs._worker_thread)
                {
                    throw incompatible_iterators{};
                }

                return _index < rhs._index;
            }

            bool operator!=(basic_iterator rhs)
            {
                return *this < rhs || rhs < *this;
            }

            bool operator==(basic_iterator rhs)
            {
                return !(*this != rhs);
            }

        private:
            std::shared_ptr<_detail::_queue_chunk<CharType>> _chunk;
            std::shared_ptr<semaphore> _ready_semaphore;
            std::size_t _index;
            std::shared_ptr<std::atomic<std::size_t>> _max_index;

            std::shared_ptr<std::thread> _worker_thread;
            std::shared_ptr<boost::optional<unexpected_characters>> _exception;

            std::shared_ptr<std::atomic<bool>> _end;
            std::shared_ptr<_detail::_ender> _ender;
        };

        using token_description = basic_token_description<char>;
        template<typename T>
        using token_definition = basic_token_definition<char, T>;
        using tokens_description = basic_tokens_description<char>;
        using token = basic_token<char>;
        using iterator = basic_iterator<char>;
    }
}

template<typename CharType>
struct std::iterator_traits<reaver::lexer::basic_iterator<CharType>>
{
    using value_type = reaver::lexer::basic_token<CharType>;
    using iterator_category = std::forward_iterator_tag;
};
