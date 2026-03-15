/*
 Copyright (c) 2005-2020 sdragonx (mail:sdragonx@foxmail.com)

 minivg.hpp

 2020-01-01 16:37:22

 这个库基本现在改名为 minivg。

 minivg 是一个比较简单、易学、易用的 C++ 库，设计目的旨在帮助初学者学习使用 C++。
 欢迎试用 minivg，也欢迎访问我的 github 提出宝贵意见。
 https://github.com/sdragonx/minivg

 编译器设置：
 Visiual Studio:
    这个库不需要其他设置就能用。
 g++ (mingw32/64):
    如果用的是 g++ 编译器，比如 DevCPP，默认一般使用 gbk 编码，可以在编译参数里面添加 -finput-charset=GBK
    来支持中文字符。如果你的工程使用的是 utf8 编码，可以把 "minivg.hpp", "minivg.inl" 这两个文件转成 utf8 编码来使用，
    编译器的参数添加 -finput-charset=utf-8 来支持 utf8 编码。
    库链接里面添加 -lgdi32 -lgdiplus 两个库的引用。

示例代码：

#include <minivg.hpp>

int main(int argc, char* argv[])
{
    // 初始化库，创建窗口
    initgraph("窗口标题", 800, 600);

    while(do_events()){
      // TODO : 输入绘图代码

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

*/
#ifndef MINIVG_HPP
#define MINIVG_HPP

#ifndef NO_WIN32_LEAN_AND_MEAN
    #define NO_WIN32_LEAN_AND_MEAN
#endif

#ifndef STRICT
    #define STRICT
#endif

#if defined(_MSC_VER)
    #ifndef _USE_MATH_DEFINES
        #define _USE_MATH_DEFINES
    #endif
#endif

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifdef max
    #undef max
#endif
#ifdef min
    #undef min
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

using std::max;
using std::min;

#include <Windows.h>
#include <objbase.h>
#include <gdiplus.h>

typedef unsigned char byte_t;

namespace minivg {

//---------------------------------------------------------------------------
// 通用
//---------------------------------------------------------------------------

// clang-format off

// 常量
enum
{
    VG_NULL,
};

// 窗口样式
enum vgWindowStyle
{
    VG_FIXED      = 0,                      // 固定大小窗口
    VG_SIZEABLE   = 1,                      // 可缩放窗口
    VG_FULLSCREEN = 2,                      // 全屏窗口
    VG_BACKBUFFER = 4,                      // 缓冲区模式。不创建窗口，需要使用 framebuf_blt(hdc) 绘制到目标 HDC
};

// 颜色格式
enum vgFormat
{
    VG_RGB565 = 0x00021006,                 // RGB565 颜色格式
    VG_RGB    = 0x00021808,                 // RGB    颜色格式
    VG_RGBA   = 0x0026200A,                 // RGBA   颜色格式
    VG_PRGBA  = 0x000E200B,                 // RGBA   预乘颜色格式
};

// 图片格式
enum vgImageFormat
{
    VG_BMP,
    VG_JPG,
    VG_GIF,
    VG_TIFF,
    VG_PNG,
};

/* 方向
 *   + 4 +
 *   1 * 2
 *   + 8 +
 */
enum vgDirection
{
    VG_LEFT  = 1,                           // 左
    VG_RIGHT = 2,                           // 右
    VG_UP    = 4,                           // 上
    VG_DOWN  = 8,                           // 下

    VG_TOP = VG_UP,                         // 顶部
    VG_BOTTOM = VG_DOWN,                    // 底部

    VG_CENTER_H = VG_LEFT | VG_RIGHT,       // 水平居中
    VG_CENTER_V = VG_UP | VG_DOWN,          // 垂直居中
    VG_CENTER = VG_CENTER_H | VG_CENTER_V,  // 居中

