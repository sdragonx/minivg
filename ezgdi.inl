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

#pragma warning(disable: 4717)

#if defined(__BORLANDC__) || defined(_MSC_VER)
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")
#endif

#define EZAPP_CLASS_NAME    PCWSTR(L"ezWindow")
#define EZAPP_DEFAULT_FONT  PCWSTR(L"微软雅黑")

#define EZAPP_TIMER_ID      1


namespace ezapi{

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

//创建一个HBITMAP
HBITMAP bm_create(int width, int height, int pixelbits = 32)
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

//资源管理类
class ezResource
{
private:
    std::map<ezstring, ezImage*> images;
    std::map<int, ezImage*> resource_images;

public:
    //加载一个图片
    ezImage* loadimage(const ezstring& name)
    {
        ezImage* bmp = NULL;
        std::map<ezstring, ezImage*>::iterator itr;
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

    //加载资源图片
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

    //释放所有资源
    void dispose()
    {
        delete_all(images);
        delete_all(resource_images);
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

    int create(PCWSTR className, PCWSTR title, int x, int y, int width, int height,
        DWORD style = WS_OVERLAPPEDWINDOW, DWORD styleEx = 0);

    void setBounds(int x, int y, int width, int height) { MoveWindow(m_handle, x, y, width, height, TRUE); }

    void move(int x, int y) { SetWindowPos(m_handle, NULL, x, y, 0, 0, SWP_NOSIZE); }
    void resize(int width, int height) { SetWindowPos(m_handle, NULL, 0, 0, width, height, SWP_NOMOVE); }

    void show() { ShowWindow(m_handle, SW_SHOW); }
    int showModel(HWND parent);

    void setID(int id) { SetWindowLong(m_handle, GWL_ID, id); }

    void setText(const ezstring& text) { SetWindowText(m_handle, text.c_str()); }

    ezstring getText()const
    {
        int size = GetWindowTextLength(m_handle);
        std::vector<wchar_t> buf;
        buf.resize(size + 1);
        GetWindowText(m_handle, &buf[0], size + 1);
        return ezstring(&buf[0], size);
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
    virtual LRESULT wndproc(UINT msg, WPARAM wparam, LPARAM lparam) { return DefWindowProc(m_handle, msg, wparam, lparam); }
    friend LRESULT CALLBACK basic_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    void MotifyStyle(DWORD style, bool enable)
    {
        if(enable){
            SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE) | style);
        }
        else{
            SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE) & (~style));
        }
    }

protected:
    //创建控件
    HWND CreateComponent(ezWindow* parent, PCTSTR classname, int x, int y, int width, int height, int style, int styleEx = 0)
    {
        HWND hwnd = CreateWindowExW(styleEx, classname, NULL, style,
            x, y, width, height, parent->handle(), (HMENU)NULL, GetModuleHandle(NULL), NULL);

        HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hwnd, WM_SETFONT, (WPARAM)font, 0);
        return hwnd;
    }

    int ezWindow::InitClass(PCWSTR className, int style, WNDPROC wndproc);
};

//注册窗口类
int ezWindow::InitClass(PCWSTR className, int style, WNDPROC wndproc)
{
    int atom = 0;
    WNDCLASSEXW wc;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    memset(&wc, 0, sizeof(wc));
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;//CS_DBLCLKS 支持鼠标双击事件
    wc.lpfnWndProc   = wndproc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = className;
    wc.hIcon         = LoadIconW(hInstance, L"ICO_MAIN");
    wc.hIconSm       = LoadIconW(hInstance, L"ICO_MAIN");

    atom = RegisterClassExW(&wc);

    if(!atom){
        //MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION|MB_OK);
        return -1;
    }
    return atom;
}

int ezWindow::create(PCWSTR className, PCWSTR title, int x, int y, int width, int height, DWORD style, DWORD styleEx)
{
    InitClass(className, 0, basic_wndproc);

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

    if(m_handle == NULL){
        MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION|MB_OK);
        return -1;
    }

    return 0;
}

