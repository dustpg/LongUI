## LongUI Xml值 类型列表 - 普通  
  
类型|实现|注|例子
:---:|----|---|----
<a name="jump_int"/>int|`LongUI::AtoI`|整型|`= "0"`, `= "2333"`
<a name="jump_bool"/>bool|`pugi::xml_attribute::as_bool`| 布尔值 |`= "true"`, `= "0"`
<a name="jump_float"/>float|`LongUI::AtoF`|浮点| `= "0"`, `= "1e3"`, `= "-1.0"`
<a name="jump_floatx2"/>floatx2|`LongUI::Helper::MakeFloats`|2浮点数组| `="0.0, 2.33333"`
<a name="jump_floatx4"/>floatx4|`LongUI::Helper::MakeFloats`|4浮点数组| `="0.0, 2.33333, -123, 1e-2"`
<a name="jump_floatx6"/>floatx6|`LongUI::Helper::MakeFloats`|6浮点数组| ...
<a name="jump_stops"/>stops|locally, `LongUI::Helper::MakeColor`|array for any stops| `[pos0, color0] [pos1, color1]`
<a name="jump_string"/>string|`LongUI::Helper::MakeString`(如果转换成宽字符串)|utf-8 数组| `="_(:3」∠)_"`
<a name="jump_cc"/>CC x1|`LongUI::Helper::MakeCC`|`name[, templateid]`| `="ScrollBarA"`, `="ScrollBarB, 2"`
<a name="jump_cc"/>CC xN|`LongUI::Helper::MakeCC`|`name[, templateid] ...`| `="Text, Text, 2, Text, 2"`,
<a name="jump_color"/>color|`LongUI::Helper::MakeColor`|颜色, `#RGB`, `#RRGGBB`, `#AARRGGBB`, `floatx4 in RGBA` | `="#FFF"`, `="#ABCDEF"`, `="#88ABCDEF"`, `="0.0, 1.0, 0.0, 1.0"`
  
  
## LongUI XML Value Type List - Text Renderer
  
value type|implement|note|e.g.|context
:--------:|---------|----|----|-------
<a name="jump_textrender"/>textrender|`CUIManager::GetTextRenderer`|int or string| `="normal"`, `="1"`|---
---|---|`0` or `normal`| normal text renderer for `TextRendererType::Type_NormalTextRenderer`| **NONE**
  
## LongUI Xml值 类型列表 - 枚举  
  
