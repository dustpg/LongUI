## LongUI 文档

### LongUI 公共资源
  LongUI 中所有公共资源索引从1开头的, 索引0是默认/保留的数据, 包括:
  - 位图(`ID2D1Bitmap1*`), 用`LongUI::CUIManager::GetBitmap` 获取, 会增加引用计数
  - 笔刷(`ID2D1Brush*`), 用`LongUI::CUIManager::GetBrush`获取, 会增加引用计数
  - 文本格式/字体(`IDWriteTextFormat*`),用`LongUI::CUIManager::GetTextFormat`获取, 会增加引用计数
  - 图元(`LongUI::Meta`), 用`LongUI::CUIManager::GetMeta`获取, 不会增加引用计数
  - (**不建议**)图元图标句柄(`HICON`), 用`LongUI::CUIManager::GetMetaHICON`获取, 不会增加引用计数
  - Xml节点(`pugi::xml_node`), 无法获取, 因为用在"模板id"
  
### 索引0资源
  更多细节查看`LongUI::CUIManager::create_indexzero_resources`
  - 位图: 一个可以map的`LongUIDefaultBitmapSize` x `LongUIDefaultBitmapSize`大小的位图
  - 笔刷: 公共纯色笔刷 `ID2D1SolidColorBrush*  LongUI::UIControl::m_pBrush_SetBeforeUse`
  - 文本格式: 使用 `LongUI::LongUIDefaultTextFontName`, `LongUI::LongUIDefaultTextFontSize`控制的默认格式
  - 图元, **无**
  - 图元图标句柄, **无**
  - Xml节点, **无**
  
### LongUI 公共笔刷
为了让公共笔刷渲染正确, 必须遵守以下规则:
  - 在高度方向**使用1.0作为单位**, 比如:
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
  - 更多细节 `LongUI::FillRectWithCommonBrush`
  
  