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

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <ole2.h>           // CreateStreamOnHGlobal

#if __cplusplus < 201103L
    #define nullptr NULL
#endif

#ifdef __GNUC
    #define __MT__
#endif

#include <process.h>

#pragma warning(disable: 4717)

#if defined(__BORLANDC__) || defined(_MSC_VER)
#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ole32.lib")
#endif

#define MINIVG_VERSION       PCWSTR(L"minivg 2021-07-30")

#define MINIVG_CLASS_NAME    PCWSTR(L"minivg_window")
#define MINIVG_DEFAULT_FONT  PCWSTR(L"msyh")                // ΢���ź�

#define MINIVG_TIMER_ID      1

#define MINIVG_INLINE inline

namespace minivg{
namespace internal{

//---------------------------------------------------------------------------
//
// ��Դ������
//
//---------------------------------------------------------------------------

// ��ȫɾ��ָ��
template<typename T>
void safe_delete(T* &p)
{
    if(p){
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

// ɾ��stl�������������
template<typename T>
void delete_all(T& obj)
{
    typename T::iterator itr = obj.begin();
    for( ; itr != obj.end(); ++itr){
        delete itr->second;
    }
    obj.clear();
}

// ����һ�� HBITMAP
MINIVG_INLINE HBITMAP bm_create(int width, int height, int pixelbits = 32)
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

// ��Դ������
class ezResource
{
private:
    std::map<unistring, ezImage*> images;       // ���ص�ͼƬ
    std::map<int, ezImage*> resource_images;    // ���ص���ԴͼƬ
    std::vector<ezImage*> image_pool;           // ������ͼƬ

public:
    // ����һ��ͼƬ
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

    // ������ԴͼƬ
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

    // ����ͼƬ
    ezImage* allocate(int width, int height)
    {
        ezImage *image = new ezImage();
        image->create(width, height);
        image_pool.push_back(image);
        return image;
    }

    // ɾ��ͼƬ
    void free(ezImage* image)
    {
        std::vector<ezImage*>::iterator itr = std::find(
            image_pool.begin(), image_pool.end(), image);
        if(itr != image_pool.end()){
            delete *itr;
            image_pool.erase(itr);
        }
    }

    // �ͷ�������Դ����������ڳ����˳���ʱ��ִ��
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
// ����
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
        static bool is_init = false;
        if (!is_init) {
            InitClass(className, 0, basic_wndproc);
            is_init = true;
        }

        #ifndef UNICODE
        std::string buf = to_ansi(title, -1);
        title = (PCWSTR)buf.c_str();
        #endif

        //��������
        m_handle = CreateWindowExW(
            styleEx,        //���ڵ���չ���
            className,      //����
            title,          //����
            style,          //���
            x,              //���λ��
            y,              //����λ��
            width,          //���
            height,         //�߶�
            NULL,           //�����ڵľ��
            NULL,           //�˵��ľ�������Ӵ��ڵı�ʶ��
            GetModuleHandle(NULL),    //Ӧ�ó���ʵ���ľ��
            this);          //ָ�򴰿ڵĴ�������

        if (m_handle == NULL) {
            MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
            return -1;
        }

        return 0;
    }

    // �رմ���
    void close()
    {
        SendMessage(m_handle, WM_CLOSE, 0, 0);
    }

    // ���ô��ڷ�Χ
    void setBounds(int x, int y, int width, int height)
    {
        MoveWindow(m_handle, x, y, width, height, TRUE);
    }

    // �ƶ�����
    void move(int x, int y)
    {
        SetWindowPos(m_handle, NULL, x, y, 0, 0, SWP_NOSIZE);
    }

    // ���ô��ڴ�С
    void resize(int width, int height)
    {
        SetWindowPos(m_handle, NULL, 0, 0, width, height, SWP_NOMOVE);
    }

    // ��ʾ����
    void show()
    {
        ShowWindow(m_handle, SW_SHOW);
    }

    // ���ش���
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

    //�������������
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

    //�����ؼ�
    HWND CreateComponent(ezWindow* parent, PCWSTR classname, int x, int y, int width, int height, int style, int styleEx = 0)
    {
        HWND hwnd = CreateWindowExW(styleEx, classname, NULL, style,
            x, y, width, height, parent->handle(), (HMENU)NULL, GetModuleHandle(NULL), NULL);

        HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hwnd, WM_SETFONT, (WPARAM)font, 0);
        return hwnd;
    }

    //ע�ᴰ����
    int InitClass(PCWSTR className, int style, WNDPROC wndproc)
    {
        int atom = 0;
        WNDCLASSEXW wc;
        HINSTANCE hInstance = GetModuleHandle(NULL);

        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;// CS_DBLCLKS ֧�����˫���¼�
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
        btnOK.setText(L"ȷ��(&O)");

        btnCancel.create(this, rect.right - 70, 36, 64, 24);
        btnCancel.setID(IDCANCEL);
        btnCancel.setText(L"ȡ��(&C)");

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
// minivg ʵ����
//
//---------------------------------------------------------------------------

template<typename T = int>
class vgContext : public ezWindow
{
public:
    HDC hdc;                        //GDI ��ͼ�豸
    Gdiplus::Graphics* g;           //GDIPlus �豸
    HBITMAP pixelbuf;               //���ػ�����

    int effectLevel;                //Ч���ȼ�

    Gdiplus::Pen* pen;              //����
    Gdiplus::SolidBrush* brush;     //��ˢ
    Gdiplus::Font* font;            //����
    Gdiplus::SolidBrush* font_color;
    unistring fontName;
    float    fontSize;
    int      fontStyle;
    bool     fontIsChange;

    WNDPROC prevWndProc;            //������������еĴ��ڣ��������д��ڵ���Ϣ������

    int initParam;                  //��������
    RECT winRect;                   //����ģʽ�´��ڴ�С
    bool fullscreen;                //�Ƿ�ȫ��

    vec4i vp;                       //�ӿ�
    bool running;                   //�����Ƿ�����

    EZ_KEY_EVENT OnKeyDown;         //�����¼�
    EZ_KEY_EVENT OnKeyUp;
    EZ_KEY_EVENT OnKeyPress;

    EZ_MOUSE_EVENT OnMouseDown;     //����¼�
    EZ_MOUSE_EVENT OnMouseUp;
    EZ_MOUSE_EVENT OnMouseMove;

    EZ_TIMER_EVENT OnTimer;         //��ʱ���¼�
    EZ_PAINT_EVENT OnPaint;         //���ڻ����¼�

    ezResource resource;            //��Դ������

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
        prevWndProc = NULL;
        gdiplusInit();
    }

    ~vgContext()
    {
        resource.dispose();
        gdiplusShutdown();
    }

    // ���õ����еĴ���
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

    // ���ñ߿���ʽ
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

    // �ؽ�����������
    void viewport(int x, int y, int width, int height)
    {
        closeGraphics();

        if(!width || !height) {
            return ;
        }

        if (width != vp.z || height != vp.w) {
            pixelbuf = bm_create(width, height);
            SelectObject(hdc, pixelbuf);
            g = new Gdiplus::Graphics(hdc);
            effect_level(effectLevel);
        }

        vp = vec4i(x, y, width, height);
    }

    // ����������ͼ����Ƶ�Ŀ�� HDC
    void bitblt(HDC dc)
    {
        BitBlt(dc, vp.x, vp.y, vp.z, vp.w, hdc, 0, 0, SRCCOPY);
    }

    // ���ô����ö�
    bool topmose(bool top)
    {
        if(top){
            return SetWindowPos(m_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
        }
        else{
            return SetWindowPos(m_handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
        }
    }

    // ��������Ϣ
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

    // �����ػ��¼�
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
vgContext<T> vgContext<T>::instance = vgContext<T>();

// ��Ļ�����߳�
MINIVG_INLINE void updateThread(void* arg)
{
    while (vgContext<>::instance.running) {
        // ˢ�´���
        vgContext<>::instance.repaint();
    }

}

}// end namespace internal

//---------------------------------------------------------------------------
//
// ������
//
//---------------------------------------------------------------------------

MINIVG_INLINE int initgraph(const unistring& title, int width, int height, int param)
{
    setlocale(LC_ALL, "");//c����
    std::locale::global(std::locale(""));//c++����

    // ���洴������
    internal::vgContext<>::instance.initParam = param;

    if(param & EZ_BACKBUFFER){
        internal::vgContext<>::instance.viewport(0, 0, width, height);
        internal::vgContext<>::instance.winRect.left = 0;
        internal::vgContext<>::instance.winRect.top = 0;
        internal::vgContext<>::instance.winRect.right = width;
        internal::vgContext<>::instance.winRect.bottom = height;
    }
    else{
        // �����Ļ��С
        int cx = GetSystemMetrics(SM_CXFULLSCREEN);
        int cy = GetSystemMetrics(SM_CYFULLSCREEN);

        // ������ʾ
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

        // ���ô�����ʽ
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

        // ���㴰�ڴ�С
        RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRectEx(&rect, dwStyle, FALSE, WS_EX_CLIENTEDGE);
        width = static_cast<int>(rect.right - rect.left);
        height = static_cast<int>(rect.bottom - rect.top);

        OffsetRect(&rect, cx, cy);
        internal::vgContext<>::instance.winRect = rect;

        // ����һ������
        internal::vgContext<>::instance.create(MINIVG_CLASS_NAME, title.c_str(),
            cx, cy, width, height,
            dwStyle, dwExStyle);

        internal::vgContext<>::instance.show();

        // ����ˢ���߳�
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE) internal::updateThread, nullptr, 0, 0);
    }

    return 0;
}

MINIVG_INLINE int initgraph(int width, int height, int param)
{
    return initgraph(MINIVG_VERSION, width, height, param);
}

MINIVG_INLINE int initgraph(HWND hwnd)
{
    setlocale(LC_ALL, "");//c����
    std::locale::global(std::locale(""));//c++����

    //����һ������
    internal::vgContext<>::instance.setWindow(hwnd);

    internal::vgContext<>::instance.show();

    return 0;
}

MINIVG_INLINE void quit()
{
    if(internal::vgContext<>::instance.prevWndProc){
        internal::vgContext<>::instance.setWindow(NULL);
    }
    else {
        internal::vgContext<>::instance.close();
    }
}

MINIVG_INLINE HWND graph_window()
{
    return internal::vgContext<>::instance.handle();
}

//���GDI��ͼ�豸
MINIVG_INLINE HDC graph_hdc()
{
    return internal::vgContext<>::instance.hdc;
}

MINIVG_INLINE void set_title(const unistring& text)
{
    internal::vgContext<>::instance.setTitle(text);
}

MINIVG_INLINE void reshape(int width, int height)
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

MINIVG_INLINE void fullscreen(bool value)
{
    if(value != internal::vgContext<>::instance.fullscreen){
        if(value) {
            GetWindowRect(internal::vgContext<>::instance.handle(), &internal::vgContext<>::instance.winRect);

            internal::vgContext<>::instance.setStyle(EZ_FULLSCREEN);
            internal::vgContext<>::instance.fullscreen = true;

            // �����Ļ��С
            int cx = GetSystemMetrics( SM_CXSCREEN );
            int cy = GetSystemMetrics( SM_CYSCREEN );
            SetWindowPos(internal::vgContext<>::instance.handle(), 0, 0, 0, cx, cy, SWP_NOZORDER);
        }
        else {
            internal::vgContext<>::instance.fullscreen = false;
            internal::vgContext<>::instance.setStyle(EZ_FIXED);

            internal::vgContext<>::instance.setBounds(
                internal::vgContext<>::instance.winRect.left,
                internal::vgContext<>::instance.winRect.top,
                internal::vgContext<>::instance.winRect.right - internal::vgContext<>::instance.winRect.left,
                internal::vgContext<>::instance.winRect.bottom - internal::vgContext<>::instance.winRect.top);
        }
    }
}

// ��Ϣѭ������
MINIVG_INLINE bool do_events()
{
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return internal::vgContext<>::instance.running;
}

// ����ִ��
MINIVG_INLINE int start_app()
{
    while(do_events());

    return 0;
}

// ��ʾfps
MINIVG_INLINE void show_fps()
{
    static DWORD t = GetTickCount();
    static int fps_total = 0;
    static int fps = 0;

    ++fps;

    unistring str = L"FPS:";
    str += unistring(fps_total);

    //ezgdi_font(L"΢���ź�", 16);
    font_color(255, 255, 255);
    textout(0, 0, str);

    if(GetTickCount() - t > 1000){
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
// �����¼������밴������
//
//---------------------------------------------------------------------------

// �жϰ����Ƿ���
MINIVG_INLINE bool keystate(int key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

//�����¼�ӳ��
MINIVG_INLINE void key_push_event(EZ_KEY_EVENT function)
{
    internal::vgContext<>::instance.OnKeyDown = function;
}

MINIVG_INLINE void key_pop_event(EZ_KEY_EVENT function)
{
    internal::vgContext<>::instance.OnKeyUp = function;
}

MINIVG_INLINE void input_event(EZ_KEY_EVENT function)
{
    internal::vgContext<>::instance.OnKeyPress = function;
}

//����¼�ӳ��
MINIVG_INLINE void mouse_push_event(EZ_MOUSE_EVENT function)
{
    internal::vgContext<>::instance.OnMouseDown = function;
}

MINIVG_INLINE void mouse_pop_event(EZ_MOUSE_EVENT function)
{
    internal::vgContext<>::instance.OnMouseUp = function;
}

MINIVG_INLINE void mouse_move_event(EZ_MOUSE_EVENT function)
{
    internal::vgContext<>::instance.OnMouseMove = function;
}

MINIVG_INLINE void start_timer(UINT interval)
{
    if(interval){
        SetTimer(graph_window(), UINT(MINIVG_TIMER_ID), interval, NULL);
    }
    else{
        KillTimer(graph_window(), UINT(MINIVG_TIMER_ID));
    }
}

MINIVG_INLINE void stop_timer()
{
    KillTimer(graph_window(), UINT(MINIVG_TIMER_ID));
}

//��ʱ���¼�
MINIVG_INLINE void timer_event(EZ_TIMER_EVENT function)
{
    internal::vgContext<>::instance.OnTimer = function;
}

//���ڻ����¼�
MINIVG_INLINE void display_event(EZ_PAINT_EVENT function)
{
    internal::vgContext<>::instance.OnPaint = function;
}

//---------------------------------------------------------------------------
//
// ��ͼ����
//
//---------------------------------------------------------------------------

// ��� GDI+ ��ͼ�豸
MINIVG_INLINE Gdiplus::Graphics* graphics()
{
    return internal::vgContext<>::instance.g;
}

// ���豳����������Сλ��
MINIVG_INLINE void viewport(int x, int y, int width, int height)
{
    internal::vgContext<>::instance.viewport(0, 0, width, height);
}

// ����������Ƶ�Ŀ�� HDC
MINIVG_INLINE void framebuf_blt(HDC hdc)
{
    internal::vgContext<>::instance.bitblt(hdc);
}

MINIVG_INLINE void set_graphics_effect_level(Gdiplus::Graphics* g, int level)
{
    switch (level) {
    case EZ_SPEED:
        g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);            //���ģʽ
        g->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);       //�������
        g->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);                 //�����
        g->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);                  //����ƫ��ģʽ
        g->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);   //ͼ����������
        break;
    case EZ_MEDIUM:
        g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);            //���ģʽ
        g->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);       //�������
        g->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);                 //�����
        g->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);                  //����ƫ��ģʽ
        g->SetInterpolationMode(Gdiplus::InterpolationModeBilinear);          //ͼ����������
        break;
    case EZ_QUALITY:
        g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);            //���ģʽ
        g->SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);     //�������
        g->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);                 //�����
        g->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);           //����ƫ��ģʽ
        g->SetInterpolationMode(Gdiplus::InterpolationModeBicubic);           //ͼ����������
        break;
    default:
        break;
    }
}