类型|实现|注|例子
:---:|----|---|----
<a name="jump_enum_bmprule"/>bmprule|`Helper::XMLGetBitmapRenderRule`|int or string| `="button"`, `="1"`
---|---|`0` or `scale`| normal rendering rule for `BitmapRenderRule::Rule_Scale`
---|---|`1` or `button`| button like rendering rule for `BitmapRenderRule::Rule_ButtonLike`
<a name="jump_enum_fontstyle"/>fontstyle|`Helper::XMLGetFontStyle`|int or string| `="clamp"`, `="1"`
---|---|`0` or `normal`|for `DWRITE_FONT_STYLE_NORMAL`
---|---|`1` or `oblique`|for `DWRITE_FONT_STYLE_OBLIQUE`
---|---|`2` or `italic`|for `DWRITE_FONT_STYLE_ITALIC`
<a name="jump_enum_extend"/>extend|`Helper::XMLGetD2DExtendMode`|int or string| `="mirror"`, `="1"`
---|---|`0` or `clamp`|for `D2D1_EXTEND_MODE_CLAMP`
---|---|`1` or `wrap`|for `D2D1_EXTEND_MODE_WRAP`
---|---|`2` or `mirror`|for `D2D1_EXTEND_MODE_MIRROR`
<a name="jump_enum_valign"/>valign|`Helper::XMLGetVAlignment`|int or string| `="top"`, `="1"`
---|---|`0` or `top`|for `DWRITE_PARAGRAPH_ALIGNMENT_NEAR`
---|---|`1` or `bottom`|for `DWRITE_PARAGRAPH_ALIGNMENT_FAR`
---|---|`2` or `middle`|for `DWRITE_PARAGRAPH_ALIGNMENT_CENTER`
<a name="jump_enum_halign"/>halign|`Helper::XMLGetHAlignment`|int or string| `="right"`, `="1"`
---|---|`0` or `left`|for `DWRITE_TEXT_ALIGNMENT_LEADING`
---|---|`1` or `right`|for `DWRITE_TEXT_ALIGNMENT_TRAILING`
---|---|`2` or `center`|for `DWRITE_TEXT_ALIGNMENT_CENTER`
---|---|`3` or `justify`|for `DWRITE_TEXT_ALIGNMENT_JUSTIFIED`
<a name="jump_enum_textantialias"/>textantialias|`Helper::XMLGetD2DTextAntialiasMode`|int or string| `="default"`, `="1"`
---|---|`0` or `default`|for `D2D1_TEXT_ANTIALIAS_MODE_DEFAULT`
---|---|`1` or `cleartype`|for `D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE`
---|---|`2` or `grayscale`|for `D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE`
---|---|`3` or `aliased`|for `D2D1_TEXT_ANTIALIAS_MODE_ALIASED`
<a name="jump_enum_richtype"/>richtype|`Helper::XMLGetRichType`|int or string| `="core"`, `="1"`
---|---|`0` or `none`| none rich type rule for `RichType::Type_None`
---|---|`1` or `core`| (LCML)[./longui-core-mark-language.md] for `RichType::Type_Core`
---|---|`2` or `xml`| (html like)[./longui-html-like.md] for `RichType::Type_Xml`
---|---|`3` or `custom`| user custom defined for `RichType::Type_Custom`
<a name="jump_enum_brushtype"/>brushtype|`Helper::XMLGetValueEnum`|int or string| `="radial"`, `="1"`
---|---|`0` or `solid`| solid color brush for `ID2D1SolidColorBrush`
---|---|`1` or `linear`| linear gradient brush for `ID2D1LinearGradientBrush`
---|---|`2` or `radial`| radial gradient brush for `ID2D1RadialGradientBrush`
---|---|`3` or `bitmap`| bitmap for `ID2D1BitmapBrush1`
<a name="jump_enum_flowdirection"/>flowdirection|`Helper::XMLGetFlowDirection`|int or string| `="right2left"`, `="1"`
---|---|`0` or `top2bottom`| for `DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM`
---|---|`1` or `bottom2top`| for `DWRITE_FLOW_DIRECTION_BOTTOM_TO_TOP`
---|---|`2` or `left2right`| for `DWRITE_FLOW_DIRECTION_LEFT_TO_RIGHT`
---|---|`3` or `right2left`| for `DWRITE_FLOW_DIRECTION_RIGHT_TO_LEFT`
<a name="jump_enum_readingdirection"/>readingdirection|`Helper::XMLGetReadingDirection`|int or string| `="right2left"`, `="1"`
---|---|`0` or `left2right`| for `DWRITE_READING_DIRECTION_LEFT_TO_RIGHT`
---|---|`1` or `right2left`| for `DWRITE_READING_DIRECTION_RIGHT_TO_LEFT`
---|---|`2` or `top2bottom`| for `DWRITE_READING_DIRECTION_TOP_TO_BOTTOM`
---|---|`3` or `bottom2top`| for `DWRITE_READING_DIRECTION_BOTTOM_TO_TOP`
<a name="jump_enum_wordwrapping"/>wordwrapping|`Helper::XMLGetWordWrapping`|int or string| `="break"`, `="1"`
---|---|`0` or `wrap`| for `DWRITE_WORD_WRAPPING_WRAP`
---|---|`1` or `nowrap`| for `DWRITE_WORD_WRAPPING_NO_WRAP`
---|---|`2` or `break`| for `DWRITE_WORD_WRAPPING_EMERGENCY_BREAK`
---|---|`3` or `word`| for `DWRITE_WORD_WRAPPING_WHOLE_WORD`
---|---|`4` or `character`| for `DWRITE_WORD_WRAPPING_CHARACTER`
<a name="jump_enum_interpolation"/>interpolation|`Helper::XMLGetD2DInterpolationMode`|int or string| `="highcubic"`, `="1"`
---|---|`0` or `neighbor`|for `D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR`
---|---|`1` or `linear`|for `D2D1_INTERPOLATION_MODE_LINEAR`
---|---|`2` or `cubic`|for `D2D1_INTERPOLATION_MODE_CUBIC`
---|---|`3` or `mslinear`|for `D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR`
---|---|`4` or `anisotropic`|for `D2D1_INTERPOLATION_MODE_ANISOTROPIC`
---|---|`5` or `highcubic`|for `D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC`
<a name="jump_enum_fontstretch"/>fontstretch|`Helper::XMLGetFontStretch`|int or string| `="normal"`, `="1"`
---|---|`0` or `undefined`|for `DWRITE_FONT_STRETCH_UNDEFINED`
---|---|`1` or `ultracondensed`|for `DWRITE_FONT_STRETCH_ULTRA_CONDENSED`
---|---|`2` or `extracondensed`|for `DWRITE_FONT_STRETCH_EXTRA_CONDENSED`
---|---|`3` or `condensed`|for `DWRITE_FONT_STRETCH_CONDENSED`
---|---|`4` or `semicondensed`|for `DWRITE_FONT_STRETCH_SEMI_CONDENSED`
---|---|`5` or `normal`|for `DWRITE_FONT_STRETCH_NORMAL`
---|---|`6` or `semiexpanded`|for `DWRITE_FONT_STRETCH_SEMI_EXPANDED`
---|---|`7` or `expanded`|for `DWRITE_FONT_STRETCH_EXPANDED`
---|---|`8` or `extraexpanded`|for `DWRITE_FONT_STRETCH_EXTRA_EXPANDED`
---|---|`9` or `ultraexpanded`|for `DWRITE_FONT_STRETCH_ULTRA_EXPANDED`
<a name="jump_enum_animationtype"/>animationtype|`Helper::XMLGetAnimationType`|int or string| `="linear"`, `="3"`
---|---|`0` or `linear`|for `AnimationType::Type_LinearInterpolation`
---|---|`1` or `quadraticim`|for `AnimationType::Type_QuadraticEaseIn`
---|---|`2` or `quadraticout`|for `AnimationType::Type_QuadraticEaseOut`
---|---|`3` or `quadraticinout`|for `AnimationType::Type_QuadraticEaseInOut`
---|---|`4` or `cubicin`|for `AnimationType::Type_CubicEaseIn`
---|---|`5` or `cubicout`|for `AnimationType::Type_CubicEaseOut`
---|---|`6` or `cubicoinout`|for `AnimationType::Type_CubicEaseInOut`
---|---|`7` or `quarticin`|for `AnimationType::Type_QuarticEaseIn`
---|---|`8` or `quarticout`|for `AnimationType::Type_QuarticEaseOut`
---|---|`9` or `quarticinout`|for `AnimationType::Type_QuarticEaseInOut`
---|---|`10` or `quinticcin`|for `AnimationType::Type_QuinticEaseIn`
---|---|`11` or `quinticcout`|for `AnimationType::Type_QuinticEaseOut`
---|---|`12` or `quinticinout`|for `AnimationType::Type_QuinticEaseInOut`
---|---|`13` or `sincin`|for `AnimationType::Type_SineEaseIn`
---|---|`14` or `sincout`|for `AnimationType::Type_SineEaseOut`
---|---|`15` or `sininout`|for `AnimationType::Type_SineEaseInOut`
---|---|`16` or `circularcin`|for `AnimationType::Type_CircularEaseIn`
---|---|`17` or `circularcout`|for `AnimationType::Type_CircularEaseOut`
---|---|`18` or `circularinout`|for `AnimationType::Type_CircularEaseInOut`
---|---|`19` or `exponentiacin`|for `AnimationType::Type_ExponentialEaseIn`
---|---|`20` or `exponentiaout`|for `AnimationType::Type_ExponentialEaseOut`
---|---|`21` or `exponentiainout`|for `AnimationType::Type_ExponentialEaseInOut`
---|---|`22` or `elasticin`|for `AnimationType::Type_ElasticEaseIn`
---|---|`23` or `elasticout`|for `AnimationType::Type_ElasticEaseOut`
---|---|`24` or `elasticinout`|for `AnimationType::Type_ElasticEaseInOut`
---|---|`25` or `backin`|for `AnimationType::Type_BackEaseIn`
---|---|`26` or `backout`|for `AnimationType::Type_BackEaseOut`
---|---|`27` or `backinout`|for `AnimationType::Type_BackEaseInOut`
---|---|`28` or `bouncein`|for `AnimationType::Type_BounceEaseIn`
---|---|`29` or `bounceout`|for `AnimationType::Type_BounceEaseOut`
---|---|`30` or `bounceinout`|for `AnimationType::Type_BounceEaseInOut`
