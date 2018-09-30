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

extern "C" {
    void set_left_mouse_button_data(b32 left_mouse_button_is_down, b32 left_mouse_button_has_gone_up,
                                    b32 left_mouse_button_has_gone_down, b32 left_mouse_button_has_gone_down_twice)
    {
       global_input.mouse.left_mouse_button_is_down = left_mouse_button_is_down;
       global_input.mouse.left_mouse_button_has_gone_up = left_mouse_button_has_gone_up;
       global_input.mouse.left_mouse_button_has_gone_down = left_mouse_button_has_gone_down;
       global_input.mouse.left_mouse_button_has_gone_down_twice = left_mouse_button_has_gone_down_twice;
    }
}
