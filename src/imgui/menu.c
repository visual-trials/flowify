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
    
    Color4 default_line_color = {  0,   0,   0, 255};
    Color4 default_fill_color = {255, 255, 255, 255};
    
    Color4 text_color = {  0,   0,   0, 255};
    
    DrawStyle draw_style = {};
    draw_style.line_color = default_line_color;
    draw_style.fill_color = default_fill_color;
    
    if (is_active)
    {
        if (label_active)
        {
            label = label_active;
            if (hovered_or_pressed.is_hovered)
            {
                draw_style.fill_color.r = 200;
                draw_style.fill_color.g = 200;
                draw_style.fill_color.b = 200;
            }
        }
        else
        {
            draw_style.fill_color.r = 255;
            draw_style.fill_color.g = 255;
            draw_style.fill_color.b = 200;
        }
    }
    else if (hovered_or_pressed.is_hovered)
    {
        draw_style.fill_color.r = 200;
        draw_style.fill_color.g = 200;
        draw_style.fill_color.b = 200;
    }
    
    draw_style.line_width = 1;
    draw_style.corner_radius = 5;
    
    draw_rounded_rectangle(position, size, draw_style);
    
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
        draw_text(text_position, label, font, text_color);
    }
    else
    {
        // Drawing menu-icon (3 horizontal lines in button)
        Color4 open_menu_line_color = {70, 70, 70, 255};
        i32 line_width = 3;
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

b32 do_menu_item(Pos2d position, Size2d size, ShortString * label, b32 is_active, Input * input)
{
    
    HoveredOrPressed hovered_or_pressed = check_hovered_or_pressed(position, size, input);
    
    Color4 default_line_color = {  0,   0,   0, 255};
    Color4 default_fill_color = {255, 255, 255, 255};
    
    Color4 text_color = {  0,   0,   0, 255};
    
    DrawStyle draw_style = {};
    draw_style.line_color = default_line_color;
    draw_style.fill_color = default_fill_color;

    if (is_active)
    {
        draw_style.fill_color.r = 255;
        draw_style.fill_color.g = 255;
        draw_style.fill_color.b = 200;
    }
    else if (hovered_or_pressed.is_hovered)
    {
        draw_style.fill_color.r = 220;
        draw_style.fill_color.g = 220;
        draw_style.fill_color.b = 220;
    }
    
    draw_style.line_width = 1;
    
    draw_rectangle(position, size, draw_style);
    
    Font font = {};
    font.height = 20;
    font.family = Font_CourierNew;
    
    Size2d text_size = get_text_size(label, font);
    
    i32 left_margin = 20;
    Pos2d text_position = {};
    text_position.x = position.x + left_margin;
    text_position.y = position.y + (size.height - text_size.height) / 2;
    draw_text(text_position, label, font, text_color);
    
    return hovered_or_pressed.is_pressed;
}
