# ezgdi
　　认识一些初学者，觉得学习C++很困难，于是我帮助他们写了一个小库。库的目的在于简单，易学，易用，欢迎初学者试用！

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
