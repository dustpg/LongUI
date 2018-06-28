#include <control/ui_control_state.h>
#include <style/ui_style.h>
#include <util/ui_unimacro.h>
#include <luiconf.h>


/// <summary>
/// Initializes a new instance of the <see cref="Box"/> struct.
/// </summary>
void LongUI::Box::Init() noexcept {
    pos = {};
    size = {};
    visible = {};
    border = {};
    margin = {};
    padding = {};
    minsize = {};
    maxsize = { DEFAULT_CONTROL_MAX_SIZE, DEFAULT_CONTROL_MAX_SIZE };
}

PCN_NOINLINE
/// <summary>
/// Gets the non contect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::Box::GetNonContect(RectF& rect) const noexcept {
    const auto pt = this->GetContentPos();
    rect.left = pt.x; rect.top = pt.y;
    rect.right = margin.right + border.right + padding.right;
    rect.bottom = margin.bottom + border.bottom + padding.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the margin rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::Box::GetMarginEdge(RectF& rect) const noexcept {
    rect.top = 0.0f;
    rect.left = 0.0f;
    rect.right = 0.0f + this->size.width;
    rect.bottom = 0.0f + this->size.height;
}


PCN_NOINLINE
/// <summary>
/// Gets the border rect.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::Box::GetBorderEdge(RectF& rect) const noexcept {
    this->GetMarginEdge(rect);
    rect.top += this->margin.top;
    rect.left += this->margin.left;
    rect.right -= this->margin.right;
    rect.bottom -= this->margin.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the padding rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::Box::GetPaddingEdge(RectF& rect) const noexcept {
    this->GetBorderEdge(rect);
    rect.top += this->border.top;
    rect.left += this->border.left;
    rect.right -= this->border.right;
    rect.bottom -= this->border.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the content rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::Box::GetContentEdge(RectF& rect) const noexcept {
    this->GetBorderEdge(rect);
    rect.top += this->padding.top;
    rect.left += this->padding.left;
    rect.right -= this->padding.right;
    rect.bottom -= this->padding.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the size of the content.
/// </summary>
/// <returns></returns>
auto LongUI::Box::GetContentSize() const noexcept -> Size2F {
    RectF rect; this->GetNonContect(rect);
    return{
        size.width - rect.left - rect.right,
        size.height - rect.top - rect.bottom
    };
}

PCN_NOINLINE
/// <summary>
/// Gets the size of the border.
/// </summary>
/// <returns></returns>
auto LongUI::Box::GetBorderSize() const noexcept -> Size2F {
    RectF rect; this->GetBorderEdge(rect);
    return{ rect.right - rect.left, rect.bottom - rect.top };
}

/// <summary>
/// Gets the content position.
/// </summary>
/// <returns></returns>
auto LongUI::Box::GetContentPos() const noexcept -> Point2F {
    Point2F pt;
    pt.x = margin.left + border.left + padding.left;
    pt.y = margin.top + border.top + padding.top;
    return pt; 
}


/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
LongUI::Style::Style() noexcept {
    constexpr auto a = sizeof(Style);
    state.Init();
    pack = Pack_Start;
    align = Align_Stretcht;
    appearance = AttributeAppearance::Appearance_NotSet;
    offset_tf = 0;
    overflow_x = Overflow_Visible;
    overflow_y = Overflow_Visible;
    unused = 0;
    flex = 0.0f;
    minsize = { INVALID_MINSIZE, INVALID_MINSIZE };
    maxsize = { DEFAULT_CONTROL_MAX_SIZE, DEFAULT_CONTROL_MAX_SIZE };
}

/// <summary>
/// Finalizes an instance of the <see cref="Style"/> class.
/// </summary>
/// <returns></returns>
LongUI::Style::~Style() noexcept {

}



// ----------------------------------------------------------------------------


/// <summary>
/// Aligns the specified view.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::AttrParser::Align(U8View view) noexcept -> AttributeAlign {
    assert(view.end() > view.begin());
    switch (*view.begin())
    {
    case 'c': return Align_Center;
    case 'e': return Align_End;
    case 'b': return Align_Baseline;
    default: return view.begin()[2] == 'r' ?
        Align_Stretcht : Align_Start;
    }
}

/// <summary>
/// Packs the specified .
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::AttrParser::Pack(U8View view) noexcept -> AttributePack {
    assert(view.end() > view.begin());
    switch (*view.begin())
    {
    default: return Pack_Start;
    case 'c': return Pack_Center;
    case 'e': return Pack_End;
    }
}

/// <summary>
/// Seltypes the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::AttrParser::Seltype(U8View view) noexcept -> AttributeSeltype {
    assert(view.end() > view.begin());
    switch (*view.begin())
    {
    default: return Seltype_Single;
    case 'm': return Seltype_Multiple;
    case 'c': return Seltype_Cell;
    case 't': return Seltype_Text;
    }
}

/// <summary>
/// Overflows the specified .
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::AttrParser::Overflow(U8View view) noexcept -> AttributeOverflow {
    assert(view.end() > view.begin());
    switch (*view.begin())
    {
    default: return Overflow_Visible;
    case 'a': return Overflow_Auto;
    case 'h': return Overflow_Hidden;
    case 's': return Overflow_Scroll;
    }
}

/// <summary>
/// Appearances the specified view.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::AttrParser::Appearance(U8View view) noexcept -> AttributeAppearance {
    assert(view.end() > view.begin());
    // TODO: NOTIMPL
    return Appearance_None;
}
