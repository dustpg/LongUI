#include <container/nonpod_vector.h>
#include <core/ui_malloc.h>

#include <cstdlib>
#include <cassert>
#include <algorithm>

#ifndef NDEBUG
#include <debugger/ui_debug.h>
#endif

#ifdef LUI_NONPOD_VECTOR
using namespace LongUI::NonPOD;

/// <summary>
/// Checks the aligned.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
inline void detail::vector_base::check_aligned(const char* ptr) noexcept {
    assert(reinterpret_cast<uintptr_t>(ptr) % m_uAligned == 0);
}

/// <summary>
/// Frees the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
inline void detail::vector_base::free(char* p) noexcept {
    return LongUI::NormalFree(p);
}

/// <summary>
/// Mallocs the specified l.
/// </summary>
/// <param name="l">The l.</param>
/// <returns></returns>
inline char* detail::vector_base::malloc(size_t l) noexcept {
    return reinterpret_cast<char*>(LongUI::NormalAlloc(l));
}

/// <summary>
/// Reallocs the specified p.
/// </summary>
/// <param name="p">The p.</param>
/// <param name="l">The l.</param>
/// <returns></returns>
inline char* detail::vector_base::realloc(char* p, size_t l) noexcept {
    return reinterpret_cast<char*>(LongUI::NormalRealloc(p, l));
}

/// <summary>
/// Tries the realloc.
/// </summary>
/// <param name="p">The p.</param>
/// <param name="l">The l.</param>
/// <returns></returns>
inline char* detail::vector_base::try_realloc(char* p, size_t l) noexcept {
    // TODO: 实现try_realloc
    return reinterpret_cast<char*>(LongUI::NormalAlloc(l)); p;
}

PCN_NOINLINE
/// <summary>
/// Vectors the base.
/// </summary>
/// <param name="vt">The vt.</param>
/// <param name="len">The length.</param>
/// <param name="ex">The ex.</param>
/// <returns></returns>
detail::vector_base::vector_base(
    const vtable_helper* vt, uint16_t len, uint16_t ex) noexcept
    : m_pVTable(vt), m_uByteLen(len), m_uAligned(ex) {

}

PCN_NOINLINE
/// <summary>
/// Frees the data.
/// </summary>
/// <returns></returns>
void detail::vector_base::free_data() noexcept {
    // 有效数据
    if (m_pData) {
        // 释放对象
        this->release_objs(m_pData, m_uVecLen);
        // 释放内存
        this->free(m_pData);
    }
}

PCN_NOINLINE
/// <summary>
/// Vectors the base.
/// </summary>
/// <param name="x">The x.</param>
/// <returns></returns>
detail::vector_base::vector_base(vector_base && x) noexcept :
m_pVTable(x.m_pVTable),
m_pData(x.m_pData),
m_uVecLen(x.m_uVecLen),
m_uVecCap(x.m_uVecCap),
m_uByteLen(x.m_uByteLen),
m_uAligned(x.m_uAligned)
{
    x.m_pData = nullptr;
    x.m_uVecLen = 0;
    x.m_uVecCap = 0;
}

PCN_NOINLINE
/// <summary>
/// Vectors the base.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
detail::vector_base::vector_base(const vector_base& x) noexcept :
m_pVTable(x.m_pVTable),
m_uByteLen(x.m_uByteLen),
m_uAligned(x.m_uAligned)
{
    // 没有对象无需复制
    if (!x.m_uVecLen) return;
    // 复制对象
    const auto alllen = x.m_uByteLen * x.m_uVecCap;
    if (const auto ptr = this->malloc(alllen)) {
        // 对齐检查
        this->check_aligned(ptr);
        // 写入数据
        m_pData = ptr;
        m_uVecCap = x.m_uVecCap;
        m_uVecLen = x.m_uVecLen;
        // 复制对象
        this->copy_objects(m_pData, x.m_pData, x.m_uVecLen);
    }
}


PCN_NOINLINE
/// <summary>
/// Ops the equal.
/// </summary>
/// <param name="x">The x.</param>
/// <returns></returns>
void detail::vector_base::op_equal(const vector_base& x) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(m_uByteLen == x.m_uByteLen && "m_uByteLen must be same");
    // 长度够用?
    if (capacity() >= x.size()) {
        // 释放对象
        this->release_objs(m_pData, m_uVecLen);
        // 复制对象
        this->copy_objects(m_pData, x.m_pData, x.m_uVecLen);
        // 修改长度
        m_uVecLen = x.size();
        assert(!"NOT IMPL");
    }
    // 重新构造
    else {
        // 释放
        this->free_data();
        ctor_dtor<vector_base>::create(this, x);
    }
}

