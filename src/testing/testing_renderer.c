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

struct WorldData
{
    Rect2d movable_rect;
    

    i32 clip_rectangle;
    i32 show_help_rectangles;
    
    i32 active_page_index;
    i32 nr_of_pages;
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!


extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->active_page_index = 0;
        world->nr_of_pages = 3;
        
        world->show_help_rectangles = true;
        
        world->clip_rectangle = false;
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        
    }
    
    void draw_basic_figures(WorldData * world)
    {
        if (world->clip_rectangle)
        {
            clip_rectangle((Pos2d){250, 70}, (Size2d){500, 350});
        }
        
        Color4 line_color_rect = { 20,  80, 205, 255};
        Color4 fill_color_rect = { 40, 173, 255, 255};
        DrawStyle draw_style_rect = {};
        draw_style_rect.line_color = line_color_rect;
        draw_style_rect.fill_color = fill_color_rect;
        draw_style_rect.line_width = 8;
        
        draw_rectangle((Pos2d){200, 100}, (Size2d){400, 300}, draw_style_rect);
        
        Color4 line_color_ellipse = {120, 140,  55, 255};
        Color4 fill_color_ellipse = {173, 240, 169, 200};
        
        draw_ellipse((Pos2d){550, 20}, (Size2d){400, 300}, line_color_ellipse, fill_color_ellipse, 1);
        
        Color4 line_color_rounded = { 20,  40,  55, 255};
        Color4 fill_color_rounded = { 40,  69, 173, 150};
        DrawStyle draw_style_rounded = {};
        draw_style_rounded.line_color = line_color_rounded;
        draw_style_rounded.fill_color = fill_color_rounded;
        draw_style_rounded.corner_radius = 20;
        draw_style_rounded.line_width = 4;
        
        draw_rounded_rectangle((Pos2d){500, 200}, (Size2d){200, 350}, draw_style_rounded);
        
        if (world->clip_rectangle)
        {
            unclip_rectangle();
        }
        
        // Button for toggling clip rectangle
        {
            Size2d size_button = {50, 50};
            Pos2d position_button = {};
            position_button.x = global_input.screen.width - size_button.width - 20;
            position_button.y = 400;
            
            ShortString label;
            copy_cstring_to_short_string("Clip", &label);
            ShortString label_active;
            copy_cstring_to_short_string("[   ]", &label_active);

            b32 button_is_pressed = do_button(position_button, size_button, &label, world->clip_rectangle, &global_input, &label_active);

            if (button_is_pressed)
            {
                world->clip_rectangle = !world->clip_rectangle;
            }
        }
        
    }
    
    // TODO: make this a more general function (add arguments nr_of_buttons and current_button_index)
    //       then return button_pressed_index (so you can show the correct page for that index)
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
        
        DrawStyle draw_style_filled = {};
        draw_style_filled.line_color = no_color;
        draw_style_filled.fill_color = black;
        draw_style_filled.line_width = 1;
        
        DrawStyle draw_style_border = {};
        draw_style_border.line_color = black;
        draw_style_border.fill_color = no_color;
        draw_style_border.line_width = 1;
        
        draw_rectangle((Pos2d){0, 0}, (Size2d){2, 2}, draw_style_filled);
        draw_rectangle((Pos2d){2, 2}, (Size2d){3, 3}, draw_style_filled);
        draw_rectangle((Pos2d){10, 0}, (Size2d){2, 2}, draw_style_border);
        draw_rectangle((Pos2d){12, 2}, (Size2d){3, 3}, draw_style_border);

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
        draw_rectangle((Pos2d){100, 0}, (Size2d){50, 50}, draw_style_filled);
        draw_rectangle((Pos2d){150, 40}, (Size2d){50, 50}, draw_style_filled);


    }
    
    void draw_example_lanes(WorldData * world, i32 lane_example_index)
    {
        Color4 line_color       = {  0,   0,   0, 255};
        Color4 unselected_color = {180, 180, 255, 255};
        Color4 selected_color   = {180, 255, 180, 255};
        Color4 rectangle_color  = {255, 0, 0, 255};
        Color4 no_color         = {};
        
        i32 line_width = 2;
        i32 bending_radius = 10;
        
        if (lane_example_index == 0)
        {
            Rect2d static_rect = {};
            
            line_width = 4;
            bending_radius = 20;
            
            static_rect.size.width = 150;
            static_rect.size.height = 100;
            static_rect.position.x = 200;
            static_rect.position.y = 100;
            
            if (world->movable_rect.size.width == 0)
            {
                world->movable_rect.size.width = 170;
                world->movable_rect.size.height = 50;
                world->movable_rect.position.x = 150;
                world->movable_rect.position.y = 220;
            }
            
            // TODO: draw lane through static and movable rects
            
            // TODO: make the movable_rect draggable by mouse-click+drag (and sizeable by mouse-scrolling)
            
        }
        
        // Button for toggling showing help rectangles
        {
            Size2d size_button = {50, 50};
            Pos2d position_button = {};
            position_button.x = global_input.screen.width - size_button.width - 20;
            position_button.y = 400;
            
            ShortString label;
            copy_cstring_to_short_string("Help", &label);
            ShortString label_active;
            copy_cstring_to_short_string("[   ]", &label_active);

            b32 button_is_pressed = do_button(position_button, size_button, &label, world->show_help_rectangles, &global_input, &label_active);

            if (button_is_pressed)
            {
                world->show_help_rectangles = !world->show_help_rectangles;
            }
        }
        

    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        if (world->active_page_index == 0)
        {
            draw_basic_figures(world);
        }
        else if (world->active_page_index == 1)
        {
            draw_example_lanes(world, 0);
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
