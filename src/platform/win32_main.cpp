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

LARGE_INTEGER clock_counter_before_update_and_render;
LARGE_INTEGER clock_counter_after_update;
LARGE_INTEGER clock_counter_after_render;
LARGE_INTEGER clock_counter_after_wait;

#include "../generic.h"
#include "../string.cpp"
#include "../geometry.cpp"
#include "../memory.cpp"
#include "../input.cpp"
#include "../render.cpp"

#include INCLUDE_PROJECT_FILE

b32 keep_running;
i64 performance_count_frequency;

b32 touch_is_enabled;
TOUCHINPUT touch_inputs[MAX_NR_OF_TOUCHES];


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
            resize_d2d(window);
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(window, &ps);
     
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
            new_input.mouse.position.x = LOWORD(l_param);
            new_input.mouse.position.y = HIWORD(l_param);
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
                                    
                                    if (new_input.touch.touches[touch_index].position.x != x ||
                                        new_input.touch.touches[touch_index].position.y != y)
                                    {
                                        new_input.touch.touches[touch_index].has_moved = true;
                                        new_input.touch.touches[touch_index].position.x = x;
                                        new_input.touch.touches[touch_index].position.y = y;
                                    }
                                }
                                
                                if (touch_input.dwFlags & TOUCHEVENTF_MOVE)
                                {
                                    // The touch_input has moved, so we should mark it as such
                                    new_input.touch.touches[touch_index].has_moved = true;
                                    new_input.touch.touches[touch_index].position.x = x;
                                    new_input.touch.touches[touch_index].position.y = y;
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
                                    new_input.touch.touches[touch_index].position.x = x;
                                    new_input.touch.touches[touch_index].position.y = y;
                                    
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

inline f32 get_seconds_elapsed(LARGE_INTEGER start_counter, LARGE_INTEGER end_counter)
{
    f32 seconds_elapsed = ((f32)(end_counter.QuadPart - start_counter.QuadPart) / (f32)performance_count_frequency);
    return seconds_elapsed;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
    // FIXME: this only works in Windows 10! And it doesn't work for direct2d!
    // SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
    
    OutputDebugStringA("Starting!\n");

    i32 dynamic_memory_size = 32 * 1024 * 1024;
    void * game_memory_address = (void *)VirtualAlloc(0, dynamic_memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    if (!game_memory_address)
    {
        MessageBox(0, "Could not allocate dynamic memory!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    global_memory.base_address = game_memory_address;
    global_memory.size = dynamic_memory_size;

    init_world();
    
    WNDCLASSA window_class = {};
    window_class.style         = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc   = WindowProcedure;
    window_class.hInstance     = instance;
    window_class.hCursor       = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "FlowifyWindowClass";
    
    if(!RegisterClassA(&window_class))
    {
        MessageBox(0, "Window registration failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
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
        MessageBox(0, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Initialize Direct2D
    if(!init_d2d(window))
    {
        MessageBox(0, "Direct2D init failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
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
    
    clock_counter_before_update_and_render = get_clock_counter();
    clock_counter_after_update = get_clock_counter();
    clock_counter_after_render = get_clock_counter();
    clock_counter_after_wait = get_clock_counter();
    
    keep_running = true;
    while(keep_running)
    {
        LARGE_INTEGER previous_clock_counter_after_wait = clock_counter_after_wait;
        clock_counter_after_wait = get_clock_counter();
        
        f32 input_time = get_seconds_elapsed(previous_clock_counter_after_wait, clock_counter_before_update_and_render);
        f32 updating_time = get_seconds_elapsed(clock_counter_before_update_and_render, clock_counter_after_update);
        f32 rendering_time = get_seconds_elapsed(clock_counter_after_update, clock_counter_after_render);
        f32 waiting_time = get_seconds_elapsed(clock_counter_after_render, clock_counter_after_wait);
        
        // Get all keyboard events and set them in the new_input
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

        // Set timings in new_input
        new_input.timing.dt = get_seconds_elapsed(previous_clock_counter_after_wait, clock_counter_after_wait);
        new_input.timing.frame_times[new_input.timing.frame_index].input_time = input_time;
        new_input.timing.frame_times[new_input.timing.frame_index].updating_time = updating_time;
        new_input.timing.frame_times[new_input.timing.frame_index].rendering_time = rendering_time;
        new_input.timing.frame_times[new_input.timing.frame_index].waiting_time = waiting_time;

        // Set screen size
        RECT window_rect;
        GetClientRect(window, &window_rect);
        new_input.screen.width = window_rect.right - window_rect.left;
        new_input.screen.height = window_rect.bottom - window_rect.top;
        new_input.screen.scale = 1.0f;                  // TODO: right now we do not support dpi-awareness
        new_input.screen.device_pixel_ratio = 1.0f;     // TODO: right now we do not support dpi-awareness
        new_input.screen.using_physical_pixels = false; // TODO: right now we do not support dpi-awareness

        // Copy the new input (recieved via WindowProcedure and above) into the global input (TODO: shouldn't this be atomic by swapping pointers instead?)
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
        
        clock_counter_before_update_and_render = get_clock_counter();
        
        // Update world
        update_frame();
        
        clock_counter_after_update = get_clock_counter();
        
        // Render world
        render_d2d(window);
        
        // Note: clock_counter_after_render is set inside render_d2d
        
        new_input.timing.frame_index++;
        if (new_input.timing.frame_index >= MAX_NR_OF_FRAMES_FOR_TIMING)
        {
            new_input.timing.frame_index = 0;
        }

    }
    
    // Uninitialize Direct2D
    uninit_d2d();
    
    return(0);
    
}
