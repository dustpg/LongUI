#include <cstring>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <container/pod_hash.h>
#include <core/ui_malloc.h>

/// <summary>
/// Mallocs the specified length.
/// </summary>
/// <param name="len">The length.</param>
/// <returns></returns>
inline void* LongUI::POD::detail::hash_base::malloc(size_t len) noexcept {
    return LongUI::NormalAlloc(len);
}

/// <summary>
/// Frees the specified PTR.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
inline void LongUI::POD::detail::hash_base::free(void* ptr) noexcept {
    return LongUI::NormalFree(ptr);
}

// detail namespace
namespace LongUI { namespace detail {
    // nearset pointer size
    template<typename T>
    inline T nearest_psize(T size) noexcept {
        constexpr T pointer_1 = sizeof(void*) - 1;
        return (size + pointer_1) & (~pointer_1);
    }
    // is power of 2
    template<typename T>
    inline bool is_power_of_2(T n) noexcept {
        return (n & (n - 1)) == 0;
    }
}}

/// <summary>
/// BKDRs the hash.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::BKDRHash(const char * str) noexcept -> uint32_t {
    // 31 131 1313 13131 131313 etc..
    const uint32_t seed = 131;
    uint32_t hash = 0;
    while (*str) hash = hash * seed + (*str++);
    return hash;
}

/// <summary>
/// BKDRs the hash.
/// </summary>
/// <param name="strbgn">The STRBGN.</param>
/// <param name="strend">The strend.</param>
/// <returns></returns>
auto LongUI::BKDRHash(const char* strbgn, const char* strend) noexcept -> uint32_t {
    const uint32_t seed = 131;
    const auto* itr = strbgn;
    const auto itrend = strend;
    uint32_t hash = 0;
    while (itr != itrend) hash = hash * seed + (*itr++);
    return hash;
}

/// <summary>
/// Fors the each.
/// </summary>
/// <param name="call">The call.</param>
/// <returns></returns>
template<typename T>
inline void LongUI::POD::detail::hash_base::for_each(T call) noexcept {
#ifdef UI_HASH_TABLE_NO_ITERATOR
    for (auto itr = m_pBaseTable; itr != m_pBaseTableEnd; ++itr) {
        auto cell = reinterpret_cast<hash_cell*>(*itr);
        while (cell) { auto ptr = cell->next; call(*cell); cell = ptr; }
    }
#else
    for (auto itr = m_itrFirst; itr.bucket != m_pBaseTableEnd; ) {
        auto cell = itr.cell; itr.move_next();
        call(*cell);
    }
#endif
}

/// <summary>
/// Finalizes an instance of the <see cref="hash_base"/> class.
/// </summary>
/// <returns></returns>
LongUI::POD::detail::hash_base::~hash_base() noexcept {
    if (m_pBaseTable) {
        this->clear();
        hash_base::free(m_pBaseTable);
    }
}

/// <summary>
/// Clears this instance.
/// </summary>
/// <returns></returns>
void LongUI::POD::detail::hash_base::clear() noexcept {
    // 释放数据
    this->for_each([](hash_cell& cell) noexcept {
        hash_base::free(&cell);
    });
    // 清空节点
    const auto begin = reinterpret_cast<char*>(m_pBaseTable);
    const auto end = reinterpret_cast<char*>(m_pBaseTableEnd);
    std::memset(begin, 0, end - begin);
    // 标记为空
    m_cItemSize = 0;
}

/// <summary>
/// Initializes a new instance of the <see cref="hash_base"/> class.
/// </summary>
/// <param name="sizeof_T">The sizeof T.</param>
LongUI::POD::detail::hash_base::hash_base(uint32_t sizeof_T) noexcept :
m_cItemByteSize(LongUI::detail::nearest_psize(sizeof_T) + sizeof(hash_cell)),
m_itrFirst(this->end_itr())
{

}

/// <summary>
/// Moves the next.
/// </summary>
void LongUI::POD::detail::hash_iterator::move_next() {
    assert(bucket && "bad bucket");
    assert(*this->bucket != end_symbol && "end of bucket");
    // 有下一个拉链就直接指向，否则查找到最近的bucket
    if (this->cell->next) {
        this->cell = this->cell->next;
        return;
    }
    // 直到bucket有效
    do { ++this->bucket; } while (!*this->bucket);
    // 设置单元格
    this->cell = *this->bucket == end_symbol ? nullptr :
        reinterpret_cast<hash_cell*>(*this->bucket);
}

#if 0
/// <summary>
/// Frees the cell.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void LongUI::POD::detail::hash_base::free_cell(const hash_cell& cell) noexcept {
    auto ptr = reinterpret_cast<const char*>(&cell);
    // 字符指针指向单元末 -> 释放本体
    if (cell.str == ptr + m_cItemByteSize) 
        hash_base::free(const_cast<char*>(ptr));
    // 字符指针非指向单元末 -> 释放有效字符指针
    else if (*cell.str)
        hash_base::free(const_cast<char*>(cell.str));
}
#endif


