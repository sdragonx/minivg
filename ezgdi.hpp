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

#ifndef UNICODE
    #error UNICODE can not defined.
#endif

#ifndef _UNICODE
    #define _UNICODE
#endif

#ifndef NO_WIN32_LEAN_AND_MEAN
    #define NO_WIN32_LEAN_AND_MEAN
#endif

#ifndef STRICT
    #define STRICT
#endif

#if defined(__GNUC__)
	#define EZ_PUBLIC_DECLARE __attribute__((weak))
#else
	#define EZ_PUBLIC_DECLARE __declspec(selectany)
#endif

#include <cmath>
#include <float.h>
#include <stdint.h>
#include <tchar.h>

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <windows.h>
#include <gdiplus.h>

/****************************************************************************
 *                                                                          *
 *                                   通用                                   *
 *                                                                          *
 ****************************************************************************/

//enum常量
enum{
    EZ_NULL,

    EZ_FIXED,               //固定大小
    EZ_SIZEABLE,            //可缩放
    EZ_FULLSCREEN,          //全屏

    EZ_LEFT   = 1,          //左
    EZ_RIGHT  = 2,          //右
    EZ_UP     = 4,          //上
    EZ_DOWN   = 8,          //下
    EZ_TOP    = EZ_UP,      //顶部
    EZ_BOTTOM = EZ_DOWN,    //底部

    EZ_CENTER_H = EZ_LEFT | EZ_RIGHT,           //水平居中
    EZ_CENTER_V = EZ_UP | EZ_DOWN,              //垂直居中
    EZ_CENTER   = EZ_CENTER_H | EZ_CENTER_V,    //居中

    EZ_MIDDLE   = 16,       //中

    EZ_OK = 0,
    EZ_ERROR = -1,
};

//byte_t类型
typedef uint8_t byte_t;

//键盘事件
typedef void (*EZ_KEY_EVENT)(int key);

//鼠标事件
typedef void (*EZ_MOUSE_EVENT)(int x, int y, int button);

//计时器事件
typedef void (*EZ_TIMER_EVENT)();

//窗口绘制事件
typedef void (*EZ_PAINT_EVENT)();

/****************************************************************************
 *                                                                          *
 *                             Unicode字符串类                              *
 *                                                                          *
 ****************************************************************************/

//字符串转其他类型
template<typename T, typename _char_t>
inline T string_cast(const std::basic_string<_char_t>& str)
{
    std::basic_stringstream<_char_t> stm(str);
    T n;
    stm>>n;
    return n;
}

//其他类型转字符串
template<typename _char_t, typename T>
std::basic_string<_char_t> to_string(const T& value)
{
    std::basic_stringstream<_char_t> stm;
    stm<<value;
    return stm.str();
}

//ansi to unicode
std::wstring to_unicode(const char* str, int length)
{
    std::vector<wchar_t> buf;
    int n = MultiByteToWideChar(CP_ACP, 0, str, length, NULL, 0);
    buf.resize(n);
    MultiByteToWideChar(CP_ACP, 0, str, length, &buf[0], n);
    return std::wstring(&buf[0], n);
}

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

    //复制构造
    ezstring(const ezstring& str) : std::wstring(str.c_str(), str.length()) { }

    using std::wstring::assign;
    ezstring& assign(const char* str, size_t size = -1)
    {
        std::vector<wchar_t> buf;
        int n = MultiByteToWideChar(CP_ACP, 0, str, int(size), NULL, 0);
        buf.resize(n);
        MultiByteToWideChar(CP_ACP, 0, str, int(size), &buf[0], n);
        std::wstring::assign(&buf[0], n);
        return *this;
    }

    int    to_int()const    { return string_cast<int>(*this);    }//字符串转整数
    float  to_float()const  { return string_cast<float>(*this);  }//字符串转单实数
    double to_double()const { return string_cast<double>(*this); }//字符串转双实数
};

/****************************************************************************
 *                                                                          *
 *                                数学、几何                                *
 *                                                                          *
 ****************************************************************************/

#ifndef M_PI
    #define M_PI      3.141592653589793238462   //acos(-1.0)
    #define M_PI_2    1.570796326794896619231   //M_PI/2
#endif

#ifndef M_RD
    #define M_RD       0.017453292519943295769  //弧度(radian)
    #define M_INV_RD  57.295779513082320876798  //弧度的倒数(reciprocal) 1.0/M_RD
#endif

