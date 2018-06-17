#pragma once



#include "ui_unimacro.h"
#include <utility>


#ifndef LUI_CANNOT_CALL_CONSTRUCTOR_DIRECTLY

// MSVC  -> can call ctor
// Clang -> call ctor via -Wmicrosoft-explicit-constructor-call
// GCC   -> cannot call ctor

#if LUI_COMPILER == LUI_COMPILER_GCC
#define LUI_CANNOT_CALL_CONSTRUCTOR_DIRECTLY
#endif

#endif

#ifdef LUI_CANNOT_CALL_CONSTRUCTOR_DIRECTLY
#include <new>
#endif

// LongUI::detail namespace
namespace LongUI { namespace detail {
    // func-vtable helper
    struct vtable_helper {
        void(*create_obj)(void*) noexcept;
        void(*copy_t_obj)(void*, const void*) noexcept;
        void(*move_t_obj)(void*, void*) noexcept;
        void(*delete_obj)(void*) noexcept;
    };
    // empty0
    struct empty0 {};
    // func-vtable getter
    template<typename T> struct ctor_dtor {
        // member
        T       m;
        // ctor
        inline ctor_dtor() noexcept {};
        // dtor
        inline ~ctor_dtor() noexcept {};
        // copy ctor
        inline ctor_dtor(const ctor_dtor& a) noexcept : m(a.m) {};
        // move ctor
        inline ctor_dtor(ctor_dtor&& a) noexcept : m(std::move(a.m)) {};
        // create the object
        static void create_obj(void* ptr) noexcept {
#ifdef LUI_CANNOT_CALL_CONSTRUCTOR_DIRECTLY
            // gcc cannot call ctor directly
            new(ptr) ctor_dtor<T>();
#else
            // msc/clang extended support
            static_cast<ctor_dtor<T>*>(ptr)->ctor_dtor<T>::ctor_dtor();
#endif
        }
        // copy the object
        static void copy_t_obj(void* ptr, const void* x) noexcept {
            assert(ptr != x);
            static_cast<ctor_dtor<T>*>(ptr)->ctor_dtor<T>
                ::ctor_dtor(*static_cast<const ctor_dtor<T>*>(x));
        }
        // move the object
        static void move_t_obj(void* ptr, void* x) noexcept {
            assert(ptr != x);
            static_cast<ctor_dtor<T>*>(ptr)->ctor_dtor<T>
                ::ctor_dtor(std::move(*static_cast<ctor_dtor<T>*>(x)));
        }
        // release the object
        static void delete_obj(void*p) noexcept { static_cast<T*>(p)->T::~T(); }
        // get the vtable ptr
        static const vtable_helper* get() noexcept {
            static const vtable_helper helper{
                create_obj,  copy_t_obj, move_t_obj,  delete_obj };
            return &helper;
        }
        // ctor
        template<typename ...Args>
        ctor_dtor(empty0, Args&&... args) noexcept : m(std::forward<Args>(args)...) {};
        // create
        template<typename ...Args>
        static T* create(void* ptr, Args&&... args) noexcept {
#ifdef LUI_CANNOT_CALL_CONSTRUCTOR_DIRECTLY
            // gcc cannot call ctor directly
            return &(new(ptr) ctor_dtor<T>(empty0{}, std::forward<Args>(args)...))->m;
#else
                // msc/clang extended support
            static_cast<ctor_dtor<T>*>(ptr)->ctor_dtor<T>::ctor_dtor(empty0{}, std::forward<Args>(args)...);
                return static_cast<T*>(ptr);
#endif
        }
    };
}}

