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
    // double click helper
    /*
        Helper::DoubleClick leftdb(500);
        auto a = leftdb.Click(); // false
        ::Sleep(1000);
        auto b = leftdb.Click(); // false
        ::Sleep(50);
        auto c = leftdb.Click(); // true
    */
    struct DoubleClick {
        // ctor
        DoubleClick(uint32_t t = ::GetDoubleClickTime()) noexcept : time(t) {};
        // click, return true if double clicked
        bool Click(const D2D1_POINT_2F& pt) noexcept;
        // time
        uint32_t        time;
        // last click time
        uint32_t        last = 0;
        // mouse point x
        float           ptx = -1.f;
        // mouse point y
        float           pty = -1.f;
    };
    // CC for CreateControl
    struct CC { CreateControlFunction func; size_t id; };
    // make cc, if data -> null, just return count
    auto MakeCC(const char* str, CC* OPTIONAL data = nullptr) noexcept ->uint32_t;
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
        static constexpr size_t LENGTH = sizeof(T) * CHAR_BIT;
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
    using BitArray_8 = BitArray<uint8_t>;
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
    // ----------------------  api helper -----------------------------
    // GlobalAlloc a string data: char overload
    auto GlobalAllocString(const char* str, size_t len) noexcept ->HGLOBAL;
    // GlobalAlloc a string data: wchar_t overload
    auto GlobalAllocString(const wchar_t* str, size_t len) noexcept ->HGLOBAL;
    // GlobalAlloc a string data: inline const wchar_t* overload
    inline auto GlobalAllocString(const wchar_t* str) noexcept {
        return GlobalAllocString(str, static_cast<size_t>(std::wcslen(str)));
    }
    // GlobalAlloc a string data: inline const char* overload
    inline auto GlobalAllocString(const char* str) noexcept {
        return GlobalAllocString(str, static_cast<size_t>(std::strlen(str)));
    }
    // GlobalAlloc a string data: inline LongUI::CUIString overload
    inline auto GlobalAllocString(const CUIString& str) noexcept {
        return GlobalAllocString(str.c_str(), static_cast<size_t>(str.length()));
    }
    // find files to buffer
    auto FindFilesToBuffer(wchar_t* buf, size_t buf_len, const wchar_t* folder, const wchar_t* name = L"*.*") noexcept -> wchar_t*;
    // ----------------------  xml helper -----------------------------
    // XMLGetValueEnum Properties
    struct GetEnumProperties {
        // values list of enum
        const char* const*  values_list;
        // length of 'values'
        uint32_t            values_length;
        // length of 'values'
        uint32_t            bad_match;
    };
    // make color form string
    bool MakeColor(const char* str, D2D1_COLOR_F& colot) noexcept;
    // make UIString form string
    bool MakeString(const char* str, CUIString& uistr) noexcept;
    // make floats from string
    bool MakeFloats(const char* str, float fary[], uint32_t count) noexcept;
    // make ints from string
    bool MakeInts(const char* str, int fary[], uint32_t count) noexcept;
    // make meta group
    bool MakeMetaGroup(pugi::xml_node node, const char* prefix, uint16_t fary[], uint32_t count) noexcept;
    // make floats from string
    bool SetBorderColor(pugi::xml_node, D2D1_COLOR_F[STATE_COUNT]) noexcept;
    // get value string
    auto XMLGetValue(pugi::xml_node node, const char* attribute, const char* prefix =nullptr) noexcept -> const char*;
    // get value enum-int
    auto GetEnumFromString(const char* value, const GetEnumProperties& prop) noexcept ->uint32_t;
    // get longui richtype
    auto GetEnumFromString(const char* value, RichType bad_match) noexcept ->RichType;
    // get animation type
    auto GetEnumFromString(const char* value, AnimationType bad_match) noexcept ->AnimationType;
    // get longui bitmap render rule
    auto GetEnumFromString(const char* value, BitmapRenderRule bad_match) noexcept ->BitmapRenderRule;
    // get DWRITE_FONT_STYLE
    auto GetEnumFromString(const char* value, DWRITE_FONT_STYLE bad_match) noexcept ->DWRITE_FONT_STYLE;
    // get DWRITE_FONT_STRETCH
    auto GetEnumFromString(const char* value, DWRITE_FONT_STRETCH bad_match) noexcept ->DWRITE_FONT_STRETCH;
    // get DWRITE_FLOW_DIRECTION
    auto GetEnumFromString(const char* value, DWRITE_FLOW_DIRECTION bad_match) noexcept ->DWRITE_FLOW_DIRECTION;
    // get DWRITE_READING_DIRECTION
    auto GetEnumFromString(const char* value, DWRITE_READING_DIRECTION bad_match) noexcept ->DWRITE_READING_DIRECTION;
    // get DWRITE_WORD_WRAPPING
    auto GetEnumFromString(const char* value, DWRITE_WORD_WRAPPING bad_match) noexcept ->DWRITE_WORD_WRAPPING;
    // get DWRITE_PARAGRAPH_ALIGNMENT
    auto GetEnumFromString(const char* value, DWRITE_PARAGRAPH_ALIGNMENT bad_match) noexcept ->DWRITE_PARAGRAPH_ALIGNMENT;
    // get DWRITE_TEXT_ALIGNMENT
    auto GetEnumFromString(const char* value, DWRITE_TEXT_ALIGNMENT bad_match) noexcept ->DWRITE_TEXT_ALIGNMENT;
    // get d2d extend mode
    auto GetEnumFromString(const char* value, D2D1_EXTEND_MODE bad_match) noexcept ->D2D1_EXTEND_MODE;
    // get D2D1_TEXT_ANTIALIAS_MODE
    auto GetEnumFromString(const char* value, D2D1_TEXT_ANTIALIAS_MODE bad_match) noexcept ->D2D1_TEXT_ANTIALIAS_MODE;
    // get d2d interpolation mode
    auto GetEnumFromString(const char* value, D2D1_INTERPOLATION_MODE bad_match) noexcept ->D2D1_INTERPOLATION_MODE;
    //    ------------------------------------------------------
    //    ------------------ INLINE OVERLOAD -------------------
    //    ------------------------------------------------------
    // get animation type
    inline auto GetEnumFromXml(pugi::xml_node node, AnimationType bad_match,
        const char* attribute = "animationtype", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get d2d interpolation mode
    inline auto GetEnumFromXml(pugi::xml_node node, D2D1_INTERPOLATION_MODE bad_match,
        const char* attribute = "interpolation", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get d2d extend mode
    inline auto GetEnumFromXml(pugi::xml_node node, D2D1_EXTEND_MODE bad_match,
        const char* attribute = "extend", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get longui bitmap render rule
    inline auto GetEnumFromXml(pugi::xml_node node, BitmapRenderRule bad_match,
        const char* attribute = "rule", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get longui richtype
    inline auto GetEnumFromXml(pugi::xml_node node, RichType bad_match,
        const char* attribute = "richtype", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_FONT_STYLE
    inline auto GetEnumFromXml(pugi::xml_node node,  DWRITE_FONT_STYLE bad_match,
        const char* attribute = "style", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_FONT_STRETCH
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_FONT_STRETCH bad_match,
        const char* attribute = "stretch", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_FLOW_DIRECTION
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_FLOW_DIRECTION bad_match,
        const char* attribute = "flowdirection", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_READING_DIRECTION
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_READING_DIRECTION bad_match,
        const char* attribute = "readingdirection", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_READING_DIRECTION
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_WORD_WRAPPING bad_match,
        const char* attribute = "wordwrapping", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_PARAGRAPH_ALIGNMENT
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_PARAGRAPH_ALIGNMENT bad_match,
        const char* attribute = "valign", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_PARAGRAPH_ALIGNMENT
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_TEXT_ALIGNMENT bad_match,
        const char* attribute = "align", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get d2d text anti-mode
    inline auto GetEnumFromXml(pugi::xml_node node, D2D1_TEXT_ANTIALIAS_MODE bad_match,
        const char* attribute = LongUI::XMLAttribute::WindowTextAntiMode, const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
}}

