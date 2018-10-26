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
 
#include "generic.h"
#include "input.cpp"
#include "render.cpp"

extern "C" {
    
    void init_world()
    {
    }
    
    void update_frame()
    {
    }
    
    void render_frame()
    {
        Color4 line_color;
        line_color.r = 0;
        line_color.g = 100;
        line_color.b = 0;
        line_color.a = 100;
        
        Color4 fill_color;
        fill_color.r = 255;
        fill_color.g = 255;
        fill_color.b = 0;
        fill_color.a = 255;
        
        i32 line_width = 5;
        
        draw_rectangle(100, 100, 300, 200, line_color, fill_color, line_width);
        
        line_color.b = 255;
        line_color.a = 255;
        line_width = 1;
        draw_line(50, 50, 400, 500, line_color, line_width);
    }
}