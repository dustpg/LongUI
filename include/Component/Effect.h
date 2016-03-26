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
#include "../Graphics/luiGrD2d.h"
#include "../Platless/luiPlHlper.h"

// longui::component namespace
namespace LongUI { namespace Component {
    // Effect
    class Effect {
        // using
        using EffectID = Helper::InfomationPointer<const IID>;
    public:
        // ctor - with effect guid
        Effect(const IID& effectid) noexcept;
        // dtor
        ~Effect() noexcept { this->release(); }
        // render
        void Render() const noexcept;
        // recreate
        auto Recreate() noexcept ->HRESULT;
        // ok?
        auto IsOK() const noexcept { return !!m_pEffect; }
        // check if Invalidate
        bool CheckInvalidate() const noexcept { return m_pEffectID.Bool1(); }
        // set Invalidate
        void SetInvalidate() noexcept { return m_pEffectID.SetBool1(true); }
        // set Validate
        void SetValidate() noexcept { return m_pEffectID.SetBool1(false); }
        // user data
        bool GetUserData() const noexcept { return m_pEffectID.Bool2(); }
        // user data
        void SetUserData(bool b) noexcept { return m_pEffectID.SetBool2(b); }
    public:
        // set the value
        template<typename T, typename U> auto SetValue(U index, const T& value) noexcept {
            return m_pEffect->SetValue(index, value);
        }
        // get the value
        template<typename T, typename U> auto GetValue(U index) noexcept {
            return m_pEffect->GetValue<T>(index);
        }
    private:
        // release
        void release() noexcept;
    private:
        // id of effect
        EffectID                    m_pEffectID;
        // effect
        ID2D1Effect*                m_pEffect = nullptr;
        // command list
        ID2D1CommandList*           m_pCmdList = nullptr;
        // effect output
        ID2D1Image*                 m_pOutput = nullptr;
    public:
        // record
        template<typename T> void Record(ID2D1DeviceContext* target, T lam) noexcept {
            // if invalidate
            if (this->CheckInvalidate()) {
                // recreate cmd list
                LongUI::SafeRelease(m_pCmdList);
                target->CreateCommandList(&m_pCmdList);
                // created
                if (m_pCmdList) {
                    // draw
                    target->SetTarget(m_pCmdList);
                    target->BeginDraw();
                    draw_record();
                    target->EndDraw();
                    m_pCmdList->Close();
                    m_pEffect->SetInput(0, m_pCmdList);
                    // validate
                    this->SetValidate();
                }
            }
        }
    };
}}
