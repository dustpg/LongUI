#pragma once

#include <cstdint>
#include <core/ui_core_type.h>

// ui namespace
namespace LongUI{
    /// <summary>
    /// text-align
    /// </summary>
    enum AttributeTextAlign : uint8_t {
        TAlign_Start = 0,
        TAlign_End,
        TAlign_Center,
        TAlign_Justified,
        // ---------------
        TAlign_Left = TAlign_Start,
        TAlign_Right = TAlign_End,
    };

    /// <summary>
    /// font weight
    /// </summary>
    enum AttributeFontWeight : uint16_t {
        /// <summary>
        /// Predefined font weight : Thin (100).
        /// </summary>
        Weight_Thin = 100,

        /// <summary>
        /// Predefined font weight : Extra-light (200).
        /// </summary>
        Weight_ExtraLight = 200,

        /// <summary>
        /// Predefined font weight : Light (300).
        /// </summary>
        Weight_Light = 300,

        /// <summary>
        /// Predefined font weight : Normal (400).
        /// </summary>
        Weight_Normal = 400,

        /// <summary>
        /// Predefined font weight : Medium (500).
        /// </summary>
        Weight_Medium = 500,

        /// <summary>
        /// Predefined font weight : Semi-bold (600).
        /// </summary>
        Weight_SemiBold = 600,

        /// <summary>
        /// Predefined font weight : Bold (700).
        /// </summary>
        Weight_Bold = 700,

        /// <summary>
        /// Predefined font weight : Extra-bold (800).
        /// </summary>
        Weight_ExtraBold = 800,

        /// <summary>
        /// Predefined font weight : Heavy (900).
        /// </summary>
        Weight_Heavy = 900,
    };
    /// <summary>
    /// font style
    /// </summary>
    enum AttributeFontStyle : uint8_t {
        /// <summary>
        /// Font slope style : Normal.
        /// </summary>
        Style_Normal = 0,

        /// <summary>
        /// Font slope style : Oblique.
        /// </summary>
        Style_Oblique,

        /// <summary>
        /// Font slope style : Italic.
        /// </summary>
        Style_Italic
    };

    /// <summary>
    /// font stretch
    /// </summary>
    enum AttributeFontStretch : uint8_t {
        /// <summary>
        /// Predefined font stretch : Not known (0).
        /// </summary>
        Stretch_Undefined = 0,

        /// <summary>
        /// Predefined font stretch : Ultra-condensed (1).
        /// </summary>
        Stretch_UltraCondensed = 1,

        /// <summary>
        /// Predefined font stretch : Extra-condensed (2).
        /// </summary>
        Stretch_ExtraCondensed = 2,

        /// <summary>
        /// Predefined font stretch : Condensed (3).
        /// </summary>
        Stretch_Condensed = 3,

        /// <summary>
        /// Predefined font stretch : Semi-condensed (4).
        /// </summary>
        Stretch_SemiCondensed = 4,

        /// <summary>
        /// Predefined font stretch : Normal (5).
        /// </summary>
        Stretch_Normal = 5,

        /// <summary>
        /// Predefined font stretch : Semi-expanded (6).
        /// </summary>
        Stretch_SemiExpanded = 6,

        /// <summary>
        /// Predefined font stretch : Expanded (7).
        /// </summary>
        Stretch_Expanded = 7,

        /// <summary>
        /// Predefined font stretch : Extra-expanded (8).
        /// </summary>
        Stretch_ExtraExpanded = 8,

        /// <summary>
        /// Predefined font stretch : Ultra-expanded (9).
        /// </summary>
        Stretch_UltraExpanded = 9
    };

    // parse the attribute
    struct TFAttrParser {
        // font-style
        static auto Style(U8View) noexcept->AttributeFontStyle;
        // font-weight
        static auto Weight(U8View) noexcept->AttributeFontWeight;
        // font stretch
        static auto Stretch(U8View) noexcept->AttributeFontStretch;
        // Align
        static auto TextAlign(U8View) noexcept->AttributeTextAlign;
    }; 
}