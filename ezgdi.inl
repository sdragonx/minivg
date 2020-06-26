#pragma once

#include "ezgdi.hpp"
#include <mmsystem.h>

#if defined(__BORLANDC__) || defined(_MSC_VER)
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib,"Winmm.lib")
#endif

#define EZGDI_CLASS_NAME   PCWSTR(L"EZGDI_WINDOW")
#define EZGDI_DEFAULT_FONT PCWSTR(L"微软雅黑")

//返回ezgdi的实例
EZGDI_PUBLIC ezgdi::ezInstance<>& __ezgdi_instance = ezgdi::ezInstance<>::instance;

//窗口消息循环
LRESULT CALLBACK WindowProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

//窗口重绘事件
int OnWindowPaint(HWND hwnd)
{
    HDC hDC = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(0);
    SelectObject(memDC, __ezgdi_instance.pixelbuf);
    BitBlt(hDC, 0, 0, 1920, 1080, memDC, 0, 0, SRCCOPY);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hDC);
    return 0;
}

//注册窗口类
int InitClass(PCWSTR className, int style, WNDPROC wndproc)
{
    WNDCLASSEXW wc = {0};
    int atom = 0;
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

//创建窗口函数
HWND InitWindow(
    LPCWSTR title,      //标题
    int     x,          //位置x
    int     y,          //位置y
    int     width,      //宽度
    int     height,     //高度
    DWORD   style,      //窗口风格
    DWORD   styleEx     //扩展风格
    )
{
    HWND hwnd;
    HINSTANCE hInstance = GetModuleHandle(NULL);//进程实例句柄

    InitClass(EZGDI_CLASS_NAME, 0, WindowProc);

    hwnd = CreateWindowExW(
        0,                  //窗口的扩展风格
        EZGDI_CLASS_NAME,   //类名
        title,              //标题
        style,              //风格
        x,                  //左边位置
        y,                  //顶部位置
        width,              //宽度
        height,             //高度
        NULL,               //父窗口的句柄
        NULL,               //菜单的句柄或是子窗口的标识符
        hInstance,          //应用程序实例的句柄
        NULL);              //wParam

    if(hwnd == NULL) {
        MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }

    return hwnd;
}

//windows主消息处理函数
LRESULT CALLBACK WindowProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message){
    case WM_CREATE:
        __ezgdi_instance.gdiplusInit();
        break;
    case WM_DESTROY:
        __ezgdi_instance.closeGraphics();
        __ezgdi_instance.gdiplusShutdown();
        PostQuitMessage(0);
        break;
    case WM_WINDOWPOSCHANGING:
        break;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_SHOWWINDOW:
        __ezgdi_instance.repaint();
        break;
    case WM_SIZE:
        __ezgdi_instance.initGraphics(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_PAINT:
        OnWindowPaint(hWnd);
        break;

    case WM_KEYDOWN:
        if(__ezgdi_instance.OnKeyDown)__ezgdi_instance.OnKeyDown(wParam);
        break;
    case WM_KEYUP:
        if(__ezgdi_instance.OnKeyUp)__ezgdi_instance.OnKeyUp(wParam);
        break;
    case WM_CHAR:
        if(__ezgdi_instance.OnKeyPress)__ezgdi_instance.OnKeyPress(wParam);
        break;

    case WM_MOUSEMOVE:
        if(__ezgdi_instance.OnMouseMove)__ezgdi_instance.OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
        break;
    case WM_LBUTTONDOWN:
        if(__ezgdi_instance.OnMouseDown)__ezgdi_instance.OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ezLeft);
        break;
    case WM_LBUTTONUP:
        if(__ezgdi_instance.OnMouseUp)__ezgdi_instance.OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ezLeft);
        break;
    case WM_RBUTTONDOWN:
        if(__ezgdi_instance.OnMouseDown)__ezgdi_instance.OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ezRight);
        break;
    case WM_RBUTTONUP:
        if(__ezgdi_instance.OnMouseUp)__ezgdi_instance.OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ezRight);
        break;
    case WM_MBUTTONDOWN:
        if(__ezgdi_instance.OnMouseDown)__ezgdi_instance.OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ezMiddle);
        break;
    case WM_MBUTTONUP:
        if(__ezgdi_instance.OnMouseUp)__ezgdi_instance.OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ezMiddle);
        break;

    default:
        break;
    }
    return DefWindowProc(hWnd, Message, wParam, lParam);
}

