# 北理大二电信小学期IDE项目  

<img width="256" height="256" alt="icon - 副本" src="https://github.com/user-attachments/assets/662b64a0-e2d7-4c1d-a130-81ae3a71e3db" />

<img width="915" height="512" alt="image" src="https://github.com/user-attachments/assets/f61eaaf0-59e2-42ca-a42b-306128eea41c" />

<img width="512" height="512" alt="image" src="https://github.com/user-attachments/assets/6ea3a475-baf8-4499-9c46-4346b418ddeb" />

这款功能全面的C/C++语言IDE集成开发环境工具，依靠QT和C++工具开发，支持多文件编辑，可新建、打开、保存及另存文件；具备全选、复制等文本编辑功能，含查找替换与撤销恢复操作。基于g++编译器，支持单独编译、运行及一键 "编译并运行"，编译失败精准提示错误，成功反馈状态。显示交互上，支持深浅色切换与定制语法高亮，左侧显示行号，可折叠函数 / 循环，左下角实时显示光标行列，还能调整字体大小并支持快捷键。内置可外置移动的调试器，集成DeepSeek模型，对话区能上下文感知沟通，自动加载代码上下文提供AI建议并支持一键插入，基于C++标准库实现智能补全，同时兼容C与C++编译。  

# Beijing Institute of Technology(BIT) Sophomore Electronic Information Engineering Major Summer Session C/C++ IDE Project
This C/C++ IDE offers a complete suite of basic functionalities and extensive advanced features.
Core Editing & File Management:
It provides essential file operations including creating, opening, saving, and "save as" for efficient multi-file project handling. Text editing is robust, featuring standard select all, copy, paste, and cut, alongside find/replace, undo, and redo capabilities.
Integrated Build System:
Leveraging the G++ compiler, the IDE supports seamless compiling, running, and a "compile and run" shortcut. It offers precise error reporting with location and type details upon compilation failure, and clear success feedback.
Enhanced User Experience:
Extended features include customizable light/dark modes and syntax highlighting. Line numbers are displayed, with support for code folding (functions and loop statements). The cursor's real-time row and column position is shown, and font size can be adjusted via shortcut keys.
Advanced Debugging:
A built-in debugger is included, with a detachable and freely movable debugging window for flexible workflow.
AI-Powered Assistance:
Integrated with the DeepSeek model, the AI assistant provides context-aware communication, automatically loading code for intelligent suggestions and one-click insertion.
Smart Code Completion:
Based on C++ standard library knowledge, it dynamically recommends variables, functions, and keywords, optimizing context understanding during input. The IDE is fully compatible with both C and C++ language compilation.

版本介绍:
北理工电信大二小学期
main分支（次分支，已舍弃）
8月30日
构建了第一个IDE基本框架，实现了文本输入，保存，编译，运行，终端界面输出等基本功能
8月31日
实现了关键词识别和高亮功能
9月1日
实现了终端界面的输入功能以及操作后的回复（撤回功能）
9月2日
修复基础的查找功能

main branch分支（主分支）
9月2日：根据资料，构建IDE基本框架，包括主界面，保存与打开功能，复制剪切粘贴全选功能，编译功能和运行功能
9月3日：cmd命令窗支持UTF-8编码，加入代码关键词高亮功能，加入运行并编译功能，加入另存功能
9月4日：添加查找替换功能，添加括号自动匹配功能，修复初始状态换行问题，添加设置字体大小功能
9月5日：添加撤销恢复功能，行号显示功能
9月7日：将主函数中关于编译和运行的相关代码独立成库，添加多文本编辑功能，修复编译并运行的BUG
9月8日：字体大小快捷键，编译报错显示，修复选中行的高亮问题，完成浅色深色主体模式，修复Tab键和行号显示BUG
9月9日：设计图标logo
9月10日：函数和循环语句折叠，代码排版，添加应用标签
9月11日：添加关键词补全和debug调试器功能
9月12日：添加高亮器刷新功能
9月13日：添加deepseek的API接口
