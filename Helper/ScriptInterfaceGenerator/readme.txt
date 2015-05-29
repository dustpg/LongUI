[汉语版请往后翻]
This generator can generate c++ code to blind the script to LongUI app.

in this generator, there are two kinds of scripts, one is you wanted, 
"blind" script, defaulty, impl 2 language: mruby and lua. if you want
to blind other language, you should impl some interface by youself, 
reference the default imp.

and another script, I call it "InterfaceScript", it is a very easy
script like C++, even with called "script". defaulty, you may not
care about it, if you want to blind other method or impl some custom 
control and want to blind to the script, I think you should impl it.

                                MUST SPACE
                                    |
define class:                       |
    [// COMMENT]                    |
    class FULLCLASSNAME[[ALIASNAME]] : SUPERLASSNAME {
        .....
        [Method Zone]
        .....
    };
** if no superclass, just set void
  
define method:
    [// COMMENT]
    [static] RETYPE METHODNAME[[ALIASNAME]](PARAMLIST);

About PARAMLIST:
    No const(and so on), No ref(&), all int32_t, uint32_t...
    is int, just easy, NO SPACE with type,
    hard to deal with space by regex

Example:
    // ABCD
    class MyCo : void {
       // Test
       static void Test(void* a);
       void Test2[test2](void* a);
    };
    class TestContainer[TeCont] : LongUI::UIContainer {
       static void Test[test](void* a);
       // Test2
       void Test2(void* a);
    };

  
这个生成器能够生成C++代码用于绑定 LongUI 应用与脚本语言.

在这个生成器里面, 分为两类脚本, 一种是您想要绑定的脚本语言, 默认提供了
2种语言: mruby 和 lua.如果您想绑定其他脚本语言, 请自己实现, 默认提供的
将成为不错的参照

还有一种就是自己称为"InterfaceScript"的脚本, 它的语法非常简单并且类似于
C++, 并且简单到不能称之为"脚本"了. 默认情况下, 您无需在意. 如果您需要链
接到其他方法(默认提供的可能不够)或者说实现了自定义的控件并且想要绑定到脚
本, 这时您可能就需要了解它了:

                       必须有空格
                          |
定义一个类:               |
    [// 注释]             |
    class 类全名[[类别名]] : 父类全名 {
        .....
        [方法区]
        .....
    };
没有父类就填写void

定义一个方法:
    [// 注释]
    [static] 返回类型 方法名称[[方法别名]](参数列表);

关于参数列表:
    没有const等限定词, 没有引用(&), 所有整形都是int, 
    简单就好. 除非真的很关心位数或者符号, 还有就是想
    利用size_t储存指针数据, 还是那样, 请自己实现.
    还有就是类型名不要带空格, 用正则表达式很难处理

例子: 参考上面的英文版的末尾