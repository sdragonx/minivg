/*
 Copyright (c) 2005-2020 sdragonx (mail:sdragonx@foxmail.com)

 minivg.hpp

 2020-01-01 16:37:22

 EZGDI ��һ���Ƚϼ򵥡���ѧ�����õ� C++ �⣬���Ŀ��ּ�ڰ�����ѧ��ѧϰʹ�� C++��
 ��ӭ���� MINIVG��Ҳ��ӭ�����ҵ� GITHUB ������������
 https://github.com/sdragonx/ezgdi

 ������õ� Visual Studio������ⲻ��Ҫ�������þ����ã�
 ����õ��� g++ �������� ���� DevCPP����Ҫ�ڱ������������� -finput-charset=GBK ��֧�������ַ�
 ������������� -lgdi32 -lgdiplus ����������á�

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
 *                                   ͨ��                                   *
 *                                                                          *
 ****************************************************************************/

namespace minivg{

// enum ����
enum{
    EZ_NULL,

    EZ_FIXED = 1,                           //�̶���С
    EZ_SIZEABLE = 2,                        //������
    EZ_FULLSCREEN = 4,                      //ȫ��
    EZ_BACKBUFFER = 8,                      //����������

    EZ_LEFT   = 1,                          //��
    EZ_RIGHT  = 2,                          //��
    EZ_UP     = 4,                          //��
    EZ_DOWN   = 8,                          //��
    EZ_TOP    = EZ_UP,                      //����
    EZ_BOTTOM = EZ_DOWN,                    //�ײ�

    EZ_CENTER_H = EZ_LEFT | EZ_RIGHT,       //ˮƽ����
    EZ_CENTER_V = EZ_UP | EZ_DOWN,          //��ֱ����
    EZ_CENTER = EZ_CENTER_H | EZ_CENTER_V,  //����

    EZ_MIDDLE   = 16,                       //��

    EZ_RGB,                                 //RGB ��ɫ
    EZ_RGBA,                                //RGBA ��ɫ

    EZ_OK = 0,
    EZ_ERROR = -1,
};

// �����¼�
typedef void (*EZ_KEY_EVENT)(int key);

// ����¼�
typedef void (*EZ_MOUSE_EVENT)(int x, int y, int button);

// ��ʱ���¼�
typedef void (*EZ_TIMER_EVENT)();

// ���ڻ����¼�
typedef void (*EZ_PAINT_EVENT)();

/****************************************************************************
 *                                                                          *
 *                             Unicode�ַ�����                              *
 *                                                                          *
 ****************************************************************************/

// �ַ���ת��������
template<typename T, typename _char_t>
inline T string_cast(const std::basic_string<_char_t>& str)
{
    std::basic_stringstream<_char_t> stm(str);
    T n;
    stm >> n;
    return n;
}

// ��������ת�ַ���
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

    // ����ת�ַ���
    unistring(int n) : std::wstring(to_string<wchar_t>(n)) { }

    // ��ʵ��ת�ַ���
    unistring(float n) : std::wstring(to_string<wchar_t>(n)) { }

    // ˫ʵ��ת�ַ���
    unistring(double n) : std::wstring(to_string<wchar_t>(n)) { }

    // ���ƹ���
    unistring(const unistring& str) : std::wstring(str.c_str(), str.length()) { }

    using std::wstring::assign;

    // ���ַ��ַ�����ֵ
    unistring& assign(const char* str, size_t size = -1)
    {
        std::vector<wchar_t> buf;
        int n = MultiByteToWideChar(CP_ACP, 0, str, int(size), NULL, 0);
        buf.resize(n);
        MultiByteToWideChar(CP_ACP, 0, str, int(size), &buf[0], n);
        std::wstring::assign(buf.begin(), buf.end());
        return *this;
    }

    int    to_int()const    { return string_cast<int>(*this);    }// �ַ���ת����
    float  to_float()const  { return string_cast<float>(*this);  }// �ַ���ת��ʵ��
    double to_double()const { return string_cast<double>(*this); }// �ַ���ת˫ʵ��
};

/****************************************************************************
 *                                                                          *
 *                                ��ѧ������                                *
 *                                                                          *
 ****************************************************************************/

#ifndef M_PI
  #define M_PI      3.141592653589793238462     // acos(-1.0)
#endif

#ifndef M_PI_2
    #define M_PI_2    1.570796326794896619231   // M_PI/2