PCN_NOINLINE
/// <summary>
/// Emplaces the back.
/// </summary>
/// <returns></returns>
char*detail::vector_base::emplace_back() noexcept {
    void* const func = m_pVTable->create_obj;
    this->push_back_help(nullptr, func);
    return is_ok() ? last_n() : nullptr;
}

PCN_NOINLINE
/// <summary>
/// Pushes the back copy.
/// </summary>
/// <param name="obj">The object.</param>
/// <returns></returns>
void detail::vector_base::push_back_copy(const char* obj) noexcept {
    void* const func = m_pVTable->copy_t_obj;
    this->push_back_help(const_cast<char*>(obj), func);
}

PCN_NOINLINE
/// <summary>
/// Pushes the back move.
/// </summary>
/// <param name="obj">The object.</param>
/// <returns></returns>
void detail::vector_base::push_back_move(char* obj) noexcept {
#ifndef NDEBUG
    LUIDebug(Warning) << "[Recommended Use] emplace back" << endl;
#endif
    void* const func = m_pVTable->move_t_obj;
    this->push_back_help(obj, func);
}

PCN_NOINLINE
/// <summary>
/// Pushes the back help.
/// </summary>
/// <param name="obj">The object.</param>
/// <param name="call">The call.</param>
/// <returns></returns>
void detail::vector_base::push_back_help(char* obj, void* func) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(m_uVecLen <= m_uVecCap && "bad case");
    // 重新申请空间
    if (m_uVecLen == m_uVecCap) {
        // 分配策略: 每次加一半
        const uint32_t newlen = m_uVecLen + m_uVecLen / 2;
        // 分配策略: 4起步
        this->reserve(std::max(newlen, static_cast<uint32_t>(4)));
        //this->reserve(newlen);
        if (!is_ok()) return;
    }
    // 复制/移动对象
    const auto ptr = m_pData + m_uVecLen * m_uByteLen;
    // 万能转换
    union { 
        void* func_ptr; 
        void(*call)(char*, char*) noexcept;
        void(*call0)(char*) noexcept;
    };
    func_ptr = func;
    // 调用对应构造函数
    obj ? call(ptr, obj) : call0(ptr);
    // +1s
    m_uVecLen++;
}

PCN_NOINLINE
/// <summary>
/// Pops the back.
/// </summary>
/// <returns></returns>
void detail::vector_base::pop_back() noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    // 没有东西?!
    assert(m_uVecLen && "no objects");
    const auto len = --m_uVecLen;
    const auto ptr = m_pData + len * m_uByteLen;
    m_pVTable->delete_obj(ptr);
}

PCN_NOINLINE
/// <summary>
/// Reserves the specified length.
/// </summary>
/// <param name="cap">The cap.</param>
/// <param name="offset">The offset.</param>
/// <returns></returns>
void detail::vector_base::reserve(uint32_t cap) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    // 扩容: 不足就重新申请
    if (cap <= m_uVecCap) return;
    // 使用realloc扩容
    const auto old = m_pData;
    const auto len = m_uVecLen;
    const auto alllen = cap * m_uByteLen;
    const auto ptr = this->try_realloc(old, alllen);
    // 对齐检查
    this->check_aligned(ptr);
    // 申请失败 释放老数据
    if (!ptr) {
        m_pData = 0;
        m_uVecLen = 0;
        m_uVecCap = 0;
        goto release_old_data;
    }
    // 申请成功
    m_uVecCap = cap;
    // 不同地址需要移动对象
    if (ptr != old) {
        // 移动原来对象到现在这里
        this->move_objects((m_pData = ptr), old, len);
        // 释放旧数据
    release_old_data:
        // 释放老对象
        this->release_objs(old, len);
        // 释放内存
        this->free(old);
    }
}

PCN_NOINLINE
/// <summary>
/// Does the objects.
/// </summary>
/// <param name="obj">The object.</param>
/// <param name="func">The function.</param>
/// <param name="count">The count.</param>
/// <param name="bytelen">The bytelen.</param>
/// <returns></returns>
void detail::vector_base::do_objects(char* obj, void* func, uint32_t count, uint32_t bytelen) noexcept {
    // 万能转换
    union { void* ptr; void(*call)(char*) noexcept; }; ptr = func;
    // 每个对象
    for (decltype(m_uVecLen) i = 0; i != count; ++i) {
        call(obj + bytelen * i);
    }
}


