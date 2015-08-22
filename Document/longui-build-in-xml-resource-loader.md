## LongUI Build-in XML Described Resource Loader Guide
  
parent node name|attribute name|value type|default|note
----------------|--------------|----------|-------|----
`Bitmap`|`res`|[string](./longui-xml-value-type.md#jump_string)|(empty)|URI for image file
  |  |  |  |  
`Brush`|`opacity`|[float](./longui-xml-value-type.md#jump_float)|1.0|opacity for `ID2D1Brush`
`Brush`|`transform`|[floatx6](./longui-xml-value-type.md#jump_jump_floatx6)|(1,0,0,1,0,0)|transform for `ID2D1Brush`
`Brush`|`type`|[enum brushtyp](./longui-xml-value-type.md#jump_enum_brushtype)|0 or "solid"|type for this brush
`Brush`|`color`|[color](./longui-xml-value-type.md#jump_color)|(0,0,0,1)|for `solid`, color for  `ID2D1SolidColorBrush`
`Brush`|`stops`|floatxn|(empty)|for `linear` and `radial`,  array in any size "stops" for `ID2D1GradientStopCollection`
`Brush`|`start`|floatx2|(0.0, 0.0)|for `linear`, start point for `D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES::startPoint`
`Brush`|`end`|floatx2|(0.0, 0.0)|for `linear`, end point for `D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES::endPoint`
`Brush`|`stops`|floatxn|(empty)|for `linear` and `radial`,  array in any size "stops" for `ID2D1GradientStopCollection`
`Brush`|`center`|floatx2|(0.0, 0.0)|for `radial`, center point for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::center`
`Brush`|`offset`|floatx2|(0.0, 0.0)|for `radial`, origin offset for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::gradientOriginOffset`
`Brush`|`rx`|[float](./longui-xml-value-type.md#jump_float)|0.0|for `radial`, x-radiu for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::radiusX`
`Brush`|`ry`|[float](./longui-xml-value-type.md#jump_float)|0.0|for `radial`, y-radiu for `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES::radiusY`
`Brush`|`bitmap`|int|(empty)|for `bitmap`, common bitmap resource index
`Brush`|`extendx`|[enum extend](./longui-xml-value-type.md#jump_enum_extend)|0 or "clamp"|for `bitmap`, x-extendy mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::extendModeX`
`Brush`|`extendy`|[enum extend](./longui-xml-value-type.md#jump_enum_extend)|0 or "clamp"|for `bitmap`, y-extendy mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::extendModeY`
`Brush`|`interpolation`|[enum interpolation](./longui-xml-value-type.md#jump_enum_interpolation)|1 or "linear"|for `bitmap`, interpolation mode for `D2D1_BITMAP_BRUSH_PROPERTIES1::interpolationMode`
`Font` or|  |  |  |  
`TextFormat`|`family`|[string](./longui-xml-value-type.md#jump_string)|"Arial"|font family name
`TextFormat`|`size`|[float](./longui-xml-value-type.md#jump_float)|12.0|font size
`TextFormat`|`weight`|int(0~1000)|400|`DWRITE_FONT_WEIGHT_NORMAL`=400 in `DWRITE_FONT_WEIGHT`
`TextFormat`|`style`|  |  |  
`TextFormat`|`stretch`|  |  |  
`TextFormat`|`flowdirection`|  |  |  
`TextFormat`|`tabstop`|float|(font size) x 4| width of `\t`
`TextFormat`|`valign`|  |  |  
`TextFormat`|`align`|  |  |  
`TextFormat`|`readingdirection`|  |  |  
`TextFormat`|`wordwrapping`|  |  |  
  |  |  |  |  
`Meta`|`rect`|floatx4|(0,0,1,1)|rect of bitmap, `src_rect`
`Meta`|`bitmap`|int|(empty)|common bitmap resource index
`Meta`|`rule`|[enum bmprule](./longui-xml-value-type.md#jump_enum_bmprule)|0 or "scale"| rendering-rule for this meta
`Meta`|`interpolation`|[enum interpolation](./longui-xml-value-type.md#jump_enum_interpolation)|0 or "neighbor"|or `D2D1_INTERPOLATION_MODE`
  