#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <cmath>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;
int startBrickPosX = 30;
int startBrickPosY = 100;
Color color;
struct rect {
    int posX;
    int posY;
    int width;
    int height;
};

struct step {
    int x;
    int y;
};

struct brick {
    int posX;
    int posY;
    int health;
    int width;
    int height;
};

vector<brick> bricks{};

step ballStep{3,3};

rect mainrect{393,500,100,10};

rect ball{ 400,450,10,10 };

bool next_to;

HWND hWnd;
ULONG_PTR gdiplusToken;
HDC hdcBuffer;
HBITMAP hBitmap;
RECT clientRect;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = _T("ArkanaWindowClass");
    wcex.lpszMenuName = NULL;

    if (!RegisterClassEx(&wcex))
        return 0;

    hWnd = CreateWindow(
        _T("ArkanaWindowClass"),
        _T("Arkana - GDI+ Project"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        600, 600,
        NULL, NULL, hInstance, NULL);

    if (!hWnd)
        return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    GdiplusShutdown(gdiplusToken);

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        for (int i = 0; i < 13; i++) {
            bricks.push_back({ startBrickPosX, startBrickPosY, 2 , 50, 50});
            startBrickPosX += 80;
            if (i % 6 == 0 && i != 0 && i != 1) {
                startBrickPosY += 100;
                startBrickPosX = 30;
            }
        }
    case WM_PAINT:
    {
        SetTimer(hWnd, 1, 30, NULL);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &clientRect);
        hdcBuffer = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
        SelectObject(hdcBuffer, hBitmap);
        Graphics graphics(hdcBuffer);
        graphics.Clear(Color::White);
        Pen pen(Color(0, 0, 0));
        SolidBrush solidBrush(Color::Red);

        for (int i = 0; i < bricks.size(); i++) {
            if (bricks[i].health == 1) {
                color = { 0,255,0 };
            }
            if (bricks[i].health == 2) {
                color = { 0,0,255 };
            }
            SolidBrush brickBrush(color);
            graphics.FillRectangle(&brickBrush, bricks[i].posX, bricks[i].posY, bricks[i].width, bricks[i].height);
            graphics.DrawRectangle(&pen, bricks[i].posX, bricks[i].posY, bricks[i].width, bricks[i].height);
        }

        graphics.FillRectangle(&solidBrush, mainrect.posX, mainrect.posY, mainrect.width, mainrect.height);
        graphics.DrawRectangle(&pen, mainrect.posX, mainrect.posY, mainrect.width, mainrect.height);
        graphics.FillEllipse(&solidBrush, ball.posX, ball.posY, ball.width, ball.height);
        graphics.DrawEllipse(&pen, ball.posX, ball.posY, ball.width, ball.height);

        BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcBuffer, 0, 0, SRCCOPY);

        DeleteObject(hBitmap);
        DeleteDC(hdcBuffer);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_SIZE:
        InvalidateRect(hWnd, NULL, TRUE);
        break;
   case WM_TIMER:
       if (wParam == 1 && ball.posX + ball.width <= 600 && ball.posX >= -10
           && ball.posY >= -10 && ball.posY + ball.height <= 570) {
           int R = ball.height/2;
           if (ball.posX + ball.width + ballStep.x >= 570) {
               ballStep.x *= -1;
           }
           if (ball.posX + ballStep.x < 0) {
               ballStep.x *= -1;
           }
           if (ball.posY <= 0 ) {
               ballStep.y *= -1;
           }
           if (ball.posY + ball.height - ballStep.y >= 570) {
               PostMessage(hWnd, WM_CLOSE, 0, 0);
               return 0;
           }
           if (ball.posY + ball.height - ballStep.y >= mainrect.posY && 
               ball.posY < mainrect.posY && 
               ball.posX + ball.width / 2 > mainrect.posX && 
               ball.posX + ball.width / 2 < mainrect.posX + mainrect.width) {
                   
                       int x1 = 0;
                       int x2 = 25;
                       int y2 = 25;
                       int y1 = abs((mainrect.posX + mainrect.width / 2) - (ball.posX + ball.width / 2));
                       float degress = (((x1 * y1 + x2 * y2) / ((sqrt(x1 * x1 + x2 * x2)) * (sqrt(y1 * y1 + y2 * y2)))) * 180 / 3.14);
                       float t = (100 * (90 - degress)) / 90;
                       int stepX = round((6 * t) / 100);
                       int stepY = round(6 - stepX);
                       if (ball.posX + ball.width / 2 > mainrect.posX + mainrect.width / 2) {
                           ballStep.x = stepX;
                           ballStep.y = stepY;
                       }
                       else if(ball.posX+ball.width/2 == mainrect.posX +mainrect.width/2){
                           ballStep.x = 0;
                           ballStep.y = 6;
                       }
                       else{
                           ballStep.x = -1*stepX;
                           ballStep.y = stepY;
                       }
                   }

           for (int i = 0; i < bricks.size(); i++) {
               bool isCollision = false;
               int nextBallX = ball.posX + ballStep.x;
               int nextBallY = ball.posY - ballStep.y;
               int ballCenterX = nextBallX + ball.width / 2;
               int ballCenterY = nextBallY + ball.height / 2;
               float x = ballCenterX;
               float y = ballCenterY;
               if (ballCenterX < bricks[i].posX) x = bricks[i].posX;
               else if (ballCenterX > bricks[i].posX + bricks[i].width) x = bricks[i].posX + bricks[i].width;
               if (ballCenterY < bricks[i].posY) y = bricks[i].posY;
               else if (ballCenterY > bricks[i].posY + bricks[i].height) y = bricks[i].posY + bricks[i].height;
               float distX = ballCenterX - x;
               float distY = ballCenterY - y;
               float distance = sqrt((distX * distX) + (distY * distY));
               isCollision = distance <= ball.width / 2;
               if (isCollision)
               {
                   bricks[i].health -= 1;
                   int colLeft = nextBallX + ball.width - bricks[i].posX;
                   int colRight = bricks[i].posX + bricks[i].width - nextBallX;
                   int colTop = nextBallY + ball.height - bricks[i].posY;
                   int colBottom = bricks[i].posY + bricks[i].height - nextBallY;
                   int minCol = min(min(colLeft, colRight), min(colTop, colBottom));
                   if (minCol == colLeft or minCol == colRight) {
                       ball.posX -= ballStep.x;
                       ballStep.x *= -1;
                   }
                   else if (minCol == colTop or minCol == colBottom) {
                       ball.posY += ballStep.y;
                       ballStep.y  *= -1;
                   }
                   if (bricks[i].health == 0) {
                       size_t index_to_remove = i;
                       bricks.erase(bricks.begin() + index_to_remove);
                       i--;
                   }
                   if (bricks.size() == 0) {
                    PostMessage(hWnd, WM_CLOSE, 0, 0); 
                    return 0;
                   }
                   break;

               }
           }
           ball.posX += ballStep.x;
           ball.posY -= ballStep.y;
           
       
       }
       if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && mainrect.posX + mainrect.width < 585) mainrect.posX += 10;
       if (GetAsyncKeyState(VK_LEFT) & 0x8000 && mainrect.posX > 0) mainrect.posX -= 10;
        InvalidateRect(hWnd, nullptr, FALSE);
        
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

