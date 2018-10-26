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
#include <d2d1.h>

// TODO: now using a global here for the device context in windows. 
//       Is there a way to pass the hdc to render_win32.cpp without 
//       cluttering the platform-independent code?
// static HDC window_dc;
// static HDC backbuffer_dc;
ID2D1HwndRenderTarget * render_target;
ID2D1Factory * d2d_factory;

#include INCLUDE_PROJECT_FILE

b32 keep_running;
i64 performance_count_frequency;

b32 touch_is_enabled;
TOUCHINPUT touch_inputs[MAX_NR_OF_TOUCHES];


void render_d2d(HWND window)
{
    render_target->BeginDraw();

    render_target->Clear( D2D1::ColorF(D2D1::ColorF::White) );

    render_frame();

    HRESULT draw_result = render_target->EndDraw();      
    
    if (FAILED(draw_result) || draw_result == D2DERR_RECREATE_TARGET)
    {
        // FIXME
        // DiscardGraphicsResources();
    }

}
/*
void render(HWND window)
{
    RECT window_rect;
    GetClientRect(window, &window_rect);

    backbuffer_dc = CreateCompatibleDC(window_dc);

    HBITMAP backbuffer_bitmap = CreateCompatibleBitmap(window_dc,
                                                       window_rect.right - window_rect.left,
                                                       window_rect.bottom - window_rect.top);

    HBITMAP window_bitmap = (HBITMAP) SelectObject(backbuffer_dc, backbuffer_bitmap);

    HBRUSH background_brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    FillRect(backbuffer_dc, &window_rect, background_brush);
    DeleteObject(background_brush);

    // FIXME: we should only copy from the backbuffer to the window-buffer when 
    //        we are only reacting to a PAINT message. Only when the world updates
    //        should we (update the world and) draw to the backbuffer.
    //        Right now we always throw away the back-buffer, so this is not possible now.
    render_frame();
    
    BitBlt(window_dc,
           window_rect.left, window_rect.top,
           window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
           backbuffer_dc,
           0, 0,
           SRCCOPY);

    SelectObject(backbuffer_dc, window_bitmap);
    DeleteObject(backbuffer_bitmap);
    DeleteDC(backbuffer_dc);
}
*/

