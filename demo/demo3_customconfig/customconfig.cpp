#include "customconfig.h"
#include <text/ui_ctl_arg.h>
#include <core/ui_string_view.h>
//#include <interface/ui_ctrlinfolist.h>

#include <cstring>
#include <cassert>
#include <algorithm>

namespace Demo {
    /// <summary>
    /// Gets the configure flag.
    /// </summary>
    /// <returns></returns>
    auto CustomConfig::GetConfigureFlag() noexcept -> ConfigureFlag {
        return Flag_None
            //| Flag_RenderByCPU
            | Flag_OutputDebugString
            | Flag_QuitOnLastWindowClosed
            //| Flag_NoAutoScaleOnHighDpi
            //| Flag_DbgOutputTimeTook 
            //| Flag_DbgDrawDirtyRect
            | Flag_DbgDrawTextCell
            | Flag_DbgDebugWindow
            ;
    }
    /// <summary>
    /// Gets the name of the locale.
    /// </summary>
    /// <param name="name">The name.</param>
    /// <returns></returns>
    void CustomConfig::GetLocaleName(char16_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept {
        // this interface for font-famil name
        // e.g. KaiTi for 'en-US', ¿¬Ìå for 'zh-CN'


        // empty for local name
        name[0] = 0;

        // now set to "en-US"
        const char16_t des[] = u"en-US";
        std::memcpy(name, des, sizeof(des));
    }
    /// <summary>
    /// Defaults the font argument.
    /// </summary>
    /// <param name="arg">The argument.</param>
    /// <returns></returns>
    void CustomConfig::DefaultFontArg(FontArg& arg) noexcept {
        arg.family = u8"KaiTi";
        arg.size = 20.f;
    }
    /// <summary>
    /// Registers the control.
    /// </summary>
    /// <param name="list">The list.</param>
    /// <returns></returns>
    void CustomConfig::RegisterControl(ControlInfoList& list) noexcept {

    }
    /// <summary>
    /// Chooses the adapter.
    /// </summary>
    /// <param name="adapters">The adapters.</param>
    /// <param name="length">The length.</param>
    /// <returns></returns>
    auto CustomConfig::ChooseAdapter(
        const GraphicsAdapterDesc adapters[],
        const uint32_t length) noexcept->uint32_t {
        m_listAdapters.clear();
        m_listAdapters.insert(
            m_listAdapters.begin(),
            adapters,
            adapters + length
        );
        // OUT OF RANGE -> first call -> Windows Advanced Rasterization Platform (WARP)
        if (m_iAdapterId >= length) {
            static const char16_t s_warp[] = u"Microsoft Basic Render Driver";
            const auto itr = std::find_if(adapters, adapters + length, [](const auto& x) noexcept {
                return !std::memcmp(x.friend_name, s_warp, sizeof(s_warp));
            });
            // choose default one if return-value out-of-range(s_warp not found)
            return itr - adapters;
        }
        return m_iAdapterId;
    }
    /// <summary>
    /// Loads the data from URL.
    /// </summary>
    /// <param name="url_in_utf8">The URL in UTF8.</param>
    /// <param name="url_in_utf16">The URL in UTF16.</param>
    /// <param name="buffer">The buffer.</param>
    /// <returns></returns>
    void CustomConfig::LoadDataFromUrl(
        U8View url_in_utf8,
        const CUIString& url_in_utf16,
        POD::Vector<uint8_t>& buffer) noexcept {

    }
    /// <summary>
    /// Called when [error].
    /// </summary>
    /// <param name="info">The information.</param>
    /// <returns></returns>
    void CustomConfig::OnError(ErrorInfo info) noexcept {
        assert(!"NOT IMPL");
    }

}