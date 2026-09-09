#include <iostream>
#include <Windows.h>
#include <cmath>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;
int nMapHeight = 16;
int nMapWidth = 16;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

float fFOV = 3.14159f / 4.0f;
float fDepth = 16.0f;
float fSpeed = 5.0f;



int main()
{
    // Create screen buffer
    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytestWritten = 0;

    wstring map;

    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#...........#..#";
    map += L"#...........#..#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Game loop
    while (1) 
    {

        tp2 = chrono::system_clock::now();
        chrono::duration<float>elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();


        //Controls
        //Handle CCW rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.8f) * fElapsedTime;
        
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (0.8f) * fElapsedTime;

         if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
         {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }

         }

         if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
         {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
         }


        for (int x = 0; x < nScreenWidth; x++)
        {
            // Calculate the projected ray angle for each column
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth * fFOV);
            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
            float fEyeY = cosf(fRayAngle);
            
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // Test if ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;     // Just set distance to maximum depth
                    fDistanceToWall = fDepth;
                }
                else 
                {
                    // Ray is inbounds so test to see if the ray cell is a wall block
                  if (map[(size_t)(nTestY * nMapWidth + nTestX)] == L'#')
                    {
                        bHitWall = true;

                        vector<pair<float, float>> p; //distance, dot
                        for (int tx = 0; tx < 2; tx++)
                          for (int ty = 0; ty < 2; ty++)
                          {
                            float vy = (float)nTestY + ty - fPlayerY;
                            float vx = (float)nTestX + tx - fPlayerX;
                            float d = sqrt(vx * vx + vy*vy);
                            float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                            p.push_back(make_pair(d, dot));
                         
                          }
                          //Sort pairs from closestto fathest
                          sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right ) {return left.first < right.first;});

                          float fBound = 0.01;
                          if (acos(p.at(0).second) < fBound) bBoundary = true;
                          if (acos(p.at(1).second) < fBound) bBoundary = true;
                          if (acos(p.at(2).second) < fBound) bBoundary = true;
                    }
                }
            }

            // Calculate distance to ceiling and floor
            int nCeilling = (int)((float)(nScreenHeight / 2.0f) - (float)nScreenHeight / fDistanceToWall);
            int nFloor = nScreenHeight - nCeilling;

            short nShade = ' ';

            if (fDistanceToWall <= fDepth / 4.0f)        nShade = 0x2588;   // very close
            else if (fDistanceToWall < fDepth / 3.0f)    nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)    nShade = 0x2592;
            else if (fDistanceToWall < fDepth )          nShade = 0x2591;
            else                                         nShade = ' ';    // too far away

            if (bBoundary)        nShade =  ' '; //Black it out                                      
            // Draw the column
            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y < nCeilling)
                    screen[y * nScreenWidth + x] = L' ';
                else if (y > nCeilling && y < nFloor)
                    screen[y * nScreenWidth + x] = nShade; 
                else 
                {
                   // Shade floor based on distance
                   float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f ));
                   if (b <  0.25)        nShade = '#';
                   else if (b < 0.5)     nShade = 'x';
                   else if (b < 0.75)    nShade = '.';
                   else if (b < 0.9)     nShade = '-';
                   else                  nShade = ' ';
                   screen[y*nScreenWidth + x] = nShade;
                   
                   
                }
            }
        }
        //Display states
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        //Display map
        for (int nx = 0; nx < nMapWidth; nx++)
             for (int ny = 0; ny < nMapWidth; ny++)
             {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
             }

            screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

        // Draw the frame to the console
        screen[nScreenWidth * nScreenHeight - 1] = L'\0';
        WriteConsoleOutputCharacterW(hConsole, screen, (DWORD)(nScreenWidth * nScreenHeight), COORD{0,0}, &dwBytestWritten);
    }

    return 0;
}
