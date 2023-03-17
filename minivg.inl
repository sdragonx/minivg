/*
 Copyright (c) 2005-2020 sdragonx (mail:sdragonx@foxmail.com)

 minivg.inl

 2020-07-08 23:31:53

*/
#ifndef MINIVG_INL_20200708233153
#define MINIVG_INL_20200708233153

//#include "minivg.hpp"

#include <stdio.h>
#include <tchar.h>
#include <time.h>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <ole2.h>               // CreateStreamOnHGlobal

#if __cplusplus < 201103L
    #define nullptr NULL
#endif

#ifdef __GNUC
    #define __MT__
#endif

#ifdef _MSC_VER
    #pragma warning(disable: 4717)
#endif

#include <process.h>

#if defined(__BORLANDC__) || defined(_MSC_VER)
#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ole32.lib")
#endif

#define MINIVG_VERSION       PCWSTR(L"minivg 2021-07-30")

#define MINIVG_CLASS_NAME    PCWSTR(L"minivg_window")
#define MINIVG_DEFAULT_FONT  PCWSTR(L"msyh")                // 微软雅黑

#define MINIVG_TIMER_ID      1

#define MINIVG_INLINE inline

namespace minivg {
namespace detail {

// 获取浮点时间戳
inline float tick_time()
{
    return static_cast<float>(clock()) * 0.001f;
}

//---------------------------------------------------------------------------
//
// 资源管理类
//
//---------------------------------------------------------------------------

// 安全删除指针
template<typename T>
void safe_delete(T* &p)
{
    if (p) {
        delete p;
        p = NULL;
    }
}

template<typename T>
void delete_object(T& obj)
{
    DeleteObject(obj);
    obj = NULL;
}

// 删除stl容器里面的内容
template<typename T>
void delete_all(T& obj)
{
    typename T::iterator itr = obj.begin();
    for (; itr != obj.end(); ++itr) {
        delete itr->second;
    }
    obj.clear();
}

// 创建一个 HBITMAP
MINIVG_INLINE HBITMAP bm_create(int width, int height, int pixelbits = 32)
{
    HBITMAP hBitmap;
    BITMAPV5HEADER bi;
    void *lpBits = 0;

    ZeroMemory(&bi, sizeof(BITMAPV5HEADER));
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = width;
    bi.bV5Height = height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = pixelbits;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    hBitmap = CreateDIBSection(GetDC(nullptr), (BITMAPINFO *) &bi, DIB_RGB_COLORS, (void **) &lpBits, nullptr, (DWORD) 0);

    return hBitmap;
}

// 资源管理类
class ezResource
{
private:
    std::map<unistring, ezImage*> images;       // 加载的图片
    std::map<int, ezImage*> resource_images;    // 加载的资源图片
    std::vector<ezImage*> image_pool;           // 创建的图片

public:
    // 加载一个图片
    ezImage* loadimage(const unistring& name)
    {
        ezImage* bmp = nullptr;
        std::map<unistring, ezImage*>::iterator itr;
        itr = images.find(name);
        if (itr == images.end()) {
            bmp = new ezImage;
            if (bmp->open(name) == EZ_OK) {
                images[name] = bmp;
            }
            else {
                delete bmp;
            }
        }
        else {
            bmp = itr->second;
        }
        return bmp;
    }

    // 加载资源图片
    ezImage* loadimage(int id, PCTSTR resource_type)
    {
        ezImage* bmp = nullptr;
        std::map<int, ezImage*>::iterator itr;
        itr = resource_images.find(id);
        if (itr == resource_images.end()) {
            bmp = new ezImage;
            if (bmp->open(id, resource_type) == EZ_OK) {
                resource_images[id] = bmp;
            }
            else {
                delete bmp;
            }
        }
        else {
            bmp = itr->second;
        }
        return bmp;
    }

    // 创建图片
    ezImage* allocate(int width, int height)
    {
        ezImage *image = new ezImage();
        image->create(width, height);
        image_pool.push_back(image);
        return image;
    }

    // 删除图片
    void free(ezImage* image)
    {
        std::vector<ezImage*>::iterator itr = std::find(
            image_pool.begin(), image_pool.end(), image);
        if (itr != image_pool.end()) {
            delete *itr;
            image_pool.erase(itr);
        }
    }

    // 释放所有资源，这个函数在程序退出的时候执行
    void dispose()
    {
        delete_all(images);
        delete_all(resource_images);

        for (size_t i = 0; i < image_pool.size(); ++i) {
            delete image_pool[i];
        }
        image_pool.clear();
    }
};

//---------------------------------------------------------------------------
//
// 界面
//
//---------------------------------------------------------------------------

class ezWindow
{
protected:
    HWND m_handle;

public:
    ezWindow() : m_handle() {}

    HWND handle()const { return m_handle; }

    int create(
        PCWSTR className,
        PCWSTR title,
        int x, int y, int width, int height,
        DWORD style = WS_OVERLAPPEDWINDOW,
        DWORD styleEx = 0)
    {
        // 2023-03-12 23:08:02 bug
        /*static bool is_init = false;
        if (!is_init) {
            InitClass(className, 0, basic_wndproc);
            is_init = true;
        }*/
        init_window_class(className, basic_wndproc);

        #ifndef UNICODE
        std::string buf = to_ansi(title, -1);
        title = (PCWSTR) buf.c_str();
        #endif

        // 获取当前进程句柄
        HINSTANCE hModule = GetModuleHandle(nullptr);

        // 创建窗口
        m_handle = CreateWindowExW(
            styleEx,        // 窗口的扩展风格
            className,      // 类名
            title,          // 标题
            style,          // 风格
            x,              // 左边位置
            y,              // 顶部位置
            width,          // 宽度
            height,         // 高度
            nullptr,        // 父窗口的句柄
            nullptr,        // 菜单的句柄或是子窗口的标识符
            hModule,        // 应用程序实例的句柄
            this);          // 指向窗口的创建数据

        if (m_handle == nullptr) {
            MessageBox(nullptr, TEXT("Window Creation Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
            return -1;
        }

        return 0;
    }

    // 关闭窗口
    void close()
    {
        SendMessage(m_handle, WM_CLOSE, 0, 0);
    }

    // 设置窗口范围
    void setBounds(int x, int y, int width, int height)
    {
        MoveWindow(m_handle, x, y, width, height, TRUE);
    }

    // 移动窗口
    void move(int x, int y)
    {
        SetWindowPos(m_handle, nullptr, x, y, 0, 0, SWP_NOSIZE);
    }

    // 设置窗口大小
    void resize(int width, int height)
    {
        SetWindowPos(m_handle, nullptr, 0, 0, width, height, SWP_NOMOVE);
    }

    // 显示窗口
    void show()
    {
        ShowWindow(m_handle, SW_SHOW);
    }

    // 隐藏窗口
    void hide()
    {
        ShowWindow(m_handle, SW_HIDE);
    }

    int showModel(HWND owner)
    {
        if (!m_handle) {
            return 0;
        }
        ShowWindow(m_handle, SW_SHOW);
        UpdateWindow(m_handle);
        if (owner) {
            SetWindowLongPtr(m_handle, GWLP_HWNDPARENT, (LONG_PTR) owner);
            EnableWindow(owner, FALSE);
        }

        MSG msg;
        while (GetMessage(&msg, 0, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (owner) {
            EnableWindow(owner, TRUE);
            SetForegroundWindow(owner);
        }

        return int(msg.wParam);
    }

    void setID(int id) { SetWindowLong(m_handle, GWL_ID, id); }

    void setTitle(const unistring& text)
    {
        #ifdef UNICODE
        SetWindowTextW(m_handle, text.c_str());
        #else
        std::string buf = to_ansi(text.c_str(), text.length());
        SetWindowTextW(m_handle, (PCWSTR) buf.c_str());
        #endif
    }

    unistring title()const
    {
        #ifdef UNICODE
        int size = GetWindowTextLengthW(m_handle);
        std::vector<wchar_t> buf;
        buf.resize(size + 1);
        GetWindowTextW(m_handle, &buf[0], size + 1);
        return unistring(&buf[0], size);
        #else
        int size = GetWindowTextLengthW(m_handle);
        std::vector<char> buf;
        buf.resize(size + 1);
        GetWindowTextW(m_handle, (wchar_t*) &buf[0], size + 1);
        return unistring(&buf[0], size);
        #endif
    }

    void setText(const unistring& text)
    {
        SetWindowTextW(m_handle, text.c_str());
    }

    unistring text()const
    {
        int size = GetWindowTextLengthW(m_handle);
        std::vector<wchar_t> buf;
        buf.resize(size + 1);
        GetWindowTextW(m_handle, &buf[0], size + 1);
        return unistring(&buf[0], size);
    }

    LRESULT send(int id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return SendDlgItemMessage(m_handle, id, msg, wParam, lParam);
    }

    void setFont(HFONT hFont)
    {
        SendMessage(m_handle, WM_SETFONT, (WPARAM) hFont, TRUE);
    }

    //这个函数不工作
    HFONT getFont()
    {
        return (HFONT) (UINT_PTR) SendMessage(m_handle, WM_GETFONT, 0, 0);
    }

    void repaint()
    {
        RECT rc;
        GetClientRect(m_handle, &rc);
        RedrawWindow(m_handle, &rc, 0, RDW_UPDATENOW | RDW_INVALIDATE | RDW_NOERASE);
    }

protected:
    static LRESULT CALLBACK basic_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        ezWindow *win = reinterpret_cast<ezWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (win) {
            return win->wndproc(msg, wparam, lparam);
        }
        else if (msg == WM_CREATE) {
            LPCREATESTRUCTW pcs = LPCREATESTRUCTW(lparam);
            win = reinterpret_cast<ezWindow*>(pcs->lpCreateParams);
            if (win) {
                win->m_handle = hwnd;
                ::SetLastError(ERROR_SUCCESS);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(win));
                if (GetLastError() != ERROR_SUCCESS)
                    return -1;
                else
                    return win->wndproc(msg, wparam, lparam);
            }
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

protected:
    virtual LRESULT wndproc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (m_handle) {
            return DefWindowProc(m_handle, msg, wparam, lparam);
        }
        else {
            return 0;
        }
    }

    void MotifyStyle(DWORD style, bool enable)
    {
        if (enable) {
            SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE) | style);
        }
        else {
            SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE) & (~style));
        }
    }

