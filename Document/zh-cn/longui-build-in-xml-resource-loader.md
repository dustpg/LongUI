## LongUI Build-in XML Described Resource Loader Guide
  
parent node name|attribute name|value type|default|note
----------------|--------------|----------|-------|----
`Bitmap`|`res`|[string](./longui-xml-value-type.md#jump_string)|(empty)|URI for image file
  |  |  |  |  
`Brush`|`opacity`|[float](./longui-xml-value-type.md#jump_float)|1.0|opacity for `ID2D1Brush`
`Brush`|`transform`|[floatx6](./longui-xml-value-type.md#jump_floatx6)|(1,0,0,1,0,0)|transform for `ID2D1Brush`
`Brush`|`type`|[enum brushtyp](./longui-xml-value-type.md#jump_enum_brushtype)|0 or "solid"|type for this brush
`Brush`|`color`|[color](./longui-xml-value-type.md#jump_color)|(0,0,0,1)|for `solid`, color for  `ID2D1SolidColorBrush`
`Brush`|`stops`|[stops](./longui-xml-value-type.md#jump_stops)|(empty)|for `linear` and `radial`,  array in any size "stops" for `ID2D1GradientStopCollection`
`Brush`|`start`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|for `linear`, start point for `D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES::startPoint`
`Brush`|`end`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|for `linear`, end point for `D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES::endPoint`
`Brush`|`stops`|[stops](./longui-xml-value-type.md#jump_stops)|(empty)|for `linear` and `radial`,  array in any size "stops" for `ID2D1GradientStopCollection`
`Brush`|`center`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|for `radial`, center point for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::center`
`Brush`|`offset`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|for `radial`, origin offset for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::gradientOriginOffset`
`Brush`|`rx`|[float](./longui-xml-value-type.md#jump_float)|0.0|for `radial`, x-radiu for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::radiusX`
`Brush`|`ry`|[float](./longui-xml-value-type.md#jump_float)|0.0|for `radial`, y-radiu for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::radiusY`
`Brush`|`bitmap`|[int](./longui-xml-value-type.md#jump_int)|(empty)|for `bitmap`, common bitmap resource index
`Brush`|`extendx`|[enum extend](./longui-xml-value-type.md#jump_enum_extend)|0 or "clamp"|for `bitmap`, x-extendy mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::extendModeX`
`Brush`|`extendy`|[enum extend](./longui-xml-value-type.md#jump_enum_extend)|0 or "clamp"|for `bitmap`, y-extendy mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::extendModeY`
`Brush`|`interpolation`|[enum interpolation](./longui-xml-value-type.md#jump_enum_interpolation)|1 or "linear"|for `bitmap`, interpolation mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::interpolationMode`
`Font` or|  |  |  |  
`TextFormat`|`family`|[string](./longui-xml-value-type.md#jump_string)|"Arial"|font family name
`TextFormat`|`size`|[float](./longui-xml-value-type.md#jump_float)|12.0|font size
`TextFormat`|`weight`|[int](./longui-xml-value-type.md#jump_int)(0~1000)|400|`DWRITE_FONT_WEIGHT_NORMAL`=400 in `DWRITE_FONT_WEIGHT`
`TextFormat`|`style`|[enum fontstyle](./longui-xml-value-type.md#jump_enum_fontstyle)|"normal"| style of font
`TextFormat`|`stretch`|[enum jfontstretch](./longui-xml-value-type.md#jump_enum_fontstretch)|"normal"|stretch of font
`TextFormat`|`flowdirection`|[enum flowdirection](./longui-xml-value-type.md#jump_enum_flowdirection)|"top2bottom"|direction for flow
`TextFormat`|`tabstop`|[float](./longui-xml-value-type.md#jump_float)|(font size) x 4| width of `\t`
`TextFormat`|`valign`|[enum valign](./longui-xml-value-type.md#jump_enum_valign)|"middle"|align in vertical
`TextFormat`|`align`|[enum halign](./longui-xml-value-type.md#jump_enum_halign)|"center"|align in horizontal
`TextFormat`|`readingdirection`|[enum readingdirection](./longui-xml-value-type.md#jump_enum_readingdirection)|"left2right"|direction for reading
`TextFormat`|`wordwrapping`|[enum wordwrapping](./longui-xml-value-type.md#jump_enum_wordwrapping)|"nowrap"|wrapping rule for word
  |  |  |  |  
`Meta`|`rect`|[floatx4](./longui-xml-value-type.md#jump_floatx4)|(0,0,1,1)|rect of bitmap, `src_rect`
`Meta`|`bitmap`|[int](./longui-xml-value-type.md#jump_int)|(empty)|common bitmap resource index
`Meta`|`rule`|[enum bmprule](./longui-xml-value-type.md#jump_enum_bmprule)|0 or "scale"| rendering-rule for this meta
`Meta`|`interpolation`|[enum interpolation](./longui-xml-value-type.md#jump_enum_interpolation)|0 or "neighbor"|or `D2D1_INTERPOLATION_MODE`
  