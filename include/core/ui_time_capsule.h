#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ui_node.h"
#include "ui_object.h"
#include "../util/ui_ctordtor.h"

#include <utility>

// ui namespace
namespace LongUI {
    // control
    class UIControl;
    // contrl control
    class CUIControlControl;
    // time capsule
    class CUITimeCapsule : public Node, public CUISmallObject {
        // friend
        friend CUIControlControl;
    protected:
        // ctor
        CUITimeCapsule(
            void(* call)(void*, float p) noexcept,
            void(* dtor)(void*) noexcept,
            float total) noexcept;
        // dtor
        ~CUITimeCapsule() noexcept ;
        // delete this
        void dispose() noexcept;
        // normal check hoster last end
        void normal_check_hoster_last_end() noexcept;
    public:
        // force terminate
        void Terminate() noexcept;
        // restart
        void Restart() noexcept;
        // call this
        bool Call(float delta) noexcept;
        // is more mo than
        bool IsMoreMOThan(const CUITimeCapsule& x) const noexcept;
        // set hoster
        void SetHoster(UIControl& hoster) noexcept { m_pHoster = &hoster; }
        // is same hoster
        bool IsSameHoster(UIControl& hoster) const noexcept { return m_pHoster == &hoster; }
    private:
        // call ptr
        void(* const m_pCall)(void*, float p) noexcept;
        // dtor ptr
        void(* const m_pDtor)(void*) noexcept;
        // pointer
        UIControl *         m_pHoster = nullptr;
        // total time
        float      const    m_fTotalTime;
        // time done
        float               m_fDoneTime = 0.f;
    };
    // impl namespace
    namespace detail {
        // impl for time capsule
        template<typename T> struct time_capsule_helper : CUITimeCapsule {
            // alignof T cannot greater double(std::max_align_t)
            static_assert(alignof(T) <= alignof(double), "to large!");
            // T data
            typename std::aligned_storage<sizeof(T), alignof(T)>::type     buffer;
            // ctor
            time_capsule_helper(
                T&& func,
                void(*call)(void*, float p) noexcept,
                void(*dtor)(void*) noexcept,
                float total) noexcept : CUITimeCapsule(call, dtor, total) {
                detail::ctor_dtor<T>::create(&this->buffer, std::move(func));
            }
            // operator ()
            static void call(void* ptr, float p) noexcept {
                const auto obj = &reinterpret_cast<time_capsule_helper*>(ptr)->buffer;
                (*reinterpret_cast<T*>(obj))(p);
            }
        };
        // create time capsule
        template<typename T>
        inline auto create(float total, T&& func) noexcept ->CUITimeCapsule* {
            return new(std::nothrow) detail::time_capsule_helper<T>(
                std::move(func), 
                detail::time_capsule_helper<T>::call,
                detail::ctor_dtor<T>::delete_obj,
                total
                );
        }
    }
}
