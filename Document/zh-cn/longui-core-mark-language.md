## LongUI Core Mark Langeuage v0.2.1

`LCML` is a easy to use but you may need a doc.
`LCML` could be used on `C++ Function` or `Inline Paramters` mode.

## C++ Function
`LCML` like `sprintf` in C, that could be use `va_list` to format text to rich.  

like `sprintf`, `LCML` use `%` as marker start but **only one** control char after `%`
(easy to code it).  
  
control char|summary|c++ type|note
:----------:|-------|--------|----
`%%`|`%` char|\[none\]|like `%%` in `sprintf`, as a '%' char
`%a`|`a`dd string|`const wchar_t *`|like `%ls` in `sprintf`, Add a `const wchar_t *` string to source
`%]`|marker-end|\[none\]|mark end of a "rich marker"
----------|Rich Maker|-------|---
`%c`|`c`olor|`D2D1_COLOR_F*`|set color(`uint32_t` in ARGB) to this range, see `LongUI::UnpackTheColorARGB`
`%s`|font `s`ize|`float`|set font size to this tange
`%n`|font `f`amily|`const wchar_t *`|set family name to this range
`%h`|font stretc`h`|`DWRITE_FONT_STRETCH`|set font stretch to this range
`%y`|font st`y`le|`DWRITE_FONT_STYLE`|set font style to this range
`%w`|font `w`eight|`DWRITE_FONT_WEIGHT`|set font weight to this range
`%u`|`u`nderline|**NONE**|NONE, just add underline
`%t`|`t`ypography|`IDWriteTypography*`|set typography for this range
`%l`|`l`ocal name|`const wchar_t*`|set local name for this range
`%d`|strikethrough(`d`elete line)|**NONE**|NONE, just add strikethrough/delete line
`%i`|`i`nline object|`IDWriteInlineObject*`, `CUIInlineObject*`|set inline object to this range
`%e`|drawing `e`ffect|`IUnknown*`|set drawing effect to this range


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
like `"%cHello%], world!%p#FFFF0000"` vs 
`auto color = D2D1::ColorF(0xFFFF0000);LongUI::DX::FormatTextCoreC(config, L"%cHello%], world!", &color);`  
  

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