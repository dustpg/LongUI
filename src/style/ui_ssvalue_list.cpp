// lui
#include <style/ui_ssvalue_list.h>
#include <typecheck/int_by_size.h>

// longui namespace
namespace LongUI {
    // BKDR Hash Function
    auto BKDRHash(const char* a, const char* b) noexcept->uint32_t;
}
/// <summary>
/// U8s the type of the view2 value.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
auto LongUI::U8View2ValueType(const char* a, const char* b) noexcept -> ExValueType {
    const auto bkdr = LongUI::BKDRHash(a, b);
    switch (bkdr)
    {
        // ------------- Position ----------------

    case 0x29867c0e_ui32:
        // overflow
        return { ValueType::Type_PositionOverflowX , 2 };
    case 0xa8321dfd_ui32:
        // overflow-x
        return { ValueType::Type_PositionOverflowX , 1 };
    case 0xa8321dfe_ui32:
        // overflow-y
        return { ValueType::Type_PositionOverflowY , 1 };


        // ------------- Margin ----------------

    case 0xcd67f276_ui32:
        // margin
        return { ValueType::Type_MarginTop , 4 };
    case 0x4b67d8e6_ui32:
        // margin-top
        return { ValueType::Type_MarginTop , 1 };
    case 0xb07f6e2d_ui32:
        // margin-right
        return { ValueType::Type_MarginRight , 1 };
    case 0x040402e6_ui32:
        // margin-bottom
        return { ValueType::Type_MarginBottom , 1 };
    case 0x950eedd6_ui32:
        // margin-left
        return { ValueType::Type_MarginLeft , 1 };

        // ------------- Padding ----------------

    case 0x44220515_ui32:
        // padding
        return { ValueType::Type_PaddingTop , 4 };
    case 0xf617c735_ui32:
        // padding-top
        return { ValueType::Type_PaddingTop , 1 };
    case 0xbc0d7bf4_ui32:
        // padding-right
        return { ValueType::Type_PaddingRight , 1 };
    case 0xedb50fbb_ui32:
        // padding-bottom
        return { ValueType::Type_PaddingBottom , 1 };
    case 0xed15e043_ui32:
        // padding-left
        return { ValueType::Type_PaddingLeft , 1 };


        // ------------- Border ----------------

    case 0x7f449c09_ui32:
        // border-width
        return { ValueType::Type_BorderTopWidth , 4 };
    case 0xe2efc419_ui32:
        // border-top-width
        return { ValueType::Type_BorderTopWidth , 1 };
    case 0xd3d82b98_ui32:
        // border-right-width
        return { ValueType::Type_BorderRightWidth , 1 };
    case 0xec99a7dd_ui32:
        // border-bottom-width
        return { ValueType::Type_BorderBottomWidth , 1 };
    case 0x3d08d15d_ui32:
        // border-left-width
        return { ValueType::Type_BorderLeftWidth , 1 };



        // ------------- Background ----------------

    case 0x985a232c_ui32:
        // background-color
        return { ValueType::Type_BackgroundColor , 1 };


        // ------------- LongUI ----------------

#if 0
    case 0xee354dac_ui32:
        // appearance
#endif
    case 0xd6e6b71a_ui32:
        // -moz-appearance
        return { ValueType::Type_UIAppearance  };
    default:
        break;
    }
    return { ValueType::Type_Unknown };
}