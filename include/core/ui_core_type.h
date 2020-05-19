#pragma once

namespace LongUI {
    // LUID - Unique LongUI ID
    struct ULID { const char* id; };
    // string view
    template<typename T> struct PodStringView;
    // UTF-8 String View
    using U8View = PodStringView<char>;
    // utf-16 string view
    using U16View = PodStringView<char16_t>;
    // basic string
    template<typename T, unsigned B> class CUIBasicString;
    // utf-8 string
    using CUIStringU8 = CUIBasicString<char, 8>;
    // utf-16 string
    using CUIString16 = CUIBasicString<char16_t, 4>;
    // utf-32 string
    using CUIString32 = CUIBasicString<char32_t, 2>;
    // utf-16 string ex
    using CUIStringEx = CUIBasicString<char16_t, 64>;
    // default string: utf-16
    using CUIString = CUIString16;
    // pod vector
    namespace POD { template<typename T> class Vector; }
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
    // EventAccept ||
    inline EventAccept operator||(EventAccept a, EventAccept b) noexcept {
        union { EventAccept x; bool y; } v, w; v.x = a; w.x = b;
        return static_cast<EventAccept>(v.y || w.y);
    }
    // EventAccept &&
    inline EventAccept operator&&(EventAccept a, EventAccept b) noexcept {
        union { EventAccept x; bool y; } v, w; v.x = a; w.x = b;
        return static_cast<EventAccept>(v.y && w.y);
    }
}