    VG_MIDDLE = 16,                         // 中 (鼠标)
};

// 错误
enum vgError
{
    VG_OK    =  0,
    VG_ERROR = -1,
};

// 键盘事件
typedef void(*VG_KEY_EVENT)(int key);

// 鼠标事件
typedef void(*VG_MOUSE_EVENT)(int x, int y, int button);

// 计时器事件
typedef void(*VG_TIMER_EVENT)(float delay);

// 窗口绘制事件
typedef void(*VG_PAINT_EVENT)();

//---------------------------------------------------------------------------
// Unicode 字符串类
//---------------------------------------------------------------------------

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
inline std::basic_string<_char_t> to_string(const T& value)
{
    std::basic_stringstream<_char_t> stm;
    stm << value;
    return stm.str();
}

// unicode to ansi
inline std::string to_ansi(const wchar_t* str, size_t length)
{
    std::vector<char> buf;
    int n = WideCharToMultiByte(CP_OEMCP, 0, str, (int) length, NULL, 0, NULL, FALSE);
    buf.resize(n);
    WideCharToMultiByte(CP_OEMCP, 0, str, (int) length, &buf[0], n, NULL, FALSE);
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
    unistring() : std::wstring() {}
    unistring(const char* str) : std::wstring() { assign(str, strlen(str)); }
    unistring(const char* str, size_t size) : std::wstring() { assign(str, size); }
    unistring(const std::string& str) : std::wstring() { assign(str.c_str(), str.length()); }

    unistring(const wchar_t* str) : std::wstring(str) {}
    unistring(const wchar_t* str, size_t size) : std::wstring(str, size) {}
    unistring(const std::wstring& str) : std::wstring(str) {}

    // 整数转字符串
    unistring(int n) : std::wstring(to_string<wchar_t>(n)) {}

    // 单实数转字符串
    unistring(float n) : std::wstring(to_string<wchar_t>(n)) {}

    // 双实数转字符串
    unistring(double n) : std::wstring(to_string<wchar_t>(n)) {}

    // 复制构造
    unistring(const unistring& str) : std::wstring(str.c_str(), str.length()) {}

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

    int    to_int() const { return string_cast<int>(*this); }       // 字符串转整数
    float  to_float() const { return string_cast<float>(*this); }   // 字符串转单实数
    double to_double() const { return string_cast<double>(*this); } // 字符串转双实数
};

//---------------------------------------------------------------------------
// 数学、几何
//---------------------------------------------------------------------------

#ifndef M_PI
#define M_PI        3.141592653589793238462     // acos(-1.0)
#endif

#ifndef M_PI_2
#define M_PI_2      1.570796326794896619231     // M_PI/2
#endif

#ifndef M_RD
#define M_RD        0.017453292519943295769     // 弧度 (radian)
#define M_INV_RD    57.295779513082320876798    // 弧度的倒数 (reciprocal) 1.0 / M_RD
#endif

// 判断数值是否是 0
template<typename T>inline bool is_zero        (T      n) { return n == 0; };
template<>          inline bool is_zero<float> (float  n) { return n < 0.0 ? (n > -FLT_EPSILON) : (n < FLT_EPSILON); }
template<>          inline bool is_zero<double>(double n) { return n < 0.0 ? (n > -DBL_EPSILON) : (n < DBL_EPSILON); }

// 判断数值是否相等
template<typename T>
inline bool is_equal(T a, T b)
{
    return is_zero(a - b);
}

// 产生 [0 ~ n] 之间的随机数
template<typename T>
inline int random(T n)
{
    return rand() % n;
}

// 产生 [0 ~ 1] 之间的随机浮点数
inline double rand_real()
{
    return double(rand()) / RAND_MAX;
}

// 产生 [a ~ b] 之间的随机浮点数
inline double rand_real(double a, double b)
{
    return a + (b - a) * rand_real();
}

// 计算点 [x, y] 到原点的角度
inline double degree_angle(double x, double y)
{
    using namespace std;

    return atan2(y, x) * M_INV_RD;
}

//---------------------------------------------------------------------------
// 向量类
//---------------------------------------------------------------------------

#if !defined(GLM_VERSION)

#define VEC2_OPERATION(op)\
    template<typename X>\
    vec2<T> operator op(const X& value) const\
    {\
        return vec2<T>(x op value, y op value);\
    }\
    template<typename X>\
    vec2<T> operator op(const vec2<X>& v) const\
    {\
        return vec2<T>(x op v.x, y op v.y);\
    }\
    template<typename X>\
    vec2<T>& operator op##=(const X& value)\
    {\
        x op##= value; y op##= value;\
        return *this;\
    }\
    template<typename X>\
    vec2<T>& operator op##=(const vec2<X>& v)\
    {\
        x op##= v.x; y op##= v.y;\
        return *this;\
    }

#define VEC4_OPERATION(op)\
    template<typename X>\
    vec4<T> operator op(const X& value) const\
    {\
        return vec4<T>(x op value, y op value, z op value, w op value);\
    }\
    template<typename X>\
    vec4<T> operator op(const vec4<X>& v) const\
    {\
        return vec4<T>(x op v.x, y op v.y, z op v.z, w op v.w);\
    }\
    template<typename X>\
    vec4<T>& operator op##=(const X& value)\
    {\
        x op##= value; y op##= value; z op##= value; w op##= value;\
        return *this;\
    }\
    template<typename X>\
    vec4<T>& operator op##=(const vec4<X>& v)\
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
    union {
        T data[2];
        struct { T x, y; };
    };

