/*
 Copyright (c) 2005-2020 sdragonx (mail:sdragonx@foxmail.com)

 ezgdi.inl

 2020-07-08 23:31:53

*/
#ifndef EZGDI_INL_20200708233153
#define EZGDI_INL_20200708233153

#pragma once

#include "ezgdi.hpp"

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <ole2.h>           // CreateStreamOnHGlobal

#include <process.h>

#pragma warning(disable: 4717)

#if defined(__BORLANDC__) || defined(_MSC_VER)
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ole32.lib")
#endif

#define EZAPP_VERSION       PCWSTR(L"Easy Graphics 2021-07-30")

#define EZAPP_CLASS_NAME    PCWSTR(L"ezWindow")
#define EZAPP_DEFAULT_FONT  PCWSTR(L"微软雅黑")

#define EZAPP_TIMER_ID      1

namespace ezapi{

//---------------------------------------------------------------------------
//
// 资源管理类
//
//---------------------------------------------------------------------------

// 安全删除指针
template<typename T>
void safe_delete(T* &p)
{
    if(p){
        delete p;
        p = NULL;
    }
}

template<typename T>
inline void delete_object(T& obj)
{
    DeleteObject(obj);
    obj = NULL;
}

// 删除stl容器里面的内容
template<typename T>
void delete_all(T& obj)
{
    typename T::iterator itr = obj.begin();
    for( ; itr != obj.end(); ++itr){
        delete itr->second;
    }
    obj.clear();
}

// 创建一个 HBITMAP
inline HBITMAP bm_create(int width, int height, int pixelbits = 32)
{
    HBITMAP hBitmap;
    BITMAPV5HEADER bi;
    void *lpBits = 0;

    ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = width;
    bi.bV5Height = height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = pixelbits;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask   = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask  = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    hBitmap = CreateDIBSection(GetDC(NULL), (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, (DWORD)0);

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
        ezImage* bmp = NULL;
        std::map<unistring, ezImage*>::iterator itr;
        itr = images.find(name);
        if(itr == images.end()){
            bmp = new ezImage;
            if(bmp->open(name) == EZ_OK){
                images[name] = bmp;
            }
            else{
                delete bmp;
            }
        }
        else{
            bmp = itr->second;
        }
        return bmp;
    }

    // 加载资源图片
    ezImage* loadimage(int id, PCTSTR resource_type)
    {
        ezImage* bmp = NULL;
        std::map<int, ezImage*>::iterator itr;
        itr = resource_images.find(id);
        if(itr == resource_images.end()){
            bmp = new ezImage;
            if(bmp->open(id, resource_type) == EZ_OK){
                resource_images[id] = bmp;
            }
            else{
                delete bmp;
            }
        }
        else{
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
        if(itr != image_pool.end()){
            delete *itr;
            image_pool.erase(itr);
        }
    }

    // 释放所有资源，这个函数在程序退出的时候执行
    void dispose()
    {
        delete_all(images);
        delete_all(resource_images);

        for(size_t i = 0; i < image_pool.size(); ++i){
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
    ezWindow() : m_handle() { }

    HWND handle()const { return m_handle; }

    int create(
        PCWSTR className, 
        PCWSTR title,
        int x, int y, int width, int height,
        DWORD style = WS_OVERLAPPEDWINDOW,
        DWORD styleEx = 0)
    {
        InitClass(className, 0, basic_wndproc);

        #ifndef UNICODE
        std::string buf = to_ansi(title, -1);
        title = (PCWSTR)buf.c_str();
        #endif

        //创建窗口
        m_handle = CreateWindowExW(
            styleEx,        //窗口的扩展风格
            className,      //类名
            title,          //标题
            style,          //风格
            x,              //左边位置
            y,              //顶部位置
            width,          //宽度
            height,         //高度
            NULL,           //父窗口的句柄
            NULL,           //菜单的句柄或是子窗口的标识符
            GetModuleHandle(NULL),    //应用程序实例的句柄
            this);          //指向窗口的创建数据

        if (m_handle == NULL) {
            MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
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
        SetWindowPos(m_handle, NULL, x, y, 0, 0, SWP_NOSIZE);
    }

    // 设置窗口大小
    void resize(int width, int height)
    {
        SetWindowPos(m_handle, NULL, 0, 0, width, height, SWP_NOMOVE);
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
        if (!m_handle){
            return 0;
        }
        ShowWindow(m_handle, SW_SHOW);
        UpdateWindow(m_handle);
        if(owner){
            SetWindowLongPtr(m_handle, GWLP_HWNDPARENT, (LONG_PTR)owner);
            EnableWindow(owner, FALSE);
        }

        MSG msg;
        while(GetMessage(&msg, 0, 0, 0)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(owner){
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
        GetWindowTextW(m_handle, (wchar_t*)&buf[0], size + 1);
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
        SendMessage(m_handle, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    //这个函数不工作
    HFONT getFont()
    {
        return (HFONT)(UINT_PTR)SendMessage(m_handle, WM_GETFONT, 0, 0);
    }

    void repaint()
    {
        RECT rc;
        GetClientRect(m_handle, &rc);
        RedrawWindow(m_handle, &rc, 0, RDW_UPDATENOW|RDW_INVALIDATE|RDW_NOERASE);
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
        if(m_handle) {
            return DefWindowProc(m_handle, msg, wparam, lparam);
        }
        else {
            return 0;
        }
    }
    
    void MotifyStyle(DWORD style, bool enable)
    {
        if(enable){
            SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE) | style);
        }
        else{
            SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE) & (~style));
        }
    }

    //创建控件
    HWND CreateComponent(ezWindow* parent, PCWSTR classname, int x, int y, int width, int height, int style, int styleEx = 0)
    {
        HWND hwnd = CreateWindowExW(styleEx, classname, NULL, style,
            x, y, width, height, parent->handle(), (HMENU)NULL, GetModuleHandle(NULL), NULL);

        HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hwnd, WM_SETFONT, (WPARAM)font, 0);
        return hwnd;
    }

    //注册窗口类
    int InitClass(PCWSTR className, int style, WNDPROC wndproc)
    {
        int atom = 0;
        WNDCLASSEXW wc;
        HINSTANCE hInstance = GetModuleHandle(NULL);

        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;// CS_DBLCLKS 支持鼠标双击事件
        wc.lpfnWndProc = wndproc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
        wc.lpszClassName = className;
        wc.hIcon = LoadIconW(hInstance, L"ICO_MAIN");
        wc.hIconSm = LoadIconW(hInstance, L"ICO_MAIN");

        atom = RegisterClassExW(&wc);

        if (!atom) {
            //MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION|MB_OK);
            return -1;
        }
        return atom;
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
            m_handle = NULL;
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
// EZGDI 实例类
//
//---------------------------------------------------------------------------

template<typename T = int>
class ezInstance : public ezWindow
{
public:
    HDC hdc;                        //GDI 绘图设备
    Gdiplus::Graphics* g;           //GDIPlus 设备
    HBITMAP pixelbuf;               //像素缓冲区

    Gdiplus::Pen* pen;              //画笔
    Gdiplus::SolidBrush* brush;     //画刷
    Gdiplus::Font* font;            //字体
    Gdiplus::SolidBrush* font_color;
    unistring fontName;
    float    fontSize;
    int      fontStyle;
    bool     fontIsChange;

    WNDPROC prevWndProc;            //如果是设置已有的窗口，保存已有窗口的消息处理函数

    int initParam;                  //创建参数
    RECT winRect;                   //窗口模式下窗口大小
    bool fullscreen;                //是否全屏

    vec4i vp;                       //视口
    bool running;                   //程序是否运行

    EZ_KEY_EVENT OnKeyDown;         //键盘事件
    EZ_KEY_EVENT OnKeyUp;
    EZ_KEY_EVENT OnKeyPress;

    EZ_MOUSE_EVENT OnMouseDown;     //鼠标事件
    EZ_MOUSE_EVENT OnMouseUp;
    EZ_MOUSE_EVENT OnMouseMove;

    EZ_TIMER_EVENT OnTimer;         //计时器事件
    EZ_PAINT_EVENT OnPaint;         //窗口绘制事件

    ezResource resource;            //资源管理器

    static ezInstance instance;

private:
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;

public:
    ezInstance() : ezWindow(),
        hdc(), g(), pixelbuf(),
        pen(), brush(), font(),
        fontName(EZAPP_DEFAULT_FONT),
        fontSize(12),
        fontStyle(EZ_NORMAL),
        running(true),
        OnKeyDown(), OnKeyUp(), OnKeyPress(),
        OnMouseDown(), OnMouseUp(), OnMouseMove(),
        OnTimer(),
        OnPaint()
    {
        prevWndProc = NULL;
        gdiplusInit();
    }

    ~ezInstance()
    {
        resource.dispose();
        gdiplusShutdown();
    }

    // 设置到已有的窗口
    void setWindow(HWND hwnd)
    {
        if(hwnd) {
            m_handle = hwnd;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
            prevWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)basic_wndproc);
        }
        else {
            if(prevWndProc) {
                SetWindowLongPtr(m_handle, GWLP_WNDPROC, (LONG_PTR)prevWndProc);
                m_handle = NULL;
                prevWndProc = NULL;

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

        switch(style){
        case EZ_FIXED:
            dwStyle = WS_POPUPWINDOW|WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX;
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

        if(!width || !height) {
            return ;
        }

        if (width != vp.width || height != vp.height) {
            pixelbuf = bm_create(width, height);
            SelectObject(hdc, pixelbuf);
            g = new Gdiplus::Graphics(hdc);
        }

        vp = vec4i(x, y, width, height);
    }

    // 将缓冲区的图像绘制到目标 HDC
    void bitblt(HDC dc)
    {
        BitBlt(dc, vp.x, vp.y, vp.width, vp.height, hdc, 0, 0, SRCCOPY);
    }

    // 设置窗口置顶
    bool topmose(bool top)
    {
        if(top){
            return SetWindowPos(m_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
        }
        else{
            return SetWindowPos(m_handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
        }
    }

    // 主窗口消息
    LRESULT wndproc(UINT Message, WPARAM wParam, LPARAM lParam)
    {
        switch(Message){
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
            if(OnTimer)OnTimer();
            break;

        case WM_KEYDOWN:
            if(OnKeyDown)OnKeyDown(int(wParam));
            break;
        case WM_KEYUP:
            if(OnKeyUp)OnKeyUp(int(wParam));
            break;
        case WM_CHAR:
            if(OnKeyPress)OnKeyPress(int(wParam));
            break;

        case WM_MOUSEMOVE:
            if(OnMouseMove)OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), int(wParam) & 0x13);
            break;
        case WM_LBUTTONDOWN:
            if(OnMouseDown)OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_LEFT);
            break;
        case WM_LBUTTONUP:
            if(OnMouseUp)OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_LEFT);
            break;
        case WM_RBUTTONDOWN:
            if(OnMouseDown)OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_RIGHT);
            break;
        case WM_RBUTTONUP:
            if(OnMouseUp)OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_RIGHT);
            break;
        case WM_MBUTTONDOWN:
            if(OnMouseDown)OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_MIDDLE);
            break;
        case WM_MBUTTONUP:
            if(OnMouseUp)OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EZ_MIDDLE);
            break;

        default:
            break;
        }

        if(prevWndProc){
            return CallWindowProc(prevWndProc, m_handle, Message, wParam, lParam);
        }
        else{
            return ezWindow::wndproc(Message, wParam, lParam);
        }
    }

protected:
    void gdiplusInit()
    {
        Gdiplus::GdiplusStartup(&token, &input, NULL);

        pen = new Gdiplus::Pen(Gdiplus::Color::Black);
        brush = new Gdiplus::SolidBrush(Gdiplus::Color::White);
        font = new Gdiplus::Font(fontName.c_str(), 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint, NULL);
        font_color = new Gdiplus::SolidBrush(Gdiplus::Color::Black);

        hdc = CreateCompatibleDC(NULL);
    }

    void gdiplusShutdown()
    {
        closeGraphics();
        delete_object(hdc);
        safe_delete(pen);
        safe_delete(brush);
        safe_delete(font);
        safe_delete(font_color);

        Gdiplus::GdiplusShutdown(token);
    }

    void closeGraphics()
    {
        safe_delete(g);
        if (pixelbuf) {
            delete_object(pixelbuf);
        }
    }

    // 窗口重绘事件
    void OnWindowPaint()
    {
        PAINTSTRUCT ps;
        BeginPaint(m_handle, &ps);
        if(OnPaint)OnPaint();
        this->bitblt(ps.hdc);
        EndPaint(m_handle, &ps);
    }
};

template<typename T>
ezInstance<T> ezInstance<T>::instance = ezInstance<T>();

}//end namespace ezapi

