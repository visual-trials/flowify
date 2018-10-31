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
        Color4 thin_line_color;
        Color4 line_color;
        Color4 fill_color;
        
        thin_line_color.r = 200;
        thin_line_color.g = 200;
        thin_line_color.b = 200;
        thin_line_color.a = 255;
        
        line_color.r = 10;
        line_color.g = 40;
        line_color.b = 155;
        line_color.a = 100;
        
        fill_color.r = 40;
        fill_color.g = 173;
        fill_color.b = 255;
        fill_color.a = 100;

        {
            draw_line(150, 100, 250, 100, thin_line_color, 1);
            draw_line(150, 300, 250, 300, thin_line_color, 1);
            draw_line(150, 500, 250, 500, thin_line_color, 1);
            
            draw_line(200, 100, 200, 300, line_color, 8);
            draw_line(200, 300, 200, 500, line_color, 8);
        }

        {        
            draw_line(450, 100, 550, 100, thin_line_color, 1);
            draw_line(350, 300, 450, 300, thin_line_color, 1);
            draw_line(450, 500, 550, 500, thin_line_color, 1);
            
            draw_line(500, 100, 400, 300, line_color, 8);
            draw_line(400, 300, 500, 500, line_color, 8);
        }
        
        {
            draw_line(650, 100, 750, 100, thin_line_color, 1);
            draw_line(850, 300, 950, 300, thin_line_color, 1);
            draw_line(1050, 500, 1150, 500, thin_line_color, 1);
            
            draw_arced_corner(700, 100, 200, 0, false, line_color, fill_color, 8);
            draw_arced_corner(1100, 500, 200, 180, true, line_color, fill_color, 8);
            draw_arced_corner(1300, 300, 200, 90, true, line_color, fill_color, 8);
        }

    }
}