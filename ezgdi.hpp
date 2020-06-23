/*
 Copyright (c) 2005-2020 sdragonx (mail:sdragonx@foxmail.com)

 ezgdi.hpp

 2020-01-01 16:37:22

 简单的win32窗口、绘图函数封装

*/
#ifndef EZGDI_HPP
#define EZGDI_HPP

#define NO_WIN32_LEAN_AND_MEAN
#ifndef STRICT
    #define STRICT
#endif

#if defined(__GNUC__)
	#define EZGDI_PUBLIC __attribute__((weak))
#else
	#define EZGDI_PUBLIC __declspec(selectany)
#endif

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <map>
#include <sstream>
#include <string>

typedef void (*EZGDI_KEYEVENT)(int key);
typedef void (*EZGDI_MOUSEEVENT)(int x, int y, int button);

//---------------------------------------------------------------------------
//
// 通用
//
//---------------------------------------------------------------------------

//Unicode字符串类
class ezstring : public std::wstring
{
public:
    ezstring() : std::wstring() { }
    ezstring(const char* str, size_t size = -1) : std::wstring()
    {
        assign(str, size);
    }

    ezstring(int n) : std::wstring()
    {
        std::wstringstream stm;
        stm<<n;
        this->assign(stm.str());
    }

    ezstring(const wchar_t* str) : std::wstring(str) { }
    ezstring(const wchar_t* str, size_t size) : std::wstring(str, size) { }

    using std::wstring::assign;
    void assign(const char* str, size_t size = -1)
    {
        wchar_t *buf;
        int n = MultiByteToWideChar(CP_ACP, 0, str, size, 0, 0);
        buf = new wchar_t[n];
        MultiByteToWideChar(CP_ACP, 0, str, size, buf, n);
        delete buf;
        this->assign(buf, n - 1);
    }
};

//点
struct point_t
{
    float x, y;
};

//矩形
struct rect_t
{
    float x, y, width, height;
};

namespace ezgdi{

//安全删除指针
template<typename T>
void safe_delete(T* &p)
{
    if(p){
        delete p;
        p = NULL;
    }
}

//删除stl容器里面的内容
template<typename T>
void delete_all(T& obj)
{
    typename T::iterator itr = obj.begin();
    for( ; itr != obj.end(); ++itr){
        delete itr->second;
    }
    obj.clear();
}

//---------------------------------------------------------------------------
//
// 图片资源管理类
//
//---------------------------------------------------------------------------

class ImageResource
{
public:
    typedef std::wstring string_t;

public:
    std::map<string_t, Gdiplus::Image*> resource;

    //加载一个图片
    Gdiplus::Image* load_image(const string_t& name)
    {
        Gdiplus::Image* bmp = NULL;
        std::map<string_t, Gdiplus::Image*>::iterator itr;
        itr = resource.find(name);
        if(itr == resource.end()){
            bmp = new Gdiplus::Image(name.c_str());
            resource[name] = bmp;
        }
        else{
            bmp = itr->second;
        }

        return bmp;
    }

    //释放所有图片资源
    void dispose()
    {
        delete_all(resource);
    }
};

//---------------------------------------------------------------------------
//
// EZGDI 实例类
//
//---------------------------------------------------------------------------

template<typename T = int>
class ezInstance
{
public:
    HWND window;                    //主窗口
    HDC  hdc;                       //GDI设备

    EZGDI_KEYEVENT OnKeyDown;
    EZGDI_KEYEVENT OnKeyUp;
    EZGDI_KEYEVENT OnKeyPress;

    EZGDI_MOUSEEVENT OnMouseDown;
    EZGDI_MOUSEEVENT OnMouseUp;
    EZGDI_MOUSEEVENT OnMouseMove;

    Gdiplus::Graphics* graphics;    //
    Gdiplus::Bitmap* colorbuf;      //背景缓冲区，使用双缓冲防止闪烁

    Gdiplus::Pen* pen;
    Gdiplus::SolidBrush* brush;

    Gdiplus::Font* font;
    Gdiplus::SolidBrush* font_color;
    ezstring fontName;
    int      fontSize;
    int      fontStyle;
    bool     fontIsChange;

    ImageResource images;

public:
    static ezInstance instance;

private:
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;

public:
    ezInstance();
    ~ezInstance();

