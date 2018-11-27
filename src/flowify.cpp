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
    u8 * file_content;
    i32 file_length;
    
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->file_length = 0;
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        if (input->file.file_was_uploaded)
        {
            // TODO: expand this!
            world->file_length = input->file.file_contents.length;
            // FIXME: this might be erased, so this pointer may become invalid. We have to copy the content!
            world->file_content = input->file.file_contents.data; 
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
        
        if (world->file_length > 0)
        {
            ShortString file_length_text;
            ShortString file_content_text;
            int_to_string(world->file_length, &file_length_text);
            
            world->file_content[30] = 0;
            copy_cstring_to_short_string((const char *)world->file_content, &file_content_text);
            //copy_char_to_string(world->file_content[0], &file_content_text);
            
            draw_text((Pos2d){700, 100}, &file_length_text, font, black);
            draw_text((Pos2d){700, 120}, &file_content_text, font, black);
        }
    }
}