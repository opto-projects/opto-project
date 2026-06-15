Language - English
Qt Solutions Component: Property Browser

A property browser framework enabling the user to edit a set of
properties.

The framework provides a browser widget that displays the given
properties with labels and corresponding editing widgets (e.g.
line edits or comboboxes). The various types of editing widgets
are provided by the framework's editor factories: For each
property type, the framework provides a property manager (e.g.
QtIntPropertyManager and QtStringPropertyManager) which can be
associated with the preferred editor factory (e.g.
QtSpinBoxFactory and QtLineEditFactory). The framework also
provides a variant based property type with corresponding variant
manager and factory. Finally, the framework provides three
ready-made implementations of the browser widget:
QtTreePropertyBrowser, QtButtonPropertyBrowser and
QtGroupBoxPropertyBrowser.

Version history:

2.1: - QtTreePropertyBrowser - tooltip of property applied to
     first column, while second column shows the value text of property
     in its tooltip
     - QtAbstractPropertyManager - initializeProperty() and
     uninitializeProperty() without const modifier now
     - QtTreePropertyBrowser and QtGroupBoxPropertyBrowser - internal
     margin set to 0
     - QtProperty - setEnabled() and isEnabled() methods added
     - QtTreePropertyBrowser - "rootIsDecorated", "indentation" and
     "headerVisible" properties added
     - QtProperty - hasValue() method added, useful for group
     properties

2.2: - FocusOut event now filtered out in case of
     Qt::ActiveWindowFocusReason reason. In that case editor is not
     closed when its sub dialog is executed
     - Removed bug in color icon generation
     - Decimals attribute added to "double" property type
     - PointF, SizeF and RectF types supported
     - Proper translation calls for tree property browser
     - QtProperty - ensure inserted subproperty is different from
     "this" property
     - QtBrowserItem class introduced, useful for identifying browser's
     gui elements
     - Possibility to control expanded state of QtTreePropertyBrowser's
     items from code
     - QtTreePropertyBrowser - "resizeMode" and "splitterPosition"
     properties added
     - QtGroupBoxPropertyBrowser - fixed crash in case of deleting the
     editor factory and then deleting the manager
     - "Decoration" example added - it shows how to add new
     responsibilities to the existing managers and editor factories

2.3: - Various bugfixes and improvements
     - QtProperty - setModified() and isModified() methods added
     - QtTreePropertyBrowser - disabling an item closes its editor
     - KeySequence, Char, Locale and Cursor types supported
     - Support for icons in enum type added
     - Kerning subproperty exposed in Font type
     - New property browser class added - QtButtonPropertyBrowser with
     drop down button as a grouping element

2.4: - Fixed memory leak of QtProperty
     - QtTreePropertyBrowser - group items are rendered better
     - QtTreePropertyBrowser - propertiesWithoutValueMarked and
     alternatingRowColors features added
     - QtTreePropertyBrowser - possibility of coloring properties added
     - QtTreePropertyBrowser - keyboard navigation improved
     - New factories providing popup dialogs added:
     QtColorEditorFactory and QtFontEditorFactory
     - Single step attribute added to: QtIntPropertyManager and
     QtDoublePropertyManager

2.5: - "Object Controller" example added. It implements a similar
     widget to the property editor in QDesigner
     - Compile with QT_NO_CURSOR
     - Expand root item with single click on the '+' icon
     - QtRectPropertyManager and QtRectFPropertyManager - by default
     constraint is null rect meaning no constraint is applied

2.6: - QtGroupPropertyBrowser - don't force the layout to show the
     whole labels' contents for read only properties, show tooltips for
     them in addition.
     - QtTreePropertyBrowser - fixed painting of the editor for color
     property type when style sheet is used (QTSOLBUG-64).
     - Make it possible to change the style of the checkboxes with a
     stylesheet (QTSOLBUG-61).
     - Change the minimum size of a combobox so that it can show at
     least one character and an icon.
     - Make it possible to properly style custom embedded editors (e.g.
     the color editor provided with the solution).
     
     
     