#endif

#ifndef M_RD
  #define M_RD       0.017453292519943295769    // ����(radian)
  #define M_INV_RD  57.295779513082320876798    // ���ȵĵ���(reciprocal) 1.0/M_RD
#endif

// �ж���ֵ�Ƿ��� 0
template<typename T>inline bool is_zero(T n) { return n == 0; };
template<>inline bool is_zero<float>(float n) { return n < 0.0 ? (n > -FLT_EPSILON) : (n < FLT_EPSILON); }
template<>inline bool is_zero<double>(double n) { return n < 0.0 ? (n > -DBL_EPSILON) : (n < DBL_EPSILON); }

// �ж���ֵ�Ƿ����
template<typename T>
inline bool is_equal(T a, T b) { return is_zero(a - b); }

// ���� 0 ~ n ֮��������
template<typename T>
inline int random(T n) { return rand() % n; }

// ���� 0 ~ 1 ֮������������
inline double rand_real() { return double(rand()) / RAND_MAX; }

// ���� minVal ~ maxVal ֮������������
inline double rand_real(double minVal, double maxVal)
{
    return minVal + (maxVal - minVal) * rand_real();
}

// ��������Ļ���
inline double radian(double x, double y)
{
    double n = 0.0;
    if(!is_zero(y)){
        n = M_PI_2 - std::atan(x / y);//����б���󻡶ȣ�����
        if(y < 0.0)n += M_PI;
    }
    return n;
}

// ͨ��xy��ýǶ�
inline double angle(double x, double y)
{
    return radian(x, y) * M_INV_RD;
}

// �� source �ݽ��� dest������Ϊ speed
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
 *                                  ������                                  *
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
        struct{ T x, y, z, w; }; // �ռ�����
        struct{ T b, g, r, a; }; // ��ɫ���� (GDI ʹ�õ��� BGRA)
        struct{ T _x, _y, width, height; }; // ����
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
 *                                  ͼƬ��                                  *
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

    // ����һ��ͼƬ��Ĭ��Ϊ32λɫ
    int create(int width, int height, int format = PixelFormat32bppARGB);

    // ��һ��ͼƬ��֧�� bmp��jpg��png����̬gif �ȳ�����ʽ
    int open(const unistring& filename);

    // ����Դ�е�ͼƬ
    int open(int id, PCTSTR resource_type = TEXT("BITMAP"));

    // ӳ��һ�� HBITMAP ����
    int map(HBITMAP hbmp);

    // �ж�ͼƬ�Ƿ�Ϊ��
    bool empty()const;

    // ����ͼƬ
    int save(const unistring& filename);

    // �Զ��ͷ�ͼƬ
    void close();

    // ����ͼƬ�Ŀ��
    int width()const;

    // ����ͼƬ�ĸ߶�
    int height()const;

    // ��ȡͼ������
    void* lock(int pixelformat = EZ_RGBA);

    // ��ԭͼ������
    void unlock();
};

/****************************************************************************
 *                                                                          *
 *                                  ������                                  *
 *                                                                          *
 ****************************************************************************/

/*

ʾ�����룺
int main(int argc, char* argv[])
{
    // ��ʼ���⣬��������
    initgraph("���ڱ���", 800, 600);

    while(do_events()){
      //TODO : �����ͼ����

    }
}

ʾ�����룺
void display();

int main(int argc, char* argv[])


    // ��ʼ���⣬��������
    initgraph("���ڱ���", 800, 600);

    // ���û�ͼ�¼�����
    display_event(display);

    // ������ִ�к���
    return start_app();
}

void display()
{
    //TODO : �����ͼ����
}

***/

/* ͼ�ο��ʼ��
 * title            ���ڱ���
 * width            ���ڿ��
 * height           ���ڸ߶�
 * param            ������
 *                      EZ_FIXED        �̶���С
 *                      EZ_SIZEABLE     ������
 *                      EZ_FULLSCREEN   ȫ��
 *                      EZ_BUFFER       ֻ������ͼ������������������
 */
int initgraph(int width, int height, int param = EZ_FIXED);
int initgraph(const unistring& title, int width, int height, int param = EZ_FIXED);

/* �����еĴ��ڽ����ϳ�ʼ����
 * ���Ժ�vcl��mfc���������������Эͬʹ�á���ò��ֻ�ж��㴰����ʵ��Ч����
 * hwnd             Ҫ��ʼ���Ĵ��ھ��
 */
