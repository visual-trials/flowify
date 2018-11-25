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

struct LaneSegment
{
    i32 begin_text_character_index;
    i32 begin_text_line_number;
    
    i32 end_text_character_index;
    i32 end_text_line_number;
    
    Pos2d left_top;
    Pos2d right_top;
    
    Pos2d left_bottom;
    Pos2d right_bottom;
};

const LaneSegment lane_segments[9] = {
  {0,0,13,1,  {250, 50}, {500, 50}, {150, 200}, {550, 200} },  // Start narrow and do widening
  {17,2,17,2, {400, 200}, {550, 200}, {450, 240}, {550, 240} }, // Going right
  {0,3,23,3,  {450, 240}, {550, 240}, {450, 280}, {600, 280} }, // Extending right 1
  {0,4,15,4,  {450, 280}, {600, 280}, {450, 500}, {600, 500} }, // Extending right 2
  {0,5,0,5, {450, 500}, {600, 500}, {400, 540}, {600, 540} }, // Right back to middle
  {0,6,6,6, {150, 200}, {400, 200}, {150, 240}, {350, 240} }, // Going left
  {0,7,13,7, {150, 240}, {350, 240}, {150, 500}, {350, 500} }, // Extending left
  {0,8,0,8, {150, 500}, {350, 500}, {150, 540}, {400, 540} }, // Left back to middle
  {0,9,13,9, {150, 540}, {600, 540}, {300, 650}, {500, 650} }  // Combining left and right
};

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
        Color4 line_color_rect = { 20,  80, 205, 255};
        Color4 fill_color_rect = { 40, 173, 255, 255};
        
        draw_rectangle((Pos2d){200, 100}, (Size2d){400, 300}, line_color_rect, fill_color_rect, 8);
        
        Color4 line_color_ellipse = {120, 140,  55, 255};
        Color4 fill_color_ellipse = {173, 240, 169, 200};
        
        draw_ellipse((Pos2d){550, 20}, (Size2d){400, 300}, line_color_ellipse, fill_color_ellipse, 1);
        
        Color4 line_color_rounded = { 20,  40,  55, 255};
        Color4 fill_color_rounded = { 40,  69, 173, 150};
        
        draw_rounded_rectangle((Pos2d){500, 200}, (Size2d){200, 350}, 20, line_color_rounded, fill_color_rounded, 4);
    }
    
    void draw_lanes()
    {
        
        Color4 line_color       = {  0,   0,   0, 255};
        Color4 unselected_color = {180, 180, 255, 255};
        Color4 selected_color   = {180, 255, 180, 255};
        Color4 no_color         = {};
        
        i32 line_width = 4;

        {
            // IDEA: remember the *ENDING* x1, x2 and y of the previous segment and use it
            //       as *STARTING* x1, x2 and y of the next segment.
            
            // TODO: use array_length(lane_segments)
            i32 lane_segments_count = sizeof(lane_segments)/sizeof(lane_segments[0]); 
            Color4 fill_color;
            for (i32 lane_segment_index = 0; lane_segment_index < lane_segments_count; lane_segment_index++)
            {
                if (lane_segment_index == 2)
                {
                    fill_color = selected_color;
                }
                else {
                    fill_color = unselected_color;
                }
                LaneSegment lane_segment = lane_segments[lane_segment_index];
                draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                                  lane_segment.left_bottom, lane_segment.right_bottom, 
                                  20, line_color, fill_color, line_width);
            }
            
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
            
            Pos2d position = {};
            Size2d size = {};
            
            for (i32 line_index = 0; line_index < nr_of_program_lines; line_index++)
            {
                
                int_to_string(line_index + 1, &line_nr_text);
                Size2d line_nr_size = get_text_size(&line_nr_text, font);
                
                copy_cstring_to_short_string(program_lines[line_index], &program_line_text);
                
                if (line_index == 3 || line_index == 4)
                {
                    Size2d program_line_size = get_text_size(&program_line_text, font);
                    
                    position.x = 750 + white_space_size.width * 3; // FIXME: workaround (there are 4 spaces in front of this line!) 
                    position.y = 200 + line_index * line_height - white_space_size.height / 4;
                    
                    size.width = program_line_size.width - white_space_size.width * 2;
                    size.height = line_height;
                    draw_rectangle(position, size, no_color, selected_color, 1);
                }
                
                draw_text((Pos2d){710 - line_nr_size.width, 200 + line_index * line_height}, &line_nr_text, font, grey);
                draw_text((Pos2d){750, 200 + line_index * line_height}, &program_line_text, font, black);
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
        
        Color4 black = {  0,   0,   0, 255};
        Color4 red =   {255,   0,   0, 255};
        Color4 blue =  {  0,   0, 255, 255};
        Color4 green = {  0, 255,   0, 255};
        Color4 white = {255, 255, 255, 255};
        Color4 no_color = {};
        
        draw_rectangle((Pos2d){0, 0}, (Size2d){2, 2}, no_color, black, 1);
        draw_rectangle((Pos2d){2, 2}, (Size2d){3, 3}, no_color, black, 1);
        draw_rectangle((Pos2d){10, 0}, (Size2d){2, 2}, black, no_color, 1);
        draw_rectangle((Pos2d){12, 2}, (Size2d){3, 3}, black, no_color, 1);

        draw_line((Pos2d){0, 900}, (Pos2d){50, 900}, red, 1);
        
        draw_line((Pos2d){5, 5}, (Pos2d){25, 25}, black, 1);

        draw_line((Pos2d){5, 10}, (Pos2d){5, 25}, black, 1);

        ShortString height_text;
        int_to_string(global_input.screen.height, &height_text);

        Font font = {};
        font.height = 10;
        font.family = Font_Arial;

        draw_text((Pos2d){500, 300}, &height_text, font, black);
        
        black.a = 100;
        draw_rectangle((Pos2d){100, 0}, (Size2d){50, 50}, no_color, black, 1);
        draw_rectangle((Pos2d){150, 40}, (Size2d){50, 50}, no_color, black, 1);


    }
    
    void draw_and_update_button_menu(WorldData * world)
    {
        // Draw (and update) button menu
        
        Size2d size_button = {50, 50};
        Pos2d position_button = {20, 20};
        i32 margin_between_buttons = 20;
        
        for (i32 page_index = 0; page_index < world->nr_of_pages; page_index++)
        {
            b32 button_is_active = false;
            if (page_index == world->active_page_index)
            {
                button_is_active = true;
            }
            
            position_button.y = 20 + page_index * (margin_between_buttons + size_button.height);
            b32 button_is_pressed = do_integer_button(position_button, size_button, page_index + 1, button_is_active, &global_input);
            
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
        do_physical_pixels_switch(&global_input);
    }
    
}
