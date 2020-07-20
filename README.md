# ezgdi
　　今天正式发布一个初学者用的程序库——EZGDI。这个库结构简洁，使用方便，容易理解。包含基础的绘图函数，键盘鼠标控制，音乐、声音播放，是写小作品的不二选择。
　　值得一提的是，这个库封装了GDI+绘图接口，GDI+是Windows Vista之后的系统内置的绘图API（xp也能使用），比起传统GDI，在图像抗锯齿、ALPHA半透明等方面得到了支持，非常容易绘制出美轮美奂的图像。Vista之后的界面绘制效果，离不开GDI+的支持。
　　整个库不需要lib文件，也不需要dll文件，只需要在你的cpp文件里面#include <ezgdi.hpp>它就能工作！测试过的编译器有：C++Builder，vs2017+，gcc。

# Exsample:

<pre><code>
void OnKeyDown(int Key);
int main(int argc, char* argv[])
{
    //初始化窗口
    ezInit("窗口标题", 520, 640);
    //设置按键响应函数
    ezOnKeyDown(OnKeyDown);
    //绘图效果
    ezEffectLevel(EZ_QUALITY);
    //播放音乐
    ezPlayMusic(TEXT("mp3音乐"));

    //主程序循环
    while(ezLoop()){
        ezClear(0, 0, 0);//清屏
        ezPenColor(255, 0, 255, 128);//半透明红色
        ezDrawLine(10, 10, 100, 100);//绘制一条线
        ezFillColor(0, 255, 0, 128);//半透明绿色
        ezFillRect(100, 100, 200, 200);//填充半透明矩形
    };
    
    //关闭库，释放资源
    ezClose();
}
</pre></code>

# Exsample:（一些简单绘图的例子）

<pre><code>

void OnPaint();
int main(int argc, char* argv[])
{
    //初始化窗口
    ezInit("窗口标题", 520, 640);
    //设置按键响应函数
    ezOnPaint(OnPaint);
    //绘图效果
    ezEffectLevel(EZ_QUALITY);
    //播放音乐
    ezPlayMusic(TEXT("mp3音乐"));

    //主程序循环
    ezRun();
    
    //关闭库，释放资源
    ezClose();
}


void OnPaint()
{
    //清屏
    ezClear(0, 128, 255);

    //画笔颜色
    ezPenColor(0xFFFFFFFF);

    //画笔样式，实心画笔
    ezPenStyle(EZ_SOLID);

    //绘制三条不同宽度的直线

    ezPenWidth(1);          //画笔宽度
    ezDrawLine(10, 10, 110, 10);

    ezPenWidth(2);
    ezDrawLine(10, 20, 110, 20);

    ezPenWidth(4);
    ezDrawLine(10, 30, 110, 30);

    //画笔样式，点画模式
    ezPenStyle(EZ_DOT);

    //绘制三条不同宽度的直线
    ezPenWidth(1);
    ezDrawLine(120, 10, 220, 10);

    ezPenWidth(2);
    ezDrawLine(120, 20, 220, 20);

    ezPenWidth(4);
    ezDrawLine(120, 30, 220, 30);

    ezPenStyle(PS_SOLID);
    ezPenWidth(1.0f);
    ezPenColor(255, 255, 255, 255);

    //绘制基本图元
    ezDrawRect(10, 50, 100, 40);
    ezDrawRoundRect(10, 100, 100, 40, 10, 10);
    ezDrawEllipse(10 + 50, 150 + 20, 100, 40);
    ezDrawCircle(10 + 50, 200 + 20, 40);

    //填充颜色
    ezFillColor(0, 255, 0, 255);

    //填充模式绘制基本图元
    ezFillRect(120, 50, 100, 40);
    ezFillRoundRect(120, 100, 100, 40, 10, 10);
    ezFillEllipse(120 + 50, 150 + 20, 100, 40);
    ezFillCircle(120 + 50, 200 + 20, 40);

    //多边形
    vec2f points[] = {
        vec2f(30, 420),
        vec2f(100, 500),
        vec2f(200, 520),
        vec2f(300, 500),
    };
    ezPenColor(255, 255, 0, 255);
    ezDrawPolygon(points, 4);   //绘制多边形

    ezFillColor(0, 255, 0, 64);
    ezFillPolygon(points, 4);   //填充多边形

    //图片绘制
    ezImage* image = ezLoadImage("a.png");
    static float angle = -80;
    //angle += 1;

    for(int i=0; i<10; ++i){
        ezRotateImage(image, 512 + i * 32, 200 + i * 20, image->width() / 1 - i * 40, image->height() / 1 - i * 40, angle + i * 16);
    }

    //文字输出
    ezPenColor(0x7FFFFFFF);         //绘制的边框颜色
    ezFontName("微软雅黑");         //字体名称
    ezFontSize(16);                 //字体大小
    ezFontColor(255, 0, 0, 128);    //字体颜色
    ezDrawRect(10, 250, 200, 50);   //绘制边框
    ezText(10, 250, 200, 50, L"文字左对齐。", EZ_CENTER_V); //输出字体
    ezFontColor(0, 255, 0, 128);
    ezDrawRect(10, 300, 200, 50);
    ezText(10, 300, 200, 50, L"文字居中显示。", EZ_CENTER);
    ezFontColor(0, 0, 255, 128);
    ezDrawRect(10, 350, 200, 50);
    ezText(10, 350, 200, 50, L"文字右对齐。", EZ_CENTER_V|EZ_RIGHT);
    }
</pre></code>
![运行效果图](https://github.com/sdragonx/ezgdi/blob/master/sample.png)
