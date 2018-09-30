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
    #include "render_win32.cpp"
#else
    #include "render_js.cpp"
#endif

#include "input.cpp"

extern "C" {
    void draw_frame(int increment)
    {
        color4 line_color = {};
        color4 fill_color = {};
        
        line_color.r = 255;
        line_color.g = 0;
        line_color.b = 0;
        line_color.a = 255;
        
        fill_color.r = 255;
        fill_color.g = 255;
        fill_color.b = 0;
        fill_color.a = 255;
        
        int line_width = 5;
        
        draw_rectangle(200, 50, 40, 40, line_color, fill_color, 3);
        
        line_color.r = 0;
        line_color.g = 255;
        line_color.b = 0;
        line_color.a = 255;
        
        fill_color.r = 0;
        fill_color.g = 0;
        fill_color.b = 255;
        fill_color.a = 128;
        
        int offset;
        if (increment % 512 < 256) {
            offset = increment % 256;
        }
        else {
            offset = 256 - (increment % 256);
        }
        draw_rectangle(offset + 10, 10, 100, 100, line_color, fill_color, line_width);
        draw_rectangle(10, offset + 10, 100, 100, line_color, fill_color, line_width);
        
        mouse_input mouse = global_input.mouse;
        
        if (mouse.left_mouse_button_is_down)
        {
            jsLogInt(mouse.left_mouse_button_is_down);

            line_color.r = 50;
            line_color.g = 50;
            line_color.b = 50;
            line_color.a = 255;
            
            fill_color.r = 240;
            fill_color.g = 200;
            fill_color.b = 255;
            fill_color.a = 255;
            
            line_width = 2;
            
            draw_rectangle(mouse.mouse_position_left, mouse.mouse_position_top, 150, 150, line_color, fill_color, line_width);
        }
        
    }
}