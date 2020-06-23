XUL has a set of elements for creating tabular grids.

# XUL Tabular Layout

XUL has a set of elements for doing layout of elements in a grid-like manner using the [grid](./grid.md) element. It has some similarities to the HTML `table` tag. The grid does not display anything itself; it is used only to position elements in a tabular form with rows and columns.

A grid contains elements that are aligned in rows just like tables. Inside a [grid](./grid.md) , you declare two things, the columns that are used and the rows that are used. Just like HTML tables, you put content such as labels and buttons inside the rows. However, the grid allows either row or column based organization so you may put content in either rows or in columns. It is most common to use rows, as with a table. However, you can still use columns to specify the size and appearance of the columns in a grid. Alternatively, you can put content inside the columns, and use the rows to specify the appearance. We'll look at the case of organizing elements by row first.

## Declaring a grid

To declare a set of [rows](./rows.md), use the rows tag, which should be a child element of [grid](./grid.md). Inside that you should add [row](./row.md) elements, which are used for each row. Inside the row element, you should place the content that you want inside that row.

Similarly, the columns are declared with the [columns](./columns.md) element, which should be placed as a child element of the [grid](./grid.md). Inside that go individual [column](./column.md) elements, one for each column you want in the grid.

This should be easier to understand with an example.

 - Example 1
 - [Source](./source/ex_grids_1.xul)

![ex_grids_1](./images/ex_grids_1.png)

```xml
<grid flex="1">
  
  <columns>
    <column flex="2"/>
    <column flex="1"/>
  </columns>

  <rows>
    <row>
      <button label="Rabbit"/>
      <button label="Elephant"/>
    </row>
    <row>
      <button label="Koala"/>
      <button label="Gorilla"/>
    </row>
  </rows>

</grid>
```

 Two rows and two columns have been added to a grid. Each column is declared with the [column](./column.md) tag. Each column has been given a `flex` attribute. Each row contains two elements, both buttons. The first element in each [row](./row.md) element is placed in the first column of the grid and the second element is each row is placed in the second column. Note that you do not need an element to declare a cell -- there is no equivalent of the HTML td element. Instead, you put the contents of cells directly in the [row](./row.md) elements.

## Grid with more elements

You can use any element in place of the [button](./button.md) element of course. If you wanted one particular cell to contain multiple elements, you can use a nested [hbox](./hbox.md) or other box element. An [hbox](./hbox.md) is a single element but you can put as many elements that you want inside it. For example:

 - Example 2
 - [Source](./source/ex_grids_2.xul)

![ex_grids_2](./images/ex_grids_2.png)

```xml
<grid flex="1">

  <columns>
    <column/>
    <column flex="1"/>
  </columns>

  <rows>
    <row>
      <label control="doctitle" value="Document Title:"/>
      <textbox id="doctitle" flex="1"/>
    </row>
    <row>
      <label control="docpath" value="Path:"/>
      <hbox flex="1">
        <textbox id="docpath" flex="1"/>
        <button label="Browse..."/>
      </hbox>   
    </row>
  </rows>

</grid>
```

Notice in the image how the first column of elements containing the labels has only a single element in each row. The second column contains a box in its second row, which in turn contains two elements, a [textbox](./textbox.md) and a [button](./button.md). You could add additional nested boxes or even another grid inside a single cell.

If you resize the window of the last example, you will see that the textboxes resize, but no other elements do. This is because of the `flex` attributes added to the textboxes and the second column. The first column does not need to be flexible as the labels do not need to change size.

The initial width of a column is determined by the largest element in the column. Similarly, the height of a row is determined by the size of the elements in a row. You can use the `minwidth` and `maxwidth` and related attributes to further define the size.

## Column based organization

You can also place the elements inside the [column](./column.md) elements instead of the rows. If you do this, the rows are just declared to specify how many rows there are.

 - Example 3
 - [Source](./source/ex_grids_3.xul)

![ex_grids_3](./images/ex_grids_3.png)

```xml
<grid>
  <rows>
    <row/>
    <row/>
    <row/>
  </rows>

  <columns>
    <column>
      <label control="first" value="First Name:"/>
      <label control="middle" value="Middle Name:"/>
      <label control="last" value="Last Name:"/>
    </column>
    <column>
      <textbox id="first"/>
      <textbox id="middle"/>
      <textbox id="last"/>
    </column>
  </columns>

</grid>
```

This grid has three rows and two columns. The [row](./row.md) elements are just placeholders to specify how many there are. You may add the `flex` attribute to a row to make it flexible. The content is placed inside in each column. The first element inside each [column](./column.md) element is placed in the first row, the second element in the second row and the third element is placed in the third row.

If you put content in both the columns and the rows, the content will overlap each other, although they will align in a grid properly. It creates an effect much like a grid of [stack](./stack.md) elements.

The order of the elements in the [grid](./grid.md) determines which is displayed on top and which is placed underneath. If the `rows` element is placed after the columns element, the content within the rows is displayed on top. If the `columns` element is placed after the `rows` element, the content within the columns is displayed on top. Similarly, events such as mouse buttons and keypresses are sent only to the set on top. This is why the columns were declared after the rows in the above example. If the columns has been placed first, the rows would have grabbed the events and you would not be able to type into the fields.

## Flexibility of grids

One advantage that grids have over a set of nested boxes is that you can create cells that are flexible both horizontally and vertically. You can do this by putting a `flex` attribute on both a row and a column. The following example demonstrates this:

 - Example 4
 - [Source](./source/ex_grids_4.xul)

![ex_grids_4](./images/ex_grids_4.png)


```xml
<grid flex="1">
 <columns>
  <column flex="5"/>
  <column/>
  <column/>
 </columns>
 <rows>
  <row flex="10">
    <button label="Cherry"/>
    <button label="Lemon"/>
    <button label="Grape"/>
  </row>
  <row flex="1">
    <button label="Strawberry"/>
    <button label="Raspberry"/>
    <button label="Peach"/>
  </row>
 </rows>
</grid>
```

The first column and both rows have been made flexible. This will result in every cell in the first column being flexible horizontally. In addition, every cell will be flexible vertically because both rows are flexible, although the first row is more so. The cell in the first column and first row (the Cherry button) will be flexible by a factor of 5 horizontally and flexible by a factor of 10 vertically. The next cell, (Lemon) will only be flexible vertically.

The `flex` attribute has also been added to the `grid` element so that the entire grid is flexible, otherwise it would only grow in one direction.

# Column Spanning

There is no means of making a cell span a particular number of multiple columns or rows (See Discussion for a way of achieving the same effect). However, it is possible to make a row or column that spans the entire width or height of the grid. To do this, just add an element inside the `rows` element that isn't inside a row element. You can use a box type for example, and put other elements inside it if you want to use several elements. Here is a simple example:


 - Example 5
 - [Source](./source/ex_grids_5.xul)

![ex_grids_5](./images/ex_grids_5.png)

```xml
<grid>
  <columns>
    <column flex="1"/>
    <column flex="1"/>
  </columns>

  <rows>
    <row>
      <label value="Northwest"/>
      <label value="Northeast"/>
    </row>
    <button label="Equator"/>
    <row>
      <label value="Southwest"/>
      <label value="Southeast"/>
    </row>
  </rows>
</grid>
```

The button will stretch to fit the entire width of the grid as it is not inside a grid row. You can use a similar technique to add an element in-between two columns. It would stretch to fit the height of the grid. You could also do both if that is desired.

# REF
 
 - [XUL-Grids](https://developer.mozilla.org/en-US/docs/Archive/Mozilla/XUL/Tutorial/Grids)