    // 创建控件
    HWND CreateComponent(ezWindow* parent, PCWSTR classname, int x, int y, int width, int height, int style, int styleEx = 0)
    {
        HWND hwnd = CreateWindowExW(styleEx, classname, nullptr, style,
            x, y, width, height, parent->handle(), (HMENU) nullptr, GetModuleHandle(nullptr), nullptr);

        HFONT font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hwnd, WM_SETFONT, (WPARAM) font, 0);
        return hwnd;
    }

    // 初始化窗口类
    BOOL init_window_class(LPCWSTR classname, WNDPROC WndProc)
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        WNDCLASSEXW wc = { sizeof(wc) };
        if (GetClassInfoExW(hInstance, classname, &wc)) {
            //printf("class is exists.\n");
        }
        else {
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;// CS_DBLCLKS 支持鼠标双击事件
            wc.lpfnWndProc = WndProc;
            wc.hInstance = hInstance;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = (HBRUSH) (COLOR_WINDOW);
            wc.lpszClassName = classname;
            wc.hIcon = LoadIcon(hInstance, TEXT("IDI_APPLICATION"));
            wc.hIconSm = LoadIcon(hInstance, TEXT("IDI_APPLICATION"));

            ATOM atom = RegisterClassExW(&wc);

            if (!atom) {
                MessageBox(nullptr, TEXT("Register Window Class Failed!"), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
            }

            //printf("register class success.\n");
        }

        return TRUE;
    }
};

class ezButton : public ezWindow
{
public:
    int create(ezWindow* parent, int x, int y, int width, int height)
    {
        DWORD style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
        m_handle = CreateComponent(parent, L"button", x, y, width, height, style);
        return 0;
    }

    void setDefault(bool value)
    {
        MotifyStyle(BS_DEFPUSHBUTTON, value);
    }
};

class ezLabel : public ezWindow
{
public:
    int create(ezWindow* parent, int x, int y, int width, int height)
    {
        DWORD style = WS_CHILD | WS_VISIBLE | SS_EDITCONTROL;
        m_handle = CreateComponent(parent, L"static", x, y, width, height, style);
        return 0;
    }
};

class ezEdit : public ezWindow
{
public:
    int create(ezWindow* parent, int x, int y, int width, int height)
    {
        DWORD style = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL;
        m_handle = CreateComponent(parent, L"edit", x, y, width, height, style, WS_EX_CLIENTEDGE);
        return 0;
    }

    void multiline(bool value)
    {
        MotifyStyle(ES_MULTILINE, value);
    }

    void setMaxLength(UINT n)
    {
        SendMessage(m_handle, EM_LIMITTEXT, n, 0);
    }
};

//
// InputBox
//

class ezInputBox : private ezWindow
{
private:
    unistring m_message;
    unistring m_text;
    bool m_result;

    ezLabel  lblMessage;
    ezEdit   editbox;
    ezButton btnOK;
    ezButton btnCancel;

public:
    bool execute(HWND parent, const unistring& title, const unistring& message, const unistring& text = unistring())
    {
        int cx = GetSystemMetrics(SM_CXFULLSCREEN);
        int cy = GetSystemMetrics(SM_CYFULLSCREEN);

        int w = 400;
        int h = 150;
        int x = (cx - w) / 2;
        int y = (cy - h) / 2;

        m_message = message;
        m_text = text;
        ezWindow::create(L"EZGDI_InputBox", title.c_str(), x, y, w, h, WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS);

        showModel(parent);

        return m_result;
    }

    unistring text()const
    {
        return m_text;
    }

protected:
    void on_create()
    {
        RECT rect;
        GetClientRect(m_handle, &rect);

        lblMessage.create(this, 4, 5, rect.right - 80, 70);
        lblMessage.setID(1000);
        lblMessage.setText(m_message);

        btnOK.create(this, rect.right - 70, 8, 64, 24);
        btnOK.setID(IDOK);
        btnOK.setDefault(true);
        btnOK.setText(L"确定(&O)");

        btnCancel.create(this, rect.right - 70, 36, 64, 24);
        btnCancel.setID(IDCANCEL);
        btnCancel.setText(L"取消(&C)");

        editbox.create(this, 4, rect.bottom - 24, rect.right - 8, 20);
        editbox.setID(2000);
        editbox.setText(m_text);
        editbox.setMaxLength(128);
    }

    void ButtonOKClick()
    {
        TCHAR buf[256];
        GetDlgItemText(m_handle, 2000, buf, 256);
        m_text = buf;
    }

    LRESULT wndproc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg) {
        case WM_CREATE:
            on_create();
            break;
        case WM_DESTROY:
            m_handle = nullptr;
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            if (wparam == VK_RETURN) {
                SendMessage(m_handle, WM_COMMAND, IDOK, 0);
            }
            else if (wparam == VK_ESCAPE) {
                SendMessage(m_handle, WM_COMMAND, IDCANCEL, 0);
            }
            break;
        case WM_CHAR:
            SendDlgItemMessage(m_handle, 2000, msg, wparam, lparam);
            break;
        case WM_SETFOCUS:
            SendDlgItemMessage(m_handle, 2000, WM_SETFOCUS, 0, 0);
            break;
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
            case IDOK:
                ButtonOKClick();
                DestroyWindow(m_handle);
                m_result = true;
                break;
            case IDCANCEL:
                DestroyWindow(m_handle);
                m_result = false;
                break;
            };
            break;
        default:
            break;
        }
        return DefWindowProc(m_handle, msg, wparam, lparam);
    }
};

