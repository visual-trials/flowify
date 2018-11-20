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
    b32 recorded_memory;
    i32 total_memory;
    i32 block_size;
    // FIXME: this should not be hardcoded
    b32 memory_usage[512];
    i32 nr_of_memory_blocks;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        world->total_memory = (input->memory.address + input->memory.size);
        world->block_size = 64 * 1024;
        world->nr_of_memory_blocks = world->total_memory / world->block_size;
        
        world->recorded_memory = false;
        
        
        i32 * my_array = (i32*)input->memory.address; // + input->memory.size
        i32 * my_array2 = (i32*)(input->memory.address + 10 * 1024 * 1024); // + input->memory.size
        
        i32 number = 1;
        for (i32 i = 0; i < 10000; i++)
        {
            my_array[i] = number++;
        }
        for (i32 i = 0; i < 10000; i++)
        {
            my_array2[i] = number++;
        }
        
        // TODO: do this (once) when triggered by the user
        for (i32 memory_block_index = 0; memory_block_index < world->nr_of_memory_blocks; memory_block_index++)
        {
            u8 * first_memory_address_in_block = (u8*)(memory_block_index * world->block_size);
            for (i32 memory_index_within_block = 0; memory_index_within_block < world->block_size; memory_index_within_block++)
            {
                if (first_memory_address_in_block[memory_index_within_block])
                {
                    world->memory_usage[memory_block_index] = true;
                }
            }
        }
        world->recorded_memory = true;
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        ShortString address;
        ShortString size;
        int_to_string(input->memory.address, &address);
        int_to_string(input->memory.size, &size);
        
        Font font;
        font.family = Font_Arial;
        font.height = 20;
        
        Color4 black = {0, 0, 0, 255};
        Color4 dark_blue = {0, 0, 100, 255};
        Color4 light_blue = {220, 220, 255, 255};
        Color4 no_color = {};
        
        draw_text((Pos2d){100,100}, &address, font, black);
        draw_text((Pos2d){100,140}, &size, font, black);
        
        for (i32 memory_block_index = 0; memory_block_index < world->nr_of_memory_blocks; memory_block_index++)
        {
            if (world->memory_usage[memory_block_index])
            {
                draw_rectangle((Pos2d){100 + memory_block_index * 2, 400}, (Size2d){2,100}, no_color, dark_blue, 1);
            }
            else {
                draw_rectangle((Pos2d){100 + memory_block_index * 2, 400}, (Size2d){2,100}, no_color, light_blue, 1);
            }
        }
    }
}