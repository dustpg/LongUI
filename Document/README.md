## LongUI Doc

### LongUI Common Resource
  common resource in LongUI all start-index is 1, and 0 index is default/retained data include:
  - bitmap in `ID2D1Bitmap1*`, use `LongUI::CUIManager::GetBitmap` to get, it will add ref-count
  - brush in `ID2D1Brush*`, use `LongUI::CUIManager::GetBrush` to get, it will add ref-count
  - text-format/font in `IDWriteTextFormat*`, use `LongUI::CUIManager::GetTextFormat` to get, it will add ref-count
  - meta in `LongUI::Meta`, use `LongUI::CUIManager::GetMeta` to get, it **wont't** add ref-count
  - (**unrecommended**)meta-hicon in `HICON`, use `LongUI::CUIManager::GetMetaHICON` to get
  - xml-node in `pugi::xml_node`, you could not get it because of it used for "templateid"
  
### Index Zero Resource
  more detail to see `LongUI::CUIManager::create_indexzero_resources`
  - bitmap: map-able `LongUIDefaultBitmapSize` x `LongUIDefaultBitmapSize` size bitmap
  - brush: common solid-color-brush in `ID2D1SolidColorBrush*  LongUI::UIControl::m_pBrush_SetBeforeUse`
  - text-format: default text format via `LongUI::LongUIDefaultTextFontName` and `LongUI::LongUIDefaultTextFontSize`
  - meta, none
  - meta-hicon, none
  - xml-node, none
  
### LongUI Common Brush
to make rendering common brush in corrent way, The following stipulations applied in LongUI
  - **use 1.0 as unit** in height, like:
```cpp
    // Normal Style Brush
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                ),
            collection,
            reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + Status_Normal)
            );
    }
```
  - more detail see `LongUI::FillRectWithCommonBrush`
  
  
[xml-attributes.md](./xml-attributes.md)