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

#include <cstdint>
#include <combaseapi.h>

// longui namespace
namespace LongUI { namespace Helper {
    // counter based COM Interface, 0 , wiil be deleted
    template <typename InterfaceChain, typename CounterType = size_t>
    class ComBase : public InterfaceChain {
    public:
        // constructor inline ver.
        explicit ComBase(ULONG init_count=1) noexcept : m_refValue(init_count) { }
        // IUnknown interface
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, OUT void** ppObject) noexcept final override {
            *ppObject = nullptr;
            InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            if (*ppObject == nullptr)
                return E_NOINTERFACE;
            this->ComBase::AddRef();
            return S_OK;
        }
        // add ref-counter
        virtual ULONG STDMETHODCALLTYPE AddRef() noexcept final override { return static_cast<ULONG>(++m_refValue); }
        // delete when 0
        virtual ULONG STDMETHODCALLTYPE Release() noexcept final override {
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
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, OUT void** ppObject) noexcept final override {
            *ppObject = nullptr;
            InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            if (*ppObject == nullptr)
                return E_NOINTERFACE;
            this->AddRef();
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
    class QiList<InterfaceName, QiListNil> : public InterfaceName {
    public:
        inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) noexcept {
            if (iid != LongUI::GetIID<InterfaceName>()) return;
            *ppObject = static_cast<InterfaceName*>(this);
        }
    };
}}