// ������ʾ����
MINIVG_INLINE int effect_level(int level)
{
    Gdiplus::Graphics* g = internal::vgContext<>::instance.g;
    if(!g){
        return -1;
    }
    set_graphics_effect_level(g, level);
    internal::vgContext<>::instance.effectLevel = level;
    
    return 0;
}

// ����
MINIVG_INLINE void clear(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(internal::vgContext<>::instance.g)internal::vgContext<>::instance.g->Clear(Gdiplus::Color(a, r, g, b));
}

//���Ļ�����ɫ
MINIVG_INLINE void pen_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(internal::vgContext<>::instance.pen)internal::vgContext<>::instance.pen->SetColor(Gdiplus::Color(a, r, g, b));
}

MINIVG_INLINE void pen_color(COLORREF argb)
{
    if(internal::vgContext<>::instance.pen)internal::vgContext<>::instance.pen->SetColor(Gdiplus::Color(argb));
}

MINIVG_INLINE void pen_color(vec4ub color)
{
    if(internal::vgContext<>::instance.pen)internal::vgContext<>::instance.pen->SetColor(Gdiplus::Color(color.a, color.r, color.g, color.b));
}

// ��ȡ������ɫ
MINIVG_INLINE COLORREF pen_color()
{
    Gdiplus::Color color;
    if(internal::vgContext<>::instance.pen){
        internal::vgContext<>::instance.pen->GetColor(&color);
    }
    return color.GetValue();
}