namespace ezgdi{

template<typename T>
ezInstance<T> ezInstance<T>::instance = ezInstance<T>();

template<typename T>
ezInstance<T>::ezInstance() :
    window(), hdc(),
    OnKeyDown(), OnKeyUp(), OnKeyPress(), OnMouseDown(), OnMouseUp(), OnMouseMove(),
    graphics(), colorbuf(), pixelbuf(), pen(), brush(),
    font(),
    fontName(EZGDI_DEFAULT_FONT),
    fontSize(12),
    fontStyle(ezNormal)
{

}

template<typename T>
ezInstance<T>::~ezInstance()
{
    closeGraphics();
}

HBITMAP ezgdi_bitmap(int width, int height)
{
    HBITMAP hBitmap;
    BITMAPV5HEADER bi;
    void *lpBits = 0;

    ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = width;
    bi.bV5Height = height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask   = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask  = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    hBitmap = CreateDIBSection(GetDC(NULL), (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, (DWORD)0);

    return hBitmap;
}

template<typename T>
int ezInstance<T>::initGraphics(int width, int height)
{
    closeGraphics();

    if(!width || !height){
        return -1;
    }

    pixelbuf = ezgdi_bitmap(width, height);
    BITMAP bm;
    GetObject(pixelbuf, sizeof(bm), &bm);
    BYTE* pixels = ((BYTE*)bm.bmBits) + (bm.bmHeight - 1) * bm.bmWidthBytes;
    colorbuf = new Gdiplus::Bitmap(bm.bmWidth, bm.bmHeight, -bm.bmWidthBytes, PixelFormat32bppARGB, pixels);
    graphics = new Gdiplus::Graphics(colorbuf);
    pen = new Gdiplus::Pen(Gdiplus::Color::Black);
    brush = new Gdiplus::SolidBrush(Gdiplus::Color::White);
    font = new Gdiplus::Font(EZGDI_DEFAULT_FONT, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint, NULL);
    font_color = new Gdiplus::SolidBrush(Gdiplus::Color::Black);

    return true;
}

template<typename T>
void ezInstance<T>::closeGraphics()
{
    safe_delete(graphics);
    safe_delete(colorbuf);
    if(pixelbuf){ DeleteObject(pixelbuf); pixelbuf = NULL; }
    safe_delete(pen);
    safe_delete(brush);
    safe_delete(font);
    safe_delete(font_color);
}

template<typename T>
void ezInstance<T>::repaint()
{
    RECT rc;
    GetClientRect(window, &rc);
    RedrawWindow(window, &rc, 0, RDW_UPDATENOW|RDW_INVALIDATE|RDW_NOERASE);
}

template<typename T>
void ezInstance<T>::gdiplusInit()
{
    Gdiplus::GdiplusStartup(&token, &input, NULL);
}

template<typename T>
void ezInstance<T>::gdiplusShutdown()
{
    Gdiplus::GdiplusShutdown(token);
}

}//end namespace ezgdi


int ezgdi_init(const ezstring& title, int width, int height, int style)
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

    //创建一个窗口
    HWND hwnd = InitWindow(
        title.c_str(),
        cx,
        cy,
        width,
        height,
        WS_POPUPWINDOW|WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX|style,
        WS_EX_CLIENTEDGE
        //|WS_EX_TOPMOST    //置顶
        );

    //显示窗口
    ShowWindow(hwnd, SW_SHOW);
    __ezgdi_instance.window = hwnd;

    return 0;
}

void ezgdi_close()
{
    SendMessage(__ezgdi_instance.window, WM_CLOSE, 0, 0);
}

HWND ezgdi_window()
{
    return __ezgdi_instance.window;
}

void ezgdi_execute()
{
    MSG Msg;
    while(GetMessage(&Msg, NULL, 0, 0) > 0){
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
        Sleep(1);
    }
}

