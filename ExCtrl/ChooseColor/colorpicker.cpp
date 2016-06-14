#define _CRT_SECURE_NO_WARNINGS
#include "colorpicker.h"
#include <algorithm>
#include <LongUI.h>
#include <Control/UIColor.h>
#include <Control/UIViewport.h>
#include "../UIColorHsv/UIColorHsv.h"
#include "../UIColorHsv/UIColorButton.h"
#include <../3rdParty/lz4/lib/lz4hc.h>
#include <../3rdParty/lz4/lib/lz4.h>
#undef min
#undef max

// longui namespace
namespace LongUI {
    // layout in lz4
    const char* const COLOR_PCIKER_LZ4 =
        "\xf1\x5e\x3c\x3f\x78\x6d\x6c\x20\x76\x65\x72\x73\x69\x6f\x6e\x3d\x22\x31\x2e\x30\x22\x20\x65\x6e"
        "\x63\x6f\x64\x69\x6e\x67\x3d\x22\x75\x74\x66\x2d\x38\x22\x3f\x3e\x0a\x3c\x57\x69\x6e\x64\x6f\x77"
        "\x20\x73\x69\x7a\x65\x3d\x22\x38\x30\x30\x2c\x20\x36\x30\x30\x22\x20\x74\x69\x74\x6c\x65\x6e\x61"
        "\x6d\x65\x3d\x22\x4c\x6f\x6e\x67\x55\x49\x20\x48\x53\x56\x20\x43\x6f\x6c\x6f\x72\x20\x50\x69\x63"
        "\x6b\x65\x72\x22\x20\x6d\x61\x72\x67\x69\x6e\x3d\x22\x38\x2c\x02\x00\xf2\x0a\x22\x20\x3e\x0a\x20"
        "\x20\x20\x20\x3c\x48\x6f\x72\x69\x7a\x6f\x6e\x74\x61\x6c\x4c\x61\x79\x6f\x75\x74\x17\x00\x01\x1b"
        "\x00\x01\x41\x00\x00\x4a\x00\x02\x5b\x00\xf1\x04\x68\x73\x76\x4d\x61\x69\x6e\x22\x20\x68\x3d\x22"
        "\x73\x6c\x64\x48\x22\x20\x73\x09\x00\x41\x53\x22\x20\x76\x09\x00\x47\x56\x22\x0a\x20\x01\x00\x11"
        "\x72\x15\x00\x41\x52\x22\x20\x67\x09\x00\x41\x47\x22\x20\x62\x09\x00\x37\x42\x22\x2f\x65\x00\x64"
        "\x56\x65\x72\x74\x69\x63\x7e\x00\xf4\x00\x20\x62\x6f\x72\x64\x65\x72\x77\x69\x64\x74\x68\x3d\x22"
        "\x31\xa6\x00\x04\x53\x00\x0d\xae\x00\x03\x0b\x01\x6a\x30\x2c\x20\x33\x32\x22\x2c\x00\x01\x30\x00"
        "\xd3\x54\x65\x78\x74\x20\x74\x65\x78\x74\x3d\x22\x48\x22\x2d\x00\x7f\x33\x32\x2c\x20\x30\x22\x2f"
        "\x2e\x00\x00\x63\x53\x6c\x69\x64\x65\x72\xef\x00\x02\xe3\x00\x70\x65\x6e\x64\x3d\x22\x33\x36\x65"
        "\x01\x43\x68\x75\x6d\x62\x7a\x01\x36\x2c\x31\x36\x52\x01\x21\x34\x2c\x02\x00\x2f\x22\x20\x53\x00"
        "\x01\x43\x45\x64\x69\x74\x51\x00\x53\x65\x64\x74\x48\x22\x8d\x00\x4f\x30\x2e\x30\x30\x42\x00\x00"
        "\x0c\xfe\x00\x02\x6f\x00\x2f\x36\x34\xb1\x00\x05\x01\xdf\x00\x02\x5e\x00\x27\x74\x78\x5e\x00\x0f"
        "\xeb\x00\x0a\x2e\x3c\x2f\xf4\x01\x0f\x65\x01\x35\x1f\x53\x65\x01\x1f\x00\x3f\x02\x41\x61\x6c\x75"
        "\x65\xf1\x00\x0f\x65\x01\x31\x14\x53\x07\x01\x1f\x31\x65\x01\x41\x06\x5e\x00\x1f\x30\x67\x01\x67"
        "\x1f\x56\x67\x01\x1f\x1f\x56\x67\x01\x3d\x1f\x56\x67\x01\x4a\x1f\x56\x67\x01\x71\x1f\x52\x67\x01"
        "\x1f\x1f\x52\x67\x01\x3d\x1f\x52\x67\x01\x4a\x05\x5e\x00\x3f\x32\x35\x35\x67\x01\x66\x1f\x47\x67"
        "\x01\x1f\x15\x47\x67\x01\x0f\x9a\x05\x34\x1f\x47\x9a\x05\x4a\x1f\x47\x9a\x05\x6f\x1f\x42\x65\x01"
        "\x1f\x1f\x42\x65\x01\x3d\x1f\x42\x65\x01\x4a\x04\x5e\x00\x2f\x20\x20\x67\x01\x68\x1f\x41\x67\x01"
        "\x1f\x1f\x41\x33\x04\x3d\x1f\x41\x33\x04\x4a\x1f\x41\x33\x04\x36\x5f\x4e\x75\x6c\x6c\x2f\x7b\x01"
        "\x31\x06\x02\x09\x3f\x74\x6d\x6c\xf1\x00\x00\x03\x01\x0a\xb4\x54\x4d\x4c\x20\x46\x6f\x72\x6d\x61"
        "\x74\x3a\xb9\x00\x2e\x2d\x31\xf5\x00\x0f\xb9\x09\x02\x01\x59\x00\x02\x48\x00\x5f\x23\x46\x46\x30"
        "\x30\x59\x01\x11\x0f\x10\x01\x18\x2a\x3c\x2f\x08\x0b\x2f\x3e\x0a\x32\x00\x05\x0f\x26\x01\x0c\x3f"
        "\x36\x34\x22\x5d\x0b\x07\x02\x26\x00\x20\x37\x30\xf4\x00\x2e\x3e\x0a\x48\x01\x06\x49\x01\x53\x4e"
        "\x6f\x77\x22\x20\xf0\x00\x9f\x43\x75\x72\x72\x65\x6e\x74\x22\x2f\x33\x00\x0b\x35\x4f\x6c\x64\x33"
        "\x00\xaf\x50\x72\x65\x76\x69\x6f\x75\x73\x22\x2f\xe3\x00\x0d\x0f\xa7\x00\x01\x50\x77\x65\x69\x67"
        "\x68\x61\x02\x0c\xa5\x00\x02\xd2\x0c\x02\x39\x03\xa0\x63\x6f\x6c\x4e\x6f\x77\x41\x22\x20\x63\x15"
        "\x00\x40\x3d\x22\x72\x65\x7f\x00\xff\x00\x20\x64\x69\x72\x65\x63\x74\x3d\x22\x74\x72\x75\x65\x22"
        "\x2f\x40\x00\x0b\x36\x4f\x6c\x64\x40\x00\x6e\x62\x6c\x61\x63\x6b\x22\x40\x00\x0f\xbe\x00\x49\x19"
        "\x42\xbe\x00\x0f\xae\x00\x0f\x1b\x42\xae\x00\x0f\xa0\x00\x21\x0b\x65\x00\x63\x42\x75\x74\x74\x6f"
        "\x6e\xb7\x01\x43\xe2\x96\xb6\x22\x71\x00\x5e\x62\x74\x6e\x44\x6f\x2b\x04\x0c\xf3\x02\x1f\x2f\xd3"
        "\x03\x00\x0f\x96\x00\x30\x01\xfb\x00\x03\x9b\x00\x03\x01\x01\x3f\x62\x6e\x41\x8f\x00\x1f\x0f\x48"
        "\x00\x02\x1f\x47\x48\x00\x0f\x0f\xc3\x00\x45\x1f\x42\xc3\x00\x34\x1f\x48\xc3\x00\x67\x1f\x43\xc3"
        "\x00\x34\x1f\x49\xc3\x00\x67\x1f\x44\xc3\x00\x34\x1f\x4a\xc3\x00\x67\x1f\x45\xc3\x00\x34\x1f\x4b"
        "\xc3\x00\x67\x1f\x46\xc3\x00\x34\x1f\x4c\xc3\x00\x10\x0f\x69\x07\x40\x09\x8f\x08\x0d\xf8\x04\x09"
        "\xa5\x00\x5a\x62\x74\x6e\x4f\x4b\x72\x08\x0f\x95\x09\x0f\x02\x92\x05\x3f\x4f\x4b\x22\x57\x00\x09"
        "\x2f\x52\x65\x57\x00\x23\x5f\x52\x65\x73\x65\x74\x5a\x00\x0a\x2f\x43\x6c\x5a\x00\x23\x8f\x43\x61"
        "\x6e\x63\x65\x6c\x22\x2f\x58\x01\x07\xa0\x3c\x2f\x57\x69\x6e\x64\x6f\x77\x3e\x0a";
#ifdef _DEBUG
    // layout
    const char* const COLOR_PCIKER_LAYOUT = 
        u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" titlename="LongUI HSV Color Picker" margin="8,8,8,8" >
    <HorizontalLayout>
        <ColorHSV name="hsvMain" h="sldH" s="sldS" v="sldV"
            r="sldR" g="sldG" b="sldB"/>
        <VerticalLayout borderwidth="1" >
            <HorizontalLayout size="0, 32">
                <Text text="H" size="32, 0"/>
                <Slider name="sldH" end="360" thumbsize="8,16" margin="4,4,4,4" />
                <Edit name="edtH" text="0.00" margin="4,4,4,4" borderwidth="1" size="64, 0"/>
                <Text name="txtH" text="0" size="32, 0"/>
            </HorizontalLayout>
            <HorizontalLayout size="0, 32">
                <Text text="S" size="32, 0"/>
                <Slider name="sldS" value="1" thumbsize="8,16" margin="4,4,4,4" />
                <Edit name="edtS" text="1.00" margin="4,4,4,4" borderwidth="1" size="64, 0"/>
                <Text name="txtS" text="100" size="32, 0"/>
            </HorizontalLayout>
            <HorizontalLayout size="0, 32">
                <Text text="V" size="32, 0"/>
                <Slider name="sldV" value="1" thumbsize="8,16" margin="4,4,4,4" />
                <Edit name="edtV" text="1.00" margin="4,4,4,4" borderwidth="1" size="64, 0"/>
                <Text name="txtV" text="100" size="32, 0"/>
            </HorizontalLayout>
            <HorizontalLayout size="0, 32">
                <Text text="R" size="32, 0"/>
                <Slider name="sldR" value="1" thumbsize="8,16" margin="4,4,4,4" />
                <Edit name="edtR" text="1.00" margin="4,4,4,4" borderwidth="1" size="64, 0"/>
                <Text name="txtR" text="255" size="32, 0"/>
            </HorizontalLayout>
            <HorizontalLayout size="0, 32">
                <Text text="G" size="32, 0"/>
                <Slider name="sldG" value="0" thumbsize="8,16" margin="4,4,4,4" />
                <Edit name="edtG" text="0.00" margin="4,4,4,4" borderwidth="1" size="64, 0"/>
                <Text name="txtG" text="0" size="32, 0"/>
            </HorizontalLayout>
            <HorizontalLayout size="0, 32">
                <Text text="B" size="32, 0"/>
                <Slider name="sldB" value="0" thumbsize="8,16" margin="4,4,4,4" />
                <Edit name="edtB" text="0.00" margin="4,4,4,4" borderwidth="1" size="64, 0"/>
                <Text name="txtB" text=  "0" size="32, 0"/>
            </HorizontalLayout>
            <HorizontalLayout size="0, 32">
                <Text text="A" size="32, 0"/>
                <Slider name="sldA" value="1" thumbsize="8,16" margin="4,4,4,4" />
                <Edit name="edtA" text="1.00" margin="4,4,4,4" borderwidth="1" size="64, 0"/>
                <Text name="txtA" text="255" size="32, 0"/>
            </HorizontalLayout>
            <Null/>
            <HorizontalLayout size="0, 32">
                <Text name="txtHtml" margin="4,4,4,4" text="HTML Format:" size="-1, 0"/>
                <Edit name="edtHtml" text="#FF0000" margin="4,4,4,4" borderwidth="1"/>
            </HorizontalLayout>
        </VerticalLayout>
    </HorizontalLayout>
    <HorizontalLayout size="0, 64">
        <VerticalLayout size="70, 0">
            <Text  name="txtNow"  text="Current"/>
            <Text  name="txtOld"  text="Previous"/>
        </VerticalLayout>
        <VerticalLayout weight="2">
            <Color name="colNowA" color="red"   direct="true"/>
            <Color name="colOldA" color="black" direct="true"/>
        </VerticalLayout>
        <VerticalLayout weight="2">
            <Color name="colNowB" color="red"/>
            <Color name="colOldB" color="black"/>
        </VerticalLayout>
        <VerticalLayout>
            <Button text="▶" name="btnDo" borderwidth="1" margin="4,4,4,4"/>
            <Null/>
        </VerticalLayout>
        <VerticalLayout>
            <ColorButton name="cbnA" borderwidth="1" margin="4,4,4,4"/>
            <ColorButton name="cbnG" borderwidth="1" margin="4,4,4,4"/>
        </VerticalLayout>
        <VerticalLayout>
            <ColorButton name="cbnB" borderwidth="1" margin="4,4,4,4"/>
            <ColorButton name="cbnH" borderwidth="1" margin="4,4,4,4"/>
        </VerticalLayout>
        <VerticalLayout>
            <ColorButton name="cbnC" borderwidth="1" margin="4,4,4,4"/>
            <ColorButton name="cbnI" borderwidth="1" margin="4,4,4,4"/>
        </VerticalLayout>
        <VerticalLayout>
            <ColorButton name="cbnD" borderwidth="1" margin="4,4,4,4"/>
            <ColorButton name="cbnJ" borderwidth="1" margin="4,4,4,4"/>
        </VerticalLayout>
        <VerticalLayout>
            <ColorButton name="cbnE" borderwidth="1" margin="4,4,4,4"/>
            <ColorButton name="cbnK" borderwidth="1" margin="4,4,4,4"/>
        </VerticalLayout>
        <VerticalLayout>
            <ColorButton name="cbnF" borderwidth="1" margin="4,4,4,4"/>
            <ColorButton name="cbnL" borderwidth="1" margin="4,4,4,4"/>
        </VerticalLayout>
    </HorizontalLayout>
    <HorizontalLayout size="0, 32">
        <Null/>
        <Button name="btnOK" size="-1, 0" margin="4,4,4,4" borderwidth="1" text="OK"/>
        <Button name="btnRe" size="-1, 0" margin="4,4,4,4" borderwidth="1" text="Reset"/>
        <Button name="btnCl" size="-1, 0" margin="4,4,4,4" borderwidth="1" text="Cancel"/>
    </HorizontalLayout>
</Window>
)xml";
#endif
}