/*
vec4ub pen_color()
{
    Gdiplus::Color color;
    if(internal::vgContext<>::instance.pen){
        internal::vgContext<>::instance.pen->GetColor(&color);
    }
    return vec4ub(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}
*/

//���ʿ��
MINIVG_INLINE void pen_width(float width)
{
    if(internal::vgContext<>::instance.pen)internal::vgContext<>::instance.pen->SetWidth(width);
}

//���û���ģʽ
MINIVG_INLINE void pen_style(int mode)
{
    if(internal::vgContext<>::instance.pen){
        //if(mode != EZ_CUSTOM){//ȡ���Զ���㻭ģʽ
        //    ezDashStyle(NULL, 0);
        //}
        internal::vgContext<>::instance.pen->SetDashStyle(Gdiplus::DashStyle(mode));
    }
}

//���õ㻭ģʽ���
MINIVG_INLINE void dash_style(const float* dash, int size)
{
    if(internal::vgContext<>::instance.pen){
        //if(internal::vgContext<>::instance.pen->GetDashStyle() == Gdiplus::DashStyleCustom){
            internal::vgContext<>::instance.pen->SetDashPattern(dash, size);
        //}
    }
}

//���������ɫ
MINIVG_INLINE void fill_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(internal::vgContext<>::instance.brush)internal::vgContext<>::instance.brush->SetColor(Gdiplus::Color(a, r, g, b));
}

