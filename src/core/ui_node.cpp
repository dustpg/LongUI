#include <core/ui_node.h>
#include <core/ui_object.h>
#include <core/ui_malloc.h>
#include <cassert>


/// <summary>
/// Swaps the ab.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void LongUI::Node::SwapAB(Node& a, Node& b) noexcept {
    // 必须是1-A-B-2
    assert(a.next == &b && "must be 'A-B' near node");
    assert(b.prev == &a && "bad linked");
    // --> 1 - B - A - 2
    const auto node1 = a.prev;
    const auto node2 = b.next;
    // NEXT
    node1->next = &b;
    node2->prev = &a;
    b.next = &a;
    a.next = node2;
    a.prev = &b;
    b.prev = node1;
}

/// <summary>
/// Swaps the node.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void LongUI::Node::SwapNode(Node& a, Node& b) noexcept {
    assert(a != b && "a == b");
    // A = B
    if (a == b) return;
    // 1 - A - B - 2
    if (a.next == &b) return Node::SwapAB(a, b);
    // 1 - B - A - 2
    if (a.prev == &b) return Node::SwapAB(b, a);
    // 1 - A - 2        3 - B - 4
    const auto node1 = a.prev;
    const auto node2 = a.next;
    const auto node3 = b.prev;
    const auto node4 = b.next;
    // --> 1 - B - 2    3 - A - 4
    // NEXT
    node1->next = &b;
    b.next = node2;
    node3->next = &a;
    a.next = node4;
    // PREV
    node4->prev = &a;
    a.prev = node3;
    node2->prev = &b;
    b.prev = node1;
}


/// <summary>
/// Implements the operator delete.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns>
/// The result of the operator.
/// </returns>
void LongUI::CUIObject::operator delete(void* ptr) noexcept {
    LongUI::NormalFree(ptr);
}

/// <summary>
/// Implements the operator new.
/// </summary>
/// <param name="size">The size.</param>
/// <param name="">The .</param>
/// <returns>
/// The result of the operator.
/// </returns>
void*LongUI::CUIObject::operator new(size_t size, const std::nothrow_t&) noexcept {
    return LongUI::NormalAlloc(size);
}

/// <summary>
/// Implements the operator delete.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="">The .</param>
/// <returns>
/// The result of the operator.
/// </returns>
void LongUI::CUIObject::operator delete(void* ptr, const std::nothrow_t&) noexcept {
    LongUI::NormalFree(ptr);
}




/// <summary>
/// Implements the operator delete.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns>
/// The result of the operator.
/// </returns>
void LongUI::CUISmallObject::operator delete(void* ptr) noexcept {
    LongUI::SmallFree(ptr);
}

/// <summary>
/// Implements the operator new.
/// </summary>
/// <param name="size">The size.</param>
/// <param name="">The .</param>
/// <returns>
/// The result of the operator.
/// </returns>
void*LongUI::CUISmallObject::operator new(size_t size, const std::nothrow_t&) noexcept {
    return LongUI::SmallAlloc(size);
}

/// <summary>
/// Implements the operator delete.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="">The .</param>
/// <returns>
/// The result of the operator.
/// </returns>
void LongUI::CUISmallObject::operator delete(void* ptr, const std::nothrow_t&) noexcept {
    LongUI::SmallFree(ptr);
}