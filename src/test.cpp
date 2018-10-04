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
 
#include "render.h"

extern "C" { 
    extern void jsLog(u8 * text_data, i32 text_length);    
    extern void jsLogInt(i32 log_integer);
}

void log(u8 * text)
{
    jsLog(text, string_length(text));
}

void draw_rectangle(color4 line_color, color4 fill_color)
{
    jsLogInt(line_color.a);
}

extern "C" {
    
    void init_world()
    {
    }
    
    void update_frame()
    {
        log((u8*)"First");
        log((u8*)"Second");
        log((u8*)"Third");
        log((u8*)"Fourth");
    }
    
    void render_frame()
    {
        color4 line_color;
        line_color.r = 255;
        line_color.g = 0;
        line_color.b = 0;
        line_color.a = 255;
        
        color4 fill_color;
        fill_color.r = 255;
        fill_color.g = 255;
        fill_color.b = 0;
        fill_color.a = 255;

        for (i32 entity_index = 0; entity_index < 1; entity_index++)
        {
            draw_rectangle(line_color, fill_color);
        }
    }
}