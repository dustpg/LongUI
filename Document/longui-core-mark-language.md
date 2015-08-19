## LongUI Core Mark Langeuage v0.1

`LCML` is a easy to use but you may need a doc.
`LCML` could be used in `C++ Function` or `Inline Paramters` mode.

## C++ Function
`LCML` like `sprintf` in C, that could be use `va_list` to format text to rich.  

like `sprintf`, `LCML` use `%` as marker start but **only one** control char after `%`
(easy to code it).  
  
control char|summary|c++ type|note
:----------:|-------|--------|----
`%%`|`%` char|\[none\]|like `%%` in `sprintf`, as a '%' char
`%a`, `%A`|`a`dd string|`const wchar_t *`|like `%ls` in `sprintf`, Add a `const wchar_t *` string to source
`%]`, `%}`|marker-end|\[none\]|mark end of a "rich marker"
----------|Rich Maker|-------|---
`%c`|`c`olor|`uint32_t`|set color(`uint32_t` in ARGB) to this range, see `LongUI::UnpackTheColorARGB`
`%C`|`c`olor|`D2D1_COLOR_F*`|set color(`D2D1_COLOR_F`) to this range
`%s`, `%S`|font `s`ize|`float`|set font size to this tange
`%n`, `%N`|family `n`ame|`const wchar_t *`|set family name to this range
`%h`, `%H`|font stretc`h`|`DWRITE_FONT_STRETCH`|set font stretch to this range
`%y`, `%Y`|font st`y`le|`DWRITE_FONT_STYLE`|set font style to this range
`%w`, `%W`|font `w`eight|`DWRITE_FONT_WEIGHT`|set font weight to this range
`%u`, `%U`|`u`nderline|`BOOL`|32-bit`BOOL`, not `bool`, set underline to this range
`%t`, `%T`|strike`t`hrough|`BOOL`|32-bit`BOOL`, not `bool`, set strikethrough to this range
`%i`, `%I`|`i`nline object|`IDWriteInlineObject*`, `CUIInlineObject*`|set inline object to this range
`%d`, `%D`|`d`raw effect|`IUnknown*`|set drawing effect to this range


## Demo in [Show1_LCML](../demos/Show1_LCML)
```cpp
  FormatTextConfig config;
  // set config
  auto layout1 = LongUI::DX::FormatTextCoreC(config, L"Text");
  auto layout2 = LongUI::DX::FormatTextCoreC(config, L"%cHello%], world!", uint32_t(0xFFFF0000));
  auto layout3 = LongUI::DX::FormatTextCoreC();
  auto layout4 = LongUI::DX::FormatTextCoreC();
```

## Inline Paramters
`LCML` support use "inline string" as param to format text to rich.  
like `"%cHello%], world!%p#FFFF0000"` vs `LongUI::DX::FormatTextCoreC(config, L"%cHello%], world!", uint32_t(0xFFFF0000));`  
  

#### Note for `FormatTextConfig`
```cpp
  struct FormatTextConfig {
    // basic text format
    IN  IDWriteTextFormat*      format;
    // text layout width
    IN  float                   width;
    // text layout hright
    IN  float                   height;
    // make the text showing progress, maybe you want
    // a "typing-effect", set 1.0f to show all, 0.0f to hide
    IN  float                   progress;
    // format for this
    IN  RichType                rich_type;
    // the text real(without format) length
    OUT mutable uint16_t        text_length;
};
```