int initgraph(HWND hwnd);

/* �˳�
 */
void quit();

/* ��������ھ��
 */
HWND graph_window();

/* ��ȡ GDI ��ͼ�豸
 */
HDC graph_hdc();

/* ���ô��ڱ���
 * text             ����
 */
void set_title(const unistring& text);

/* �������ô��ڴ�С���ͻ�����С��
 * width            ���ڿ��
 * height           ���ڸ߶�
 */
void reshape(int width, int height);

/* ����ȫ��
 * value    true ȫ����false ȡ��ȫ��
 */
void fullscreen(bool value);

/* ��Ϣѭ������������� false����ʾ�����˳���
 */
bool do_events();

/* ������ִ�к���
 */
int start_app();

/****************************************************************************
 *                                                                          *
 *                          �����¼������밴������                          *
 *                                                                          *
 ****************************************************************************/

/* �жϰ����Ƿ���
 * ���ð�����
 * VK_ESCAPE        ESC
 * VK_RETURN        �س�
 * VK_LEFT          �����
 * VK_RIGHT         �ҷ����
 * VK_UP            �Ϸ����
 * VK_DOWN          �·����
 * VK_SPACE         �ո�
 */
bool keystate(int key);

/* �����¼�ӳ��
 */
void key_push_event(EZ_KEY_EVENT function);
void key_pop_event(EZ_KEY_EVENT function);
void input_event(EZ_KEY_EVENT function);

/* ����¼�ӳ��
 */
void mouse_push_event(EZ_MOUSE_EVENT function);
void mouse_pop_event(EZ_MOUSE_EVENT function);
void mouse_move_event(EZ_MOUSE_EVENT function);

/* ���ü�ʱ��
 * interval         ��ʱ��ʱ��������λ���룬���� 0 ֹͣ��ʱ��
 */
void start_timer(UINT interval);

/* �رռ�ʱ��
 */
void stop_timer();

/* ��ʱ���¼�
 */
void timer_event(EZ_TIMER_EVENT function);

/* ���ڻ����¼�
 */
void display_event(EZ_PAINT_EVENT function);

/****************************************************************************
 *                                                                          *
 *                                 ��ͼ����                                 *
 *                                                                          *
 ****************************************************************************/

/* ��� GDI+ ��ͼ�豸
 */
Gdiplus::Graphics* graphics();

/* ���豳����������Сλ��
 *
 */
void viewport(int x, int y, int width, int height);

/* ����������Ƶ�Ŀ�� HDC
 */
void framebuf_blt(HDC hdc);

/* ������ʾ����
 */
enum {
    EZ_SPEED,   //�ٶ�����
    EZ_MEDIUM,  //�е�����
    EZ_QUALITY, //��������
};

int effect_level(int level);

/* ����
 */
void clear(BYTE r, BYTE g, BYTE b, BYTE a = 255);

/* ���Ļ�����ɫ
 * r                ��ɫ����
 * g                ��ɫ����
 * b                ��ɫ����
 * a                ͸����
 */
void pen_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void pen_color(COLORREF argb);
void pen_color(vec4ub color);

/* ��ȡ������ɫ
 */
COLORREF pen_color();

/* ���ʿ��
 */
void pen_width(float width);

/* ����ģʽ
 */
enum{
    EZ_SOLID,       // ʵ�Ļ��ʣ�Ĭ�ϣ�
    EZ_DASH,        // -------
    EZ_DOT,         // .......
    EZ_DASH_DOT,    // -.-.-.-
    EZ_CUSTOM = 5   // �Զ���
};

/* ���û���ģʽ
 */
void pen_style(int mode);

/* ���õ㻭ģʽ�������һ�鸡��������㻭ģʽ�Ŀ�ȡ�
 * dash             �㻭ģʽ���������������
 * size             �����������С
 */
void dash_style(const float* dash, int size);

/* ���������ɫ
 * r                ��ɫ����
 * g                ��ɫ����
 * b                ��ɫ����
 * a                ͸����
 */
void fill_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void fill_color(COLORREF argb);

/* ��ȡ�����ɫ
 */
COLORREF fill_color();

/* ����һ����
 * x                x ��������
 * y                y ��������
 */
void draw_point(float x, float y);

/* �����߶�
 * x1, y1           ��һ����
 * x2, y2           �ڶ�����
 */
