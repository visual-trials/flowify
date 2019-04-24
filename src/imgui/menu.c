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
 
b32 do_button(Pos2d position, Size2d size, ShortString * label, b32 is_active, Input * input, ShortString * label_active = 0)
{
    
    HoveredOrPressed hovered_or_pressed = check_hovered_or_pressed(position, size, input);
    
    Color4 line_color = {  0,   0,   0, 255};
    Color4 fill_color = {255, 255, 255, 255};
    
    if (is_active)
    {
        if (label_active)
        {
            label = label_active;
            if (hovered_or_pressed.is_hovered)
            {
                fill_color.r = 200;
                fill_color.g = 200;
                fill_color.b = 200;
            }
        }
        else
        {
            fill_color.r = 255;
            fill_color.g = 255;
            fill_color.b = 200;
        }
    }
    else if (hovered_or_pressed.is_hovered)
    {
        fill_color.r = 200;
        fill_color.g = 200;
        fill_color.b = 200;
    }
    
    i32 line_width = 1;
    
    draw_rounded_rectangle(position, size, 5, line_color, fill_color, line_width);
    
    // FIXME: HACK!
    if (label)
    {
    
        Font font = {};
        font.height = 13;
        font.family = Font_Arial;
        
        Size2d text_size = get_text_size(label, font);
        
        Pos2d text_position = {};
        text_position.x = position.x + (size.width - text_size.width) / 2;
        text_position.y = position.y + (size.height - text_size.height) / 2;
        draw_text(text_position, label, font, line_color);
    }
    else
    {
        // Drawing menu-icon (3 horizontal lines in button)
        Color4 open_menu_line_color = {70, 70, 70, 255};
        line_width = 3;
        b32 round_cap = true;
        Pos2d start_position = {};
        Pos2d end_position = {};
        start_position.x = position.x + size.width * 3 / 16;
        start_position.y = position.y + size.height * 5 / 16;
        end_position.x = position.x + size.width * 13 / 16;
        end_position.y = start_position.y;
        draw_line(start_position, end_position, open_menu_line_color, line_width, round_cap);
        start_position.y = position.y + size.height * 8 / 16;
        end_position.y = start_position.y;
        draw_line(start_position, end_position, open_menu_line_color, line_width, round_cap);
        start_position.y = position.y + size.height * 11 / 16;
        end_position.y = start_position.y;
        draw_line(start_position, end_position, open_menu_line_color, line_width, round_cap);
    }
    
    return hovered_or_pressed.is_pressed;
}

b32 do_integer_button(Pos2d position, Size2d size, i32 number, b32 is_active, Input * input)
{
    ShortString decimal_string;
    int_to_string(number, &decimal_string);
    // FIXME: HACK!
    if (number != 0)
    {
        return do_button(position, size, &decimal_string, is_active, input);
    }
    else
    {
        return do_button(position, size, 0, is_active, input);
    }
}
