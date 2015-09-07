#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
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

// LongUI COM Object Method
#define LONGUICOMMETHOD virtual HRESULT STDMETHODCALLTYPE

// longui::helper namespace
namespace LongUI { namespace Helper {
    // ------------------- Windows COM Interface Helper -----------------------------
    // counter based COM Interface, 0 , wiil be deleted
    template <typename InterfaceChain, typename CounterType = std::atomic<size_t>>
    class ComBase : public InterfaceChain {
    public:
        // constructor inline ver.
        explicit ComBase(ULONG init_count=1) noexcept : m_refValue(init_count) { }
        // IUnknown interface
        LONGUICOMMETHOD QueryInterface(IID const& iid, OUT void** ppObject) noexcept final override {
            *ppObject = nullptr;
            InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            if (*ppObject == nullptr)
                return E_NOINTERFACE;
            this->ComBase::AddRef();
            return S_OK;
        }
        // add ref-counter
        ULONG STDMETHODCALLTYPE AddRef() noexcept final override { return static_cast<ULONG>(++m_refValue); }
        // delete when 0
        ULONG STDMETHODCALLTYPE Release() noexcept final override {
            assert(m_refValue != 0 && "bad idea to release zero ref-count object");
            ULONG newCount = static_cast<ULONG>(--m_refValue);
            if (newCount == 0)  delete this;
            return newCount;
        }
        // virtual destructor
        virtual ~ComBase() noexcept { }
    protected:
        // the counter 
        CounterType             m_refValue;
    public:
        // No copy construction allowed.
        ComBase(const ComBase& b) = delete;
        ComBase& operator=(ComBase const&) = delete;
    };
    // None Counter COM(Static)
    template <typename InterfaceChain>
    class ComStatic : public InterfaceChain {
    public:
        // constructor inline ver.
        explicit ComStatic() noexcept { }
        // IUnknown interface
        LONGUICOMMETHOD QueryInterface(IID const& iid, OUT void** ppObject) noexcept final override {
            *ppObject = nullptr;
            InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            if (*ppObject == nullptr)
                return E_NOINTERFACE;
            AddRef();
            return S_OK;
        }
        // allways return 2
        virtual ULONG STDMETHODCALLTYPE AddRef() noexcept final override { return  2; }
        // allways return 1
        virtual ULONG STDMETHODCALLTYPE Release() noexcept final override { return 1; }
    public:
        // No copy construction allowed.
        ComStatic(const ComStatic& b) = delete;
        ComStatic& operator=(ComStatic const&) = delete;
    };
    // None
    struct QiListNil { };
    // When the QueryInterface list refers to itself as class,
    // which hasn't fully been defined yet.
    template <typename InterfaceName, typename InterfaceChain>
    class QiListSelf : public InterfaceChain {
    public:
        inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) noexcept {
            if (iid != LongUI::GetIID<InterfaceName>())
                return InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            *ppObject = static_cast<InterfaceName*>(this);
        }
    };
    // When this interface is implemented and more follow.
    template <typename InterfaceName, typename InterfaceChain = QiListNil>
    class QiList : public InterfaceName, public InterfaceChain {
    public:
        inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) noexcept {
            if (iid != LongUI::GetIID<InterfaceName>())
                return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

            *ppObject = static_cast<InterfaceName*>(this);
        }
    };
    // When the this is the last implemented interface in the list.
    template <typename InterfaceName>
    class QiList<InterfaceName, QiListNil> : public InterfaceName{
    public:
        inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) noexcept {
            if (iid != LongUI::GetIID<InterfaceName>()) return;
            *ppObject = static_cast<InterfaceName*>(this);
        }
    };
    // ------------------- Save Memory Helper -----------------------------
    // Bit Array 计算机中每一字节都很宝贵
    template<typename T>
    class BitArray {
    public:
        // bit wide
        static constexpr size_t LENGTH = sizeof(T) * 8;
        // ctor
        BitArray() noexcept {};
        // dtor
        ~BitArray() noexcept {};
        // is true or fasle
        auto Test(uint32_t index) const noexcept { assert(index<LENGTH); return !!(m_data & (1 << index)); }
        // set to true
        auto SetTrue(uint32_t index) noexcept { assert(index<LENGTH); m_data |= (1 << index); };
        // set to false
        auto SetFalse(uint32_t index) noexcept { assert(index<LENGTH); m_data &= ~(1 << index); };
        // set to NOT
        auto SetNot(uint32_t index) noexcept { assert(index<LENGTH); m_data ^= (1 << index); };
        // set to???
        template<typename V>
        auto SetTo(uint32_t index, V& value) noexcept { assert(index<LENGTH); value ? this->SetTrue(index) : this->SetFalse(index); }
    private:
        // data for bit-array
        T           m_data = T(0);
    };
    // 特例化
    using BitArray16 = BitArray<uint16_t>;
    using BitArray32 = BitArray<uint32_t>;
    using BitArray64 = BitArray<uint64_t>;
    // data 放肆!450交了么!
    constexpr size_t  INFOPDATA12_ZONE = (size_t(3));
    constexpr size_t  INFOPOINTER_ZONE = ~INFOPDATA12_ZONE;
    constexpr size_t  INFOPTDATA1_ZONE = ~(size_t(2));
    constexpr size_t  INFOPTDATA2_ZONE = ~(size_t(1));
    // Infomation-ed pointer  计算机中每一字节都很宝贵
    template<typename T>
    class InfomationPointer {
    public:
        // constructor
        InfomationPointer(T* pointer) :data(reinterpret_cast<size_t>(pointer)) { assert(!(data&INFOPDATA12_ZONE)); }
        // copy constructor
        InfomationPointer(const InfomationPointer&) = delete;
        // move constructor
        InfomationPointer(InfomationPointer&&) = delete;
        // operator =
        T* operator=(T* pt) { assert(!(data&INFOPDATA12_ZONE)); data = reinterpret_cast<size_t>(pt) | (data&INFOPDATA12_ZONE); return pt; }
        // operator ->
        T* operator->() noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // operator T*
        operator T*() const noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // operator []
        T& operator [](const int index) noexcept { return (reinterpret_cast<T*>(data & INFOPOINTER_ZONE))[index]; }
        // operator [] const ver.
        const T& operator [](const int index) const noexcept { return (reinterpret_cast<T*>(data & INFOPOINTER_ZONE))[index]; }
        // pointer
        T* Ptr() const noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // bool1
        bool Bool1() const noexcept { return (data & (1 << 0)) > 0; }
        // bool2
        bool Bool2() const noexcept { return (data & (1 << 1)) > 0; }
        // set bool1
        void SetBool1(bool b) noexcept { data = (data & INFOPTDATA2_ZONE) | size_t(b); }
        // bool2
        void SetBool2(bool b) noexcept { data = (data & INFOPTDATA1_ZONE) | (size_t(b) << 1); }
        // SafeRelease if keep a Relase() interface(COM like)
        void SafeRelease() noexcept { T* t = reinterpret_cast<T*>(data & INFOPOINTER_ZONE); if (t) { t->Release(); data &= INFOPDATA12_ZONE; } }
    private:
        // pointer & boolx2 data
        size_t          data;
    };
    // ----------------------  xml helper -----------------------------
    // XMLGetValueEnum Properties
    struct XMLGetValueEnumProperties {
        // attribute
        const char*         attribute;
        // values list of enum
        const char* const*  values;
        // length of 'values'
        size_t              values_length;
        // prefix
        const char*         prefix;
    };
    // make color form string
    bool MakeColor(const char*, D2D1_COLOR_F&) noexcept;
    // make UIString form string
    bool MakeString(const char*, CUIString&) noexcept;
    // make floats from string
    bool MakeFloats(const char*, float*, int) noexcept;
    // make floats from string
    bool SetBorderColor(pugi::xml_node, D2D1_COLOR_F[STATUS_COUNT]) noexcept;
    // get value string
    auto XMLGetValue(pugi::xml_node node, const char* attribute, const char* prefix =nullptr) noexcept -> const char*;
    // get value enum-int
    auto XMLGetValueEnum(pugi::xml_node node, const XMLGetValueEnumProperties& prop, uint32_t bad_match = 0) noexcept->uint32_t;
    // get animation type
    auto XMLGetAnimationType(
        pugi::xml_node node,
        AnimationType bad_match,
        const char* attribute = "animationtype",
        const char* prefix = nullptr
        ) noexcept->AnimationType;
    // get d2d interpolation mode
    auto XMLGetD2DInterpolationMode(
        pugi::xml_node node, 
        D2D1_INTERPOLATION_MODE bad_match,
        const char* attribute = "interpolation",
        const char* prefix = nullptr
        ) noexcept ->D2D1_INTERPOLATION_MODE;
    // get d2d extend mode
    auto XMLGetD2DExtendMode(
        pugi::xml_node node,
        D2D1_EXTEND_MODE bad_match,
        const char* attribute = "extend",
        const char* prefix = nullptr
        ) noexcept->D2D1_EXTEND_MODE;
    // get longui bitmap render rule
    auto XMLGetBitmapRenderRule(
        pugi::xml_node node,
        BitmapRenderRule bad_match,
        const char* attribute = "rule",
        const char* prefix = nullptr
        ) noexcept->BitmapRenderRule;
    // get longui richtype
    auto XMLGetRichType(
        pugi::xml_node node,
        RichType bad_match,
        const char* attribute = "richtype",
        const char* prefix = nullptr
        ) noexcept->RichType;
    // get DWRITE_FONT_STYLE
    auto XMLGetFontStyle(
        pugi::xml_node node,
        DWRITE_FONT_STYLE bad_match,
        const char* attribute = "style",
        const char* prefix = nullptr
        ) noexcept->DWRITE_FONT_STYLE;
    // get DWRITE_FONT_STRETCH
    auto XMLGetFontStretch(
        pugi::xml_node node,
        DWRITE_FONT_STRETCH bad_match,
        const char* attribute = "stretch",
        const char* prefix = nullptr
        ) noexcept->DWRITE_FONT_STRETCH;
    // get DWRITE_FLOW_DIRECTION
    auto XMLGetFlowDirection(
        pugi::xml_node node,
        DWRITE_FLOW_DIRECTION bad_match,
        const char* attribute = "flowdirection",
        const char* prefix = nullptr
        ) noexcept->DWRITE_FLOW_DIRECTION;
    // get DWRITE_READING_DIRECTION
    auto XMLGetReadingDirection(
        pugi::xml_node node,
        DWRITE_READING_DIRECTION bad_match,
        const char* attribute = "readingdirection",
        const char* prefix = nullptr
        ) noexcept->DWRITE_READING_DIRECTION;
    // get DWRITE_READING_DIRECTION
    auto XMLGetWordWrapping(
        pugi::xml_node node,
        DWRITE_WORD_WRAPPING bad_match,
        const char* attribute = "wordwrapping",
        const char* prefix = nullptr
        ) noexcept->DWRITE_WORD_WRAPPING;
    // get DWRITE_PARAGRAPH_ALIGNMENT
    auto XMLGetVAlignment(
        pugi::xml_node node,
        DWRITE_PARAGRAPH_ALIGNMENT bad_match,
        const char* attribute = "valign",
        const char* prefix = nullptr
        ) noexcept->DWRITE_PARAGRAPH_ALIGNMENT;
    // get DWRITE_PARAGRAPH_ALIGNMENT
    auto XMLGetHAlignment(
        pugi::xml_node node,
        DWRITE_TEXT_ALIGNMENT bad_match,
        const char* attribute = "align",
        const char* prefix = nullptr
        ) noexcept->DWRITE_TEXT_ALIGNMENT;
    // get d2d text anti-mode
    auto XMLGetD2DTextAntialiasMode(pugi::xml_node node, D2D1_TEXT_ANTIALIAS_MODE bad_match) noexcept->D2D1_TEXT_ANTIALIAS_MODE;
}}