void draw_line(float x1, float y1, float x2, float y2);

/* ����һ�����ľ���
 * x, y             ���Ͻ�����
 * width            ���ο��
 * height           ���θ߶�
 */
void draw_rect(float x, float y, float width, float height);

/* ���һ������
 * x, y             ���Ͻ�����
 * width            ���ο��
 * height           ���θ߶�
 */
void fill_rect(float x, float y, float width, float height);

/* ����Բ�Ǿ���
 * x, y             ���Ͻ�����
 * width            ���ο��
 * height           ���θ߶�
 * cx, cy           Բ��Բ�δ�С
 */
void draw_roundrect(float x, float y, float width, float height, float cx, float cy);

/* ���Բ�Ǿ���
 * x, y             ���Ͻ�����
 * width            ���ο��
 * height           ���θ߶�
 * cx, cy           Բ��Բ�δ�С
 */
void fill_roundrect(float x, float y, float width, float height, float cx, float cy);

/* ���ƿ�����Բ
 * x, y             Բ������
 * cx, cy           ��Բ�ĺ���뾶������뾶
 */
void draw_ellipse(float x, float y, float cx, float cy);

/* �����Բ
 * x, y             Բ������
 * cx, cy           ��Բ�ĺ���뾶������뾶
 */
void fill_ellipse(float x, float y, float cx, float cy);

/* ���ƿ���Բ��xyΪԲ��
 * x, y             Բ������
 * r                Բ�뾶
 */
void draw_circle(float x, float y, float r);

/* ���ƿ���Բ��xyΪԲ��
 * x, y             Բ������
 * r                Բ�뾶
 */
void fill_circle(float x, float y, float r);

/* �����������߶�
 * points           ������
 * size             �������С
 */
void draw_polyline(const vec2f* points, size_t size);

/* ���ƶ����
 * points           ������
 * size             �������С
 */
void draw_polygon(const vec2f* points, size_t size);

/* �������
 * points               ������
 * size             �������С
 */
void fill_polygon(const vec2f* points, size_t size);

/****************************************************************************
 *                                                                          *
 *                                 ���庯��                                 *
 *                                                                          *
 ****************************************************************************/

/* ������ʽ�������������
 */
enum EZGDI_FONTSTYLE{
    EZ_NORMAL       = 0,    //��ͨ����
    EZ_BOLD         = 1,    //����
    EZ_ITALIC       = 2,    //б��
    EZ_UNDERLINE    = 4,    //�»���
    EZ_STRIKEOUT    = 8     //ɾ����
};

/* ��������
 * name             ��������
 * size             �����С
 * style            ������ʽ��EZGDI_FONTSTYLE ���͵����
 */
void setfont(const unistring& name, float size, EZGDI_FONTSTYLE style = EZ_NORMAL);

/* ��������
 * name             ��������
 * size             �����С
 * bold             �Ƿ����
 * underline        �Ƿ��»���
 * strikeout        �Ƿ�ɾ����
 */
void setfont(const unistring& name, float size, bool bold, bool, bool underline, bool strikeout);

/* ������������
 */
void font_name(const unistring& name);
void font_size(float size);
void font_style(int style);

/* ��ȡ��������
 */
unistring font_name();
int font_size();
int font_style();

/* ������ɫ
 * r                ��ɫ����
 * g                ��ɫ����
 * b                ��ɫ����
 * a                ͸����
 */
void font_color(BYTE r, BYTE g, BYTE b, BYTE a = 255);
void font_color(COLORREF color);

//�������
void textout(float x, float y, const char* text, size_t length);
void textout(float x, float y, const wchar_t* text, size_t length);
void textout(float x, float y, const unistring& text);

void drawtext(float x, float y, float width, float height, const unistring& text, int align = 0);

/* �����ʽ���������printfʹ������
 * x, y             ���Ƶ��ַ�������
 * param            ��ʽ���ַ���
 * ...              �ɱ����
 */
void print(float x, float y, const char* param, ...);
void print(float x, float y, const wchar_t* param, ...);

/* ����ַ��������ؿ��
 */
float textwidth(const unistring& text);

/* ����ַ��������ظ߶�
 */
float textheight(const unistring& text);

/****************************************************************************
 *                                                                          *
 *                                 ͼƬ����                                 *
 *                                                                          *
 ****************************************************************************/

/* ����ͼƬ
 * width            ���
 * height           �߶�
 */
