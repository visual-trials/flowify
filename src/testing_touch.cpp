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
    
    void draw_touches()
    {
        TouchesInput * touch_info = &global_input.touch;
        ShortString nr_of_touches_string;
        
        Color4 green;
        green.r = 40;
        green.g = 200;
        green.b = 80;
        green.a = 255;
        
        Color4 transparant = {};
        
        Font font = {};
        font.height = 10;
        font.family = Font_Arial;
        
        int_to_string(touch_info->touch_count, &nr_of_touches_string);
        draw_text(500 , 50, &nr_of_touches_string, font, green);
        
        // draw_ellipse(50, 50, 50, 50, green, transparant, 6);
        
        for (i32 touch_index = 0; touch_index < touch_info->touch_count; touch_index++)
        {
            TouchInput touch = touch_info->touches[touch_index];
            draw_ellipse(touch.x - 20, touch.y - 20, 
                           40, 40,
                           green, transparant, 
                           4);
                           
            // Drawing a cross around the touch-point
            
            i32 line_width = 1;
            i32 distance_from_center = 30;
            i32 line_length = 30;
            draw_cross(touch.x, touch.y, distance_from_center, line_length, green, line_width);
        }
    }
    
    void render_frame()
    {
        draw_touches();
    }
    
}