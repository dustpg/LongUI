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

// LongUI namespace
namespace LongUI{
    // CUIFileLoader类
    class CUIFileLoader{
    private:
        // 私有化构造函数
        CUIFileLoader() noexcept{}
        // 私有化析构函数
        ~CUIFileLoader() noexcept{
            if (m_pData){
                free(m_pData);
                m_pData = nullptr;
            }
        }
    public:
        // 读取文件
        bool LongUIMethodCall ReadFile(WCHAR* file_name) noexcept;
        // 获取数据指针
        auto LongUIMethodCall GetData() const  noexcept{ return m_pData; }
        // 获取数据长度
        auto LongUIMethodCall GetLength()const noexcept{ return m_cLength; }
    private:
        // 数据指针
        void*           m_pData = nullptr;
        // 数据长度
        size_t          m_cLength = 0;
        // 实际长度
        size_t          m_cLengthReal = 0;
    public:
        // 主线程线程单例
        static CUIFileLoader s_instanceForMainThread;
    };
}
#define UIFileLoader (LongUI::CUIFileLoader::s_instanceForMainThread)