    vec2() : x(), y() {}
    vec2(T scalar) : x(scalar), y(scalar) {}
    vec2(T _x, T _y) : x(_x), y(_y) {}

    vec2& operator=(const vec2& other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    vec2& set(T _x, T _y) { x = _x; y = _y; return *this; }

    VEC2_OPERATION(+);
    VEC2_OPERATION(-);
    VEC2_OPERATION(*);
    VEC2_OPERATION(/);

          T& operator[](size_t i)       { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }

    bool operator==(const vec2& other) const
    {
        return is_equal(x, other.x) && is_equal(y, other.y);
    }

    bool operator!=(const vec2& other) const
    {
        return !is_equal(x, other.x) || !is_equal(y, other.y);
    }
};

template<typename T>
class vec4
{
public:
    union
    {
        T data[4];
        struct { T x, y, z, w; }; // 空间坐标
        struct { T b, g, r, a; }; // 颜色分量 (GDI 使用的是 BGRA)
    };

    vec4() : x(), y(), z(), w() {}
    vec4(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
    vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}

    vec4& set(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; return *this; }

          T& operator[](size_t i)       { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }
};

typedef vec2<int>       vec2i;
typedef vec2<float>     vec2f;
typedef vec2<double>    vec2d;

typedef vec4<uint8_t>   vec4ub;
typedef vec4<int>       vec4i;
typedef vec4<float>     vec4f;
typedef vec4<double>    vec4d;

// 获取向量距离原点的距离
template<typename T>
inline float length(const vec2<T>& v)
{
    using namespace std;

    return static_cast<T>(sqrt(v.x * v.x + v.y * v.y));
}

// 获取两个向量的距离
template<typename T>
inline float distance(const vec2<T>& v1, const vec2<T>& v2)
{
    return length(v2 - v1);
}

// 归一化向量
template<typename T>
inline vec2<T> normalize(const vec2<T>& v)
{
    float n = length(v);
    if (n == 0) {
        return v;
    }

    n = 1.0f / n;

    return vec2<T>(v.x * n, v.y * n);
}

// 旋转向量（角度）
template<typename T, typename A>
inline vec2<T> rotate(const vec2<T>& v, A angle)
{
    using namespace std;

    angle *= M_RD;
    T sine = static_cast<T>(sin(angle));
    T cosine = static_cast<T>(cos(angle));
    return vec2<T>(
        v.x * cosine - v.y * sine,
        v.x * sine + v.y * cosine);
}

// 计算角度
template<typename T>
inline T degree_angle(const vec2<T>& v)
{
    return degree_angle(v.x, v.y);
}

#else // GLM_VERSION

#if !defined(MATH_PUBLIC_H_20220126134219)

} // end namespace minivg