//判断数值是否是0
template<typename T>inline bool is_zero(T n) { return n == 0; };
template<>inline bool is_zero<float>(float n) { return n < 0.0 ? (n > -FLT_EPSILON) : (n < FLT_EPSILON); }
template<>inline bool is_zero<double>(double n) { return n < 0.0 ? (n > -DBL_EPSILON) : (n < DBL_EPSILON); }

//判断数值是否相等
template<typename T>
inline bool is_equal(T a, T b) { return is_zero(a - b); }

//产生0 ~ n之间的随机数
#ifdef _MSC_VER
inline int random(int n) { return rand() % n; }
#endif

//产生0 ~ 1之间的随机浮点数
double randReal() { return double(rand()) / RAND_MAX; }

//产生minVal ~ maxVal之间的随机浮点数
double randReal(double minVal, double maxVal)
{
    return minVal + (maxVal - minVal) * randReal();
}

//获得向量的弧度
double radian(double x, double y)
{
    double n = 0.0;
    if(!is_zero(y)){
        n = M_PI_2 - std::atan(x / y);//根据斜率求弧度，反向
        if(y < 0.0)n += M_PI;
    }
    return n;
}

//通过xy获得角度
double get_angle(double x, double y)
{
    return radian(x, y) * M_INV_RD;
}

//从source递进到dest，步长为speed
template<typename T>
T step(T source, T dest, T speed)
{
    if(source < dest){
        source += speed;
        if(source > dest){
            source = dest;
        }
    }
    else if(source > dest){
        source -= speed;
        if(source < dest){
            source = dest;
        }
    }
    return source;
}

/****************************************************************************
 *                                                                          *
 *                                  向量类                                  *
 *                                                                          *
 ****************************************************************************/

template<typename T> class vec2;
template<typename T> class vec4;

template<typename T>
class vec2
{
public:
    T x, y;

    vec2() : x(), y() { }
    vec2(T _x, T _y) : x(_x), y(_y) { }

    vec2& operator=(const vec2& other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

	vec2& set(T _x, T _y) { x = _x; y = _y; return *this; }

    vec2 operator+(const vec2& other) { return vec2(x + other.x, y + other.y); }
    vec2 operator-(const vec2& other) { return vec2(x - other.x, y - other.y); }
    vec2& operator+=(const vec2& other) { x += other.x; y += other.y; return *this; }
    vec2& operator-=(const vec2& other) { x -= other.x; y -= other.y; return *this; }

    bool operator==(const vec2& other)const
    {
        return is_equal(x, other.x) && is_equal(y, other.y);
    }

    bool operator!=(const vec2& other)const
    {
        return !is_equal(x, other.x) || !is_equal(y, other.y);
    }

    T length()const
    {
        return sqrt(x * x + y * y);
    }

    vec2& rotate(double angle)
    {
        angle *= M_RD;
        double sine = sin(angle);
        double cosine = cos(angle);
        return set(
            x * cosine - y * sine,
            y * cosine + x * sine);
    }
};

template<typename T>
class vec4
{
public:
    union{
        struct{ T x, y, z, w; }; //空间坐标
        struct{ T r, g, b, a; }; //颜色分量
        struct{ T _x, _y, width, height; }; //矩形
    };

    vec4() : x(), y(), z(), w() { }
    vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) { }

    bool contains(int vx, int vy)
    {
        return vx >= left && vx <= (left + width) && vy >= top && vy <= (top + height);
    }

    vec4 operator&(const vec4& v)
    {
        int max_x = max(a.x, b.x);
        int max_y = max(a.y, b.y);
        return vec4(max_x, max_y, min(a.x + a.width, b.x + b.width)-max_x, min(a.y + a.height, b.y + b.height)-max_y);
    }
};

typedef vec2<int>       vec2i;
typedef vec2<float>     vec2f;
typedef vec2<double>    vec2d;

typedef vec4<byte_t>    vec4ub;
typedef vec4<int>       vec4i;
typedef vec4<float>     vec4f;
typedef vec4<double>    vec4d;

/****************************************************************************
 *                                                                          *
 *                                  图片类                                  *
 *                                                                          *
 ****************************************************************************/

class ezImage
{
public:
    Gdiplus::Bitmap* m_handle;

public:
    ezImage();
    ~ezImage();

    Gdiplus::Bitmap* handle()const;

    //创建一个图片，默认为32位色
    int create(int width, int height, int format = PixelFormat32bppARGB);

    //打开一个图片，支持bmp、jpg、png、静态gif等常见格式
    int open(const ezstring& filename);

    //打开资源中的图片
    int open(int id, PCTSTR resource_type = TEXT("BITMAP"));

