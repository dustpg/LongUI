#include <control/ui_viewport.h>
#include <control/ui_tree.h>
#include <core/ui_unsafe.h>
#include <constexpr/const_bkdr.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <algorithm>

struct ViewTree : LongUI::UIViewport {
    void do_infos(
        const LongUI::MetaControlCP* a, 
        const LongUI::MetaControlCP* b,
        LongUI::MetaControlCP super) noexcept {
        assert(super);
        std::for_each(a, b, [=](LongUI::MetaControlCP cp) noexcept {
            if (cp->super_class != super) return;
            do_infos(a, b, cp);
        });
    }
    ViewTree(LongUI::CUIWindow* parent,
        LongUI::CUIWindow::WindowConfig config
    ) noexcept : tree(this) {
        using namespace LongUI;
        Unsafe::AddAttrUninited(tree, "flex"_bkdr, "1"_sv);
        const auto& infos = UIManager.RefControlInfos();
        do_infos(infos.info_list, infos.end_of_list, &UIControl::s_meta);
    }
    ~ViewTree() noexcept override { m_state.destructing = true; }
    LongUI::UITree      tree;
};

auto CreateViewport_Controls(
    LongUI::CUIWindow* parent, 
    LongUI::CUIWindow::WindowConfig config) noexcept -> LongUI::UIViewport * {
    LongUI::UIControl::ControlMakingBegin();
    const auto rv = new(std::nothrow) ViewTree{ parent, config };
    LongUI::UIControl::ControlMakingEnd();
    return rv;
}