LRESULT CALLBACK basic_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    ezWindow *win = reinterpret_cast<ezWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if(win){
        return win->wndproc(msg, wparam, lparam);
    }
    else if(msg == WM_CREATE){
        LPCREATESTRUCTW pcs = LPCREATESTRUCTW(lparam);
        win = reinterpret_cast<ezWindow*>(pcs->lpCreateParams);
        if(win){
            win->m_handle = hwnd;
            ::SetLastError(ERROR_SUCCESS);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(win));
            if(GetLastError()!=ERROR_SUCCESS)
                return -1;
            else
                return win->wndproc(msg, wparam, lparam);
        }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

int ezWindow::showModel(HWND window)
{
    if (!m_handle){
        return 0;
    }
    ShowWindow(m_handle, SW_SHOW);
    UpdateWindow(m_handle);
    if(window){
        SetWindowLongPtr(m_handle, GWLP_HWNDPARENT, (LONG_PTR)window);
        EnableWindow(window, FALSE);
    }

    MSG msg;
    while(GetMessage(&msg, 0, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if(window){
        EnableWindow(window, TRUE);
        SetForegroundWindow(window);
    }

    return int(msg.wParam);
}


class ezButton : public ezWindow
{
public:
    int create(ezWindow* parent, int x, int y, int width, int height)
    {
        DWORD style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
        m_handle = CreateComponent(parent, TEXT("button"), x, y, width, height, style);
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
//InputBox
//

class ezInputBox : private ezWindow
{
private:
    ezstring m_message;
    ezstring m_text;
    bool m_result;

    ezLabel  lblMessage;
    ezEdit   editbox;
    ezButton btnOK;
    ezButton btnCancel;

public:
    bool execute(HWND parent, const ezstring& title, const ezstring& message, const ezstring& text = ezstring());
    ezstring text()const { return m_text; }

protected:
    void on_create();
    void ButtonOKClick();
    LRESULT wndproc(UINT msg, WPARAM wparam, LPARAM lparam);
};

bool ezInputBox::execute(HWND parent, const ezstring& title, const ezstring& message, const ezstring& text)
{
    int cx = GetSystemMetrics( SM_CXFULLSCREEN );
    int cy = GetSystemMetrics( SM_CYFULLSCREEN );

    int w = 400;
    int h = 150;
    int x = (cx - w) / 2;
    int y = (cy - h) / 2;

    m_message = message;
    m_text = text;
    ezWindow::create(L"EZGDI_InputBox", title.c_str(), x, y, w, h, WS_CAPTION|WS_SYSMENU|WS_CLIPSIBLINGS);

    showModel(parent);

    return m_result;
}

void ezInputBox::on_create()
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

void ezInputBox::ButtonOKClick()
{
    TCHAR buf[256];
    GetDlgItemText(m_handle, 2000, buf, 256);
    m_text = buf;
}

LRESULT ezInputBox::wndproc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch(msg){
    case WM_CREATE:
        on_create();
        break;
    case WM_DESTROY:
        m_handle = NULL;
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        if (wparam == VK_RETURN){
            SendMessage(m_handle, WM_COMMAND, IDOK, 0);
        }
        else if(wparam == VK_ESCAPE){
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
        switch(LOWORD(wparam)){
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
    return DefWindowProc(m_handle, msg, wparam, lparam);;
}

//---------------------------------------------------------------------------
//
// EZGDI 实例类
//
//---------------------------------------------------------------------------

template<typename T = int>
class ezInstance : public ezWindow
{
public:
    HDC hdc;                        //GDI绘图设备
    Gdiplus::Graphics* graphics;    //GDIPlus设备
    HBITMAP pixelbuf;               //像素缓冲区

    Gdiplus::Pen* pen;              //画笔
    Gdiplus::SolidBrush* brush;     //画刷
    Gdiplus::Font* font;            //字体
    Gdiplus::SolidBrush* font_color;
    ezstring fontName;
    float    fontSize;
    int      fontStyle;
    bool     fontIsChange;

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
        hdc(), graphics(), pixelbuf(),
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
        gdiplusInit();
    }

    ~ezInstance()
    {
        resource.dispose();
        gdiplusShutdown();
    }

    //重建背景缓冲区
    void viewport(int x, int y, int width, int height)
    {
        closeGraphics();

        if(!width || !height){
            return ;
        }

        vp = vec4i(x, y, width, height);

        pixelbuf = bm_create(width, height);
        SelectObject(hdc, pixelbuf);
        graphics = new Gdiplus::Graphics(hdc);
    }

    //窗口重绘事件
    int OnWindowPaint()
    {
        PAINTSTRUCT ps;
        BeginPaint(m_handle, &ps);
        if(OnPaint)OnPaint();
        BitBlt(ps.hdc, vp.x, vp.y, vp.width, vp.height, hdc, 0, 0, SRCCOPY);
        EndPaint(m_handle, &ps);
        return 0;
    }

    //设置窗口置顶
    bool Topmose(bool top)
    {
        if(top){
            return SetWindowPos(m_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
        }
        else{
            return SetWindowPos(m_handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
        }
    }

    //主窗口消息
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
        return ezWindow::wndproc(Message, wParam, lParam);
    }
private:
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
        DeleteObject(hdc);
        safe_delete(pen);
        safe_delete(brush);
        safe_delete(font);
        safe_delete(font_color);

        Gdiplus::GdiplusShutdown(token);
    }

    void closeGraphics()
    {
        safe_delete(graphics);
        if(pixelbuf){ DeleteObject(pixelbuf); pixelbuf = NULL; }
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

int ezInit(const ezstring& title, int width, int height, int style)
{
    setlocale(LC_ALL, "");//c中文
    std::locale::global(std::locale(""));//c++中文

    //获得屏幕大小
    int cx = GetSystemMetrics( SM_CXFULLSCREEN );
    int cy = GetSystemMetrics( SM_CYFULLSCREEN );
    if(width < cx){
        cx = (cx - width) / 2;
    }
    else{
        width = cx;
        cx = 0;
    }
    if(height < cy){
        cy = (cy - height) / 2;
    }
    else{
        height = cy;
        cy = 0;
    }

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
        cx = cy = 0;
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
        dwStyle = WS_POPUP;
        dwExStyle = 0;
        break;
    }

    //创建一个窗口
    __ezgdi_instance.create(EZAPP_CLASS_NAME, title.c_str(),
        cx, cy, width, height,
        dwStyle, dwExStyle);

    __ezgdi_instance.show();

    return 0;
}

void ezClose()
{
    SendMessage(__ezgdi_instance.handle(), WM_CLOSE, 0, 0);
}

HWND ezHWnd()
{
    return __ezgdi_instance.handle();
}

void ezResize(int width, int height)
{
    SetWindowPos(ezHWnd(), 0, 0, 0, width, height, SWP_NOMOVE|SWP_NOZORDER);
}

void ezClientResize(int width, int height)
{
    HWND hwnd = ezHWnd();
    RECT rcWindow;
    RECT rcClient;
    int borderWidth, borderHeight;

    GetWindowRect(hwnd, &rcWindow);
    GetClientRect(hwnd, &rcClient);

    borderWidth = (rcWindow.right-rcWindow.left) - (rcClient.right-rcClient.left);
    borderHeight = (rcWindow.bottom-rcWindow.top) - (rcClient.bottom-rcClient.top);

    SetWindowPos(hwnd, 0, 0, 0, borderWidth + width, borderHeight + height, SWP_NOMOVE|SWP_NOZORDER);
}

//获得GDI绘图设备
HDC ezHDC()
{
    return __ezgdi_instance.hdc;
}

//消息循环处理
bool ezLoop()
{
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    __ezgdi_instance.repaint(); //刷新窗口
    return __ezgdi_instance.running;
}

//程序执行
void ezRun()
{
    while(ezLoop());
}

//显示fps
void ezFps()
{
    static DWORD t = GetTickCount();
    static int fps_total = 0;
    static int fps = 0;

    ++fps;

    ezstring str = L"FPS:";
    str += ezstring(fps_total);

    //ezgdi_font(L"微软雅黑", 16);
    ezFontColor(255, 255, 255);
    ezText(0, 0, str);

    if(GetTickCount() - t > 1000){
        t = GetTickCount();
        fps_total = fps;
        fps = 0;
    }
}

std::basic_string<TCHAR> ezTempPath()
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

//判断按键是否按下
bool ezKeyState(int key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

//键盘事件映射
void ezOnKeyUp(EZ_KEY_EVENT function)
{
    __ezgdi_instance.OnKeyDown = function;
}

void ezOnKeyDown(EZ_KEY_EVENT function)
{
    __ezgdi_instance.OnKeyDown= function;
}

void ezOnChar(EZ_KEY_EVENT function)
{
    __ezgdi_instance.OnKeyPress = function;
}

//鼠标事件映射
void ezOnMouseDown(EZ_MOUSE_EVENT function)
{
    __ezgdi_instance.OnMouseDown = function;
}

void ezOnMouseUp(EZ_MOUSE_EVENT function)
{
    __ezgdi_instance.OnMouseUp = function;
}

void ezOnMouseMove(EZ_MOUSE_EVENT function)
{
    __ezgdi_instance.OnMouseMove = function;
}

//设置计时器
void ezTimer(UINT interval)
{
    if(interval){
        SetTimer(ezHWnd(), UINT(EZAPP_TIMER_ID), interval, NULL);
    }
    else{
        KillTimer(ezHWnd(), UINT(EZAPP_TIMER_ID));
    }
}

//计时器事件
void ezOnTimer(EZ_TIMER_EVENT function)
{
    __ezgdi_instance.OnTimer = function;
}

//窗口绘制事件
void ezOnPaint(EZ_PAINT_EVENT function)
{
    __ezgdi_instance.OnPaint = function;
}

//---------------------------------------------------------------------------
//
// 绘图函数
//
//---------------------------------------------------------------------------

//获得GDI+绘图设备
Gdiplus::Graphics* ezGraphics()
{
    return __ezgdi_instance.graphics;
}

//设置显示质量
int ezEffectLevel(int level)
{
    if(!__ezgdi_instance.graphics){
        return -1;
    }

    switch(level){
    case EZ_SPEED:
        __ezgdi_instance.graphics->SetCompositingMode( Gdiplus::CompositingModeSourceOver );           //混合模式
        __ezgdi_instance.graphics->SetCompositingQuality( Gdiplus::CompositingQualityHighSpeed );      //混合质量
        __ezgdi_instance.graphics->SetSmoothingMode( Gdiplus::SmoothingModeHighSpeed );                //反锯齿
        __ezgdi_instance.graphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );                 //像素偏移模式
        __ezgdi_instance.graphics->SetInterpolationMode( Gdiplus::InterpolationModeNearestNeighbor );  //图形缩放质量
        break;
    case EZ_MEDIUM:
        __ezgdi_instance.graphics->SetCompositingMode( Gdiplus::CompositingModeSourceOver );           //混合模式
        __ezgdi_instance.graphics->SetCompositingQuality( Gdiplus::CompositingQualityHighSpeed );      //混合质量
        __ezgdi_instance.graphics->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );                //反锯齿
        __ezgdi_instance.graphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );                 //像素偏移模式
        __ezgdi_instance.graphics->SetInterpolationMode( Gdiplus::InterpolationModeBilinear );         //图形缩放质量
        break;
    case EZ_QUALITY:
        __ezgdi_instance.graphics->SetCompositingMode( Gdiplus::CompositingModeSourceOver );           //混合模式
        __ezgdi_instance.graphics->SetCompositingQuality( Gdiplus::CompositingQualityHighQuality );    //混合质量
        __ezgdi_instance.graphics->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );                //反锯齿
        __ezgdi_instance.graphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeHighQuality );          //像素偏移模式
        __ezgdi_instance.graphics->SetInterpolationMode( Gdiplus::InterpolationModeBicubic );          //图形缩放质量
        break;
    default:
        break;
    }
    return 0;
}

//清屏
void ezClear(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->Clear(Gdiplus::Color(a, r, g, b));
}

//更改画笔颜色
void ezPenColor(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetColor(Gdiplus::Color(a, r, g, b));
}

void ezPenColor(DWORD argb)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetColor(Gdiplus::Color(argb));
}

void ezPenColor(vec4ub color)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetColor(Gdiplus::Color(color.a, color.r, color.g, color.b));
}

