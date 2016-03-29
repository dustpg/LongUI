#include <Core/luiMenu.h>
#include <Core/luiWindow.h>
#include <windows.h>
// TODO: Add LongUI Popup Menu Support

// longui 命名空间
namespace LongUI {
    // windows helper
    inline auto windows(HSystemMenu menu) { return reinterpret_cast<HMENU>(menu); }
    // longui helper
    inline auto longui(HMENU menu) { return reinterpret_cast<HSystemMenu>(menu); }
}

// 摧毁弹出菜单
void LongUI::CUIMenu::Destroy() noexcept {
    if (m_hMenu) {
        ::DestroyMenu(windows(m_hMenu));
        m_hMenu = nullptr;
    }
}

// 直接创建才菜单
bool LongUI::CUIMenu::Create() noexcept {
    assert(!m_hMenu && "cannot create again! should destroy this");
    m_hMenu = longui(::CreatePopupMenu());
    return !!m_hMenu;
}

// 使用XML字符串创建菜单
bool LongUI::CUIMenu::Create(const char* xml) noexcept {
    pugi::xml_document document;
    auto re = document.load_string(xml);
    // 错误
    if (re.status) {
        assert(!"failed to load string");
        ::MessageBoxA(
            nullptr,
            re.description(),
            "<LongUI::CUIMenu::Create>: Failed to Parse/Load XML",
            MB_ICONERROR
        );
        return false;
    }
    // 创建结点
    return this->Create(document.first_child());
}

// 使用XML结点创建菜单
bool LongUI::CUIMenu::Create(pugi::xml_node node) noexcept {
    UNREFERENCED_PARAMETER(node);
    assert(!m_hMenu && "cannot create again!");
    this->Create();
    return !!m_hMenu;
}

// 添加物品
bool LongUI::CUIMenu::AppendItem(const CUIMenu::Item& prop) noexcept {
    assert(m_hMenu && "create this at first!");
    // 分割线
    if (prop.style & Style_Separator) {
        return !!::AppendMenuW(windows(m_hMenu), MF_SEPARATOR, 0, nullptr);
    }
    UINT flag = 0;
    // 选中
    if (prop.style & Style_Checked) {
        flag |= MF_CHECKED;
    }
    // 无效化
    if (prop.style & Style_Disabled) {
        flag |= MF_DISABLED;
    }
    // 有meta图标
    if (prop.style & Style_Meta) {

    }
    // 子菜单
    if (prop.style & Style_Menu) {
        flag |= MF_POPUP;
    }
    return !!::AppendMenuW(windows(m_hMenu), MF_POPUP, prop.index, prop.text);
}

// 显示菜单
void LongUI::CUIMenu::Show(XUIBaseWindow* window, POINT*  pos) noexcept {
    assert(window && "bad argment");
    // 窗口
    HWND hp = window->GetHwnd();
    // 获取坐标
    POINT pt = { 0,0 };  if (pos) pt = *pos; else ::GetCursorPos(&pt);
    // 置前
    ::SetForegroundWindow(hp);
    // 跟踪菜单项的选择
    auto index = ::TrackPopupMenu(
        windows(m_hMenu), 
        TPM_RIGHTBUTTON | TPM_RETURNCMD, 
        pt.x, pt.y, 
        0, hp,
        nullptr
    );
    // 调用
    if (m_uiCall.IsOK()) {
        m_uiCall(size_t(index));
    }
}