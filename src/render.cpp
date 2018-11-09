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

void draw_frame_timing(Timing * timing, i32 x_start, i32 y_start)
{
    i32 bar_width = 8;
    i32 margin_between_bars = 2;
    i32 normal_bar_height = 100;
    r32 normal_value = (r32)1 / (r32)60; // TODO: should we put 60(fps) in a global?
    
    Color4  line_color;
    line_color.r = 0;
    line_color.g = 50;
    line_color.b = 0;
    line_color.a = 255;
    
    Color4 fill_color;
    fill_color.r = 0;
    fill_color.g = 100;
    fill_color.b = 0;
    fill_color.a = 255;
    
    i32 line_width = 1;
    
    i32 graph_width = MAX_NR_OF_FRAMES_FOR_TIMING * (bar_width + margin_between_bars);
    
    Color4 light_color;
    light_color.r = 150;
    light_color.g = 255;
    light_color.b = 150;
    light_color.a = 255;
    draw_line(x_start, y_start, x_start + graph_width, y_start, light_color, 1);
    
    for (i32 frame_index_offset = 0; frame_index_offset < MAX_NR_OF_FRAMES_FOR_TIMING; frame_index_offset++)
    {
        i32 frame_index = (timing->frame_index + 1 + frame_index_offset) % MAX_NR_OF_FRAMES_FOR_TIMING;
        r32 dt = timing->frame_times[frame_index];
        
        i32 x_left = x_start + frame_index_offset * (bar_width + margin_between_bars);
        i32 bar_height = (dt / normal_value) * normal_bar_height;
        
        draw_rectangle(x_left, y_start + normal_bar_height - bar_height, 
                       bar_width, bar_height,
                       line_color, fill_color, 
                       line_width);
        
        if (frame_index % 5 == 0)
        {
            draw_line(x_left, y_start + normal_bar_height, x_left, y_start, light_color, 1);
        }
    }
    
    
}