//返回ezgdi的实例
EZ_PUBLIC_DECLARE ezapi::ezInstance<>& __ezgdi_instance = ezapi::ezInstance<>::instance;

//---------------------------------------------------------------------------
//
// 主函数
//
//---------------------------------------------------------------------------

inline int initgraph(const unistring& title, int width, int height, int param)
{
    setlocale(LC_ALL, "");//c中文
    std::locale::global(std::locale(""));//c++中文

    // 保存创建参数
    __ezgdi_instance.initParam = param;

    if(param & EZ_BACKBUFFER){
        __ezgdi_instance.viewport(0, 0, width, height);
        __ezgdi_instance.winRect.left = 0;
        __ezgdi_instance.winRect.top = 0;
        __ezgdi_instance.winRect.right = width;
        __ezgdi_instance.winRect.bottom = height;
    }
    else{
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

        __ezgdi_instance.winRect.left = cx;
        __ezgdi_instance.winRect.top = cy;
        __ezgdi_instance.winRect.right = cx + width;
        __ezgdi_instance.winRect.bottom = cy + height;

        DWORD dwStyle;
        DWORD dwExStyle;

        switch(param){
        case EZ_FIXED:
            dwStyle = WS_POPUPWINDOW|WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX;
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

        // 创建一个窗口
        __ezgdi_instance.create(EZAPP_CLASS_NAME, title.c_str(),
            cx, cy, width, height,
            dwStyle, dwExStyle);

        __ezgdi_instance.show();
    }

    return 0;
}

inline int initgraph(int width, int height, int style)
{
    return initgraph(EZAPP_VERSION, width, height, style);
}

inline int initgraph(HWND hwnd)
{
    setlocale(LC_ALL, "");//c中文
    std::locale::global(std::locale(""));//c++中文

    //创建一个窗口
    __ezgdi_instance.setWindow(hwnd);

    __ezgdi_instance.show();

    return 0;
}

inline void quit()
{
    if(__ezgdi_instance.prevWndProc){
        __ezgdi_instance.setWindow(NULL);
    }
    else {
        __ezgdi_instance.close();
    }
}

inline HWND graph_window()
{
    return __ezgdi_instance.handle();
}

//获得GDI绘图设备
inline HDC graph_hdc()
{
    return __ezgdi_instance.hdc;
}

inline void set_title(const unistring& text)
{
    __ezgdi_instance.setTitle(text);
}

inline void reshape(int width, int height)
{
    HWND hwnd = graph_window();
    RECT rcWindow;
    RECT rcClient;
    int borderWidth, borderHeight;

    GetWindowRect(hwnd, &rcWindow);
    GetClientRect(hwnd, &rcClient);

    borderWidth = (rcWindow.right-rcWindow.left) - (rcClient.right-rcClient.left);
    borderHeight = (rcWindow.bottom-rcWindow.top) - (rcClient.bottom-rcClient.top);

    SetWindowPos(hwnd, 0, 0, 0, borderWidth + width, borderHeight + height, SWP_NOMOVE | SWP_NOZORDER);
}

inline void fullscreen(bool value)
{
    if(value != __ezgdi_instance.fullscreen){
        if(value) {
            GetWindowRect(__ezgdi_instance.handle(), &__ezgdi_instance.winRect);

            __ezgdi_instance.setStyle(EZ_FULLSCREEN);
            __ezgdi_instance.fullscreen = true;

            // 获得屏幕大小
            int cx = GetSystemMetrics( SM_CXSCREEN );
            int cy = GetSystemMetrics( SM_CYSCREEN );
            SetWindowPos(__ezgdi_instance.handle(), 0, 0, 0, cx, cy, SWP_NOZORDER);
        }
        else {
            __ezgdi_instance.fullscreen = false;
            __ezgdi_instance.setStyle(EZ_FIXED);

            __ezgdi_instance.setBounds(
                __ezgdi_instance.winRect.left,
                __ezgdi_instance.winRect.top,
                __ezgdi_instance.winRect.right - __ezgdi_instance.winRect.left,
                __ezgdi_instance.winRect.bottom - __ezgdi_instance.winRect.top);
        }
    }
}

// 消息循环处理
inline bool do_events()
{
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return __ezgdi_instance.running;
}

// 屏幕更新线程
inline void thread_proc(void* arg)
{
    while (__ezgdi_instance.running) {
        // 刷新窗口
        __ezgdi_instance.repaint(); 
    }
}

// 程序执行
inline int start_app()
{
    // 启动刷新线程
    _beginthread(thread_proc, 1024, NULL);//default 1k

    while(do_events());

    return 0;
}

// 显示fps
inline void show_fps()
{
    static DWORD t = GetTickCount();
    static int fps_total = 0;
    static int fps = 0;

    ++fps;

    unistring str = L"FPS:";
    str += unistring(fps_total);

    //ezgdi_font(L"微软雅黑", 16);
    font_color(255, 255, 255);
    textout(0, 0, str);

    if(GetTickCount() - t > 1000){
        t = GetTickCount();
        fps_total = fps;
        fps = 0;
    }
}

inline std::basic_string<TCHAR> ezTempPath()
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
inline bool keystate(int key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

//键盘事件映射
inline void key_push_event(EZ_KEY_EVENT function)
{
    __ezgdi_instance.OnKeyDown = function;
}

inline void key_pop_event(EZ_KEY_EVENT function)
{
    __ezgdi_instance.OnKeyUp = function;
}

inline void input_event(EZ_KEY_EVENT function)
{
    __ezgdi_instance.OnKeyPress = function;
}

//鼠标事件映射
inline void mouse_push_event(EZ_MOUSE_EVENT function)
{
    __ezgdi_instance.OnMouseDown = function;
}

inline void mouse_pop_event(EZ_MOUSE_EVENT function)
{
    __ezgdi_instance.OnMouseUp = function;
}

inline void mouse_move_event(EZ_MOUSE_EVENT function)
{
    __ezgdi_instance.OnMouseMove = function;
}

inline void start_timer(UINT interval)
{
    if(interval){
        SetTimer(graph_window(), UINT(EZAPP_TIMER_ID), interval, NULL);
    }
    else{
        KillTimer(graph_window(), UINT(EZAPP_TIMER_ID));
    }
}

inline void stop_timer()
{
    KillTimer(graph_window(), UINT(EZAPP_TIMER_ID));
}

//计时器事件
inline void timer_event(EZ_TIMER_EVENT function)
{
    __ezgdi_instance.OnTimer = function;
}

//窗口绘制事件
inline void display_event(EZ_PAINT_EVENT function)
{
    __ezgdi_instance.OnPaint = function;
}

//---------------------------------------------------------------------------
//
// 绘图函数
//
//---------------------------------------------------------------------------

// 获得 GDI+ 绘图设备
inline Gdiplus::Graphics* graphics()
{
    return __ezgdi_instance.g;
}

// 重设背景缓冲区大小位置
void viewport(int x, int y, int width, int height)
{
    __ezgdi_instance.viewport(0, 0, width, height);
}

// 背景缓冲绘制到目标 HDC
inline void framebuf_blt(HDC hdc)
{
    __ezgdi_instance.bitblt(hdc);
}

// 设置显示质量
inline int effect_level(int level)
{
    Gdiplus::Graphics* g = __ezgdi_instance.g;
    if(!g){
        return -1;
    }

    switch(level){
    case EZ_SPEED:
        g->SetCompositingMode( Gdiplus::CompositingModeSourceOver );            //混合模式
        g->SetCompositingQuality( Gdiplus::CompositingQualityHighSpeed );       //混合质量
        g->SetSmoothingMode( Gdiplus::SmoothingModeHighSpeed );                 //反锯齿
        g->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );                  //像素偏移模式
        g->SetInterpolationMode( Gdiplus::InterpolationModeNearestNeighbor );   //图形缩放质量
        break;
    case EZ_MEDIUM:
        g->SetCompositingMode( Gdiplus::CompositingModeSourceOver );            //混合模式
        g->SetCompositingQuality( Gdiplus::CompositingQualityHighSpeed );       //混合质量
        g->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );                 //反锯齿
        g->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );                  //像素偏移模式
        g->SetInterpolationMode( Gdiplus::InterpolationModeBilinear );          //图形缩放质量
        break;
    case EZ_QUALITY:
        g->SetCompositingMode( Gdiplus::CompositingModeSourceOver );            //混合模式
        g->SetCompositingQuality( Gdiplus::CompositingQualityHighQuality );     //混合质量
        g->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );                 //反锯齿
        g->SetPixelOffsetMode( Gdiplus::PixelOffsetModeHighQuality );           //像素偏移模式
        g->SetInterpolationMode( Gdiplus::InterpolationModeBicubic );           //图形缩放质量
        break;
    default:
        break;
    }
    return 0;
}

