# グループボックス

HTML では、関連する要素をグループ化するために使用可能な、`fieldset` 要素が提供されています。**【訳注: HTML 4.0 以降です】** 多くの場合には、グループ化された要素が関連していることを示すために周囲に境界線が描画されます。 具体例をあげるとチェックボックスをグループ化するような場合に使用されます。 XUL でも、同等な要素として [groupbox](./groupbox.md) を提供しており、やはり同じような目的のために使用可能です。

その名前が示しているように、[groupbox](./groupbox.md) はボックスの一種です。 このため、内部の要素はボックスの規則に従って配置されます。 グループボックスと通常のボックスには 2 つ違いがあります。

 - デフォルトでグループボックスの周囲には、(ベベルのついた) 溝状スタイルの境界線が描画されます。この振る舞いは、CSS でスタイルを変えることによって変更できます。
 - グループボックスには、キャプション([caption](./caption.md))を設定できます。これは境界線の上辺にそって置かれます。

ボックスの一種なので、グループボックスでも `orient` や `flex` といったボックスと同じ属性を使うことができます。 大抵の場合、グループボックスの内部には、何らかの形で関連性があるものを置きますが、必要ならばどんな要素を置いてもかまいません。

グループボックスの上辺にかかって表示されるラベルは、[caption](./caption.md) 要素を使って作ることができます。 これは、HTML の legend 要素と同じような動作をします。 [caption](./caption.md) 要素は、グループ内の最初の子要素として、1 つだけ置いておけば十分です。

## 単純なグループボックスの例

以下の例は、単純なグループボックスを示しています。

 - 例 1 
 - [ソース](./source/ex_titledbox_1.xul)

 ![ex_titledbox_1](./images/ex_titledbox_1.png)


```xml
<groupbox>
  <caption label="Answer"/>
  <label value="Banana"/>
  <label value="Tangerine"/>
  <label value="Phone Booth"/>
  <label value="Kiwi"/>
</groupbox>
```

この例では、4 つのテキストが Answer というラベルをもつボックス内に表示されます。 デフォルトではグループボックスは垂直配置であることに注意して下さい。 垂直配置は、テキスト要素を 1 つのカラムに並べていくような場合に必要です。

## より複雑なキャプション

より複雑なキャプションを設定するために、[caption](./caption.md) 要素を、開始と終了タグで構成して、間に子要素を加えることも可能です。 例えば、Mozilla のフォント設定パネルでは、キャプションとしてドロップダウンメニューを使っています。 キャプションとして、どんな要素でも使用することができますが、チェックボックスかドロップダウンメニューのどちらか使う場合が、ほとんどだと思います。

 - 例 2 
 - [ソース](./source/ex_titledbox_2.xul)

![ex_titledbox_2](./images/ex_titledbox_2.png)

```xml
<groupbox flex="1">
  <caption>
    <checkbox label="Enable Backups"/>
  </caption>
  <hbox>
    <label control="dir" value="Directory:"/>
    <textbox id="dir" flex="1"/>
  </hbox>
  <checkbox label="Compress archived files"/>
</groupbox>
```

この例では、キャプションとしてチェックボックスが使われています。 このチェックボックスのチェック状態が変更されるのに応じて、グループボックスに含まれるコンテントの有効と無効を切り替えるために、スクリプトが使用されるかもしれません。 グループボックスにはラベルとテキスト入力欄をもつ水平ボックスが置かれています。 ウィンドウが広げられたときにテキスト入力欄が広がるために、テキスト入力欄とグループボックスの両方が伸縮可能になっています。 グループボックスは垂直配置であるため、後ろのチェックボックスはテキスト入力欄の下に表示されます。 なお、次のセクションでは、ファイル検索ダイアログにグループボックスを加えてみる予定です。

# ラジオグループ

[radio](./radio.md) 要素をグループ化するために、[radiogroup](./radiogroup.md) 要素を使用します。 [radiogroup](./radiogroup.md) も、ボックスの一種です。 グループの内部には、どんな要素でも置くことができ、 ラジオボタンのための特殊な処理を除けば、他種類のボックスと同じように動作します。

ラジオグループの内部に置かれたラジオボタンは、ネストしたボックスの中にあるものも含めて、全てグループ化されます。 これは、次の例に示すように、ラジオグループの構造の中にラジオボタン以外の要素を加える場合に利用できます。

 - 例 3
 - [ソース](./source/ex_titledbox_3.xul)

![ex_titledbox_3](./images/ex_titledbox_3.png)

```xml
<radiogroup>
  <radio id="no" value="no" label="No Number"/>
  <radio id="random" value="random" label="Random Number"/>
  <hbox>
    <radio id="specify" value="specify" label="Specify Number:"/>
    <textbox id="specificnumber"/>
  </hbox>
</radiogroup>
```

[radiogroup](./radiogroup.md) 要素では境界線は描画されないことを注記しておきます。 もし、キャプションや境界線が必要な場合は、[グループボックス](./groupbox.md)で囲うようにしてください。

# REF

 - [XUL-Groupboxes](https://developer.mozilla.org/en-US/docs/Archive/Mozilla/XUL/Tutorial/Groupboxes)
