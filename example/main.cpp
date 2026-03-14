
#include "pch.h"

//---------------------------------------------------------------------------
// 变量
//---------------------------------------------------------------------------

vec2f mouse;
vec2f lastpos;
bool buttonDown = false;

//---------------------------------------------------------------------------
// 函数
//---------------------------------------------------------------------------

void key_push(int key);                    // 按键按下
void key_pop(int key);                     // 按键弹起
void input(int key);                       // 字符输入
void mouse_push(int x, int y, int button); // 鼠标按下
void mouse_pop(int x, int y, int button);  // 鼠标弹起
void mouse_move(int x, int y, int button); // 鼠标移动
void display();                            // 显示

//---------------------------------------------------------------------------
// main
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{

    initgraph(1024, 576, VG_SIZEABLE);
    set_title(L"minivg 图形库");

    key_push_event(key_push);
    mouse_push_event(mouse_push);
    mouse_move_event(mouse_move);
    display_event(display);

    // 效果等级 VG_SPEED VG_MEDIUM VG_QUALITY
    effect_level(VG_SPEED);

    set_fps(60);

    return start_app();
}

//---------------------------------------------------------------------------

// 按键按下
void key_push(int key)
{
    switch (key) {
    case VK_F1:
        effect_level(VG_SPEED);
        // effect = L"速度";
        break;
    case VK_F2:
        effect_level(VG_MEDIUM);
        // effect = L"中等";
        break;
    case VK_F3:
        // effect_level(VG_QUALITY);
        // effect = L"质量";
        break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        break;
    case ' ':
        break;
    }
}

// 按键弹起
void key_pop(int key)
{
}

// 字符输入
void input(int key)
{
}

// 鼠标按下
void mouse_push(int x, int y, int button)
{
    // play_resource_sound(ID_SOUND);

    lastpos    = mouse;
    buttonDown = true;

    if (button == VG_LEFT) {
    }
    else {
    }
}

// 鼠标弹起
void mouse_pop(int x, int y, int button)
{
    buttonDown = false;
}

// 鼠标移动
void mouse_move(int x, int y, int button)
{
    mouse = vec2f(x, y);

    if (buttonDown) {
        vec2f offset = mouse - lastpos;
        lastpos      = mouse;
    }
}

void draw_demo()
{
    // 画笔颜色
    pen_color(0xFFFFFFFF);

    // 画笔样式，实心画笔
    pen_style(VG_SOLID);

    // 绘制三条不同宽度的直线

    pen_width(1); // 画笔宽度
    draw_line(10, 10, 110, 10);

    pen_width(2);
    draw_line(10, 20, 110, 20);

    pen_width(4);
    draw_line(10, 30, 110, 30);

    // 画笔样式，点画模式
    pen_style(VG_DOT);

    // 绘制三条不同宽度的直线
    pen_width(1);
    draw_line(120, 10, 220, 10);

    pen_width(2);
    draw_line(120, 20, 220, 20);

    pen_width(4);
    draw_line(120, 30, 220, 30);

    pen_style(VG_SOLID);
    pen_width(1.0f);
    pen_color(255, 255, 255, 255);

    // 绘制基本图元
    draw_rect(10, 50, 100, 40);
    draw_roundrect(10, 100, 100, 40, 10, 10);
    draw_ellipse(10 + 50, 150 + 20, 50, 20);
    draw_circle(10 + 50, 200 + 20, 20);

    // 填充颜色
    fill_color(0, 255, 0, 255);

    // 填充模式绘制基本图元
    fill_rect(120, 50, 100, 40);
    fill_roundrect(120, 100, 100, 40, 10, 10);
    fill_ellipse(120 + 50, 150 + 20, 50, 20);
    fill_circle(120 + 50, 200 + 20, 20);

    // 多边形
    vec2f points[] = {
        vec2f(30, 420),
        vec2f(100, 500),
        vec2f(200, 520),
        vec2f(300, 500),
    };
    pen_color(255, 255, 0, 255);
    draw_polygon(points, 4); // 绘制多边形

    fill_color(0, 255, 0, 64);
    fill_polygon(points, 4); // 填充多边形

    // 文字输出
    pen_color(0x7FFFFFFF);                                    // 绘制的边框颜色
    font_name(L"微软雅黑");                                   // 字体名称
    font_size(16);                                            // 字体大小
    text_color(255, 0, 0, 255);                               // 字体颜色
    draw_rect(10, 250, 200, 50);                              // 绘制边框
    drawtext(10, 250, 200, 50, L"文字左对齐。", VG_CENTER_V); // 输出字体
    text_color(0, 255, 0, 255);
    draw_rect(10, 300, 200, 50);
    drawtext(10, 300, 200, 50, L"文字居中显示。", VG_CENTER);
    text_color(0, 0, 255, 255);
    draw_rect(10, 350, 200, 50);
    drawtext(10, 350, 200, 50, L"文字右对齐。", VG_CENTER_V | VG_RIGHT);

    // 图片绘制
    vgImage* image     = loadimage("assets/sprite.png");
    static float angle = 0.0f;

    for (int i = 0; i < 10; ++i) {
        drawsprite(
            image, 640.0f, 256.0f, 1.0f - i * 0.05f, 1.0f - i * 0.05f,
            // angle + i * 4 + i * angle * 0.5f,
            -(i + 1.0f) * angle * 0.05f, 0.5f, 0.5f
        );
    }

    angle += 1.0f;

    float cx       = image->width();
    float cy       = image->height();
    static float a = 0.0f;
    float scale    = 0.25f;
    vec2f center   = vec2f(cx / 2, 0.75f);

    drawsprite(image, 0.0f, 0.0f, cx, cy, mouse.x, mouse.y, scale, scale, a, center.x, center.y);
    a++;
}

// 显示
void display()
{
    // 清屏
    clear(0, 128, 255);

    draw_demo();

    show_fps();
}