// 清屏
inline void clear(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.g)__ezgdi_instance.g->Clear(Gdiplus::Color(a, r, g, b));
}

//更改画笔颜色
inline void pen_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetColor(Gdiplus::Color(a, r, g, b));
}

inline void pen_color(COLORREF argb)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetColor(Gdiplus::Color(argb));
}

inline void pen_color(vec4ub color)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetColor(Gdiplus::Color(color.a, color.r, color.g, color.b));
}

// 获取画笔颜色
inline COLORREF pen_color()
{
    Gdiplus::Color color;
    if(__ezgdi_instance.pen){
        __ezgdi_instance.pen->GetColor(&color);
    }
    return color.GetValue();
}

/*
vec4ub pen_color()
{
    Gdiplus::Color color;
    if(__ezgdi_instance.pen){
        __ezgdi_instance.pen->GetColor(&color);
    }
    return vec4ub(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}
*/

//画笔宽度
inline void pen_width(float width)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetWidth(width);
}

//设置画笔模式
inline void pen_style(int mode)
{
    if(__ezgdi_instance.pen){
        //if(mode != EZ_CUSTOM){//取消自定义点画模式
        //    ezDashStyle(NULL, 0);
        //}
        __ezgdi_instance.pen->SetDashStyle(Gdiplus::DashStyle(mode));
    }
}