//---------------------------------------------------------------------------
//
// minivg 实例类
//
//---------------------------------------------------------------------------

template<typename T = int>
class vgContext : public ezWindow
{
public:
    HDC hdc;                        // GDI 绘图设备
    Gdiplus::Graphics* g;           // GDIPlus 设备
    HBITMAP pixelbuf;               // 像素缓冲区

    int effectLevel;                // 效果等级

    Gdiplus::Pen* pen;              // 画笔
    Gdiplus::SolidBrush* brush;     // 画刷
    Gdiplus::Font* font;            // 字体
    Gdiplus::SolidBrush* font_color;
    unistring fontName;
    float    fontSize;
    int      fontStyle;
    bool     fontIsChange;

    WNDPROC prevWndProc;            // 如果是设置已有的窗口，保存已有窗口的消息处理函数

    int initParam;                  // 创建参数
    RECT winRect;                   // 窗口模式下窗口大小
    bool fullscreen;                // 是否全屏

    vec4i vp;                       // 视口
    bool running;                   // 程序是否运行

    EZ_KEY_EVENT OnKeyDown;         // 键盘事件
    EZ_KEY_EVENT OnKeyUp;
    EZ_KEY_EVENT OnKeyPress;

    EZ_MOUSE_EVENT OnMouseDown;     // 鼠标事件
    EZ_MOUSE_EVENT OnMouseUp;
    EZ_MOUSE_EVENT OnMouseMove;

    EZ_TIMER_EVENT OnTimer;         // 计时器事件
    float tick;

    EZ_PAINT_EVENT OnPaint;         // 窗口绘制事件

    ezResource resource;            // 资源管理器

    static vgContext instance;

private:
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;

public:
    vgContext() :
        ezWindow(),
        hdc(),
        g(),
        pixelbuf(),
        effectLevel(EZ_MEDIUM),
        pen(),
        brush(),
        font(),
        fontName(MINIVG_DEFAULT_FONT),
        fontSize(12),
        fontStyle(EZ_NORMAL),
        running(true),

        OnKeyDown(), OnKeyUp(), OnKeyPress(),
        OnMouseDown(), OnMouseUp(), OnMouseMove(),
        OnTimer(),
        OnPaint()
    {
        prevWndProc = nullptr;
        gdiplusInit();
        tick = tick_time();
    }

    ~vgContext()
    {
        resource.dispose();
        gdiplusShutdown();
    }

    // 设置到已有的窗口
    void setWindow(HWND hwnd)
    {
        if (hwnd) {
            m_handle = hwnd;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
            prevWndProc = (WNDPROC) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) basic_wndproc);
        }
        else {
            if (prevWndProc) {
                SetWindowLongPtr(m_handle, GWLP_WNDPROC, (LONG_PTR) prevWndProc);
                m_handle = nullptr;
                prevWndProc = nullptr;

                resource.dispose();
                gdiplusShutdown();
            }
        }
    }

    // 设置边框样式
    void setStyle(int style)
    {
        DWORD dwStyle;
        DWORD dwExStyle;

        switch (style) {
        case EZ_FIXED:
            dwStyle = WS_POPUPWINDOW | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
            dwExStyle = WS_EX_CLIENTEDGE;
            break;
        case EZ_SIZEABLE:
            dwStyle = WS_OVERLAPPEDWINDOW;
            dwExStyle = WS_EX_CLIENTEDGE;
            break;
        case EZ_FULLSCREEN:
            dwStyle = WS_POPUP;
            dwExStyle = 0;
            break;
        default:
            dwStyle = WS_OVERLAPPEDWINDOW;
            dwExStyle = WS_EX_CLIENTEDGE;
            break;
        }

        dwStyle |= WS_VISIBLE;
        dwExStyle |= WS_EX_APPWINDOW;

        SetWindowLong(m_handle, GWL_STYLE, dwStyle);
        SetWindowLong(m_handle, GWL_EXSTYLE, dwExStyle);
    }

    // 重建背景缓冲区
    void viewport(int x, int y, int width, int height)
    {
        closeGraphics();

        if (!width || !height) {
            return;
        }

        if (width != vp.z || height != vp.w) {
            pixelbuf = bm_create(width, height);
            SelectObject(hdc, pixelbuf);
            g = new Gdiplus::Graphics(hdc);
            effect_level(effectLevel);
        }

        vp = vec4i(x, y, width, height);
    }

    // 将缓冲区的图像绘制到目标 HDC
    void bitblt(HDC dc)
    {
        BitBlt(dc, vp.x, vp.y, vp.z, vp.w, hdc, 0, 0, SRCCOPY);
    }

    // 设置窗口置顶
    bool topmose(bool top)
    {
        if (top) {
            return SetWindowPos(m_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
        else {
            return SetWindowPos(m_handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
    }

    // 主窗口消息
    LRESULT wndproc(UINT Message, WPARAM wParam, LPARAM lParam)
    {
        switch (Message) {
        case WM_CREATE:
            break;
        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
            break;
        case WM_WINDOWPOSCHANGING:
            break;
        case WM_ERASEBKGND:
            return TRUE;
        case WM_QUIT:
            running = false;
            break;
        case WM_SHOWWINDOW:
            this->repaint();
            break;
        case WM_SIZE:
            this->viewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_PAINT:
            this->OnWindowPaint();
            break;
        case WM_TIMER:
            {
                float t = tick_time();
                if (OnTimer)OnTimer(tick - t);
                tick = t;
            }
            break;

        case WM_KEYDOWN:
            if (OnKeyDown)OnKeyDown(int(wParam));
            break;
        case WM_KEYUP:
            if (OnKeyUp)OnKeyUp(int(wParam));
            break;
        case WM_CHAR:
            if (OnKeyPress)OnKeyPress(int(wParam));
            break;

        case WM_MOUSEMOVE:
            if (OnMouseMove)OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), int(wParam) & 0x13);
            break;
        case WM_LBUTTONDOWN:
            if (OnMouseDown)OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_LEFT);
            break;
        case WM_LBUTTONUP:
            if (OnMouseUp)OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_LEFT);
            break;
        case WM_RBUTTONDOWN:
            if (OnMouseDown)OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_RIGHT);
            break;
        case WM_RBUTTONUP:
            if (OnMouseUp)OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_RIGHT);
            break;
        case WM_MBUTTONDOWN:
            if (OnMouseDown)OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_MIDDLE);
            break;
        case WM_MBUTTONUP:
            if (OnMouseUp)OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_MIDDLE);
            break;

        default:
            break;
        }

        if (prevWndProc) {
            return CallWindowProc(prevWndProc, m_handle, Message, wParam, lParam);
        }
        else {
            return ezWindow::wndproc(Message, wParam, lParam);
        }
    }

