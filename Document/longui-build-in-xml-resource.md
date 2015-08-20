  
## XML Value Type List
value type|implement("LongUI::" omitted)|note|e.g.
:--------:|---------|----|----
<span id="jump_float">float</span>|`AtoF`|float| `= "0"`, `= "1e3"`, `= "-1.0"`
<span id="jump_floatx6">floatx6</span>|`Helper::MakeFloats`|array for 6 floats| `="0.0, 2.33333, -123, 1e-2,0,0"`
<span id="jump_string">string</span>|`Helper::MakeString`|string, utf-8 only| `="_(:3」∠)_"`
<span id="jump_color">color</span>|`Helper::MakeColor`|color, `#RGB`, `#RRGGBB`, `#AARRGGBB`, `floatx4 in RGBA` | `="#FFF"`, `="#ABCDEF"`, `="#88ABCDEF"`, `="0.0, 1.0, 0.0, 1.0"`
<span id="jump_enum_brushtype">enum brushtype</span>|`Helper::XMLGetValueEnum`|int or string| `="radial"`, `="1"`
---|---|`0` or `solid`| solid color brush for `ID2D1SolidColorBrush`
---|---|`1` or `linear`| linear gradient brush for `ID2D1LinearGradientBrush`
---|---|`2` or `radial`| radial gradient brush for `ID2D1RadialGradientBrush`
---|---|`3` or `bitmap`| bitmap for `ID2D1BitmapBrush1`
<span id="jump_enum_extend">enum extend</span>|`Helper::XMLGetValueEnum`|int or string| `="mirror"`, `="1"`
---|---|`0` or `clamp`|for `D2D1_EXTEND_MODE_CLAMP`
---|---|`1` or `wrap`|for `D2D1_EXTEND_MODE_WRAP`
---|---|`2` or `mirror`|for `D2D1_EXTEND_MODE_MIRROR`
<span id="jump_enum_extend">enum extend</span>|`Helper::XMLGetValueEnum`|int or string| `="highcubic"`, `="1"`
---|---|`0` or `neighbor`|for `D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR`
---|---|`1` or `linear`|for `D2D1_INTERPOLATION_MODE_LINEAR`
---|---|`2` or `cubic`|for `D2D1_INTERPOLATION_MODE_CUBIC`
---|---|`3` or `mslinear`|for `D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR`
---|---|`4` or `anisotropic`|for `D2D1_INTERPOLATION_MODE_ANISOTROPIC`
---|---|`5` or `highcubic`|for `D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC`
  
## LongUI Build-in XML Described Resource
  
parent node name|attribute name|value type|default|note
----------------|--------------|----------|-------|----
`Bitmap`|`res`|[string](#jump_string)|(empty)|URI for image file
----------------|--------------|----------|-------|----
----------------|--------------|----------|-------|----
`Brush`|`opacity`|[float](#jump_float)|1.0|opacity for `ID2D1Brush`
`Brush`|`transform`|[floatx6](#jump_jump_floatx6)|(1.0,0.0,0.0,1.0,0.0,0.0)|transform for `ID2D1Brush`
`Brush`|`type`|[enum brushtyp](#jump_enum_brushtype)|0 or "solid"|type for this brush
----------------|--------------|----------|-------|----
`Brush`|`color`|[color](#jump_color)|D2D1::ColorF(D2D1::ColorF::Black)|for `solid`, color for  `ID2D1SolidColorBrush`
----------------|--------------|----------|-------|----
`Brush`|`stops`|floatxn|(empty)|for `linear` and `radial`,  array in any size "stops" for `ID2D1GradientStopCollection`
`Brush`|`start`|floatx2|(0.0, 0.0)|for `linear`, start point for `D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES::startPoint`
`Brush`|`end`|floatx2|(0.0, 0.0)|for `linear`, end point for `D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES::endPoint`
----------------|--------------|----------|-------|----
`Brush`|`stops`|floatxn|(empty)|for `linear` and `radial`,  array in any size "stops" for `ID2D1GradientStopCollection`
`Brush`|`center`|floatx2|(0.0, 0.0)|for `radial`, center point for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::center`
`Brush`|`offset`|floatx2|(0.0, 0.0)|for `radial`, origin offset for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::gradientOriginOffset`
`Brush`|`rx`|[float](#jump_float)|0.0|for `radial`, x-radiu for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::radiusX`
`Brush`|`ry`|[float](#jump_float)|0.0|for `radial`, y-radiu for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::radiusY`
----------------|--------------|----------|-------|----
`Brush`|`bitmap`|int|(empty)|for `bitmap`, common bitmap resource index
`Brush`|`extendx`|[enum extend](#jump_enum_extend)|0 or "clamp"|for `bitmap`, x-extendy mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::extendModeX`
`Brush`|`extendy`|[enum extend](#jump_enum_extend)|0 or "clamp"|for `bitmap`, y-extendy mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::extendModeY`
`Brush`|`interpolation`|[enum interpolation](#jump_enum_interpolation)|1 or "linear"|for `bitmap`, interpolation mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::interpolationMode`

```cpp
    Helper::MakeFloats(node.attribute("center").value(), &rgbprop.center.x, 2);
    Helper::MakeFloats(node.attribute("offset").value(), &rgbprop.gradientOriginOffset.x, 2);
    Helper::MakeFloats(node.attribute("rx").value(), &rgbprop.radiusX, 1);
    Helper::MakeFloats(node.attribute("ry").value(), &rgbprop.radiusY, 1);
```