#include <glm/glm.hpp>

namespace cgl {

typedef glm::ivec2      vec2i;
typedef glm::vec2       vec2f;
typedef glm::dvec2      vec2d;

typedef glm::u8vec4     vec4ub;
typedef glm::ivec4      vec4i;
typedef glm::vec4       vec4f;
typedef glm::dvec4      vec4d;

} // end namespace cgl
namespace minivg {

#endif // GLM_HPP_20211019161738

using cgl::vec2i;
using cgl::vec2f;
using cgl::vec2d;

using cgl::vec4ub;
using cgl::vec4i;
using cgl::vec4f;
using cgl::vec4d;

#endif // GLM_VERSION

// clang-format on

// 矩形
class vgRect
{
public:
    float x;
    float y;
    float w;
    float h;

public:
    vgRect() : x(), y(), w(), h() { }

    vgRect(float x, float y, float width, float height) :
        x(x),
        y(y),
        w(width),
        h(height) { }

    // 返回点是否在矩形内
    bool contaits(float x, float y) const
    {
        return x > this->x &&
               y > this->y &&
               x < this->x + this->w &&
               y < this->y + this->h;
    }

    bool contaits(const vec2i& v) const
    {
        return this->contaits(v.x, v.y);
    }

    bool contaits(const vec2f& v) const
    {
        return this->contaits(v.x, v.y);
    }

    // 判断矩形是否相交
    bool collision(const vgRect& other) const
    {
        using namespace std;

        float left   = max(x, other.x);
        float top    = max(y, other.y);
        float right  = min(x + w, other.x + other.w);
        float bottom = min(y + h, other.y + other.h);

        return (right - left) > 0.0f && (bottom - top) > 0.0f;
    }
};

// 包围盒
class AABB
{
public:
    float x1, y1, x2, y2;

public:
    AABB()
    {
        reset();
    }

    AABB(float x1, float y1, float x2, float y2)
    {
        this->x1 = x1;
        this->y1 = y1;
        this->x2 = x2;
        this->y2 = y2;
    }

    // 重置
    void reset()
    {
        // 初始化一个无效的包围盒
        x1 = y1 = FLT_MAX;  // 左上角最大值
        x2 = y2 = -FLT_MAX; // 右下角最小值
    }

    // 判断包围盒面积是否为正数
    bool is_valid() const
    {
        return x1 < x2 && y1 < y2;
    }

    // 附加一个点，扩展包围盒
    void append(float x, float y)
    {
        using namespace std;

        x1 = min(x1, x);
        x2 = max(x2, x);
        y1 = min(y1, y);
        y2 = max(y2, y);
    }

    void append(const vec2f& v)
    {
        this->append(v.x, v.y);
    }

    // 返回宽度
    float width() const
    {
        return x2 - x1;
    }

    // 返回高度
    float height() const
    {
        return y2 - y1;
    }

    // 获取包围盒中心位置
    vec2f center() const
    {
        // 无效
        if (x1 > y2 || y2 < y1) {
            return vec2f();
        }

        return vec2f(x1 + (this->width() / 2), y1 + (this->height() / 2));
    }

    // 移动包围盒
    void move(float x, float y)
    {
        x1 += x;
        y1 += y;
        x2 += x;
        y2 += y;
    }

    void move(const vec2f& v)
    {
        this->move(v.x, v.y);
    }

    // 判断包围盒是否包含点
    bool contaits(float x, float y) const
    {
        return x >= x1 && y >= y1 && x < x2 && y < y2;
    }

    bool contaits(const vec2f& p) const
    {
        return this->contaits(p.x, p.y);
    }