protected:
    void gdiplusInit()
    {
        Gdiplus::GdiplusStartup(&token, &input, nullptr);

        pen = new Gdiplus::Pen(Gdiplus::Color::Black);
        brush = new Gdiplus::SolidBrush(Gdiplus::Color::White);
        font = new Gdiplus::Font(fontName.c_str(), 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint, nullptr);
        font_color = new Gdiplus::SolidBrush(Gdiplus::Color::Black);

        hdc = CreateCompatibleDC(nullptr);
    }

    void gdiplusShutdown()
    {
        closeGraphics();
        delete_object(hdc);
        detail::safe_delete(pen);
        detail::safe_delete(brush);
        detail::safe_delete(font);
        detail::safe_delete(font_color);

        Gdiplus::GdiplusShutdown(token);
    }

    void closeGraphics()
    {
        detail::safe_delete(g);
        if (pixelbuf) {
            delete_object(pixelbuf);
        }
    }

    // 窗口重绘事件
    void OnWindowPaint()
    {
        PAINTSTRUCT ps;
        BeginPaint(m_handle, &ps);
        if (OnPaint)OnPaint();
        this->bitblt(ps.hdc);
        EndPaint(m_handle, &ps);
    }
};

template<typename T>
vgContext<T> vgContext<T>::instance = vgContext<T>();

// 屏幕更新线程
MINIVG_INLINE void updateThread(void* arg)
{
    while (vgContext<>::instance.running) {
        // 刷新窗口
        vgContext<>::instance.repaint();
    }

}

}// end namespace detail

 //---------------------------------------------------------------------------
 //
 // 主函数
 //
 //---------------------------------------------------------------------------

MINIVG_INLINE int initgraph(const unistring& title, int width, int height, int param)
{
    setlocale(LC_ALL, "chs");                   // c 中文环境
    std::locale::global(std::locale("chs"));    // c++ 中文环境

    // 保存创建参数
    detail::vgContext<>::instance.initParam = param;

    if (param & EZ_BACKBUFFER) {
        detail::vgContext<>::instance.viewport(0, 0, width, height);
        detail::vgContext<>::instance.winRect.left = 0;
        detail::vgContext<>::instance.winRect.top = 0;
        detail::vgContext<>::instance.winRect.right = width;
        detail::vgContext<>::instance.winRect.bottom = height;
    }
    else {
        // 获得屏幕大小
        int cx = GetSystemMetrics(SM_CXFULLSCREEN);
        int cy = GetSystemMetrics(SM_CYFULLSCREEN);

        // 居中显示
        if (width < cx) {
            cx = (cx - width) / 2;
        }
        else {
            width = cx;
            cx = 0;
        }
        if (height < cy) {
            cy = (cy - height) / 2;
        }
        else {
            height = cy;
            cy = 0;
        }

        // 设置窗口样式
        DWORD dwStyle;
        DWORD dwExStyle;

        switch (param) {
        case EZ_FIXED:
            dwStyle = WS_POPUPWINDOW | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
            dwExStyle = WS_EX_CLIENTEDGE;
            break;
        case EZ_SIZEABLE:
            dwStyle = WS_OVERLAPPEDWINDOW;
            dwExStyle = WS_EX_CLIENTEDGE;
            break;
        case EZ_FULLSCREEN:
            cx = cy = 0;
            width = GetSystemMetrics(SM_CXSCREEN);
            height = GetSystemMetrics(SM_CYSCREEN);
            dwStyle = WS_POPUP;
            dwExStyle = 0;
            break;
        }

        // 计算窗口大小
        RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRectEx(&rect, dwStyle, FALSE, WS_EX_CLIENTEDGE);
        width = static_cast<int>(rect.right - rect.left);
        height = static_cast<int>(rect.bottom - rect.top);

        OffsetRect(&rect, cx, cy);
        detail::vgContext<>::instance.winRect = rect;

        // 创建一个窗口
        detail::vgContext<>::instance.create(MINIVG_CLASS_NAME, title.c_str(),
            cx, cy, width, height,
            dwStyle, dwExStyle);

        detail::vgContext<>::instance.show();

        // 启动刷新线程
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE) detail::updateThread, nullptr, 0, 0);
    }

    return 0;
}

MINIVG_INLINE int initgraph(int width, int height, int param)
{
    return initgraph(MINIVG_VERSION, width, height, param);
}

MINIVG_INLINE int initgraph(HWND hwnd)
{
    setlocale(LC_ALL, "");                  // c 中文
    std::locale::global(std::locale(""));   // c++ 中文

    // 设置图形库窗口
    detail::vgContext<>::instance.setWindow(hwnd);
    detail::vgContext<>::instance.show();

    return 0;
}

MINIVG_INLINE void quit()
{
    if (detail::vgContext<>::instance.prevWndProc) {
        detail::vgContext<>::instance.setWindow(nullptr);
    }
    else {
        detail::vgContext<>::instance.close();
    }
}

MINIVG_INLINE HWND graph_window()
{
    return detail::vgContext<>::instance.handle();
}

// 获得 GDI 绘图设备
MINIVG_INLINE HDC graph_hdc()
{
    return detail::vgContext<>::instance.hdc;
}

MINIVG_INLINE void set_title(const unistring& text)
{
    detail::vgContext<>::instance.setTitle(text);
}

MINIVG_INLINE void reshape(int width, int height)
{
    HWND hwnd = graph_window();
    RECT rcWindow;
    RECT rcClient;
    int borderWidth, borderHeight;

    GetWindowRect(hwnd, &rcWindow);
    GetClientRect(hwnd, &rcClient);

    borderWidth = (rcWindow.right - rcWindow.left) - (rcClient.right - rcClient.left);
    borderHeight = (rcWindow.bottom - rcWindow.top) - (rcClient.bottom - rcClient.top);

    SetWindowPos(hwnd, 0, 0, 0, borderWidth + width, borderHeight + height, SWP_NOMOVE | SWP_NOZORDER);
}

MINIVG_INLINE void fullscreen(bool value)
{
    if (value != detail::vgContext<>::instance.fullscreen) {
        if (value) {
            GetWindowRect(detail::vgContext<>::instance.handle(), &detail::vgContext<>::instance.winRect);

            detail::vgContext<>::instance.setStyle(EZ_FULLSCREEN);
            detail::vgContext<>::instance.fullscreen = true;

            // 获得屏幕大小
            int cx = GetSystemMetrics(SM_CXSCREEN);
            int cy = GetSystemMetrics(SM_CYSCREEN);
            SetWindowPos(detail::vgContext<>::instance.handle(), 0, 0, 0, cx, cy, SWP_NOZORDER);
        }
        else {
            detail::vgContext<>::instance.fullscreen = false;
            detail::vgContext<>::instance.setStyle(EZ_FIXED);

            detail::vgContext<>::instance.setBounds(
                detail::vgContext<>::instance.winRect.left,
                detail::vgContext<>::instance.winRect.top,
                detail::vgContext<>::instance.winRect.right - detail::vgContext<>::instance.winRect.left,
                detail::vgContext<>::instance.winRect.bottom - detail::vgContext<>::instance.winRect.top);
        }
    }
}

// 消息循环处理
MINIVG_INLINE bool do_events()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return detail::vgContext<>::instance.running;
}

// 程序执行
MINIVG_INLINE int start_app()
{
    while (do_events());

    return 0;
}

// 显示fps
MINIVG_INLINE void show_fps()
{
    static DWORD t = GetTickCount();
    static int fps_total = 0;
    static int fps = 0;

    ++fps;

    unistring str = L"FPS:";
    str += unistring(fps_total);

    setfont(L"msyh", 16);
    font_color(255, 0, 0);
    textout(0, 0, str);

    if (GetTickCount() - t > 1000) {
        t = GetTickCount();
        fps_total = fps;
        fps = 0;
    }
}

MINIVG_INLINE std::basic_string<TCHAR> ezTempPath()
{
    TCHAR buf[MAX_PATH];
    ::GetTempPath(MAX_PATH, buf);
    return std::basic_string<TCHAR>(buf);
}

//---------------------------------------------------------------------------
//
// 窗口事件、输入按键管理
//
//---------------------------------------------------------------------------

