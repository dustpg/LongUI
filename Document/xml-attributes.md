## XML Value Type List
value type|implement|note|e.g.
:--------:|---------|----|----
`int`|`LongUI::AtoI`|integer |`= "0"`, `= "2333"`
`bool`|`pugi::xml_attribute::as_bool`| boolean |`= "true"`, `= "0"`
`float`|`LongUI::AtoF`|float| `= "0"`, `= "1e3"`, `= "-1.0"`
`floatx2`|`LongUI::Helper::MakeFloats`|array for 2 floats| `="(0.0, 2.33333)"`
`floatx4`|`LongUI::Helper::MakeFloats`|array for 4 floats| `="(0.0, 2.33333, -123, 1e-2)"`
`CC`|`std::strchr`,`LongUI::AtoI`|`CC` for `CreateControl`, `name[, templateid]`| `="ScrollBarA"`, `="ScrollBarB, 2"`
`color`|`LongUI::Helper::MakeColor`|color, `#RGB`, `#RRGGBB`, `#AARRGGBB`, `floatx4 in RGBA` | `="#FFF"`, `="#ABCDEF"`
`="#88ABCDEF"`, `="0.0, 1.0, 0.0, 1.0"`

## XML Attributes List


  
class|super|attribute name|value type|default|note
:---:|------|--------------|----------|-------|---
`UIControl`|**NONE**|`script`|string|(empty)|interpreted by youself
--|--|`weight`|float|1.0|interpreted by parent/container
--|--|`name`|string|(empty)|name of control, uniqued in **one window**
--|--|`visible`|bool|true|as you can see
--|--|`pos`|floatx2|(0.0, 0.0)|set left-top position if floating
--|--|`size`|floatx2|(0.0, 0.0)|set contrl size if static
--|--|`margin`|floatx4|(0.0, 0.0, 0.0, 0.0)|set margin of contrl
--|--|`borderwidth`|float|0.0|set width of border
--|--|`priority`|int(0~255)|127|just a byte, priority for rendering, **now valid** for dirty-rendering
--|--|`templateid`|int(>=0)|0|you can set some default value as **template**, like `class` in html
--|--|`strictclip`|bool|??|**invalid yet**
--|--|`renderparent`|bool|false|if render this control, must render parent(parent will render itself and **all children**)
`UIContainer`|`UIControl`|`hostchild`|bool|false|like 'renderparent' in control, `renderparent` is for container.
--|--|--|--|--|if it's child want to be rendered, it will render this container. to implement
--|--|--|--|--|this, it will **force** mark all children `renderparent` to true
--|--|`leftcontrol`|CC|(empty)|left marginal control **name** and **templateid** ,btw, `CC` for `CreateControl`
--|--|`topcontrol`|CC|(empty)|top marginal control, like `leftcontrol`
--|--|`rightcontrol`|CC|(empty)|right marginal control, like `leftcontrol`
--|--|`bottomcontrol`|CC|(empty)|bottom marginal control, like `leftcontrol`
--|--|`zoommarginal`|bool|true|marginal control will be zoomed as **current container**
`UIWindow`|`UIVerticalLayout`|`clearcolor`|color|false|like 'renderparent' in control, `renderparent` is for container.