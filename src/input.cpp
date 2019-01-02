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

struct MouseInput
{
    // FIXME: remove the term 'mouse' here, it's implicit!
    
    b32 left_button_is_down;
    b32 left_button_has_gone_up;
    b32 left_button_has_gone_down;
    b32 left_button_has_gone_down_twice;
    
    b32 right_button_is_down;
    b32 right_button_has_gone_up;
    b32 right_button_has_gone_down;
    b32 right_button_has_gone_down_twice;
    
    b32 wheel_has_moved;
    f32 wheel_delta;

    b32 has_moved;
    Pos2d position;
};

#define MAX_KEY_SEQUENCE_PER_FRAME 25 // Keep this the same in input.js!

struct KeyboardInput
{
    u8 keys_that_are_down[255];
    
    u8 sequence_keys_up_down[MAX_KEY_SEQUENCE_PER_FRAME * 2];
    i32 sequence_keys_length;
    
    // TODO: maybe add these (or compute it using the others) u8 keys_that_have_gone_down[255];
    // u8 keys_that_have_gone_down[255];
    // u8 keys_that_have_gone_up[255];
};

#define MAX_NR_OF_TOUCHES 10 // Keep this the same in input.js!

struct TouchInput
{
    i32 identifier;
    b32 has_moved;
    b32 has_started;
    b32 has_ended;
    b32 was_canceled;
    Pos2d position;
};

struct TouchesInput
{
    TouchInput touches[MAX_NR_OF_TOUCHES];
    i32 touch_count;
};

struct FileUpload
{
    DynamicString file_contents;
    DynamicString file_name;
    b32 file_was_uploaded;
};

#define MAX_NR_OF_FRAMES_FOR_TIMING 60 // Keep this the same in input.js!

struct FrameTiming
{
    f32 input_time;
    f32 updating_time;
    f32 rendering_time;
    f32 waiting_time;
};

struct Timing
{
    f32 dt;
    FrameTiming frame_times[MAX_NR_OF_FRAMES_FOR_TIMING];
    i32 frame_index;
};

struct Screen
{
    i32 width;
    i32 height;
    f32 scale;
    
    f32 device_pixel_ratio;
    b32 using_physical_pixels;
};

struct Input
{
    MouseInput     mouse;
    KeyboardInput  keyboard;
    TouchesInput   touch;
    FileUpload     file;
    Timing         timing;
    Screen         screen;
};

Input global_input = {};
Input new_input = {};

extern "C" {
    
    // Mouse
    void set_left_mouse_button_data(b32 left_button_is_down, b32 left_button_has_gone_up,
                                    b32 left_button_has_gone_down, b32 left_button_has_gone_down_twice)
    {
       global_input.mouse.left_button_is_down = left_button_is_down;
       global_input.mouse.left_button_has_gone_up = left_button_has_gone_up;
       global_input.mouse.left_button_has_gone_down = left_button_has_gone_down;
       global_input.mouse.left_button_has_gone_down_twice = left_button_has_gone_down_twice;
    }
    
    void set_right_mouse_button_data(b32 right_button_is_down, b32 right_button_has_gone_up,
                                    b32 right_button_has_gone_down, b32 right_button_has_gone_down_twice)
    {
       global_input.mouse.right_button_is_down = right_button_is_down;
       global_input.mouse.right_button_has_gone_up = right_button_has_gone_up;
       global_input.mouse.right_button_has_gone_down = right_button_has_gone_down;
       global_input.mouse.right_button_has_gone_down_twice = right_button_has_gone_down_twice;
    }

    void set_mouse_wheel_data(b32 wheel_has_moved, f32 wheel_delta)
    {
       global_input.mouse.wheel_has_moved = wheel_has_moved;
       global_input.mouse.wheel_delta = wheel_delta;
    }
    
    void set_mouse_position_data(b32 has_moved, i32 x, i32 y)
    {
       global_input.mouse.has_moved = has_moved;
       global_input.mouse.position.x = x;
       global_input.mouse.position.y = y;
    }
    
    // Keyboard
    u8 * get_address_keys_that_are_down()
    {
        return global_input.keyboard.keys_that_are_down;
    }
    
    u8 * get_address_sequence_keys_up_down()
    {
        return global_input.keyboard.sequence_keys_up_down;
    }

    void set_sequence_keys_length(i32 sequence_keys_length)
    {
       global_input.keyboard.sequence_keys_length = sequence_keys_length;
    }
    
    // Touch 
    void set_touch_count(i32 touch_count)
    {
        // TODO: make sure touch_count <= MAX_NR_OF_TOUCHES
        global_input.touch.touch_count = touch_count;
    }
    
    void set_touch_data(i32 touch_index, i32 identifier,
                        b32 has_moved, b32 has_started, 
                        b32 has_ended, b32 was_canceled, 
                        i32 x, i32 y)
    {
        // TODO: make sure touch_index < MAX_NR_OF_TOUCHES
        
        // TODO: what if a touch (with a identifier) has BOTH *ended* and then *started* in the SAME frame?
        
        global_input.touch.touches[touch_index].identifier = identifier;
        global_input.touch.touches[touch_index].has_moved = has_moved;
        global_input.touch.touches[touch_index].has_started = has_started;
        global_input.touch.touches[touch_index].has_ended = has_ended;
        global_input.touch.touches[touch_index].was_canceled = was_canceled;
        global_input.touch.touches[touch_index].position.x = x;
        global_input.touch.touches[touch_index].position.y = y;
    }

    // File upload
    u8 * get_address_file_upload(i32 length)
    {
        if(!global_memory.block_size)
        {
            init_memory(&global_memory);
        }
        global_input.file.file_contents = create_dynamic_string((Color4){0,0,255,255}, cstring_to_string("File content"), length);
        return global_input.file.file_contents.string.data;
    }
    
    u8 * get_address_file_name(i32 file_name_length)
    {
        if(!global_memory.block_size)
        {
            init_memory(&global_memory);
        }
        global_input.file.file_name = create_dynamic_string((Color4){0,255,0,255}, cstring_to_string("File name"), file_name_length);
        return global_input.file.file_name.string.data;
    }
    
    void set_file_was_uploaded(b32 file_was_uploaded)
    {
        global_input.file.file_was_uploaded = file_was_uploaded;
    }

    //Frame times
    void set_frame_time(i32 frame_index, f32 input_time, f32 updating_time, f32 rendering_time, f32 waiting_time)
    {
        // TODO: make sure frame_index <= MAX_NR_OF_FRAMES_FOR_TIMING
        global_input.timing.dt = input_time + updating_time + rendering_time + waiting_time;
        global_input.timing.frame_index = frame_index;
        global_input.timing.frame_times[frame_index].input_time = input_time;
        global_input.timing.frame_times[frame_index].updating_time = updating_time;
        global_input.timing.frame_times[frame_index].rendering_time = rendering_time;
        global_input.timing.frame_times[frame_index].waiting_time = waiting_time;
    }

    // Screen
    void set_screen_size(i32 width, i32 height, f32 scale, f32 device_pixel_ratio, b32 using_physical_pixels)
    {
        global_input.screen.width = width;
        global_input.screen.height = height;
        global_input.screen.scale = scale;

        global_input.screen.device_pixel_ratio = device_pixel_ratio;
        global_input.screen.using_physical_pixels = using_physical_pixels;
    }

}


