## LongUI XML Value Type List - Normal
value type|implement|note|e.g.
:--------:|---------|----|----
<a name="jump_int"/>int|`LongUI::AtoI`|integer |`= "0"`, `= "2333"`
<a name="jump_bool"/>bool|`pugi::xml_attribute::as_bool`| boolean |`= "true"`, `= "0"`
<a name="jump_float"/>float|`LongUI::AtoF`|float| `= "0"`, `= "1e3"`, `= "-1.0"`
<a name="jump_floatx2"/>floatx2|`LongUI::Helper::MakeFloats`|array for 2 floats| `="0.0, 2.33333"`
<a name="jump_floatx4"/>floatx4|`LongUI::Helper::MakeFloats`|array for 4 floats| `="0.0, 2.33333, -123, 1e-2"`
<a name="jump_floatx6"/>floatx6|`LongUI::Helper::MakeFloats`|array for 6 floats| ...
<a name="jump_floatxn"/>floatxn|locally|array for any floats| ...
<a name="jump_string"/>string|`LongUI::Helper::MakeString`|string, utf-8 only| `="_(:3」∠)_"`
<a name="jump_cc"/>CC|`std::strchr`,`LongUI::AtoI`|"CC" for "CreateControl", `name[, templateid]`| `="ScrollBarA"`, `="ScrollBarB, 2"`
<a name="jump_color"/>color|`LongUI::Helper::MakeColor`|color, `#RGB`, `#RRGGBB`, `#AARRGGBB`, `floatx4 in RGBA` | `="#FFF"`, `="#ABCDEF"`, `="#88ABCDEF"`, `="0.0, 1.0, 0.0, 1.0"`
  
## LongUI XML Value Type List - Enum
  
value type|implement|note|e.g.
:--------:|---------|----|----
<a name="jump_enum_brushtype"/>enum brushtype|`Helper::XMLGetValueEnum`|int or string| `="radial"`, `="1"`
---|---|`0` or `solid`| solid color brush for `ID2D1SolidColorBrush`
---|---|`1` or `linear`| linear gradient brush for `ID2D1LinearGradientBrush`
---|---|`2` or `radial`| radial gradient brush for `ID2D1RadialGradientBrush`
---|---|`3` or `bitmap`| bitmap for `ID2D1BitmapBrush1`
<a name="jump_enum_extend"/>enum extend|`Helper::XMLGetValueEnum`|int or string| `="mirror"`, `="1"`
---|---|`0` or `clamp`|for `D2D1_EXTEND_MODE_CLAMP`
---|---|`1` or `wrap`|for `D2D1_EXTEND_MODE_WRAP`
---|---|`2` or `mirror`|for `D2D1_EXTEND_MODE_MIRROR`
<a name="jump_enum_interpolation"/>enum interpolation|`Helper::XMLGetValueEnum`|int or string| `="highcubic"`, `="1"`
---|---|`0` or `neighbor`|for `D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR`
---|---|`1` or `linear`|for `D2D1_INTERPOLATION_MODE_LINEAR`
---|---|`2` or `cubic`|for `D2D1_INTERPOLATION_MODE_CUBIC`
---|---|`3` or `mslinear`|for `D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR`
---|---|`4` or `anisotropic`|for `D2D1_INTERPOLATION_MODE_ANISOTROPIC`
---|---|`5` or `highcubic`|for `D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC`
<a name="jump_enum_animationtype"/>enum animationtype|`Helper::XMLGetValueEnum`|int or string| `="linear"`, `="3"`
---|---|`0` or `linear`|for `AnimationType::Type_LinearInterpolation`

```cpp
    // 属性值列表
    static const char* type_list[] = {
        "linear",
        "quadraticim",
        "quadraticout",
        "quadraticinout",
        "cubicin",
        "cubicout",
        "cubicoinout",
        "quarticin",
        "quarticout",
        "quarticinout",
        "quinticcin",
        "quinticcout",
        "quinticinout",
        "sincin",
        "sincout",
        "sininout",
        "circularcin",
        "circularcout",
        "circularinout",
        "exponentiacin",
        "exponentiaout",
        "exponentiainout",
        "elasticin",
        "elasticout",
        "elasticinout",
        "backin",
        "backout",
        "backinout",
        "bouncein",
        "bounceout",
        "bounceinout",
    };
```