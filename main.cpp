#include <graphics.h>
#include <iterator>


int main() {
    initgraph(1280, 720);
    BeginBatchDraw();

    IMAGE img;
    loadimage(&img, TEXT(""));

    bool isRunning = true;
    while (isRunning) {
        DWORD beginTime = GetTickCount();
        ExMessage msg;
        while (peekmessage(&msg)) {

        }

        FlushBatchDraw();
        DWORD endTime = GetTickCount();
        DWORD divTime = endTime - beginTime;
        if (divTime < 1000 / 144) {
            Sleep(1000 / 144 - divTime);
        }

    }


    EndBatchDraw();
    return 0;
}