LRESULT CALLBACK WindowProcedure(HWND window, 
                                 UINT msg, 
                                 WPARAM w_param, 
                                 LPARAM l_param)
{
    switch(msg)
    {
        case WM_CLOSE:
            keep_running = false;
        break;
        case WM_DESTROY:
            keep_running = false;
        break;
        case WM_SIZE:
        {
            // FIXME: how should we handle resizing with Direct2D?
            
            /*
            RECT window_rect;
            GetClientRect(window, &window_rect);
            */
            
            /*
            D2D1_SIZE_U size = D2D1::SizeU(window_rect.right, window_rect.bottom);

            render_target->Resize(size);
            InvalidateRect(window, NULL, FALSE);
            */
        
            /*
            // FIXME: we probably don't want to resize by re-creating the render_target each time!
            if (render_target)
            {
                render_target->Release();
                render_target = 0;
            }
            HRESULT render_target_result = d2d_factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    window,
                    D2D1::SizeU(
                        window_rect.right - window_rect.left,
                        window_rect.bottom - window_rect.top)
                ),
                &render_target
            );    
            */
            
        }
        break;
        /*
        case WM_ERASEBKGND:
            return (LRESULT)1; // Say we handled it.
        break;
        */
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(window, &ps);
     
            // render(window);  // TODO: we should only do the BitBlt here
            render_d2d(window);
            
            
            EndPaint(window, &ps);
        }
        break;
        
        // TODO: maybe don't process these input-messages in WindowProcedure, 
        //       since we don't have access to our variables here. We now need to use a global (new_input)
           
        case WM_LBUTTONDOWN: 
        {
           new_input.mouse.left_button_has_gone_down = true;
           new_input.mouse.left_button_is_down = true;
           // FIXME: new_input.mouse.left_button_has_gone_down_twice = ...;
        }
        break;
        case WM_LBUTTONUP:
        {
            new_input.mouse.left_button_has_gone_up = true;
            new_input.mouse.left_button_is_down = false;
        }
        break;
        case WM_RBUTTONDOWN: 
        {
           new_input.mouse.right_button_has_gone_down = true;
           new_input.mouse.right_button_is_down = true;
           // FIXME: new_input.mouse.right_button_has_gone_down_twice = ...;
        }
        break;
        case WM_RBUTTONUP:
        {
            new_input.mouse.right_button_has_gone_up = true;
            new_input.mouse.right_button_is_down = false;
        }
        break;
        case WM_MOUSEMOVE: 
        {
            new_input.mouse.has_moved = true;
            new_input.mouse.x = LOWORD(l_param);
            new_input.mouse.y = HIWORD(l_param);
        }
        break;
        case WM_MOUSEWHEEL:
        {
            new_input.mouse.wheel_has_moved = true;
            new_input.mouse.wheel_delta = GET_WHEEL_DELTA_WPARAM(w_param) / 120;
        }
        break;
        
        case WM_TOUCH:
        {
            i32 nr_of_touch_inputs = LOWORD(w_param);
            
            if (nr_of_touch_inputs > 0)
            {
                
                if (nr_of_touch_inputs <= MAX_NR_OF_TOUCHES)
                {
                    if (GetTouchInputInfo((HTOUCHINPUT)l_param, nr_of_touch_inputs, (PTOUCHINPUT)&touch_inputs, sizeof(TOUCHINPUT)))
                    {
                        for (i32 touch_input_index = 0; touch_input_index < nr_of_touch_inputs; touch_input_index++)
                        {
                            TOUCHINPUT touch_input = touch_inputs[touch_input_index];
                            
                            i32 identifier = touch_input.dwID;
                            
                            POINT point_input;
                            point_input.x = TOUCH_COORD_TO_PIXEL(touch_input.x);
                            point_input.y = TOUCH_COORD_TO_PIXEL(touch_input.y);
                            ScreenToClient(window, &point_input);
                            i32 x = point_input.x;
                            i32 y = point_input.y;

                            // Find (or create) corresponding touch in the list we currenlty keep
                            b32 touch_found = false;
                            i32 touch_index;
                            for (touch_index = 0; touch_index < new_input.touch.touch_count; touch_index++)
                            {
                                if (new_input.touch.touches[touch_index].identifier == identifier)
                                {
                                    touch_found = true;
                                    break;
                                }
                            }
                            if (touch_found)
                            {
                                if (touch_input.dwFlags & TOUCHEVENTF_UP)
                                {
                                    // The touch_input has ended, so we should mark it as such
                                    new_input.touch.touches[touch_index].has_ended = true;
                                    
                                    if (new_input.touch.touches[touch_index].x != x ||
                                        new_input.touch.touches[touch_index].y != y)
                                    {
                                        new_input.touch.touches[touch_index].has_moved = true;
                                        new_input.touch.touches[touch_index].x = x;
                                        new_input.touch.touches[touch_index].y = y;
                                    }
                                }
                                
                                if (touch_input.dwFlags & TOUCHEVENTF_MOVE)
                                {
                                    // The touch_input has moved, so we should mark it as such
                                    new_input.touch.touches[touch_index].has_moved = true;
                                    new_input.touch.touches[touch_index].x = x;
                                    new_input.touch.touches[touch_index].y = y;
                                }
                            }
                            else
                            {
                                if (touch_input.dwFlags & TOUCHEVENTF_DOWN)
                                {
                                    // Create new touch
                                    touch_index = new_input.touch.touch_count;
                                    new_input.touch.touches[touch_index].identifier = identifier;
                                    new_input.touch.touches[touch_index].has_moved = false;
                                    new_input.touch.touches[touch_index].has_started = false;
                                    new_input.touch.touches[touch_index].has_ended = false;
                                    new_input.touch.touches[touch_index].was_canceled = false;
                                    new_input.touch.touches[touch_index].x = x;
                                    new_input.touch.touches[touch_index].y = y;
                                    
                                    new_input.touch.touch_count += 1;
                                }
                                else {
                                    // TODO: we only add it if dwFlags & TOUCHEVENTF_DOWN is the case 
                                    //       if we reach this point, we somehow got an unknown touch identifier (maybe log?)
                                    
                                }
                            }
                            
                        }    
                    }
                    else
                    {
                         // TODO: handle the error
                    }
                }
                else {
                    // TODO: too many touches. handle this appropriately.
                }
            }
            
            // TODO: do we always handle all touch-input events? if not, let DefWindowProc do it.
            CloseTouchInputHandle((HTOUCHINPUT)l_param);
            
        }
        break;
        
        default:
            // OutputDebugStringA("default\n");
            return DefWindowProc(window, msg, w_param, l_param);
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

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
    OutputDebugStringA("Starting!\n");

    /*
    char filename[ MAX_PATH ];

    OPENFILENAME ofn;
    ZeroMemory( &filename, sizeof( filename ) );
    ZeroMemory( &ofn,      sizeof( ofn ) );
    ofn.lStructSize  = sizeof( ofn );
    ofn.hwndOwner    = NULL;  // If you have a window to center over, put its HANDLE here
    ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
    ofn.lpstrFile    = filename;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = "Select a File, yo!";
    ofn.Flags        = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA( &ofn ))
    {
    // std::cout << "You chose the file \"" << filename << "\"\n";
    }
    else
    {
      // ERROR
    }
    */
    
    init_world();
    
    WNDCLASSA window_class = {};
    window_class.style         = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc   = WindowProcedure;
    window_class.hInstance     = instance;
    window_class.hCursor       = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "FlowifyWindowClass";
    
    if(!RegisterClassA(&window_class))
    {
        MessageBox(0, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Creating the Window
    HWND window = CreateWindowExA(
        0, // WS_EX_TOPMOST|WS_EX_LAYERED,  -- WS_EX_CLIENTEDGE
        window_class.lpszClassName,
        "Flowify",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        0, 0, instance, 0);

    if(window == 0)
    {
        MessageBox(0, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Initialize Direct2D
    
    d2d_factory = 0;
    HRESULT factory_result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
    
    RECT window_rect;
    GetClientRect(window, &window_rect);
    
    render_target = 0;
    HRESULT render_target_result = d2d_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            window,
            D2D1::SizeU(
                window_rect.right - window_rect.left,
                window_rect.bottom - window_rect.top)
        ),
        &render_target
    );    
    
    
    
    // window_dc = GetDC(window);

    // Checking if touch is enabled on this machine and if so, registering for touch messages
    i32 touch_info = GetSystemMetrics(SM_DIGITIZER);
    touch_is_enabled = false;
    if ((touch_info & NID_READY) && (touch_info & NID_MULTI_INPUT))
    {
        RegisterTouchWindow(window, 0);
        touch_is_enabled = true;
    }
    
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
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP:
                {
                    // TODO: can we be sure to cast to u8 here?
                    u8 vk_code = (u8)msg.wParam;
                    
                    b32 was_down = ((msg.lParam & (1 << 30)) != 0); // the last != 0 converts it to 0 or 1
                    b32 is_down = ((msg.lParam & (1 << 31)) == 0);  // the last == 0 converts it to 0 or 1
                    
                    new_input.keyboard.keys_that_are_down[vk_code] = (u8)is_down;
                    
                    if (new_input.keyboard.sequence_keys_length < MAX_KEY_SEQUENCE_PER_FRAME)
                    {
                        new_input.keyboard.sequence_keys_up_down[new_input.keyboard.sequence_keys_length * 2] = is_down;
                        new_input.keyboard.sequence_keys_up_down[new_input.keyboard.sequence_keys_length * 2 + 1] = vk_code;
                        new_input.keyboard.sequence_keys_length++;
                    }
                    
                } break;                
                default:
                {
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                } break;
            }

        }

        LARGE_INTEGER clock_counter_before_wait = get_clock_counter();

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
            
            // Set timings in new_input
            new_input.timing.dt = get_seconds_elapsed(last_clock_counter, clock_counter_before_wait);
            new_input.timing.frame_times[new_input.timing.frame_index] = new_input.timing.dt;
            
            if (new_input.timing.frame_index < MAX_NR_OF_FRAMES_FOR_TIMING)
            {
                new_input.timing.frame_index++;
            }
            else
            {
                new_input.timing.frame_index = 0;
            }

            // Set new last_clock_counter
            // TODO: should we call this: clock_counter_before_update_and_render?
            last_clock_counter = get_clock_counter();
        }


        // Copy the new input (recieved via WindowProcedure) into the global input (TODO: shouldn't this be atomic by swapping pointers instead?)
        global_input = new_input;
        
        // TODO: The resetting-code below changed new_input. But WindowProcedure also changes it. 
        //       How do we make sure they don't do it at the same time?
        
        // Resetting mouse input
        new_input.mouse.left_button_has_gone_up = false;
        new_input.mouse.left_button_has_gone_down = false;
        new_input.mouse.left_button_has_gone_down_twice = false;
       
        new_input.mouse.right_button_has_gone_up = false;
        new_input.mouse.right_button_has_gone_down = false;
        new_input.mouse.right_button_has_gone_down_twice = false;
       
        new_input.mouse.wheel_has_moved = false;
       
        new_input.mouse.has_moved = false;

        // Resetting keyboard input
        new_input.keyboard.sequence_keys_length = 0;
        
        // Resetting touch input
        i32 touches_to_delete[MAX_NR_OF_TOUCHES];
        i32 nr_of_touches_to_delete = 0;
        for (i32 touch_index = 0; touch_index < new_input.touch.touch_count; touch_index++)
        {
            if (new_input.touch.touches[touch_index].has_ended || new_input.touch.touches[touch_index].was_canceled)
            {
                touches_to_delete[nr_of_touches_to_delete] = touch_index;
                nr_of_touches_to_delete++;
            }
            
            new_input.touch.touches[touch_index].has_moved = false;
            new_input.touch.touches[touch_index].has_started = false;
            // new_input.touch.touches[touch_index].has_ended = false; // this is irrelevant, since we are going to delete it anyway
            // new_input.touch.touches[touch_index].was_canceled = false; // this is irrelevant, since we are going to delete it anyway
        }
        
        for (i32 touch_to_delete_index = 0; touch_to_delete_index < nr_of_touches_to_delete; touch_to_delete_index++)
        {
            // Delete touch from new_input.touch.touches by copying all touches one index back
            i32 touch_index_to_delete = touches_to_delete[touch_to_delete_index];
            for(i32 touch_index_to_move = touch_index_to_delete + 1; touch_index_to_move < new_input.touch.touch_count; touch_index_to_move++)
            {
                // Copy each touch back to the previous index (removing the 'hole' of the deleted touch)
                new_input.touch.touches[touch_index_to_move - 1] = new_input.touch.touches[touch_index_to_move];
            }
            new_input.touch.touch_count--;
        }
        
        // Update world
        update_frame();
        
        // Render world
        render_d2d(window);
        
        
    }
    
    // FIXME: use template!
    if (render_target)
    {
        render_target->Release();
    }
    if (d2d_factory)
    {
        d2d_factory->Release();
    }
    
    return(0);
    
}
