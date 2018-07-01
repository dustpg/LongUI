#include <algorithm>
#include <cstring>
#include <container/pod_vector.h>
#include <core/ui_malloc.h>

#ifndef NDEBUG
extern "C" void ui_debug_output_infow(const wchar_t*);
#endif

// TODO: 等待aligned_malloc进入C++标准, 使用m_uAlignLen

PCN_NOINLINE
/// <summary>
/// Initializes a new instance of the <see cref="vector_base" /> class.
/// </summary>
/// <param name="x">The x.</param>
LongUI::POD::detail::vector_base::vector_base(const vector_base& x) noexcept :
m_uByteLen(x.m_uByteLen), 
m_uVecCap(x.get_extra_fbl()), 
m_uVecLen(x.m_uVecLen), 
m_uExtra(x.m_uExtra) {
    // 没有必要?
    if (!m_uVecLen) {
        m_uVecCap = 0;
        return;
    }
    // 申请对应大小内存
    this->alloc_memory(m_uVecLen);
    if (!is_ok()) return;
    const auto cpylen = m_uVecLen + get_extra_buy();
    std::memcpy(m_pData, x.m_pData, cpylen*m_uByteLen);
#ifndef NDEBUG
    if (this->get_extra_buy()) {
        wchar_t buf[64];
        static int counter = 0; ++counter;
        auto code = std::swprintf(
            buf, sizeof(buf) / sizeof(buf[0]),
            L"pod::vector copy ctor called: #%4d\n",
            counter
        );
        assert(code > 0);
        ui_debug_output_infow(buf);
    }
#endif
}

/// <summary>
/// Forces the reset.
/// </summary>
/// <returns></returns>
void LongUI::POD::detail::vector_base::force_reset() noexcept {
    m_pData = this->invalid_heap();
    m_uVecCap = this->get_extra_fbl();
}

PCN_NOINLINE
/// <summary>
/// Initializes a new instance of the <see cref="vector_base" /> class.
/// </summary>
/// <param name="x">The x.</param>
LongUI::POD::detail::vector_base::vector_base(vector_base && x) noexcept: 
m_pData(x.m_pData),
m_uByteLen(x.m_uByteLen), 
m_uVecCap(x.m_uVecCap),
m_uVecLen(x.m_uVecLen), 
m_uExtra(x.m_uExtra) {
    assert(&x != this && "can not move same object");
    // 对面有效堆数据
    if (x.is_valid_heap(x.m_pData)) {
        x.force_reset();
        x.m_uVecLen = 0;
    }
    // 自己也必须是缓存数据
    else {
        m_pData = this->invalid_heap();
        const auto cpylen = m_uVecLen + get_extra_buy();
        std::memcpy(m_pData, x.m_pData, cpylen*m_uByteLen);
    }
}


PCN_NOINLINE
/// <summary>
/// Initializes a new instance of the <see cref="vector_base" /> class.
/// </summary>
/// <param name="bl">The bye length for single object.</param>
/// <param name="ex">The extra-data.</param>
LongUI::POD::detail::vector_base::vector_base(
    size_type bl, uhalfptr_t ex) noexcept :
m_uByteLen(static_cast<uhalfptr_t>(bl)),
m_uExtra(ex) {
    m_uVecCap = this->get_extra_fbl();
    assert(this->get_extra_ali() != 0);
}


/// <summary>
/// Allocs the memory.
/// </summary>
/// <param name="len">The length.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
inline auto LongUI::POD::detail::vector_base::malloc(size_type len) noexcept -> char * {
    assert(len && "cannot malloc 0 length");
    return reinterpret_cast<char*>(LongUI::NormalAlloc(len));
}

/// <summary>
/// Frees the memory.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
inline void LongUI::POD::detail::vector_base::free(char* ptr) noexcept {
    return LongUI::NormalFree(ptr);
}

