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
    String file_lines[1000];
    i32 nr_of_file_lines;
    i32 line_offset;
    
    Font font;
    i32 line_margin;
    
    Pos2d text_start_position;
    i32 bottom_margin;
    i32 nr_of_lines_to_show;
    
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
        
        i32 line_margin = 4;
        world->line_margin = line_margin;
        
        Pos2d start_position = {200, 120};
        world->text_start_position = start_position;

        i32 bottom_margin = 30;
        world->bottom_margin = bottom_margin;
        
        // TODO: we do not know screen size at this point, so we have to calculate that each frame.
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        MouseInput * mouse = &global_input.mouse;
        
        world->nr_of_lines_to_show = (i32)(((f32)input->screen.height - (f32)world->text_start_position.y - world->bottom_margin) / 
                                           ((f32)world->font.height + (f32)world->line_margin));

        if (input->file.file_was_uploaded)
        {
            // FIXME: this might be erased, so the pointer in .data may become invalid. We have to copy the content!
            String file_contents = input->file.file_contents;
            
            i32 file_line_index = 0;
            world->nr_of_file_lines = 1;
            world->file_lines[file_line_index].data = file_contents.data;
            world->file_lines[file_line_index].length = 0;
            
            i32 position = 0;
            i32 start_of_line = 0;
            while (position < file_contents.length)
            {
                char ch = file_contents.data[position++];
                
                if (ch == '\n')
                {
                    // TODO: somewhere we need to remove the newline from either start_of_line or length!
                    world->file_lines[file_line_index].length = (position - 1) - start_of_line;
                    start_of_line = position;
                    
                    // FIXME: limit to length of file_lines[]!
                    file_line_index++;
                    
                    world->file_lines[file_line_index].data = (u8 *)((i32)file_contents.data + start_of_line);
                    world->file_lines[file_line_index].length = 0;
                }
            }
            world->nr_of_file_lines = file_line_index + 1;
            
            // If file has just loaded, show it from the start
            world->line_offset = 0;
        }
        
        if (mouse->wheel_has_moved)
        {
            // TODO: account for a "Mac" mouse! (which has a 'continous' wheel)
            if (mouse->wheel_delta > 0)
            {
                // TODO: limit scrolling! if (world->line_offset < world->nr_of_file_lines)
                world->line_offset -= 2;
            }
            
            if (mouse->wheel_delta < 0)
            {
                // TODO: limit scrolling! if (world->line_offset < world->nr_of_file_lines)
                world->line_offset += 2;
            }
        }
        
        
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        Color4 black = {};
        black.a = 255;
        
        Color4 grey = {};
        grey.a = 100;
        
        Font font = world->font;
        i32 line_margin = world->line_margin;
        i32 nr_of_lines_to_show = world->nr_of_lines_to_show;
        Pos2d start_position = world->text_start_position;
        
        ShortString help_text;
        copy_cstring_to_short_string("Press Ctrl + L to open file", &help_text);
        draw_text((Pos2d){700, 50}, &help_text, font, black);
        
        if (world->nr_of_file_lines > 0)
        {
            ShortString line_nr_text;
            
            // TODO: show file-name and file-length
            // ShortString file_length_text;
            // int_to_string(world->file_length, &file_length_text);
            
            for (i32 line_on_screen_index = 0; line_on_screen_index < nr_of_lines_to_show; line_on_screen_index++)
            {
                i32 file_line_index = world->line_offset + line_on_screen_index;
                
                // CHECK: 0 AND nr_of_file_lines
                
                // TODO: use start_position!
                if (file_line_index >= 0 && file_line_index < world->nr_of_file_lines)
                {
                    Pos2d position = start_position;
                    position.x = start_position.x;
                    position.y = start_position.y + line_on_screen_index * (font.height + line_margin);
                    
                    Pos2d position_line_nr = position;
                    // Line number
                    int_to_string(file_line_index + 1, &line_nr_text);
                    Size2d line_nr_size = get_text_size(&line_nr_text, font);
                    position_line_nr.x -= 10 + line_nr_size.width;
                    draw_text(position_line_nr, &line_nr_text, font, grey);
                    
                    // Line text
                    String line_text = world->file_lines[file_line_index];
                    
                    position.x += 30;
                    draw_text(position, &line_text, font, black);
                }
            }
        }
        
        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
    }
}