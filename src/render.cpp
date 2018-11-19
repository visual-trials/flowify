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
 
// NOTE: always keep this in sync with my.fontFamilies in canvas.js and font_families in win32/render.cpp!
enum FontFamily
{
    Font_Arial = 0x00,
    Font_CourierNew = 0x01
};

struct Font
{
    FontFamily family;
    i32 height;  // font.height is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
};

#if BUILD_FOR_NATIVE_PLATFORM
    #include "win32/render.cpp"
#else
    #include "browser/render.cpp"
#endif

void draw_cross(Pos2d position, i32 distance_from_center, i32 line_length, Color4 line_color, i32 line_width)
{
    draw_line(position.x - distance_from_center, position.y, 
              position.x - distance_from_center - line_length, position.y, 
              line_color, line_width);
    
    draw_line(position.x + distance_from_center, position.y, 
              position.x + distance_from_center + line_length, position.y, 
              line_color, line_width);
    
    draw_line(position.x, position.y - distance_from_center, 
              position.x, position.y - distance_from_center - line_length, 
              line_color, line_width);
              
    draw_line(position.x, position.y + distance_from_center, 
              position.x, position.y + distance_from_center + line_length, 
              line_color, line_width);
}

struct HoveredOrPressed
{
    b32 is_hovered;
    b32 is_pressed;
};

HoveredOrPressed check_hovered_or_pressed(Pos2d position, Size2d size, MouseInput * mouse_input, TouchesInput * touches_input)
{
    HoveredOrPressed result = {};
    
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

b32 do_button(Pos2d position, Size2d size, i32 number, b32 is_active, MouseInput * mouse_input, TouchesInput * touches_input)
{
    
    HoveredOrPressed hovered_or_pressed = check_hovered_or_pressed(position, size, mouse_input, touches_input);
    
    Color4 line_color = {  0,   0,   0, 255};
    Color4 fill_color = {255, 255, 255, 255};
    
    if (is_active)
    {
        fill_color.r = 80;
        fill_color.g = 80;
        fill_color.b = 255;
    }
    else if (hovered_or_pressed.is_hovered)
    {
        fill_color.r = 200;
        fill_color.g = 200;
        fill_color.b = 200;
    }
    
    i32 line_width = 1;
    
    draw_rounded_rectangle(position.x, position.y, size.width, size.height, 5, line_color, fill_color, line_width);
    
    ShortString decimal_string;
    int_to_string(number, &decimal_string);
    Font font = {};
    font.height = 13;
    font.family = Font_Arial;
    Size2d text_size = get_text_size(&decimal_string, font);
    draw_text(position.x + (size.width - text_size.width) / 2 , position.y + (size.height - text_size.height) / 2 , &decimal_string, font, line_color);
    
    return hovered_or_pressed.is_pressed;
}

void do_frame_timing(Input * input, b32 * is_verbose)
{
    Timing * timing = &input->timing;
    Screen * screen = &input->screen;
    MouseInput * mouse_input = &input->mouse;
    TouchesInput * touches_input = &input->touch;
    
    i32 nr_of_bars_to_show = MAX_NR_OF_FRAMES_FOR_TIMING;
    i32 bar_width = 6;
    i32 margin_between_bars = 2;
    i32 normal_bar_height = 100;
    f32 normal_value = (f32)1 / (f32)60; // TODO: should we put 60(fps) in a global?
    
    if (!*is_verbose)
    {
        bar_width = 3;
        margin_between_bars = 0;
        normal_bar_height = 33;
        nr_of_bars_to_show = MAX_NR_OF_FRAMES_FOR_TIMING / 2;
    }
    
    Pos2d start_position = {};
    start_position.x = screen->width - nr_of_bars_to_show * (bar_width + margin_between_bars) - 50;
    start_position.y = screen->height - normal_bar_height - 50;
    
    Color4 input_color =     {  0, 150,   0, 255};
    Color4 updating_color =  {150,   0,   0, 255};
    Color4 rendering_color = {  0,   0, 150, 255};
    Color4 waiting_color =   {220, 220, 220, 255};
    Color4 light_color =     {150, 255, 150, 150};
    Color4 no_color = {};
    
    i32 line_width = 1;
    
    Size2d graph_size = {};
    graph_size.width = nr_of_bars_to_show * (bar_width + margin_between_bars);
    graph_size.height = normal_bar_height;
    
    draw_line(start_position.x, start_position.y, start_position.x + graph_size.width, start_position.y, light_color, 1);
    
    i32 bar_start = start_position.y + normal_bar_height;
    i32 bar_height = 0;
    
    for (i32 frame_index_offset = 0; frame_index_offset > -nr_of_bars_to_show; frame_index_offset--)
    {
        i32 frame_index = (timing->frame_index + frame_index_offset + MAX_NR_OF_FRAMES_FOR_TIMING) % MAX_NR_OF_FRAMES_FOR_TIMING;
        i32 x_left = start_position.x + (nr_of_bars_to_show - 1 + frame_index_offset) * (bar_width + margin_between_bars);
        
        f32 input_time = timing->frame_times[frame_index].input_time;
        f32 updating_time = timing->frame_times[frame_index].updating_time;
        f32 rendering_time = timing->frame_times[frame_index].rendering_time;
        f32 waiting_time = timing->frame_times[frame_index].waiting_time;

        if (!*is_verbose)
        {
            bar_start = start_position.y + normal_bar_height;
            bar_height = ((input_time + updating_time + rendering_time) / normal_value) * normal_bar_height;
            
            draw_rectangle(x_left, bar_start - bar_height, 
                           bar_width, bar_height,
                           no_color, rendering_color,
                           line_width);
            
        }
        else
        {

            // Input time
            
            bar_start = start_position.y + normal_bar_height;
            
            bar_height = (input_time / normal_value) * normal_bar_height;
            
            draw_rectangle(x_left, bar_start - bar_height, 
                           bar_width, bar_height,
                           no_color, input_color, 
                           line_width);
                           
            // Updating time
            
            bar_start = bar_start - bar_height;
            
            bar_height = (updating_time / normal_value) * normal_bar_height;
            
            draw_rectangle(x_left, bar_start - bar_height, 
                           bar_width, bar_height,
                           no_color, updating_color, 
                           line_width);
                       
            // Rendering time
            
            bar_start = bar_start - bar_height;
            
            bar_height = (rendering_time / normal_value) * normal_bar_height;
            
            draw_rectangle(x_left, bar_start - bar_height, 
                           bar_width, bar_height,
                           no_color, rendering_color,
                           line_width);
                           
            // Waiting time
            
            bar_start = bar_start - bar_height;
            
            bar_height = (waiting_time / normal_value) * normal_bar_height;
            
            draw_rectangle(x_left, bar_start - bar_height, 
                           bar_width, bar_height,
                           no_color, waiting_color, 
                           line_width);
        }
    }
    
    HoveredOrPressed hovered_or_pressed = check_hovered_or_pressed(start_position, graph_size, mouse_input, touches_input);
    
    if (hovered_or_pressed.is_pressed)
    {
        *is_verbose = !*is_verbose;
    }
}