// Used this as basis: https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values

enum KeyboardKeyCode
{
    Key_Unidentified = 0x00,
    
    Key_Invisible = 0x01,

    Key_Backspace = 0x08,
    Key_Tab = 0x09,
    Key_Clear = 0x0C,
    Key_Enter = 0x0D,
    Key_Shift = 0x10,
    Key_Control = 0x11,
    Key_Alt = 0x12,
    Key_Pause = 0x13,
    Key_CapsLock = 0x14,
    Key_Escape = 0x1B,
    Key_Space = 0x20,

    Key_PageUp = 0x21,
    Key_PageDown = 0x22,
    Key_End = 0x23,
    Key_Home = 0x24,

    Key_ArrowLeft = 0x25,
    Key_ArrowUp = 0x26,
    Key_ArrowRight = 0x27,
    Key_ArrowDown = 0x28,

    Key_Select = 0x29,
    Key_PrintScreen = 0x2C,

    Key_Insert = 0x2D,
    Key_Delete = 0x2E,

    Key_LeftMeta = 0x5B,
    Key_RightMeta = 0x5C,

    Key_StandBy = 0x5F,

    Key_Pad_0 = 0x60,
    Key_Pad_1 = 0x61,
    Key_Pad_2 = 0x62,
    Key_Pad_3 = 0x63,
    Key_Pad_4 = 0x64,
    Key_Pad_5 = 0x65,
    Key_Pad_6 = 0x66,
    Key_Pad_7 = 0x67,
    Key_Pad_8 = 0x68,
    Key_Pad_9 = 0x69,

    Key_F1 = 0x70,
    Key_F2 = 0x71,
    Key_F3 = 0x72,
    Key_F4 = 0x73,
    Key_F5 = 0x74,
    Key_F6 = 0x75,
    Key_F7 = 0x76,
    Key_F8 = 0x77,
    Key_F9 = 0x78,
    Key_F10 = 0x79,
    Key_F11 = 0x7A,
    Key_F12 = 0x7B,
    Key_F13 = 0x7C,
    Key_F14 = 0x7D,
    Key_F15 = 0x7E,
    Key_F16 = 0x7F,
    Key_F17 = 0x80,
    Key_F18 = 0x81,
    Key_F19 = 0x82,
    Key_F20 = 0x83,

    Key_NumLock = 0x90,
    Key_ScrollLock = 0x91,

    Key_LeftShift = 0xA0,
    Key_RightShift = 0xA1,

    Key_LeftControl = 0xA2,
    Key_RightControl = 0xA3,

    Key_LeftAlt = 0xA4,
    Key_RightAlt = 0xA5,

    Key_SemiColon = 0xBA,
    Key_Equals = 0xBB,
    Key_Comma = 0xBC,
    Key_Minus = 0xBD,
    Key_Period = 0xBE,
    Key_ForwardSlash = 0xBF,
    Key_Backtick = 0xC0,
    Key_OpeningSquareBracket = 0xDB,
    Key_BackSlash = 0xDC,
    Key_ClosingSquareBracket = 0xDD,
    Key_SingleQuote = 0xDE,

    Key_ExSel = 0xF8,
    Key_EraseEof = 0xF9,
    Key_Play = 0xFA,
    Key_Oem_Clear = 0xFE
};