/// <summary>
/// Allocs the cell.
/// </summary>
/// <param name="str_begin">The string begin.</param>
/// <param name="str_end">The string end.</param>
/// <param name="data">The data.</param>
/// <returns></returns>
auto LongUI::POD::detail::hash_base::alloc_cell(
    const char* str_begin, 
    const char* str_end, 
    const char* data) noexcept -> hash_cell* {
    assert(str_end >= str_begin && "bad string");
    const auto baselen = m_cItemByteSize;
    // TODO: 支持多类型的话需要 + N
    const auto extrlen = str_end - str_begin + 1;
    // 申请内存
    if (const auto ptr = hash_base::malloc(baselen + extrlen)) {
        const auto cell_str = reinterpret_cast<char*>(ptr) + baselen;
        const auto cell = reinterpret_cast<hash_cell*>(ptr);
        cell->next = nullptr;
        cell->str = cell_str;
        std::memcpy(cell_str, str_begin, str_end - str_begin);
        cell_str[str_end - str_begin] = 0;
        std::memcpy(&cell->data, data, baselen - sizeof(hash_cell));
        return cell;
    }
    return nullptr;
}

/// <summary>
/// Finds the specified .
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::POD::detail::hash_base::find(const char* str)const noexcept ->hash_iterator {
    assert(str && "bad string");
    // 有的话
    if (m_cItemSize) {
        const auto celln = m_pBaseTableEnd - m_pBaseTable;
        assert(LongUI::detail::is_power_of_2(celln) && "N must be power of 2");
        const auto code = LongUI::BKDRHash(str);
        const auto index = code & (celln - 1);
        auto cell = reinterpret_cast<hash_cell*>(m_pBaseTable[index]);
        while (cell) {
            if (!std::strcmp(cell->str, str)) return{
                m_pBaseTable + index, cell
            };
            cell = cell->next;
        }
    }
    return end_itr();
}

/// <summary>
/// Finds the specified .
/// </summary>
/// <param name="str_begin">The string begin.</param>
/// <param name="str_end">The string end.</param>
/// <returns></returns>
auto LongUI::POD::detail::hash_base::find(
    const char* str_begin, 
    const char* str_end
) const noexcept -> hash_iterator {
    assert(str_begin && str_end > str_begin);
    // 字符串相同
    const auto is_same = [](const hash_cell& cell, const char* a, const char* b) noexcept {
        const auto old_length = std::strlen(cell.str);
        if (old_length != (b - a)) return false;
        return !std::strncmp(cell.str, a, b - a);
    };
    // 有的话
    if (m_cItemSize) {
        const auto celln = m_pBaseTableEnd - m_pBaseTable;
        assert(LongUI::detail::is_power_of_2(celln) && "N must be power of 2");
        const auto code = LongUI::BKDRHash(str_begin, str_end);
        const auto index = code & (celln - 1);
        auto cell = reinterpret_cast<hash_cell*>(m_pBaseTable[index]);
        while (cell) {
            // XXX: 可以保存必要数据优化
            if (is_same(*cell, str_begin, str_end))
                return { m_pBaseTable + index, cell };
            // 推动
            cell = cell->next;
        }
    }
    return end_itr();
}

/// <summary>
/// Inserts the specified string.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="data">The data.</param>
/// <returns></returns>
auto LongUI::POD::detail::hash_base::insert(
    const char* str, const char* data) noexcept ->std::pair<hash_iterator, bool> {
    assert(str && "bad string");
    return insert(str, str + std::strlen(str), data);
}

/// <summary>
/// Inserts the specified string begin.
/// </summary>
/// <param name="str_begin">The string begin.</param>
/// <param name="str_end">The string end.</param>
/// <param name="data">The data.</param>
/// <returns></returns>
auto LongUI::POD::detail::hash_base::insert(
    const char* str_begin,
    const char* str_end,
    const char* data) noexcept ->std::pair<hash_iterator, bool> {
    const auto itr = this->find(str_begin, str_end);
    // 不存在就插入
    if (!itr.cell) {
        const auto iterator = this->force_insert(str_begin, str_end, data);
        return{ iterator, !!iterator.bucket };
    }
    return{ itr , false};
}

