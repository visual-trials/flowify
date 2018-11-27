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
    
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->nr_of_file_lines = 0;
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
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
        }
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        Color4 black = {};
        black.a = 255;
        
        Font font = {};
        font.height = 20;
        font.family = Font_CourierNew;
        
        i32 line_margin = 4;
        
        ShortString help_text;
        copy_cstring_to_short_string("Press Ctrl + L to open file", &help_text);
        draw_text((Pos2d){700, 50}, &help_text, font, black);
        
        if (world->nr_of_file_lines > 0)
        {
            // TODO: show file-name and file-length
            // ShortString file_length_text;
            // int_to_string(world->file_length, &file_length_text);
            
            for (i32 file_line_index = 0; file_line_index < world->nr_of_file_lines; file_line_index++)
            {
                String line_text = world->file_lines[file_line_index];
                draw_text((Pos2d){700, 120 + file_line_index * (font.height + line_margin)}, &line_text, font, black);
            }
        }
    }
}