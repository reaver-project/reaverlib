/**
 * Reaver Library Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include <memory>
#include <exception>
#include <type_traits>

#include "exception.h"
#include "optional.h"
#include "overloads.h"
#include "tpl/rebind.h"
#include "tpl/filter.h"
#include "prelude/functor.h"
#include "function.h"
#include "executor.h"
#include "default_executor.h"
#include "static_if.h"

namespace reaver { inline namespace _v1
{
    constexpr struct ready_type {} ready = {};

    template<typename T, typename... Args>
    std::shared_ptr<executor> make_executor(Args &&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    class broken_promise : public exception
    {
    public:
        broken_promise() : exception{ logger::fatal }
        {
            *this << "broken promise.";
        }
    };

    class multiple_noncopyable_continuations : public exception
    {
    public:
        multiple_noncopyable_continuations() : exception{ logger::error }
        {
            *this << "attempted to attach multiple continuations to a future promising a noncopyable object.";
        }
    };

    class multiple_exceptional_continuations : public exception
    {
    public:
        multiple_exceptional_continuations() : exception{ logger::error }
        {
            *this << "attempted to attach multiple exceptional continuations to a future.";
        }
    };

    template<typename T = void>
    class future;

    template<typename T>
    struct future_package_pair;

    template<typename F>
    auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>;

    namespace _detail
    {
        template<typename T>
        struct _shared_state;

        using _continuation_type = reaver::function<void ()>;

        template<typename T, typename F, typename std::enable_if<std::is_void<T>::value || std::is_copy_constructible<T>::value, int>::type = 0>
        void _add_continuation(_shared_state<T> & state, F && f)
        {
            std::lock_guard<std::mutex> lock{ state.continuations_lock };
            state.continuations.emplace_back(_continuation_type{ std::forward<F>(f) });
        }

        template<typename T, typename F, typename std::enable_if<!std::is_void<T>::value && !std::is_copy_constructible<T>::value, int>::type = 0>
        void _add_continuation(_shared_state<T> & state, F && f)
        {
            std::lock_guard<std::mutex> lock{ state.continuations_lock };
            if (state.continuations)
            {
                throw multiple_noncopyable_continuations{};
                return;
            }

            state.continuations = _continuation_type{ std::forward<F>(f) };
        }

        template<typename T, typename F>
        void _add_exceptional_continuation(_shared_state<T> & state, F && f)
        {
            std::lock_guard<std::mutex> lock{ state.continuations_lock };
            if (state.exceptional_continuations)
            {
                throw multiple_exceptional_continuations{};
                return;
            }

            state.exceptional_continuations = _continuation_type{ std::forward<F>(f) };
        }

        template<typename T>
        bool _is_valid(_shared_state<T> & state)
        {
            return state.value.index() == 0 || state.function;
        }

        template<typename T>
        bool _is_pending(_shared_state<T> & state)
        {
            return state.value.index() == 2 && state.function;
        }

        template<typename... Args, typename std::enable_if<all_of<std::is_copy_constructible<Args>::value...>::value, int>::type = 0>
        auto _move_or_copy(variant<Args...> & v, bool move)
        {
            if (move)
            {
                auto ret = std::move(v);
                v = none;
                return ret;
            }

            return v;
        }

        template<typename... Args, typename std::enable_if<!all_of<std::is_copy_constructible<Args>::value...>::value, int>::type = 0>
        auto _move_or_copy(variant<Args...> & v, bool)
        {
            auto ret = std::move(v);
            v = none;
            return ret;
        }

        template<typename T>
        struct _shared_state : public std::enable_shared_from_this<_shared_state<T>>
        {
            using then_t = std::conditional_t<
                std::is_copy_constructible<T>::value,
                std::vector<_continuation_type>,
                optional<_continuation_type>
            >;
            using failed_then_t = optional<_continuation_type>;

            _shared_state(T t) : value{ std::move(t) }
            {
            }

            _shared_state(std::exception_ptr ptr) : value{ ptr }
            {
            }

            _shared_state() : value{ none }
            {
            }

            std::mutex lock;
            variant<T, std::exception_ptr, none_t> value;
            std::atomic<std::size_t> promise_count{ 0 };
            std::atomic<std::size_t> shared_count{ 0 };

            std::shared_ptr<executor> scheduler;

            std::mutex continuations_lock;
            then_t continuations;
            failed_then_t exceptional_continuations;

            // TODO: reaver::function
            optional<class function<T ()>> function;

            optional<T> try_get()
            {
                std::lock_guard<std::mutex> l(lock);
                return get<0>(fmap(_move_or_copy(value, shared_count == 1 && value.index() != 1), make_overload_set(
                    [&](T t) {
                        if (shared_count == 1)
                        {
                            value = none;
                        }
                        return make_optional(std::move(t));
                    },

                    [&](std::exception_ptr ptr) {
                        std::rethrow_exception(ptr);
                        return unit{};
                    },

                    [&](none_t) {
                        return optional<T>();
                    }
                )));
            }

        private:
            T _get()
            {
                assert(value.index() != 2);

                if (value.index() == 1)
                {
                    std::rethrow_exception(get<1>(value));
                }

                auto ret = get<0>(_move_or_copy(value, shared_count == 1 && value.index() != 1));
                if (shared_count == 1)
                {
                    value = none;
                }
                return ret;
            }

        public:
            template<typename F>
            auto then(std::shared_ptr<executor> provided_sched, F && f) -> future<decltype(std::forward<F>(f)(std::declval<T>()))>
            {
                std::lock_guard<std::mutex> l{ lock };

                if (!_is_valid(*this))
                {
                    assert(!"what do?");
                }

                ++shared_count;

                auto sched = [this, provided = std::move(provided_sched)]() {
                    if (provided)
                    {
                        return provided;
                    }

                    if (scheduler)
                    {
                        return scheduler;
                    }

                    return default_executor();
                };

                return get<0>(fmap(value, make_overload_set(
                    [&](variant<const T &, std::exception_ptr>) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            return std::forward<F>(f)(_get());
                        });

                        // GCC is deeply confused when this is directly in the capture list
                        auto state = std::enable_shared_from_this<_shared_state>::shared_from_this();

                        sched()->push([sched, task = std::move(pair.packaged_task), state = std::move(state)](){ task(sched()); });
                        return std::move(pair.future);
                    },

                    [&](none_t) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            return std::forward<F>(f)(_get());
                        });

                        // GCC is deeply confused when this is directly in the capture list
                        auto state = std::enable_shared_from_this<_shared_state>::shared_from_this();

                        _add_continuation(*this, [sched, task = std::move(pair.packaged_task), state = std::move(state)]() mutable {
                            sched()->push([sched, task = std::move(task), state = std::move(state)](){ task(sched()); });
                        });
                        return std::move(pair.future);
                    }
                )));
            }

            template<typename F>
            auto on_error(std::shared_ptr<executor> provided_sched, F && f) -> future<decltype(std::forward<F>(f)(std::declval<std::exception_ptr>()))>
            {
                std::lock_guard<std::mutex> l{ lock };

                if (!_is_valid(*this))
                {
                    assert(!"what do?");
                }

                ++shared_count;

                auto sched = [this, provided = std::move(provided_sched)]() {
                    if (provided)
                    {
                        return provided;
                    }

                    if (scheduler)
                    {
                        return scheduler;
                    }

                    return default_executor();
                };

                return get<0>(fmap(value, make_overload_set(
                    [&](const T &) {
                        auto pair = package([]() -> decltype(std::forward<F>(f)(std::declval<std::exception_ptr>())) { std::terminate(); });
                        return std::move(pair.future);
                    },

                    [&](std::exception_ptr ptr) {
                        auto pair = package([this, f = std::forward<F>(f), ptr]() mutable {
                            return std::forward<F>(f)(ptr);
                        });

                        // GCC is deeply confused when this is directly in the capture list
                        auto state = std::enable_shared_from_this<_shared_state>::shared_from_this();

                        sched()->push([sched, task = std::move(pair.packaged_task), state = std::move(state)](){ task(sched()); });
                        return std::move(pair.future);
                    },

                    [&](none_t) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            if (value.index() == 1)
                            {
                                auto ptr = get<1>(value);
                                return std::forward<F>(f)(ptr);
                            }
                            assert(!"or maybe make the resulting future exceptional? this would be a solution, though not a stellar one");
                            assert(!"I guess we should somehow propagate the value here, but there's no obvious way to do that");
                        });

                        // GCC is deeply confused when this is directly in the capture list
                        auto state = std::enable_shared_from_this<_shared_state>::shared_from_this();

                        _add_exceptional_continuation(*this, [sched, task = std::move(pair.packaged_task), state = std::move(state)]() mutable {
                            sched()->push([sched, task = std::move(task), state = std::move(state)](){ task(sched()); });
                        });
                        return std::move(pair.future);
                    }
                )));
            }

            template<typename F>
            auto then(F && f)
            {
                return then(scheduler, std::forward<F>(f));
            }

            template<typename F>
            auto on_error(F && f)
            {
                return on_error(scheduler, std::forward<F>(f));
            }
        };

        template<>
        struct _shared_state<void> : std::enable_shared_from_this<_shared_state<void>>
        {
            using then_t = std::vector<_continuation_type>;
            using failed_then_t = optional<_continuation_type>;

            _shared_state(ready_type) : value{ ready }
            {
            }

            _shared_state(std::exception_ptr ptr) : value{ ptr }
            {
            }

            _shared_state() : value{ none }
            {
            }

            mutable std::mutex lock;
            variant<ready_type, std::exception_ptr, none_t> value;
            std::atomic<std::size_t> promise_count{ 0 };
            std::atomic<std::size_t> shared_count{ 0 };

            std::shared_ptr<executor> scheduler;

            std::mutex continuations_lock;
            then_t continuations;
            failed_then_t exceptional_continuations;

            // TODO: reaver::function
            optional<class function<void ()>> function;

            bool try_get()
            {
                std::lock_guard<std::mutex> l(lock);
                return get<0>(fmap(value, make_overload_set(
                    [&](ready_type) {
                        if (shared_count == 1)
                        {
                            value = none;
                        }
                        return true;
                    },

                    [&](std::exception_ptr ptr) {
                        std::rethrow_exception(ptr);
                        return unit{};
                    },

                    [&](none_t) {
                        return false;
                    }
                )));
            }

        private:
            void _get()
            {
                assert(value.index() != 2);

                if (value.index() == 1)
                {
                    std::rethrow_exception(get<1>(value));
                }

                if (shared_count == 1)
                {
                    value = none;
                }
            }

        public:
            template<typename F>
            auto then(std::shared_ptr<executor> provided_sched, F && f) -> future<decltype(std::forward<F>(f)())>
            {
                std::lock_guard<std::mutex> l{ lock };

                if (!_is_valid(*this))
                {
                    assert(!"what do?");
                }

                auto sched = [this, provided = std::move(provided_sched)]() {
                    if (provided)
                    {
                        return provided;
                    }

                    if (scheduler)
                    {
                        return scheduler;
                    }

                    return default_executor();
                };

                ++shared_count;

                return get<0>(fmap(value, make_overload_set(
                    [&](variant<ready_type, std::exception_ptr>) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            _get(); // will throw if in exceptional state
                            return std::forward<F>(f)();
                        });

                        // GCC ICEs when this is in capture list directly
                        auto state = shared_from_this();

                        sched()->push([sched, task = std::move(pair.packaged_task), state = std::move(state)](){ task(sched()); });
                        return std::move(pair.future);
                    },

                    [&](none_t) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            _get(); // will throw if in exceptional state
                            return std::forward<F>(f)();
                        });

                        // GCC ICEs when this is in capture list directly
                        auto state = shared_from_this();

                        _add_continuation(*this, [sched, task = std::move(pair.packaged_task), state = std::move(state)]() mutable {
                            sched()->push([sched, task = std::move(task), state = std::move(state)](){ task(sched()); });
                        });
                        return std::move(pair.future);
                    }
                )));
            }

            template<typename F>
            auto on_error(std::shared_ptr<executor> provided_sched, F && f) -> future<decltype(std::forward<F>(f)(std::declval<std::exception_ptr>()))>
            {
                std::lock_guard<std::mutex> l{ lock };

                if (!_is_valid(*this))
                {
                    assert(!"what do?");
                }

                ++shared_count;

                auto sched = [this, provided = std::move(provided_sched)]() {
                    if (provided)
                    {
                        return provided;
                    }

                    if (scheduler)
                    {
                        return scheduler;
                    }

                    return default_executor();
                };

                return get<0>(fmap(value, make_overload_set(
                    [&](ready_type) {
                        auto pair = package([]() -> decltype(std::forward<F>(f)(std::declval<std::exception_ptr>())) { std::terminate(); });
                        return std::move(pair.future);
                    },

                    [&](std::exception_ptr ptr) {
                        auto pair = package([this, f = std::forward<F>(f), ptr]() mutable {
                            return std::forward<F>(f)(ptr);
                        });

                        // GCC ICEs when this is in capture list directly
                        auto state = shared_from_this();

                        sched()->push([sched, task = std::move(pair.packaged_task), state = std::move(state)](){ task(sched()); });
                        return std::move(pair.future);
                    },

                    [&](none_t) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            if (value.index() == 1)
                            {
                                auto ptr = get<1>(value);
                                return std::forward<F>(f)(ptr);
                            }
                            assert(!"or maybe make the resulting future exceptional? this would be a solution, though not a stellar one");
                            assert(!"I guess we should somehow propagate the value here, but there's no obvious way to do that");
                        });

                        // GCC ICEs when this is in capture list directly
                        auto state = shared_from_this();

                        _add_exceptional_continuation(*this, [sched, task = std::move(pair.packaged_task), state = std::move(state)]() mutable {
                            sched()->push([sched, task = std::move(task), state = std::move(state)](){ task(sched()); });
                        });
                        return std::move(pair.future);
                    }
                )));
            }

            template<typename F>
            auto then(F && f)
            {
                return then(scheduler, std::forward<F>(f));
            }

            template<typename F>
            auto on_error(F && f)
            {
                return on_error(scheduler, std::forward<F>(f));
            }
        };

        template<typename T>
        void _add_promise(_shared_state<T> & state)
        {
            ++state.promise_count;
        }

        template<typename T>
        void _remove_promise(_shared_state<T> & state)
        {
            if (--state.promise_count == 0)
            {
                std::lock_guard<std::mutex> lock{ state.lock };
                if (_is_pending(state))
                {
                    state.function = none;
                    state.value = std::make_exception_ptr(broken_promise{});
                }
            }
        }
    }

    template<typename T>
    class packaged_task
    {
        packaged_task(std::weak_ptr<_detail::_shared_state<T>> ptr) : _state{ std::move(ptr) }
        {
            _add_promise();
        }

        void _add_promise()
        {
            auto state = _state.lock();
            if (state)
            {
                _detail::_add_promise(*state);
            }
        }

        void _remove_promise()
        {
            auto state = _state.lock();
            if (state)
            {
                _detail::_remove_promise(*state);
            }
        }

    public:
        template<typename F>
        friend auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>;

        packaged_task(const packaged_task & other) : _state{ other._state }
        {
            _add_promise();
        }

        packaged_task(packaged_task && other) noexcept : _state{ other._state }
        {
            other._state = {};
        }

        packaged_task & operator=(const packaged_task & other)
        {
            _remove_promise();
            _state = other._state;
            _add_promise();
        }

        packaged_task & operator=(packaged_task && other) noexcept
        {
            _remove_promise();
            _state = other._state;
            other._state = {};
        }

        ~packaged_task()
        {
            _remove_promise();
        }

        void operator()(std::shared_ptr<executor> sched = nullptr) const
        {
            auto state = _state.lock();

            if (!state)
            {
                return;
            }

            std::unique_lock<std::mutex> lock{ state->lock };
            state->scheduler = std::move(sched);

            try
            {
                lock.unlock();
                auto && ret_value = (*state->function)();
                lock.lock();
                state->value = std::move(ret_value);
                if (state->promise_count != 1)
                {
                    lock.unlock();
                }
            }
            catch (...)
            {
                std::lock_guard<std::mutex> lock{ state->continuations_lock };
                state->value = std::current_exception();
                if (state->exceptional_continuations)
                {
                    fmap(std::move(state->exceptional_continuations), [](auto && cont){ cont(); return unit{}; });
                }
            }

            auto conts = decltype(state->continuations){};

            static_if(std::is_copy_constructible<T>{}, [&](auto) {
                // shenanigans
                while (
                    [&]{
                        std::lock_guard<std::mutex> lock{ state->continuations_lock };
                        return !state->continuations.empty();
                    }())
                {
                    {
                        std::lock_guard<std::mutex> lock{ state->continuations_lock };
                        using std::swap;
                        swap(conts, state->continuations);
                    }

                    fmap(conts, [&](auto && cont) {
                        if (state->value.index() != 2)
                        {
                            cont();
                        }
                        return unit{};
                    });
                }
            }).static_else([&](auto) {
                std::lock_guard<std::mutex> lock{ state->continuations_lock };
                fmap(state->continuations, [&](auto && cont) {
                    if (state->value.index() != 2)
                    {
                        cont();
                    }
                    return unit{};
                });
            });

            state->function = none;
            state->continuations = decltype(state->continuations){};
            state->exceptional_continuations = decltype(state->exceptional_continuations){};
        }

    private:
        std::weak_ptr<_detail::_shared_state<T>> _state;
    };

    template<>
    inline void packaged_task<void>::operator()(std::shared_ptr<executor> sched) const
    {
        auto state = _state.lock();

        if (!state)
        {
            return;
        }

        std::unique_lock<std::mutex> lock{ state->lock };
        state->scheduler = std::move(sched);

        try
        {
            lock.unlock();
            (*state->function)();
            lock.lock();
            state->value = ready;
            if (state->promise_count != 1)
            {
                lock.unlock();
            }
        }
        catch (...)
        {
            std::lock_guard<std::mutex> lock{ state->continuations_lock };
            state->value = std::current_exception();
            if (state->exceptional_continuations)
            {
                fmap(state->exceptional_continuations, [](auto && cont){ cont(); return unit{}; });
            }
        }

        auto conts = decltype(state->continuations){};

        // shenanigans
        while (
            [&]{
                std::lock_guard<std::mutex> lock{ state->continuations_lock };
                return !state->continuations.empty();
            }())
        {
            {
                std::lock_guard<std::mutex> lock{ state->continuations_lock };
                swap(conts, state->continuations);
            }

            fmap(conts, [&](auto && cont) {
                if (state->value.index() != 2)
                {
                    cont();
                }
                return unit{};
            });
        }

        state->function = none;
        state->continuations = decltype(state->continuations){};
        state->exceptional_continuations = decltype(state->exceptional_continuations){};
    }

    template<typename T>
    future<T> join(std::shared_ptr<executor>, future<future<T>>);

    namespace _detail
    {
        template<typename F, typename U>
        static auto _unwrap(F, future<U> fut)
        {
            return std::move(fut);
        }

        template<typename F, typename U>
        static auto _unwrap(F sched, future<future<U>> fut)
        {
            return join(sched(), std::move(fut));
        }
    }

    constexpr struct do_not_unwrap_type {} do_not_unwrap{};

    template<typename T>
    class future
    {
        future(std::shared_ptr<_detail::_shared_state<T>> state) : _state{ std::move(state) }
        {
            _add_count();
        }

        void _add_count() const
        {
            if (_state)
            {
                ++_state->shared_count;
            }
        }

        void _remove_count() const
        {
            if (_state)
            {
                --_state->shared_count;
            }
        }

    public:
        using value_type = T;

        template<typename F>
        friend auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>;

        future(const future & other) : _state{ other._state }
        {
            _add_count();
        }

        future(future && other) noexcept : _state{ std::move(other._state) }
        {
            other._state = {};
        }

        future & operator=(const future & other)
        {
            _remove_count();
            _state = other._state;
            _add_count();

            return *this;
        }

        future & operator=(future && other) noexcept
        {
            _remove_count();
            _state = std::move(other._state);
            other._state = {};

            return *this;
        }

        explicit future(T value) : future{ std::make_shared<_detail::_shared_state<T>>(std::move(value)) }
        {
        }

        ~future()
        {
            _remove_count();
        }

        auto try_get()
        {
            return _state->try_get();
        };

        template<typename F>
        auto then(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f) -> future<decltype(std::forward<F>(f)(std::declval<T>()))>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->then(std::move(sched), std::forward<F>(f));
        }

        template<typename F>
        auto then(std::shared_ptr<executor> sched, F && f)
        {
            return _detail::_unwrap([sched = sched]{ return sched; }, then(do_not_unwrap, std::move(sched), std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f) -> future<decltype(std::forward<F>(f)(std::declval<std::exception_ptr>()))>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->on_error(std::move(sched), std::forward<F>(f));
        }

        template<typename F>
        auto on_error(std::shared_ptr<executor> sched, F && f)
        {
            return _detail::_unwrap([sched = sched]{ return sched; }, on_error(do_not_unwrap, std::move(sched), std::forward<F>(f)));
        }

        template<typename F>
        auto then(do_not_unwrap_type, F && f) -> future<decltype(std::forward<F>(f)(std::declval<T>()))>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->then(std::forward<F>(f));
        }

        template<typename F>
        auto then(F && f)
        {
            return _detail::_unwrap(&default_executor, then(do_not_unwrap, std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, F && f) -> future<decltype(std::forward<F>(f)(std::declval<std::exception_ptr>()))>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->on_error(std::forward<F>(f));
        }

        template<typename F>
        auto on_error(F && f)
        {
            return _detail::_unwrap(&default_executor, on_error(do_not_unwrap, std::forward<F>(f)));
        }

        void detach()
        {
            _add_count();
            then([keep = _state](auto &&){});
        }

        auto scheduler() const
        {
            std::lock_guard<std::mutex> lock{ _state->lock };
            return _state->scheduler;
        }

    private:
        std::shared_ptr<_detail::_shared_state<T>> _state;
    };

    template<typename T>
    auto make_ready_future(T && t)
    {
        return future<T>{ std::forward<T>(t) };
    }

    template<>
    class future<void>
    {
        future(std::shared_ptr<_detail::_shared_state<void>> state) : _state{ std::move(state) }
        {
            _add_count();
        }

        void _add_count() const
        {
            if (_state)
            {
                ++_state->shared_count;
            }
        }

        void _remove_count() const
        {
            if (_state)
            {
                --_state->shared_count;
            }
        }

    public:
        using value_type = void;

        template<typename F>
        friend auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>;

        future(const future & other) : _state{ other._state }
        {
            _add_count();
        }

        future(future && other) noexcept : _state{ std::move(other._state) }
        {
            other._state = {};
        }

        future & operator=(const future & other)
        {
            _remove_count();
            _state = other._state;
            _add_count();

            return *this;
        }

        future & operator=(future && other) noexcept
        {
            _remove_count();
            _state = std::move(other._state);
            other._state = {};

            return *this;
        }

        explicit future(ready_type) : future{ std::make_shared<_detail::_shared_state<void>>(ready) }
        {
        }

        ~future()
        {
            _remove_count();
        }

        bool try_get()
        {
            return _state->try_get();
        };

        template<typename F>
        auto then(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f) -> future<decltype(std::forward<F>(f)())>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->then(std::move(sched), std::forward<F>(f));
        }

        template<typename F>
        auto then(std::shared_ptr<executor> sched, F && f)
        {
            return _detail::_unwrap([sched = sched]{ return sched; }, then(do_not_unwrap, std::move(sched), std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f) -> future<decltype(std::forward<F>(f)(std::declval<std::exception_ptr>()))>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->on_error(std::move(sched), std::forward<F>(f));
        }

        template<typename F>
        auto on_error(std::shared_ptr<executor> sched, F && f)
        {
            return _detail::_unwrap([sched = sched]{ return sched; }, on_error(do_not_unwrap, std::move(sched), std::forward<F>(f)));
        }

        template<typename F>
        auto then(do_not_unwrap_type, F && f) -> future<decltype(std::forward<F>(f)())>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->then(std::forward<F>(f));
        }

        template<typename F>
        auto then(F && f)
        {
            return _detail::_unwrap(&default_executor, then(do_not_unwrap, std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, F && f) -> future<decltype(std::forward<F>(f)(std::declval<std::exception_ptr>()))>
        {
            if (!_state)
            {
                assert(!"handle this somehow (new exception type!)");
            }

            return _state->on_error(std::forward<F>(f));
        }

        template<typename F>
        auto on_error(F && f)
        {
            return _detail::_unwrap(&default_executor, on_error(do_not_unwrap, std::forward<F>(f)));
        }

        void detach()
        {
            _add_count();
            then([keep = _state]{});
        }

        auto scheduler() const
        {
            std::lock_guard<std::mutex> lock{ _state->lock };
            return _state->scheduler;
        }

    private:
        std::shared_ptr<_detail::_shared_state<void>> _state;
    };

    inline auto make_ready_future()
    {
        return future<>{ ready };
    }

    template<typename T>
    struct future_package_pair
    {
        class packaged_task<T> packaged_task;
        class future<T> future;
    };

    template<typename F>
    auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>
    {
        using T = decltype(std::forward<F>(f)());

        auto state = std::make_shared<_detail::_shared_state<T>>();
        state->function = std::forward<F>(f);

        return future_package_pair<T>{ { state }, { std::move(state) } };
    };

    namespace _detail
    {
        template<typename T>
        struct _replace_void
        {
            using type = T;
        };

        template<>
        struct _replace_void<void>
        {
            using type = ready_type;
        };

        template<typename T>
        using _manual_promise_state = variant<typename _replace_void<T>::type, std::exception_ptr, none_t>;
    }

    template<typename T>
    class manual_promise
    {
    public:
        manual_promise(std::shared_ptr<_detail::_manual_promise_state<T>> state, packaged_task<T> task) : _state{ std::move(state) }, _task{ std::move(task) }
        {
        }

        manual_promise(const manual_promise &) = default;
        manual_promise(manual_promise &&) = default;
        manual_promise & operator=(const manual_promise &) = default;
        manual_promise & operator=(manual_promise &&) = default;

        void set(std::shared_ptr<executor> sched, typename _detail::_replace_void<T>::type value = {}) const
        {
            *_state = std::move(value);
            sched->push([sched, task = std::move(_task)]{ task(sched); });
        }

        void set(typename _detail::_replace_void<T>::type value = {}) const
        {
            set(default_executor(), std::move(value));
        }

        void set(std::shared_ptr<executor> sched, std::exception_ptr ex) const
        {
            *_state = std::move(ex);
            sched->push([sched, task = std::move(_task)]{ task(sched); });
        }

        void set(std::exception_ptr ex) const
        {
            set(default_executor(), ex);
        }

    private:
        std::shared_ptr<_detail::_manual_promise_state<T>> _state;
        packaged_task<T> _task;
    };

    template<typename T>
    struct future_promise_pair
    {
        manual_promise<T> promise;
        class future<T> future;
    };

    template<typename T>
    auto make_promise()
    {
        auto state = std::make_shared<_detail::_manual_promise_state<T>>(none);
        auto packaged = package([state]{
            assert(state->index() != 2);
            if (state->index() == 1)
            {
                std::rethrow_exception(get<1>(*state));
            }

            return get<0>(std::move(*state));
        });

        return future_promise_pair<T>{ { std::move(state), std::move(packaged.packaged_task) }, std::move(packaged.future) };
    }

    template<>
    auto make_promise<void>()
    {
        auto state = std::make_shared<_detail::_manual_promise_state<ready_type>>(none);
        auto packaged = package([state]{
            assert(state->index() != 2);
            if (state->index() == 1)
            {
                std::rethrow_exception(get<1>(*state));
            }
        });

        return future_promise_pair<void>{ { std::move(state), std::move(packaged.packaged_task) }, std::move(packaged.future) };
    }

    template<typename T>
    future<T> join(std::shared_ptr<executor> sched, future<future<T>> fut)
    {
        auto pair = make_promise<T>();
        fut.then(sched, [promise = std::move(pair.promise), sched](auto && inner) {
            inner.then(sched, [promise = std::move(promise), sched](typename _detail::_replace_void<T>::type value = {}) {
                promise.set(std::move(sched), std::move(value));
            }).detach();
        }).detach();

        return std::move(pair.future);
    }

    template<typename T>
    future<T> join(future<future<T>> fut)
    {
        return join(default_executor(), std::move(fut));
    }

    namespace _detail
    {
        template<typename T>
        struct _is_not_void : std::integral_constant<bool, !std::is_void<T>::value> {};

        template<typename... Args>
        using _optional_tuple = std::tuple<optional<Args>...>;

        template<typename... Args, std::size_t... I>
        auto _remove_optional(std::tuple<optional<Args>...> & opt, std::index_sequence<I...>)
        {
            return std::tuple<Args...>{ std::move(*get<I>(opt))... };
        }

        template<std::size_t N>
        using _int = std::integral_constant<std::size_t, N>;
    }

    enum class exception_policy
    {
        aggregate,
        abort_on_first_failure
    };

    template<typename... Ts, typename std::enable_if<(sizeof...(Ts) > 0), int>::type = 0>
    auto when_all(std::shared_ptr<executor> sched, exception_policy policy, future<Ts>... futures)
    {
        using nonvoid = tpl::filter<
            tpl::vector<Ts...>,
            _detail::_is_not_void
        >;

        using return_type = tpl::rebind<
            nonvoid,
            std::tuple
        >;

        using buffer_type = tpl::rebind<
            nonvoid,
            _detail::_optional_tuple
        >;

        struct internal_state
        {
            buffer_type buffer;
            std::atomic<std::size_t> remaining{ sizeof...(Ts) };
            optional<packaged_task<return_type>> task;
            std::vector<future<>> futures;
            exception_list exceptions;
        };

        auto state = std::make_shared<internal_state>();
        auto pair = package([state](){
            state->futures = {};

            if (state->exceptions.size())
            {
                throw std::move(state->exceptions);
            }

            return _detail::_remove_optional(state->buffer, std::make_index_sequence<nonvoid::size>());
        });
        state->task = std::move(pair.packaged_task);

        auto void_handler = [state, sched]() {
            if (--state->remaining == 0)
            {
                sched->push([sched, task = std::move(*state->task)](){ task(sched); });
            }
        };

        auto nonvoid_handler = [&](auto index) {
            return [state, sched](auto value) {
                get<decltype(index)::value>(state->buffer) = std::move(value);

                if (--state->remaining == 0)
                {
                    sched->push([sched, task = std::move(*state->task)](){ task(sched); });
                }
            };
        };

        auto on_error = [state, sched, policy](auto exception_ptr) {
            switch (policy)
            {
                case exception_policy::aggregate:
                    state->exceptions.push_back(exception_ptr);
                    if (--state->remaining == 0)
                    {
                        sched->push([sched, task = std::move(*state->task)](){ task(sched); });
                    }
                    break;

                case exception_policy::abort_on_first_failure:
                    assert(!"implement this shit");
                    break;
            }
        };

        auto handler = make_overload_set(
            [](auto /* self */, auto /* index */){},

            [&](auto self, auto index, future<void> & vf, auto &... rest) {
                auto scheduler = vf.scheduler() ? vf.scheduler() : sched;

                state->futures.push_back(vf.then(std::move(scheduler), void_handler));
                state->futures.push_back(vf.on_error(std::move(scheduler), on_error));

                self(self, _detail::_int<index>(), rest...);
            },

            [&](auto self, auto index, auto & nvf, auto &... rest) {
                auto scheduler = nvf.scheduler() ? nvf.scheduler() : sched;

                state->futures.push_back(nvf.then(std::move(scheduler), nonvoid_handler(index)));
                state->futures.push_back(nvf.on_error(std::move(scheduler), on_error));

                self(self, _detail::_int<index + 1>(), rest...);
            }
        );

        handler(handler, _detail::_int<0>(), futures...);

        return std::move(pair.future);
    }

    template<typename... Ts, typename std::enable_if<(sizeof...(Ts) > 0), int>::type = 0>
    auto when_all(exception_policy policy, future<Ts>... futures)
    {
        return when_all(default_executor(), policy, std::move(futures)...);
    }

    template<typename... Ts, typename std::enable_if<(sizeof...(Ts) > 0), int>::type = 0>
    auto when_all(std::shared_ptr<executor> sched, future<Ts>... futures)
    {
        return when_all(std::move(sched), exception_policy::aggregate, std::move(futures)...);
    }

    template<typename... Ts, typename std::enable_if<(sizeof...(Ts) > 0), int>::type = 0>
    auto when_all(future<Ts>... futures)
    {
        return when_all(exception_policy::aggregate, std::move(futures)...);
    }

    inline auto when_all(exception_policy = exception_policy::aggregate)
    {
        return make_ready_future();
    }

    template<typename T>
    auto when_all(std::shared_ptr<executor> sched, exception_policy policy, const std::vector<future<T>> & futures)
    {
        using value_type = std::conditional_t<
            std::is_void<T>::value,
            ready_type,
            std::vector<T>
        >;

        using task_type = std::conditional_t<
            std::is_void<T>::value,
            void,
            std::vector<T>
        >;

        if (futures.size() == 0)
        {
            return future<task_type>(value_type{});
        }

        struct internal_state
        {
            std::atomic<std::size_t> remaining;
            optional<packaged_task<task_type>> task;
            std::vector<future<T>> futures;
            std::vector<future<>> keep_alive;
            exception_list exceptions;
        };

        if (!sched)
        {
            assert(!"what do");
        }

        auto state = std::make_shared<internal_state>();
        state->remaining = futures.size();

        auto to_package = make_overload_set(
            [&](auto) {
                return [state]() {
                    if (state->exceptions.size())
                    {
                        throw std::move(state->exceptions);
                    }

                    auto ret = fmap(state->futures, [](auto & future){ return *future.try_get(); });
                    state->futures = {};
                    return ret;
                };
            },

            [&](id<void>) {
                return [state]() {
                    state->futures = {};

                    if (state->exceptions.size())
                    {
                        throw std::move(state->exceptions);
                    }
                };
            }
        );

        auto pair = package(to_package(id<T>()));
        state->task = std::move(pair.packaged_task);
        state->futures = futures;
        state->keep_alive.reserve(futures.size());

        fmap(state->futures, [&](auto & future) {
            auto scheduler = future.scheduler() ? future.scheduler() : sched;

            state->keep_alive.push_back(future.then(std::move(scheduler), [sched, state](auto &&... arg) {
                // GCC fails to understand that the argument pack can be empty...
                // ...unless it has a name
                // WTF.
                swallow{ arg... };

                if (--state->remaining == 0)
                {
                    sched->push([sched, task = std::move(*state->task)](){ task(sched); });
                }
            }));

            state->keep_alive.push_back(future.on_error([state, sched, policy](auto exception_ptr) {
                switch (policy)
                {
                    case exception_policy::aggregate:
                        state->exceptions.push_back(exception_ptr);
                        if (--state->remaining == 0)
                        {
                            sched->push([sched, task = std::move(*state->task)](){ task(sched); });
                        }
                        break;

                    case exception_policy::abort_on_first_failure:
                        assert(!"implement this shit");
                        break;
                }
            }));

            return unit{};
        });

        return std::move(pair.future);
    }

    template<typename T>
    auto when_all(exception_policy policy, const std::vector<future<T>> & futures)
    {
        return when_all(default_executor(), policy, futures);
    }

    template<typename T>
    auto when_all(const std::vector<future<T>> & futures)
    {
        return when_all(exception_policy::aggregate, futures);
    }

    template<typename T>
    auto when_all(std::shared_ptr<executor> sched, const std::vector<future<T>> & futures)
    {
        return when_all(std::move(sched), exception_policy::aggregate, futures);
    }

    namespace _detail
    {
        // I wish C++ just allowed generalized lambda captures on packs
        // why the heck isn't that a thing, C++?
        template<typename F, typename... Args, std::size_t... Is>
        auto _async_impl(std::index_sequence<Is...>, std::shared_ptr<executor> scheduler, F && f, Args &&... args)
        {
            auto pair = package([f = std::forward<F>(f), args = std::forward_as_tuple<Args...>(std::forward<Args>(args)...)]() mutable {
                return std::forward<F>(f)(std::forward<Args>(std::get<Is>(args))...);
            });

            scheduler->push([scheduler = scheduler, task = std::move(pair.packaged_task)](){ task(scheduler); });

            return std::move(pair.future);
        }
    }

    template<typename F, typename... Args>
    auto async(std::shared_ptr<executor> scheduler, F && f, Args &&... args)
    {
        return _detail::_async_impl(std::make_index_sequence<sizeof...(Args)>(), std::move(scheduler), std::forward<F>(f), std::forward<Args>(args)...);
    }

    template<typename F, typename... Args>
    auto async(F && f, Args &&... args)
    {
        return _detail::_async_impl(std::make_index_sequence<sizeof...(Args)>(), default_executor(), std::forward<F>(f), std::forward<Args>(args)...);
    }
}}