    //映射一个HBITMAP对象
    int map(HBITMAP hbmp);

    //保存图片
    int save(const ezstring& filename);

    //自动释放图片
    void close();

    //返回图片的宽度
    int width()const;

    //返回图片的高度
    int height()const;
};

/****************************************************************************
 *                                                                          *
 *                                  主函数                                  *
 *                                                                          *
 ****************************************************************************/

/*

示例代码1：
int main(int argc, char* argv[])
{
    //初始化库，创建窗口
    ezInit("窗口标题", 800, 600);

    while(ezLoop()){
      //TODO : 输入绘图代码

    }

    //退出时清理资源
    ezClose();
}

示例代码2：
void MyPaint();

int main(int argc, char* argv[])


    //初始化库，创建窗口
    ezInit("窗口标题", 800, 600);

    //设置绘图事件函数
    ezOnPaint(MyPaint);

    //主进程执行函数
    ezRun();

    //退出时清理资源
    ezClose();
}

void MyPaint()
{
    //TODO : 输入绘图代码
}

***/

//图形库初始化
int ezInit(
    const ezstring& title,  //窗口标题
    int width,              //窗口宽度
    int height,             //窗口高度
    int style               //窗口样式 EZ_FIXED 固定大小；EZ_SIZEABLE 可缩放；EZ_FULLSCREEN 全屏
    );

//图形库关闭
void ezClose();

//获得主窗口句柄
HWND ezHWnd();

//重新设置窗口大小
void ezResize(
    int width,              //窗口宽度
    int height              //窗口高度
    );

//设置窗口客户去大小
void ezClientResize(
    int width,              //客户区宽度
    int height              //客户区高度
    );

//窗口全屏
void ezFullScreen(
    bool value              //true 全屏；false 取消全屏
    );

//获得GDI绘图设备
HDC ezHDC();

//消息循环处理
bool ezLoop();

//主进程执行函数
void ezRun();

/****************************************************************************
 *                                                                          *
 *                          窗口事件、输入按键管理                          *
 *                                                                          *
 ****************************************************************************/

/* 判断按键是否按下
 * 常用按键：
 * VK_ESCAPE        ESC
 * VK_RETURN        回车
 * VK_LEFT          左方向键
 * VK_RIGHT         右方向键
 * VK_UP            上方向键
 * VK_DOWN          下方向键
 * VK_SPACE         空格
 */
bool ezKeyState(int key);

//键盘事件映射
void ezOnKeyUp(EZ_KEY_EVENT function);
void ezOnKeyDown(EZ_KEY_EVENT function);
void ezOnChar(EZ_KEY_EVENT function);

//鼠标事件映射
void ezOnMouseMove(EZ_MOUSE_EVENT function);
void ezOnMouseDown(EZ_MOUSE_EVENT function);
void ezOnMouseUp(EZ_MOUSE_EVENT function);

//设置计时器
void ezTimer(
    UINT interval       //计时器时间间隔，单位毫秒，输入0停止计时器
    );

//计时器事件
void ezOnTimer(EZ_TIMER_EVENT function);

//窗口绘制事件
void ezOnPaint(EZ_PAINT_EVENT function);

/****************************************************************************
 *                                                                          *
 *                                 绘图函数                                 *
 *                                                                          *
 ****************************************************************************/

//获得GDI+绘图设备
Gdiplus::Graphics* ezGraphics();

//设置显示质量
enum {
    EZ_SPEED,   //速度优先
    EZ_MEDIUM,  //中等质量
    EZ_QUALITY, //质量优先
};

int ezEffectLevel(int level);

//清屏
void ezClear(BYTE r, BYTE g, BYTE b, BYTE a = 255);

