#include <style/ui_ssvalue.h>
#include <core/ui_color_list.h>
#include <core/ui_manager.h>

struct A {
    bool f1 : 1;
};

/// <summary>
/// Parses the stylesheet.
/// </summary>
/// <param name="">The .</param>
/// <param name="vs">The vs.</param>
/// <param name="ss">The ss.</param>
/// <returns></returns>
bool LongUI::ParseStylesheet(
    const char* str, 
    SSValues& vs, 
    SSSelectors& ss
) noexcept {
    SSSelector sss;
    str;
    /* 测试样式表
    button {
        background-color: green;
    }
    */
    vs.push_back({
        ValueType::Type_BackgroundColor,
        RGBA_Green,
    });
    // button [0, 1)
    sss = {
        nullptr,
        UIManager.GetUniqueText("button"_sv),
        nullptr,
        0, 1-0
    };
    ss.push_back(sss);
    return true;
}
