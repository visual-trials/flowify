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

struct WorldData
{
    i32 active_page_index;
    i32 nr_of_pages;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->active_page_index = 0;
        world->nr_of_pages = 2;
    }
    
    void update_frame()
    {
    }
    
    void render_frame()
    {
        Color4 line_color;
        Color4 fill_color;
        
        line_color.r = 20;
        line_color.g = 80;
        line_color.b = 205;
        line_color.a = 255;
        
        fill_color.r = 40;
        fill_color.g = 173;
        fill_color.b = 255;
        fill_color.a = 255;

        draw_rectangle(200, 100, 400, 300, line_color, fill_color, 8);
        
        line_color.r = 120;
        line_color.g = 140;
        line_color.b = 55;
        line_color.a = 255;
        
        fill_color.r = 173;
        fill_color.g = 240;
        fill_color.b = 169;
        fill_color.a = 200;
        
        draw_ellipse(550, 20, 400, 300, line_color, fill_color, 1);
        
        line_color.r = 20;
        line_color.g = 40;
        line_color.b = 55;
        line_color.a = 255;
        
        fill_color.r = 40;
        fill_color.g = 69;
        fill_color.b = 173;
        fill_color.a = 150;
        
        draw_rounded_rectangle(500, 200, 200, 350, 20, line_color, fill_color, 4);

        // Menu
        
        WorldData * world = &global_world;
        
        i32 width_button = 50;
        i32 height_button = 50;
        i32 margin_between_buttons = 20;
        
        for (i32 page_index = 0; page_index < world->nr_of_pages; page_index++)
        {
            b32 button_is_active = false;
            if (page_index == world->active_page_index)
            {
                button_is_active = true;
            }
            b32 button_is_pressed = do_button(20, 20 + page_index * (margin_between_buttons + height_button), width_button, height_button, page_index + 1, button_is_active, &global_input.mouse);
            
            if (button_is_pressed)
            {
                world->active_page_index = page_index;
            }
        }
        
        
    }
    
}