/// <summary>
/// Reallocs the memory .
/// </summary>
/// <param name="">The .</param>
/// <param name="len">The length.</param>
/// <returns></returns>
inline auto LongUI::POD::detail::
vector_base::realloc(char* ptr, size_type len) noexcept -> char * {
    assert((ptr || len) && "cannot realloc nullptr to 0");
    return reinterpret_cast<char*>(LongUI::NormalRealloc(ptr, len));
}

/// <summary>
/// Ends this instance.
/// </summary>
/// <returns></returns>
auto LongUI::POD::detail::vector_base::end() const noexcept ->const void* {
    return m_pData + m_uByteLen * m_uVecLen;
}

PCN_NOINLINE
/// <summary>
/// Erases the specified position.
/// </summary>
/// <param name="start">The start.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::erase(size_type start, size_type end) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(start <= size() && "erase:[start] out of range");
    end = std::min(end, size());
    // 移动内存
    if (end != m_uVecLen) {
        // 剩余的长度
        const auto remain = (m_uVecLen - end) * m_uByteLen;
        const auto wptr = m_pData + start * m_uByteLen;
        const auto rptr = m_pData + end * m_uByteLen;
        std::memmove(wptr, rptr, remain);
    }
    // 直接减少长度即可
    m_uVecLen -= static_cast<uint32_t>(end - start);
}

PCN_NOINLINE
/// <summary>
/// Frees the memory.
/// </summary>
/// <returns></returns>
void LongUI::POD::detail::vector_base::free_memory() noexcept {
    if (this->is_valid_heap(m_pData)) this->free(m_pData);
#ifndef NDEBUG
    m_pData = nullptr; m_pData = this->invalid_heap();
#endif
}

PCN_NOINLINE
/// <summary>
/// Allocs the memory.
/// </summary>
/// <param name="len">The length.</param>
/// <remarks>不会修改 m_uVecLen<remarks>
/// <returns></returns>
void LongUI::POD::detail::vector_base::alloc_memory(size_type len) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
#ifndef NDEBUG
    assert(!is_valid_heap(m_pData) && "free memory first");
#endif
    // 额外内存
    const auto cap = len + this->get_extra_buy();
    const auto bytelen = m_uByteLen * cap;
    // 申请内存
    if (const auto ptr = this->malloc(bytelen)) {
        m_pData = ptr;
        m_uVecCap = static_cast<uint32_t>(cap);
    }
    // 内存不足
    else {
        this->force_reset();
    }
}

PCN_NOINLINE
/// <summary>
/// Swaps the specified another.
/// </summary>
/// <param name="x">The x.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::swap(vector_base & x) noexcept {
    assert(m_uByteLen == x.m_uByteLen);
    assert(m_uExtra == x.m_uExtra);
    std::swap(m_uVecLen, x.m_uVecLen);
    std::swap(m_uVecCap, x.m_uVecCap);
    std::swap(m_pData, x.m_pData);
    if (m_pData == x.invalid_heap()) m_pData = this->invalid_heap();
    if (x.m_pData == invalid_heap()) x.m_pData = x.invalid_heap();
}

PCN_NOINLINE
/// <summary>
/// Operators the op_equal.
/// </summary>
/// <param name="x">The x.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::op_equal(const vector_base& x) noexcept {
    // 容量允许时
    if (capacity() >= x.size() + get_extra_buy()) {
        m_uVecLen = x.m_uVecLen;
        const auto cpylen = m_uVecLen + get_extra_buy();
        std::memcpy(m_pData, x.m_pData, cpylen * m_uByteLen);
    }
    // 重新申请
    else {
        this->free_memory();
        ctor_dtor<vector_base>::create(this, x);
    }
}

