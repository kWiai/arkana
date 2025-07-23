#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;
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

int fullstep = 1000;

step ballStep{ fullstep / 2,fullstep / 2 };
rect gameRect{100,50,600,600};

rect mainrect{ gameRect.posX+390,gameRect.posY+500,100,10 };

rect ball{ gameRect.posX + 400,gameRect.posY + 450,10,10 };

int startBrickPosX = gameRect.posX + 30;
int startBrickPosY = gameRect.posY + 100;

int score = 0;

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
        1000, 800,
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
        //создание блоков
        for (int i = 1; i < 15; i++) 
        {
            bricks.push_back({ startBrickPosX, startBrickPosY, 2 , 80, 20 });
            startBrickPosX += 100;
            if (i % 5 == 0 && i != 0 && i != 1) 
            {
                startBrickPosY += 100;
                startBrickPosX = gameRect.posX+30;
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
        graphics.Clear(Color(160, 165, 185));

        Pen pen(Color(0, 0, 0),2.0f);
        Pen ballPen(Color(0, 0, 0));
        Pen bgPen(Color::Black, 10.f);

        float comp[6] = { 0.0f, 0.2f, 0.3f, 0.7f, 0.8f, 1.0f }; 
        bgPen.SetCompoundArray(comp, 6); 

        SolidBrush solidBrush(Color::Red);
        SolidBrush bgBrush(Color(240, 245, 255));
        HatchBrush brush1(HatchStyleDiagonalCross, Color(180, 160, 196, 255),  Color(0, 0, 0, 0));
        HatchBrush brush2(HatchStyleDottedGrid, Color(80, 255, 154, 162), Color(0, 0, 0, 0)
        );

        graphics.DrawRectangle(&bgPen, gameRect.posX-1, gameRect.posY-1, gameRect.width+1, gameRect.height+1);
        graphics.FillRectangle(&bgBrush, gameRect.posX, gameRect.posY, gameRect.width, gameRect.height);
        graphics.FillRectangle(&brush1, gameRect.posX, gameRect.posY, gameRect.width, gameRect.height);
        graphics.FillRectangle(&brush2, gameRect.posX, gameRect.posY, gameRect.width, gameRect.height);

        //отрисовка блоков
        for (int i = 0; i < bricks.size(); i++) 
        {
            if (bricks[i].health == 1) color = { 0,255,0};
            if (bricks[i].health == 2) color = { 0,0,255 };
            SolidBrush brickBrush(color);
            graphics.FillRectangle(&brickBrush, bricks[i].posX, bricks[i].posY, bricks[i].width, bricks[i].height);
            graphics.DrawRectangle(&pen, bricks[i].posX, bricks[i].posY, bricks[i].width, bricks[i].height);
        }

        graphics.FillRectangle(&solidBrush, mainrect.posX, mainrect.posY, mainrect.width, mainrect.height);
        graphics.DrawRectangle(&pen, mainrect.posX, mainrect.posY, mainrect.width, mainrect.height);
        graphics.FillEllipse(&solidBrush, ball.posX, ball.posY, ball.width, ball.height);
        graphics.DrawEllipse(&ballPen, ball.posX, ball.posY, ball.width, ball.height);

        Font font(L"Times New Roman", 20.f, FontStyleBold);

        wstringstream wss;
        wss << L"Счет: " << score;
        wstring scoreText = wss.str();

        SolidBrush blackBrush(Color::Black);
        graphics.DrawString(scoreText.c_str(), -1, &font, PointF(gameRect.posX+gameRect.width+50,100.f ), &blackBrush);

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

        if (wParam == 1) {
            bool shouldBallMove = true;
            float d = sqrtf(ballStep.x * ballStep.x + ballStep.y * ballStep.y);

            for (float j = 0; j < d; j++) {
                float t = j / d;
                float currentX = ball.posX + ballStep.x * t;
                float currentY = ball.posY - ballStep.y * t;  
                int ballCenterX = currentX + ball.width / 2;
                int ballCenterY = currentY + ball.height / 2;

   
                for (int i = 0; i < bricks.size(); i++) {
                    float x = ballCenterX;
                    float y = ballCenterY;

                    if (ballCenterX < bricks[i].posX) x = bricks[i].posX;
                    else if (ballCenterX > bricks[i].posX + bricks[i].width) x = bricks[i].posX + bricks[i].width;

                    if (ballCenterY < bricks[i].posY) y = bricks[i].posY;
                    else if (ballCenterY > bricks[i].posY + bricks[i].height) y = bricks[i].posY + bricks[i].height;

                    float distX = ballCenterX - x;
                    float distY = ballCenterY - y;
                    float distance = sqrt(distX * distX + distY * distY);

                    if (distance <= ball.width / 2) {
                        bricks[i].health -= 1;
                        score += 500;

 
                        int colLeft = (ball.posX + ball.width) - bricks[i].posX;
                        int colRight = (bricks[i].posX + bricks[i].width) - ball.posX;
                        int colTop = (ball.posY + ball.height) - bricks[i].posY;
                        int colBottom = (bricks[i].posY + bricks[i].height) - ball.posY;

                        int minCol = min(min(colLeft, colRight), min(colTop, colBottom));

                        if (minCol == colLeft || minCol == colRight) {
                            ballStep.x *= -1;
                        }
                        else if (minCol == colTop || minCol == colBottom) {
                            ballStep.y *= -1;  
                        }

                        if (bricks[i].health == 0) {
                            bricks.erase(bricks.begin() + i);
                            i--;
                        }
                        if (bricks.size() == 0) {
                            PostMessage(hWnd, WM_CLOSE, 0, 0);
                        }
                        shouldBallMove = false;
                        break;
                    }
                }


                if (shouldBallMove) {
                    float x = ballCenterX;
                    float y = ballCenterY;

                    if (ballCenterX < mainrect.posX) x = mainrect.posX;
                    else if (ballCenterX > mainrect.posX + mainrect.width) x = mainrect.posX + mainrect.width;

                    if (ballCenterY < mainrect.posY) y = mainrect.posY;
                    else if (ballCenterY > mainrect.posY + mainrect.height) y = mainrect.posY + mainrect.height;

                    float distX = ballCenterX - x;
                    float distY = ballCenterY - y;
                    float distance = sqrt(distX * distX + distY * distY);

                    if (distance <= ball.width / 2 && ballStep.y < 0) {  

                        int x1 = 0;
                        int x2 = 25;
                        int y2 = 25;
                        int y1 = abs((mainrect.posX + mainrect.width / 2) - ballCenterX);
                        float degrees = (((x1 * y1 + x2 * y2) / ((sqrt(x1 * x1 + x2 * x2)) * (sqrt(y1 * y1 + y2 * y2)))) * 180 / 3.14);
                        float t_angle = (100 * (90 - degrees)) / 90;
                        int stepX = round((fullstep * t_angle) / 100);
                        int stepY = round(fullstep - stepX);

                        if (ballCenterX > mainrect.posX + mainrect.width / 2) {
                            ballStep.x = stepX;
                            ballStep.y = stepY;  
                        }
                        else if (ballCenterX == mainrect.posX + mainrect.width / 2) {
                            ballStep.x = 0;
                            ballStep.y = fullstep;  
                        }
                        else {
                            ballStep.x = -stepX;
                            ballStep.y = stepY;  
                        }

                        shouldBallMove = false;
                        break;
                    }
                }

                if (shouldBallMove) {
                    int R = ball.width / 2;
                    if (ballCenterX < gameRect.posX + R || ballCenterX > gameRect.posX + gameRect.width - R) {
                        ballStep.x *= -1;
                        break;
                    }
                    if (ballCenterY < gameRect.posY + R) {
                        ballStep.y *= -1;  
                        break;
                    }
                    if (ballCenterY > gameRect.posY + gameRect.height - R) {
                        PostMessage(hWnd, WM_CLOSE, 0, 0);
                        break;
                    }
                }
                if (j+1 > d - 0.0001f) {
                    ball.posX = currentX;
                    ball.posY = currentY;
                }
            }
        }

        if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && mainrect.posX + mainrect.width < gameRect.posX + gameRect.width)
            mainrect.posX += 10;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000 && mainrect.posX > gameRect.posX)
            mainrect.posX -= 10;

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