//设置点画模式间隔
inline void dash_style(const float* dash, int size)
{
    if(__ezgdi_instance.pen){
        //if(__ezgdi_instance.pen->GetDashStyle() == Gdiplus::DashStyleCustom){
            __ezgdi_instance.pen->SetDashPattern(dash, size);
        //}
    }
}

//更改填充颜色
inline void fill_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.brush)__ezgdi_instance.brush->SetColor(Gdiplus::Color(a, r, g, b));
}

inline void fill_color(COLORREF argb)
{
    if(__ezgdi_instance.brush)__ezgdi_instance.brush->SetColor(Gdiplus::Color(argb));
}

//获取填充颜色
inline COLORREF fill_color()
{
    Gdiplus::Color color;
    if(__ezgdi_instance.brush){
        __ezgdi_instance.brush->GetColor(&color);
    }
    return color.GetValue();
}

//绘制一个点
inline void draw_point(float x, float y, float size)
{
    if(__ezgdi_instance.g){
        Gdiplus::SolidBrush brush(pen_color());
        float half = size * 0.5f;
        __ezgdi_instance.g->FillEllipse(&brush, x - half, y - half, size, size);
    }
}

//绘制线段
inline void draw_line(float x1, float y1, float x2, float y2)
{
    if(__ezgdi_instance.g)__ezgdi_instance.g->DrawLine(__ezgdi_instance.pen, x1, y1, x2, y2);
}

