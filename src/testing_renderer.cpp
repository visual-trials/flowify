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
        
        world->active_page_index = 3;
        world->nr_of_pages = 4;
    }
    
    void update_frame()
    {
    }
    
    void draw_basic_figures()
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
    }
    
    void draw_arced_corners()
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
    
    void draw_lanes()
    {
        Color4 thin_line_color;
        Color4 line_color;
        Color4 fill_color;
        Color4 no_color = {};
        
        thin_line_color.r = 200;
        thin_line_color.g = 200;
        thin_line_color.b = 200;
        thin_line_color.a = 255;
        
        line_color.r = 0;
        line_color.g = 0;
        line_color.b = 0;
        line_color.a = 255;
        
        fill_color.r = 200;
        fill_color.g = 200;
        fill_color.b = 200;
        fill_color.a = 255;
        
        i32 line_width = 4;

        {
            draw_rectangle(320, 100, 20, 20, no_color, fill_color, line_width);
            draw_arced_corner(300, 100, 20, 90, true, line_color, fill_color, line_width);
            draw_arced_corner(320, 120, 20, 180, true, line_color, fill_color, line_width);
            draw_arced_corner(320, 120, 20, 0, false, line_color, fill_color, line_width);
            draw_arced_corner(260, 140, 20, 270, false, line_color, fill_color, line_width);
        }
        
    }
    
    void draw_and_update_button_menu(WorldData * world)
    {
        // Draw (and update) button menu
        
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
            b32 button_is_pressed = do_button(20, 20 + page_index * (margin_between_buttons + height_button), width_button, height_button, page_index + 1, button_is_active, &global_input.mouse, &global_input.touch);
            
            if (button_is_pressed)
            {
                world->active_page_index = page_index;
            }
        }
        
    }
    
    void draw_pixel_borders()
    {
        
        // See articles about DPI awareness / High DPI screens :
        
        // In browser: https://www.html5rocks.com/en/tutorials/canvas/hidpi/
        // Native: https://docs.microsoft.com/en-us/windows/desktop/learnwin32/dpi-and-device-independent-pixels
        
        Color4 black = {};
        Color4 red = {};
        Color4 blue = {};
        Color4 green = {};
        Color4 white = {};
        Color4 no_color = {};
        
        black.a = 255;
        
        red.r = 255;
        red.a = 255;
        
        blue.b = 255;
        blue.a = 255;
        
        green.g = 255;
        green.a = 255;
        
        white.r = 255;
        white.g = 255;
        white.b = 255;
        white.a = 255;
        
        draw_line(0, 0, 50, 0, red, 1);
        draw_line(10, 1, 60, 1, blue, 1);
        draw_line(20, 2, 70, 2, green, 1);
        draw_line(30, 3, 80, 3, white, 1);
        draw_line(40, 4, 90, 4, red, 1);
        draw_line(50, 5, 250, 5, black, 1);
        
        draw_line(0, 500, 50, 500, red, 1);
        draw_line(10, 501, 60, 501, blue, 1);
        draw_line(20, 502, 70, 502, green, 1);
        draw_line(30, 503, 80, 503, white, 1);
        draw_line(40, 504, 90, 504, red, 1);
        draw_line(50, 505, 250, 505, black, 1);

        ShortString height_text;
        int_to_string(global_input.screen.height, &height_text);

        draw_text(500, 300, &height_text, 10, black);
        
        black.a = 100;
        draw_rectangle(100, 0, 50, 50, no_color, black, 1);
        draw_rectangle(150, 40, 50, 50, no_color, black, 1);


    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        if (world->active_page_index == 0)
        {
            draw_basic_figures();
        }
        else if (world->active_page_index == 1)
        {
            draw_arced_corners();
        }
        else if (world->active_page_index == 2)
        {
            draw_lanes();
        }
        else if (world->active_page_index == 3)
        {
            draw_pixel_borders();
        }
        

        draw_and_update_button_menu(world);
        
        // Draw frame timing
        draw_frame_timing(&global_input.timing, &global_input.screen);
    }
    
}