PCN_NOINLINE
/// <summary>
/// Does the objobj.
/// </summary>
/// <param name="obj">The object.</param>
/// <param name="obj2">The obj2.</param>
/// <param name="func">The function.</param>
/// <param name="count">The count.</param>
/// <param name="bytelen">The bytelen.</param>
/// <returns></returns>
void detail::vector_base::do_objobj(
    char * obj, const char* obj2, void * func, 
    uint32_t count, uint32_t bytelen) noexcept {
    // 万能转换
    union { void* ptr; void(*call)(char*, const char*) noexcept; }; ptr = func;
    // 每个对象
    for (decltype(m_uVecLen) i = 0; i != count; ++i) {
        call(obj + bytelen * i, obj2 + bytelen * i);
    }
}

/// <summary>
/// Releases the objs.
/// </summary>
/// <param name="">The .</param>
/// <param name="count">The count.</param>
/// <returns></returns>
void detail::vector_base::release_objs(char* ptr, uint32_t count) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    this->do_objects(ptr, m_pVTable->delete_obj, count, m_uByteLen);
}

/// <summary>
/// Creates the objs.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="count">The count.</param>
/// <returns></returns>
void detail::vector_base::create_objs(char* ptr, uint32_t count) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    this->do_objects(ptr, m_pVTable->create_obj, count, m_uByteLen);
}

/// <summary>
/// Moves the objects.
/// </summary>
/// <param name="to">To.</param>
/// <param name="from">From.</param>
/// <param name="count">The count.</param>
/// <returns></returns>
void detail::vector_base::move_objects(char* to, char* from, uint32_t count) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(to != from && "cannot move self to self");
    this->do_objobj(to, from, m_pVTable->move_t_obj, count, m_uByteLen);
}

/// <summary>
/// Copies the objects.
/// </summary>
/// <param name="to">To.</param>
/// <param name="from">From.</param>
/// <param name="count">The count.</param>
/// <returns></returns>
void detail::vector_base::copy_objects(char* to, const char* from, uint32_t count) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(to != from && "cannot copy self to self");
    this->do_objobj(to, from, m_pVTable->copy_t_obj, count, m_uByteLen);
}

PCN_NOINLINE
/// <summary>
/// Shrinks to fit.
/// </summary>
/// <returns></returns>
void detail::vector_base::shrink_to_fit() noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    // 释放多余内存
    if (m_uVecCap > m_uVecLen) {
        // 收缩内存不存在失败?
        const auto len = m_uVecLen * m_uByteLen;
        const auto ptr = this->realloc(m_pData, len);
        assert(ptr == m_pData);
    }
}

PCN_NOINLINE
/// <summary>
/// Erases the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void detail::vector_base::erase(uint32_t pos, uint32_t len) noexcept {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(pos < m_uVecLen && (pos + len) <= m_uVecLen && "out of range");
    // 位置不在服务区
    if (pos >= m_uVecLen) return;
    // 计算服务范围
    const auto end_pos = std::min(pos + len, m_uVecLen);
    const auto reallen = end_pos - pos;
    const auto start = m_pData + pos * m_uByteLen;
    // 释放对象
    this->release_objs(start, reallen);
    // 删除中间的?
    if (end_pos != m_uVecLen) {
        const auto move_from = start + reallen * m_uByteLen;
        const auto move_count = m_uVecLen - end_pos;
        // 将后面的移动过来
        this->move_objects(start, move_from, move_count);
        // 删除后面的
        this->release_objs(move_from, move_count);
    }
    // 调整长度
    m_uVecLen -= reallen;
}


/// <summary>
/// Ends the n.
/// </summary>
/// <returns></returns>
auto detail::vector_base::end_c() const noexcept -> const char * {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    return m_pData + m_uByteLen * m_uVecLen;
}

/// <summary>
/// Lasts the c.
/// </summary>
/// <returns></returns>
auto detail::vector_base::last_c() const noexcept -> const char * {
    assert(m_uByteLen && "m_uByteLen cannot be 0");
    assert(m_uVecLen && "no object");
    return m_pData + m_uByteLen * (m_uVecLen - 1);
}

