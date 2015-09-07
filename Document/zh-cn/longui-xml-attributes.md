## LongUI 内建控件列表  
  
类|基类|注
---|----|---
[UIControl](#jump_control)|**NONE**|LongUI基本控件类
[UIContainer](#jump_container)|[UIControl](#jump_control)|LongUI基本容器类
[UIVerticalLayout](#jump_vlayout)|[UIContainer](#jump_container)|LongUI垂直布局类
[UIWindow](#jump_window)|[UIVerticalLayout](#jump_vlayout)|LongUI基本窗口类

##<a name="jump_control"/>LongUI::UIControl Xml属性
  
相关类|注
------|----
类 | `LongUI::UIControl`
基 |  **NONE**
  
属性名|值类型|默认值|注
------|------|------|---
`pos`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|设置浮动控件的左上角坐标
`size`|[floatx2](./longui-xml-value-type.md#jump_floatx2)|(0.0, 0.0)|设置固定大小控件的尺寸
`name`|[string](./longui-xml-value-type.md#jump_string)|(empty)|控件名称, **o同一窗口**内必须独一无二
`weight`|[float](./longui-xml-value-type.md#jump_float)|1.0|布局权重, 被父容器解释
`margin`|[floatx4](./longui-xml-value-type.md#jump_floatx4)|(0.0, 0.0, 0.0, 0.0)|外边距
`script`|[string](./longui-xml-value-type.md#jump_string)|(empty)|脚本数据, 需要您自己解释
`visible`|[bool](./longui-xml-value-type.md#jump_bool)|true|可视性
`bgbrush`|[int](./longui-xml-value-type.md#jump_int)|0|背景笔刷id, 0为空而非默认笔刷
`priority`|[int](./longui-xml-value-type.md#jump_int)(0~255)|127|一字节, 渲染优先级, **目前**用于脏矩形渲染
`strictclip`|[bool](./longui-xml-value-type.md#jump_bool)|??|**i目前无效**
`templateid`|[int](./longui-xml-value-type.md#jump_int)(>=0)|0|模板id, 类似于html的`class`/`id`用于设定默认值
`userstring`|[string](./longui-xml-value-type.md#jump_string)|(empty)|用户字符串数据, 需要您自己解释
`borderwidth`|[float](./longui-xml-value-type.md#jump_float)|0.0|边框宽度
`renderparent`|[bool](./longui-xml-value-type.md#jump_bool)|false|预渲染此控件, 必渲染父控件(父控件会渲染自己和**所有子控件**)

##<a name="jump_container"/>LongUI::UIContainer Xml属性
  
相关类|注
------|----
类 | `LongUI::UIContainer`
基 |  [UIControl](#jump_control)
  
属性名|值类型|默认值|注
------|------|------|--
`hostchild`|[bool](./longui-xml-value-type.md#jump_bool)|false|类似于基本控件的"renderparent", "hostchild"是针对容器的
--|--|--|如果该容器的一个子控件需要渲染, 那么就需要渲染自己. 为了实现这个功能, 会**强行标记**子控件"renderparent"属性为true
`leftcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|left marginal control **name** and **templateid** ,btw, `CC` for `CreateControl`
`topcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|top marginal control, like `leftcontrol`
`rightcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|right marginal control, like `leftcontrol`
`bottomcontrol`|[CC](./longui-xml-value-type.md#jump_cc)|(empty)|bottom marginal control, like `leftcontrol`
`zoommarginal`|[bool](./longui-xml-value-type.md#jump_bool)|true|marginal control will be zoomed as **current container**

##<a name="jump_window"/>LongUI::UIWindow Xml属性
  
relevant class|note
--------------|----
class | `LongUI::UIWindow`
super | [UIVerticalLayout](#jump_vlayout)
  
attribute name|value type|default|note
--------------|----------|-------|----
`clearcolor`|[color](#jump_color)|(1.0, 1.0, 1.0, 1.0)|clear color to call `ID2D1RenderTarget::Clear`