MINIVG_INLINE void fill_color(COLORREF argb)
{
    if(internal::vgContext<>::instance.brush)internal::vgContext<>::instance.brush->SetColor(Gdiplus::Color(argb));
}

//��ȡ�����ɫ
MINIVG_INLINE COLORREF fill_color()
{
    Gdiplus::Color color;
    if(internal::vgContext<>::instance.brush){
        internal::vgContext<>::instance.brush->GetColor(&color);
    }
    return color.GetValue();
}

//����һ����
MINIVG_INLINE void draw_point(float x, float y, float size)
{
    if(internal::vgContext<>::instance.g){
        Gdiplus::SolidBrush brush(pen_color());
        float half = size * 0.5f;
        internal::vgContext<>::instance.g->FillEllipse(&brush, x - half, y - half, size, size);
    }
}

//�����߶�
MINIVG_INLINE void draw_line(float x1, float y1, float x2, float y2)
{
    if(internal::vgContext<>::instance.g)internal::vgContext<>::instance.g->DrawLine(internal::vgContext<>::instance.pen, x1, y1, x2, y2);
}

//����һ�����ľ���
MINIVG_INLINE void draw_rect(float x, float y, float width, float height)
{
    if(internal::vgContext<>::instance.g)internal::vgContext<>::instance.g->DrawRectangle(internal::vgContext<>::instance.pen, x, y, width, height);
}

