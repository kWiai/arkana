#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <cmath>
#define _USE_MATH_DEFINES
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

int fullstep = 10;

step ballStep{ fullstep / 2,fullstep / 2 };
rect gameRect{ 100,50,600,600 };

rect mainrect{ gameRect.posX + 390,gameRect.posY + 500,100,10 };

rect ball{ gameRect.posX + 400,gameRect.posY + 450,20,20 };

int startBrickPosX = gameRect.posX + 30;
int startBrickPosY = gameRect.posY + 100;



POINT p;

int score = 0;

bool debug = false;

HWND hWnd;
ULONG_PTR gdiplusToken;
HDC hdcBuffer;
HBITMAP hBitmap;
RECT clientRect;



void DrawingGameRect(Graphics& graf, Pen& bgpen, SolidBrush& bgBrush, HatchBrush& brush1, HatchBrush& brush2) {
    graf.DrawRectangle(&bgpen, gameRect.posX - 1, gameRect.posY - 1, gameRect.width + 1, gameRect.height + 1);
    graf.FillRectangle(&bgBrush, gameRect.posX, gameRect.posY, gameRect.width, gameRect.height);
    graf.FillRectangle(&brush1, gameRect.posX, gameRect.posY, gameRect.width, gameRect.height);
    graf.FillRectangle(&brush2, gameRect.posX, gameRect.posY, gameRect.width, gameRect.height);
}

void DrawingMainRect(Graphics &graf, SolidBrush &solidBrush, Pen &pen) {
    graf.DrawRectangle(&pen, mainrect.posX - 1, mainrect.posY - 1, mainrect.width + 1, mainrect.height + 1);
    graf.FillRectangle(&solidBrush, mainrect.posX, mainrect.posY, mainrect.width, mainrect.height);
}

void DrawingBall(Graphics& graf, Pen& ballPen, SolidBrush& solidBrush) {
    graf.DrawEllipse(&ballPen, ball.posX - 1, ball.posY - 1, ball.width + 1, ball.height + 1);
    graf.FillEllipse(&solidBrush, ball.posX, ball.posY, ball.width, ball.height);
}

void DrawingBricks(Graphics& graf, Pen& pen) {
    for (auto brick : bricks) {
        if (brick.health == 1) color = { 0,255,0 };
        else color = { 0,0,255 };
        SolidBrush brickBrush(color);
        graf.FillRectangle(&brickBrush, brick.posX, brick.posY, brick.width, brick.height);
        graf.DrawRectangle(&pen, brick.posX, brick.posY, brick.width, brick.height);
    }
}
void move(float currentStep,float distance,step &currentBallStep, float &currentX, float &currentY) {
    
    if (currentStep + 1 >= distance && !debug) {
        ballStep = currentBallStep;
        ball.posX = currentX;
        ball.posY = currentY;

    }

}
void debuging() {
    
        GetCursorPos(&p);
        ball.posX = p.x;
        ball.posY = p.y;
    
}

