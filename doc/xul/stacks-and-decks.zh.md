有时候需要将元素按照一系列重叠的卡片一样进行显示。栈(`stack`)和层(`deck`)元素可以达到这一目的。

# 容器

每一个XUL格子元素都是一个可以容纳其他元素的容器。有很多具有特殊样式的格子，例如[工具条](./toolbars.md)和[标签面板](./tabboxes.zh.md)。标签`box`可以创建不带有任何特殊属性的最简单格子。然而，具有特殊样式的格子与常规的格子一样对其内部的元素进行排列，只是他们有附加特性而已。

事实上，很多组件可以容纳其他元素。我们已经看到了按钮可以容纳除了默认元素之外的其他元素。滚动条也是也是一种特殊的格子类型，如果你不提供元素的话，他们会自己创建自己的元素。它也会自己处理滑块的运动。

接下来的几节，我们会介绍一些用来容纳其他元素的元素。他们都是有特殊样式的格子，可以将格子的所有属性都用在这些元素上。

# 栈

元素[stack](./stack.md)是一个简单的格子，与其他格子的工作模式一样，只是它的子元素将会处于所有其他元素的最上面。第一个子元素会在最下面。第二个紧接着第一个，接着是第三个，等等。在一个`栈`中可以堆上任意多的元素。

属性`orient`没有什么意义，因为`栈`中的子元素都是一个在一个的上面，而不是一个紧挨一个的。`栈`的大小由最大的子元素的大小决定。但你可以使用CSS属性的`width`，`height`，`min-width`和其他相关属性来控制`栈`和他的子元素。

元素[stack](./stack.md)可以在需要为已经存在的元素添加状态指示器时使用。例如，进度条可能会用一个普通的进度条和上面一个`label`来创建。


## 带有阴影的栈


为了方便的使用[stack](./stack.md)元素，可能需要为它仿制很多CSS属性。比如像下面这样为它创建一个文字阴影(`text-shadow`效果)的效果：


 - 例 1
 - [源代码](./source/ex_stacks_1.xul)

![ex_stacks_1](./images/ex_stacks_1.png)

```xml
<stack>
  <label value="Shadowed" style="padding-left: 1px; padding-top: 1px; font-size: 15px"/>
  <label value="Shadowed" style="color: red; font-size: 15px;"/>
</stack>
```

两个`label`元素都创建了一个大小为15像素的文字。首先是通过在左边和顶部添加一个1像素的偏移。这样的结果是重复绘制文字'Shadowed'但会与另一个有一点点的偏移。

这种方法的比使用`text-shadow`有一些优势，因为你可以完全将主体文字与阴影分开进行控制。它可以有自己的字体，下划线或者大小。（你甚至可以制作阴影闪烁的效果）。这在mozilla不支持CSS的文字阴影时也很有用。缺点是阴影的存在使得堆的尺寸变大了。。阴影效果对于创建禁用的按钮的外观很有用：

 - 例 2
 - [源代码](./source/ex_stacks_2.xul)

![ex_stacks_2](./images/ex_stacks_2.png)

```xml
<stack style="background-color: #C0C0C0">
  <label value="Disabled" style="color: white; padding-left: 1px; padding-top: 1px;"/>
  <label value="Disabled" style="color: grey;"/>
</stack>
```

这样安排文字和阴影颜色在某些平台上会产生禁用的外观

注意，鼠标点击或者按键按下的事件是在堆上顶部元素，即堆中的最后一个元素。就意味着按钮只有是堆中最后一个元素的时候才能工作。

# 层

元素[deck](./deck.md)也是和[stack](./stack.md)一样只在顶部显示一个子元素，但是面板一次只显示一个子元素。这对向导接口非常有用，其中一系列相似的面板顺序显示。你可以通过使面板中的内容发生变化而只创建一个窗口，而不是创建多个独立的窗口和导航按钮。

与堆相似，`deck`元素的直接子元素组成的页面。面板的显示页面可以通过改变设置`selectedIndex`属性来控制显示出来的子元素。索引是一个数字，可以指定哪一个页面将会显示。页面索引起始值为0.因此第一个子元素的page0，第二个是page1，等等。


以下は、デッキの例です。

 - 例 3
 - [源代码](./source/ex_stacks_3.xul)

![ex_stacks_3](./images/ex_stacks_3.png)

```xml
<deck selectedIndex="2">
  <label value="This is the first page"/>
  <button label="This is the second page"/>
  <box>
    <label value="This is the third page"/>
    <button label="This is also the third page"/>
  </box>
</deck>
```

这里有三个页面，默认为显示第三个。第三个页面是一个格子，里面有两个元素。格子里面的元素和格子本身组成了一个页面。面板的大小是子元素中最大的大小这里就是第三个页面。

你可以改变`selectedIndex`属性来对页面进行切换。

# REF

 - [XUL-Stacks_and_Decks](https://developer.mozilla.org/en-US/docs/Archive/Mozilla/XUL/Tutorial/Stacks_and_Decks)
