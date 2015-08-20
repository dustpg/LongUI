## XML Value Type List
value type|implement|note|e.g.
:--------:|---------|----|----
<span id="jump_int">int</span>|`LongUI::AtoI`|integer |`= "0"`, `= "2333"`
<span id="jump_bool">bool</span>|`pugi::xml_attribute::as_bool`| boolean |`= "true"`, `= "0"`
<span id="jump_float">float</span>|`LongUI::AtoF`|float| `= "0"`, `= "1e3"`, `= "-1.0"`
<span id="jump_floatx2">floatx2</span>|`LongUI::Helper::MakeFloats`|array for 2 floats| `="0.0, 2.33333"`
<span id="jump_floatx4">floatx4</span>|`LongUI::Helper::MakeFloats`|array for 4 floats| `="0.0, 2.33333, -123, 1e-2"`
<span id="jump_string">string</span>|`LongUI::Helper::MakeString`|string, utf-8 only| `="_(:3」∠)_"`
<span id="jump_cc">CC</span>|`std::strchr`,`LongUI::AtoI`|"CC" for "CreateControl", `name[, templateid]`| `="ScrollBarA"`, `="ScrollBarB, 2"`
<span id="jump_color">color</span>|`LongUI::Helper::MakeColor`|color, `#RGB`, `#RRGGBB`, `#AARRGGBB`, `floatx4 in RGBA` | `="#FFF"`, `="#ABCDEF"`, `="#88ABCDEF"`, `="0.0, 1.0, 0.0, 1.0"`

## LongUI Build-in Control List
  
class|super|note
---|-----|----
[UIControl](#jump_control)|**NONE**|LongUI control base class
[UIContainer](#jump_container)|[UIControl](#jump_control)|LongUI container base class
[UIVerticalLayout](#jump_vlayout)|[UIContainer](#jump_container)|LongUI vertical-layout container
[UIWindow](#jump_window)|[UIVerticalLayout](#jump_vlayout)|LongUI window base class

## <span id="jump_control">LongUI::UIControl XML Attribute(s)</span>
  
relevant class|note
--------------|----
class | `LongUI::UIControl`
super |  **NONE**
  
attribute name|value type|default|note
--------------|----------|-------|----
`pos`|[floatx2](#jump_floatx2)|(0.0, 0.0)|set left-top position if floating
`size`|[floatx2](#jump_floatx2)|(0.0, 0.0)|set contrl size if static
`name`|[string](#jump_string)|(empty)|name of control, uniqued in **one window**
`weight`|[float](#jump_float)|1.0|interpreted by parent/container
`margin`|[floatx4](#jump_floatx4)|(0.0, 0.0, 0.0, 0.0)|set margin of contrl
`script`|[string](#jump_string)|(empty)|interpreted by yourself
`visible`|[bool](#jump_bool)|true|as you can see
`priority`|[int](#jump_int)(0~255)|127|just a byte, priority for rendering, **now valid** for dirty-rendering
`strictclip`|[bool](#jump_bool)|??|**invalid yet**
`templateid`|[int](#jump_int)(>=0)|0|you can set some default value as **template**, like `class`/`id` in html but int only
`userstring`|[string](#jump_string)|(empty)|user defined string, interpreted by yourself
`renderparent`|[bool](#jump_bool)|false|if render this control, must render parent (parent will render itself and **all children**)
`borderwidth`|[float](#jump_float)|0.0|set width of border

## <span id="jump_control">LongUI::UIContainer XML Attribute(s)</span>
  
relevant class|note
--------------|----
class | `LongUI::UIContainer`
super | [UIControl](#jump_control)
  
attribute name|value type|default|note
--------------|----------|-------|----
`hostchild`|[bool](#jump_bool)|false|like 'renderparent' in control, `renderparent` is for container.
--|--|--|if it's child want to be rendered, it will render this container. to implement this, it will **force** mark all children `renderparent` to true
`leftcontrol`|[CC](#jump_cc)|(empty)|left marginal control **name** and **templateid** ,btw, `CC` for `CreateControl`
`topcontrol`|[CC](#jump_cc)|(empty)|top marginal control, like `leftcontrol`
`rightcontrol`|[CC](#jump_cc)|(empty)|right marginal control, like `leftcontrol`
`bottomcontrol`|[CC](#jump_cc)|(empty)|bottom marginal control, like `leftcontrol`
`zoommarginal`|[bool](#jump_bool)|true|marginal control will be zoomed as **current container**

## <span id="jump_window">LongUI::UIWindow XML Attribute(s)</span>
  
relevant class|note
--------------|----
class | `LongUI::UIWindow`
super | [UIVerticalLayout](#jump_vlayout)
  
attribute name|value type|default|note
--------------|----------|-------|----
`clearcolor`|[color](#jump_color)|(1.0, 1.0, 1.0, 1.0)|clear color to call `ID2D1RenderTarget::Clear`