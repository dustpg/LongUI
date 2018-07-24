**property**

  0. Instructions
  - [O]basic support 基本支持
  - [-]partial support 部分支持
  - [X]unsupport 不支持
  - remarks: unsupport inherited property 不支持属性继承

  1. Position
  - [O]overflow { x [y] }
  - [O]overflow-x { auto hidden scroll visible }
  - [O]overflow-y { auto hidden scroll visible }
  - [-]left { ??px }
  - [-]top { ??px }

  2. Dimension
  - [-]width { ??px }
  - [-]height { ??px }
  - [-]min-width { ??px }
  - [-]min-height { ??px }
  - [-]max-width { ??px }
  - [-]max-height { ??px }

  3. Margin Padding Border
  - [O]margin { x [y] [z] [w] }
  - [-]margin-top { ??px }
  - [-]margin-right { ??px }
  - [-]margin-bottom { ??px }
  - [-]margin-left { ??px }
  - [O]padding { x [y] [z] [w] }
  - [-]padding-top { ??px }
  - [-]padding-right { ??px }
  - [-]padding-bottom { ??px }
  - [-]padding-left { ??px }
  - [X]border
  - [O]border-width { x [y] [z] [w] }
  - [-]border-top-width { ??px }
  - [-]border-right-width { ??px }
  - [-]border-bottom-width { ??px }
  - [-]border-left-width { ??px }
  - [-]border-style { solid }
  - [X]border-*-style
  - [-]border-color { *color format* }
  - [X]border-*-color
  - [-]border-radius { ??px [ / ??px] }
  - [X]border-*-radius
  - [X]border-image
  - [-]border-image-source { url(???) (only one) }
  - [O]border-image-slice { ?? ??% [y] [z] [w] && fill? }
  - [X]border-image-width
  - [X]border-image-outset
  - [O]border-image-repeat { stretch repeat round [y] }

  4. Background
  - [X]background
  - [O]background-color { *color format* }
  - [-]background-image { url(???) (only one) }
  - [O]background-clip { border-box padding-box content-box }
  - [O]background-origin { border-box padding-box content-box }
  - [O]background-repeat { repeat-x repeat-y|repeat round no-repeat [y] }
  - [X]background-position
  - [X]background-size

  5. Text&Font
  - [O]color { *color format* }
  - [-]text-align { start end center justify left right  }
  - [O]-webkit-text-stroke { ??px *color format* }
  - [-]-webkit-text-stroke-width { ??px }
  - [O]-webkit-text-stroke-color { ??px *color format* }
  - [X]font
  - [-]font-size { ??px }
  - [O]font-style { normal italic oblique }
  - [O]font-stretch { normal ultra-condensed~ultra-expanded  }
  - [O]font-weight { normal bold 100~900 }
  - [-]font-family { ?? (only one) }

  6. Transition&Animation
  - [O]transition-duration { ?s ?ms (0.0s~65.535s, only one) }
  - [-]transition-timing-function { linear ease ease-in ease-out ease-in-out AT* }
  - [X]transition-property

  7. LongUI
  - [-]-moz-appearance { none }

**color format**
  - color name, e.g: red
  - transparent
  - rebeccapurple
  - #rgb
  - #rgba
  - #rrggbbaa
  - #rrggbbaa
  
**AT\***
  - LongUI extension
  - id for AnimationType
  - e.g. 26 for AnimationType::Type_BackEaseOut
  - remarks: "ease" function just one of AnimationType, not a real cubic-bezier

---

  **grammar**

  0. Precedence
  - no special precedence, post item will overwrite the previous item, inline style is "high-precedence"
  - 没有特殊的优先级. 后项会覆盖前项, 内联样式拥有"最高优先级"

  1. Selector
  - type selector: button
  - class selector: .win10-button
  - id selector: #button0
  - universal selector: *
  - **remarks**: becareful on "type selector" with "c++ inheritance class": "type" is control now type, has no relations with super/base classes. [UIButton based on UIBoxLayout, but "box" won't match "button". in other words, if you implement MyButton based on UIButton, "button" won't match your control]
  - **注**: 小心"类型选择器"与"C++继承类", "类型"是指控件的当前类型, 和它的基类/超类无关. [比如UIButton继承于UIBoxLayout, 但是"box"是匹配不了"button"的. 换句话说, 您继承于UIButton实现了MyButton, "button"是匹配不到您的控件的]

  2. Combinators
  - adjacent sibling combinator: A + B
  - general  sibling combinator: A ~ B
  - child            combinator: A > B
  - descendant       combinator: A   B
  - **remarks**: won't modify style on relationship changed
  - **注**: 当关系改变时不会修改样式

  3. Pseudo-classes
  - **css base:** :active :checked :default :disabled :enabled :focus :hover :indeterminate
  - **longui ex:** :selected :closed *and more*
  - **remarks**: pseudo-classes as state on LongUI, (state) animation based on this. 
  - **limit**: one selector-chain has **zero or one** pseudo-class only 
  - **注**: 伪类是作为状态存在于LongUI中的, (状态)动画是基于这个的实现的. 
  - **限制**: 一条选择器链只有至多有一个伪类选择器.