// longui::impl 命名空间
namespace LongUI { namespace impl { 
    // to longui
    inline auto&longui(const D2D1_COLOR_F& c) noexcept {
        return reinterpret_cast<const ColorPickerF&>(c);
    }
    // to d2d
    inline auto&d2d(const ColorPickerF& c) noexcept {
        return reinterpret_cast<const D2D1_COLOR_F&>(c);
    }
    // clamp 01
    inline void clamp01(float& f) noexcept {
        f = std::max(std::min(f, 1.f), 0.f);
    }
}}


/// <summary>
/// Initializes a new instance of the <see cref="CUIColorPicker"/> class.
/// </summary>
LongUI::CUIColorPicker::CUIColorPicker() noexcept {
    ColorPickerF dc { 1.f, 1.f, 1.f, 1.f };
    for (auto&c : m_aSaveColor) c = dc;
    std::memset(m_apTexted, 0, sizeof(m_apTexted));
    std::memset(m_apColorCtrl, 0, sizeof(m_apColorCtrl));
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIColorPicker"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIColorPicker::~CUIColorPicker() noexcept {
    if (m_pWindow) {
        m_pWindow->Close();
    }
}


/// <summary>
/// Sets the old color.
/// </summary>
/// <param name="c">The c.</param>
/// <returns></returns>
void LongUI::CUIColorPicker::SetOldColor(const D2D1_COLOR_F& c) noexcept {
    m_colorOld = impl::longui(c);
    this->set_old_color(c);
}

