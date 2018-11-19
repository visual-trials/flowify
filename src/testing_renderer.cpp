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
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

const char * program_lines[] = { 
    "number = arg[0]",
    "increased = 0",
    "if (number < 39) {",
    "    number = number + 3",
    "    increased++",
    "}",
    "else {",
    "    number = number - 5",
    "}",
    "print(increased, number)",
};


extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->active_page_index = 1;
        world->nr_of_pages = 3;
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
    
    void draw_lanes()
    {
        Color4 thin_line_color;
        Color4 line_color;
        Color4 fill_color;
        Color4 fill_color2;
        Color4 no_color = {};
        
        thin_line_color.r = 200;
        thin_line_color.g = 200;
        thin_line_color.b = 200;
        thin_line_color.a = 255;
        
        line_color.r = 0;
        line_color.g = 0;
        line_color.b = 0;
        line_color.a = 255;
        
        fill_color.r = 180;
        fill_color.g = 180;
        fill_color.b = 255;
        fill_color.a = 255;
        
        fill_color2.r = 180;
        fill_color2.g = 255;
        fill_color2.b = 180;
        fill_color2.a = 255;
        
        i32 line_width = 4;

        {
            // IDEA: remember the *ENDING* x1, x2 and y of the previous segment and use it
            //       as *STARTING* x1, x2 and y of the next segment.
            
            // Start narrow and do widening
            draw_lane_segment(250, 500, 50, 
                              150, 550, 200, 
                              20, line_color, fill_color, line_width);
            
            // Going left
            draw_lane_segment(150, 400, 200, 
                              150, 350, 240, 
                              20, line_color, fill_color, line_width);
            
            // Extending left
            draw_lane_segment(150, 350, 240, 
                              150, 350, 500, 
                              20, line_color, fill_color, line_width);
                             
            // Going right
            draw_lane_segment(400, 550, 200, 
                              450, 550, 240, 
                              20, line_color, fill_color, line_width);
                              
            // Extending right
            draw_lane_segment(450, 550, 240, 
                              450, 600, 280, 
                              20, line_color, fill_color, line_width);
                              
            // Extending right
            draw_lane_segment(450, 600, 280, 
                              450, 600, 500, 
                              20, line_color, fill_color2, line_width);
                              
            // Left back to middle
            draw_lane_segment(150, 350, 500, 
                              150, 400, 540, 
                              20, line_color, fill_color, line_width);
            
            // Right back to middle
            draw_lane_segment(450, 600, 500, 
                              400, 600, 540, 
                              20, line_color, fill_color2, line_width);
                              
            // Combining left and right
            draw_lane_segment(150, 600, 540, 
                              300, 500, 650, 
                              20, line_color, fill_color, line_width);
            
        }
        
        
        {
            i32 nr_of_program_lines = sizeof(program_lines) / sizeof(char *);
            
            Color4 black = {};
            black.a = 255;
            
            Color4 grey = {};
            grey.a = 100;
            
            Font font = {};
            font.height = 20;
            font.family = Font_CourierNew;

            ShortString line_nr_text;
            ShortString program_line_text;
            
            copy_char_to_string(' ', &program_line_text);
            Size2d white_space_size = get_text_size(&program_line_text, font);
            i32 line_height = white_space_size.height * 1.5;
            
            for (i32 line_index = 0; line_index < nr_of_program_lines; line_index++)
            {
                
                int_to_string(line_index + 1, &line_nr_text);
                Size2d line_nr_size = get_text_size(&line_nr_text, font);
                
                copy_cstring_to_short_string(program_lines[line_index], &program_line_text);
                
                if (line_index == 3 || line_index == 4)
                {
                    Size2d program_line_size = get_text_size(&program_line_text, font);
                    draw_rectangle(
                        750 + white_space_size.width * 3, // FIXME: workaround (there are 4 spaces in front of this line!) 
                        200 + line_index * line_height - white_space_size.height / 4, 
                        program_line_size.width - white_space_size.width * 2, 
                        line_height, 
                        no_color, 
                        fill_color2, 
                        1
                    );
                }
                
                draw_text(710 - line_nr_size.width, 200 + line_index * line_height, &line_nr_text, font, grey);
                draw_text(750, 200 + line_index * line_height, &program_line_text, font, black);
            }
        }
        
    }
    
    void draw_pixel_borders()
    {
        
        // Also see articles about DPI awareness / High DPI screens :
        // In browser: https://www.html5rocks.com/en/tutorials/canvas/hidpi/
        //             https://jsfiddle.net/4JH75
        // Native: https://docs.microsoft.com/en-us/windows/desktop/learnwin32/dpi-and-device-independent-pixels
        //         https://docs.microsoft.com/en-us/windows/desktop/hidpi/high-dpi-desktop-application-development-on-windows
        //         https://technet.microsoft.com/en-us/evalcenter/dn469266(v=vs.90)

        // Below we are trying to figure out if and how adjacent lines/rects behave
        
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
        
        //draw_line(0, 0, 50, 0, red, 1);
        // draw_line(0, 0, 50, 0, red, 1);
        draw_rectangle(0, 0, 2, 2, no_color, black, 1);
        draw_rectangle(2, 2, 3, 3, no_color, black, 1);
        draw_rectangle(10, 0, 2, 2, black, no_color, 1);
        draw_rectangle(12, 2, 3, 3, black, no_color, 1);
        /*
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
        */

        draw_line(0, 900, 50, 900, red, 1);
        
        draw_line(5, 5, 25, 25, black, 1);

        draw_line(5, 10, 5, 25, black, 1);

        ShortString height_text;
        int_to_string(global_input.screen.height, &height_text);

        Font font = {};
        font.height = 10;
        font.family = Font_Arial;

        draw_text(500, 300, &height_text, font, black);
        
        black.a = 100;
        draw_rectangle(100, 0, 50, 50, no_color, black, 1);
        draw_rectangle(150, 40, 50, 50, no_color, black, 1);


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
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        if (world->active_page_index == 0)
        {
            draw_basic_figures();
        }
        else if (world->active_page_index == 1)
        {
            draw_lanes();
        }
        else if (world->active_page_index == 2)
        {
            draw_pixel_borders();
        }
        

        draw_and_update_button_menu(world);
        
        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
    }
    
}
