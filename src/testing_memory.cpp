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
#include "memory.cpp"
#include "input.cpp"
#include "render.cpp"

struct WorldData
{
    b32 recorded_memory;
};

/*
// BEGIN Usage code
struct FlowElementsArray
{
    FlowElement * flow_elements;
    i32 nr_of_elements;
    
    MemoryArena memory_arena;
}

FlowElement * add_element(FlowElement element, FlowElementsArray flow_elements_array)
{
    // TODO: implement this!
    
    // if not enough room
    //     create new DynamicArrayMemory (double in size)
    //     copy all data to new memory
    //     free all blocks of old one
    // add element to memory (at current index)
}

FlowElement * get_element(FlowElementsArray flow_elements_array, element_index)
{
    return &flow_elements_array.flow_elements[element_index];
}
// END Usage code
*/

WorldData global_world = {};  // FIXME: allocate this properly!

struct LargeStruct
{
    ShortString lots_of_data[400];
};

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        Memory * memory = &global_memory;
        
        world->recorded_memory = false;
        
        MemoryArena * memory_arena = new_memory_arena(memory, false, (Color4){0,255,0,255});
        
        LargeStruct * large_struct = (LargeStruct *)push_struct(memory_arena, sizeof(LargeStruct));

        log_int(sizeof(LargeStruct));
        
        log_int((i32)large_struct);
        
        MemoryArena * consecutive_memory_arena = new_memory_arena(memory, true, (Color4){200,0,255,255}, 10);
        
        LargeStruct * large_struct2 = (LargeStruct *)push_struct(memory_arena, sizeof(LargeStruct));
        
        log_int((i32)large_struct2);
        
        increase_consecutive_memory_blocks(consecutive_memory_arena, 20);
        // reset_memory_arena(memory_arena);
        
        /*
        // Filling with fake memory
        i32 * my_array = (i32 *)memory->base_address; // + memory->size
        i32 * my_array2 = (i32 *)((i32)memory->base_address + 10 * 1024 * 1024); // + memory->size
        
        i32 number = 1;
        for (i32 i = 0; i < 70000; i++)
        {
            my_array[i] = number++;
        }
        for (i32 i = 0; i < 30000; i++)
        {
            my_array2[i] = number++;
        }
        
        // TODO: do this (once) when triggered by the user
        for (i32 memory_block_index = 0; memory_block_index < memory->nr_of_blocks; memory_block_index++)
        {
            u8 * first_memory_address_in_block = (u8*)((i32)memory->base_address + memory_block_index * memory->block_size);
            for (i32 memory_index_within_block = 0; memory_index_within_block < memory->block_size; memory_index_within_block++)
            {
                if (first_memory_address_in_block[memory_index_within_block])
                {
                    memory->blocks_used[memory_block_index] = true;
                }
            }
        }
        */
        
        world->recorded_memory = true;
    }
    
    void update_frame()
    {
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Memory * memory = &global_memory;
        
        ShortString address;
        ShortString size;
        ShortString pointer_length;
        
        int_to_string((i32)memory->base_address, &address);
        int_to_string((i32)memory->size, &size);
        int_to_string(sizeof(void *), &pointer_length);
        
        Font font;
        font.family = Font_Arial;
        font.height = 20;
        
        Color4 black = {0, 0, 0, 255};
        Color4 dark_blue = {0, 0, 100, 255};
        Color4 light_blue = {220, 220, 255, 255};
        Color4 no_color = {};
        
        draw_text((Pos2d){100,100}, &address, font, black);
        draw_text((Pos2d){100,140}, &size, font, black);
        draw_text((Pos2d){100,180}, &pointer_length, font, black);
        
        
        for (i32 memory_block_index = 0; memory_block_index < memory->nr_of_blocks; memory_block_index++)
        {
            if (memory->blocks_used[memory_block_index])
            {
                i32 bytes_used = memory->blocks[memory_block_index].bytes_used;
                i32 block_size = memory->block_size;
                
                i32 percentage_used = (i32)(100 * (f32)((f32)bytes_used / (f32)block_size));
                
                Color4 color = memory->blocks[memory_block_index].memory_arena->color;
                
                draw_rectangle((Pos2d){100 + memory_block_index * 2, 400 + 100 - percentage_used}, (Size2d){2,percentage_used}, no_color, color, 1);
                draw_rectangle((Pos2d){100 + memory_block_index * 2, 400}, (Size2d){2,100 - percentage_used}, no_color, light_blue, 1);
            }
            else {
                draw_rectangle((Pos2d){100 + memory_block_index * 2, 400}, (Size2d){2,100}, no_color, light_blue, 1);
            }
        }
    }
}