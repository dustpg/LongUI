#pragma once
#include "../core/ui_color.h"

// ui
namespace LongUI {
    // colorsystem
    namespace ColorSystem {
        // CSs
        struct RGBA; struct HSVA; struct HSLA;
        // RGBA
        struct RGBA { 
            // rgba data
            float r, g, b, a; 
            // to colorf
            operator ColorF&() noexcept { 
                return reinterpret_cast<ColorF&>(*this); 
            }
            // to const colorf
            operator const ColorF&() const noexcept { 
                return reinterpret_cast<const ColorF&>(*this); 
            }
            // to HSLA
            //HSLA toHSLA() const noexcept;
            // to HSVA
            //HSVA toHSVA() const noexcept;
        };
        // HSVA(0~360, 0~1, 0~1, 0~1)
        //struct HSVA { 
        //    // hsva data
        //    float h, s, v, a;
        //    // to HSLA
        //    HSLA toHSLA() const noexcept;
        //    // to RGBA
        //    RGBA toRGBA() const noexcept;
        //};
        // HSLA(0~360, 0~1, 0~1, 0~1)
        struct HSLA { 
            // hsla data
            float h, s, l, a;
            // to RGBA
            RGBA toRGBA() const noexcept;
            // to HSVA
            //HSVA toHSVA() const noexcept;
        };
    }
    // CS
    namespace CS = ColorSystem;
}