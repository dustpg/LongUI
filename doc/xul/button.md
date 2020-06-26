# Button

A button that can be pressed by the user. Event handlers can be used to trap mouse, keyboard and other events. It is typically rendered as a grey outset rectangle. You can specify the label of the button using the [label](#label) attribute or by placing content inside the button.

ユーザが押すことができる button。イベントハンドラは、マウス、キーボード、その他のイベントのトラップに使えます。これは普通、グレーの長方形で描画されます。[label](#label) 属性または button 内のコンテンツでボタンのラベルを指定できます。

按钮就是可以点击的东西。事件处理程序可以用来捕获鼠标、键盘和其他事件。它通常呈现为一个灰色开始长方形。您可以指定按钮的 [label](#label) 属性来将按钮的文字设置好，或者直接写在标签中间也行。

 - LongUI::UIButton


## Attributes List
[accesskey](#accesskey), autocheck, checkState, [checked](#checked), command, crop, [dir](#dir), [disabled](#disabled), dlgtype, [group](#group), icon, [image](#image), [label](#label), open, [orient](#orient), tabindex, [type](#type)

## Gui Event List

 - **new from UIButton**:
 - oncommnad: `GuiEvent::Event_OnCommand`
 - **inherited from UIControl**:
 - onfocus: `GuiEvent::Event_OnFocus`
 - onblur: `GuiEvent::Event_OnBlur`
 - onclick: `GuiEvent::Event_OnClick`
 - (unsupport yet) ondblclick: `GuiEvent::Event_OnDblClick`

## Examples


```xml
<button label="Press Me" oncommand="alert('You pressed me!');"/>

<button type="menu" label="Menu">
    <menupopup>
        <menuitem label="Option 1" oncommand="setText('menu-text','Option 1');" />
        <menuitem label="Option 2" oncommand="setText('menu-text','Option 2');" />
        <menuitem label="Option 3" oncommand="setText('menu-text','Option 3');" />
        <menuitem label="Option 4" oncommand="setText('menu-text','Option 4');" />
    </menupopup>
</button>

<button type="radio" label="type: radio" oncommand="alert('C')" group="G"/>
<button type="radio" label="type: radio" oncommand="alert('D')" group="G"/>
```

## Attributes

### accesskey

 - `Type: character`
 - This should be set to a character that is used as a shortcut key. This should be one of the characters that appears in the [label](#label) attribute for the element.
 - basically, press `ALT + (ch)` to fire the event
 - ショートカットキーとして使用される文字を設定します。この文字はその要素の [label](#label) 属性のテキストに現れる文字の一つでなければいけません。
 - `ALT + (ch)` でイベントをトリガーする。実は、(この文字は)現れなくてもかまわない: `保存(S)`。 
 - 该字符用于使用快捷键访问本控件。通常需要[label](#label)属性中出现过一次的字符。
 - 通过`ALT + (ch)`触发事件。 (指定字符)没有出现过可以接受: `保存(S)`。


*FIRE - HOW IMPL*

```cpp
    const auto ctrl = this->access_key_map[i];
    if (ctrl && ctrl->IsEnabled() && ctrl->IsVisibleEx()) {
        ctrl->DoEvent(nullptr, { NoticeEvent::Event_DoAccessAction, 0 });
    }
```

*BUTTON - HOW IMPL*

```cpp
    case NoticeEvent::Event_DoAccessAction:
        this->SetAsDefaultAndFocus();
        this->Click();
        break;
```

### checked

 - `Type: boolean`
 - Indicates whether the element is checked or not. For buttons, the [type](#type) attribute must be set to `checkbox`or`radio` for this attribute to have any effect.
 - 要素にチェックが入っているかどうかを示します。button 要素で、この属性による効果を得るには [type](#type) 属性に `checkbox` か `radio` を設定しなければなりません
 - 表示元素`checked`状态与否。使用该属性需要将[type](#type)设置为`checkbox`或`radio`。

### dir

 - `Type: one of the values below`
 - The direction in which the child elements of the element are placed.
 - `normal`
 - `reverse`
 - 要素の子要素が配置される方向。
 - 指示该元素的子元素放置方向

### disabled

 - `Type: boolean`
 - Indicates whether the element is disabled or not. If this attribute is set, the element is disabled. Disabled elements are usually drawn with grayed-out text. If the element is disabled, it does not respond to user actions, it cannot be focused
 - 要素が無効化されているかどうかを示します。ある要素が true に設定されていたら、その要素は無効化されています。無効化された要素は通常グレイ表示のテキストで描画されます
 - 如果这个元素的disabled属性被设置为true，表示元素被禁用，被禁用的属性在页面上通常会显示灰色文本

### group
 
 - `Type: string group name`
 - Buttons with `type="radio"` and the same value for their group attribute are put into the same group. Only one button from each group can be checked at a time. If the user selects one the buttons, the others in the group are unchecked.
 - group 属性に同じ値を持つ、`type="radio"` のボタンは同じグループ内に置かれます。各グループでは一度に一つのボタンのみにチェックを入れることができます。ユーザが一つのボタンを選択すると、グループ内の他のボタンのチェックが外れます。
 - 拥有一致 group属性并且使用`type="radio"`的按钮将标记为同一个分组。同一个分组有一个元素被选中，那么其他元素会被取消选中状态。

### image

 - `Type: URI`
 - The URI of the image to appear on the element. If this attribute is empty or left out, no image appears. The position of the image is determined by the dir and [orient](#orient) attributes.
 - 要素上に表示する画像の URL。この属性が空または除外された場合、画像は表示されません。画像の位置は dir 属性および [orient](#orient) 属性によって決定されます。
 - 指定元素上显示图像的URL。省略该属性将不会显示任何图像。图像位置通过 dir 以及 [orient](#orient) 指定

### label

 - `Type: string`
 - The label that will appear on the element. If this is left out, no text appears.
 - 要素上に表示するlabel。 省略された場合、テキストは表示されない。
 - 元素上表示的label。省略就不显示任何文字。


### orient

 - `Type: one of the values below`
 - Used to specify whether the children of the element are oriented horizontally or vertically. The default value depends on the element
 - `horizontal` Child elements of the element are placed next to each other in a row in the order that they appear in the XUL source.
 - `vertical` Child elements of the element are placed under each other in a column in the order that they appear in the XUL source.
 - ある要素の子要素の並びが水平方向なのか、垂直方向なのかを指定するのに使われる。デフォルト値はその要素に依存する
 - `horizontal` : ある要素の子要素それぞれが、XUL のソース中で現れる順に、水平に一行に並んでいる。
 - `vertical` : ある要素の子要素それぞれが、XUL のソース中で現れる順に、垂直に一列に並んでいる。
 - 指定了子控件的布局(orient)为水平分布的(horizontally)或者是垂直分布的(vertically)。默认值依赖于控件本身
 - `horizontal` : 子控件会被按照在xul源文件中出现的位置依次布置在一行中。
 - `vertical` : 子控件会被按照在xul源文件中出现的位置依次布置在一列中。

### type

 - `Type: one of the values below`
 - The type of button. If this attribute is not present, a normal button is created. Leave the attribute out for a normal button.
 - `checkbox` This type of button can be in two states. The user can click the button to switch between the states. This is not the same as a checkbox because it looks like a button.
 - `menu` Set the `type` attribute to the value menu to create a button with a menu popup. Place a [menupopup](./menupopup.md) element inside the button in this case. The user may click anywhere on the button to open and close the menu.
 - `radio` The button acts like a radio button. Only one button in the group can be on a once.
 - button の形式。この属性が提供されていないときは通常のボタンが作成されます。通常のボタンではこの属性を取り除いてください。
 - `checkbox`: この button の形式には二つの状態があります。ユーザが button をクリックして状態を切り替えることができます。これは button のように見えるため、checkbox とは異なります。
 - `menu`: メニューポップアップを持つ `button` を作成するには `type` 属性の値を menu に設定します。この場合、button 内に [menupopup](./menupopup.md) 要素を置きます。ユーザがメニューを開いたり閉じたりするには button 上のどこかをクリックします。
 - `radio`: button は radio ボタンのように動作します。グループ内の button は一度に一つのボタンのみを on にすることができます。
 - 按钮的行为类型。不支持的类型**可能**会退化为普通按钮，这种情况建议留空。
 - `checkbox`: 这种类型的按钮正常情况只有两种状态，用户通过点击按钮本身来切换状态。和**复选框**不同的是——这个东西叫做**按钮**
 - `menu`: 通过指定本类型来指定按钮的弹出窗口。需要在button元素标签内部添加[menupopup](./menupopup.md) 元素之类的添加对应的菜单。用于能够点击按钮打开弹出菜单。
 - `radio`: 记得设置[group](#group)属性, 行为会类似于**单选框**

## REF
 
 - [XUL-button](https://developer.mozilla.org/en-US/docs/Archive/Mozilla/XUL/button)
 - [LongUI::UIButton](../../src/control/ui_button.cpp)