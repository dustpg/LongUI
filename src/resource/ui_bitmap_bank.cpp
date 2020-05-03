#define NOMINMAX
// ui
#include <core/ui_manager.h>
#include <container/pod_vector.h>
#include <resource/ui_bitmap_bank.h>
#include <graphics/ui_graphics_impl.h>
#include <graphics/ui_graphics_impl.h>
// c++
#include <cassert>
#include <algorithm>



/// <summary>
/// private impl for <see cref="CUIBitmapBank"/>
/// </summary>
struct LongUI::CUIBitmapBank::Private {
    // alloc new window
    static auto AllocNewWindow(CUIBitmapBank& bank) noexcept->BitbankWindow*;
    // alloc rect
    static void AllocRect(BitbankWindow&, BitmapFrame&, Size2U) noexcept;
    // could alloc?
    static bool CanAllocRect(const BitbankWindow*, Size2U) noexcept;
    // alloc rect template
    template<typename T>
    static bool RectAlloc(const BitbankWindow&, Size2U, T) noexcept;
};

/// <summary>
/// Initializes a new instance of the <see cref="CUIBitmapBank"/> class.
/// </summary>
LongUI::CUIBitmapBank::CUIBitmapBank() noexcept {
}


/// <summary>
/// Finalizes an instance of the <see cref="CUIBitmapBank"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIBitmapBank::~CUIBitmapBank() noexcept {
    assert(m_count == 0);
}

