/*
 Copyright (c) 2005-2020 sdragonx (mail:sdragonx@foxmail.com)

 minivg.hpp

 2020-01-01 16:37:22

 EZGDI 是一个比较简单、易学、易用的 C++ 库，设计目的旨在帮助初学者学习使用 C++。
 欢迎试用 MINIVG，也欢迎访问我的 GITHUB 提出宝贵意见。
 https://github.com/sdragonx/ezgdi

 如果是用的 Visual Studio，这个库不需要其他设置就能用；
 如果用的是 g++ 编译器， 比如 DevCPP，需要在编译参数里面添加 -finput-charset=GBK 来支持中文字符
 库链接里面添加 -lgdi32 -lgdiplus 两个库的引用。

*/
#ifndef MINIVG_HPP
#define MINIVG_HPP

//#ifndef UNICODE
//  #error UNICODE can not defined.
//#endif

#ifndef UNICODE
  #define UNICODE
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

#define NOMINMAX

#if defined(__GNUC__)
  #define EZ_PUBLIC_DECLARE __attribute__((weak))
#else
  #define EZ_PUBLIC_DECLARE __declspec(selectany)
  #ifdef _MSC_VER
    #define _USE_MATH_DEFINES
  #endif
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

using std::min;
using std::max;

#include <Windows.h>
#include <gdiplus.h>



/****************************************************************************
 *                                                                          *
 *                                   通用                                   *
 *                                                                          *
 ****************************************************************************/

namespace minivg{

// enum 常量
enum{
    EZ_NULL,

    EZ_FIXED = 1,                           //固定大小
    EZ_SIZEABLE = 2,                        //可缩放
    EZ_FULLSCREEN = 4,                      //全屏
    EZ_BACKBUFFER = 8,                      //不创建窗口

    EZ_LEFT   = 1,                          //左
    EZ_RIGHT  = 2,                          //右
    EZ_UP     = 4,                          //上
    EZ_DOWN   = 8,                          //下
    EZ_TOP    = EZ_UP,                      //顶部
    EZ_BOTTOM = EZ_DOWN,                    //底部

    EZ_CENTER_H = EZ_LEFT | EZ_RIGHT,       //水平居中
    EZ_CENTER_V = EZ_UP | EZ_DOWN,          //垂直居中
    EZ_CENTER = EZ_CENTER_H | EZ_CENTER_V,  //居中

    EZ_MIDDLE   = 16,                       //中

    EZ_RGB,                                 //RGB 颜色
    EZ_RGBA,                                //RGBA 颜色

    EZ_OK = 0,
    EZ_ERROR = -1,
};

// 键盘事件
typedef void (*EZ_KEY_EVENT)(int key);

// 鼠标事件
typedef void (*EZ_MOUSE_EVENT)(int x, int y, int button);

// 计时器事件
typedef void (*EZ_TIMER_EVENT)();

// 窗口绘制事件
typedef void (*EZ_PAINT_EVENT)();

/****************************************************************************
 *                                                                          *
 *                             Unicode字符串类                              *
 *                                                                          *
 ****************************************************************************/

// 字符串转其他类型
template<typename T, typename _char_t>
inline T string_cast(const std::basic_string<_char_t>& str)
{
    std::basic_stringstream<_char_t> stm(str);
    T n;
    stm >> n;
    return n;
}

// 其他类型转字符串
template<typename _char_t, typename T>
std::basic_string<_char_t> to_string(const T& value)
{
    std::basic_stringstream<_char_t> stm;
    stm << value;
    return stm.str();
}

// unicode to ansi
inline std::string to_ansi(const wchar_t* str, int length)
{
    std::vector<char> buf;
    int n = WideCharToMultiByte(CP_OEMCP, 0, str, length, NULL, 0, NULL, FALSE);
    buf.resize(n);
    WideCharToMultiByte(CP_OEMCP, 0, str, length, &buf[0], n, NULL, FALSE);
    return std::string(&buf[0], n);
}

// ansi to unicode
inline std::wstring to_unicode(const char* str, int length)
{
    std::vector<wchar_t> buf;
    int n = MultiByteToWideChar(CP_ACP, 0, str, length, NULL, 0);
    buf.resize(n);
    MultiByteToWideChar(CP_ACP, 0, str, length, &buf[0], n);
    return std::wstring(&buf[0], n);
}

class unistring : public std::wstring
{
public:
    unistring() : std::wstring() { }
    unistring(const char* str) : std::wstring() { assign(str, strlen(str)); }
    unistring(const char* str, size_t size) : std::wstring() { assign(str, size); }
    unistring(const std::string& str) : std::wstring() { assign(str.c_str(), str.length()); }