PCN_NOINLINE
/// <summary>
/// Assigns the specified first.
/// </summary>
/// <param name="first">The first.</param>
/// <param name="last">The last.</param>
void LongUI::POD::detail::vector_base::assign(const char* first, const char* last) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    this->clear();
    if (!first) return;
    assert(last >= first && "bad data");
    const auto len = (last - first) / m_uByteLen;
    m_uVecLen = static_cast<uint32_t>(len);
    assert(m_uVecLen * m_uByteLen == std::uintptr_t(last - first) && "bad align");
    // 超过容器当前上限
    if (m_uVecLen > m_uVecCap) { 
        // 拥有额外数据的必须自己保证
        assert(this->get_extra_buy() == 0 && "you must call reserve before, if ex-buy");
        free_memory(); 
        alloc_memory(len); 
    }
    // 数据有效
    if (is_ok()) std::memcpy(m_pData, first, last - first);
}

PCN_NOINLINE
/// <summary>
/// Assigns the specified n.
/// </summary>
/// <param name="n">The n.</param>
/// <param name="data">The data.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::assign(size_type n, const char* data) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    this->clear(); if (!n) return;
    assert(data && "bad data");
    m_uVecLen = static_cast<uint32_t>(n);
    // 超过容器当前上限
    if (n > m_uVecCap) { 
        // 拥有额外数据的必须自己保证
        assert(this->get_extra_buy() == 0 && "you must call reserve before, if ex-buy");
        free_memory(); 
        alloc_memory(n);
    }
    // 数据有效
    if (is_ok()) {
        auto address = m_pData;
        const auto step = m_uByteLen;
        auto copy = [=]() mutable noexcept {
            std::memcpy(address, data, step);
            address += step;
        };
        for (size_type i = 0; i != n; ++i) copy();
    }
}

PCN_NOINLINE
/// <summary>
/// Pushes the back.
/// </summary>
/// <param name="data">The data.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::push_back(const char* data) noexcept {
    // XXX: 针对小于sizeof(void*)对象优化, 其他常用函数也是如此
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(m_uVecLen <= m_uVecCap && "bad case");
    // 重新申请空间
    if (m_uVecLen == m_uVecCap) {
        // 分配策略: 在reserve中实现
        this->reserve(m_uVecLen + 1);
        // 内存不足
        if (!is_ok()) return;
    }
    // 写入数据
    const auto write_ptr = m_pData + m_uByteLen * m_uVecLen;
    // 对于指针大小的手动优化
    if (m_uByteLen == sizeof(void*))
        std::memcpy(write_ptr, data, sizeof(void*));
    // 其他情况
    else
        std::memcpy(write_ptr, data, m_uByteLen);
    // 增加了
    ++m_uVecLen;
    assert(size() <= max_size());
}

PCN_NOINLINE
/// <summary>
/// Resizes the specified n.
/// </summary>
/// <param name="n">The n.</param>
/// <param name="data">The data.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::resize(size_type n, const char* data) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    if (n > size()) {
        this->reserve(n);  if (!is_ok()) return;
        auto write_ptr = m_pData + m_uByteLen*m_uVecLen;
        for (size_t i = 0; i != n - size(); ++i) {
            std::memcpy(write_ptr, data, m_uByteLen);
            write_ptr += m_uByteLen;
        }
    }
    m_uVecLen = static_cast<uint32_t>(n);
    assert(size() <= max_size());
}