/// <summary>
/// Releases all.
/// </summary>
/// <returns></returns>
void LongUI::CUIBitmapBank::ReleaseAll() noexcept {
    m_count = 0;
    for (auto& wnd : m_window)
        LongUI::SafeRelease(wnd.bitmap);
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIBitmapBank::Recreate() noexcept -> Result {
    for (auto& wnd : m_window) {
        if (wnd.bitmap) {
            wnd.bitmap->Release();
            wnd.bitmap = nullptr;
            const Size2U bank{ BITBANK_BITMAP_SIZE , BITBANK_BITMAP_SIZE };
            const auto hr = UIManager.CreateBitmap(bank, luiref wnd.bitmap);
            if (!hr) return hr;
        }
    }
    return { Result::RS_OK };
}

/// <summary>
/// Allocs the specified .
/// </summary>
/// <param name="size">The size.</param>
/// <param name="frame">The frame.</param>
/// <returns></returns>
auto LongUI::CUIBitmapBank::Alloc(Size2U size, BitmapFrame& frame) noexcept ->Result {
    assert(size.width < BITBANK_BITMAP_RMAX);
    assert(size.height < BITBANK_BITMAP_RMAX);
    BitbankWindow* window = nullptr;
    // 窗口列表为空或者不够
    if (!(m_count && Private::CanAllocRect(window = &m_window.back(), size))) {
        // 需要创建一个新窗口
        window = Private::AllocNewWindow(*this);
        if (!window) return { Result::RE_OUTOFMEMORY };
#ifndef NDEBUG
        if (m_count > 1) { LUIDebug(Warning);}
        else { LUIDebug(Hint); }
        _lui_inter_debug << "New Bitbank Window Alloced: " << m_count << endl;
#endif // !NDEBUG

        const Size2U bank{ BITBANK_BITMAP_SIZE , BITBANK_BITMAP_SIZE };
        const auto hr = UIManager.CreateBitmap(bank, luiref window->bitmap);
        // 失败回退操作
        if (!hr) {
            --m_count;
#ifndef NDEBUG
            LUIDebug(Error)
                << "rect(" << size
                << ") alloced failed  -  window: " << m_count
                << endl;
#endif // !NDEBUG
            return hr;
        }
    }
    assert(window && window->bitmap);
    Private::AllocRect(*window, frame, size);
#ifndef NDEBUG
    const float aall = (float)BITBANK_BITMAP_SIZE * BITBANK_BITMAP_SIZE / 100.f;
    LUIDebug(Hint) 
        << "rect alloced @ " << frame.rect
        << DDFFloat2{ (float)window->area / aall }
        //<< window->area
        << "%%  -  window: " << m_count
        << endl;
#endif // !NDEBUG
    return { Result::RS_OK };
}

/// <summary>
/// Frees the specified .
/// </summary>
/// <param name="frame">The frame.</param>
/// <returns></returns>
void LongUI::CUIBitmapBank::Free(BitmapFrame& frame) noexcept {
    assert(frame.window && "bad args");
    if (!frame.window) return;
    const auto window = frame.window;
#ifndef NDEBUG
    const float aall = (float)BITBANK_BITMAP_SIZE * BITBANK_BITMAP_SIZE / 100.f;
    const auto cmp = [&](const BitbankWindow& w) noexcept { return &w == frame.window; };
    const auto itr = std::find_if(m_window.begin(), m_window.end(), cmp);
    assert(itr != m_window.end() && "window not find");
    LUIDebug(Hint)
        << "rect free on @" << frame.rect
        << DDFFloat2{ (float)window->area / aall }
        //<< window->area
        << "%%"
        << endl;
#endif
    // 链接前后节点
    if (frame.prev) frame.prev->next = frame.next;
    if (frame.next) frame.next->prev = frame.prev;
    // 减少使用面积
    const uint32_t area = frame.rect.width * frame.rect.height;
    assert(window->area >= area);
    window->area -= area;

    // 完全释放
    if (!window->area) {
        window->bitmap->Release();
#ifndef NDEBUG
        std::memset(window, -1, sizeof(*window));
#endif
        window->bitmap = nullptr;
        assert(m_count);
        m_count--;
        return;
    }
    // 如果是终节点
    if (window->last == &frame) {
        window->last = frame.prev;
        return;
    }
    // TODO: 面积少于阈值则进行位图移动 
    if (window->area < BITBANK_BITMAP_MOVE_THRESHOLD) {
    }
}

// ----------------------------------------------------------------------------
//                           LongUI::CUIBitmapBank::Private
// ----------------------------------------------------------------------------


/// <summary>
/// Allocs the new window.
/// </summary>
/// <param name="bank">The bank.</param>
/// <returns></returns>
auto LongUI::CUIBitmapBank::Private::AllocNewWindow(
    CUIBitmapBank& bank) noexcept -> BitbankWindow * {
    constexpr uint32_t recycle1 = 2;
    constexpr uint32_t recycle2 = 4;
    BitbankWindow* win = nullptr;
    const auto win_size = bank.m_window.size();
    // 复用已有的: 如果超过 指定数量 就直接末尾添加
    if (bank.m_count >= win_size * recycle1 / recycle2) {
        bank.m_window.resize(win_size + 1);
        // 内存不足
        if (bank.m_window.is_ok()) {
            bank.m_count++;
            win = &bank.m_window[win_size];
        }
    }
    else {
        const auto bitr = bank.m_window.begin();
        const auto eitr = bank.m_window.end();
        for (auto&x : bank.m_window) {
            if (!x.bitmap) {
                std::memset(&x, 0, sizeof(x));
                win = &x;
                bank.m_count++;
                break;
            }
        }
        assert(win && "BUG");
    }
    return win;
}


/// <summary>
/// Rects the alloc.
/// </summary>
/// <param name="win">The win.</param>
/// <param name="size">The size.</param>
/// <param name="call">The call.</param>
/// <returns></returns>
template<typename T>
bool LongUI::CUIBitmapBank::Private::RectAlloc(const BitbankWindow& win, Size2U size, T call) noexcept {
    // TODO: 优化算法
    RectWHU out = { 0, 0, size.width, size.height };
    uint32_t new_top = win.top;
    uint32_t new_bottom = size.height;
    bool ret;
    // 第一个
    if (!win.last) {
        assert(size.width < BITBANK_BITMAP_RMAX);
        assert(size.height < BITBANK_BITMAP_RMAX);
        ret = true;
    }
    else {
        uint32_t height_remain = 0;
        const auto& last_rect = win.last->rect;
        const uint32_t last_right = last_rect.left + last_rect.width;
        // 同行上一个下面还有空间?
        /*
           ++++----             ++++----
           ++++----             ++++----
           ++++                 ++++
           ++++                 ++++

           -> XX                -> XX

           ++++----XX           ++++----
           ++++----             ++++----
           ++++                 ++++  XX
           ++++                 ++++
        */
        if (last_rect.width >= size.width && 
            last_rect.top + last_rect.height + size.height <= win.bottom) {
            out.left = last_rect.left + last_rect.width;
            out.left -= size.width;
            out.top = last_rect.top + last_rect.height;
            new_bottom = win.bottom;
            ret = true;
        }
        else {
            // 同一行还有横向空间
            if (last_right + size.width <= BITBANK_BITMAP_SIZE) {
                out.left = last_right; out.top = win.top;
                height_remain = BITBANK_BITMAP_SIZE - win.top;
                const uint32_t this_b = size.height + win.top;
                new_bottom = std::max(this_b, win.bottom);
            }
            // 下一行
            else { 
                new_top = out.top = win.bottom;
                new_bottom += win.bottom; 
                height_remain = BITBANK_BITMAP_SIZE - win.bottom;
            }
            ret = height_remain >= size.height;
        }
    }
    call(out, new_top, new_bottom);
    return ret;
}

/// <summary>
/// Allocs the rect.
/// </summary>
/// <param name="win">The win.</param>
/// <param name="frame">The frame.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUIBitmapBank::Private::AllocRect(BitbankWindow& win,
    BitmapFrame& frame, Size2U size) noexcept {
    assert(CanAllocRect(&win, size));
    Private::RectAlloc(win, size, [&](const RectWHU& rect, uint32_t nt, uint32_t nb) noexcept {
        assert(win.bitmap);
        if (const auto last = win.last) last->next = &frame;
        frame.prev = win.last;
        frame.next = nullptr;

        (frame.bitmap = win.bitmap)->AddRef();
        frame.window = &win;
        frame.source.left = static_cast<float>(rect.left);
        frame.source.top = static_cast<float>(rect.top);
        frame.source.right = frame.source.left + static_cast<float>(rect.width);
        frame.source.bottom = frame.source.top + static_cast<float>(rect.height);
        frame.rect = rect;

        win.last = &frame;
        win.top = nt;
        win.bottom = nb;
        //win.area += rect.width * rect.width;
        win.area += rect.width * rect.height;
    });
}


PCN_NOINLINE
/// <summary>
/// Determines whether this instance [can alloc rect] the specified win.
/// </summary>
/// <param name="win">The win.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
bool LongUI::CUIBitmapBank::Private::CanAllocRect(const BitbankWindow* win, Size2U size) noexcept {
    assert(win);
    const auto z3 = [](const RectWHU&, uint32_t, uint32_t) noexcept {};
    return Private::RectAlloc(*win, size, z3);
}