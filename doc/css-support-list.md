**property**

  0. Instructions
  - [O]basic support
  - [-]partial support
  - [X]unsupport
  1. Position
  - [O]overflow { x [y] }
  - [O]overflow-x { auto hidden scroll visible }
  - [O]overflow-y { auto hidden scroll visible }
  2. Margin Padding Border
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
  3. Background
  - [X]background
  - [O]background-color { *color format* }
  - [-]background-image { url(???) (only one) }
  - [O]background-clip { border-box padding-box content-box }
  - [O]background-origin { border-box padding-box content-box }
  - [O]background-repeat { repeat-x repeat-y|repeat round no-repeat [y] }
  - [X]background-position
  - [X]background-size
  4. Text&Font
  - [O]color { *color format* }
  - [X]font
  - [-]font-size { ??px }
  - [O]font-style { normal italic oblique }
  - [O]font-stretch { normal ultra-condensed~ultra-expanded  }
  - [O]font-weight { normal bold 100~900 }
  - [-]font-family { ?? (only one) }
  5. Transition&Animation
  - [O]transition-duration { ?s ?ms (0.0s~65.535s) }
  - [-]transition-timing-function { linear ease ease-in ease-out ease-in-out AT* }
  6. LongUI
  - [-]-moz-appearance { none }

**color format**
  - color name, e.g: red
  - transparent
  - rebeccapurple
  - #rgb
  - #rgba
  - #rrggbbaa
  - #rrggbbaa
  