ezImage* newimage(int width, int height);

/* ɾ��ͼƬ
 * image            ͼƬָ��
 */
void freeimage(ezImage* image);

/* ����ͼƬ�����ù����ͷ�
 */
ezImage* loadimage(const unistring& filename);

/* ������Դ�е�ͼƬ
 */
ezImage* loadimage(int id, PCTSTR resource_type = TEXT("PNG"));

/* ����ͼƬ
 * image            Ҫ�����ͼƬ
 * filename         png ͼƬ�ļ���
 */
int saveimage(ezImage* image, const unistring& filename);

/* ԭʼ��С����ͼƬ
 * xy               ͼƬ����λ��
 */
void drawimage(ezImage* image, float x, float y);

/* ����λ�÷�Χ����ͼƬ
 * x, y             ͼƬ����λ��
 * width            ���ƵĿ��
 * height           ���Ƶĸ߶�
 */
void drawimage(ezImage* image, float x, float y, float width, float height);

/* ԭʼ��С��ת����ͼƬ����ת������ͼƬ���ġ�
 * x, y             ͼƬ����λ��
 * rotate           ��ת�ĽǶȣ���λ�ǽǶ�
 */
void rotate_image(ezImage* image, float x, float y, float rotate);

/* ����λ�÷�Χ��ת����ͼƬ����ת������ͼƬ���ġ�
 * x, y             ͼƬ����λ��
 * width            ���ƵĿ��
 * height           ���Ƶĸ߶�
 * rotate           ��ת�ĽǶȣ���λ�ǽǶ�
 */
void rotate_image(ezImage* image, float x, float y, float width, float height, float rotate);

/* ������ֱ�ӻ��Ƶ���Ļ�ϣ����ظ�ʽ������ BGRA 32λ��
 * x, y             ���ػ���λ��
 * width            ���ƵĿ��
 * height           ���Ƶĸ߶�
 * pwidth           ͼƬ���صĿ��
 * pheight          ͼƬ���صĸ߶�
 */
void draw_pixels(float x, float y, float width, float height, void* pixels, int pwidth, int pheight);

/****************************************************************************
 *                                                                          *
 *                                  ��ý��                                  *
 *                                                                          *
 ****************************************************************************/

/* ���ű�������
 * filename         �����ļ�·��
 */
void play_music(PCTSTR filename);

/* ������Դ�е�����
 * filename         �����ļ���Դ����
 * resource_type    �Զ�����Դ��������
 */
void play_resource_music(PCTSTR filename, PCTSTR resource_type = TEXT("MP3"));

/* ������Դ�е�����
 * id               �����ļ���Դ ID
 * resource_type    �Զ�����Դ��������
 */
void play_resource_music(int id, PCTSTR resource_type = TEXT("MP3"));

/* ֹͣ��������
 */
void stop_music();


/* ���� wav �ļ�
 * filename         wav �ļ�·��
 * loop             �Ƿ�ѭ��
 */
int play_sound(PCTSTR filename, bool loop = false);

/* ������Դ�е�wav�ļ�
 * filename         wav ��Դ����
 * loop             �Ƿ�ѭ��
 */
int play_resource_sound(PCTSTR filename, bool loop = false);

/* ������Դ�е�wav�ļ�
 * id               wav ��Դ ID
 * loop             �Ƿ�ѭ��
 */
int play_resource_sound(int id, bool loop = false);

/* ֹͣ������������
 */
void stop_sound();

/****************************************************************************
 *                                                                          *
 *                                   ����                                   *
 *                                                                          *
 ****************************************************************************/

/* ��Ϣ�Ի���
 * msg              ��Ϣ�ı�
 * title            ��Ϣ���ڱ���
 * type             ��ť����
 * ����ֵ           ���ص���İ�ť ID
 */
int msgbox(const unistring& message, const unistring& title = L"��Ϣ", int type = MB_OK);

/* ��ʾ�����
 * message          ��Ϣ�ı�
 * title            ��������
 * default_value    Ĭ��ֵ
 * ����ֵ           �ɹ����룬���طǿյ��ַ���
 */
unistring inputbox(const unistring& message, const unistring& title = L"����", const unistring& default_value = unistring());

/* ��ʾһ��ͼƬ
 */
//void imshow(mat image);

}// end namespace minivg

#include "minivg.inl"

using namespace minivg;

#endif// MINIVG_HPP
