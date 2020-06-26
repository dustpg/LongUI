# vbox

 - A container element which can contain any number of child elements. This is equivalent to the [box](./box.md) element, except it defaults to vertical orientation.
 - 様々な子要素を含むことができるコンテナ要素です。これはデフォルトで縦方向に並ぶこと以外は [box](./box.md) 要素と同等です。
 - 一个可以包含任何子元素的容器元素。这相当于默认为垂直方向的 [box](./box.md) 元素。
 - LongUI::UIVBoxLayout

*HOW IMPL*

```cpp
// v-box layout
class UIVBoxLayout final : public UIBoxLayout {
    UIVBoxLayout(/*XXX*/) /*XXX*/ {
        m_state.orient = Orient_Vertical;
    }
};


```

## Example

```xml
<!-- Two labels at bottom -->
<vbox>
  <spacer flex="1"/>
  <label value="One"/>
  <label value="Two"/>
</vbox>
```


## REF

 - [XUL-vbox](https://developer.mozilla.org/en-US/docs/Archive/Mozilla/XUL/vbox)
 - [box](./box.md)
 - [LongUI::UIVBoxLayout](../../src/control/ui_box.cpp)

