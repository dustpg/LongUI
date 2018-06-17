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

#include "ui_object.h"
#include "../util/ui_function.h"

// ui namespace
namespace LongUI {
    // Time Capsule Return Code, 
    enum TimeCapsuleReturnCode : bool {
        Code_UntilFinished  = false,
        Code_Terminate      = true,
    };
    // impl namespace
    namespace impl {
        // time capsule pod data
        struct pod_time_capsule {
            // callback pointer
            void*           call;
            // time total
            float           total;
            // time done
            float           done;
        };
    }
    // time capsule
    class CUITimeCapsule final : impl::pod_time_capsule {
    public:
        // callback
        using TimeCallBack = CUIFunction<TimeCapsuleReturnCode(float)>;
        // dispose this time capsule
        void Dispose() noexcept;
    public:
        // add one time capsule to longui
        static auto AddOne(TimeCallBack&& call, float time) noexcept ->CUITimeCapsule*;
    private:
        //// call back
        //TimeCallBack        m_call;
        //// time total
        //const float         m_fTimeTotal;
        //// time done
        //float               m_fTimeDone = 0.f;
    };
}
