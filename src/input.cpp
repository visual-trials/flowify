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

struct mouse_input
{
    b32 left_mouse_button_is_down;
    b32 left_mouse_button_has_gone_up;
    b32 left_mouse_button_has_gone_down;
    b32 left_mouse_button_has_gone_down_twice;
    
    b32 right_mouse_button_is_down;
    b32 right_mouse_button_has_gone_up;
    b32 right_mouse_button_has_gone_down;
    b32 right_mouse_button_has_gone_down_twice;
    
    b32 mouse_wheel_has_moved;
    r32 mouse_wheel_delta;

    b32 mouse_has_moved;
    i32 mouse_position_left;
    i32 mouse_position_top;
};

#define MAX_KEY_SEQUENCE_PER_FRAME 25 // Keep this the same in input.js!

struct keyboard_input
{
    u8 keys_that_are_down[255];
    
    u8 sequence_keys_up_down[MAX_KEY_SEQUENCE_PER_FRAME * 2];
    i32 sequence_keys_length;
    
    // TODO: maybe add these (or compute it using the others) u8 keys_that_have_gone_down[255];
    // u8 keys_that_have_gone_down[255];
    // u8 keys_that_have_gone_up[255];
};

struct touch_input
{

};    

struct input
{
    mouse_input     mouse;
    keyboard_input  keyboard;
    touch_input     touch;
};

input global_input = {};
input new_input = {};

extern "C" {
    
    // Mouse
    void set_left_mouse_button_data(b32 left_mouse_button_is_down, b32 left_mouse_button_has_gone_up,
                                    b32 left_mouse_button_has_gone_down, b32 left_mouse_button_has_gone_down_twice)
    {
       global_input.mouse.left_mouse_button_is_down = left_mouse_button_is_down;
       global_input.mouse.left_mouse_button_has_gone_up = left_mouse_button_has_gone_up;
       global_input.mouse.left_mouse_button_has_gone_down = left_mouse_button_has_gone_down;
       global_input.mouse.left_mouse_button_has_gone_down_twice = left_mouse_button_has_gone_down_twice;
    }
    
    void set_right_mouse_button_data(b32 right_mouse_button_is_down, b32 right_mouse_button_has_gone_up,
                                    b32 right_mouse_button_has_gone_down, b32 right_mouse_button_has_gone_down_twice)
    {
       global_input.mouse.right_mouse_button_is_down = right_mouse_button_is_down;
       global_input.mouse.right_mouse_button_has_gone_up = right_mouse_button_has_gone_up;
       global_input.mouse.right_mouse_button_has_gone_down = right_mouse_button_has_gone_down;
       global_input.mouse.right_mouse_button_has_gone_down_twice = right_mouse_button_has_gone_down_twice;
    }

    void set_mouse_wheel_data(b32 mouse_wheel_has_moved, r32 mouse_wheel_delta)
    {
       global_input.mouse.mouse_wheel_has_moved = mouse_wheel_has_moved;
       global_input.mouse.mouse_wheel_delta = mouse_wheel_delta;
    }
    
    void set_mouse_position_data(b32 mouse_has_moved, i32 mouse_position_left, i32 mouse_position_top)
    {
       global_input.mouse.mouse_has_moved = mouse_has_moved;
       global_input.mouse.mouse_position_left = mouse_position_left;
       global_input.mouse.mouse_position_top = mouse_position_top;
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
    
}


// Used this as basis: https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values

enum keyboard_key_code
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