    unistring(const wchar_t* str) : std::wstring(str) { }
    unistring(const wchar_t* str, size_t size) : std::wstring(str, size) { }
    unistring(const std::wstring& str) : std::wstring(str) { }

    // 整数转字符串
    unistring(int n) : std::wstring(to_string<wchar_t>(n)) { }

    // 单实数转字符串
    unistring(float n) : std::wstring(to_string<wchar_t>(n)) { }

    // 双实数转字符串
    unistring(double n) : std::wstring(to_string<wchar_t>(n)) { }

    // 复制构造
    unistring(const unistring& str) : std::wstring(str.c_str(), str.length()) { }

    using std::wstring::assign;

    // 多字符字符串赋值
    unistring& assign(const char* str, size_t size = -1)
    {
        std::vector<wchar_t> buf;
        int n = MultiByteToWideChar(CP_ACP, 0, str, int(size), NULL, 0);
        buf.resize(n);
        MultiByteToWideChar(CP_ACP, 0, str, int(size), &buf[0], n);
        std::wstring::assign(buf.begin(), buf.end());
        return *this;
    }

    int    to_int()const    { return string_cast<int>(*this);    }// 字符串转整数
    float  to_float()const  { return string_cast<float>(*this);  }// 字符串转单实数
    double to_double()const { return string_cast<double>(*this); }// 字符串转双实数
};

/****************************************************************************
 *                                                                          *
 *                                数学、几何                                *
 *                                                                          *
 ****************************************************************************/

#ifndef M_PI
  #define M_PI      3.141592653589793238462     // acos(-1.0)
#endif

#ifndef M_PI_2
    #define M_PI_2    1.570796326794896619231   // M_PI/2
#endif

#ifndef M_RD
  #define M_RD       0.017453292519943295769    // 弧度(radian)
  #define M_INV_RD  57.295779513082320876798    // 弧度的倒数(reciprocal) 1.0/M_RD
#endif

// 判断数值是否是 0
template<typename T>inline bool is_zero(T n) { return n == 0; };
template<>inline bool is_zero<float>(float n) { return n < 0.0 ? (n > -FLT_EPSILON) : (n < FLT_EPSILON); }
template<>inline bool is_zero<double>(double n) { return n < 0.0 ? (n > -DBL_EPSILON) : (n < DBL_EPSILON); }

// 判断数值是否相等
template<typename T>
inline bool is_equal(T a, T b) { return is_zero(a - b); }

// 产生 0 ~ n 之间的随机数
template<typename T>
inline int random(T n) { return rand() % n; }

// 产生 0 ~ 1 之间的随机浮点数
inline double rand_real() { return double(rand()) / RAND_MAX; }

// 产生 minVal ~ maxVal 之间的随机浮点数
inline double rand_real(double minVal, double maxVal)
{
    return minVal + (maxVal - minVal) * rand_real();
}

// 获得向量的弧度
inline double radian(double x, double y)
{
    double n = 0.0;
    if(!is_zero(y)){
        n = M_PI_2 - std::atan(x / y);//根据斜率求弧度，反向
        if(y < 0.0)n += M_PI;
    }
    return n;
}

// 通过xy获得角度
inline double angle(double x, double y)
{
    return radian(x, y) * M_INV_RD;
}

// 从 source 递进到 dest，步长为 speed
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

#ifndef GLM_VERSION

#define VEC2_OPERATION(op)\
    template<typename X>\
    vec2 operator op(const X& value)const\
    {\
        return vec2(x op value, y op value);\
    }\
    template<typename X>\
    vec2 operator op(const vec2<X>& v)const\
    {\
        return vec2(x op v.x, y op v.y);\
    }\
    template<typename X>\
    vec2& operator op##=(const X& value)\
    {\
        x op##= value; y op##= value;\
        return *this;\
    }\
    template<typename X>\
    vec2& operator op##=(const vec2<X>& v)\
    {\
        x op##= v.x; y op##= v.y;\
        return *this;\
    }

#define VEC4_OPERATION(op)\
    template<typename X>\
    vec4T operator op(const X& value)const\
    {\
        return this_type(x op value, y op value, z op value, w op value);\
    }\
    template<typename X>\
    vec4T operator op(const vec4<X>& v)const\
    {\
        return this_type(x op v.x, y op v.y, z op v.z, w op v.w);\
    }\
    template<typename X>\
    vec4T& operator op##=(const X& value)\
    {\
        x op##= value; y op##= value; z op##= value; w op##= value;\
        return *this;\
    }\
    template<typename X>\
    vec4& operator op##=(const vec4<X>& v)\
    {\
        x op##= v.x; y op##= v.y; z op##= v.z; w op##= v.w;\
        return *this;\
    }

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