void checkMainRectCollisions(float dotx,float doty,float ballCenterX,bool &reflectedThisStep, step& currentBallStep, float& currentX, float& currentY) {
    if (dotx > mainrect.posX && dotx < mainrect.posX + mainrect.width &&
        doty > mainrect.posY && doty < mainrect.posY + mainrect.height) {

        int x1 = 0, x2 = 25, y2 = 25;
        int y1 = abs((mainrect.posX + mainrect.width / 2) - ballCenterX);
        float degrees = (((x1 * y1 + x2 * y2) / ((sqrt(x1 * x1 + x2 * x2)) * (sqrt(y1 * y1 + y2 * y2)))) * 180 / 3.14f);
        float t_angle = (100 * (90 - degrees)) / 90;
        int stepX = round((fullstep * t_angle) / 100);
        int stepY = round(fullstep - stepX);

        if (ballCenterX > mainrect.posX + mainrect.width / 2) {
            currentBallStep.x = stepX;
            currentBallStep.y = stepY;
        }
        else if (ballCenterX == mainrect.posX + mainrect.width / 2) {
            currentBallStep.x = 0;
            currentBallStep.y = fullstep;
        }
        else {
            currentBallStep.x = -stepX;
            currentBallStep.y = stepY;
        }

        reflectedThisStep = true;
    }
}
void checkGameRectCollisions(float &dotx,float &doty,bool &reflectedThisStep, step& currentBallStep, float& currentX, float& currentY) {
    if (dotx < gameRect.posX && !reflectedThisStep) {
        currentBallStep.x *= -1;
        currentX = gameRect.posX + 1;
        reflectedThisStep = true;
    }

    if (dotx > gameRect.posX + gameRect.width && !reflectedThisStep) {
        currentBallStep.x *= -1;
        currentX = gameRect.posX + gameRect.width - ball.width - 1;
        reflectedThisStep = true;
    }

    if (doty < gameRect.posY && !reflectedThisStep) {
        currentBallStep.y *= -1;
        currentY = gameRect.posY + 1;
        reflectedThisStep = true;
    }

    if (doty > gameRect.posY + gameRect.height && !reflectedThisStep) {
        currentBallStep.y *= -1;
        currentY = gameRect.posY + gameRect.height - 1 - ball.width;
        reflectedThisStep = true;
        /*PostMessage(hWnd, WM_CLOSE, 0, 0);*/
    }
}
void checkBricksCollisions(float &dotx,float &doty,float &distance,std::vector<int> &hitBricks,bool &reflectedThisStep, step& currentBallStep, float& currentX, float& currentY) {
    for (int h = 0; h < bricks.size(); h++) {
        if (dotx + currentBallStep.x / distance > bricks[h].posX && dotx + currentBallStep.x / distance < bricks[h].posX + bricks[h].width &&
            doty - currentBallStep.y / distance > bricks[h].posY && doty - currentBallStep.y / distance < bricks[h].posY + bricks[h].height) {

            if (std::find(hitBricks.begin(), hitBricks.end(), h) == hitBricks.end()) {
                if (!reflectedThisStep) {
                    hitBricks.push_back(h);

                    int colLeft = dotx - bricks[h].posX;
                    int colRight = (bricks[h].posX + bricks[h].width) - dotx;
                    int colTop = doty - bricks[h].posY;
                    int colBottom = (bricks[h].posY + bricks[h].height) - doty;
                    int minCol = min(min(colLeft, colRight), min(colTop, colBottom));

                    if (min(colLeft, colRight) == min(colTop, colBottom)) {
                        if (colTop == min(colLeft, colRight)) {
                            currentBallStep.y = abs(currentBallStep.y) * -1;
                            currentBallStep.x *= -1;
                        }
                        else {
                            currentBallStep.y = abs(currentBallStep.y);
                            currentBallStep.x *= -1;
                        }
                    }
                    else if (minCol == colTop || minCol == colBottom) {
                        currentBallStep.y *= -1;
                    }
                    else if (minCol == colLeft || minCol == colRight) {
                        currentBallStep.x *= -1;
                    }

                    currentX -= currentBallStep.x / distance;
                    currentY += currentBallStep.y / distance;

                    reflectedThisStep = true;

                    if (--bricks[h].health <= 0) {
                        bricks.erase(bricks.begin() + h);
                        h--;
                    }
                    if (bricks.size() == 0) {
                        PostMessage(hWnd, WM_CLOSE, 0, 0);
                    }
                    score += 500;
                    break;
                }
            }

        }
    }
}


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
            startBrickPosX += 140;
            if (i % 4 == 0 && i != 0 && i != 1)
            {
                startBrickPosY += 100;
                startBrickPosX = gameRect.posX + 30;
            }
        }

    case WM_PAINT:
    {
        

        SetTimer(hWnd, 1, 30, NULL);

       
       Pen pen(Color(0, 0, 0), 2.0f);
       Pen ballPen(Color(0, 0, 0));
       Pen bgPen(Color::Black, 10.f);
       SolidBrush solidBrush(Color::Red);
       SolidBrush bgBrush(Color(240, 245, 255));
       HatchBrush brush1(HatchStyleDiagonalCross, Color(180, 160, 196, 255), Color(0, 0, 0, 0));
       HatchBrush brush2(HatchStyleDottedGrid, Color(80, 255, 154, 162), Color(0, 0, 0, 0));
       step currentBallStep = ballStep;
       float currentX = ball.posX;
       float currentY = ball.posY;

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hWnd, &ps);

        GetClientRect(hWnd, &clientRect);
        hdcBuffer = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
        SelectObject(hdcBuffer, hBitmap);

        Graphics graphics(hdcBuffer);
        graphics.Clear(Color(160, 165, 185));
        
        DrawingGameRect(graphics,bgPen,bgBrush,brush1,brush2);
        DrawingMainRect(graphics,solidBrush,pen);
        DrawingBall(graphics,ballPen,solidBrush);
        DrawingBricks(graphics, pen);


        float comp[6] = { 0.0f, 0.2f, 0.3f, 0.7f, 0.8f, 1.0f };
        bgPen.SetCompoundArray(comp, 6);
        
        
        
        
        float d = sqrtf(ballStep.x * ballStep.x + ballStep.y * ballStep.y);
        int R = ball.width / 2;


        for (float j = 0; j < d; j++) {
            float step_dx = currentBallStep.x / d;
            float step_dy = -currentBallStep.y / d;
            currentX += step_dx;
            currentY += step_dy;

            float calcStepY = -currentBallStep.y;
            float bdegress = atan2(calcStepY, currentBallStep.x) * (180.0f / 3.14f);
            float startDegress = bdegress - 90.0f;
            float finishDegress = bdegress + 90.0f;

            float ballCenterX = currentX + ball.width / 2;
            float ballCenterY = currentY + ball.height / 2;

            bool reflectedThisStep = false;
            std::vector<int> hitBricks;

            for (float i = startDegress; i < finishDegress; i += 12) {
                float ri = i * (3.14f / 180.0f);
                float dotx = ballCenterX + R * cos(ri);
                float doty = ballCenterY + R * sin(ri);

                checkMainRectCollisions(dotx, doty, ballCenterX, reflectedThisStep,currentBallStep,currentX,currentY);
                checkGameRectCollisions(dotx, doty, reflectedThisStep, currentBallStep, currentX, currentY);
                checkBricksCollisions(dotx, doty, d, hitBricks, reflectedThisStep, currentBallStep, currentX, currentY);

                SetPixel(hdcBuffer, dotx, doty, Color::Black);
            }
            move(j, d, currentBallStep, currentX, currentY);

            if (debug) {

                debuging();
            }
        }

        Font font(L"Times New Roman", 20.f, FontStyleBold);

        wstringstream wss;
        wss << L"Score: " << score;
        wstring scoreText = wss.str();

        SolidBrush blackBrush(Color::Black);
        graphics.DrawString(scoreText.c_str(), -1, &font, PointF(gameRect.posX + gameRect.width + 50, 100.f), &blackBrush);

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
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && mainrect.posX + mainrect.width < gameRect.posX + gameRect.width)
                mainrect.posX += 10;
            if (GetAsyncKeyState(VK_LEFT) & 0x8000 && mainrect.posX > gameRect.posX)
                mainrect.posX -= 10;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                debug = true;
            }
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                debug = false;
            }
            InvalidateRect(hWnd, nullptr, FALSE);
            break;
        }

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}