PCN_NOINLINE
/// <summary>
/// Resizes the specified length.
/// </summary>
/// <param name="len">The length.</param>
/// <returns></returns>
void detail::vector_base::resize(uint32_t len) noexcept {
    const auto this_len = m_uVecLen;
    // 扩大
    if (len > this_len) {
        // 保证空间
        this->reserve(len);
        // 内存不足
        if (!this->is_ok()) return;
        // 创建对象
        this->create_objs(m_pData + m_uByteLen * this_len, len - this_len);
    }
    // 缩小
    else {
        this->release_objs(m_pData + len * m_uByteLen, this_len - len);
    }
    // 修改长度
    m_uVecLen = len;
}


PCN_NOINLINE
/// <summary>
/// Clears this instance.
/// </summary>
/// <returns></returns>
void detail::vector_base::clear() noexcept {
    // 有效数据
    if (m_pData) {
        // 释放对象
        this->release_objs(m_pData, m_uVecLen);
        // 清0
        m_uVecLen = 0;
    }
}


PCN_NOINLINE
/// <summary>
/// Assigns the range.
/// </summary>
/// <param name="data">The data.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void detail::vector_base::assign_range(const char* data, uint32_t len) noexcept {
    // 清空
    this->clear();
    // 检查长度
    this->reserve(len);
    // 有效
    if (this->is_ok()) {
        // 复制对象
        this->copy_objects(m_pData, data, len);
    }
}

PCN_NOINLINE
/// <summary>
/// Assigns the count.
/// </summary>
/// <param name="data">The data.</param>
/// <param name="count">The count.</param>
/// <returns></returns>
void detail::vector_base::assign_count(const char* data, uint32_t count) noexcept {
    // 清空
    this->clear();
    // 检查长度
    this->reserve(count);
    // 有效
    if (this->is_ok()) {
        const auto ptr0 = m_pData;
        const auto table = m_pVTable;
        // 复制对象
        for (uint32_t i = 0; i != count; ++i) {
            table->copy_t_obj(ptr0 + m_uByteLen * i, data);
        }
    }
}


//PCN_NOINLINE
///// <summary>
///// Emplaces the objs.
///// </summary>
///// <param name="pos">The position.</param>
///// <param name="len">The length.</param>
///// <returns></returns>
//char*detail::vector_base::emplace_objs(uint32_t pos, uint32_t len) noexcept {
//    void* const func = m_pVTable->create_obj;
//    this->insert_help(nullptr, pos, len, func);
//    return is_ok() ? m_pData + m_uByteLen * pos : nullptr;
//}

//PCN_NOINLINE
///// <summary>
///// Inserts the help.
///// </summary>
///// <param name="obj">The object.</param>
///// <param name="pos">The position.</param>
///// <param name="n">The n.</param>
///// <param name="func">The function.</param>
///// <returns></returns>
//void detail::vector_base::insert_help(char* obj, uint32_t pos, uint32_t n, void* func) noexcept {
//    assert(m_uByteLen && "m_uByteLen cannot be 0");
//    assert(n && "cannot insert 0 element");
//    // move 操作只能一位
//#ifndef NDEBUG
//    if (func == m_pVTable->move_t_obj) {
//        assert(n == 1 && "move only one");
//    }
//#endif
//    // 插入位置
//    const auto this_len = m_uVecLen;
//    // 越界操作
//    assert(pos <= this_len && "out of range");
//    // XXX: 重新申请会导致对象移动两次?!
//
//    // 长度不足则重新申请
//    const uint32_t remain = m_uVecCap - m_uVecLen;
//    if (remain < n) this->reserve(size() + n);
//    // 数据无效
//    if (!is_ok()) return;
//    // 移动插入位置后的对象
//
//    // 万能转换
//    union {
//        void* func_ptr;
//        void(*call)(char*, char*) noexcept;
//        void(*call0)(char*) noexcept;
//    };
//    func_ptr = func;
//    // 存在操作对象
//    if (obj) {
//        // 全体调用
//        const auto ptr = m_pData + m_uByteLen * pos;
//        for (uint32_t i = 0; i != n; ++i) {
//            call(ptr + i * m_uByteLen, obj);
//        }
//    }
//    else {
//        // 全体调用
//        const auto ptr = m_pData + m_uByteLen * pos;
//        for (uint32_t i = 0; i != n; ++i) {
//            call0(ptr + i * m_uByteLen);
//        }
//    }
//    // +ns
//    m_uVecLen += n;
//}
#endif