Language - 中文
     
Qt解决方案组件：属性浏览器
    
    属性浏览器框架，允许用户编辑一组属性。该框架提供了一个浏览器小部件，用于显示给定的带有标签和相应编辑小部件的属性（例如。行编辑或组合框）。各种类型的编辑小部件由框架的编辑器工厂提供：对于每个属性类型，框架提供属性管理器（例如。QtIntPropertyManager和QtStringPropertyManager），可以与首选编辑器工厂相关（例如。QtSpinBoxFactory和QtLineEditFactory）。该框架也提供基于变量的属性类型和相应的变量经理和工厂。最后，该框架提供了三个
浏览器小部件的现成实现：
QtTreePropertyBrowser、QtButtonPropertyBrowser和QtGroupBoxPropertyBrowser。

版本历史记录：
2.1:- QtTreePropertyBrowser-应用于的属性的工具提示
第一列，第二列显示属性的值文本
在其工具提示中
- QtAbstractPropertyManager-initializeProperty()和现在不带常量修饰符的uninitializeProperty()
- QtTreePropertyBrowser和QtGroupBoxPropertyBrowser-内部边距设置为0
- 添加了QtProperty-setEnabled()和isEnabled()方法
- QtTreePropertyBrowser-“rootIsDecorated”，“indentation”和添加了“headervible”属性
- 添加了QtProperty-hasValue()方法，对组有用
属性

2.2:- 在以下情况下，FocusOut事件现在被过滤掉：
Qt::ActiveWindowFocusReason原因。在这种情况下，编辑器不是执行其子对话框时关闭
- 删除了颜色图标生成中的错误
- 添加到“double”属性类型的小数属性
- 支持PointF、SizeF和RectF类型
- 正确的翻译需要树属性浏览器
- QtProperty-确保插入的子属性不同于“this”属性
- 引入了QtBrowserItem类，用于识别浏览器的gui元素
- 控制QtTreePropertyBrowser的扩展状态的可能性代码中的项目
- QtTreePropertyBrowser-“resizeMode”和“splitterPosition”添加的属性
- QtGroupBoxPropertyBrowser-修复了删除编辑器工厂，然后删除管理器
- 添加了“装饰”示例-它显示了如何添加新的对现有经理和编辑工厂的责任

2.3:- 各种错误修复和改进
- 添加了QtProperty-setModified()和isModified()方法
- QtTreePropertyBrowser-禁用项目将关闭其编辑器
- 支持的键序列、字符、区域设置和光标类型
- 添加了对枚举类型中图标的支持
- 字型中显示的紧排子属性
- 添加了新的属性浏览器类 - QtButtonPropertyBrowser，带有下拉按钮作为分组元素

2.4:- QtProperty的固定内存泄漏
- QtTreePropertyBrowser-组项目渲染得更好
- QtTreePropertyBrowser-未标记值的属性和添加了alternatingRowColors功能
- QtTreePropertyBrowser-添加着色属性的可能性
- QtTreePropertyBrowser-改进键盘导航
- 添加了提供弹出对话框的新工厂：QtColorEditorFactory和QtFontEditorFactory
- 单步属性添加到：qtinPropertyManager和QtDoublePropertyManager

2.5：增加了“对象控制器”示例。它实现了类似的QDesigner中属性编辑器的小部件
- 使用QT_NO_游标编译
- 单击“+”图标展开根项目
- QtRectPropertyManager和QtRectFPropertyManager-默认情况下约束为null rect，表示未应用任何约束

2.6:- QtGroupPropertyBrowser-不要强制布局显示只读属性的完整标签内容，显示工具提示除此之外。
- QtTreePropertyBrowser-修复了编辑器的颜色绘制使用样式表时的属性类型（QTSOLBUG-64）。
- 可以使用样式表（QTSOLBUG-61）。
- 更改组合框的最小大小，使其可以在至少一个字符和一个图标。
- 可以正确设置自定义嵌入式编辑器的样式（例如。解决方案提供的颜色编辑器）。

