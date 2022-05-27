# minivg
　　今天正式发布一个初学者用的程序库——minivg。这个库结构简洁，使用方便，容易理解。包含基础的绘图函数，键盘鼠标控制，音乐、声音播放，是写小作品的不二选择。<br>
　　值得一提的是，这个库封装了 GDI+ 绘图接口，GDI+ 是 Windows Vista 之后的系统内置的绘图 API（XP 也能使用），比起传统 GDI，在图像抗锯齿、ALPHA 半透明等方面得到了支持，非常容易绘制出美轮美奂的图像。Vista 之后的界面绘制效果，离不开 GDI+ 的支持。<br>
　　整个库不需要 lib 文件，也不需要 dll 文件，只需要在你的 cpp 文件里面 #include <minivg.hpp> 它就能工作！测试过的编译器有：C++Builder，vs2017+，gcc。<br>
  
　　主文件：<br>
　　　　minivg.hpp    主要接口，有注释说明。<br>
　　　　minivg.inl    代码实现。<br>
　　项目Github地址：https://github.com/sdragonx/minivg<br>
　　博客地址：https://www.cnblogs.com/sdragonx/p/13184935.html<br>
　　开源协议：MIT（代码在minivg.inl里面，开源，有兴趣自己可以琢磨）<br>

# Exsample:

<pre><code>
void OnKeyDown(int Key);
int main(int argc, char* argv[])
{
    //初始化窗口
    initgraph(1024, 600, EZ_SIZEABLE);
    //设置按键响应函数
    key_push_event(OnKeyDown);
    //绘图效果
    effect_level(EZ_QUALITY);
    //播放音乐
    play_music(TEXT("mp3音乐"));

    //主程序循环
    while(do_events()){
        clear(0, 0, 0);//清屏
        pen_color(255, 0, 255, 128);//半透明红色
        draw_line(10, 10, 100, 100);//绘制一条线
        fill_color(0, 255, 0, 128);//半透明绿色
        fill_rect(100, 100, 200, 200);//填充半透明矩形
    };
    
    //关闭库，释放资源
    quit();
}
</pre></code>

# Exsample:（一些简单绘图的例子）

<pre><code>

void display();

int main(int argc, char* argv[])
{
    // 初始化窗口
    initgraph("窗口标题", 520, 640);

    // 设置绘制函数
    display_event(display);

    // 绘图效果（高质量）
    effect_level(EZ_QUALITY);
    
    // 主程序循环
    return start_app();
}


void display()
{
    //清屏
    clear(0, 128, 255);

    //画笔颜色
    pen_color(0xFFFFFFFF);

    //画笔样式，实心画笔
    pen_style(EZ_SOLID);

    //绘制三条不同宽度的直线

    pen_width(1);          //画笔宽度
    draw_line(10, 10, 110, 10);

    pen_width(2);
    draw_line(10, 20, 110, 20);

    pen_width(4);
    draw_line(10, 30, 110, 30);

    //画笔样式，点画模式
    pen_style(EZ_DOT);

    //绘制三条不同宽度的直线
    pen_width(1);
    draw_line(120, 10, 220, 10);

    pen_width(2);
    draw_line(120, 20, 220, 20);

    pen_width(4);
    draw_line(120, 30, 220, 30);

    pen_style(PS_SOLID);
    pen_width(1.0f);
    pen_color(255, 255, 255, 255);

    //绘制基本图元
    draw_rect(10, 50, 100, 40);
    draw_roundrect(10, 100, 100, 40, 10, 10);
    draw_ellipse(10 + 50, 150 + 20, 100, 40);
    draw_circle(10 + 50, 200 + 20, 40);

    //填充颜色
    fill_color(0, 255, 0, 255);

    //填充模式绘制基本图元
    fill_rect(120, 50, 100, 40);
    fill_roundrect(120, 100, 100, 40, 10, 10);
    fill_ellipse(120 + 50, 150 + 20, 100, 40);
    fill_circle(120 + 50, 200 + 20, 40);

    //多边形
    vec2f points[] = {
        vec2f(30, 420),
        vec2f(100, 500),
        vec2f(200, 520),
        vec2f(300, 500),
    };
    pen_color(255, 255, 0, 255);
    draw_polygon(points, 4);   //绘制多边形

    fill_color(0, 255, 0, 64);
    fill_polygon(points, 4);   //填充多边形

    //文字输出
    pen_color(0x7FFFFFFF);          //绘制的边框颜色
    font_name("微软雅黑");          //字体名称
    font_size(16);                  //字体大小
    font_color(255, 0, 0, 128);     //字体颜色
    draw_rect(10, 250, 200, 50);    //绘制边框
    drawtext(10, 250, 200, 50, L"文字左对齐。", EZ_CENTER_V); //输出字体
    font_color(0, 255, 0, 128);
    draw_rect(10, 300, 200, 50);
    drawtext(10, 300, 200, 50, L"文字居中显示。", EZ_CENTER);
    font_color(0, 0, 255, 128);
    draw_rect(10, 350, 200, 50);
    drawtext(10, 350, 200, 50, L"文字右对齐。", EZ_CENTER_V|EZ_RIGHT);

    //图片绘制
    ezImage* image = loadimage("assets/nv.png");
    static float angle = 0.0f;

    for(int i=0; i<10; ++i){
        rotate_image(image, 512 + i * 32, 200 + i * 20, image->width() / 1 - i * 40, image->height() / 1 - i * 40, angle + i * 16);
    }
}
</pre></code>
![运行效果图](https://github.com/sdragonx/minivg/blob/master/sample.jpg)


# 更新历史

## 2021-07-30
修正 bug。

## 2021-07-30
库名字由 EZGDI 正式更名为 minivg，计划做成 OpenCV 的迷你版。<br>
支持在对接到已有窗口中，创建背景缓冲区，通过 framebuf_blt() 函数显示。<br>
增加多线程渲染（未加入锁帧功能）。<br>
支持全屏和窗口动态切换：fullscreen(bool) 函数。<br>
支持直接绘制内存二进制数据：draw_pixels() 函数，像素必须是 ABGR 格式。<br>
