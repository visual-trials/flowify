/*

   Copyright 2018 Jeffrey Hullekes

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 */

#include <windows.h>

// TODO: now using a global here for the device context in windows. 
//       Is there a way to pass the hdc to render_win32.cpp without 
//       cluttering the platform-independent code?
static HDC device_context;

#include "flowify.cpp"

b32 keep_running;
i64 performance_count_frequency;

// global_variable i32 increment;

LRESULT CALLBACK WindowProcedure(HWND hwnd, 
                                 UINT msg, 
                                 WPARAM wParam, 
                                 LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            keep_running = false;
            // OutputDebugStringA("Close message\n");
            // DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            keep_running = false;
            // OutputDebugStringA("Destroy message\n");
            // PostQuitMessage(0);
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            
            // increment++;
            // FIXME
            HDC device_context_tmp = BeginPaint(hwnd, &ps);
            
            // TODO: we are calling draw_frame here. But we should do it independent of the Windows Messages loop, to garantee a stable frame rate
            // draw_frame(increment);

            EndPaint(hwnd, &ps);
        }
        break;
        default:
            // OutputDebugStringA("default\n");
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA wc = {};
    HWND hwnd;

    // Registering the Window Class
    // wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc   = WindowProcedure;
    // wc.cbClsExtra    = 0;
    // wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    // wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    // wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    // wc.lpszMenuName  = 0;
    wc.lpszClassName = "FlowifyWindowClass";
    // wc.hIconSm       = LoadIcon(0, IDI_APPLICATION);

//    WindowClass.lpfnWndProc = Win32MainWindowCallback;

    if(!RegisterClassA(&wc))
    {
        MessageBox(0, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Creating the Window
    hwnd = CreateWindowExA(
        0, // WS_EX_TOPMOST|WS_EX_LAYERED,  -- WS_EX_CLIENTEDGE
        wc.lpszClassName,
        "Flowify",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
        0, 0, hInstance, 0);

    if(hwnd == 0)
    {
        MessageBox(0, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
//    device_context = GetDC(hwnd);

    LARGE_INTEGER performance_count_frequency_result;
    QueryPerformanceFrequency(&performance_count_frequency_result);
    performance_count_frequency = performance_count_frequency_result.QuadPart;
    
    keep_running = true;

    i32 iteration = 0;
    
    while(keep_running)
    {
        MSG msg;
        
        //OutputDebugString("Begin of loop");
        
        while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            switch(msg.message)
            {
                case WM_QUIT:
                {
                    keep_running = false;
                } break;
                
                default:
                {
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                } break;
            }

        }
        
        Sleep(16);
        
        device_context = GetDC(hwnd);
        
        draw_frame(iteration);
        iteration++;
        
        ReleaseDC(hwnd, device_context);

    }
    
    return(0);
    
}