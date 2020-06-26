# hbox

 - This is equivalent to the [box](./box.md) element.
 - この要素は [box](./box.md) 要素と同等です
 - 本元素等价于[box](./box.md)
 - LongUI::UIHBoxLayout

*HOW IMPL*

```cpp
  // h-box layout
  using UIHBoxLayout = UIBoxLayout;
```

## Example

```xml
<!-- Two button on the right -->
<hbox>
  <spacer flex="1"/>
  <button label="Connect"/>
  <button label="Ping"/>
</hbox>
```


## REF

 - [XUL-hbox](https://developer.mozilla.org/en-US/docs/Archive/Mozilla/XUL/hbox)
 - [box](./box.md)
 - [LongUI::UIHBoxLayout](../../src/control/ui_box.cpp)
