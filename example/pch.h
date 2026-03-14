#pragma once

#include <minivg.hpp>
#include <vector>

#if defined(_MSC_VER)
    #if defined _M_IX86
        #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #elif defined _M_IA64
        #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #elif defined _M_X64
        #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #else
        #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #endif
#endif

//
// 事件处理函数
//

void key_push(int key);                    // 按键按下
void key_pop(int key);                     // 按键弹起
void input(int key);                       // 字符输入
void mouse_push(int x, int y, int button); // 鼠标按下
void mouse_pop(int x, int y, int button);  // 鼠标弹起
void mouse_move(int x, int y, int button); // 鼠标移动
void display();                            // 显示
