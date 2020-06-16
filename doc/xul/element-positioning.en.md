# Box Element Positioning

So far, we know how to position elements either horizontally or vertically inside a box. We will often need more control over the position and size of elements within the box. For this, we first need to look at how a box works.

The position of an element is determined by the layout style of its container. For example, the position of a button in a horizontal box is to the right of the previous button, if any. The size of an element is determined by two factors, the size that the element wants to be and the size you specify. The size that an element wants to be is determined by what is in the element. For example, a button's width is determined by the amount of text inside the button.

An element will generally be as large as it needs to be to hold its contents, and no larger. Some elements, such as textboxes have a default size, which will be used. A box will be large enough to hold the elements inside the box. A horizontal box with three buttons in it will be as wide as the three buttons, plus a small amount of padding.

![ex_boxstyle_2](./images/ex_boxstyle_2.png)

In the image, the first two buttons have been given a suitable size to hold their text. The third button is larger because it contains more content. The width of the box containing the buttons is the total width of the buttons plus the padding between them. The height of the buttons is a suitable size to hold the text.


## Width and height attributes

You may need to have more control over the size of an element in a window. There are a number of features that allow you to control the size of an element. The quick way is to simply add the `width` and `height` attributes on an element, much like you might do on an HTML img tag. An example is shown below:


## Example 1

 - [Source](./source/ex_boxstyle_1.xul)

![ex_boxstyle_1](./images/ex_boxstyle_1.png)

```xml
<button label="OK" width="100" height="40"/>
```

However, it is not recommended that you do this. It is not very portable and may not fit in with some themes. A better way is to use style properties, which work similarly to style sheets in HTML. The following CSS properties can be used.

 - `width`: This specifies the width of the element
 - `height`: This specifies the height of the element

By setting either of the two properties, the element will be created with that width and height. If you specify only one size property, the other is calculated as needed. The size of these style properties should be specified as a number followed by a unit.


## Flexible elements

The sizes are fairly easy to calculate for non-flexible elements. They simply obey their specified widths and heights, and if the size wasn't specified, the element's default size is just large enough to fit the contents. For flexible elements, the calculation is slightly trickier.

Flexible elements are those that have a `flex` attribute set to a value greater than 0. Recall that flexible elements grow and shrink to fit the available space. Their default size is still calculated the same as for inflexible elements. The following example demonstrates this:

## Example 2 

 - [Source](./source/ex_boxstyle_2.xul)

![ex_boxstyle_2](./images/ex_boxstyle_2.png)

```xml
<window orient="horizontal"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">

<hbox>
  <button label="Yes" flex="1"/>
  <button label="No"/>
  <button label="I really don't know one way or the other"/>
</hbox>

</window>
```

The window will initially appear like in the image earlier. The first two buttons will be sized at a suitable default width and the third button will be larger because it has a longer label. The first button is made flexible and all three elements have been placed inside a box. The width of the box will be set to the initial total width of all three buttons.

If you increase the width of the window, elements are checked to see whether they are flexible to fill the blank space that would appear. The first button is the only flexible element, but it will not grow wider. This is because the box that the button is inside is not flexible. An inflexible element never changes size even when space is available, so the button can't grow either. Thus, the button won't get wider.

The solution is to make the box flexible also. Then, when you make the window wider, extra space will be available, so the box will grow to fill the extra space. Because the box is larger, more extra space will be created inside it, and the flexible button inside it will grow to fit the available space. This process repeats for as many nested boxes as necessary.


# Box Packing

Let's say you have a box with two child elements, both of which are not flexible, but the box is flexible. For example:


## Example 3

 - [Source](./source/ex_boxstyle_3.xul)

![ex_boxstyle_3](./images/ex_boxstyle_3.png)

```xml
<box flex="1">
  <button label="Happy"/>
  <button label="Sad"/>
</box>
```

If you resize the window, the box will stretch to fit the window size. The buttons are not flexible, so they will not change their widths. The result is extra space that will appear on the right side of the window, inside the box. You may wish, however, for the extra space to appear on the left side instead, so that the buttons stay right aligned in the window.

You could accomplish this by placing a [spacer](./spacer.md) inside the box, but that gets messy when you have to do it numerous times. A better way is to use an additional attribute `pack` on the [box](./box.md). This attribute indicates how to pack the child elements inside the box. For horizontally oriented boxes, it controls the horizonal positioning of the children. For vertically oriented boxes, it controls the vertical positioning of the children. You can use the following values:

 - `start` This positions elements at the left edge for horizontal boxes and at the top edge for vertical boxes. This is the default value.
 - `center` This centers the child elements in the box.
 - `end` This positions elements at the right edge for horizontal boxes and at the bottom edge for vertical boxes.

 The `pack` attribute is applied to the box containing the elements to be packed, not to the elements themselves.

We can change the earlier example to center the elements as follows:

## Example 4

 - [Source](./source/ex_boxstyle_4.xul)

![ex_boxstyle_4](./images/ex_boxstyle_4.png)

```xml
<box flex="1" pack="center">
  <button label="Happy"/>
  <button label="Sad"/>
</box>
```

Now, when the window is resized, the buttons center themselves horizontally. Compare this behavior to that of the previous example.

# Box Alignment

If you resize the window in the Happy-Sad example above horizontally, the box will grow in width. If you resize the window vertically however, you will note that the buttons grow in height. This is because the flexibility is assumed by default in the other direction.

You can control this behavior with the `align` attribute. For horizontal boxes, it controls the position of the children vertically. For vertical boxes, it controls the position of the children horizontally. The possible values are similar to those for `pack`.

 - `start` This aligns elements along the top edge for horizontal boxes and along the left edge for vertical boxes.
 - `center` This centers the child elements in the box.
 - `end` This aligns elements along the bottom edge for horizontal boxes and along the right edge for vertical boxes.
 - `baseline` This aligns the elements so that the text lines up. This is only useful for horizontal boxes.
 - `stretch` This value, the default, causes the elements to grow to fit the size of the box, much like a flexible element, but in the opposite direction.

 As with the `pack` attribute, the `align` attribute applies to the box containing the elements to be aligned, not to the elements themselves.

For example, the first box below will have its children stretch, because that is the default. The second box has an `align` attribute, so its children will be placed centered.


## Example 5

 - [Source](./source/ex_boxstyle_5.xul)

![ex_boxstyle_5](./images/ex_boxstyle_5.png)

```xml
<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin/" type="text/css"?>

<window id="yesno" title="Question" orient="horizontal"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">

  <hbox>
    <button label="Yes"/>
    <button label="No"/>
  </hbox>
  <hbox align="center">
    <button label="Maybe"/>
    <button label="Perhaps"/>
  </hbox>

</window>
```

You can also use the style properties `-moz-box-pack` and `-moz-box-align` instead of specifying attributes.


# REF

 - [XUL-Element_Positioning](https://developer.mozilla.org/en-US/docs/Mozilla/Tech/XUL/Tutorial/Element_Positioning)