// 判断按键是否按下
MINIVG_INLINE bool keystate(int key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

// 键盘事件映射
MINIVG_INLINE void key_push_event(EZ_KEY_EVENT function)
{
    detail::vgContext<>::instance.OnKeyDown = function;
}

MINIVG_INLINE void key_pop_event(EZ_KEY_EVENT function)
{
    detail::vgContext<>::instance.OnKeyUp = function;
}

MINIVG_INLINE void input_event(EZ_KEY_EVENT function)
{
    detail::vgContext<>::instance.OnKeyPress = function;
}

// 鼠标事件映射
MINIVG_INLINE void mouse_push_event(EZ_MOUSE_EVENT function)
{
    detail::vgContext<>::instance.OnMouseDown = function;
}

MINIVG_INLINE void mouse_pop_event(EZ_MOUSE_EVENT function)
{
    detail::vgContext<>::instance.OnMouseUp = function;
}

MINIVG_INLINE void mouse_move_event(EZ_MOUSE_EVENT function)
{
    detail::vgContext<>::instance.OnMouseMove = function;
}

// 计时器
MINIVG_INLINE void start_timer(UINT interval)
{
    if (interval) {
        SetTimer(graph_window(), UINT(MINIVG_TIMER_ID), interval, nullptr);
    }
    else {
        KillTimer(graph_window(), UINT(MINIVG_TIMER_ID));
    }
}

MINIVG_INLINE void stop_timer()
{
    KillTimer(graph_window(), UINT(MINIVG_TIMER_ID));
}

// 计时器事件
MINIVG_INLINE void timer_event(EZ_TIMER_EVENT function)
{
    detail::vgContext<>::instance.OnTimer = function;
}

// 窗口绘制事件
MINIVG_INLINE void display_event(EZ_PAINT_EVENT function)
{
    detail::vgContext<>::instance.OnPaint = function;
}

//---------------------------------------------------------------------------
//
// 绘图函数
//
//---------------------------------------------------------------------------

// 获得 GDI+ 绘图设备
MINIVG_INLINE Gdiplus::Graphics* graphics()
{
    return detail::vgContext<>::instance.g;
}

// 重设背景缓冲区大小位置
MINIVG_INLINE void viewport(int x, int y, int width, int height)
{
    detail::vgContext<>::instance.viewport(0, 0, width, height);
}

// 背景缓冲绘制到目标 HDC
MINIVG_INLINE void framebuf_blt(HDC hdc)
{
    detail::vgContext<>::instance.bitblt(hdc);
}

MINIVG_INLINE void set_graphics_effect_level(Gdiplus::Graphics* g, int level)
{
    switch (level) {
    case EZ_SPEED:
        g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);            // 混合模式
        g->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);       // 混合质量
        g->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);                 // 反锯齿
        g->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);                  // 像素偏移模式
        g->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);   // 图形缩放质量
        break;
    case EZ_MEDIUM:
        g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);            // 混合模式
        g->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);       // 混合质量
        g->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);                 // 反锯齿
        g->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);                  // 像素偏移模式
        g->SetInterpolationMode(Gdiplus::InterpolationModeBilinear);          // 图形缩放质量
        break;
    case EZ_QUALITY:
        g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);            // 混合模式
        g->SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);     // 混合质量
        g->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);                 // 反锯齿
        g->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);           // 像素偏移模式
        g->SetInterpolationMode(Gdiplus::InterpolationModeBicubic);           // 图形缩放质量
        break;
    default:
        break;
    }
}

// 设置显示质量
MINIVG_INLINE int effect_level(int level)
{
    Gdiplus::Graphics* g = detail::vgContext<>::instance.g;
    if (!g) {
        return -1;
    }
    set_graphics_effect_level(g, level);
    detail::vgContext<>::instance.effectLevel = level;

    return 0;
}

// 清屏
MINIVG_INLINE void clear(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if (detail::vgContext<>::instance.g)detail::vgContext<>::instance.g->Clear(Gdiplus::Color(a, r, g, b));
}

// 更改画笔颜色
MINIVG_INLINE void pen_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if (detail::vgContext<>::instance.pen)detail::vgContext<>::instance.pen->SetColor(Gdiplus::Color(a, r, g, b));
}

MINIVG_INLINE void pen_color(COLORREF argb)
{
    if (detail::vgContext<>::instance.pen)detail::vgContext<>::instance.pen->SetColor(Gdiplus::Color(argb));
}

MINIVG_INLINE void pen_color(vec4ub color)
{
    if (detail::vgContext<>::instance.pen)detail::vgContext<>::instance.pen->SetColor(Gdiplus::Color(color.a, color.r, color.g, color.b));
}

// 获取画笔颜色
MINIVG_INLINE COLORREF pen_color()
{
    Gdiplus::Color color;
    if (detail::vgContext<>::instance.pen) {
        detail::vgContext<>::instance.pen->GetColor(&color);
    }
    return color.GetValue();
}

/*
vec4ub pen_color()
{
    Gdiplus::Color color;
    if(detail::vgContext<>::instance.pen) {
        detail::vgContext<>::instance.pen->GetColor(&color);
    }
    return vec4ub(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}
*/

// 画笔宽度
MINIVG_INLINE void pen_width(float width)
{
    if (detail::vgContext<>::instance.pen)detail::vgContext<>::instance.pen->SetWidth(width);
}

// 获取画笔宽度
MINIVG_INLINE float pen_width()
{
    return detail::vgContext<>::instance.pen ? detail::vgContext<>::instance.pen->GetWidth() : 1.0f;
}

// 设置画笔模式
MINIVG_INLINE void pen_style(int mode)
{
    if (detail::vgContext<>::instance.pen) {
        //if(mode != EZ_CUSTOM){//取消自定义点画模式
        //    ezDashStyle(nullptr, 0);
        //}
        detail::vgContext<>::instance.pen->SetDashStyle(Gdiplus::DashStyle(mode));
    }
}

// 设置点画模式间隔
MINIVG_INLINE void dash_style(const float* dash, int size)
{
    if (detail::vgContext<>::instance.pen) {
        //if(detail::vgContext<>::instance.pen->GetDashStyle() == Gdiplus::DashStyleCustom){
        detail::vgContext<>::instance.pen->SetDashPattern(dash, size);
        //}
    }
}

// 更改填充颜色
MINIVG_INLINE void fill_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if (detail::vgContext<>::instance.brush)detail::vgContext<>::instance.brush->SetColor(Gdiplus::Color(a, r, g, b));
}

MINIVG_INLINE void fill_color(COLORREF argb)
{
    if (detail::vgContext<>::instance.brush)detail::vgContext<>::instance.brush->SetColor(Gdiplus::Color(argb));
}

// 获取填充颜色
MINIVG_INLINE COLORREF fill_color()
{
    Gdiplus::Color color;
    if (detail::vgContext<>::instance.brush) {
        detail::vgContext<>::instance.brush->GetColor(&color);
    }
    return color.GetValue();
}

// 绘制一个点
MINIVG_INLINE void draw_point(float x, float y, float size)
{
    if (detail::vgContext<>::instance.g) {
        Gdiplus::SolidBrush brush(pen_color());
        float half = size * 0.5f;
        detail::vgContext<>::instance.g->FillEllipse(&brush, x - half, y - half, size, size);
    }
}

MINIVG_INLINE void draw_point(float x, float y)
{
    draw_point(x, y, pen_width());
}

// 绘制线段
MINIVG_INLINE void draw_line(float x1, float y1, float x2, float y2)
{
    if (detail::vgContext<>::instance.g)detail::vgContext<>::instance.g->DrawLine(detail::vgContext<>::instance.pen, x1, y1, x2, y2);
}

