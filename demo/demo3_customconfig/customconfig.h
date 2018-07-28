#pragma once

#include <luiconf.h>
#include <interface/ui_default_config.h>
#include <graphics/ui_adapter_desc.h>
#include <container/pod_vector.h>

namespace Demo {
    // using longui
    using namespace LongUI;
    /// <summary>
    /// custom config
    /// </summary>
    /// <seealso cref="LongUI::CUIDefaultConfigure" />
    class CustomConfig : public CUIDefaultConfigure {
    public:
        // get flags for configure
        auto GetConfigureFlag() noexcept->ConfigureFlag override;
        // get locale name of ui(for text)
        void GetLocaleName(char16_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept override;
        // get default font arg
        void DefaultFontArg(FontArg& arg) noexcept override;
        // add all controls
        void RegisterControl(ControlInfoList& list) noexcept override;
        // if use gpu render, you should choose a video card, return the index
        auto ChooseAdapter(const GraphicsAdapterDesc adapters[], const uint32_t length) noexcept->uint32_t override;
        // load data from url on file not found
        void LoadDataFromUrl(U8View url_in_utf8, const CUIString& url_in_utf16, POD::Vector<uint8_t>& buffer) noexcept override;
        // show the error string
        void OnError(ErrorInfo info) noexcept override;
    public:
        // set adapter index
        void SetAdapterIndex(uint32_t i) noexcept { m_iAdapterId = i; }
        // ref the adapter list
        auto&RefAdapters() const noexcept { return m_listAdapters; }
    private:
        // adapter index
        uint32_t                        m_iAdapterId = MAX_GRAPHICS_ADAPTERS;
        // adapter list
        POD::Vector<GraphicsAdapterDesc>m_listAdapters;
    };

}