/// <summary>
/// Sets the old color.
/// </summary>
/// <param name="c">The c.</param>
/// <returns></returns>
void LongUI::CUIColorPicker::set_old_color(const D2D1_COLOR_F& c) noexcept {
    // 存在窗口
    if (!m_pWindow) return;
    auto& window = *m_pWindow;
    // 设置
    auto set = [&](const char* name) noexcept {
        auto a = window.FindControl(name);
        auto b = longui_cast<UIColor*>(a);
        b->SetColor(c);
    };
    // 指定控件
    set("colOldA"); set("colOldB");
}

/// <summary>
/// Refresh saved color for the specified index.
/// </summary>
/// <param name="index">The index.</param>
/// <returns></returns>
void LongUI::CUIColorPicker::refresh_saved_color(int index) noexcept {
    assert(size_t(index) < lengthof(m_apColorCtrl) && "out of range");
    assert(m_pWindow && "bad window");
    m_apColorCtrl[index]->SetColor(impl::d2d(m_aSaveColor[index]));
}

// longui namespace
namespace LongUI {
    // viewport for color picker
    class UIPickerView final : public UIViewport {
        // super class
        using Super = UIViewport;
        // clean up
        void cleanup() noexcept override { before_deleted(); delete this; }
        // id
        enum ID : uint32_t {
            ID_Null = 0,    // null
            ID_COLORBUTTON, // color buttons
            ID_RESET,       // reset
            ID_OK,          // ok
            ID_CANCEL,      // cancel
            ID_SAVECOLOR,   // save color
        };
    public:
        // ctor
        UIPickerView(XUIBaseWindow* window) : Super(window) {}
        // init with picker
        void TreeFinished(CUIColorPicker* picker) noexcept;
        // do some event
        bool DoEvent(const EventArgument& arg) noexcept override {
            // longui event
            switch (arg.event)
            {
            case LongUI::Event::Event_SubEvent:
                this->subevent(arg.ui.subevent, arg.sender);
                return true;
            default:
                return Super::DoEvent(arg);
            }
        }
        // onclosed
        void OnClose() noexcept override {
            auto color = m_pHsvControl->PickColor();
            color.a = m_pAlphaDisplay->GetFloat();
            m_pColorPicker->m_uiCallback(this->IsOK() ? &color : nullptr);
            m_pColorPicker->m_pWindow = nullptr;
        }
        // set OK
        void SetOK() noexcept { m_state.SetTrue<State_Self7>(); }
        // is OK?
        bool IsOK() const noexcept { return m_state.Test<State_Self7>(); }
        // set color
        inline void SetColor(const D2D1_COLOR_F& c) noexcept {
            m_pHsvControl->SetColor(c);
            m_pAlphaDisplay->SetFloat(c.a);
        }
    private:
        // tree finished
        void subevent(SubEvent e, UIControl* sender) noexcept {
            if (e == LongUI::SubEvent::Event_ItemClicked) {
                this->command(sender);
            }
        }
        // clicked/command
        void command(UIControl* sender) noexcept {
            // on color buttons clicked/command
            auto on_colorbutton = [=]() noexcept {
                auto cbn = longui_cast<UIColorButton*>(sender);
                this->SetColor(cbn->GetColor());
            };
            const auto _this = m_pColorPicker;
            const auto alpha = m_pAlphaDisplay;
            const auto hsvct = m_pHsvControl;
            // on save color
            auto on_savecolor = [=]() noexcept {
                std::memmove(
                    _this->m_aSaveColor + 1, _this->m_aSaveColor + 0,
                    sizeof(_this->m_aSaveColor) - sizeof(_this->m_aSaveColor[0])
                );
                auto color = hsvct->PickColor();
                color.a = alpha->GetFloat();
                _this->m_aSaveColor[0] = impl::longui(color);
                for (int i = 0; i < _this->SAVED_COLOR_COUNT; ++i) {
                    _this->refresh_saved_color(i);
                }
            };
            // witch control?
            switch (ID(sender->user_data))
            {
            case LongUI::UIPickerView::ID_COLORBUTTON:
                on_colorbutton();
                break;
            case LongUI::UIPickerView::ID_RESET:
                this->SetColor(impl::d2d(_this->m_colorOld));
                break;
            case LongUI::UIPickerView::ID_OK:
                this->SetOK();
                __fallthrough;
            case LongUI::UIPickerView::ID_CANCEL:
                m_pWindow->Close();
                break;
            case LongUI::UIPickerView::ID_SAVECOLOR:
                on_savecolor();
                break;
            }
        }
    public:
        // hsv control
        UIColorHsv*         m_pHsvControl = nullptr;
        // alpha data
        UIControl*          m_pAlphaDisplay = nullptr;
        // color picker
        CUIColorPicker*     m_pColorPicker = nullptr;
    };
}