    void gdiplusInit();
    void gdiplusShutdown();

    int initGraphics(int width, int height);
    void closeGraphics();

    void repaint();
};

};

//---------------------------------------------------------------------------
//
// 图片类
//
//---------------------------------------------------------------------------

class ezBitmap
{
public:
    Gdiplus::Bitmap* handle;

public:
    ezBitmap() : handle() { }

    ~ezBitmap()
    {
        this->dispose();
    }

    //创建一个图片，默认为32位色
    int create(int width, int height, int format = PixelFormat32bppARGB)
    {
        this->dispose();
        handle = new Gdiplus::Bitmap(width, height, format);
        return 0;
    }

    //打开一个图片，支持bmp、jpg、png、静态gif等常见格式
    int open(const ezstring& filename)
    {
        this->dispose();
        handle = Gdiplus::Bitmap::FromFile(filename.c_str());
        return 0;
    }

    //自动释放图片
    void dispose()
    {
        if(handle){
            delete handle;
            handle = NULL;
        }
    }

    //返回图片的宽度
    int width()const
    {
        return handle->GetWidth();
    }

    //返回图片的高度
    int height()const
    {
        return handle->GetHeight();
    }
};

//---------------------------------------------------------------------------
//
// 主函数
//
//---------------------------------------------------------------------------

//图形库初始化
int ezgdi_init(const ezstring& name, int width, int height, int style = 0);

//图形库关闭
void ezgdi_close();

//主进程执行函数
void ezgdi_execute();

//获得主窗口句柄
HWND ezgdi_window();

//消息循环处理
bool ezgdi_loop();

//显示信息
void show_message(const ezstring& msg);

//---------------------------------------------------------------------------
//
// 绘图函数
//
//---------------------------------------------------------------------------

//获得GDI+绘图设备
Gdiplus::Graphics* ezgdi_canvas();

//设置显示质量
enum EFFECT_LEVEL{
    EZGDI_SPEED,        //速度优先
    EZGDI_QUALITY       //质量优先
};

int ezgdi_effect_level(EFFECT_LEVEL level);

//清屏
void clear(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//更改画笔颜色
void pen_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//更改填充颜色
void fill_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//画笔宽度
void pen_width(float width);

//绘制线段
void draw_line(int x1, int y1, int x2, int y2);

//绘制一个空心矩形
void draw_rect(int x, int y, int width, int height);

//填充一个矩形
void fill_rect(int x, int y, int width, int height);

//绘制空心椭圆，xy为圆心
void draw_ellipse(float x, float y, float cx, float cy);

//填充椭圆
void fill_ellipse(float x, float y, float cx, float cy);

//---------------------------------------------------------------------------
//
// 字体函数
//
//---------------------------------------------------------------------------

enum EZGDI_FONTSTYLE{
    ezfsNormal     = 0,
    ezfsBold       = 1,
    ezfsItalic     = 2,
    ezfsBoldItalic = 3,
    ezfsUnderline  = 4,
    ezfsStrikeout  = 8
};

//设置字体。字体名字、大小、风格
void ezgdi_font(const ezstring& name, int size, EZGDI_FONTSTYLE style = ezfsNormal);
void font_name(const ezstring& name);
void font_size(int size);
void font_style(int style);


//字体颜色
void text_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//输出字体
void text_out(int x, int y, const ezstring& text);

//---------------------------------------------------------------------------
//
// 输入控制
//
//---------------------------------------------------------------------------

//键盘事件映射
void ezgdi_keyup(EZGDI_KEYEVENT function);
void ezgdi_keydown(EZGDI_KEYEVENT function);
void ezgdi_keypress(EZGDI_KEYEVENT function);

//鼠标事件映射
void ezgdi_mousedown(EZGDI_MOUSEEVENT function);
void ezgdi_mouseup(EZGDI_MOUSEEVENT function);
void ezgdi_mousemove(EZGDI_MOUSEEVENT function);

//判断按键是否按下
bool key_state(int key);

//---------------------------------------------------------------------------
//
// 多媒体
//
//---------------------------------------------------------------------------

//播放背景音乐
int playmusic(PCTSTR filename);

//播放wav文件
int playsound(PCTSTR filename);

#include "ezgdi.inl"

#endif //EZGDI_HPP
