#include <control/ui_viewport.h>
#include <control/ui_tree.h>
#include <control/ui_treecell.h>
#include <control/ui_treecol.h>
#include <control/ui_treecols.h>
#include <core/ui_unsafe.h>
#include <constexpr/const_bkdr.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <algorithm>

using namespace LongUI;

struct ViewTree : UIViewport {
    void do_infos(
        const MetaControlCP* a, 
        const MetaControlCP* b,
        UITreeItem* parent,
        MetaControlCP super) noexcept {
        assert(super);
        std::for_each(a, b, [=](MetaControlCP cp) noexcept {
            if (cp->super_class != super) return;
            const auto get_hash = [cp]() noexcept {
                CUIString string; 
                string.format(u"0x%08x", cp->bkdr_hash);
                return string;
            };
            // TODO: OOM handle
            const auto item = parent->AddItem();
            item->AddCell()->SetText(CUIString::FromUtf8(cp->element_name));
            item->AddCell()->SetText(get_hash());
            do_infos(a, b, item, cp);
        });
    }
    ViewTree(CUIWindow* parent,
        CUIWindow::WindowConfig config
    ) noexcept : tree(this), cols(&tree), 
        col_name(&cols), /*col_addr(&cols),*/ col_hash(&cols) {
        constexpr auto flex = "flex"_bkdr;
        constexpr auto label = "label"_bkdr;
        Unsafe::AddAttrUninited(tree, flex, "1"_sv);
        Unsafe::AddAttrUninited(col_name, flex, "1"_sv);
        //Unsafe::AddAttrUninited(col_addr, flex, "1"_sv);
        Unsafe::AddAttrUninited(col_hash, flex, "1"_sv);
        Unsafe::AddAttrUninited(col_name, label, "name"_sv);
        //Unsafe::AddAttrUninited(col_addr, label, "function adress"_sv);
        Unsafe::AddAttrUninited(col_hash, label, "hash"_sv);
        const auto& infos = UIManager.RefControlInfos();
        const auto ctrl = tree.AddItem();
        ctrl->AddCell()->SetText(u"control"_sv);
        do_infos(infos.info_list, infos.end_of_list, ctrl, &UIControl::s_meta);
    }
    ~ViewTree() noexcept override { m_state.destructing = true; }
    UITree          tree;
    //{
    UITreeCols      cols;
    UITreeCol       col_name;
    //UITreeCol       col_addr;
    UITreeCol       col_hash;
    //}
};

auto CreateViewport_Controls(
    LongUI::CUIWindow* parent, 
    LongUI::CUIWindow::WindowConfig config) noexcept -> LongUI::UIViewport * {
    LongUI::UIControl::ControlMakingBegin();
    const auto rv = new(std::nothrow) ViewTree{ parent, config };
    LongUI::UIControl::ControlMakingEnd();
    return rv;
}
