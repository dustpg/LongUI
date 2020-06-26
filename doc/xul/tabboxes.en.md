It is common in preference dialogs for tabbed pages to appear. We'll find out how to create them here.

# Tabboxes

Tabboxes are typically used in an application in the preferences window. A series of tabs appears across the top of a window. The user can click each tab to see a different set of options. It is useful in cases when you have more options than will fit in one screen.

XUL provides a method to create such dialogs. It involves five new elements, which are described briefly here and in more detail below.

 - [tabbox](./tabbox.md) The outer box that contains the tabs along the top and the tab pages themselves.
 - [tabs](./tabs.md) The inner box that contains the individual tabs. In other words, this is the row of tabs.
 - [tab](./tab.md) A specific tab. Clicking on the tab brings the tab page to the front.
 - [tabpanels](./tabpanels.md) The container for the pages.
 - [tabpanel](./tabpanels.md) The body of a single page. You would place the content for a page within this element. The first tabpanel corresponds to the first tab, the second tabpanel corresponds to the second tab and so on.

 The `tabbox` is the outer element. It consists of two children, a `tabs` element which contains the row of tabs and a `tabpanels` elements which contains the tabbed pages.

 Shown below is the general syntax of a tabbox:

 ```xml
 <tabbox id="tablist">
  <tabs>
    <!-- tab elements go here -->
  </tabs>
  <tabpanels>
    <!-- tabpanel elements go here -->
  </tabpanels>
</tabbox>
```

The `tab` elements are placed inside a `tabs` element, which is much like a regular box. The `tabs` element itself has been placed inside a `tabbox`. The `tabbox` also contains a `tabpanels` element which will appear below the `tabs` due to the vertical orientation on the whole tabbox.

There is really nothing special about the tab elements that make them different than boxes. Like boxes, tabs can contain any element. The difference is that the tabs render slightly differently and only one tab panel's contents are visible at once, much like a `deck`.

The contents of the individual tab pages should go inside each `tabpanel` element. They do not go in the `tab` elements, as that is where the contents of the tabs along the top go.

Each `tabpanel` element becomes a page on the tabbed display. The first panel corresponds to the first tab, the second panel corresponds to the second tab, and so on. There is a one-to-one relationship between the `tab` and `tabpanel` elements.

When determining the size of the `tabbox`, the size of the largest page is used. That means that if there are ten textboxes on one tab page and only one on another, the tab page will be sized to fit the one with the ten on it as this takes up more room. The area taken up by the tab area does not change when the user switches to a new tab page.

# Tabbox example

 - Example 1
 - 

# REF 

 - [XUL-Tabboxes](https://developer.mozilla.org/en-US/docs/Archive/Mozilla/XUL/Tutorial/Tabboxes)