/// <summary>
/// Creates the specified parent.
/// </summary>
/// <param name="parent">The parent.</param>
/// <returns></returns>
bool LongUI::CUIColorPicker::Create(XUIBaseWindow* parent) noexcept {
    assert(parent && "bad argument");
    if (m_pWindow) return true;
    bool a = UIManager.IsRegisteredControlClass("ColorHSV");
    bool b = UIManager.IsRegisteredControlClass("ColorButton");
#ifdef _DEBUG
    /* 注册例子 {
        cc = LongUI::UIColorHsv::CreateControl;
        UIManager.RegisterControlClass(cc, "ColorHSV");
        cc = LongUI::UIColorButton::CreateControl;
        UIManager.RegisterControlClass(cc, "ColorButton");
    }*/
    assert(a && b && "you must register the control!");
#endif
    if(!(a && b)) return false;
    // 原大小
    constexpr int laylen = 5366;
#ifdef _DEBUG
    const int laylen1 = int(std::strlen(COLOR_PCIKER_LAYOUT));
    assert(laylen1 == laylen && "bad length");
    if (false) {
        auto buflen = ::LZ4_compressBound(laylen);
        LongUI::SafeBuffer<char>(size_t(buflen), [=](char* buf) noexcept {
            auto src = COLOR_PCIKER_LAYOUT;
            auto code = ::LZ4_compress_HC(src, buf, laylen, buflen, 16);
            assert(code > 0);
            constexpr int line = 24;
            if (auto file = std::fopen("xml.txt", "w")) {
                for (int i = 0, j = 0; i != code; ++i) {
                    if (j == 0) std::fputc('"', file);
                    char ch = buf[i];
                    std::fprintf(file, "\\x%02x", int(unsigned char(ch)));
                    if (++j == line) {
                        std::fprintf(file, "\"\n");
                        j = 0;
                    }
                }
                std::for_each(buf, buf + code, [=](char ch) noexcept {
                });
                std::fclose(file);
            }
        });
    }
    {
        if (auto out = std::malloc(laylen+1)) {
            auto outchar = reinterpret_cast<char*>(out);
            auto dcode = ::LZ4_decompress_fast(COLOR_PCIKER_LZ4, outchar, laylen);
            outchar[laylen] = 0;
            assert(std::strcmp(COLOR_PCIKER_LAYOUT, outchar) == 0 && "NO!");
            std::free(out);
        }
    }
#endif
    // 申请字符串空间
    if (auto out = LongUI::NormalAllocT<char>(laylen + 1)) {
        auto dcode = ::LZ4_decompress_fast(COLOR_PCIKER_LZ4, out, laylen);
        out[laylen] = 0;
        LongUI::NormalFree(out);
    }
    m_pWindow = parent->CreateChildWindow<UIPickerView>(COLOR_PCIKER_LAYOUT);
    // 创建窗口
    //m_pWindow = UIManager.CreateUIWindow<UIPickerView>(COLOR_PCIKER_LAYOUT);
    // 成功时写入指针
    if (m_pWindow) {
        auto view = m_pWindow->GetViewport();
        static_cast<UIPickerView*>(view)->TreeFinished(this);
    }
    // 正式创建
    return !!m_pWindow;
}


