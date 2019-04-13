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

void draw_cross(Pos2d position, i32 distance_from_center, i32 line_length, Color4 line_color, i32 line_width)
{
    draw_line((Pos2d){position.x - distance_from_center, position.y}, 
              (Pos2d){position.x - distance_from_center - line_length, position.y}, 
              line_color, line_width);
    
    draw_line((Pos2d){position.x + distance_from_center, position.y}, 
              (Pos2d){position.x + distance_from_center + line_length, position.y}, 
              line_color, line_width);
    
    draw_line((Pos2d){position.x, position.y - distance_from_center}, 
              (Pos2d){position.x, position.y - distance_from_center - line_length}, 
              line_color, line_width);
              
    draw_line((Pos2d){position.x, position.y + distance_from_center}, 
              (Pos2d){position.x, position.y + distance_from_center + line_length}, 
              line_color, line_width);
}

void do_physical_pixels_switch(Input * input)
{
    Screen * screen = &input->screen;
    if (screen->device_pixel_ratio != 1.0f)
    {
        Size2d size_button = {50, 50};
        
        Pos2d position_button = {};
        position_button.x = screen->width - size_button.width - 20;
        position_button.y = 20;
        
        ShortString label;
        copy_char_to_string('-', &label);
        ShortString label_active;
        copy_char_to_string('+', &label_active);
        
        b32 button_is_pressed = do_button(position_button, size_button, &label, screen->using_physical_pixels, input, &label_active);
        
        if (button_is_pressed)
        {
            set_using_physical_pixels(!screen->using_physical_pixels);
        }
    }
}

void do_memory_usage(Memory * memory, Input * input, b32 * is_verbose)
{
    Screen * screen = &input->screen;
    
    Font font;
    font.family = Font_Arial;
    font.height = 20;
    
    Color4 black = {0, 0, 0, 255};
    Color4 dark_blue = {0, 0, 100, 255};
    Color4 light_blue = {220, 220, 255, 255};
    Color4 no_color = {};
    
    if (!*is_verbose)
    {
        // TODO: make it small
    }
    
    i32 bar_height = 100;
    i32 bar_width = 10;
    
    i32 left_margin = 100;
    i32 right_margin = 450;
    
    Pos2d start_position = {};
    start_position.x = left_margin;
    start_position.y = screen->height - bar_height - 50;
    
    i32 nr_of_bars_to_show = memory->nr_of_blocks;
    if (left_margin + nr_of_bars_to_show * bar_width + right_margin > screen->width)
    {
        nr_of_bars_to_show = (screen->width - left_margin - right_margin) / bar_width;
    }
    
    for (i32 memory_block_index = 0; memory_block_index < nr_of_bars_to_show; memory_block_index++)
    {
        if (memory->blocks_used[memory_block_index])
        {
            i32 bytes_used = memory->blocks[memory_block_index].bytes_used;
            i32 block_size = memory->block_size;
            
            i32 percentage_used = (i32)(bar_height * (f32)((f32)bytes_used / (f32)block_size));
            
            Color4 color = memory->blocks[memory_block_index].color;
            
            Color4 light_color = color;
            light_color.a /= 3;
            
            draw_rectangle((Pos2d){start_position.x + memory_block_index * bar_width, start_position.y + bar_height - percentage_used}, (Size2d){bar_width,percentage_used}, no_color, color, 1);
            draw_rectangle((Pos2d){start_position.x + memory_block_index * bar_width, start_position.y}, (Size2d){bar_width, bar_height - percentage_used}, no_color, light_color, 1);
        }
        else {
            draw_rectangle((Pos2d){start_position.x + memory_block_index * bar_width, start_position.y}, (Size2d){bar_width, bar_height}, no_color, light_blue, 1);
        }
    }
    
}

void do_frame_timing(Input * input, b32 * is_verbose)
{
    Timing * timing = &input->timing;
    Screen * screen = &input->screen;
    
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
        nr_of_bars_to_show = MAX_NR_OF_FRAMES_FOR_TIMING / 8;
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
    
    draw_line(start_position, (Pos2d){start_position.x + graph_size.width, start_position.y}, light_color, 1);
    
    Pos2d bar_position = {};
    Size2d bar_size = {};
    
    i32 bar_start = start_position.y + normal_bar_height;
    
    bar_size.width = bar_width;
    bar_size.height = 0;
    
    for (i32 frame_index_offset = 0; frame_index_offset > -nr_of_bars_to_show; frame_index_offset--)
    {
        i32 frame_index = (timing->frame_index + frame_index_offset + MAX_NR_OF_FRAMES_FOR_TIMING) % MAX_NR_OF_FRAMES_FOR_TIMING;
        i32 x_left = start_position.x + (nr_of_bars_to_show - 1 + frame_index_offset) * (bar_width + margin_between_bars);
        
        f32 input_time = timing->frame_times[frame_index].input_time;
        f32 updating_time = timing->frame_times[frame_index].updating_time;
        f32 rendering_time = timing->frame_times[frame_index].rendering_time;
        f32 waiting_time = timing->frame_times[frame_index].waiting_time;

        bar_position.x = x_left;
        
        if (!*is_verbose)
        {
            bar_start = start_position.y + normal_bar_height;
            bar_size.height = ((input_time + updating_time + rendering_time) / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, rendering_color, line_width);
        }
        else
        {

            // Input time
            
            bar_start = start_position.y + normal_bar_height;
            bar_size.height = (input_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, input_color, line_width);
                           
            // Updating time
            
            bar_start = bar_start - bar_size.height;
            bar_size.height = (updating_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, updating_color, line_width);
                       
            // Rendering time
            
            bar_start = bar_start - bar_size.height;
            bar_size.height = (rendering_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, rendering_color, line_width);
                           
            // Waiting time
            
            bar_start = bar_start - bar_size.height;
            bar_size.height = (waiting_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, waiting_color, line_width);
        }
    }
    
    HoveredOrPressed hovered_or_pressed = check_hovered_or_pressed(start_position, graph_size, input);
    
    if (hovered_or_pressed.is_pressed)
    {
        *is_verbose = !*is_verbose;
    }
}