//绘制一个空心矩形
inline void draw_rect(float x, float y, float width, float height)
{
    if(__ezgdi_instance.g)__ezgdi_instance.g->DrawRectangle(__ezgdi_instance.pen, x, y, width, height);
}

//填充一个矩形
inline void fill_rect(float x, float y, float width, float height)
{
    if(__ezgdi_instance.g)__ezgdi_instance.g->FillRectangle(__ezgdi_instance.brush, x, y, width, height);
}

//绘制圆角矩形
inline void draw_roundrect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if(cx > width)cx = width;
    if(cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = __ezgdi_instance.g;
    if(g){
        Gdiplus::GraphicsPath path;
        path.AddArc(x, y, cx, cy, 180, 90);
        path.AddArc(x2, y, cx, cy, 270, 90);
        path.AddArc(x2, y2, cx, cy, 0, 90);
        path.AddArc(x, y2, cx, cy, 90, 90);
        path.CloseFigure();
        g->DrawPath(__ezgdi_instance.pen, &path);
    }
}

//填充圆角矩形
inline void fill_roundrect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if(cx > width)cx = width;
    if(cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = __ezgdi_instance.g;
    if(g){
        Gdiplus::GraphicsPath path;
        path.AddArc(x, y, cx, cy, 180, 90);
        path.AddArc(x2, y, cx, cy, 270, 90);
        path.AddArc(x2, y2, cx, cy, 0, 90);
        path.AddArc(x, y2, cx, cy, 90, 90);
        path.CloseFigure();
        g->FillPath(__ezgdi_instance.brush, &path);
    }
}

