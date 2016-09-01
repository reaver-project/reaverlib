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

    template<typename T = void>
    class future;

    template<typename T>
    struct future_package_pair;

    template<typename F>
    auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>;

    template<typename T>
    struct future_promise_pair;

    template<typename T>
    future_promise_pair<T> make_promise();

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
            state.exceptional_continuations.emplace_back(std::forward<F>(f));
        }

        template<typename T>
        bool _is_valid(_shared_state<T> & state)
        {
            return state.value.index() != 2 || state.function;
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
        struct _wrap_impl
        {
            template<typename F>
            static auto wrap(F && f)
            {
                return std::forward<F>(f);
            }
        };

        template<>
        struct _wrap_impl<void>
        {
            template<typename F>
            static auto wrap(F && f)
            {
                return [f = std::forward<F>(f)](ready_type) mutable {
                    return std::forward<F>(f)();
                };
            }
        };

        template<typename T, typename F>
        auto _wrap(F && f)
        {
            return _wrap_impl<T>::wrap(std::forward<F>(f));
        }

        template<typename T>
        struct _shared_state : public std::enable_shared_from_this<_shared_state<T>>
        {
            using _replaced = typename _replace_void<T>::type;
            using then_t = std::conditional_t<
                std::is_copy_constructible<_replaced>::value,
                std::vector<_continuation_type>,
                optional<_continuation_type>
            >;
            using failed_then_t = std::vector<_continuation_type>;

            _shared_state(_replaced t) : value{ std::move(t) }
            {
            }

            _shared_state(std::exception_ptr ptr) : value{ ptr }
            {
            }

            _shared_state() : value{ none }
            {
            }

            std::mutex lock;
            variant<_replaced, std::exception_ptr, none_t> value;
            std::atomic<std::size_t> promise_count{ 0 };
            std::atomic<std::size_t> shared_count{ 0 };

            std::shared_ptr<executor> scheduler;

            std::mutex continuations_lock;
            then_t continuations;
            failed_then_t exceptional_continuations;

            // TODO: reaver::function
            optional<class function<T ()>> function;

            optional<_replaced> try_get()
            {
                std::lock_guard<std::mutex> l(lock);
                return get<0>(fmap(_move_or_copy(value, shared_count == 1 && value.index() != 1), make_overload_set(
                    [&](_replaced t) {
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
                        return optional<_replaced>();
                    }
                )));
            }

            void set(_replaced v)
            {
                {
                    std::unique_lock<std::mutex> l{ lock };
                    value = std::move(v);
                }

                _invoke_continuations();
            }

            void set(std::exception_ptr ex)
            {
                {
                    std::unique_lock<std::mutex> l{ lock };
                    value = std::move(ex);
                }

                auto conts = failed_then_t{};

                while (
                    [&]{
                        std::lock_guard<std::mutex> lock{ continuations_lock };
                        return !exceptional_continuations.empty();
                    }())
                {
                    {
                        std::lock_guard<std::mutex> lock{ continuations_lock };
                        using std::swap;
                        swap(conts, exceptional_continuations);
                    }

                    fmap(conts, [&](auto && cont) { cont(); return unit{}; });
                }

                _invoke_continuations();
            }

        private:
            void _invoke_continuations()
            {
                auto conts = then_t{};

                static_if(std::is_copy_constructible<_replaced>{}, [&](auto) {
                    // shenanigans
                    while (
                        [&]{
                            std::lock_guard<std::mutex> lock{ continuations_lock };
                            return !continuations.empty();
                        }())
                    {
                        {
                            std::lock_guard<std::mutex> lock{ continuations_lock };
                            using std::swap;
                            swap(conts, continuations);
                        }

                        fmap(conts, [&](auto && cont) { cont(); return unit{}; });
                    }
                }).static_else([&](auto) {
                    std::lock_guard<std::mutex> lock{ continuations_lock };
                    fmap(continuations, [&](auto && cont) { cont(); return unit{}; });
                });

                function = none;
                continuations = then_t{};
                exceptional_continuations = failed_then_t{};
            }

            _replaced _get()
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
            auto then(std::shared_ptr<executor> provided_sched, F && f) -> future<decltype(_wrap<T>(std::forward<F>(f))(std::declval<_replaced>()))>
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
                    [&](variant<const _replaced &, std::exception_ptr>) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            return _wrap<T>(std::forward<F>(f))(_get());
                        });

                        // GCC is deeply confused when this is directly in the capture list
                        auto state = std::enable_shared_from_this<_shared_state>::shared_from_this();

                        sched()->push([sched, task = std::move(pair.packaged_task), state = std::move(state)](){ task(sched()); });
                        return std::move(pair.future);
                    },

                    [&](none_t) {
                        auto pair = package([this, f = std::forward<F>(f)]() mutable {
                            return _wrap<T>(std::forward<F>(f))(_get());
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
                    [&](const _replaced &) {
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

        template<typename T>
        class _future_ptr
        {
            void _add_count() const
            {
                if (_ptr)
                {
                    ++_ptr->shared_count;
                }
            }

            void _remove_count() const
            {
                if (_ptr)
                {
                    --_ptr->shared_count;
                }
            }

        public:
            _future_ptr(std::shared_ptr<_shared_state<T>> ptr) : _ptr{ std::move(ptr) }
            {
                _add_count();
            }

            _future_ptr(const _future_ptr & other) : _ptr{ other._ptr }
            {
                _add_count();
            }

            _future_ptr(_future_ptr && other) noexcept : _ptr{ std::move(other._ptr) }
            {
                other._ptr = {};
            }

            _future_ptr & operator=(const _future_ptr & other)
            {
                _remove_count();
                _ptr = other._ptr;
                _add_count();

                return *this;
            }

            _future_ptr & operator=(_future_ptr && other) noexcept
            {
                _remove_count();
                _ptr = std::move(other._ptr);
                other._ptr = {};

                return *this;
            }

            ~_future_ptr()
            {
                _remove_count();
            }

            operator bool() const
            {
                return _ptr != nullptr;
            }

            auto operator*()
            {
                return *_ptr;
            }

            auto operator*() const
            {
                return *_ptr;
            }

            auto operator->()
            {
                return _ptr.operator->();
            }

            auto operator->() const
            {
                return _ptr.operator->();
            }

        private:
            std::shared_ptr<_shared_state<T>> _ptr;
        };

        template<typename T>
        class _promise_ptr
        {
            void _add_promise()
            {
                auto ptr = _ptr.lock();
                if (ptr)
                {
                    _detail::_add_promise(*ptr);
                }
            }

            void _remove_promise()
            {
                auto ptr = _ptr.lock();
                if (ptr)
                {
                    _detail::_remove_promise(*ptr);
                }
            }

        public:
            _promise_ptr(std::weak_ptr<_shared_state<T>> ptr) : _ptr{ std::move(ptr) }
            {
                _add_promise();
            }

            _promise_ptr(const _promise_ptr & other) : _ptr{ other._ptr }
            {
                _add_promise();
            }

            _promise_ptr(_promise_ptr && other) noexcept : _ptr{ other._ptr }
            {
                other._ptr = {};
            }

            _promise_ptr & operator=(const _promise_ptr & other)
            {
                _remove_promise();
                _ptr = other._ptr;
                _add_promise();
            }

            _promise_ptr & operator=(_promise_ptr && other) noexcept
            {
                _remove_promise();
                _ptr = other._ptr;
                other._ptr = {};
            }

            ~_promise_ptr()
            {
                _remove_promise();
            }

            auto lock() const
            {
                return _ptr.lock();
            }

        private:
            std::weak_ptr<_shared_state<T>> _ptr;
        };
    }

    template<typename T>
    class packaged_task
    {
        packaged_task(std::weak_ptr<_detail::_shared_state<T>> ptr) : _state{ std::move(ptr) }
        {
        }

    public:
        template<typename F>
        friend auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>;

        packaged_task(const packaged_task &) = default;
        packaged_task(packaged_task &&) = default;
        packaged_task & operator=(const packaged_task &) = default;
        packaged_task & operator=(packaged_task &&) = default;

        void operator()(std::shared_ptr<executor> sched = nullptr) const
        {
            auto state = _state.lock();

            if (!state)
            {
                return;
            }

            state->scheduler = std::move(sched);

            try
            {
                auto && value = (*state->function)();
                state->set(std::move(value));
            }
            catch (...)
            {
                state->set(std::current_exception());
            }
        }

    private:
        _detail::_promise_ptr<T> _state;
    };

    template<>
    inline void packaged_task<void>::operator()(std::shared_ptr<executor> sched) const
    {
        auto state = _state.lock();

        if (!state)
        {
            return;
        }

        state->scheduler = std::move(sched);

        try
        {
            (*state->function)();
            state->set(ready);
        }
        catch (...)
        {
            state->set(std::current_exception());
        }
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
        }

    public:
        using value_type = T;

        template<typename F>
        friend auto package(F && f) -> future_package_pair<decltype(std::forward<F>(f)())>;

        template<typename U>
        friend future_promise_pair<U> make_promise();

        future(const future &) = default;
        future(future &&) = default;
        future & operator=(const future &) = default;
        future & operator=(future &&) = default;

        explicit future(T value) : future{ std::make_shared<_detail::_shared_state<T>>(std::move(value)) }
        {
        }

        explicit future(std::exception_ptr ex) : future{ std::make_shared<_detail::_shared_state<T>>(std::move(ex)) }
        {
        }

        auto try_get()
        {
            return _state->try_get();
        };

        template<typename F>
        auto then(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f)
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
            return _detail::_unwrap([sched = sched]{ return sched; }, then(do_not_unwrap, sched, std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f)
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
            return _detail::_unwrap([sched = sched]{ return sched; }, on_error(do_not_unwrap, sched, std::forward<F>(f)));
        }

        template<typename F>
        auto then(do_not_unwrap_type, F && f)
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
            return _detail::_unwrap([]{ return default_executor(); }, then(do_not_unwrap, std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, F && f)
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
            return _detail::_unwrap([]{ return default_executor(); }, on_error(do_not_unwrap, std::forward<F>(f)));
        }

        void detach()
        {
            then([keep = _state](auto &&){});
        }

        auto scheduler() const
        {
            std::lock_guard<std::mutex> lock{ _state->lock };
            return _state->scheduler;
        }

    private:
        _detail::_future_ptr<T> _state;
    };

    template<typename T>
    auto make_ready_future(T && t)
    {
        return future<std::remove_reference_t<T>>{ std::forward<T>(t) };
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

        template<typename U>
        friend future_promise_pair<U> make_promise();

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

        explicit future(std::exception_ptr ex) : future{ std::make_shared<_detail::_shared_state<void>>(std::move(ex)) }
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
        auto then(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f)
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
            return _detail::_unwrap([sched = sched]{ return sched; }, then(do_not_unwrap, sched, std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, std::shared_ptr<executor> sched, F && f)
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
            return _detail::_unwrap([sched = sched]{ return sched; }, on_error(do_not_unwrap, sched, std::forward<F>(f)));
        }

        template<typename F>
        auto then(do_not_unwrap_type, F && f)
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
            return _detail::_unwrap([]{ return default_executor(); }, then(do_not_unwrap, std::forward<F>(f)));
        }

        template<typename F>
        auto on_error(do_not_unwrap_type, F && f)
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
            return _detail::_unwrap([]{ return default_executor(); }, on_error(do_not_unwrap, std::forward<F>(f)));
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

    template<typename T, typename E>
    inline auto make_exceptional_future(E && e)
    {
        return future<T>(std::make_exception_ptr(std::move(e)));
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

    template<typename T>
    class manual_promise
    {
        manual_promise(std::weak_ptr<_detail::_shared_state<T>> state) : _state{ std::move(state) }
        {
        }

    public:
        template<typename U>
        friend future_promise_pair<U> make_promise();

        manual_promise(const manual_promise &) = default;
        manual_promise(manual_promise &&) = default;
        manual_promise & operator=(const manual_promise &) = default;
        manual_promise & operator=(manual_promise &&) = default;

        void set(typename _detail::_replace_void<T>::type value = {}) const
        {
            auto state = _state.lock();

            if (!state)
            {
                return;
            }

            state->set(std::move(value));
        }

        void set(std::exception_ptr ex) const
        {
            auto state = _state.lock();

            if (!state)
            {
                return;
            }

            state->set(std::move(ex));
        }

    private:
        _detail::_promise_ptr<T> _state;
    };

    template<typename T>
    struct future_promise_pair
    {
        manual_promise<T> promise;
        class future<T> future;
    };

    template<typename T>
    future_promise_pair<T> make_promise()
    {
        auto state = std::make_shared<_detail::_shared_state<T>>();
        state->function = []() -> T { assert(!"I need to handle this somehow"); };

        return { { state }, { (state) } };
    }

    template<typename T>
    future<T> join(std::shared_ptr<executor> sched, future<future<T>> fut)
    {
        auto pair = make_promise<T>();
        fut.then(sched, [promise = std::move(pair.promise), sched](auto && inner) {
            inner.then(std::move(sched), [promise = std::move(promise)](typename _detail::_replace_void<T>::type value = {}) {
                promise.set(std::move(value));
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

