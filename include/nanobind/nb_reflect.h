/*
    nanobind/nb_reflect.h: Automatic binding via C++26 static reflection

    Requires GCC with -std=c++26 -freflection

    Copyright (c) 2025 Matthew Kolbe

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#if defined(__cpp_reflection) || defined(__cpp_impl_reflection)

#include <meta>
#include "nanobind.h"
#include "stl/string.h"
#include <string_view>
#include <type_traits>

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)

template <typename T, std::meta::info mem>
void reflect_bind_member(auto& cls) {
    constexpr auto name =
        std::define_static_string(std::meta::identifier_of(mem));
    using MemberType = [:std::meta::type_of(mem):];
    cls.def_prop_rw(name,
        [](T& self) -> MemberType& { return self.[:mem:]; },
        [](T& self, const MemberType& val) { self.[:mem:] = val; }
    );
}

template <typename T, std::meta::info fn, typename FnType>
struct reflect_method_binder;

template <typename T, std::meta::info fn, typename Ret, typename... Args>
struct reflect_method_binder<T, fn, Ret(Args...)> {
    static void bind(auto& cls) {
        constexpr auto name =
            std::define_static_string(std::meta::identifier_of(fn));
        cls.def(name, [](T& self, Args... args) -> Ret {
            return self.[:fn:](std::forward<Args>(args)...);
        });
    }
};

template <typename T, std::meta::info fn, typename Ret, typename... Args>
struct reflect_method_binder<T, fn, Ret(Args...) const> {
    static void bind(auto& cls) {
        constexpr auto name =
            std::define_static_string(std::meta::identifier_of(fn));
        cls.def(name, [](const T& self, Args... args) -> Ret {
            return self.[:fn:](std::forward<Args>(args)...);
        });
    }
};

template <typename T, std::meta::info fn>
void reflect_bind_method(auto& cls) {
    using FnType = [:std::meta::type_of(fn):];
    reflect_method_binder<T, fn, FnType>::bind(cls);
}

template <typename T>
void reflect_class(module_& m) {
    constexpr auto name =
        std::define_static_string(std::meta::identifier_of(^^T));

    auto cls = class_<T>(m, name)
        .def(init<>());

    template for (constexpr auto mem :
        std::define_static_array(std::meta::nonstatic_data_members_of(
            ^^T, std::meta::access_context::unchecked()))) {
        if constexpr (std::meta::is_public(mem)) {
            reflect_bind_member<T, mem>(cls);
        }
    };

    template for (constexpr auto fn :
        std::define_static_array(std::meta::members_of(
            ^^T, std::meta::access_context::unchecked()))) {
        if constexpr (std::meta::is_function(fn)
            && std::meta::is_public(fn)
            && !std::meta::is_static_member(fn)
            && !std::meta::is_constructor(fn)
            && !std::meta::is_destructor(fn)
            && !std::meta::is_special_member_function(fn)) {
            reflect_bind_method<T, fn>(cls);
        }
    };
}

template <typename E>
void reflect_enum(module_& m) {
    constexpr auto name =
        std::define_static_string(std::meta::identifier_of(^^E));

    auto e = enum_<E>(m, name);

    template for (constexpr auto val :
        std::define_static_array(std::meta::enumerators_of(^^E))) {
        constexpr auto vname =
            std::define_static_string(std::meta::identifier_of(val));
        e.value(vname, [:val:]);
    };
}

template <std::meta::info r>
void reflect_dispatch(module_& m) {
    if constexpr (std::meta::is_namespace(r)) {
        template for (constexpr auto mem :
            std::define_static_array(std::meta::members_of(
                r, std::meta::access_context::unchecked()))) {
            if constexpr (std::meta::is_type(mem)
                && std::meta::is_class_type(mem)) {
                reflect_class<typename [:mem:]>(m);
            } else if constexpr (std::meta::is_type(mem)
                && std::meta::is_enum_type(mem)) {
                reflect_enum<typename [:mem:]>(m);
            }
        };
    } else if constexpr (std::meta::is_type(r)
        && std::meta::is_class_type(r)) {
        reflect_class<typename [:r:]>(m);
    } else if constexpr (std::meta::is_type(r)
        && std::meta::is_enum_type(r)) {
        reflect_enum<typename [:r:]>(m);
    }
}

NAMESPACE_END(detail)

/// Automatically bind classes, enums, and namespaces via C++26 reflection.
///
/// Pass any mix of reflected types and namespaces:
///   nb::reflect_<^^Point, ^^Player, ^^Color, ^^game>(m);
///
template <std::meta::info... Rs>
void reflect_(module_& m) {
    (detail::reflect_dispatch<Rs>(m), ...);
}

NAMESPACE_END(NB_NAMESPACE)

#endif // __cpp_reflection || __cpp_impl_reflection
