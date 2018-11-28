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
    i32 file_length;
    String file_name;
    
    String file_lines[1000];
    i32 nr_of_file_lines;
    i32 line_offset;
    
    Font font;
    i32 line_margin;
    
    i32 left_margin;
    i32 top_margin;
    i32 bottom_margin;
    i32 right_margin;
    
    i32 nr_of_lines_to_show;
    i32 max_line_width_in_characters;
    
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        world->nr_of_file_lines = 0;
        world->line_offset = 0;
        
        Font font = {};
        font.height = 20;
        font.family = Font_CourierNew;
        world->font = font;
        
        world->line_margin = 4;

        // TODO: do we want to use nr-of-characters or percentage-of-screen for margins?
        world->left_margin = 130;
        world->top_margin = 120;
        world->right_margin = 10;
        world->bottom_margin = 10;
        
        // TODO: we do not know screen size at this point, so we have to calculate that each frame.
    }
        
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        MouseInput * mouse = &global_input.mouse;
        KeyboardInput * keyboard = &global_input.keyboard;
        
        u8 sequence_keys_up_down[MAX_KEY_SEQUENCE_PER_FRAME * 2];
        i32 sequence_keys_length;

        // TODO: implement keys_that_have_gone_down[255] in input.cpp as general way of retrieving this info 
        //       OR a function that does this loop for one key you want to know (or a set of keys)
        
        b32 arrow_up_pressed = false;
        b32 arrow_down_pressed = false;
        b32 page_up_pressed = false;
        b32 page_down_pressed = false;
        for (i32 frame_sequence_index = 0; frame_sequence_index < keyboard->sequence_keys_length; frame_sequence_index++)
        {
            b32 is_down = (b32)keyboard->sequence_keys_up_down[frame_sequence_index * 2];
            u8 key_code = keyboard->sequence_keys_up_down[frame_sequence_index * 2 + 1];
            
            if (is_down)
            {
                if (key_code == Key_ArrowUp)
                {
                    arrow_up_pressed = true;
                }
                
                if (key_code == Key_ArrowDown)
                {
                    arrow_down_pressed = true;
                }
                
                if (key_code == Key_PageUp)
                {
                    page_up_pressed = true;
                }
                
                if (key_code == Key_PageDown)
                {
                    page_down_pressed = true;
                }
            }
        }
        
        world->nr_of_lines_to_show = (i32)(((f32)input->screen.height - (f32)world->top_margin - world->bottom_margin) / 
                                           ((f32)world->font.height + (f32)world->line_margin));

        ShortString white_space;
        copy_char_to_string(' ', &white_space);
        Size2d white_space_size = get_text_size(&white_space, world->font);
        world->max_line_width_in_characters = (i32)((f32)(input->screen.width - world->left_margin - world->right_margin) / (f32)white_space_size.width);
            
        if (input->file.file_was_uploaded)
        {
            // FIXME: this might be erased, so the pointer in .data may become invalid. We have to copy the content!
            String file_contents = input->file.file_contents;
            world->file_length = file_contents.length;
            world->file_name = input->file.file_name;
            
            world->nr_of_file_lines = split_string_into_lines(file_contents, world->file_lines);
            
            // If file has just loaded, show it from the start
            world->line_offset = 0;
        }
        
        if (mouse->wheel_has_moved)
        {
            // TODO: account for a "Mac" mouse! (which has a 'continous' wheel)
            if (mouse->wheel_delta > 0)
            {
                // TODO: limit scrolling! if (world->line_offset < world->nr_of_file_lines)
                world->line_offset -= 3;
            }
            
            if (mouse->wheel_delta < 0)
            {
                // TODO: limit scrolling! if (world->line_offset < world->nr_of_file_lines)
                world->line_offset += 3;
            }
        }
        
        if (arrow_down_pressed)
        {
            world->line_offset += 1;
        }

        if (arrow_up_pressed)
        {
            world->line_offset -= 1;
        }
        
        if (page_down_pressed)
        {
            world->line_offset += world->nr_of_lines_to_show - 2;
        }

        if (page_up_pressed)
        {
            world->line_offset -= world->nr_of_lines_to_show - 2;
        }
        
        if (world->line_offset > world->nr_of_file_lines - world->nr_of_lines_to_show)
        {
            world->line_offset = world->nr_of_file_lines - world->nr_of_lines_to_show;
        }
        
        if (world->line_offset < 0)
        {
            world->line_offset = 0;
        }
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        Color4 black = {};
        black.a = 255;
        
        Color4 grey = {};
        grey.a = 100;
        
        Font font = world->font;
        i32 line_margin = world->line_margin;
        i32 nr_of_lines_to_show = world->nr_of_lines_to_show;
        
        ShortString help_text;
        copy_cstring_to_short_string("Press Ctrl + L to open file", &help_text);
        Size2d help_text_size = get_text_size(&help_text, world->font);
        i32 help_text_x = ((f32)input->screen.width / (f32)2) - ((f32)help_text_size.width / (f32)2);
        draw_text((Pos2d){help_text_x, 50}, &help_text, font, black);
        
        Size2d file_name_size = get_text_size(&world->file_name, world->font);
        i32 file_name_x = ((f32)input->screen.width / (f32)2) - ((f32)file_name_size.width / (f32)2);
        draw_text((Pos2d){file_name_x, 80}, &world->file_name, font, black);
        
        if (world->nr_of_file_lines > 0)
        {
            ShortString line_nr_text;
            
            // TODO: show file-name and file-length
            // ShortString file_length_text;
            // int_to_string(world->file_length, &file_length_text);
            // draw_text((Pos2d){10,10}, &file_length_text, font, black);
            
            for (i32 line_on_screen_index = 0; line_on_screen_index < nr_of_lines_to_show; line_on_screen_index++)
            {
                i32 file_line_index = world->line_offset + line_on_screen_index;
                
                if (file_line_index >= 0 && file_line_index < world->nr_of_file_lines)
                {
                    // Line text
                    Pos2d position;
                    position.x = world->left_margin;
                    position.y = world->top_margin + line_on_screen_index * (font.height + line_margin);
                    
                    String line_text = world->file_lines[file_line_index];
                    if (line_text.length > world->max_line_width_in_characters)
                    {
                        line_text.length = world->max_line_width_in_characters;
                    }
                    draw_text(position, &line_text, font, black);
                    
                    // Line number
                    Pos2d position_line_nr = position;
                    int_to_string(file_line_index + 1, &line_nr_text);
                    Size2d line_nr_size = get_text_size(&line_nr_text, font);
                    position_line_nr.x -= 40 + line_nr_size.width;
                    draw_text(position_line_nr, &line_nr_text, font, grey);
                }
            }
        }
        
        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
        do_physical_pixels_switch(&global_input);
    }
}