// 绘制一个空心矩形
MINIVG_INLINE void draw_rect(float x, float y, float width, float height)
{
    if (detail::vgContext<>::instance.g)detail::vgContext<>::instance.g->DrawRectangle(detail::vgContext<>::instance.pen, x, y, width, height);
}

// 填充一个矩形
MINIVG_INLINE void fill_rect(float x, float y, float width, float height)
{
    if (detail::vgContext<>::instance.g)detail::vgContext<>::instance.g->FillRectangle(detail::vgContext<>::instance.brush, x, y, width, height);
}

// 绘制圆角矩形
MINIVG_INLINE void draw_roundrect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if (cx > width)cx = width;
    if (cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = detail::vgContext<>::instance.g;
    if (g) {
        Gdiplus::GraphicsPath path;
        path.AddArc(x, y, cx, cy, 180, 90);
        path.AddArc(x2, y, cx, cy, 270, 90);
        path.AddArc(x2, y2, cx, cy, 0, 90);
        path.AddArc(x, y2, cx, cy, 90, 90);
        path.CloseFigure();
        g->DrawPath(detail::vgContext<>::instance.pen, &path);
    }
}

// 填充圆角矩形
MINIVG_INLINE void fill_roundrect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if (cx > width)cx = width;
    if (cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = detail::vgContext<>::instance.g;
    if (g) {
        Gdiplus::GraphicsPath path;
        path.AddArc(x, y, cx, cy, 180, 90);
        path.AddArc(x2, y, cx, cy, 270, 90);
        path.AddArc(x2, y2, cx, cy, 0, 90);
        path.AddArc(x, y2, cx, cy, 90, 90);
        path.CloseFigure();
        g->FillPath(detail::vgContext<>::instance.brush, &path);
    }
}

