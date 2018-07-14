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
#include <utility>

// ui namespace
namespace LongUI {
    // control
    class UIControl;
    // time capsule
    class CUITimeCapsule : public Node, public CUISmallObject {
    protected:
        // ctor
        CUITimeCapsule(float total) noexcept;
        // dtor
        virtual ~CUITimeCapsule() noexcept = default;
        // call
        virtual void call(float p) noexcept = 0;
    public:
        // delete this
        void Dispose() noexcept;
        // call this
        bool Call(float delta) noexcept;
        // is more mo than
        bool IsMoreMOThan(const CUITimeCapsule& x) const noexcept;
        // set hoster
        void SetHoster(UIControl& hoster) noexcept { m_pHoster = &hoster; }
        // is same hoster
        bool IsSameHoster(UIControl& hoster) const noexcept { return m_pHoster == &hoster; }
    private:
        // pointer
        UIControl *         m_pHoster = nullptr;
        // total time
        float      const    m_fTotalTime;
        // time done
        float               m_fDoneTime = 0.f;
    };
    // impl namespace
    namespace impl {
        // impl for time capsule
        template<typename T>
        class time_capsule : public CUITimeCapsule {
            // func object
            T           m_func;
            // dtor
            ~time_capsule() noexcept = default;
            // impl for call
            void call(float p) noexcept override { return m_func(p); };
        public:
            // ctor
            time_capsule(T&& func, float total) noexcept : CUITimeCapsule(total), m_func(std::move(func)) {}
        };
        // create time capsule
        template<typename T>
        inline auto create(float total, T&& func) noexcept ->CUITimeCapsule* {
            return new(std::nothrow) impl::time_capsule<T>(std::move(func), total);
        }
    }
}
