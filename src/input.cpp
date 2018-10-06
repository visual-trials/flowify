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

struct keyboard_input
{
    b32 ctrl_key_is_down;
    b32 ctrl_key_has_gone_down;
    b32 ctrl_key_has_gone_up;

    b32 shift_key_is_down;
    b32 shift_key_has_gone_down;
    b32 shift_key_has_gone_up;

    b32 alt_key_is_down;
    b32 alt_key_has_gone_down;
    b32 alt_key_has_gone_up;
   
    b32 key_is_down;
    i32 key_that_is_down;
    
    b32 key_has_gone_down;
    i32 key_that_has_gone_down;
    
    b32 key_has_gone_up;
    i32 key_that_has_gone_up;
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
    void set_ctrl_key_data(b32 ctrl_key_is_down, b32 ctrl_key_has_gone_down, b32 ctrl_key_has_gone_up)
    {
       global_input.keyboard.ctrl_key_is_down = ctrl_key_is_down;
       global_input.keyboard.ctrl_key_has_gone_down = ctrl_key_has_gone_down;
       global_input.keyboard.ctrl_key_has_gone_up = ctrl_key_has_gone_up;
    }
    
    void set_shift_key_data(b32 shift_key_is_down, b32 shift_key_has_gone_down, b32 shift_key_has_gone_up)
    {
       global_input.keyboard.shift_key_is_down = shift_key_is_down;
       global_input.keyboard.shift_key_has_gone_down = shift_key_has_gone_down;
       global_input.keyboard.shift_key_has_gone_up = shift_key_has_gone_up;
    }
    
    void set_alt_key_data(b32 alt_key_is_down, b32 alt_key_has_gone_down, b32 alt_key_has_gone_up)
    {
       global_input.keyboard.alt_key_is_down = alt_key_is_down;
       global_input.keyboard.alt_key_has_gone_down = alt_key_has_gone_down;
       global_input.keyboard.alt_key_has_gone_up = alt_key_has_gone_up;
    }
    
    void set_other_key_data(b32 key_is_down, i32 key_that_is_down, 
                            b32 key_has_gone_down, i32 key_that_has_gone_down,
                            b32 key_has_gone_up, i32 key_that_has_gone_up)
    {
        global_input.keyboard.key_is_down = key_is_down;
        global_input.keyboard.key_that_is_down = key_that_is_down; 
        global_input.keyboard.key_has_gone_down = key_has_gone_down;
        global_input.keyboard.key_that_has_gone_down = key_that_has_gone_down;
        global_input.keyboard.key_has_gone_up = key_has_gone_up; 
        global_input.keyboard.key_that_has_gone_up = key_that_has_gone_up;
    }
    
}