PCN_NOINLINE
/// <summary>
/// Reserves the specified n.
/// </summary>
/// <param name="n">The n.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::reserve(size_type n) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(n < (1 << 20) && "to huge");
    // 获取目标容量
    const auto cap = static_cast<uint32_t>(n + this->get_extra_buy());
    // 扩容: 不足就重新申请
    if (m_uVecCap < cap) {
        auto new_cap = cap;
        // 额外分配策略是加一半
        const auto target = m_uVecCap + m_uVecCap / 2;
        // 所以没有一半就加上一半
        if (new_cap < target) new_cap = target;
        // 最少是4
        if (new_cap < 4) new_cap = 4;
        // 如果才加
        const auto bytelen = new_cap * m_uByteLen;
#if 0
        // 有效堆, 用realloc扩容
        if (this->is_valid_heap(m_pData)) {
            m_pData = this->realloc(m_pData, bytelen);
        }
        // 无效堆, 用realloc/malloc申请, memcpy复制
        else {
            m_pData = this->realloc(nullptr, bytelen);
        }
#else
        // 有效堆
        const auto ptr = this->is_valid_heap(m_pData) ? m_pData : nullptr;
        // 使用realloc扩容
        if (const auto data = this->realloc(ptr, bytelen)) {
            // 无效堆: 复制旧数据
            m_pData = data;
            if (!ptr) std::memcpy(m_pData, this->invalid_heap(), m_uVecCap*m_uByteLen);
            m_uVecCap = new_cap;
        }
        // 内存不足, 释放旧数据
        else {
            if (ptr) std::free(m_pData);
            this->force_reset();
        }
#endif
    }
}

PCN_NOINLINE
/// <summary>
/// Inserts the helper.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="n">The n.</param>
/// <returns></returns>
bool LongUI::POD::detail::vector_base::insert_helper(size_type pos, size_type n) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    // 越界操作
    assert(pos <= size() && "out of range");
    // 计算长度
    const size_type remain = m_uVecCap - m_uVecLen;
    // 长度不足则重新申请
    if (remain < n) this->reserve(size() + n);
    // 数据无效
    if (!is_ok()) return false;
    // 数据插入中间?
    if (pos != size()) {
        const size_type S = m_uByteLen;
        const auto src_pos = m_pData + pos*S;
        const auto des_pos = src_pos + n*S;
        // 移动内存
        std::memmove(des_pos, src_pos, (size() - pos)*S);
    }
    // 增加容量
    m_uVecLen += static_cast<uint32_t>(n);
    assert(size() <= max_size());
    return true;
}

PCN_NOINLINE
/// <summary>
/// Inserts the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="first">The first.</param>
/// <param name="last">The last.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::insert(
    size_type pos, const char* first, const char* last) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    // 空数据
    if (!first) return;
    // 尾指针不允许空
    assert(last && "bad pointer");
    // 准备数据
    if (insert_helper(pos, (last - first) / m_uByteLen)) {
        std::memcpy(m_pData + pos*m_uByteLen, first, (last - first));
    }
}

PCN_NOINLINE
/// <summary>
/// Inserts the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="n">The n.</param>
/// <param name="data">The data.</param>
/// <returns></returns>
void LongUI::POD::detail::vector_base::insert(
    size_type pos, size_type n, const char* data) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    // 这种情况不允许空对象
    assert(data && "bad data ptr");
    // 准备数据
    if (insert_helper(pos, n)) {
        const auto S = m_uByteLen;
        auto write_pos = m_pData + pos*S;
        for (size_type i = 0; i != n; ++i) {
            std::memcpy(write_pos, data, S);
            write_pos += S;
        }
    }
}

PCN_NOINLINE
/// <summary>
/// Shrinks to fit.
/// </summary>
/// <returns></returns>
void LongUI::POD::detail::vector_base::shrink_to_fit() noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    // 无效
    if (!this->is_valid_heap(m_pData)) return;
    // 为空直接释放
    if (m_uVecLen == 0) {
        this->free(m_pData);
        this->force_reset();
        return;
    }
    // 因为使用使用realloc继续缩水
    const auto cap = m_uVecLen + this->get_extra_buy();
    // 所以只要有空间就可以缩
    if (cap < m_uVecCap) {
        const auto bytelen = m_uByteLen * cap;
        // 使用realloc缩水
        if (const auto ptr = this->realloc(m_pData, bytelen)) {
            // 需要检查一下vector_base::realloc的实现
            assert(ptr == m_pData && "check vector_base::realloc implment");
            m_pData = ptr;
            m_uVecCap = cap;
        }
        // 失败时原始指针不变(缩小时基本不可能, 不过看具体实现)
        else {

        }
    }
}