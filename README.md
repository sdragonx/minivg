# ezgdi
　　认识一些初学者，觉得学习C++很困难，于是我帮助他们写了一个小库。库的目的在于简单，易学，易用，欢迎初学者试用！

# Exsample:

<pre><code>
void OnKeyDown(int Key);
int main()
{
    //初始化窗口
    ezgdi_init("窗口标题", 520, 640);
    //设置按键响应函数
    ezgdi_keydown(OnKeyDown);
    //绘图效果
    ezgdi_effect_level(EZGDI_QUALITY);
    //播放音乐
    playmusic(TEXT("mp3音乐"));

    while(ezgdi_loop()){
        clear(0, 0, 0);//清屏
        pen_color(255, 0, 255, 128);//半透明红色
        draw_line(10, 10, 100, 100);//绘制一条线
        fill_color(0, 255, 0, 128);//半透明绿色
        fill_rect(100, 100, 200, 200);//填充半透明矩形
    };
}
</pre></code>
