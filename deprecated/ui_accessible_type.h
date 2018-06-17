#pragma once

#include <cstdint>

// ui namespace
namespace UI{
    /// <summary>
    /// type for accessible
    /// </summary>
    enum class AccessibleType : uint16_t {
        Type_XULAlert            = 0x1001,
        Type_XULButton           = 0x1002,
        Type_XULCheckbox         = 0x1003,
        Type_XULColorPicker      = 0x1004,
        Type_XULColorPickerTile  = 0x1005,
        Type_XULCombobox         = 0x1006,
        Type_XULDropmarker       = 0x1007,
        Type_XULGroupbox         = 0x1008,
        Type_XULImage            = 0x1009,
        Type_XULLink             = 0x100A,
        Type_XULListbox          = 0x100B,
        Type_XULListCell         = 0x1026,
        Type_XULListHead         = 0x1024,
        Type_XULListHeader       = 0x1025,
        Type_XULListitem         = 0x100C,
        Type_XULMenubar          = 0x100D,
        Type_XULMenuitem         = 0x100E,
        Type_XULMenupopup        = 0x100F,
        Type_XULMenuSeparator    = 0x1010,
        Type_XULPane             = 0x1011,
        Type_XULProgressMeter    = 0x1012,
        Type_XULScale            = 0x1013,
        Type_XULStatusBar        = 0x1014,
        Type_XULRadioButton      = 0x1015,
        Type_XULRadioGroup       = 0x1016,
        Type_XULTab              = 0x1017,
        Type_XULTabBox           = 0x1018,
        Type_XULTabs             = 0x1019,
        Type_XULText             = 0x101A,
        Type_XULTextBox          = 0x101B,
        Type_XULThumb            = 0x101C,
        Type_XULTree             = 0x101D,
        Type_XULTreeColumns      = 0x101E,
        Type_XULTreeColumnItem   = 0x101F,
        Type_XULToolbar          = 0x1020,
        Type_XULToolbarSeparator = 0x1021,
        Type_XULTooltip          = 0x1022,
        Type_XULToolbarButton    = 0x1023,
    };
}