#pragma once

namespace LongUI {
    // string view
    template<typename T> struct PodStringView;
    // UTF-8 String View
    using U8View = PodStringView<char>;
    // wchar string view
    using WcView = PodStringView<wchar_t>;
    // basic string
    template<typename T, unsigned B> class CUIBasicString;
    // utf-8 string
    using CUIStringU8 = CUIBasicString<char, 8>;
    // wchar string
    using CUIStringWC = CUIBasicString<wchar_t, 8 / sizeof(wchar_t)>;
    // utf-16 string
    using CUIString16 = CUIBasicString<char16_t, 4>;
    // utf-32 string
    using CUIString32 = CUIBasicString<char32_t, 2>;
    // large fiexed buffer
    using CUIStringEx = CUIBasicString<wchar_t, 64>;
    // default string
    using CUIString = CUIStringWC;
    // string vectors
    //struct CUIStringList;
    // EventAccept
    enum EventAccept : bool { Event_Accept = true, Event_Ignore = false };
    // graphics adapter desc
    struct GraphicsAdapterDesc;
    // stylesheet value
    struct SSValue;
    // viewport
    class UIViewport;
    // control class
    class UIControl;
    // window
    class CUIWindow;
    // manager
    class CUIManager;
    // signature
    template<typename signature> class CUIFunction;
    // Gui Event Listener
    using GuiEventListener = CUIFunction<EventAccept(UIControl& host)>;
    // is remove listener?
    inline bool IsRemoveListener(const GuiEventListener& l) { return !(&l); }
}