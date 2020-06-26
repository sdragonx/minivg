/*
 Copyright (c) 2005-2020 sdragonx (mail:sdragonx@foxmail.com)

 ezgdi.hpp

 2020-01-01 16:37:22

 EZGDI是一个比较简单、易学、易用的C++库，设计目的旨在帮助初学者学习使用C++。
 欢迎试用EZGDI，也欢迎访问我的GITHUB提出宝贵意见。
 https://github.com/sdragonx/ezgdi

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

#include <map>
#include <sstream>
#include <string>
#include <vector>

//---------------------------------------------------------------------------
//
// 通用
//
//---------------------------------------------------------------------------

enum{
    ezNone,
    ezFixed,
    ezSizeable,
    ezFullScreen,

    ezLeft   = 1,
    ezRight  = 2,
    ezUp     = 4,
    ezDown   = 8,
    ezMiddle = 16,
};


typedef void (*EZGDI_KEYEVENT)(int key);                    //键盘事件
typedef void (*EZGDI_MOUSEEVENT)(int x, int y, int button); //鼠标事件


template<typename T, typename _char_t>
inline T string_cast(const std::basic_string<_char_t>& str)
{
    std::basic_stringstream<_char_t> stm(str);
    T n;
    stm>>n;
    return n;
}

template<typename _char_t, typename T>
std::basic_string<_char_t> to_string(const T& value)
{
    std::basic_stringstream<_char_t> stm;
    stm<<value;
    return stm.str();
}

//Unicode字符串类
class ezstring : public std::wstring
{
public:
    ezstring() : std::wstring() { }
    ezstring(const char* str) : std::wstring() { assign(str, strlen(str)); }
    ezstring(const char* str, size_t size) : std::wstring() { assign(str, size); }
    ezstring(const wchar_t* str) : std::wstring(str) { }
    ezstring(const wchar_t* str, size_t size) : std::wstring(str, size) { }

    //整数转字符串
    ezstring(int n) : std::wstring(to_string<wchar_t>(n)) { }

    //单实数转字符串
    ezstring(float n) : std::wstring(to_string<wchar_t>(n)) { }

    //双实数转字符串
    ezstring(double n) : std::wstring(to_string<wchar_t>(n)) { }

    ezstring(const ezstring& str) : std::wstring(str.c_str(), str.length()) { }

    using std::wstring::assign;
    ezstring& assign(const char* str, size_t size = -1)
    {
        std::vector<wchar_t> buf;
        int n = MultiByteToWideChar(CP_ACP, 0, str, size, 0, 0);
        buf.resize(n);
        MultiByteToWideChar(CP_ACP, 0, str, size, &buf[0], n);
        std::wstring::assign(&buf[0], n);
        return *this;
    }

    int    to_int()    { return string_cast<int>(*this);    }//字符串转整数
    float  to_float()  { return string_cast<float>(*this);  }//字符串转单实数
    double to_double() { return string_cast<double>(*this); }//字符串转双实数
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

#ifdef _MSC_VER
int random(int n)
{
    return rand() % n;
}
#endif

//---------------------------------------------------------------------------
//
// 图片类
//
//---------------------------------------------------------------------------

class ezImage
{
private:
    Gdiplus::Bitmap* m_handle;

public:
    ezImage() : m_handle() { }
    ~ezImage() { this->dispose(); }

    Gdiplus::Bitmap* handle()const { return m_handle; }

    //创建一个图片，默认为32位色
    int create(int width, int height, int format = PixelFormat32bppARGB)
    {
        this->dispose();
        m_handle = new Gdiplus::Bitmap(width, height, format);
        return 0;
    }

    //打开一个图片，支持bmp、jpg、png、静态gif等常见格式
    int open(const ezstring& filename)
    {
        this->dispose();
        m_handle = Gdiplus::Bitmap::FromFile(filename.c_str());
        return 0;
    }

    //自动释放图片
    void dispose()
    {
        if(m_handle){
            delete m_handle;
            m_handle = NULL;
        }
    }

    //返回图片的宽度
    int width()const
    {
        return m_handle ? m_handle->GetWidth() : 0;
    }

    //返回图片的高度
    int height()const
    {
        return m_handle ? m_handle->GetHeight() : 0;
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
    EZGDI_QUALITY,      //质量优先
    ezSpeed,
    ezQuality,
};

int effect_level(EFFECT_LEVEL level);

//清屏
void clear(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//更改画笔颜色
void pen_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//更改填充颜色
void fill_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//画笔宽度
void pen_width(float width);

//绘制线段
void draw_line(float x1, float y1, float x2, float y2);

//绘制一个空心矩形
void draw_rect(float x, float y, float width, float height);

//填充一个矩形
void fill_rect(float x, float y, float width, float height);

//绘制空心椭圆，xy为圆心
void draw_ellipse(float x, float y, float cx, float cy);

//填充椭圆
void fill_ellipse(float x, float y, float cx, float cy);

//绘制空心圆，xy为圆心
void draw_circle(float x, float y, float r);

//填充圆
void fill_circle(float x, float y, float r);

//---------------------------------------------------------------------------
//
// 字体函数
//
//---------------------------------------------------------------------------

enum EZGDI_FONTSTYLE{
    ezNormal     = 0,
    ezBold       = 1,
    ezItalic     = 2,
    ezBoldItalic = 3,
    ezUnderline  = 4,
    ezStrikeout  = 8
};

//设置字体。字体名字、大小、风格
void ezgdi_font(const ezstring& name, float size, EZGDI_FONTSTYLE style = ezNormal);
void font_name(const ezstring& name);
void font_size(float size);
void font_style(int style);


//字体颜色
void text_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//输出字体
void text_out(float x, float y, const ezstring& text);

//---------------------------------------------------------------------------
//
// 图片操作
//
//---------------------------------------------------------------------------

//加载图片，不用关心释放
ezImage* loadimage(const ezstring& filename);

//在xy位置绘制图片，原始大小
void draw_image(ezImage* image, float x, float y);

//在xy位置绘制图片，缩放
void draw_image(ezImage* image, float x, float y, float width, float height);

//在xy位置绘制图片，缩放，并旋转一个角度
void point_image(ezImage* image, float x, float y, float width, float height, float rotate);

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

//---------------------------------------------------------------------------
//
// 界面
//
//---------------------------------------------------------------------------

//显示信息
void showmessage(const ezstring& msg);

//显示输入框
ezstring inputbox(const ezstring& title, const ezstring& message, const ezstring value = ezstring());

//
// 窗口类
//

class ezWindow
{
public:
    HWND m_handle;

public:
    ezWindow() : m_handle() { }

    int create(PCWSTR className, PCWSTR title, int x, int y, int width, int height, int style = WS_OVERLAPPEDWINDOW);

    HWND create_button(PCWSTR title, int x, int y, int width, int height, int id)
    {
        DWORD style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;//BS_DEFPUSHBUTTON;
        return CreateWindowExW(0, L"button", title, style, x, y, width, height, m_handle, (HMENU)id, GetModuleHandle(NULL), 0);
    }

    HWND create_label(PCWSTR text, int x, int y, int width, int height, int id)
    {
        DWORD style = WS_CHILD | WS_VISIBLE | SS_EDITCONTROL;
        return CreateWindowExW(0, L"static", text, style, x, y, width, height, m_handle, (HMENU)id, GetModuleHandle(NULL), 0);
    }

    HWND create_edit(PCWSTR text, int x, int y, int width, int height, int id)
    {
        DWORD style = WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL;
        return CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", text, style, x, y, width, height, m_handle, (HMENU)id, GetModuleHandle(NULL), 0);
    }

    void set_bounds(int x, int y, int width, int height) { MoveWindow(m_handle, x, y, width, height, TRUE); }
    void show() { ShowWindow(m_handle, SW_SHOW); }
    int show_model(HWND parent);

protected:
    virtual int wndproc(UINT msg, WPARAM wparam, LPARAM lparam) { return DefWindowProc(m_handle, msg, wparam, lparam); }
    friend LRESULT CALLBACK basic_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

namespace ezgdi{

//---------------------------------------------------------------------------
//
// 资源管理类
//
//---------------------------------------------------------------------------

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

class ezResource
{
private:
    std::map<ezstring, ezImage*> images;

public:
    //加载一个图片
    ezImage* loadimage(const ezstring& name)
    {
        ezImage* bmp = NULL;
        std::map<ezstring, ezImage*>::iterator itr;
        itr = images.find(name);
        if(itr == images.end()){
            bmp = new ezImage;
            bmp->open(name);
            images[name] = bmp;
        }
        else{
            bmp = itr->second;
        }
        return bmp;
    }

    //释放所有资源
    void dispose()
    {
        delete_all(images);
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

    Gdiplus::Graphics* graphics;    //GDIPlus设备
    Gdiplus::Bitmap* colorbuf;      //背景缓冲区，使用双缓冲防止闪烁
    HBITMAP pixelbuf;               //像素缓冲区

    Gdiplus::Pen* pen;
    Gdiplus::SolidBrush* brush;

    Gdiplus::Font* font;
    Gdiplus::SolidBrush* font_color;
    ezstring fontName;
    float    fontSize;
    int      fontStyle;
    bool     fontIsChange;

    ezResource resource;

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

    static ezInstance instance;
};

};//end namespace ezgdi

#include "ezgdi.inl"

#endif //EZGDI_HPP
