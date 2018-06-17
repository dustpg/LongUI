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
#include <new>
#include "../luiconf.h"

// longui namespace
namespace UI {
    /// <summary>
    /// control node
    /// </summary>
    struct Node { Node* prev, *next; };
    /// <summary>
    /// single object
    /// </summary>
    struct CUISingleObject {
        // throw new []
        auto operator new(size_t size) ->void* = delete;
        // throw new []
        auto operator new[](size_t size) ->void* = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
    };
    /// <summary>
    /// normal single object
    /// </summary>
    struct CUISingleNormalObject : CUISingleObject {
        // nothrow new 
        void*operator new(size_t size, const std::nothrow_t&) noexcept { return UI::NormalAlloc(size); };
        // nothrow delete 
        void operator delete(void* address, const std::nothrow_t&) { UI::NormalFree(address); }
        // delete
        void operator delete(void* address) noexcept { UI::NormalFree(address); }
    };
    /// <summary>
    /// small single object
    /// </summary>
    struct CUISingleSmallObject : CUISingleObject {
        // nothrow new 
        void*operator new(size_t size, const std::nothrow_t&) noexcept { return UI::SmallAlloc(size); };
        // nothrow delete 
        void operator delete(void* address, const std::nothrow_t&) { UI::SmallFree(address); }
        // delete
        void operator delete(void* address) noexcept { UI::SmallFree(address); }
    };
}