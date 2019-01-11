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
 
struct Window
{
    Rect2d screen_rect;
    Rect2d inside_rect;
    
    b32 has_vertical_scrollbar;
    i32 vertical_scrollbar_width;
    
    b32 has_horizontal_scrollbar;
    i32 horizontal_scrollbar_width;
};

struct HoveredOrPressed
{
    b32 is_hovered;
    b32 is_pressed;
};

HoveredOrPressed check_hovered_or_pressed(Pos2d position, Size2d size, Input * input)
{
    HoveredOrPressed result = {};
    
    MouseInput * mouse_input = &input->mouse;
    TouchesInput * touches_input = &input->touch;
    
    if (mouse_input->position.x >= position.x && mouse_input->position.x <= position.x + size.width &&
        mouse_input->position.y >= position.y && mouse_input->position.y <= position.y + size.height)
    {
        result.is_hovered = true;
        if (mouse_input->left_button_has_gone_down)
        {
            result.is_pressed = true;
        }
    }
    
    if (touches_input->touch_count == 1)
    {
        TouchInput * touch_input = &touches_input->touches[0];
        
        if (touch_input->has_started)
        {
            if (touch_input->position.x >= position.x && touch_input->position.x <= position.x + size.width &&
                touch_input->position.y >= position.y && touch_input->position.y <= position.y + size.height)
            {
                result.is_pressed = true;
            }
        }
    }
    
    return result;
}