    // 判断两个包围盒是否相交
    bool collision(const AABB& other) const
    {
        using namespace std;

        float left   = max(x1, other.x1);
        float top    = max(y1, other.y1);
        float right  = min(x2, other.x2);
        float bottom = min(y2, other.y2);

        return (right - left) > 0.0f && (bottom - top) > 0.0f;
    }
};

//---------------------------------------------------------------------------
// 图片类
//---------------------------------------------------------------------------

class vgImage
{
protected:
    Gdiplus::Bitmap* m_handle;   // 图片指针
    Gdiplus::BitmapData* m_data; // 图片 map 数据指针

public:
    vgImage();
    ~vgImage();

    // 返回图片的指针
    Gdiplus::Bitmap* handle() const;

    // 创建一个图片，默认为 32 位色
    int create(int width, int height, int format = VG_RGBA);

    // 打开一个图片，支持 bmp、jpg、png、静态 gif 等常见格式
    int open(const unistring& filename);

    // 打开资源中的图片
    int open(int id, PCTSTR resource_type = TEXT("BITMAP"));

    // 绑定 HBITMAP 对象，直接操作 HBITMAP。
    int bind(HBITMAP hbmp);

    // 判断图片是否为空
    bool empty() const;

    // 保存图片
    int save(const unistring& filename, int type = VG_PNG);

    // 自动释放图片
    void close();

    // 返回图片的宽度
    int width() const;

    // 返回图片的高度
    int height() const;

    // 获取图像数据指针
    void* map(bool readonly = false, int pixelformat = VG_RGBA);