//绘制椭圆，xy为圆心
inline void draw_ellipse(float x, float y, float cx, float cy)
{
    if(__ezgdi_instance.g)__ezgdi_instance.g->DrawEllipse(__ezgdi_instance.pen, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//填充椭圆
inline void fill_ellipse(float x, float y, float cx, float cy)
{
    if(__ezgdi_instance.g)__ezgdi_instance.g->FillEllipse(__ezgdi_instance.brush, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//绘制空心圆，xy为圆心
inline void draw_circle(float x, float y, float r)
{
    return draw_ellipse(x, y, r, r);
}

//填充圆
inline void fill_circle(float x, float y, float r)
{
    return fill_ellipse(x, y, r, r);
}

//绘制连续的线段
inline void draw_polyline(const vec2f* points, size_t size)
{
    if(__ezgdi_instance.g){
        __ezgdi_instance.g->DrawLines(__ezgdi_instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//绘制多边形
inline void draw_polygon(const vec2f* points, size_t size)
{
    if(__ezgdi_instance.g){
        __ezgdi_instance.g->DrawPolygon(__ezgdi_instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//填充多边形
inline void fill_polygon(const vec2f* points, size_t size)
{
    if(__ezgdi_instance.g){
        __ezgdi_instance.g->FillPolygon(__ezgdi_instance.brush, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//---------------------------------------------------------------------------
//
// 字体函数
//
//---------------------------------------------------------------------------

//设置字体。字体名字、大小、风格
inline void setfont(const unistring& name, float size, EZGDI_FONTSTYLE style)
{
    if(__ezgdi_instance.font){
        delete __ezgdi_instance.font;
        __ezgdi_instance.font = new Gdiplus::Font(name.c_str(), size, style, Gdiplus::UnitPoint, NULL);
    }
}

inline void setfont(const unistring& name, float size, bool bold, bool, bool underline, bool strikeout)
{
    EZGDI_FONTSTYLE style = EZ_NORMAL;
    if (bold) {
        style = (EZGDI_FONTSTYLE)(style | EZ_BOLD);
    }
    if (underline) {
        style = (EZGDI_FONTSTYLE) (style | EZ_UNDERLINE);
    }
    if (strikeout) {
        style = (EZGDI_FONTSTYLE) (style | EZ_STRIKEOUT);
    }
    setfont(name, size, style);
}

inline void font_name(const unistring& name)
{
    __ezgdi_instance.fontName = name;
    __ezgdi_instance.fontIsChange = true;
}

inline void font_size(float size)
{
    __ezgdi_instance.fontSize = size;
    __ezgdi_instance.fontIsChange = true;
}

inline void font_style(int style)
{
    __ezgdi_instance.fontStyle = style;
    __ezgdi_instance.fontIsChange = true;
}

//获取字体属性
//unistring font_name();
//int font_size();
//int font_style();

//字体颜色
inline void font_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.font_color)__ezgdi_instance.font_color->SetColor(Gdiplus::Color(a, r, g, b));
}

inline void font_color(COLORREF color)
{
    if(__ezgdi_instance.font_color)__ezgdi_instance.font_color->SetColor(Gdiplus::Color(color));
}

//输出字体
inline void textout(float x, float y, const char* text, size_t length)
{
    textout(x, y, unistring(text, length));
}

inline void textout(float x, float y, const wchar_t* text, size_t length)
{
    if(__ezgdi_instance.g){
        if(__ezgdi_instance.fontIsChange){
            setfont(__ezgdi_instance.fontName, __ezgdi_instance.fontSize, EZGDI_FONTSTYLE(__ezgdi_instance.fontStyle));
            __ezgdi_instance.fontIsChange = false;
        }

        Gdiplus::StringFormat format;
        __ezgdi_instance.g->DrawString(text, INT(length), __ezgdi_instance.font,
            Gdiplus::PointF(x, y), &format, __ezgdi_instance.font_color);
    }
}

inline void textout(float x, float y, const unistring& text)
{
    return textout(x, y, text.c_str(), text.length());
}

inline void drawtext(float x, float y, float width, float height, const unistring& text, int align)
{
    if(__ezgdi_instance.g){
        if(__ezgdi_instance.fontIsChange){
            setfont(__ezgdi_instance.fontName, __ezgdi_instance.fontSize, EZGDI_FONTSTYLE(__ezgdi_instance.fontStyle));
            __ezgdi_instance.fontIsChange = false;
        }

        Gdiplus::StringFormat format;

        int hAlign = 0;
        int vAlign = 0;
        if((align & EZ_CENTER_H) == EZ_CENTER_H){
            hAlign = Gdiplus::StringAlignmentCenter;
        }
        else if(align & EZ_RIGHT){
            hAlign = Gdiplus::StringAlignmentFar;
        }

        if((align & EZ_CENTER_V) == EZ_CENTER_V){
            vAlign = Gdiplus::StringAlignmentCenter;
        }
        else if(align & EZ_DOWN){
            vAlign = Gdiplus::StringAlignmentFar;
        }

        format.SetAlignment((Gdiplus::StringAlignment)hAlign);//水平对齐
        format.SetLineAlignment((Gdiplus::StringAlignment)vAlign);//垂直对齐
        Gdiplus::RectF rect(x, y, width, height);
        __ezgdi_instance.g->DrawString(text.c_str(), INT(text.length()),
            __ezgdi_instance.font,
            rect,
            &format,
            __ezgdi_instance.font_color);

    }
}

// 字体格式化输出，和printf使用类似
inline void print(float x, float y, const char* param, ...)
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

inline void print(float x, float y, const wchar_t* param, ...)
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

//获得字符串的像素宽度
inline float textwidth(const unistring& text)
{
    if(__ezgdi_instance.g){
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        __ezgdi_instance.g->MeasureString(text.c_str(), int(text.length()), __ezgdi_instance.font, layoutSize, NULL, &size);
        return size.Width;
    }
    return 0;
}

//获得字符串的像素高度
inline float textheight(const unistring& text)
{
    if(__ezgdi_instance.g){
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        __ezgdi_instance.g->MeasureString(text.c_str(), int(text.length()), __ezgdi_instance.font, layoutSize, NULL, &size);
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

inline Gdiplus::Bitmap* ezImage::handle()const
{
    return m_handle;
}

//创建一个图片，默认为32位色
inline int ezImage::create(int width, int height, int format)
{
    this->close();
    m_handle = new Gdiplus::Bitmap(width, height, format);
    return 0;
}

//打开一个图片，支持bmp、jpg、png、静态gif等常见格式
inline int ezImage::open(const unistring& filename)
{
    this->close();
    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(filename.c_str());
    if(bmp){
        m_handle = bmp->Clone(0, 0, bmp->GetWidth(), bmp->GetHeight(), PixelFormat32bppPARGB);
        delete bmp;
        return 0;
    }
    return -1;
}

//从资源中加载图片
inline Gdiplus::Bitmap* LoadResourceImage(UINT id, PCTSTR type)
{
	Gdiplus::Bitmap* image = NULL;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	if (type == RT_BITMAP) {
		return Gdiplus::Bitmap::FromResource(GetModuleHandle(NULL), MAKEINTRESOURCEW(id));
	}

	HRSRC hResSource = ::FindResource(hInstance, MAKEINTRESOURCE(id), type);
	if (hResSource) {
		// load resource into memory
		DWORD size = SizeofResource(hInstance, hResSource);
		BYTE* source = (BYTE*)LoadResource(hInstance, hResSource);
		if (source) {
			// allocate global memory on which to create stream
			HGLOBAL hmem = GlobalAlloc(GMEM_FIXED, size);
			if (hmem) {
				// copy data to global memory
				BYTE* dest = (BYTE*)GlobalLock(hmem);
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

//打开资源中的图片
inline int ezImage::open(int id, PCTSTR resource_type)
{
    this->close();
    Gdiplus::Bitmap* bmp  = LoadResourceImage(id, resource_type);
    if(bmp){
        m_handle = bmp->Clone(0, 0, bmp->GetWidth(), bmp->GetHeight(), PixelFormat32bppPARGB);
        delete bmp;
        return 0;
    }
    return -1;
}

//映射一个HBITMAP对象
inline int ezImage::map(HBITMAP hbmp)
{
    this->close();
    BITMAP bm;
    GetObject(hbmp, sizeof(bm), &bm);
    if(bm.bmBits){
        BYTE* pixels = ((BYTE*)bm.bmBits) + (bm.bmHeight - 1) * bm.bmWidthBytes;
        m_handle = new Gdiplus::Bitmap(bm.bmWidth, bm.bmHeight, -bm.bmWidthBytes, PixelFormat32bppARGB, pixels);
    }
    return 0;
}

//图片格式
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_BMP  = L"image/bmp";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_JPG  = L"image/jpeg";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_GIF  = L"image/gif";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_TIFF = L"image/tiff";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_PNG  = L"image/png";

inline int GetImageCLSID(const WCHAR* format, CLSID* pCLSID)
{
    //得到格式为format的图像文件的编码值，访问该格式图像的COM组件的GUID值保存在pCLSID中
    UINT num  = 0;
    UINT size = 0;

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
    Gdiplus::GetImageEncodersSize(&num, &size);

    if(size == 0)return -1;    //编码信息不可用

    //分配内存
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if(!pImageCodecInfo)return -2;    //分配失败

    //获得系统中可用的编码方式的所有信息
    GetImageEncoders(num, size, pImageCodecInfo);

    //在可用编码信息中查找format格式是否被支持
    for(UINT i = 0; i < num; ++i){
        //MimeType：编码方式的具体描述
        if(wcscmp(pImageCodecInfo[i].MimeType, format) == 0){
            *pCLSID = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return 0;    //成功
        }
    }

    free(pImageCodecInfo);
    return -3;
}

inline int ezImage::save(const unistring& filename)
{
    if(m_handle){
        CLSID id;
        if(!GetImageCLSID(GDIPLUS_IMAGE_PNG, &id)){
            return m_handle->Save(filename.c_str(), &id, NULL);
        }
        return 0;
    }
    return -1;
}

//自动释放图片
inline void ezImage::close()
{
    if(m_handle){
        delete m_handle;
        m_handle = NULL;
    }
}

//返回图片的宽度
inline int ezImage::width()const
{
    return m_handle ? m_handle->GetWidth() : 0;
}

//返回图片的高度
inline int ezImage::height()const
{
    return m_handle ? m_handle->GetHeight() : 0;
}

// 获取图像数据
inline void* ezImage::lock()
{
    if(m_data){
        msgbox(L"图片已经锁定。", L"错误");
    }
    else{
        m_data = new Gdiplus::BitmapData();
    	Gdiplus::Rect rect(0, 0, this->width(), this->height());
	    Gdiplus::Status n = m_handle->LockBits(0, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, m_data);
        if(n == Gdiplus::Ok){
            return m_data->Scan0;
        }
        else{
            return NULL;
        }
    }
}

// 还原图像数据
inline void ezImage::unlock()
{
    if(m_data){
        m_handle->UnlockBits(m_data);
        ezapi::safe_delete(m_data);
    }
}

//
// API 部分
//

// 创建图片
ezImage* newimage(int width, int height)
{
    return __ezgdi_instance.resource.allocate(width, height);
}

// 删除图片
void freeimage(ezImage* image)
{
    __ezgdi_instance.resource.free(image);
}

inline ezImage* loadimage(const unistring& filename)
{
    return __ezgdi_instance.resource.loadimage(filename);
}

inline ezImage* loadimage(int id, PCTSTR resource_type)
{
    return __ezgdi_instance.resource.loadimage(id, resource_type);
}

inline int saveimage(ezImage* image, const unistring& filename)
{
    if(image){
        image->save(filename);
    }
    return -1;
}

inline void drawimage(ezImage* image, float x, float y)
{
    if(__ezgdi_instance.g && image && image->handle())__ezgdi_instance.g->DrawImage(image->handle(), x, y);
}

inline void drawimage(ezImage* image, float x, float y, float width, float height)
{
    if(__ezgdi_instance.g && image && image->handle())__ezgdi_instance.g->DrawImage(image->handle(), x, y, width, height);
}

//在xy位置绘制图片，并旋转一个角度
inline void rotate_image(ezImage* image, float x, float y, float rotate)
{
    return rotate_image(image, x, y, float(image->width()), float(image->height()), rotate);
}

//在xy位置绘制图片，缩放，并旋转一个角度
inline void rotate_image(ezImage* image, float x, float y, float width, float height, float rotate)
{
    Gdiplus::Graphics* g = __ezgdi_instance.g;
    if(g && image){
        float cx = width / 2;
        float cy = height / 2;
        Gdiplus::Matrix m;
        g->GetTransform(&m);
        g->TranslateTransform(x, y);//移动到当前位置
        g->RotateTransform(rotate); //旋转
        g->TranslateTransform(-cx, -cy);//移动到旋转中心
        g->DrawImage(image->handle(), 0.0f, 0.0f, width, height);//绘制图像
        g->SetTransform(&m);
    }
}

// 把像素直接绘制到屏幕上，像素格式必须是 BGRA 32位。
void draw_pixels(float x, float y, float width, float height, void* pixels, int pwidth, int pheight)
{
    Gdiplus::Graphics* g = __ezgdi_instance.g;
    if(g) {
        BYTE* data = static_cast<BYTE*>(pixels);
        data += (pwidth * 4) * (pheight - 1);
        Gdiplus::Bitmap bmp(pwidth, pheight, pwidth * 4, PixelFormat32bppARGB, data);
        g->DrawImage(&bmp, x, y, width, height);
    }
}

//---------------------------------------------------------------------------
//
// 多媒体
//
//---------------------------------------------------------------------------

//播放音乐
inline void playmusic(PCTSTR filename)
{
	unistring command = L"open ";
	command.append((const wchar_t*)filename);
	command += L" alias background";

    mciSendString((PCTSTR)command.c_str(), NULL, 0, NULL);
    mciSendString(TEXT("play background repeat"), NULL, 0, NULL);
}

inline void stopmusic()
{
    mciSendString(TEXT("stop background"), NULL, 0, NULL);//!
	mciSendString(TEXT("close background"), NULL, 0, NULL);
}

//导出资源到文件
//参考文章：https://www.cnblogs.com/zjutlitao/p/3577592.html
inline bool ExtractResource(LPCTSTR filename, LPCTSTR resource_type, LPCTSTR resource_name)
{
    // 创建文件
    HANDLE hFile = ::CreateFile(filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hFile == INVALID_HANDLE_VALUE){
        return false;
    }

    // 查找资源文件中、加载资源到内存、得到资源大小
    HRSRC   hRes   = FindResource(NULL, resource_name, resource_type);
    HGLOBAL hMem   = LoadResource(NULL, hRes);
    DWORD   dwSize = SizeofResource(NULL, hRes);

    // 写入文件
    DWORD dwWrite = 0;      // 返回写入字节
    WriteFile(hFile, hMem, dwSize, &dwWrite, NULL);
    CloseHandle(hFile);

    return true;
}

//播放资源中的音乐
inline void play_resource_music(PCTSTR filename, PCTSTR resource_type)
{
	std::basic_string<TCHAR> tempfile = ezTempPath();
    tempfile += TEXT("background.mp3");
    // 将mp3资源提取为临时文件
    ExtractResource(tempfile.c_str(), resource_type, filename);
	play_resource_music(tempfile.c_str());
}

inline void play_resource_music(int id, PCTSTR resource_type)
{
	return play_resource_music(MAKEINTRESOURCE(id), resource_type);
}

//播放wav文件
inline int playsound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_FILENAME|SND_ASYNC;
    if(loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, 0, fdwSound);
}

//播放资源中的wav文件
inline int play_resource_sound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_RESOURCE|SND_ASYNC;
    if(loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, GetModuleHandle(NULL), fdwSound);
}

inline int play_resource_sound(int id, bool loop)
{
    return play_resource_sound(MAKEINTRESOURCE(id), loop);
}

//停止声音播放
inline void stopsound()
{
	PlaySound(NULL, NULL, SND_FILENAME);
}

//---------------------------------------------------------------------------
//
// 对话框
//
//---------------------------------------------------------------------------

//消息对话框
inline int msgbox(const unistring& message, const unistring& title, int type)
{
	return MessageBoxW(graph_window(), message.c_str(), title.c_str(), type);
}

//显示输入框
inline unistring inputbox(const unistring& message, const unistring& title, const unistring& default_value)
{
    ezapi::ezInputBox box;
    if(box.execute(graph_window(), title, message, default_value)){
        return box.text();
    }
    else{
        return unistring();
    }
}


#endif //EZGDI_INL_20200708233153
