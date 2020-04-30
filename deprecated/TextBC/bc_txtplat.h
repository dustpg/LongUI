#pragma once


// config marco for this

// enable UNDO-REDO feature
#define TBC_UNDOREDO




#ifndef PCN_NOVTABLE
#ifdef _MSC_VER
#define PCN_NOVTABLE _declspec(novtable)
#else
#define PCN_NOVTABLE
#endif
#endif

#ifndef PCN_NOINLINE
#ifdef _MSC_VER
#define PCN_NOINLINE _declspec(noinline)
#else
#define PCN_NOINLINE
#endif
#endif




#include <cstdint>

// richtb namespace
namespace TextBC {
    struct Node { Node* prev, *next; };
    struct SizeF { float width, height; };
    struct Point2F { float x, y; };
    struct HitTest { uint32_t pos; uint32_t u16_trailing; };
    struct RectWHF { float x, y, width, height; };
    struct U16View { const char16_t* first, *second; };
    struct CharMetrics { float x, width; };
    struct Range { uint32_t pos; uint32_t len; };
    struct CBCSmallObject {};
    inline bool operator==(const Point2F& a, const Point2F& b) noexcept {
        return a.x == b.x && a.y == b.y; }
}

// richtb namespace
namespace TextBC {
    // text content
    struct IBCTextContent;
    // text platform
    struct PCN_NOVTABLE IBCTextPlatform {
        // metrics event
        enum MetricsEvent : uint32_t {
            // get size.        arg:(union) [out]<SizeF*>
            Event_GetSize = 0,
            // get baseline.    arg:(union) [out]<float*>
            Event_GetBaseline,
            // hit test         arg:(union) [out]<HitTest*>        [in]<float*>        {relative position in this cell}
            Event_HitTest,
            // char metrics     arg:(union) [out]<CharMetrics*>    [in]<uint32_t*>     {relative position in this cell}
            Event_CharMetrics,
        };
        // error beep
        virtual void ErrorBeep() noexcept = 0;
        // is valid password [char16_t only]
        virtual bool IsValidPassword(char16_t) noexcept = 0;
        // generate text
        virtual void GenerateText(void* string, U16View view) noexcept = 0;
        // need redraw
        virtual void NeedRedraw() noexcept = 0;
        // draw caret, TODO: set caret rect
        virtual void DrawCaret(void* ctx, Point2F offset, const RectWHF& rect) noexcept = 0;
        // draw selection
        virtual void DrawSelection(void* ctx, Point2F offset, const RectWHF[], uint32_t len) noexcept = 0;
        // delete content
        virtual void DeleteContent(IBCTextContent&) noexcept = 0;
        // draw content
        virtual void DrawContent(IBCTextContent&, void* ctx, Point2F pos) noexcept = 0;
        // content metrics event
        virtual void ContentEvent(IBCTextContent&, MetricsEvent, void*) noexcept = 0;
        // create content
        virtual auto CreateContent(
            const char16_t*, 
            uint32_t len, 
            IBCTextContent&& old /* maybe null */
        ) noexcept->IBCTextContent* = 0;
#ifndef NDEBUG
        // debug output
        virtual void DebugOutput(const char*) noexcept = 0;
        // debug draw cell
        virtual void DrawCell(void* ctx, const RectWHF& rect, int index) noexcept = 0;
#endif
    };
}
