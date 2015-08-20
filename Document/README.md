## LongUI Doc

### LongUI Common Resource
  common resource in LongUI all start-index is 1, and 0 index is default/retained data include:
  - bitmap in `ID2D1Bitmap1*`, use `LongUI::CUIManager::GetBitmap` to get, it will add ref-count
  - brush in `ID2D1Brush*`, use `LongUI::CUIManager::GetBrush` to get, it will add ref-count
  - text-format/font in `IDWriteTextFormat*`, use `LongUI::CUIManager::GetTextFormat` to get, it will add ref-count
  - meta in `LongUI::Meta`, use `LongUI::CUIManager::GetMeta` to get, it **wont't** add ref-count
  - (**unrecommended**)meta-hicon in `HICON`, use `LongUI::CUIManager::GetMetaHICON` to get
  - xml-node in `pugi::xml_node`, you counld not get it because of it used in **CreateControl**
  
### Index Zero Resource
  more detail to see `LongUI::CUIManager::create_indexzero_resources`
  - bitmap: map-able `LongUIDefaultBitmapSize` x `LongUIDefaultBitmapSize` size bitmap
  - brush: common solid-color-brush in `ID2D1SolidColorBrush*  LongUI::UIControl::m_pBrush_SetBeforeUse`
  - text-format: default text format via `LongUI::LongUIDefaultTextFontName` and `LongUI::LongUIDefaultTextFontSize`
  - meta, none
  - meta-hicon, none
  - xml-node, none
  
  
[xml-attributes.md](./xml-attributes.md)