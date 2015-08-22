## LongUI Build-in Control List
  
class|super|note
---|-----|----
[UIControl](#jump_control)|**NONE**|LongUI control base class
[UIContainer](#jump_container)|[UIControl](#jump_control)|LongUI container base class
[UIVerticalLayout](#jump_vlayout)|[UIContainer](#jump_container)|LongUI vertical-layout container
[UIWindow](#jump_window)|[UIVerticalLayout](#jump_vlayout)|LongUI window base class

##<a name="jump_control"/>LongUI::UIControl XML Attribute(s)
  
relevant class|note
--------------|----
class | `LongUI::UIControl`
super |  **NONE**
  
attribute name|value type|default|note
--------------|----------|-------|----
`pos`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|set left-top position if floating
`size`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|set contrl size if static
`name`|[string](./longui-xml-value-type.md#jump_string)|(empty)|name of control, uniqued in **one window**
`weight`|[float](./longui-xml-value-type.md#jump_float)|1.0|interpreted by parent/container
`margin`|[floatx4](./longui-xml-value-type.md#jump_floatx4)|(0.0, 0.0, 0.0, 0.0)|set margin of contrl
`script`|[string](./longui-xml-value-type.md#jump_string)|(empty)|interpreted by yourself
`visible`|[bool](./longui-xml-value-type.md#jump_bool)|true|as you can see
`bgbrush`|[int](./longui-xml-value-type.md#jump_int)|0|bursh id for back groud, 0 for null not for default brush
`priority`|[int](./longui-xml-value-type.md#jump_int)(0~255)|127|just a byte, priority for rendering, **now valid** for dirty-rendering
`strictclip`|[bool](./longui-xml-value-type.md#jump_bool)|??|**invalid yet**
`templateid`|[int](./longui-xml-value-type.md#jump_int)(>=0)|0|you can set some default value as **template**, like `class`/`id` in html but int only
`userstring`|[string](./longui-xml-value-type.md#jump_string)|(empty)|user defined string, interpreted by yourself
`renderparent`|[bool](./longui-xml-value-type.md#jump_bool)|false|if render this control, must render parent (parent will render itself and **all children**)
`borderwidth`|[float](./longui-xml-value-type.md#jump_float)|0.0|set width of border

##<a name="jump_container"/>LongUI::UIContainer XML Attribute(s)
  
relevant class|note
--------------|----
class | `LongUI::UIContainer`
super | [UIControl](#jump_control)
  
attribute name|value type|default|note
--------------|----------|-------|----
`hostchild`|[bool](./longui-xml-value-type.md#jump_bool)|false|like 'renderparent' in control, `renderparent` is for container.
--|--|--|if it's child want to be rendered, it will render this container. to implement this, it will **force** mark all children `renderparent` to true
`leftcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|left marginal control **name** and **templateid** ,btw, `CC` for `CreateControl`
`topcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|top marginal control, like `leftcontrol`
`rightcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|right marginal control, like `leftcontrol`
`bottomcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|bottom marginal control, like `leftcontrol`
`zoommarginal`|[bool](./longui-xml-value-type.md#jump_bool)|true|marginal control will be zoomed as **current container**

##<a name="jump_window"/>LongUI::UIWindow XML Attribute(s)
  
relevant class|note
--------------|----
class | `LongUI::UIWindow`
super | [UIVerticalLayout](#jump_vlayout)
  
attribute name|value type|default|note
--------------|----------|-------|----
`clearcolor`|[color](#jump_color)|(1.0, 1.0, 1.0, 1.0)|clear color to call `ID2D1RenderTarget::Clear`