    // 还原图像数据
    void unmap();
};

//---------------------------------------------------------------------------
// 主函数
//---------------------------------------------------------------------------

/* 图形库初始化
 * title            窗口标题
 * width            窗口宽度
 * height           窗口高度
 * param            参数：
 *                      VG_FIXED        固定大小
 *                      VG_SIZEABLE     可缩放
 *                      VG_FULLSCREEN   全屏
 *                      VG_BUFFER       只创建绘图缓冲区，不创建窗口
 */
int initgraph(int width, int height, int param = VG_FIXED);
int initgraph(const unistring& title, int width, int height, int param = VG_FIXED);

/* 在已有的窗口界面上初始化。
 * 可以和 vcl、mfc，或者其他界面库协同使用。（貌似只有顶层窗口能实现效果）
 * hwnd             要初始化的窗口句柄
 */
int initgraph(HWND hwnd);

// 退出
void quit();

// 获取主窗口句柄
HWND graph_window();

// 获取 GDI 绘图设备
HDC graph_hdc();

// 获取 GDI+ 绘图设备
Gdiplus::Graphics* graphics();

// 设置视口显示范围 (宽高改变会重设背景缓冲区大小)
void viewport(int x, int y, int width, int height);

// 返回视口宽度
int view_width();

// 返回视口宽度
int view_height();

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

// 消息循环处理，如果返回 false，表示程序退出。
bool do_events();

// 主进程执行函数
int start_app();

//---------------------------------------------------------------------------
// 窗口事件、输入按键管理
//---------------------------------------------------------------------------

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

// 键盘事件映射
void key_push_event(VG_KEY_EVENT function);
void key_pop_event(VG_KEY_EVENT function);
void input_event(VG_KEY_EVENT function);

// 鼠标事件映射
void mouse_push_event(VG_MOUSE_EVENT function);
void mouse_pop_event(VG_MOUSE_EVENT function);
void mouse_move_event(VG_MOUSE_EVENT function);

/* 设置计时器
 * interval         计时器时间间隔，单位毫秒，输入 0 停止计时器
 */
void start_timer(UINT interval);

// 关闭计时器
void stop_timer();

// 计时器事件
void timer_event(VG_TIMER_EVENT function);

// 窗口绘制事件
void display_event(VG_PAINT_EVENT function);

//---------------------------------------------------------------------------
// 绘图函数
//---------------------------------------------------------------------------

// 设置剪裁矩形
void cliprect(int x, int y, int width, int height);

// 压入剪裁矩形
void push_cliprect(int x, int y, int width, int height);

// 弹出剪裁矩形
void pop_cliprect();

// 背景缓冲绘制到目标 HDC
void framebuf_blt(HDC hdc);

// 设置显示质量
enum vgEffectLevel
{
    VG_SPEED,   // 速度优先
    VG_MEDIUM,  // 中等质量
    VG_QUALITY, // 质量优先
};

int effect_level(int level);

// 设置帧率
void set_fps(int value);

// 返回帧率
int fps();

// 清屏
void clear(BYTE r, BYTE g, BYTE b, BYTE a = 255);

// 获取画笔颜色
vec4ub pen_color();

/* 设置画笔颜色
 * r                红色分量
 * g                绿色分量
 * b                蓝色分量
 * a                透明度
 */
void pen_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void pen_color(COLORREF argb);
void pen_color(vec4ub rgba);

// 获取画笔宽度
float pen_width();

// 设置画笔宽度
void pen_width(float width);

// 画笔样式
enum vgPenStyle
{
    VG_SOLID,     // 实心画笔（默认）
    VG_DASH,      // -------
    VG_DOT,       // .......
    VG_DASH_DOT,  // -.-.-.-
    VG_CUSTOM = 5 // 自定义点画模式
};

// 返回画笔样式
vgPenStyle pen_style();

// 设置画笔样式
void pen_style(int mode);

/* 设置点画模图案样式，由一组浮点数代表点画模式的宽度。
 * dash             点画模式间隔，浮点数数组
 * size             浮点数数组大小
 */
void dash_style(const float* dash, int size);

// 获取填充颜色
vec4ub fill_color();

/* 设置填充颜色
 * r                红色分量
 * g                绿色分量
 * b                蓝色分量
 * a                透明度
 */
void fill_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void fill_color(COLORREF argb);
void fill_color(vec4ub rgba);

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

/* 通过圆心半径绘制空心椭圆
 * ox, oy           圆心坐标
 * rx, ry           椭圆的横向半径和纵向半径
 */
void draw_ellipse(float ox, float oy, float rx, float ry);

/* 通过圆心半径填充椭圆
 * ox, oy           圆心坐标
 * rx, ry           椭圆的横向半径和纵向半径
 */
void fill_ellipse(float ox, float oy, float rx, float ry);

/* 通过矩形范围绘制空心椭圆
 * x, y             矩形左上角坐标
 * width, height    矩形宽度高度
 */
void draw_ellipse_r(float x, float y, float width, float height);

/* 通过矩形范围填充椭圆
 * x, y             矩形左上角坐标
 * width, height    矩形宽度高度
 */
void fill_ellipse_r(float x, float y, float width, float height);

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
 * points           点数组
 * size             点数组大小
 */
void fill_polygon(const vec2f* points, size_t size);

//---------------------------------------------------------------------------
// 字体函数
//---------------------------------------------------------------------------

// 字体样式，可以任意组合
enum vgFontStyle
{
    VG_NORMAL    = 0, // 普通字体
    VG_BOLD      = 1, // 粗体
    VG_ITALIC    = 2, // 斜体
    VG_UNDERLINE = 4, // 下划线
    VG_STRIKEOUT = 8  // 删除线
};

// 获取字体名称
unistring font_name();

// 获取字体大小
int font_size();

// 获取字体样式
int font_style();

/* 设置字体
 * name             字体名称
 * size             字体大小
 * style            字体样式，EZGDI_FONTSTYLE 类型的组合
 */
void setfont(const unistring& name, float size, vgFontStyle style = VG_NORMAL);

/* 设置字体
 * name             字体名称
 * size             字体大小
 * bold             是否粗体
 * underline        是否下划线
 * strikeout        是否删除线
 */
void setfont(const unistring& name, float size, bool bold, bool, bool underline, bool strikeout);

// 设置字体属性
void font_name(const unistring& name);
void font_size(float size);
void font_style(int style);

// 获取文字颜色
vec4ub text_color();

/* 设置文字颜色
 * r                红色分量
 * g                绿色分量
 * b                蓝色分量
 * a                透明度
 */
void text_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void text_color(COLORREF abgr);
void text_color(vec4ub rgba);

// 输出文字
void textout(float x, float y, const char* text, size_t length);
void textout(float x, float y, const wchar_t* text, size_t length);
void textout(float x, float y, const unistring& text);

/* 格式化输出文字
 * x, y             文字输出矩形
 * width, height
 * text             文字
 * align            对齐方式 (VG_LEFT, VG_RIGHT, VG_TOP, VG_DOWN 以及组合)
 */
void drawtext(float x, float y, float width, float height, const unistring& text, int align = 0);

/* 文字格式化输出，和 printf 使用类似
 * x, y             绘制的字符串坐标
 * param            格式化字符串
 * ...              可变参数
 */
void print(float x, float y, const char* param, ...);
void print(float x, float y, const wchar_t* param, ...);

// 获取字符串的像素宽度
float textwidth(const unistring& text);

// 获取字符串的像素高度
float textheight(const unistring& text);

//---------------------------------------------------------------------------
// 图片操作
//---------------------------------------------------------------------------

/* 创建图片
 * width            宽度
 * height           高度
 */
vgImage* newimage(int width, int height);

/* 删除图片
 * image            图片指针
 */
void freeimage(vgImage* image);

// 加载图片。（可以不用关心释放，未释放的图片，会在程序结束前全部释放）
vgImage* loadimage(const unistring& filename);

// 加载资源中的图片
vgImage* loadimage(int id, PCTSTR resource_type = TEXT("PNG"));

/* 保存图片
 * image            要保存的图片
 * filename         png 图片文件名
 */
int saveimage(vgImage* image, const unistring& filename);

/* 原始大小绘制图片
 * xy               图片绘制位置
 */
void drawimage(vgImage* image, float x, float y);

/* 根据位置范围绘制图片
 * x, y             图片绘制位置
 * width            绘制的宽度
 * height           绘制的高度
 */
void drawimage(vgImage* image, float x, float y, float width, float height);

// 绘制精灵图片，支持旋转、缩放、镜像。
void drawsprite(
    vgImage* image,                        // 绘制的图片
    float sourceX, float sourceY,          // 图片源位置
    float sourceWidth, float sourceHeight, // 图片源大小
    float x, float y,                      // 目标位置
    float scaleX, float scaleY,            // 缩放
    float rotation,                        // 旋转角度
    float centerX = 0.5f,                  // 旋转中心(0.0 - 1.0 之间，按源大小比例设置旋转中心)
    float centerY = 0.5f,
    float alpha   = 1.0f
);

void drawsprite(
    vgImage* image,             // 绘制的图片
    float x, float y,           // 目标位置
    float scaleX, float scaleY, // 缩放
    float rotation,             // 旋转角度
    float centerX = 0.5f,       // 旋转中心(0.0 - 1.0 之间，按源大小比例设置旋转中心)
    float centerY = 0.5f
);

/* 把像素直接绘制到屏幕上，像素格式必须是 BGRA 32位。
 * x, y             像素绘制位置
 * width            绘制的宽度
 * height           绘制的高度
 * pixels           绘制的像素指针
 * imageWidth       图片像素的宽度
 * imageHeight      图片像素的高度
 */
void draw_pixels(float x, float y, float width, float height, const void* pixels, int imageWidth, int imageHeight);

//---------------------------------------------------------------------------
// 多媒体
//---------------------------------------------------------------------------

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

// 停止所有声音播放
void stop_sound();

//---------------------------------------------------------------------------
// 界面
//---------------------------------------------------------------------------

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

// 显示一个图片
// void imshow(mat image);

} // end namespace minivg

#include "minivg.inl"

using namespace minivg;

#endif // MINIVG_HPP
