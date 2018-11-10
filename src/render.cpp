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
 
#if BUILD_FOR_NATIVE_PLATFORM
    #include "win32/render.cpp"
#else
    #include "browser/render.cpp"
#endif


void draw_cross(i32 x, i32 y, i32 distance_from_center, i32 line_length, Color4 line_color, i32 line_width)
{
    draw_line(x - distance_from_center, y, 
              x - distance_from_center - line_length, y, 
              line_color, line_width);
    
    draw_line(x + distance_from_center, y, 
              x + distance_from_center + line_length, y, 
              line_color, line_width);
    
    draw_line(x, y - distance_from_center, 
              x, y - distance_from_center - line_length, 
              line_color, line_width);
              
    draw_line(x, y + distance_from_center, 
              x, y + distance_from_center + line_length, 
              line_color, line_width);
}

void draw_frame_timing(Timing * timing, Screen * screen, b32 draw_verbose = false)
{
    
    i32 nr_of_bars_to_show = MAX_NR_OF_FRAMES_FOR_TIMING;
    i32 bar_width = 6;
    i32 margin_between_bars = 2;
    i32 normal_bar_height = 100;
    r32 normal_value = (r32)1 / (r32)60; // TODO: should we put 60(fps) in a global?
    
    if (!draw_verbose)
    {
        bar_width = 3;
        margin_between_bars = 0;
        normal_bar_height = 33;
        nr_of_bars_to_show = MAX_NR_OF_FRAMES_FOR_TIMING / 2;
    }
    
    i32 x_start = screen->width - nr_of_bars_to_show * (bar_width + margin_between_bars) - 50;
    i32 y_start = screen->height - normal_bar_height - 50;
    
    Color4  input_color;
    input_color.r = 0;
    input_color.g = 150;
    input_color.b = 0;
    input_color.a = 255;
    
    Color4  updating_color;
    updating_color.r = 150;
    updating_color.g = 0;
    updating_color.b = 0;
    updating_color.a = 255;
    
    Color4  rendering_color;
    rendering_color.r = 0;
    rendering_color.g = 0;
    rendering_color.b = 150;
    rendering_color.a = 255;
    
    Color4 waiting_color;
    waiting_color.r = 220;
    waiting_color.g = 220;
    waiting_color.b = 220;
    waiting_color.a = 255;
    
    Color4 no_color;
    no_color.r = 0;
    no_color.g = 0;
    no_color.b = 0;
    no_color.a = 0;
    
    i32 line_width = 1;
    
    i32 graph_width = nr_of_bars_to_show * (bar_width + margin_between_bars);
    
    Color4 light_color;
    light_color.r = 150;
    light_color.g = 255;
    light_color.b = 150;
    light_color.a = 150;
    draw_line(x_start, y_start, x_start + graph_width, y_start, light_color, 1);
    
    i32 bar_start = y_start + normal_bar_height;
    i32 bar_height = 0;
    
    for (i32 frame_index_offset = 0; frame_index_offset > -nr_of_bars_to_show; frame_index_offset--)
    {
        i32 frame_index = (timing->frame_index + frame_index_offset + MAX_NR_OF_FRAMES_FOR_TIMING) % MAX_NR_OF_FRAMES_FOR_TIMING;
        i32 x_left = x_start + (nr_of_bars_to_show - 1 + frame_index_offset) * (bar_width + margin_between_bars);
        
        r32 input_time = timing->frame_times[frame_index].input_time;
        r32 updating_time = timing->frame_times[frame_index].updating_time;
        r32 rendering_time = timing->frame_times[frame_index].rendering_time;
        r32 waiting_time = timing->frame_times[frame_index].waiting_time;

        if (!draw_verbose)
        {
            bar_start = y_start + normal_bar_height;
            bar_height = ((input_time + updating_time + rendering_time) / normal_value) * normal_bar_height;
            
            draw_rectangle(x_left, bar_start - bar_height, 
                           bar_width, bar_height,
                           no_color, rendering_color,
                           line_width);
            
        }
        else
        {

            // Input time
            
            bar_start = y_start + normal_bar_height;
            
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
    
    
}