//获取画笔颜色
DWORD ezPenColor()
{
    Gdiplus::Color color;
    if(__ezgdi_instance.pen){
        __ezgdi_instance.pen->GetColor(&color);
    }
    return color.GetValue();
}
/*
vec4ub ezPenColor()
{
    Gdiplus::Color color;
    if(__ezgdi_instance.pen){
        __ezgdi_instance.pen->GetColor(&color);
    }
    return vec4ub(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}
*/

//画笔宽度
void ezPenWidth(float width)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetWidth(width);
}

//设置画笔模式
void ezPenStyle(int mode)
{
    if(__ezgdi_instance.pen){
        //if(mode != EZ_CUSTOM){//取消自定义点画模式
        //    ezDashStyle(NULL, 0);
        //}
        __ezgdi_instance.pen->SetDashStyle(Gdiplus::DashStyle(mode));
    }
}

//设置点画模式间隔
void ezDashStyle(const float* dash, int size)
{
    if(__ezgdi_instance.pen){
        //if(__ezgdi_instance.pen->GetDashStyle() == Gdiplus::DashStyleCustom){
            __ezgdi_instance.pen->SetDashPattern(dash, size);
        //}
    }
}

//更改填充颜色
void ezFillColor(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.brush)__ezgdi_instance.brush->SetColor(Gdiplus::Color(a, r, g, b));
}

