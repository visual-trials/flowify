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
#include <stdio.h>

// TODO: now using a global here for the device context in windows. 
//       Is there a way to pass the hdc to render_win32.cpp without 
//       cluttering the platform-independent code?
static HDC window_dc;
static HDC backbuffer_dc;

#include "flowify.cpp"

b32 keep_running;
i64 performance_count_frequency;

global_variable i32 increment;

void render(HWND hWnd)
{
    RECT window_rect;
    HBITMAP backbuffer_bitmap, window_bitmap;
    HBRUSH background_brush;

    //
    // Get the size of the client rectangle.
    //

    GetClientRect(hWnd, &window_rect);

    //
    // Create a compatible DC.
    //

    backbuffer_dc = CreateCompatibleDC(window_dc);

    //
    // Create a bitmap big enough for our client rectangle.
    //

    backbuffer_bitmap = CreateCompatibleBitmap(window_dc,
                                               window_rect.right - window_rect.left,
                                               window_rect.bottom - window_rect.top);

    //
    // Select the bitmap into the off-screen DC.
    //

    window_bitmap = (HBITMAP) SelectObject(backbuffer_dc, backbuffer_bitmap);

    //
    // Erase the background.
    //

    // TODO: should we do this here? Of should we do this in the back-buffer itself?
    background_brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    FillRect(backbuffer_dc, &window_rect, background_brush);
    DeleteObject(background_brush);

    //
    // Draw to back buffer
    //
    
    // FIXME: we should only copy from the backbuffer to the window-buffer when 
    //        we are only reacting to a PAINT message. Only when the world updates
    //        should we (update the world and) draw to the backbuffer.
    //        Right now we always throw away the back-buffer, so this is not possible now.
    draw_frame(increment);
    
    //
    // Blt the changes to the screen DC.
    //

    BitBlt(window_dc,
           window_rect.left, window_rect.top,
           window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
           backbuffer_dc,
           0, 0,
           SRCCOPY);

    //
    // Done with off-screen bitmap and DC.
    //

    SelectObject(backbuffer_dc, window_bitmap);
    DeleteObject(backbuffer_bitmap);
    DeleteDC(backbuffer_dc);

}

LRESULT CALLBACK WindowProcedure(HWND hwnd, 
                                 UINT msg, 
                                 WPARAM wParam, 
                                 LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            keep_running = false;
        break;
        case WM_DESTROY:
            keep_running = false;
        break;
        /*
        case WM_ERASEBKGND:
            return (LRESULT)1; // Say we handled it.
        break;
        */
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            render(hwnd);  // FIXME: we should only do the BitBlt here
            EndPaint(hwnd, &ps);
        }
        break;
        
        // FIXME: implement mouse wheel!
        
        case WM_LBUTTONDOWN: 
        {
           new_input.mouse.left_mouse_button_has_gone_down = true;
           new_input.mouse.left_mouse_button_is_down = true;
           // FIXME: new_input.mouse.left_mouse_button_has_gone_down_twice = ...;
        }
        break;
        case WM_LBUTTONUP:
        {
            new_input.mouse.left_mouse_button_has_gone_up = true;
            new_input.mouse.left_mouse_button_is_down = false;
        }
        break;
        case WM_RBUTTONDOWN: 
        {
           new_input.mouse.right_mouse_button_has_gone_down = true;
           new_input.mouse.right_mouse_button_is_down = true;
           // FIXME: new_input.mouse.right_mouse_button_has_gone_down_twice = ...;
        }
        break;
        case WM_RBUTTONUP:
        {
            new_input.mouse.right_mouse_button_has_gone_up = true;
            new_input.mouse.right_mouse_button_is_down = false;
        }
        break;
        case WM_MOUSEMOVE: 
        {
            new_input.mouse.mouse_has_moved = true;
            new_input.mouse.mouse_position_left = LOWORD(lParam);
            new_input.mouse.mouse_position_top = HIWORD(lParam);
        }
        break;
        default:
            // OutputDebugStringA("default\n");
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

inline LARGE_INTEGER get_clock_counter(void)
{    
    LARGE_INTEGER clock_counter;
    QueryPerformanceCounter(&clock_counter);
    return clock_counter;
}

inline r32 get_seconds_elapsed(LARGE_INTEGER start_counter, LARGE_INTEGER end_counter)
{
    r32 seconds_elapsed = ((r32)(end_counter.QuadPart - start_counter.QuadPart) / (r32)performance_count_frequency);
    return seconds_elapsed;
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
    
    window_dc = GetDC(hwnd);

    LARGE_INTEGER performance_count_frequency_result;
    QueryPerformanceFrequency(&performance_count_frequency_result);
    performance_count_frequency = performance_count_frequency_result.QuadPart;
    
    r32 target_seconds_per_frame = 1.0f / 60; // Note: aiming for 60 frames per second
    
    UINT desired_scheduler_in_ms = 1;
    b32 sleep_is_granular = (timeBeginPeriod(desired_scheduler_in_ms) == TIMERR_NOERROR);
    
    LARGE_INTEGER last_clock_counter = get_clock_counter();
    
    keep_running = true;
    while(keep_running)
    {
        MSG msg;
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
        
        // Ensuring a stable frame rate
        {
            LARGE_INTEGER current_clock_counter = get_clock_counter();

            r32 seconds_elapsed_for_frame = get_seconds_elapsed(last_clock_counter, current_clock_counter);
            
            r32 sleep_in_ms = 1000.0f * (target_seconds_per_frame - seconds_elapsed_for_frame);
            DWORD sleep_in_ms_integer = (DWORD)sleep_in_ms;
            if(sleep_in_ms > 0)
            {
                // Sleep a few (whole) milliseconds
                if (sleep_is_granular && sleep_in_ms_integer > 1) {
                    Sleep(sleep_in_ms_integer - 1);
                }
                
                // Then loop the last micro seconds
                while(seconds_elapsed_for_frame < target_seconds_per_frame)
                {                            
                    seconds_elapsed_for_frame = get_seconds_elapsed(last_clock_counter, get_clock_counter());
                }
            }
            else {
                // Missed frame rate!
            }
            last_clock_counter = get_clock_counter();
        }
        

        // Copy the new input (recieved via WindowProcedure) into the global input 
        global_input = new_input;
        
        // Resetting mouse input
        new_input.mouse.left_mouse_button_has_gone_up = false;
        new_input.mouse.left_mouse_button_has_gone_down = false;
        new_input.mouse.left_mouse_button_has_gone_down_twice = false;
       
        new_input.mouse.right_mouse_button_has_gone_up = false;
        new_input.mouse.right_mouse_button_has_gone_down = false;
        new_input.mouse.right_mouse_button_has_gone_down_twice = false;
       
        new_input.mouse.mouse_wheel_has_moved = false;
       
        new_input.mouse.mouse_has_moved = false;
        
        
        // Update
        increment++;
        
        // Render
        render(hwnd);
        
        
        // FIXME: reset other input
        
    }
    
    return(0);
    
}