/// <summary>
/// Sets the color of the initialize.
/// </summary>
/// <param name="c">The c.</param>
/// <returns></returns>
void LongUI::CUIColorPicker::SetInitColor(const D2D1_COLOR_F& c) noexcept{
    this->SetOldColor(c);
    if (!m_pWindow) return;
    auto view = static_cast<UIPickerView*>(m_pWindow->GetViewport());
    view->SetColor(c);
}

/// <summary>
/// Sets the name of the title.
/// </summary>
/// <param name="str">The string for title name.</param>
/// <returns></returns>
void LongUI::CUIColorPicker::SetTitleName(const wchar_t* str) noexcept {
    assert(m_pWindow && "check return code for CUIColorPicker::Create");
    assert(str && "bad argument");
    m_pWindow->SetTitleName(str);
}

/// <summary>
/// Shows the window.
/// </summary>
/// <param name="nCmdShow">The n command show.</param>
/// <returns></returns>
void LongUI::CUIColorPicker::ShowWindow(int nCmdShow) noexcept {
    assert(m_pWindow && "check return code for CUIColorPicker::Create");
    m_pWindow->ShowWindow(nCmdShow);
}

/// <summary>
/// Closes the window.
/// </summary>
/// <returns></returns>
void LongUI::CUIColorPicker::CloseWindow() noexcept {
    assert(m_pWindow && "check return code for CUIColorPicker::Create");
    m_pWindow->Close();
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="index">The index.</param>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::CUIColorPicker::SetText(
    ControlIndex index, const wchar_t* text) noexcept {
    assert(m_pWindow && "check return code for CUIColorPicker::Create");
    assert(index < INDEX_COUNT && "out of range");
    m_apTexted[index]->SetText(text);
}

/// <summary>
/// Trees bulit.
/// </summary>
/// <param name="picker">The picker.</param>
/// <returns></returns>
void LongUI::UIPickerView::TreeFinished(CUIColorPicker* picker) noexcept {
    assert(picker);
    auto window = m_pWindow;
    auto hsv = longui_cast<UIColorHsv*>(window->FindControl("hsvMain"));
    m_pHsvControl = hsv;
    m_pColorPicker = picker;
    m_pAlphaDisplay = window->FindControl("sldA");
    assert(hsv && "control not found");
    // controls
    {
        char buf[] = "cbn ";
        auto cbuttons = picker->m_apColorCtrl;
        auto ccolors = picker->m_aSaveColor;
        for (int i = 0; i < picker->SAVED_COLOR_COUNT; ++i) {
            buf[3] = char(i) + 'A';
            auto ctrl = longui_cast<UIColorButton*>(window->FindControl(buf));
            cbuttons[i] = longui_cast<UIColorButton*>(ctrl);
            assert(ctrl && "ctrl not found");
            ctrl->user_data = ID_COLORBUTTON;
            ctrl->SetColor(impl::d2d(ccolors[i]));
        }
    }
    // reset/ok/cancel
    {
        const auto re = window->FindControl("btnRe");
        const auto ok = window->FindControl("btnOK");
        const auto cl = window->FindControl("btnCl");
        const auto sc = window->FindControl("btnDo");
        assert(re && ok && cl && sc && "control not found");
        re->user_data = ID_RESET;
        ok->user_data = ID_OK;
        cl->user_data = ID_CANCEL;
        sc->user_data = ID_SAVECOLOR;
        // texted
        const auto texted = picker->m_apTexted;
        texted[picker->Index_OK] = ok;
        texted[picker->Index_Cancel] = cl;
        texted[picker->Index_Reset] = re;
        texted[picker->Index_Now] = window->FindControl("txtNow");
        texted[picker->Index_Old] = window->FindControl("txtOld");
        texted[picker->Index_Html] = window->FindControl("txtHtml");
    }
    // hsv
    {
        const auto a = window->FindControl("colNowA");
        const auto b = window->FindControl("colNowB");
        const auto c = m_pAlphaDisplay;
        const auto html = window->FindControl("edtHtml");
        assert(a && b && c && "control not found");
        assert(html && "control not found");
        {
            const auto ca = longui_cast<UIColor*>(a);
            const auto cb = longui_cast<UIColor*>(b);
            const auto color_changed = [=](UIControl*) noexcept {
                auto base = hsv->PickColor();
                base.a = c->GetFloat();
                ca->SetColor(base);
                cb->SetColor(base);
                return true;
            };
            hsv->Add_OnChanged(color_changed);
            c->Add_OnChanged(color_changed);
            const auto fa = window->FindControl("edtA");
            const auto ia = window->FindControl("txtA");
            c->Add_OnChanged([fa, ia](UIControl* alpha) noexcept {
                CUIString str;
                float a = alpha->GetFloat();
                str.Format(L"%.3f", a); fa->SetText(str);
                str.Format(L"%d", int(a*255.f)); ia->SetText(str);
                return true;
            });
            hsv->Add_OnChanged([=](UIControl* ) noexcept {
                CUIString str;
                auto base = hsv->PickColor();
                str.Format(
                    L"#%02X%02X%02X",
                    int(base.r*255.f + 0.5f),
                    int(base.g*255.f + 0.5f),
                    int(base.b*255.f + 0.5f)
                );
                // 不同再修改
                html->SetText(str);
                return true;
            });
            // 修改HTML文本
            html->Add_OnChanged([=](UIControl* h) noexcept {
                char buf[32];
                auto str = h->GetText();
                auto len = std::wcslen(str);
                // 缓存不足
                if (len + 1 < lengthof(buf)) {
                    auto itr = buf;
                    for (; *str; ++itr, ++str) *itr = char(*str);
                    *itr = 0;
                    D2D1_COLOR_F color { 1.f, 1.f, 1.f, 1.f };
                    Helper::MakeColor(buf, color);
                    impl::clamp01(color.r);
                    impl::clamp01(color.g);
                    impl::clamp01(color.b);
                    impl::clamp01(color.a);
                    hsv->SetColor(color);
                }
                return true;
            });
        }
    }
    // RGB
    {
        const auto fr = window->FindControl("edtR");
        const auto fg = window->FindControl("edtG");
        const auto fb = window->FindControl("edtB");
        const auto ir = window->FindControl("txtR");
        const auto ig = window->FindControl("txtG");
        const auto ib = window->FindControl("txtB");
        const auto fh = window->FindControl("edtH");
        const auto fs = window->FindControl("edtS");
        const auto fv = window->FindControl("edtV");
        const auto ih = window->FindControl("txtH");
        const auto is = window->FindControl("txtS");
        const auto iv = window->FindControl("txtV");
        assert(fr && fg && fb && "control not found");
        assert(ir && ig && ib && "control not found");
        assert(fh && fs && fv && "control not found");
        assert(ih && is && iv && "control not found");
        // 修改事件
        hsv->Add_OnChanged([=](UIControl*) noexcept {
            CUIString str;
            auto base = hsv->PickColor();
            auto hsv_h = hsv->GetH();
            auto hsv_s = hsv->GetS();
            auto hsv_v = hsv->GetV();
            str.Format(L"%.1f", hsv_h ); fh->SetText(str);
            str.Format(L"%d", int(hsv_h)); ih->SetText(str);

            str.Format(L"%.3f", hsv_s ); fs->SetText(str);
            str.Format(L"%d", int(hsv_s*100.f)); is->SetText(str);

            str.Format(L"%.3f", hsv_v ); fv->SetText(str);
            str.Format(L"%d", int(hsv_v*100.f)); iv->SetText(str);

            str.Format(L"%.3f", base.r); fr->SetText(str);
            str.Format(L"%d", int(base.r*255.f)); ir->SetText(str);

            str.Format(L"%.3f", base.g); fg->SetText(str);
            str.Format(L"%d", int(base.g*255.f)); ig->SetText(str);

            str.Format(L"%.3f", base.b); fb->SetText(str);
            str.Format(L"%d", int(base.b*255.f)); ib->SetText(str);

            return true;
        });
    }
    // H
    {
        const auto slider = window->FindControl("sldH");
        const auto edit = window->FindControl("edtH");
        assert(slider && edit);
        auto on_changed = [=](UIControl* hc) noexcept {
            auto h = std::abs(hc->GetFloat());
            const float end = 359.99f;
            hsv->SetH(h >= 360.f ? end : h);
            return true;
        };
        slider->Add_OnChanged(on_changed);
        edit->Add_OnChanged(on_changed);
    }
    // S
    {
        const auto slider = window->FindControl("sldS");
        const auto edit = window->FindControl("edtS");
        assert(slider && edit);
        auto on_changed = [=](UIControl* sc) noexcept {
            auto s = sc->GetFloat();
            hsv->SetS(std::max(std::min(s, 1.f), 0.f));
            return true;
        };
        slider->Add_OnChanged(on_changed);
        edit->Add_OnChanged(on_changed);
    }
    // V
    {
        const auto slider = window->FindControl("sldV");
        const auto edit = window->FindControl("edtV");
        assert(slider && edit);
        auto on_changed = [=](UIControl* vc) noexcept {
            auto v = vc->GetFloat();
            hsv->SetV(std::max(std::min(v, 1.f), 0.f));
            return true;
        };
        slider->Add_OnChanged(on_changed);
        edit->Add_OnChanged(on_changed);
    }
    // R
    {
        const auto slider = window->FindControl("sldR");
        const auto edit = window->FindControl("edtR");
        assert(slider && edit);
        auto on_changed = [=](UIControl* rc) noexcept {
            auto r = rc->GetFloat();
            auto color = hsv->PickColor();
            color.r = std::max(std::min(r, 1.f), 0.f);
            hsv->SetColor(color);
            return true;
        };
        slider->Add_OnChanged(on_changed);
        edit->Add_OnChanged(on_changed);
    }
    // G
    {
        const auto slider = window->FindControl("sldG");
        const auto edit = window->FindControl("edtG");
        assert(slider && edit);
        auto on_changed = [=](UIControl* gc) noexcept {
            auto g = gc->GetFloat();
            auto color = hsv->PickColor();
            color.g = std::max(std::min(g, 1.f), 0.f);
            hsv->SetColor(color);
            return true;
        };
        slider->Add_OnChanged(on_changed);
        edit->Add_OnChanged(on_changed);
    }
    // B
    {
        const auto slider = window->FindControl("sldB");
        const auto edit = window->FindControl("edtB");
        assert(slider && edit);
        auto on_changed = [=](UIControl* bc) noexcept {
            auto b = bc->GetFloat();
            auto color = hsv->PickColor();
            color.b = std::max(std::min(b, 1.f), 0.f);
            hsv->SetColor(color);
            return true;
        };
        slider->Add_OnChanged(on_changed);
        edit->Add_OnChanged(on_changed);
    }
}