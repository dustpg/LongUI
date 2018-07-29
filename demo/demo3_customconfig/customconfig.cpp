#include "customconfig.h"
#include <text/ui_ctl_arg.h>
#include <core/ui_string_view.h>
#include <core/ui_string.h>
//#include <interface/ui_ctrlinfolist.h>

#include <cstring>
#include <cassert>
#include <algorithm>

// ZIP LIBRARY
#include "zip.h"

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
        // this demo use lib: https://github.com/kuba--/
        // this library use fopen so that cannot support utf-8 on window
        auto name = LongUI::SplitStr(url_in_utf8, u8".zip/"_sv);
        if (url_in_utf8.begin() != url_in_utf8.end()) {
            // move *.zip/ -> *.zip
            name.second--;
            // view to c-style string
            const CUIStringU8 zipname = name;
            // file open
            if (const auto zip = ::zip_open(zipname.c_str(), 0, 'r')) {
                const CUIStringU8 entry = url_in_utf8;
                if (!::zip_entry_open(zip, entry.c_str())) {
                    // 4GB is large enough here
                    const auto size = static_cast<uint32_t>(::zip_entry_size(zip));
                    // +1 for nul-terminated string to aviod re-alloc the memory
                    buffer.reserve(size + 1);
                    buffer.resize(size);
                    if (buffer.is_ok()) if (::zip_entry_noallocread(zip, &buffer.front(), size))
                        buffer.clear();
                    ::zip_entry_close(zip);
                }
                else assert(!"NOT IMPL");
                ::zip_close(zip);
            }
        }
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