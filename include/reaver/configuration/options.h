/**
 * Reaver Library Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
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

#include <cstddef>
#include <type_traits>
#include <memory>
#include <unordered_map>

#include <boost/program_options.hpp>
#include <boost/range/algorithm.hpp>

#include "../configuration.h"
#include "../swallow.h"
#include "../unit.h"
#include "../id.h"
#include "../logger.h"
#include "../exception.h"

namespace reaver
{
    namespace options { inline namespace _v1
    {
        static struct positional_type
        {
            constexpr positional_type()
            {
            }

            constexpr positional_type(bool specified, std::size_t position) : position_specified{ specified }, required_position{ position }
            {
            }

            constexpr positional_type operator()(std::size_t required_position) const
            {
                return { true, required_position };
            }

            bool position_specified = false;
            std::size_t required_position = 0;
        } positional;

        static struct required_type
        {
        } required;

        class duplicate_option : public exception
        {
        public:
            duplicate_option(const char * name) : exception{ logger::error }
            {
                *this << "attempted to register option `" << name << "` twice in the same option_registry.";
            }
        };

        class option_registry
        {
        public:
            template<typename Option, typename std::enable_if<Option::options.position_specified, int>::type = 0>
            void append()
            {
                if (boost::range::find_if(_positional_options, [](const auto & elem){ return dynamic_cast<_positional_impl<Option> *>(elem.get()) != nullptr; }) != _positional_options.end())
                {
                    throw duplicate_option{ Option::name };
                }

                _positional_options.push_back(std::make_unique<_positional_impl<Option>>());
            }

            template<typename Option, typename std::enable_if<!Option::options.position_specified, int>::type = 0>
            void append()
            {
                if (boost::range::find_if(_options, [](const auto & elem){ return dynamic_cast<_impl<Option> *>(elem.get()) != nullptr; }) != _options.end())
                {
                    throw duplicate_option{ Option::name };
                }

                _options.push_back(std::make_unique<_impl<Option>>());
            }

            boost::program_options::options_description generate_visible() const
            {
                boost::program_options::options_description desc;
                std::unordered_map<std::string, boost::program_options::options_description> sections;

                for (const auto & opt : _options)
                {
                    if (opt->visible())
                    {
                        sections[opt->section()].add(opt->generate());
                    }
                }

                for (auto & section : sections)
                {
                    desc.add(section.second);
                }

                return desc;
            }

            boost::program_options::options_description generate_hidden() const
            {
                boost::program_options::options_description desc;

                for (const auto & opt : _options)
                {
                    if (!opt->visible())
                    {
                        desc.add(opt->generate());
                    }
                }

                return desc;
            }

            boost::program_options::positional_options_description generate_positional() const
            {
                boost::program_options::positional_options_description desc;

                for (const auto & opt : _positional_options)
                {
                    opt->generate(desc);
                }

                return desc;
            }

        private:
            class _base
            {
            public:
                virtual ~_base()
                {
                }

                virtual boost::program_options::options_description generate() const = 0;
                virtual const char * section() const = 0;
                virtual bool visible() const = 0;
            };

            template<typename T>
            class _impl : public _base
            {
            public:
                virtual boost::program_options::options_description generate() const override
                {
                    boost::program_options::options_description desc;
                    desc.add_options()(T::name, boost::program_options::value<typename T::type>(), T::description);
                    return desc;
                }

                virtual const char * section() const override
                {
                    return T::options.has_section ? T::options.section : "";
                }

                virtual bool visible() const override
                {
                    return T::options.is_visible;
                }
            };

            class _positional_base
            {
            public:
                virtual ~_positional_base()
                {
                }

                virtual void generate(boost::program_options::positional_options_description &) const = 0;
                virtual positional_type position() const = 0;

                bool operator<(const _positional_base & other) const
                {
                    if (!position().position_specified || !other.position().position_specified)
                    {
                        return false;
                    }

                    return position().required_position < other.position().required_position;
                }
            };

            template<typename T>
            class _positional_impl : public _positional_base
            {
            public:
                virtual void generate(boost::program_options::positional_options_description & desc) const override
                {
                    desc.add(T::name, T::count);
                }

                virtual positional_type position() const override
                {
                    return { T::options.position.position_specified, T::options.position.required_position };
                }
            };

            std::vector<std::unique_ptr<_base>> _options;
            std::vector<std::unique_ptr<_positional_base>> _positional_options;
        };

        inline option_registry & default_option_registry()
        {
            static option_registry registry;
            return registry;
        }

        namespace _detail
        {
            template<typename OptionType, bool>
            struct _option_registrar
            {
                _option_registrar()
                {
                    try
                    {
                        default_option_registry().append<OptionType>();
                    }

                    catch (reaver::exception & ex)
                    {
                        ex.print(reaver::logger::default_logger());
                        std::exit(2);
                    }

                    catch (std::exception & ex)
                    {
                        reaver::logger::dlog(reaver::logger::crash) << ex.what();
                        std::exit(2);
                    }
                }
            };

            template<typename OptionType>
            struct _option_registrar<OptionType, false>
            {
            };
        }

        namespace _detail
        {
            template<typename T>
            struct _false_type : public std::false_type
            {
            };
        }

        struct option_set
        {
            template<typename... Args>
            constexpr option_set(Args &&... args)
            {
                swallow{ initialize(std::forward<Args>(args))... };
            }

            constexpr unit initialize(positional_type && pos)
            {
                position_specified = true;
                position = pos;
                return {};
            }

            constexpr unit initialize(required_type)
            {
                required = true;
                return {};
            }

            template<typename T>
            unit initialize(T &&)
            {
                static_assert(_detail::_false_type<T>(), "tried to use an unknown reaver::configuration option");
            }

            bool position_specified = false;
            positional_type position;
            bool required = false;
            bool is_visible = true;
            bool has_section = false;
            const char * section = nullptr;
        };

        template<typename CRTP, typename ValueType, bool Register = true>
        struct option
        {
            option()
            {
                (void)_registrar;
            }

            using type = ValueType;
            static const char * const name;
            static constexpr const char * description = "";
            static constexpr std::size_t count = 1;
            static constexpr option_set options = {};
            static constexpr bool is_void = false;
        private:
            static _detail::_option_registrar<CRTP, Register> _registrar;
        };

        template<typename CRTP, bool Register>
        struct option<CRTP, void, Register> : option<CRTP, bool, Register>
        {
            static constexpr bool is_void = true;
        };

        template<typename CRTP, typename ValueType>
        using opt = option<CRTP, ValueType, false>;

        template<typename CRTP, typename ValueType, bool Register>
        const char * const option<CRTP, ValueType, Register>::name = boost::typeindex::type_id<CRTP>().name();

        template<typename CRTP, typename ValueType, bool Register>
        _detail::_option_registrar<CRTP, Register> option<CRTP, ValueType, Register>::_registrar;

        inline auto parse_argv(int argc, char ** argv, const option_registry & registry = default_option_registry())
        {
            assert(false);
            // TODO: not implemented yet

            configuration ret;

            auto visible = registry.generate_visible();
            auto hidden = registry.generate_hidden();
            auto positional = registry.generate_positional();

            return ret;
        }

        namespace _detail
        {
            std::string _name(const char * full_name)
            {
                std::string buf{ full_name };
                return buf.substr(0, buf.find(','));
            }

            template<typename T, typename std::enable_if<T::is_void, int>::type = 0>
            unit _handle(boost::program_options::options_description & desc)
            {
                desc.add_options()(T::name, T::description);

                return {};
            }

            template<typename T, typename std::enable_if<!T::is_void, int>::type = 0>
            unit _handle(boost::program_options::options_description & desc)
            {
                desc.add_options()(T::name, boost::program_options::value<typename T::type>(), T::description);

                return {};
            }

            template<typename... Args>
            auto _handle_visible(id<Args>...)
            {
                boost::program_options::options_description desc;
                swallow{ (Args::options.is_visible ? _handle<Args>(desc) : unit{})... };
                return desc;
            }

            template<typename... Args>
            auto _handle_hidden(id<Args>...)
            {
                boost::program_options::options_description desc;
                swallow{ (!Args::options.is_visible ? _handle<Args>(desc) : unit{})... };
                return desc;
            }

            template<typename T>
            unit _handle_positional(boost::program_options::positional_options_description & desc)
            {
                return {};
            }

            template<typename... Args>
            auto _handle_positional(id<Args>...)
            {
                boost::program_options::positional_options_description desc;
                swallow{ _handle_positional<Args>(desc)... };
                return desc;
            }

            template<typename Config>
            auto _get(boost::program_options::variables_map & map, Config && config)
            {
                return std::forward<Config>(config);
            }

            template<typename Head, typename... Tail, typename Config, typename std::enable_if<Head::is_void, int>::type = 0>
            auto _get(boost::program_options::variables_map & map, Config && config)
            {
                return _get<Tail...>(map, std::forward<Config>(config).template add<Head>(map.count(_name(Head::name))));
            }

            template<typename Head, typename... Tail, typename Config, typename std::enable_if<!Head::is_void, int>::type = 0>
            auto _get(boost::program_options::variables_map & map, Config && config)
            {
                return _get<Tail...>(map, std::forward<Config>(config).template add<Head>(map[_name(Head::name)].template as<typename Head::type>()));
            }
        }

        template<typename... Args>
        auto parse_argv(int argc, const char ** argv, id<Args>... args)
        {
            auto visible = _detail::_handle_visible(args...);
            auto hidden = _detail::_handle_hidden(args...);
            auto positional = _detail::_handle_positional(args...);

            boost::program_options::options_description all;
            all.add(visible).add(hidden);

            boost::program_options::variables_map variables;
            boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(all)
                .style(boost::program_options::command_line_style::allow_short
                    | boost::program_options::command_line_style::allow_long
                    | boost::program_options::command_line_style::allow_sticky
                    | boost::program_options::command_line_style::allow_dash_for_short
                    | boost::program_options::command_line_style::long_allow_next
                    | boost::program_options::command_line_style::short_allow_next
                    | boost::program_options::command_line_style::allow_long_disguise).run(), variables);

            return _detail::_get<Args...>(variables, bound_configuration<>{});
        }
    }}
}

