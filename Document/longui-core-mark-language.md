## LongUI Core Mark Langeuage v0.1

Just for easy to code it, `LCML` is a easy to use but you may need a doc.
`LCML` could be used in `C++ Function` or `Inline Param` mode.

## C++ Function
`LCML` like `sprintf` in C, that could be use `va` to format text to rich.  

like `sprintf`, `LCML` use `%` as mark and **only one** control char after `%`
(easy to code it).  
control char|summary|c++ type|note
:----------:|-------|--------|----
`%%`|`%` char|\[none\]|like `%%` in `sprintf`, as a '%' char
`%a`, `%A`|string|`const wchar_t *`|like `%ls` in `sprintf`, add a `const wchar_t *` string to source
`%]`, `%}`|marker-end|\[none\]|mark end of a "rich marker"
:----------:|-------|--------|----
`%C`|color|`D2D1_COLOR_F*`|set color(`D2D1_COLOR_F`) to this tange
`%c`|color|`uint32_t`set color(`uint32_t`, ARGB) to this tange, see `LongUI::UnpackTheColorARGB`
`%s`, `%S`|font size|`float`|set font size to this tange
`%n`, `%N`|font family|`const wchar_t *`|set family name to this tange
`%h`, `%H`|font stretch|`DWRITE_FONT_STRETCH`|set font stretch to this tange
`%y`, `%Y`|font style|`DWRITE_FONT_STYLE`|set font style to this tange
`%w`, `%W`|font weight|`DWRITE_FONT_WEIGHT`|set font weight to this tange
`%u`, `%U`|underline|`BOOL`|32-bit`BOOL`, not `bool`, set underline to this tange
`%t`, `%T`|strikethrough|`BOOL`|32-bit`BOOL`, not `bool`, set strikethrough to this tange
`%i`, `%I`|inline object|`IDWriteInlineObject*`, `CUIInlineObject*`|set inline object to this tange
`%d`, `%D`|drawing effect|`IUnknown*`|set drawing effect to this tange


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
like `L"%cHello%], world!%p#FFFF0000"` vs `LongUI::DX::FormatTextCoreC(config, L"%cHello%], world!", uint32_t(0xFFFF0000));`



## Note for `FormatTextConfig`
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