// 绘制椭圆，xy 为圆心
MINIVG_INLINE void draw_ellipse(float x, float y, float cx, float cy)
{
    if (detail::vgContext<>::instance.g)detail::vgContext<>::instance.g->DrawEllipse(detail::vgContext<>::instance.pen, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

// 填充椭圆
MINIVG_INLINE void fill_ellipse(float x, float y, float cx, float cy)
{
    if (detail::vgContext<>::instance.g)detail::vgContext<>::instance.g->FillEllipse(detail::vgContext<>::instance.brush, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

// 绘制空心圆，xy 为圆心
MINIVG_INLINE void draw_circle(float x, float y, float r)
{
    return draw_ellipse(x, y, r, r);
}

// 填充圆
MINIVG_INLINE void fill_circle(float x, float y, float r)
{
    return fill_ellipse(x, y, r, r);
}

// 绘制连续的线段
MINIVG_INLINE void draw_polyline(const vec2f* points, size_t size)
{
    if (detail::vgContext<>::instance.g) {
        detail::vgContext<>::instance.g->DrawLines(detail::vgContext<>::instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

// 绘制多边形
MINIVG_INLINE void draw_polygon(const vec2f* points, size_t size)
{
    if (detail::vgContext<>::instance.g) {
        detail::vgContext<>::instance.g->DrawPolygon(detail::vgContext<>::instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

// 填充多边形
MINIVG_INLINE void fill_polygon(const vec2f* points, size_t size)
{
    if (detail::vgContext<>::instance.g) {
        detail::vgContext<>::instance.g->FillPolygon(detail::vgContext<>::instance.brush, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//---------------------------------------------------------------------------
//
// 字体函数
//
//---------------------------------------------------------------------------

// 设置字体。字体名字、大小、风格
MINIVG_INLINE void setfont(const unistring& name, float size, EZGDI_FONTSTYLE style)
{
    if (detail::vgContext<>::instance.font) {
        delete detail::vgContext<>::instance.font;
        detail::vgContext<>::instance.font = new Gdiplus::Font(name.c_str(), size, style, Gdiplus::UnitPoint, nullptr);
    }
}

MINIVG_INLINE void setfont(const unistring& name, float size, bool bold, bool, bool underline, bool strikeout)
{
    EZGDI_FONTSTYLE style = EZ_NORMAL;
    if (bold) {
        style = (EZGDI_FONTSTYLE) (style | EZ_BOLD);
    }
    if (underline) {
        style = (EZGDI_FONTSTYLE) (style | EZ_UNDERLINE);
    }
    if (strikeout) {
        style = (EZGDI_FONTSTYLE) (style | EZ_STRIKEOUT);
    }
    setfont(name, size, style);
}

MINIVG_INLINE void font_name(const unistring& name)
{
    detail::vgContext<>::instance.fontName = name;
    detail::vgContext<>::instance.fontIsChange = true;
}

MINIVG_INLINE void font_size(float size)
{
    detail::vgContext<>::instance.fontSize = size;
    detail::vgContext<>::instance.fontIsChange = true;
}

MINIVG_INLINE void font_style(int style)
{
    detail::vgContext<>::instance.fontStyle = style;
    detail::vgContext<>::instance.fontIsChange = true;
}

// 获取字体属性
//unistring font_name();
//int font_size();
//int font_style();

// 字体颜色
MINIVG_INLINE void font_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if (detail::vgContext<>::instance.font_color)detail::vgContext<>::instance.font_color->SetColor(Gdiplus::Color(a, r, g, b));
}

MINIVG_INLINE void font_color(COLORREF color)
{
    if (detail::vgContext<>::instance.font_color)detail::vgContext<>::instance.font_color->SetColor(Gdiplus::Color(color));
}

// 输出文字
MINIVG_INLINE void textout(float x, float y, const char* text, size_t length)
{
    textout(x, y, unistring(text, length));
}

MINIVG_INLINE void textout(float x, float y, const wchar_t* text, size_t length)
{
    if (detail::vgContext<>::instance.g) {
        if (detail::vgContext<>::instance.fontIsChange) {
            setfont(detail::vgContext<>::instance.fontName, detail::vgContext<>::instance.fontSize, EZGDI_FONTSTYLE(detail::vgContext<>::instance.fontStyle));
            detail::vgContext<>::instance.fontIsChange = false;
        }

        Gdiplus::StringFormat format;
        detail::vgContext<>::instance.g->DrawString(text, INT(length), detail::vgContext<>::instance.font,
            Gdiplus::PointF(x, y), &format, detail::vgContext<>::instance.font_color);
    }
}

MINIVG_INLINE void textout(float x, float y, const unistring& text)
{
    return textout(x, y, text.c_str(), text.length());
}

// 格式输出文字（左对齐、右对齐、居中）
MINIVG_INLINE void drawtext(float x, float y, float width, float height, const unistring& text, int align)
{
    if (detail::vgContext<>::instance.g) {
        if (detail::vgContext<>::instance.fontIsChange) {
            setfont(detail::vgContext<>::instance.fontName, detail::vgContext<>::instance.fontSize, EZGDI_FONTSTYLE(detail::vgContext<>::instance.fontStyle));
            detail::vgContext<>::instance.fontIsChange = false;
        }

        Gdiplus::StringFormat format;

        int hAlign = 0;
        int vAlign = 0;
        if ((align & EZ_CENTER_H) == EZ_CENTER_H) {
            hAlign = Gdiplus::StringAlignmentCenter;
        }
        else if (align & EZ_RIGHT) {
            hAlign = Gdiplus::StringAlignmentFar;
        }

        if ((align & EZ_CENTER_V) == EZ_CENTER_V) {
            vAlign = Gdiplus::StringAlignmentCenter;
        }
        else if (align & EZ_DOWN) {
            vAlign = Gdiplus::StringAlignmentFar;
        }

        format.SetAlignment((Gdiplus::StringAlignment)hAlign);//水平对齐
        format.SetLineAlignment((Gdiplus::StringAlignment)vAlign);//垂直对齐
        Gdiplus::RectF rect(x, y, width, height);
        detail::vgContext<>::instance.g->DrawString(text.c_str(), INT(text.length()),
            detail::vgContext<>::instance.font,
            rect,
            &format,
            detail::vgContext<>::instance.font_color);

    }
}

// 字体格式化输出，和 printf 使用类似
MINIVG_INLINE void print(float x, float y, const char* param, ...)
{
    const size_t size = 1024;
    va_list body;
    char buf[size] = { 0 };
    va_start(body, param);
    #ifdef _MSC_VER
    _vsnprintf_s(buf, size, size - 1, param, body);
    #else
    vsnprintf(buf, 1024, param, body);
    #endif
    textout(x, y, buf, strlen(buf));
    va_end(body);
}

MINIVG_INLINE void print(float x, float y, const wchar_t* param, ...)
{
    const size_t size = 1024;
    va_list body;
    wchar_t buf[size] = { 0 };
    va_start(body, param);
    #ifdef _MSC_VER
    _vsnwprintf_s(buf, size, size - 1, param, body);
    #else
    vsnwprintf(buf, 1024, param, body);
    #endif
    textout(x, y, buf, wcslen(buf));
    va_end(body);
}

// 获得字符串的像素宽度
MINIVG_INLINE float textwidth(const unistring& text)
{
    if (detail::vgContext<>::instance.g) {
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        detail::vgContext<>::instance.g->MeasureString(text.c_str(), int(text.length()), detail::vgContext<>::instance.font, layoutSize, nullptr, &size);
        return size.Width;
    }
    return 0;
}

// 获得字符串的像素高度
MINIVG_INLINE float textheight(const unistring& text)
{
    if (detail::vgContext<>::instance.g) {
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        detail::vgContext<>::instance.g->MeasureString(text.c_str(), int(text.length()), detail::vgContext<>::instance.font, layoutSize, nullptr, &size);
        return size.Height;
    }
    return 0;
}

//---------------------------------------------------------------------------
//
// 图片操作
//
//---------------------------------------------------------------------------

inline ezImage::ezImage() : m_handle(), m_data()
{
}

inline ezImage::~ezImage()
{
    this->close();
}

// 返回图片的 gdiplus 图片指针
inline Gdiplus::Bitmap* ezImage::handle()const
{
    return m_handle;
}

// 创建一个图片，默认为 32 位色
inline int ezImage::create(int width, int height, int format)
{
    this->close();
    m_handle = new Gdiplus::Bitmap(width, height, format);
    return 0;
}

// 打开一个图片，支持 bmp、jpg、png、静态 gif 等常见格式
inline int ezImage::open(const unistring& filename)
{
    this->close();
    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(filename.c_str());
    if (bmp->GetLastStatus() == Gdiplus::Ok) {
        m_handle = bmp->Clone(0, 0, bmp->GetWidth(), bmp->GetHeight(), PixelFormat32bppPARGB);

        // 调整图片 DPI
        m_handle->SetResolution(96.0f, 96.0f);

        return 0;
    }

    detail::safe_delete(bmp);

    return -1;
}

// 从资源中加载图片
inline Gdiplus::Bitmap* LoadResourceImage(UINT id, PCTSTR type)
{
    Gdiplus::Bitmap* image = nullptr;
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    if (type == RT_BITMAP) {
        return Gdiplus::Bitmap::FromResource(GetModuleHandle(nullptr), MAKEINTRESOURCEW(id));
    }

    HRSRC hResSource = ::FindResource(hInstance, MAKEINTRESOURCE(id), type);
    if (hResSource) {
        // load resource into memory
        DWORD size = SizeofResource(hInstance, hResSource);
        BYTE* source = (BYTE*) LoadResource(hInstance, hResSource);
        if (source) {
            // allocate global memory on which to create stream
            HGLOBAL hmem = GlobalAlloc(GMEM_FIXED, size);
            if (hmem) {
                // copy data to global memory
                BYTE* dest = (BYTE*) GlobalLock(hmem);
                memcpy(dest, source, size);
                GlobalUnlock(hmem);

                // create stream
                IStream* pstm;
                CreateStreamOnHGlobal(hmem, FALSE, &pstm);
                // load from stream
                image = Gdiplus::Bitmap::FromStream(pstm);
                // free/release stuff
                pstm->Release();
                GlobalFree(hmem);
            }
            FreeResource(source);
        }
    }

    return image;
}

// 打开资源中的图片
inline int ezImage::open(int id, PCTSTR resource_type)
{
    this->close();
    Gdiplus::Bitmap* bmp = LoadResourceImage(id, resource_type);
    if (bmp) {
        m_handle = bmp->Clone(0, 0, bmp->GetWidth(), bmp->GetHeight(), PixelFormat32bppPARGB);
        delete bmp;

        // 调整图片 DPI
        m_handle->SetResolution(96.0f, 96.0f);

        return 0;
    }
    return -1;
}

// 映射一个 HBITMAP 对象
inline int ezImage::bind(HBITMAP hbmp)
{
    this->close();
    BITMAP bm;
    GetObject(hbmp, sizeof(bm), &bm);
    if (bm.bmBits) {
        BYTE* pixels = ((BYTE*) bm.bmBits) + (bm.bmHeight - 1) * bm.bmWidthBytes;
        m_handle = new Gdiplus::Bitmap(bm.bmWidth, bm.bmHeight, -bm.bmWidthBytes, PixelFormat32bppARGB, pixels);
    }
    return 0;
}

// 判断图片是否为空
inline bool ezImage::empty()const
{
    return !m_handle;
}

inline const wchar_t* GetImageType(int type)
{
    // 图片格式
    const wchar_t* GDIPLUS_IMAGE_BMP = L"image/bmp";
    const wchar_t* GDIPLUS_IMAGE_JPG = L"image/jpeg";
    const wchar_t* GDIPLUS_IMAGE_GIF = L"image/gif";
    const wchar_t* GDIPLUS_IMAGE_TIFF = L"image/tiff";
    const wchar_t* GDIPLUS_IMAGE_PNG = L"image/png";

    switch (type) {
    case EZ_BMP:
        return GDIPLUS_IMAGE_BMP;
    case EZ_JPG:
        return GDIPLUS_IMAGE_JPG;
    case EZ_GIF:
        return GDIPLUS_IMAGE_GIF;
    case EZ_TIFF:
        return GDIPLUS_IMAGE_TIFF;
    case EZ_PNG:
        return GDIPLUS_IMAGE_PNG;
    default:
        return nullptr;
    }
}

MINIVG_INLINE int GetImageCLSID(const WCHAR* format, CLSID* pCLSID)
{
    // 得到格式为format的图像文件的编码值，访问该格式图像的COM组件的GUID值保存在pCLSID中
    UINT num = 0;
    UINT size = 0;

    Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;
    Gdiplus::GetImageEncodersSize(&num, &size);

    if (size == 0) {
        // 编码信息不可用
        return -1;
    }

    // 分配内存
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (!pImageCodecInfo) {
        // 分配失败
        return -2;
    }

    // 获得系统中可用的编码方式的所有信息
    GetImageEncoders(num, size, pImageCodecInfo);

    // 在可用编码信息中查找format格式是否被支持
    for (UINT i = 0; i < num; ++i) {
        // MimeType：编码方式的具体描述
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pCLSID = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);

            // 成功
            return 0;
        }
    }

    free(pImageCodecInfo);
    return -3;
}

inline int ezImage::save(const unistring& filename, int type)
{
    if (m_handle) {
        CLSID id;
        if (!GetImageCLSID(GetImageType(type), &id)) {
            return m_handle->Save(filename.c_str(), &id, nullptr);
        }
        return 0;
    }
    return -1;
}

// 释放图片
inline void ezImage::close()
{
    if (m_handle) {
        delete m_handle;
        m_handle = nullptr;
    }
}

// 返回图片的宽度
inline int ezImage::width()const
{
    return m_handle ? m_handle->GetWidth() : 0;
}

// 返回图片的高度
inline int ezImage::height()const
{
    return m_handle ? m_handle->GetHeight() : 0;
}

// 获取图像数据指针
inline void* ezImage::map(bool readonly, int pixelformat)
{
    if (m_data) {
        msgbox(L"图片已经锁定。", L"错误");
    }
    else {
        m_data = new Gdiplus::BitmapData();
        Gdiplus::Rect rect(0, 0, this->width(), this->height());
        Gdiplus::Status stat;
        UINT flags;
        if (readonly) {
            flags = Gdiplus::ImageLockModeRead;
        }
        else {
            flags = Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite;
        }

        if (pixelformat == EZ_RGB) {
            stat = m_handle->LockBits(0, flags, PixelFormat24bppRGB, m_data);
        }
        else {
            // 2022-02-06 05:11:28 修改格式为 PixelFormat32bppPARGB
            stat = m_handle->LockBits(0, flags, PixelFormat32bppPARGB, m_data);
        }

        if (stat == Gdiplus::Ok) {
            return m_data->Scan0;
        }
    }

    return nullptr;
}

// 还原图像数据
inline void ezImage::unmap()
{
    if (m_data) {
        m_handle->UnlockBits(m_data);
        detail::safe_delete(m_data);
    }
}

//
// API 部分
//

// 创建图片
MINIVG_INLINE ezImage* newimage(int width, int height)
{
    return detail::vgContext<>::instance.resource.allocate(width, height);
}

// 删除图片
MINIVG_INLINE void freeimage(ezImage* image)
{
    detail::vgContext<>::instance.resource.free(image);
}

MINIVG_INLINE ezImage* loadimage(const unistring& filename)
{
    return detail::vgContext<>::instance.resource.loadimage(filename);
}

MINIVG_INLINE ezImage* loadimage(int id, PCTSTR resource_type)
{
    return detail::vgContext<>::instance.resource.loadimage(id, resource_type);
}

MINIVG_INLINE int saveimage(ezImage* image, const unistring& filename)
{
    if (image) {
        image->save(filename);
    }
    return -1;
}

MINIVG_INLINE void drawimage(ezImage* image, float x, float y)
{
    if (detail::vgContext<>::instance.g && image && image->handle())detail::vgContext<>::instance.g->DrawImage(image->handle(), x, y);
}

MINIVG_INLINE void drawimage(ezImage* image, float x, float y, float width, float height)
{
    if (detail::vgContext<>::instance.g && image && image->handle())detail::vgContext<>::instance.g->DrawImage(image->handle(), x, y, width, height);
}

// 在xy位置绘制图片，并旋转一个角度
MINIVG_INLINE void rotate_image(ezImage* image, float x, float y, float rotate)
{
    return rotate_image(image, x, y, float(image->width()), float(image->height()), rotate);
}

// 在xy位置绘制图片，缩放，并旋转一个角度
MINIVG_INLINE void rotate_image(ezImage* image, float x, float y, float width, float height, float rotate)
{
    Gdiplus::Graphics* g = detail::vgContext<>::instance.g;
    if (g && image) {
        float cx = width / 2;
        float cy = height / 2;
        Gdiplus::Matrix m;
        g->GetTransform(&m);
        g->TranslateTransform(x, y);        // 移动 xy 到原点
        g->RotateTransform(rotate);         // 旋转
        g->TranslateTransform(-cx, -cy);    // 移动回原位置
        g->DrawImage(image->handle(), 0.0f, 0.0f, width, height);   // 绘制图像
        g->SetTransform(&m);
    }
}

// 把像素直接绘制到屏幕上，像素格式必须是 BGRA 32位。
MINIVG_INLINE void draw_pixels(float x, float y, float width, float height, const void* pixels, int imageWidth, int imageHeight)
{
    Gdiplus::Graphics* g = detail::vgContext<>::instance.g;
    if (g) {
        BYTE* data = (BYTE*) pixels;
        data += (imageWidth * 4) * (imageHeight - 1);
        Gdiplus::Bitmap bmp(imageWidth, imageHeight, -imageWidth * 4, PixelFormat32bppPARGB, data);
        g->DrawImage(&bmp, x, y, width, height);
    }
}

//---------------------------------------------------------------------------
//
// 多媒体
//
//---------------------------------------------------------------------------

//播放音乐
MINIVG_INLINE void play_music(PCTSTR filename)
{
    std::basic_string<TCHAR> command = TEXT("open ");
    command.append(filename);
    command += TEXT(" alias background");

    mciSendString(command.c_str(), nullptr, 0, nullptr);
    mciSendString(TEXT("play background repeat"), nullptr, 0, nullptr);
}

MINIVG_INLINE void stop_music()
{
    mciSendString(TEXT("stop background"), nullptr, 0, nullptr);//!
    mciSendString(TEXT("close background"), nullptr, 0, nullptr);
}

// 导出资源到文件
// 参考文章：https://www.cnblogs.com/zjutlitao/p/3577592.html
MINIVG_INLINE bool ExtractResource(LPCTSTR filename, LPCTSTR resource_type, LPCTSTR resource_name)
{
    // 创建文件
    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    // 查找资源文件中、加载资源到内存、得到资源大小
    HRSRC   hRes = FindResource(nullptr, resource_name, resource_type);
    HGLOBAL hMem = LoadResource(nullptr, hRes);
    DWORD   dwSize = SizeofResource(nullptr, hRes);

    // 写入文件
    DWORD dwWrite = 0;      // 返回写入字节
    WriteFile(hFile, hMem, dwSize, &dwWrite, nullptr);
    CloseHandle(hFile);

    return true;
}

// 播放资源中的音乐
MINIVG_INLINE void play_resource_music(PCTSTR filename, PCTSTR resource_type)
{
    std::basic_string<TCHAR> tempfile = ezTempPath();
    tempfile += TEXT("background.mp3");
    // 将 mp3 资源提取为临时文件
    ExtractResource(tempfile.c_str(), resource_type, filename);
    play_resource_music(tempfile.c_str());
}

MINIVG_INLINE void play_resource_music(int id, PCTSTR resource_type)
{
    return play_resource_music(MAKEINTRESOURCE(id), resource_type);
}

// 播放 wav 文件
MINIVG_INLINE int play_sound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_FILENAME | SND_ASYNC;
    if (loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, 0, fdwSound);
}

// 播放资源中的 wav 文件
MINIVG_INLINE int play_resource_sound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_RESOURCE | SND_ASYNC;
    if (loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, GetModuleHandle(nullptr), fdwSound);
}

MINIVG_INLINE int play_resource_sound(int id, bool loop)
{
    return play_resource_sound(MAKEINTRESOURCE(id), loop);
}

// 停止声音播放
MINIVG_INLINE void stop_sound()
{
    PlaySound(nullptr, nullptr, SND_FILENAME);
}

//---------------------------------------------------------------------------
//
// 对话框
//
//---------------------------------------------------------------------------

// 消息对话框
MINIVG_INLINE int msgbox(const unistring& message, const unistring& title, int type)
{
    return MessageBoxW(graph_window(), message.c_str(), title.c_str(), type);
}

// 显示输入框
MINIVG_INLINE unistring inputbox(const unistring& message, const unistring& title, const unistring& default_value)
{
    detail::ezInputBox box;
    if (box.execute(graph_window(), title, message, default_value)) {
        return box.text();
    }
    else {
        return unistring();
    }
}

}// end namespace minivg

#endif //EZGDI_INL_20200708233153
