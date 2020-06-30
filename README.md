# Fresh-Editor-Qt-based-cross-platform-code-editor
Based on Qt 5.14.0
基于Qt 5.14.0的跨平台代码编辑器 

## 项目需求/Project Requirement：
![project requirement](https://github.com/ChowKing161/Fresh-Editor-Qt-based-cross-platform-code-editor-/blob/master/requirement.png?raw=true)

## Implemented Function/实现功能
支持Linux和Windows系统
具备基本的代码编辑器功能，目前支持C++语法高亮及代码补全，C++代码编译运行，向前查找及向后查找并替换，支持ANSI和UTF-8编码
function: SyntaxHiglighter,code completer(C++ only,of coure you can fork and implment some other languages),compile and run, find and replace, ANSI and UTF-8 coding format(but has a little bug)

## User Interface/用户界面:
![running interface](https://github.com/ChowKing161/Fresh-Editor-Qt-based-cross-platform-code-editor-/blob/master/interface.png?raw=true)

## Other/其他
关于编辑器的运行功能，如果在Linux下使用编辑器的运行功能，需要修改mainwindow.cpp的run函数下的代码。运行功能实现的方法是用Qprocess调用cmd然后调用g++来实现的，因此Windows需要提前安装配置好g++。
About the editor's running function, if you use the editor's running function under Linux, you need to modify few codes in mainwindow.cpp's run function. The way to run the c++ code is to use Qprocess to call CMD and then call g++, so Windows needs to install g++ in advance.

## Bugs/问题
如果有任何问题可以邮箱联系：chowking161@163.com 或者 直接在GitHub发issue。
Any bugs or problems,please cotact me at chowking161@163.com or file an issue on GitHub.

## Thanks/致谢
[m-iDev-0792](https://github.com/m-iDev-0792)
my colleague Guan Qin & Yilin Wang
