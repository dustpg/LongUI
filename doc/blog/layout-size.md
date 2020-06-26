以width作为例子

类型 |         下限 |      建议    | 上限
----|--------------|--------------|----------
CSS | min-width    |         width| max-width
LUI | limited-width| fitting-width| max-width


CSS的属性会覆写LUI属性对应值，并且处于高优先级。即：
 - 如果指定了CSS属性，那么写入对应的LUI属性，再标记处于CSS状态
 - 如果没有指定CSS属性，那么LUI属性会按照相应的规则处理
 - 可以通过`style`或者直接的属性进行CSS属性设置

```cpp
case BKDR_MINWIDTH:
    // minwidth
    m_oStyle.limited.width = value.ToFloat();
    m_oStyle.overflow_sizestyled.min_width = true;
    break;
case BKDR_MINHEIGHT:
    // minheight
    m_oStyle.limited.height = value.ToFloat();
    m_oStyle.overflow_sizestyled.min_height = true;
    break;
```

一个最简单的例子就是图像`<image>`，建议宽度就是使用图片的宽度，下限宽度可以考虑`0`或者`1`。

复杂点的例子就是箱型布局`<box>`。LongUI中，使用对应滚动条(这里就是水平滚动条)的`<box>`下限宽度会调整到`0`，建议宽度依然是有效子控件建议宽度(以及边框什么的)累加。如果没有使用滚动条则两值相同。

而一旦使用CSS值，则以设置值为准。

正常控件大小应该介于建议值与最大值之间，特殊情况会处于最小值与建议值之间。而这种特殊情况`<box>`处理方式有点违背常理——`flex`越大的值反而最小，可以理解为以建议值作为起点，`flex`作为伸缩参数，越大的伸得越大，缩得也越大。