//消息循环处理
bool ezgdi_loop()
{
    MSG msg;
    bool run = true;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
        if(msg.message == WM_QUIT){
            run = false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    __ezgdi_instance.repaint(); //刷新窗口
    return run;
}

//显示fps
void show_fps()
{
    static DWORD t = GetTickCount();
    static int fps_total = 0;
    static int fps = 0;

    ++fps;

    ezstring str = L"FPS:";
    str += ezstring(fps_total);

    //ezgdi_font(L"微软雅黑", 16);
    text_color(255, 255, 255);
    text_out(0, 0, str);

    if(GetTickCount() - t > 1000){
        t = GetTickCount();
        fps_total = fps;
        fps = 0;
    }
}

//---------------------------------------------------------------------------
//
// 绘图函数
//
//---------------------------------------------------------------------------

//获得GDI+绘图设备
Gdiplus::Graphics* ezgdi_canvas()
{
    return __ezgdi_instance.graphics;
}

//设置显示质量
int effect_level(EFFECT_LEVEL level)
{
    if(__ezgdi_instance.graphics){
        switch(level){
        case EZGDI_SPEED:
            __ezgdi_instance.graphics->SetSmoothingMode( Gdiplus::SmoothingModeHighSpeed );
            __ezgdi_instance.graphics->SetInterpolationMode( Gdiplus::InterpolationModeNearestNeighbor );
            __ezgdi_instance.graphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeHalf );
            break;
        case EZGDI_QUALITY:
            __ezgdi_instance.graphics->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
            __ezgdi_instance.graphics->SetInterpolationMode( Gdiplus::InterpolationModeBilinear );
            __ezgdi_instance.graphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeHighQuality );
            break;
        default:
            break;
        }
    }
    return 0;
}

//清屏
void clear(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->Clear(Gdiplus::Color(a, r, g, b));
}

//更改画笔颜色
void pen_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetColor(Gdiplus::Color(a, r, g, b));
}

//更改填充颜色
void fill_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.brush)__ezgdi_instance.brush->SetColor(Gdiplus::Color(a, r, g, b));
}

//画笔宽度
void pen_width(float width)
{
    if(__ezgdi_instance.pen)__ezgdi_instance.pen->SetWidth(width);
}

//绘制线段
void draw_line(float x1, float y1, float x2, float y2)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawLine(__ezgdi_instance.pen, x1, y1, x2, y2);
}

//绘制一个空心矩形
void draw_rect(float x, float y, float width, float height)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawRectangle(__ezgdi_instance.pen, x, y, width, height);
}

//填充一个矩形
void fill_rect(float x, float y, float width, float height)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->FillRectangle(__ezgdi_instance.brush, x, y, width, height);
}

//绘制椭圆，xy为圆心
void draw_ellipse(float x, float y, float cx, float cy)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawEllipse(__ezgdi_instance.pen, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//填充椭圆
void fill_ellipse(float x, float y, float cx, float cy)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->FillEllipse(__ezgdi_instance.brush, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//绘制空心圆，xy为圆心
void draw_circle(float x, float y, float r)
{
    return draw_ellipse(x, y, r, r);
}

//填充圆
void fill_circle(float x, float y, float r)
{
    return fill_ellipse(x, y, r, r);
}

//---------------------------------------------------------------------------
//
// 字体函数
//
//---------------------------------------------------------------------------

//设置字体。字体名字、大小、风格
void ezgdi_font(const ezstring& name, float size, EZGDI_FONTSTYLE style)
{
    if(__ezgdi_instance.font){
        delete __ezgdi_instance.font;
        __ezgdi_instance.font = new Gdiplus::Font(name.c_str(), size, style, Gdiplus::UnitPoint, NULL);
    }
}

void font_name(const ezstring& name)
{
    __ezgdi_instance.fontName = name;
    __ezgdi_instance.fontIsChange = true;
}

void font_size(float size)
{
    __ezgdi_instance.fontSize = size;
    __ezgdi_instance.fontIsChange = true;
}

void font_style(int style)
{
    __ezgdi_instance.fontStyle = style;
    __ezgdi_instance.fontIsChange = true;
}

//字体颜色
void text_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(__ezgdi_instance.font_color)__ezgdi_instance.font_color->SetColor(Gdiplus::Color(a, r, g, b));
}

//输出字体
void text_out(float x, float y, const ezstring& text)
{
    if(__ezgdi_instance.graphics){
        if(__ezgdi_instance.fontIsChange){
            ezgdi_font(__ezgdi_instance.fontName, __ezgdi_instance.fontSize, EZGDI_FONTSTYLE(__ezgdi_instance.fontStyle));
            __ezgdi_instance.fontIsChange = false;
        }

        Gdiplus::StringFormat fmt;
        __ezgdi_instance.graphics->DrawString(text.c_str(), text.length(), __ezgdi_instance.font,
            Gdiplus::PointF(x, y), &fmt, __ezgdi_instance.font_color);
    }
}

//---------------------------------------------------------------------------
//
// 图片操作
//
//---------------------------------------------------------------------------

ezImage* loadimage(const ezstring& filename)
{
    return __ezgdi_instance.resource.loadimage(filename);
}

void draw_image(ezImage* image, float x, float y)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawImage(image->handle(), x, y);
}