//更改画笔颜色
void ezPenColor(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void ezPenColor(DWORD argb);

//获取画笔颜色
DWORD ezPenColor();

//画笔宽度
void ezPenWidth(float width);

//画笔模式
enum{
    EZ_SOLID,       //实心画笔（默认）
    EZ_DASH,        // -------
    EZ_DOT,         // .......
    EZ_DASH_DOT,    // -.-.-.-
    EZ_CUSTOM = 5   //自定义
};

//设置画笔模式
void ezPenStyle(int mode);

//设置点画模式间隔
void ezDashStyle(
    const float* dash,      //点画模式间隔，浮点数数组
    int size                //浮点数数组大小
    );

//更改填充颜色
void ezFillColor(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void ezFillColor(DWORD argb);

//获取填充颜色
DWORD ezFillColor();

//绘制一个点
void ezPoint(float x, float y);

//绘制线段
void ezDrawLine(float x1, float y1, float x2, float y2);

//绘制一个空心矩形
void ezDrawRect(float x, float y, float width, float height);

//填充一个矩形
void ezFillRect(float x, float y, float width, float height);

//绘制圆角矩形
void ezDrawRoundRect(float x, float y, float width, float height, float cx, float cy);

//填充圆角矩形
void ezFillRoundRect(float x, float y, float width, float height, float cx, float cy);

//绘制空心椭圆，xy为圆心
void ezDrawEllipse(float x, float y, float cx, float cy);

//填充椭圆
void ezFillEllipse(float x, float y, float cx, float cy);

//绘制空心圆，xy为圆心
void ezDrawCircle(float x, float y, float r);

//填充圆
void ezFillCircle(float x, float y, float r);

//绘制连续的线段
void ezDrawPolyline(const vec2f* points, size_t size);

//绘制多边形
void ezDrawPolygon(const vec2f* points, size_t size);

//填充多边形
void ezFillPolygon(const vec2f* points, size_t size);

/****************************************************************************
 *                                                                          *
 *                                 字体函数                                 *
 *                                                                          *
 ****************************************************************************/

//字体样式，可以随意组合
enum EZGDI_FONTSTYLE{
    EZ_NORMAL       = 0,    //普通字体
    EZ_BOLD         = 1,    //粗体
    EZ_ITALIC       = 2,    //斜体
    EZ_UNDERLINE    = 4,    //下划线
    EZ_STRIKEOUT    = 8     //删除线
};

//设置字体。字体名字、大小、风格
void ezFont(const ezstring& name, float size, EZGDI_FONTSTYLE style = EZ_NORMAL);

//设置字体属性
void ezFontName(const ezstring& name);
void ezFontSize(float size);
void ezFontStyle(int style);

//获取字体属性
ezstring ezFontName();
int ezFontSize();
int ezFontStyle();

//字体颜色
void ezFontColor(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void ezFontColor(UINT color);

//输出字体
void ezText(float x, float y, const wchar_t* text, size_t length);
void ezText(float x, float y, const ezstring& text);

void ezText(float x, float y, float width, float height, const ezstring& text, int align = 0);

//字体格式化输出，和printf使用类似
void ezPrint(float x, float y, const wchar_t* format, ...);

//获得字符串的像素宽度
float ezTextWidth(const ezstring& text);

//获得字符串的像素高度
float ezTextHeight(const ezstring& text);

/****************************************************************************
 *                                                                          *
 *                                 图片操作                                 *
 *                                                                          *
 ****************************************************************************/

//加载图片，不用关心释放
ezImage* ezLoadImage(const ezstring& filename);

//加载资源中的图片
ezImage* ezLoadImage(int id, PCTSTR resource_type = TEXT("PNG"));

//保存图片，默认保存为PNG格式
int ezSaveImage(ezImage* image, const ezstring& filename);

//在xy位置绘制图片，原始大小
void ezDraw(ezImage* image, float x, float y);

//在xy位置绘制图片，缩放
void ezDraw(ezImage* image, float x, float y, float width, float height);

//在xy位置绘制图片，并旋转一个角度
void ezRotateImage(ezImage* image, float x, float y, float rotate);

//在xy位置绘制图片，缩放，并旋转一个角度
void ezRotateImage(ezImage* image, float x, float y, float width, float height, float rotate);

//---------------------------------------------------------------------------
//
// 多媒体
//
//---------------------------------------------------------------------------

//播放背景音乐
void ezPlayMusic(PCTSTR filename);

//播放资源中的音乐
void ezPlayResourceMusic(PCTSTR filename, PCTSTR resource_type = TEXT("MP3"));
void ezPlayResourceMusic(int id, PCTSTR resource_type = TEXT("MP3"));

//播放wav文件，loop是否循环
int ezPlaySound(PCTSTR filename, bool loop = false);

//播放资源中的wav文件
int ezPlayResourceSound(PCTSTR filename, bool loop = false);
int ezPlayResourceSound(int id, bool loop = false);

//---------------------------------------------------------------------------
//
// 界面
//
//---------------------------------------------------------------------------

//消息对话框
int ezMsgBox(const ezstring& msg, const ezstring& title, int type = MB_OK);

//显示信息
void ezMessage(const ezstring& msg);

//显示输入框
ezstring ezInputBox(const ezstring& title, const ezstring& message, const ezstring value = ezstring());

#include "ezgdi.inl"

#endif //EZGDI_HPP
