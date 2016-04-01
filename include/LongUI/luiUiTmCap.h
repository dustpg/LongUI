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

#include "../luibase.h"
#include "../luiconf.h"
#include "../Platless/luiPlUtil.h"

// longui namespace
namespace LongUI {
    // time capsule
    class CUITimeCapsule final : public CUISingleSmallObject {
    public:
        // time callback, return true if want to terminate time capsule
        using TimeCallBack = CUIFunction<bool(float)>;
        // ctor
        CUITimeCapsule(TimeCallBack&& call, size_t identifier, float time);
        // update
        bool Update(float delta_time) noexcept;
        // dispose
        void Dispose() { delete this; }
        // get id
        auto GetId() const noexcept { return m_id; }
    public:
        // create one
        static auto Create(TimeCallBack&& call, size_t id, float time) noexcept {
            return new(std::nothrow) CUITimeCapsule(std::move(call), id, time);
        }
    private:
        // identifier
        size_t              m_id;
        // call back
        TimeCallBack        m_call;
        // time total
        const float         m_fTimeTotal;
        // time done
        float               m_fTimeDone = 0.f;
    };
}