//���һ������
MINIVG_INLINE void fill_rect(float x, float y, float width, float height)
{
    if(internal::vgContext<>::instance.g)internal::vgContext<>::instance.g->FillRectangle(internal::vgContext<>::instance.brush, x, y, width, height);
}

//����Բ�Ǿ���
MINIVG_INLINE void draw_roundrect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if(cx > width)cx = width;
    if(cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = internal::vgContext<>::instance.g;
    if(g){
        Gdiplus::GraphicsPath path;
        path.AddArc(x, y, cx, cy, 180, 90);
        path.AddArc(x2, y, cx, cy, 270, 90);
        path.AddArc(x2, y2, cx, cy, 0, 90);
        path.AddArc(x, y2, cx, cy, 90, 90);
        path.CloseFigure();
        g->DrawPath(internal::vgContext<>::instance.pen, &path);
    }
}

//���Բ�Ǿ���
MINIVG_INLINE void fill_roundrect(float x, float y, float width, float height, float cx, float cy)
{
    cx *= 2.0f;
    cy *= 2.0f;

    if(cx > width)cx = width;
    if(cy > height)cy = height;

    float x2 = x + width - cx;
    float y2 = y + height - cy;

    Gdiplus::Graphics *g = internal::vgContext<>::instance.g;
    if(g){
        Gdiplus::GraphicsPath path;
        path.AddArc(x, y, cx, cy, 180, 90);
        path.AddArc(x2, y, cx, cy, 270, 90);
        path.AddArc(x2, y2, cx, cy, 0, 90);
        path.AddArc(x, y2, cx, cy, 90, 90);
        path.CloseFigure();
        g->FillPath(internal::vgContext<>::instance.brush, &path);
    }
}