    VEC2_OPERATION(+)
    VEC2_OPERATION(-)
    VEC2_OPERATION(*)
    VEC2_OPERATION(/)

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
        return (T)sqrt(x * x + y * y);
    }

    vec2& rotate(double angle)
    {
        using namespace std;
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
        struct{ T x, y, z, w; }; // 空间坐标
        struct{ T b, g, r, a; }; // 颜色分量 (GDI 使用的是 BGRA)
        struct{ T _x, _y, width, height; }; // 矩形
    };

    vec4() : x(), y(), z(), w() { }
    vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) { }

    vec4& set(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; return *this; }

    bool contains(int vx, int vy)
    {
        return vx >= x && vx <= (x + width) && vy >= y && vy <= (y + height);
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

typedef vec4<uint8_t>   vec4ub;
typedef vec4<int>       vec4i;
typedef vec4<float>     vec4f;
typedef vec4<double>    vec4d;

#else

#if !defined(GLM_HPP_20211019161738)
namespace cgl
{
	typedef glm::ivec2      vec2i;
	typedef glm::vec2       vec2f;
	typedef glm::dvec2      vec2d;
	
	typedef glm::u8vec4     vec4ub;
	typedef glm::ivec4      vec4i;
	typedef glm::vec4       vec4f;
	typedef glm::dvec4      vec4d;
}
#endif

using cgl::vec2i;
using cgl::vec2f;
using cgl::vec2d;

using cgl::vec4ub;
using cgl::vec4i;
using cgl::vec4f;
using cgl::vec4d;

#endif

/****************************************************************************
 *                                                                          *
 *                                  图片类                                  *
 *                                                                          *
 ****************************************************************************/

class ezImage
{
protected:
    Gdiplus::Bitmap* m_handle;
    Gdiplus::BitmapData* m_data;

public:
    ezImage();
    ~ezImage();

    Gdiplus::Bitmap* handle()const;

    // 创建一个图片，默认为32位色
    int create(int width, int height, int format = PixelFormat32bppARGB);

    // 打开一个图片，支持 bmp、jpg、png、静态gif 等常见格式
    int open(const unistring& filename);

    // 打开资源中的图片
    int open(int id, PCTSTR resource_type = TEXT("BITMAP"));

    // 映射一个 HBITMAP 对象
    int map(HBITMAP hbmp);

    // 判断图片是否为空
    bool empty()const;

    // 保存图片
    int save(const unistring& filename);

    // 自动释放图片
    void close();

    // 返回图片的宽度
    int width()const;

    // 返回图片的高度
    int height()const;

    // 获取图像数据
    void* lock(int pixelformat = EZ_RGBA);

    // 还原图像数据
    void unlock();
};

/****************************************************************************
 *                                                                          *
 *                                  主函数                                  *
 *                                                                          *
 ****************************************************************************/

/*

示例代码：
int main(int argc, char* argv[])
{
    // 初始化库，创建窗口
    initgraph("窗口标题", 800, 600);

    while(do_events()){
      //TODO : 输入绘图代码

    }
}

示例代码：
void display();

int main(int argc, char* argv[])


    // 初始化库，创建窗口
    initgraph("窗口标题", 800, 600);

    // 设置绘图事件函数
    display_event(display);

    // 主进程执行函数
    return start_app();
}

void display()
{
    //TODO : 输入绘图代码
}

***/

/* 图形库初始化
 * title            窗口标题
 * width            窗口宽度
 * height           窗口高度
 * param            参数：
 *                      EZ_FIXED        固定大小
 *                      EZ_SIZEABLE     可缩放
 *                      EZ_FULLSCREEN   全屏
 *                      EZ_BUFFER       只创建绘图缓冲区，不创建窗口
 */
int initgraph(int width, int height, int param = EZ_FIXED);
int initgraph(const unistring& title, int width, int height, int param = EZ_FIXED);

/* 在已有的窗口界面上初始化。
 * 可以和vcl、mfc，或者其他界面库协同使用。（貌似只有顶层窗口能实现效果）
 * hwnd             要初始化的窗口句柄
 */
int initgraph(HWND hwnd);

/* 退出
 */
void quit();

/* 获得主窗口句柄
 */
HWND graph_window();

/* 获取 GDI 绘图设备
 */
HDC graph_hdc();

/* 设置窗口标题
 * text             标题
 */
void set_title(const unistring& text);

/* 重新设置窗口大小（客户区大小）
 * width            窗口宽度
 * height           窗口高度
 */
void reshape(int width, int height);

/* 窗口全屏
 * value    true 全屏；false 取消全屏
 */
void fullscreen(bool value);

/* 消息循环处理，如果返回 false，表示程序退出。
 */
bool do_events();

/* 主进程执行函数
 */
int start_app();

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
bool keystate(int key);

/* 键盘事件映射
 */
void key_push_event(EZ_KEY_EVENT function);
void key_pop_event(EZ_KEY_EVENT function);
void input_event(EZ_KEY_EVENT function);

/* 鼠标事件映射
 */
void mouse_push_event(EZ_MOUSE_EVENT function);
void mouse_pop_event(EZ_MOUSE_EVENT function);
void mouse_move_event(EZ_MOUSE_EVENT function);

/* 设置计时器
 * interval         计时器时间间隔，单位毫秒，输入 0 停止计时器
 */
void start_timer(UINT interval);

/* 关闭计时器
 */
void stop_timer();

/* 计时器事件
 */
void timer_event(EZ_TIMER_EVENT function);

/* 窗口绘制事件
 */
void display_event(EZ_PAINT_EVENT function);

/****************************************************************************
 *                                                                          *
 *                                 绘图函数                                 *
 *                                                                          *
 ****************************************************************************/

/* 获得 GDI+ 绘图设备
 */
Gdiplus::Graphics* graphics();

/* 重设背景缓冲区大小位置
 *
 */
void viewport(int x, int y, int width, int height);

/* 背景缓冲绘制到目标 HDC
 */
void framebuf_blt(HDC hdc);

/* 设置显示质量
 */
enum {
    EZ_SPEED,   //速度优先
    EZ_MEDIUM,  //中等质量
    EZ_QUALITY, //质量优先
};

int effect_level(int level);

/* 清屏
 */
void clear(BYTE r, BYTE g, BYTE b, BYTE a = 255);

/* 更改画笔颜色
 * r                红色分量
 * g                绿色分量
 * b                蓝色分量
 * a                透明度
 */
void pen_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void pen_color(COLORREF argb);
void pen_color(vec4ub color);

/* 获取画笔颜色
 */
COLORREF pen_color();

/* 画笔宽度
 */
void pen_width(float width);

/* 画笔模式
 */
enum{
    EZ_SOLID,       // 实心画笔（默认）
    EZ_DASH,        // -------
    EZ_DOT,         // .......
    EZ_DASH_DOT,    // -.-.-.-
    EZ_CUSTOM = 5   // 自定义
};

/* 设置画笔模式
 */
void pen_style(int mode);

/* 设置点画模式间隔，由一组浮点数代表点画模式的宽度。
 * dash             点画模式间隔，浮点数数组
 * size             浮点数数组大小
 */
void dash_style(const float* dash, int size);

/* 更改填充颜色
 * r                红色分量
 * g                绿色分量
 * b                蓝色分量
 * a                透明度
 */
void fill_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void fill_color(COLORREF argb);

/* 获取填充颜色
 */
COLORREF fill_color();

/* 绘制一个点
 * x                x 方向坐标
 * y                y 方向坐标
 */
void draw_point(float x, float y);

/* 绘制线段
 * x1, y1           第一个点
 * x2, y2           第二个点
 */
void draw_line(float x1, float y1, float x2, float y2);

/* 绘制一个空心矩形
 * x, y             左上角坐标
 * width            矩形宽度
 * height           矩形高度
 */
void draw_rect(float x, float y, float width, float height);

/* 填充一个矩形
 * x, y             左上角坐标
 * width            矩形宽度
 * height           矩形高度
 */
void fill_rect(float x, float y, float width, float height);

/* 绘制圆角矩形
 * x, y             左上角坐标
 * width            矩形宽度
 * height           矩形高度
 * cx, cy           圆角圆形大小
 */
void draw_roundrect(float x, float y, float width, float height, float cx, float cy);

/* 填充圆角矩形
 * x, y             左上角坐标
 * width            矩形宽度
 * height           矩形高度
 * cx, cy           圆角圆形大小
 */
void fill_roundrect(float x, float y, float width, float height, float cx, float cy);

/* 绘制空心椭圆
 * x, y             圆心坐标
 * cx, cy           椭圆的横向半径和纵向半径
 */
void draw_ellipse(float x, float y, float cx, float cy);

/* 填充椭圆
 * x, y             圆心坐标
 * cx, cy           椭圆的横向半径和纵向半径
 */
void fill_ellipse(float x, float y, float cx, float cy);

/* 绘制空心圆，xy为圆心
 * x, y             圆心坐标
 * r                圆半径
 */
void draw_circle(float x, float y, float r);

/* 绘制空心圆，xy为圆心
 * x, y             圆心坐标
 * r                圆半径
 */
void fill_circle(float x, float y, float r);

/* 绘制连续的线段
 * points           点数组
 * size             点数组大小
 */
void draw_polyline(const vec2f* points, size_t size);

/* 绘制多边形
 * points           点数组
 * size             点数组大小
 */
void draw_polygon(const vec2f* points, size_t size);

/* 填充多边形
 * points               点数组
 * size             点数组大小
 */
void fill_polygon(const vec2f* points, size_t size);

/****************************************************************************
 *                                                                          *
 *                                 字体函数                                 *
 *                                                                          *
 ****************************************************************************/

/* 字体样式，可以随意组合
 */
enum EZGDI_FONTSTYLE{
    EZ_NORMAL       = 0,    //普通字体
    EZ_BOLD         = 1,    //粗体
    EZ_ITALIC       = 2,    //斜体
    EZ_UNDERLINE    = 4,    //下划线
    EZ_STRIKEOUT    = 8     //删除线
};

/* 设置字体
 * name             字体名称
 * size             字体大小
 * style            字体样式，EZGDI_FONTSTYLE 类型的组合
 */
void setfont(const unistring& name, float size, EZGDI_FONTSTYLE style = EZ_NORMAL);

/* 设置字体
 * name             字体名称
 * size             字体大小
 * bold             是否粗体
 * underline        是否下划线
 * strikeout        是否删除线
 */
void setfont(const unistring& name, float size, bool bold, bool, bool underline, bool strikeout);

/* 设置字体属性
 */
void font_name(const unistring& name);
void font_size(float size);
void font_style(int style);

/* 获取字体属性
 */
unistring font_name();
int font_size();
int font_style();

/* 字体颜色
 * r                红色分量
 * g                绿色分量
 * b                蓝色分量
 * a                透明度
 */
void font_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void font_color(COLORREF color);

//输出字体
void textout(float x, float y, const char* text, size_t length);
void textout(float x, float y, const wchar_t* text, size_t length);
void textout(float x, float y, const unistring& text);

void drawtext(float x, float y, float width, float height, const unistring& text, int align = 0);

/* 字体格式化输出，和printf使用类似
 * x, y             绘制的字符串坐标
 * param            格式化字符串
 * ...              可变参数
 */
void print(float x, float y, const char* param, ...);
void print(float x, float y, const wchar_t* param, ...);

/* 获得字符串的像素宽度
 */
float textwidth(const unistring& text);

/* 获得字符串的像素高度
 */
float textheight(const unistring& text);

/****************************************************************************
 *                                                                          *
 *                                 图片操作                                 *
 *                                                                          *
 ****************************************************************************/

/* 创建图片
 * width            宽度
 * height           高度
 */
ezImage* newimage(int width, int height);

/* 删除图片
 * image            图片指针
 */
void freeimage(ezImage* image);

/* 加载图片，不用关心释放
 */
ezImage* loadimage(const unistring& filename);

/* 加载资源中的图片
 */
ezImage* loadimage(int id, PCTSTR resource_type = TEXT("PNG"));

/* 保存图片
 * image            要保存的图片
 * filename         png 图片文件名
 */
int saveimage(ezImage* image, const unistring& filename);

/* 原始大小绘制图片
 * xy               图片绘制位置
 */
void drawimage(ezImage* image, float x, float y);

/* 根据位置范围绘制图片
 * x, y             图片绘制位置
 * width            绘制的宽度
 * height           绘制的高度
 */
void drawimage(ezImage* image, float x, float y, float width, float height);

/* 原始大小旋转绘制图片，旋转中心是图片中心。
 * x, y             图片绘制位置
 * rotate           旋转的角度，单位是角度
 */
void rotate_image(ezImage* image, float x, float y, float rotate);

/* 根据位置范围旋转绘制图片，旋转中心是图片中心。
 * x, y             图片绘制位置
 * width            绘制的宽度
 * height           绘制的高度
 * rotate           旋转的角度，单位是角度
 */
void rotate_image(ezImage* image, float x, float y, float width, float height, float rotate);

/* 把像素直接绘制到屏幕上，像素格式必须是 BGRA 32位。
 * x, y             像素绘制位置
 * width            绘制的宽度
 * height           绘制的高度
 * pwidth           图片像素的宽度
 * pheight          图片像素的高度
 */
void draw_pixels(float x, float y, float width, float height, void* pixels, int pwidth, int pheight);

/****************************************************************************
 *                                                                          *
 *                                  多媒体                                  *
 *                                                                          *
 ****************************************************************************/

/* 播放背景音乐
 * filename         音乐文件路径
 */
void play_music(PCTSTR filename);

/* 播放资源中的音乐
 * filename         音乐文件资源名称
 * resource_type    自定义资源类型名称
 */
void play_resource_music(PCTSTR filename, PCTSTR resource_type = TEXT("MP3"));

/* 播放资源中的音乐
 * id               音乐文件资源 ID
 * resource_type    自定义资源类型名称
 */
void play_resource_music(int id, PCTSTR resource_type = TEXT("MP3"));

/* 停止播放音乐
 */
void stop_music();


/* 播放 wav 文件
 * filename         wav 文件路径
 * loop             是否循环
 */
int play_sound(PCTSTR filename, bool loop = false);

/* 播放资源中的wav文件
 * filename         wav 资源名称
 * loop             是否循环
 */
int play_resource_sound(PCTSTR filename, bool loop = false);

/* 播放资源中的wav文件
 * id               wav 资源 ID
 * loop             是否循环
 */
int play_resource_sound(int id, bool loop = false);

/* 停止所有声音播放
 */
void stop_sound();

/****************************************************************************
 *                                                                          *
 *                                   界面                                   *
 *                                                                          *
 ****************************************************************************/

/* 消息对话框
 * msg              消息文本
 * title            消息窗口标题
 * type             按钮类型
 * 返回值           返回点击的按钮 ID
 */
int msgbox(const unistring& message, const unistring& title = L"消息", int type = MB_OK);

/* 显示输入框
 * message          消息文本
 * title            输入框标题
 * default_value    默认值
 * 返回值           成功输入，返回非空的字符串
 */
unistring inputbox(const unistring& message, const unistring& title = L"输入", const unistring& default_value = unistring());

/* 显示一个图片
 */
//void imshow(mat image);

}// end namespace minivg

#include "minivg.inl"

using namespace minivg;

#endif// MINIVG_HPP