void ezFillColor(DWORD argb)
{
    if(__ezgdi_instance.brush)__ezgdi_instance.brush->SetColor(Gdiplus::Color(argb));
}

//获取填充颜色
DWORD ezFillColor()
{
    Gdiplus::Color color;
    if(__ezgdi_instance.brush){
        __ezgdi_instance.brush->GetColor(&color);
    }
    return color.GetValue();
}

//绘制一个点
void ezPoint(float x, float y, float size)
{
    if(__ezgdi_instance.graphics){
        Gdiplus::SolidBrush brush(ezPenColor());
        float half = size * 0.5f;
        __ezgdi_instance.graphics->FillEllipse(&brush, x - half, y - half, size, size);
    }
}

//绘制线段
void ezDrawLine(float x1, float y1, float x2, float y2)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawLine(__ezgdi_instance.pen, x1, y1, x2, y2);
}

//绘制一个空心矩形
void ezDrawRect(float x, float y, float width, float height)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawRectangle(__ezgdi_instance.pen, x, y, width, height);
}

//填充一个矩形
void ezFillRect(float x, float y, float width, float height)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->FillRectangle(__ezgdi_instance.brush, x, y, width, height);
}

//绘制圆角矩形
void ezDrawRoundRect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if(cx > width)cx = width;
    if(cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = ezGraphics();
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
void ezFillRoundRect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if(cx > width)cx = width;
    if(cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = ezGraphics();
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
void ezDrawEllipse(float x, float y, float cx, float cy)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawEllipse(__ezgdi_instance.pen, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//填充椭圆
void ezFillEllipse(float x, float y, float cx, float cy)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->FillEllipse(__ezgdi_instance.brush, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//绘制空心圆，xy为圆心
void ezDrawCircle(float x, float y, float r)
{
    return ezDrawEllipse(x, y, r, r);
}

//填充圆
void ezFillCircle(float x, float y, float r)
{
    return ezFillEllipse(x, y, r, r);
}

//绘制连续的线段
void ezDrawPolyline(const vec2f* points, size_t size)
{
    Gdiplus::Graphics *g = ezGraphics();
    if(g){
        g->DrawLines(__ezgdi_instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//绘制多边形
void ezDrawPolygon(const vec2f* points, size_t size)
{
    Gdiplus::Graphics *g = ezGraphics();
    if(g){
        g->DrawPolygon(__ezgdi_instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//填充多边形
void ezFillPolygon(const vec2f* points, size_t size)
{
    Gdiplus::Graphics *g = ezGraphics();
    if(g){
        g->FillPolygon(__ezgdi_instance.brush, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//---------------------------------------------------------------------------
//
// 字体函数
//
//---------------------------------------------------------------------------

//设置字体。字体名字、大小、风格
void ezFont(const ezstring& name, float size, EZGDI_FONTSTYLE style)
{
    if(__ezgdi_instance.font){
        delete __ezgdi_instance.font;
        __ezgdi_instance.font = new Gdiplus::Font(name.c_str(), size, style, Gdiplus::UnitPoint, NULL);
    }
}

void ezFontName(const ezstring& name)
{
    __ezgdi_instance.fontName = name;
    __ezgdi_instance.fontIsChange = true;
}

void ezFontSize(float size)
{
    __ezgdi_instance.fontSize = size;
    __ezgdi_instance.fontIsChange = true;
}

void ezFontStyle(int style)
{
    __ezgdi_instance.fontStyle = style;
    __ezgdi_instance.fontIsChange = true;
}

//获取字体属性
//ezstring font_name();
//int font_size();
//int font_style();

//字体颜色
void ezFontColor(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.font_color)__ezgdi_instance.font_color->SetColor(Gdiplus::Color(a, r, g, b));
}

void ezFontColor(UINT color)
{
    if(__ezgdi_instance.font_color)__ezgdi_instance.font_color->SetColor(Gdiplus::Color(color));
}

//输出字体
void ezText(float x, float y, const wchar_t* text, size_t length)
{
    if(__ezgdi_instance.graphics){
        if(__ezgdi_instance.fontIsChange){
            ezFont(__ezgdi_instance.fontName, __ezgdi_instance.fontSize, EZGDI_FONTSTYLE(__ezgdi_instance.fontStyle));
            __ezgdi_instance.fontIsChange = false;
        }

        Gdiplus::StringFormat format;
        __ezgdi_instance.graphics->DrawString(text, INT(length), __ezgdi_instance.font,
            Gdiplus::PointF(x, y), &format, __ezgdi_instance.font_color);
    }
}

void ezText(float x, float y, const ezstring& text)
{
    return ezText(x, y, text.c_str(), text.length());
}

void ezText(float x, float y, float width, float height, const ezstring& text, int align)
{
    if(__ezgdi_instance.graphics){
        if(__ezgdi_instance.fontIsChange){
            ezFont(__ezgdi_instance.fontName, __ezgdi_instance.fontSize, EZGDI_FONTSTYLE(__ezgdi_instance.fontStyle));
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
        __ezgdi_instance.graphics->DrawString(text.c_str(), INT(text.length()),
            __ezgdi_instance.font,
            rect,
            &format,
            __ezgdi_instance.font_color);

    }
}

//字体格式化输出，和printf使用类似
void ezPrint(float x, float y, const wchar_t* format, ...)
{
	va_list body;
	wchar_t buf[1024] = { 0 };
	va_start(body, format);
    #ifdef _MSC_VER
	_vsntprintf_s(buf, 1024, format, body);
    #else
    vsnwprintf(buf, 1024, format, body);
    #endif
    ezText(x, y, buf, wcslen(buf));
    va_end(body);
}

//获得字符串的像素宽度
float ezTextWidth(const ezstring& text)
{
    if(__ezgdi_instance.graphics){
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        __ezgdi_instance.graphics->MeasureString(text.c_str(), int(text.length()), __ezgdi_instance.font, layoutSize, NULL, &size);
        return size.Width;
    }
    return 0;
}

//获得字符串的像素高度
float ezTextHeight(const ezstring& text)
{
    if(__ezgdi_instance.graphics){
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        __ezgdi_instance.graphics->MeasureString(text.c_str(), int(text.length()), __ezgdi_instance.font, layoutSize, NULL, &size);
        return size.Height;
    }
    return 0;
}

//---------------------------------------------------------------------------
//
// 图片操作
//
//---------------------------------------------------------------------------

ezImage::ezImage() : m_handle()
{
}

ezImage::~ezImage()
{
    this->close();
}

Gdiplus::Bitmap* ezImage::handle()const
{
    return m_handle;
}

//创建一个图片，默认为32位色
int ezImage::create(int width, int height, int format)
{
    this->close();
    m_handle = new Gdiplus::Bitmap(width, height, format);
    return 0;
}

//打开一个图片，支持bmp、jpg、png、静态gif等常见格式
int ezImage::open(const ezstring& filename)
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
Gdiplus::Image* LoadResourceImage(UINT id, PCTSTR type)
{
	Gdiplus::Image* image = NULL;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	if (type == RT_BITMAP) {
		return Gdiplus::Bitmap::FromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(id));
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
				image = Gdiplus::Image::FromStream(pstm);
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
int ezImage::open(int id, PCTSTR resource_type)
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
int ezImage::map(HBITMAP hbmp)
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

int GetImageCLSID(const WCHAR* format, CLSID* pCLSID)
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

int ezImage::save(const ezstring& filename)
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
void ezImage::close()
{
    if(m_handle){
        delete m_handle;
        m_handle = NULL;
    }
}

//返回图片的宽度
int ezImage::width()const
{
    return m_handle ? m_handle->GetWidth() : 0;
}

//返回图片的高度
int ezImage::height()const
{
    return m_handle ? m_handle->GetHeight() : 0;
}



ezImage* ezLoadImage(const ezstring& filename)
{
    return __ezgdi_instance.resource.loadimage(filename);
}

ezImage* ezLoadImage(int id, PCTSTR resource_type)
{
    return __ezgdi_instance.resource.loadimage(id, resource_type);
}

int ezSaveImage(ezImage* image, const ezstring& filename)
{
    if(image){
        image->save(filename);
    }
    return -1;
}

void ezDraw(ezImage* image, float x, float y)
{
    if(__ezgdi_instance.graphics && image && image->handle())__ezgdi_instance.graphics->DrawImage(image->handle(), x, y);
}

void ezDraw(ezImage* image, float x, float y, float width, float height)
{
    if(__ezgdi_instance.graphics && image && image->handle())__ezgdi_instance.graphics->DrawImage(image->handle(), x, y, width, height);
}

//在xy位置绘制图片，并旋转一个角度
void ezRotateImage(ezImage* image, float x, float y, float rotate)
{
    return ezRotateImage(image, x, y, float(image->width()), float(image->height()), rotate);
}

//在xy位置绘制图片，缩放，并旋转一个角度
void ezRotateImage(ezImage* image, float x, float y, float width, float height, float rotate)
{
    if(__ezgdi_instance.graphics && image){
        Gdiplus::Graphics* g = __ezgdi_instance.graphics;
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

//---------------------------------------------------------------------------
//
// 多媒体
//
//---------------------------------------------------------------------------

//播放音乐
void ezPlayMusic(PCTSTR filename)
{
	ezstring command = L"open ";
	command.append((const wchar_t*)filename);
	command += L" alias background";

    mciSendString((PCTSTR)command.c_str(), NULL, 0, NULL);
    mciSendString(TEXT("play background repeat"), NULL, 0, NULL);
}

void ezStopMusic()
{
    mciSendString(TEXT("stop background"), NULL, 0, NULL);//!
	mciSendString(TEXT("close background"), NULL, 0, NULL);
}

//导出资源到文件
//参考文章：https://www.cnblogs.com/zjutlitao/p/3577592.html
bool ExtractResource(LPCTSTR filename, LPCTSTR resource_type, LPCTSTR resource_name)
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
void ezPlayResourceMusic(PCTSTR filename, PCTSTR resource_type)
{
	std::basic_string<TCHAR> tempfile = ezTempPath();
    tempfile += TEXT("background.mp3");
    // 将mp3资源提取为临时文件
    ExtractResource(tempfile.c_str(), resource_type, filename);
	ezPlayResourceMusic(tempfile.c_str());
}

void ezPlayResourceMusic(int id, PCTSTR resource_type)
{
	return ezPlayResourceMusic(MAKEINTRESOURCE(id), resource_type);
}

//播放wav文件
int ezPlaySound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_FILENAME|SND_ASYNC;
    if(loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, 0, fdwSound);
}

//播放资源中的wav文件
int ezPlayResourceSound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_RESOURCE|SND_ASYNC;
    if(loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, GetModuleHandle(NULL), fdwSound);
}

int ezPlayResourceSound(int id, bool loop)
{
    return ezPlayResourceSound(MAKEINTRESOURCE(id), loop);
}

//停止声音播放
void ezStopSound()
{
	PlaySound(NULL, NULL, SND_FILENAME);
}

//---------------------------------------------------------------------------
//
// 对话框
//
//---------------------------------------------------------------------------

//消息对话框
int ezMsgBox(const ezstring& msg, const ezstring& title, int type)
{
	return MessageBoxW(ezHWnd(), msg.c_str(), title.c_str(), type);
}

//显示消息
void ezMessage(const ezstring& msg)
{
    MessageBoxW(ezHWnd(), msg.c_str(), L"消息", MB_OK);
}

//显示输入框
ezstring ezInputBox(const ezstring& title, const ezstring& message, const ezstring value)
{
    ezapi::ezInputBox box;
    if(box.execute(ezHWnd(), title, message, value)){
        return box.text();
    }
    else{
        return ezstring();
    }
}


#endif //EZGDI_INL_20200708233153