//������Բ��xyΪԲ��
MINIVG_INLINE void draw_ellipse(float x, float y, float cx, float cy)
{
    if(internal::vgContext<>::instance.g)internal::vgContext<>::instance.g->DrawEllipse(internal::vgContext<>::instance.pen, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//�����Բ
MINIVG_INLINE void fill_ellipse(float x, float y, float cx, float cy)
{
    if(internal::vgContext<>::instance.g)internal::vgContext<>::instance.g->FillEllipse(internal::vgContext<>::instance.brush, x - cx * 0.5f, y - cy * 0.5f, cx, cy);
}

//���ƿ���Բ��xyΪԲ��
MINIVG_INLINE void draw_circle(float x, float y, float r)
{
    return draw_ellipse(x, y, r, r);
}

//���Բ
MINIVG_INLINE void fill_circle(float x, float y, float r)
{
    return fill_ellipse(x, y, r, r);
}

//�����������߶�
MINIVG_INLINE void draw_polyline(const vec2f* points, size_t size)
{
    if(internal::vgContext<>::instance.g){
        internal::vgContext<>::instance.g->DrawLines(internal::vgContext<>::instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//���ƶ����
MINIVG_INLINE void draw_polygon(const vec2f* points, size_t size)
{
    if(internal::vgContext<>::instance.g){
        internal::vgContext<>::instance.g->DrawPolygon(internal::vgContext<>::instance.pen, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//�������
MINIVG_INLINE void fill_polygon(const vec2f* points, size_t size)
{
    if(internal::vgContext<>::instance.g){
        internal::vgContext<>::instance.g->FillPolygon(internal::vgContext<>::instance.brush, reinterpret_cast<const Gdiplus::PointF*>(points), int(size));
    }
}

//---------------------------------------------------------------------------
//
// ���庯��
//
//---------------------------------------------------------------------------

//�������塣�������֡���С�����
MINIVG_INLINE void setfont(const unistring& name, float size, EZGDI_FONTSTYLE style)
{
    if(internal::vgContext<>::instance.font){
        delete internal::vgContext<>::instance.font;
        internal::vgContext<>::instance.font = new Gdiplus::Font(name.c_str(), size, style, Gdiplus::UnitPoint, NULL);
    }
}

MINIVG_INLINE void setfont(const unistring& name, float size, bool bold, bool, bool underline, bool strikeout)
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

MINIVG_INLINE void font_name(const unistring& name)
{
    internal::vgContext<>::instance.fontName = name;
    internal::vgContext<>::instance.fontIsChange = true;
}

MINIVG_INLINE void font_size(float size)
{
    internal::vgContext<>::instance.fontSize = size;
    internal::vgContext<>::instance.fontIsChange = true;
}

MINIVG_INLINE void font_style(int style)
{
    internal::vgContext<>::instance.fontStyle = style;
    internal::vgContext<>::instance.fontIsChange = true;
}

//��ȡ��������
//unistring font_name();
//int font_size();
//int font_style();

//������ɫ
MINIVG_INLINE void font_color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    if(internal::vgContext<>::instance.font_color)internal::vgContext<>::instance.font_color->SetColor(Gdiplus::Color(a, r, g, b));
}

MINIVG_INLINE void font_color(COLORREF color)
{
    if(internal::vgContext<>::instance.font_color)internal::vgContext<>::instance.font_color->SetColor(Gdiplus::Color(color));
}

//�������
MINIVG_INLINE void textout(float x, float y, const char* text, size_t length)
{
    textout(x, y, unistring(text, length));
}

MINIVG_INLINE void textout(float x, float y, const wchar_t* text, size_t length)
{
    if(internal::vgContext<>::instance.g){
        if(internal::vgContext<>::instance.fontIsChange){
            setfont(internal::vgContext<>::instance.fontName, internal::vgContext<>::instance.fontSize, EZGDI_FONTSTYLE(internal::vgContext<>::instance.fontStyle));
            internal::vgContext<>::instance.fontIsChange = false;
        }

        Gdiplus::StringFormat format;
        internal::vgContext<>::instance.g->DrawString(text, INT(length), internal::vgContext<>::instance.font,
            Gdiplus::PointF(x, y), &format, internal::vgContext<>::instance.font_color);
    }
}

MINIVG_INLINE void textout(float x, float y, const unistring& text)
{
    return textout(x, y, text.c_str(), text.length());
}

MINIVG_INLINE void drawtext(float x, float y, float width, float height, const unistring& text, int align)
{
    if(internal::vgContext<>::instance.g){
        if(internal::vgContext<>::instance.fontIsChange){
            setfont(internal::vgContext<>::instance.fontName, internal::vgContext<>::instance.fontSize, EZGDI_FONTSTYLE(internal::vgContext<>::instance.fontStyle));
            internal::vgContext<>::instance.fontIsChange = false;
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

        format.SetAlignment((Gdiplus::StringAlignment)hAlign);//ˮƽ����
        format.SetLineAlignment((Gdiplus::StringAlignment)vAlign);//��ֱ����
        Gdiplus::RectF rect(x, y, width, height);
        internal::vgContext<>::instance.g->DrawString(text.c_str(), INT(text.length()),
            internal::vgContext<>::instance.font,
            rect,
            &format,
            internal::vgContext<>::instance.font_color);

    }
}

// �����ʽ���������printfʹ������
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

//����ַ��������ؿ��
MINIVG_INLINE float textwidth(const unistring& text)
{
    if(internal::vgContext<>::instance.g){
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        internal::vgContext<>::instance.g->MeasureString(text.c_str(), int(text.length()), internal::vgContext<>::instance.font, layoutSize, NULL, &size);
        return size.Width;
    }
    return 0;
}

//����ַ��������ظ߶�
MINIVG_INLINE float textheight(const unistring& text)
{
    if(internal::vgContext<>::instance.g){
        Gdiplus::SizeF layoutSize(FLT_MAX, FLT_MAX);
        Gdiplus::SizeF size;
        internal::vgContext<>::instance.g->MeasureString(text.c_str(), int(text.length()), internal::vgContext<>::instance.font, layoutSize, NULL, &size);
        return size.Height;
    }
    return 0;
}

//---------------------------------------------------------------------------
//
// ͼƬ����
//
//---------------------------------------------------------------------------

inline ezImage::ezImage() : m_handle(), m_data()
{
}

inline ezImage::~ezImage()
{
    this->close();
}

// ����ͼƬ�� gdiplus ͼƬָ��
inline Gdiplus::Bitmap* ezImage::handle()const
{
    return m_handle;
}

// ����һ��ͼƬ��Ĭ��Ϊ32λɫ
inline int ezImage::create(int width, int height, int format)
{
    this->close();
    m_handle = new Gdiplus::Bitmap(width, height, format);
    return 0;
}

// ��һ��ͼƬ��֧��bmp��jpg��png����̬gif�ȳ�����ʽ
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

// ����Դ�м���ͼƬ
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

// ����Դ�е�ͼƬ
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

//ӳ��һ��HBITMAP����
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

// �ж�ͼƬ�Ƿ�Ϊ��
inline bool ezImage::empty()const
{
    return !m_handle;
}

//ͼƬ��ʽ
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_BMP  = L"image/bmp";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_JPG  = L"image/jpeg";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_GIF  = L"image/gif";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_TIFF = L"image/tiff";
EZ_PUBLIC_DECLARE const wchar_t* GDIPLUS_IMAGE_PNG  = L"image/png";

MINIVG_INLINE int GetImageCLSID(const WCHAR* format, CLSID* pCLSID)
{
    //�õ���ʽΪformat��ͼ���ļ��ı���ֵ�����ʸø�ʽͼ���COM�����GUIDֵ������pCLSID��
    UINT num  = 0;
    UINT size = 0;

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
    Gdiplus::GetImageEncodersSize(&num, &size);

    if(size == 0)return -1;    //������Ϣ������

    //�����ڴ�
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if(!pImageCodecInfo)return -2;    //����ʧ��

    //���ϵͳ�п��õı��뷽ʽ��������Ϣ
    GetImageEncoders(num, size, pImageCodecInfo);

    //�ڿ��ñ�����Ϣ�в���format��ʽ�Ƿ�֧��
    for(UINT i = 0; i < num; ++i){
        //MimeType�����뷽ʽ�ľ�������
        if(wcscmp(pImageCodecInfo[i].MimeType, format) == 0){
            *pCLSID = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return 0;    //�ɹ�
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

// �ͷ�ͼƬ
inline void ezImage::close()
{
    if(m_handle){
        delete m_handle;
        m_handle = NULL;
    }
}

// ����ͼƬ�Ŀ��
inline int ezImage::width()const
{
    return m_handle ? m_handle->GetWidth() : 0;
}

// ����ͼƬ�ĸ߶�
inline int ezImage::height()const
{
    return m_handle ? m_handle->GetHeight() : 0;
}

// ��ȡͼ������
inline void* ezImage::lock(int pixelformat)
{
    if(m_data){
        msgbox(L"ͼƬ�Ѿ�������", L"����");
    }
    else{
        m_data = new Gdiplus::BitmapData();
        Gdiplus::Rect rect(0, 0, this->width(), this->height());
        Gdiplus::Status stat;
        if (pixelformat == EZ_RGB) {
            stat = m_handle->LockBits(0, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat24bppRGB, m_data);
        }
        else {
            stat = m_handle->LockBits(0, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, m_data);
        }

        if(stat == Gdiplus::Ok){
            return m_data->Scan0;
        }
        else{
            return NULL;
        }
    }
}

// ��ԭͼ������
inline void ezImage::unlock()
{
    if(m_data){
        m_handle->UnlockBits(m_data);
        internal::safe_delete(m_data);
    }
}

//
// API ����
//

// ����ͼƬ
MINIVG_INLINE ezImage* newimage(int width, int height)
{
    return internal::vgContext<>::instance.resource.allocate(width, height);
}

// ɾ��ͼƬ
MINIVG_INLINE void freeimage(ezImage* image)
{
    internal::vgContext<>::instance.resource.free(image);
}

MINIVG_INLINE ezImage* loadimage(const unistring& filename)
{
    return internal::vgContext<>::instance.resource.loadimage(filename);
}

MINIVG_INLINE ezImage* loadimage(int id, PCTSTR resource_type)
{
    return internal::vgContext<>::instance.resource.loadimage(id, resource_type);
}

MINIVG_INLINE int saveimage(ezImage* image, const unistring& filename)
{
    if(image){
        image->save(filename);
    }
    return -1;
}

MINIVG_INLINE void drawimage(ezImage* image, float x, float y)
{
    if(internal::vgContext<>::instance.g && image && image->handle())internal::vgContext<>::instance.g->DrawImage(image->handle(), x, y);
}

MINIVG_INLINE void drawimage(ezImage* image, float x, float y, float width, float height)
{
    if(internal::vgContext<>::instance.g && image && image->handle())internal::vgContext<>::instance.g->DrawImage(image->handle(), x, y, width, height);
}

//��xyλ�û���ͼƬ������תһ���Ƕ�
MINIVG_INLINE void rotate_image(ezImage* image, float x, float y, float rotate)
{
    return rotate_image(image, x, y, float(image->width()), float(image->height()), rotate);
}

//��xyλ�û���ͼƬ�����ţ�����תһ���Ƕ�
MINIVG_INLINE void rotate_image(ezImage* image, float x, float y, float width, float height, float rotate)
{
    Gdiplus::Graphics* g = internal::vgContext<>::instance.g;
    if(g && image){
        float cx = width / 2;
        float cy = height / 2;
        Gdiplus::Matrix m;
        g->GetTransform(&m);
        g->TranslateTransform(x, y);//�ƶ�����ǰλ��
        g->RotateTransform(rotate); //��ת
        g->TranslateTransform(-cx, -cy);//�ƶ�����ת����
        g->DrawImage(image->handle(), 0.0f, 0.0f, width, height);//����ͼ��
        g->SetTransform(&m);
    }
}

// ������ֱ�ӻ��Ƶ���Ļ�ϣ����ظ�ʽ������ BGRA 32λ��
MINIVG_INLINE void draw_pixels(float x, float y, float width, float height, void* pixels, int pwidth, int pheight)
{
    Gdiplus::Graphics* g = internal::vgContext<>::instance.g;
    if(g) {
        BYTE* data = static_cast<BYTE*>(pixels);
        data += (pwidth * 4) * (pheight - 1);
        Gdiplus::Bitmap bmp(pwidth, pheight, -pwidth * 4, PixelFormat32bppPARGB, data);
        g->DrawImage(&bmp, x, y, width, height);
    }
}

//---------------------------------------------------------------------------
//
// ��ý��
//
//---------------------------------------------------------------------------

//��������
MINIVG_INLINE void play_music(PCTSTR filename)
{
    std::basic_string<TCHAR> command = TEXT("open ");
    command.append(filename);
    command += TEXT(" alias background");

    mciSendString(command.c_str(), NULL, 0, NULL);
    mciSendString(TEXT("play background repeat"), NULL, 0, NULL);
}

MINIVG_INLINE void stop_music()
{
    mciSendString(TEXT("stop background"), NULL, 0, NULL);//!
    mciSendString(TEXT("close background"), NULL, 0, NULL);
}

//������Դ���ļ�
//�ο����£�https://www.cnblogs.com/zjutlitao/p/3577592.html
MINIVG_INLINE bool ExtractResource(LPCTSTR filename, LPCTSTR resource_type, LPCTSTR resource_name)
{
    // �����ļ�
    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hFile == INVALID_HANDLE_VALUE){
        return false;
    }

    // ������Դ�ļ��С�������Դ���ڴ桢�õ���Դ��С
    HRSRC   hRes   = FindResource(NULL, resource_name, resource_type);
    HGLOBAL hMem   = LoadResource(NULL, hRes);
    DWORD   dwSize = SizeofResource(NULL, hRes);

    // д���ļ�
    DWORD dwWrite = 0;      // ����д���ֽ�
    WriteFile(hFile, hMem, dwSize, &dwWrite, NULL);
    CloseHandle(hFile);

    return true;
}

//������Դ�е�����
MINIVG_INLINE void play_resource_music(PCTSTR filename, PCTSTR resource_type)
{
    std::basic_string<TCHAR> tempfile = ezTempPath();
    tempfile += TEXT("background.mp3");
    // ��mp3��Դ��ȡΪ��ʱ�ļ�
    ExtractResource(tempfile.c_str(), resource_type, filename);
    play_resource_music(tempfile.c_str());
}

MINIVG_INLINE void play_resource_music(int id, PCTSTR resource_type)
{
    return play_resource_music(MAKEINTRESOURCE(id), resource_type);
}

//����wav�ļ�
MINIVG_INLINE int play_sound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_FILENAME|SND_ASYNC;
    if(loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, 0, fdwSound);
}

//������Դ�е�wav�ļ�
MINIVG_INLINE int play_resource_sound(PCTSTR filename, bool loop)
{
    DWORD fdwSound = SND_RESOURCE|SND_ASYNC;
    if(loop)fdwSound |= SND_LOOP;
    return PlaySound(filename, GetModuleHandle(NULL), fdwSound);
}

MINIVG_INLINE int play_resource_sound(int id, bool loop)
{
    return play_resource_sound(MAKEINTRESOURCE(id), loop);
}

//ֹͣ��������
MINIVG_INLINE void stop_sound()
{
    PlaySound(NULL, NULL, SND_FILENAME);
}

//---------------------------------------------------------------------------
//
// �Ի���
//
//---------------------------------------------------------------------------

//��Ϣ�Ի���
MINIVG_INLINE int msgbox(const unistring& message, const unistring& title, int type)
{
    return MessageBoxW(graph_window(), message.c_str(), title.c_str(), type);
}

//��ʾ�����
MINIVG_INLINE unistring inputbox(const unistring& message, const unistring& title, const unistring& default_value)
{
    internal::ezInputBox box;
    if(box.execute(graph_window(), title, message, default_value)){
        return box.text();
    }
    else{
        return unistring();
    }
}

}// end namespace minivg

#endif //EZGDI_INL_20200708233153