/// <summary>
/// Inserts the specified string begin.
/// </summary>
/// <param name="str_begin">The string begin.</param>
/// <param name="str_end">The string end.</param>
/// <param name="data">The data.</param>
/// <returns></returns>
auto LongUI::POD::detail::hash_base::force_insert(
    const char* str_begin, 
    const char* str_end, 
    const char* data) noexcept -> hash_iterator {
    assert(str_begin && "bad string start");
    assert(str_end > str_begin && "bad string length");
    assert(data && "bad data");
    assert(find(str_begin, str_end) == end_itr() && "key exsit");
    // 计算长度
    const auto backup = m_pBaseTable;
    const auto celln = m_pBaseTableEnd - m_pBaseTable;
    // 扩容判断(目前因子为1.0, 或许0.75?)
    if (m_cItemSize + 1 > static_cast<uint32_t>(celln)) {
        this->growup(); 
        if (m_pBaseTable == backup) return { m_pBaseTableEnd, nullptr };
    }
    // 内存不足
    if (!this->is_ok())  return { m_pBaseTableEnd, nullptr };
    // 创建单元
    auto cell = this->alloc_cell(str_begin, str_end, data);
    // 插入单元
    uintptr_t* bucket = cell ? this->force_insert(*cell) : m_pBaseTableEnd;
    // 返回单元
    return{ bucket, cell };
}

/// <summary>
/// Inserts the specified cell.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
auto LongUI::POD::detail::hash_base::force_insert(hash_cell& cell) noexcept ->uintptr_t* {
    cell.next = nullptr;
    assert(m_itrFirst.bucket && "bad itr");
    const auto celln = m_pBaseTableEnd - m_pBaseTable;
    assert(LongUI::detail::is_power_of_2(celln) && "N must be power of 2");
    const auto hcode = LongUI::BKDRHash(cell.str);
    const auto index = hcode & (celln - 1);
    const auto pos = m_pBaseTable + index;
    auto list = reinterpret_cast<hash_cell**>(m_pBaseTable + index);
    while (*list) list = &(*list)->next;
    *list = &cell; ++m_cItemSize;
    const auto list_address = m_pBaseTable + index;
    if (m_itrFirst.bucket > list_address) {
        m_itrFirst.bucket = list_address;
        m_itrFirst.cell = &cell;
    }
    return pos;
}

/// <summary>
/// Growups this instance.
/// </summary>
/// <returns></returns>
void LongUI::POD::detail::hash_base::growup() noexcept {
    // 直接翻倍
#ifdef NDEBUG
    constexpr auto INIT_LEN = 32;
#else
    constexpr auto INIT_LEN = 1;
#endif
    const auto celln = m_pBaseTableEnd - m_pBaseTable;
    const auto nowlen = celln ? celln + celln : INIT_LEN;
    // 申请缓存, +1为END迭代器提供READ空间
    const auto ptr = hash_base::malloc(sizeof(uintptr_t) * (nowlen + 1));
    const auto table = reinterpret_cast<uintptr_t*>(ptr);
    // 内存不足
    if (!ptr) return; std::memset(ptr, 0, sizeof(uintptr_t) * (nowlen + 1));
    // 转移数据
    std::aligned_storage<sizeof(hash_base), alignof(hash_base)>::type another;
    auto& x = reinterpret_cast<hash_base&>(another);
    x.m_pBaseTable = table;
    x.m_pBaseTableEnd = table + nowlen;
    x.m_pBaseTableEnd[0] = end_symbol;
    x.m_itrFirst = x.end_itr();
    const_cast<uint32_t&>(x.m_cItemByteSize) = m_cItemByteSize;
    this->for_each([&x](hash_cell& cell) noexcept { x.force_insert(cell); });
    // 释放数据
    std::free(m_pBaseTable);
    // 复制数据
    m_pBaseTable = x.m_pBaseTable;
    m_pBaseTableEnd = x.m_pBaseTableEnd;
    m_itrFirst = x.m_itrFirst;
    // END迭代器
}

#if 0
/// <summary>
/// Removes the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
bool LongUI::POD::detail::hash_base::remove(const char* str) noexcept {
    return this->remove(this->find(str));
}

/// <summary>
/// Removes the specified .
/// </summary>
/// <param name="str_begin">The string begin.</param>
/// <param name="str_end">The string end.</param>
/// <returns></returns>
bool LongUI::POD::detail::hash_base::remove(const char* str_begin, const char* str_end) noexcept {
    return this->remove(this->find(str_begin, str_end));
}


/// <summary>
/// Removes the specified itr.
/// </summary>
/// <param name="itr">The itr.</param>
/// <returns></returns>
bool LongUI::POD::detail::hash_base::remove(hash_iterator itr) noexcept {
    // 有效迭代器
    if (itr.bucket != m_pBaseTableEnd) {
        assert(itr.cell && "YES?");
        assert(m_cItemSize && "NO?");
        // 链接前后节点
        auto cell = reinterpret_cast<hash_cell**>(itr.bucket);
        while (*cell != itr.cell) cell = &(*cell)->next;
        *cell = itr.cell->next;
        this->free(itr.cell);
        --m_cItemSize;
        return true;
    }
    return false;
}
#endif