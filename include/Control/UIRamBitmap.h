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

#include "UIControl.h"
#include "../Component/Text.h"

// LongUI namespace
namespace LongUI {
    // display bitmap witch from RAM
    class UIRamBitmap final : public UIControl {
        // super class
        using Super = UIControl ;
        // clean this
        virtual void cleanup() noexcept override;
    public:
        // const expr
        enum : uint32_t { MIN_SIZE = 32, MIN_PITCH = 4 };
        // RGBA
        struct RGBA { uint8_t b, g, r, a; };
        // Render 渲染
        virtual void Render() const noexcept override;
        // update 刷新
        //virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // render chain -> background: do nothing
        void render_chain_background() const noexcept;
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept  { return Super::render_chain_foreground(); }
    public:
        // create 创建
        static auto CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIRamBitmap(UIContainer* cp) noexcept : Super(cp) {}
        // get picth width = (bitmap_width + 3) / 4 * 4;
        auto GetPicthWidth() const noexcept { return m_cPitchWidth; }
        // get bitmap width
        auto GetBitmapsSize() const noexcept { return m_szBitmap; }
        // redraw: call(RGBA* buffer, uint32_t pitch_width)
        template<typename T> auto Redraw(T call) noexcept {
            if (m_pBitmapData) {
                call(m_pBitmapData, m_cPitchWidth);
                auto hr = this->write_data();
                this->InvalidateThis();
                return hr;
            }
            return E_OUTOFMEMORY;
        }
        // set D2D1_INTERPOLATION_MODE
        void SetInterpolationMode(D2D1_INTERPOLATION_MODE mode) noexcept {
            if (m_modeInterpolation != mode) {
                m_modeInterpolation = mode;
                this->InvalidateThis();
            }
        }
        // resize  bitmap
        auto ResizeBitmap(D2D1_SIZE_U size) noexcept ->HRESULT;
    protected:
        // write data
        auto write_data() noexcept ->HRESULT;
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // dtor
        ~UIRamBitmap() noexcept;
        // copy ctor = delete
        UIRamBitmap(const UIRamBitmap&) = delete;
    protected:
        // size of bitmap
        D2D1_SIZE_U         m_szBitmap = D2D1_SIZE_U{ 128, 128 };
        // bitmap data
        RGBA*               m_pBitmapData = nullptr;
        // D2D1_INTERPOLATION_MODE
        uint32_t            m_modeInterpolation = 0;
        // pitch width
        uint32_t            m_cPitchWidth = 0;
        // bitmap interface
        ID2D1Bitmap1*       m_pBitmap = nullptr;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIRamBitmap>() noexcept;
#endif
}