void draw_image(ezImage* image, float x, float y, float width, float height)
{
    if(__ezgdi_instance.graphics)__ezgdi_instance.graphics->DrawImage(image->handle(), x, y, width, height);
}

//在xy位置绘制图片，缩放，并旋转一个角度
void point_image(ezImage* image, float x, float y, float width, float height, float rotate)
{
    if(__ezgdi_instance.graphics){
        Gdiplus::Graphics* g = __ezgdi_instance.graphics;
//        float cx = image->width() / 2;
//        float cy = image->height() / 2;
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

void point_image(ezImage* image, int x, int y, int width, int height, float rotate)
{
    if(__ezgdi_instance.graphics){
        Gdiplus::Graphics* g = __ezgdi_instance.graphics;
//        float cx = image->width() / 2;
//        float cy = image->height() / 2;
        int cx = width / 2;
        int cy = height / 2;
        Gdiplus::Matrix m;
        g->GetTransform(&m);
        g->TranslateTransform(x, y);//移动到当前位置
        g->RotateTransform(rotate); //旋转
        g->TranslateTransform(-cx, -cy);//移动到旋转中心
        g->DrawImage(image->handle(), 0, 0, width, height);//绘制图像
        g->SetTransform(&m);
    }
}

//---------------------------------------------------------------------------
//
// 输入控制
//
//---------------------------------------------------------------------------

//判断按键是否按下
bool key_state(int key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

//键盘事件映射
void ezgdi_keyup(EZGDI_KEYEVENT function)
{
    __ezgdi_instance.OnKeyDown = function;
}

void ezgdi_keydown(EZGDI_KEYEVENT function)
{
    __ezgdi_instance.OnKeyDown= function;
}

void ezgdi_keypress(EZGDI_KEYEVENT function)
{
    __ezgdi_instance.OnKeyPress = function;
}

//鼠标事件映射
void ezgdi_mousedown(EZGDI_MOUSEEVENT function)
{
    __ezgdi_instance.OnMouseDown = function;
}

void ezgdi_mouseup(EZGDI_MOUSEEVENT function)
{
    __ezgdi_instance.OnMouseUp = function;
}

void ezgdi_mousemove(EZGDI_MOUSEEVENT function)
{
    __ezgdi_instance.OnMouseMove = function;
}

//---------------------------------------------------------------------------
//
// 多媒体
//
//---------------------------------------------------------------------------

//播放音乐
int playmusic(PCTSTR filename)
{
    if(!filename){
        mciSendString(TEXT("stop background"), NULL, 0, NULL);
    }
    else{
        ezstring command = L"open ";
		command.append((const wchar_t*)filename);
		command += L" alias background";

        mciSendString((PCTSTR)command.c_str(), NULL, 0, NULL);
        mciSendString(TEXT("play background repeat"), NULL, 0, NULL);
    }
    return 0;
}

//播放wav文件
int playsound(PCTSTR filename)
{
    return PlaySound(filename, 0, SND_FILENAME|SND_ASYNC);
}

//---------------------------------------------------------------------------
//
// 界面
//
//---------------------------------------------------------------------------

LRESULT CALLBACK basic_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int ezWindow::create(PCWSTR className, PCWSTR title, int x, int y, int width, int height, int style)
{
    InitClass(className, 0, basic_wndproc);

    //创建窗口
    m_handle = CreateWindowExW(
        0,              //窗口的扩展风格
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
    ezWindow *win = reinterpret_cast<ezWindow*>(GetWindowLongPtr(hwnd, GWL_USERDATA));
    if(win){
        return win->wndproc(msg, wparam, lparam);
    }
    else if(msg == WM_CREATE){
        LPCREATESTRUCTW pcs = LPCREATESTRUCTW(lparam);
        win = reinterpret_cast<ezWindow*>(pcs->lpCreateParams);
        if(win){
            win->m_handle = hwnd;
            ::SetLastError(ERROR_SUCCESS);
            SetWindowLongPtr(hwnd, GWL_USERDATA, reinterpret_cast<LPARAM>(win));
            if(GetLastError()!=ERROR_SUCCESS)
                return -1;
            else
                return win->wndproc(msg, wparam, lparam);
        }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

int ezWindow::show_model(HWND parent)
{
    if (!m_handle){
        return 0;
    }
    ShowWindow(m_handle, SW_SHOW);
    UpdateWindow(m_handle);
    if(parent){
        SetWindowLongPtr(m_handle, GWL_HWNDPARENT, (LONG_PTR)parent);
        EnableWindow(parent, FALSE);
    }

    MSG msg;
    while(GetMessage(&msg, 0, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if(parent){
        EnableWindow(parent, TRUE);
        SetForegroundWindow(parent);
    }

    return msg.wParam;
}

//InputBox
class ezInputBox : private ezWindow
{
private:
    HFONT hFont;
    ezstring m_message;
    ezstring m_text;
    bool result;

public:
    bool execute(HWND parent, const ezstring& title, const ezstring& message, const ezstring& text = ezstring());
    ezstring text()const { return m_text; }

protected:
    void on_create();
    void ButtonOKClick();
    int wndproc(UINT msg, WPARAM wparam, LPARAM lparam);
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

    show_model(parent);

    return result;
}

void ezInputBox::on_create()
{
    hFont = CreateFontW(20, 0, 0, 0, 20, 0, 0, 0,
    GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, EZGDI_DEFAULT_FONT);
    HDC hDC = GetDC(m_handle);
    SelectObject(hDC, hFont);
    ReleaseDC(m_handle, hDC);

    RECT rect;
    GetClientRect(m_handle, &rect);
    HWND Label          = create_label(m_message.c_str(), 4, 5, rect.right - 80, 70, 1000);
    HWND buttonOK       = create_button(L"确定(&K)", rect.right - 70, 8, 65, 24, IDOK);
    HWND buttonCancel   = create_button(L"取消(&C)", rect.right - 70, 36, 65, 24, IDCANCEL);
    HWND Edit           = create_edit(m_text.c_str(), 4, rect.bottom - 32, rect.right - 8, 28, 2000);
    SendMessage(Edit, EM_SETLIMITTEXT, 256, 0);

    SendDlgItemMessage(m_handle, 1000, WM_SETFONT, (WPARAM)hFont, 0);
    SendDlgItemMessage(m_handle, 2000, WM_SETFONT, (WPARAM)hFont, 0);
    SendDlgItemMessage(m_handle, IDOK, WM_SETFONT, (WPARAM)hFont, 0);
    SendDlgItemMessage(m_handle, IDCANCEL, WM_SETFONT, (WPARAM)hFont, 0);
    SendDlgItemMessage(m_handle, IDHELP, WM_SETFONT, (WPARAM)hFont, 0);
}

void ezInputBox::ButtonOKClick()
{
    TCHAR buf[256];
    GetDlgItemText(m_handle, 2000, buf, 256);
    m_text = buf;
}

int ezInputBox::wndproc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch(msg){
    case WM_CREATE:
        on_create();
        break;
    case WM_DESTROY:
        if(hFont)DeleteObject(hFont);
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
            result = true;
            break;
        case IDCANCEL:
            DestroyWindow(m_handle);
            result = false;
            break;
        };
        break;
    default:
        break;
    }
    return DefWindowProc(m_handle, msg, wparam, lparam);;
}

//显示消息
void showmessage(const ezstring& msg)
{
    MessageBoxW(ezgdi_window(), msg.c_str(), L"消息", MB_OK);
}

//显示输入框
ezstring inputbox(const ezstring& title, const ezstring& message, const ezstring value)
{
    ezInputBox b;
    if(b.execute(ezgdi_window(), title, message, value)){
        return b.text();
    }
    else{
        return ezstring();
    }
}
