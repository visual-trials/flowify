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
    
    b32 memory_usage[512];
    i32 nr_of_memory_blocks;
    
};

struct MemoryBlock
{
    i32 bytes_used;
    
    u16 next_block_index;
    u16 previous_block_index;
};

/*

// FIXME: replace current Memory struct with the one below!

struct Memory
{
    void * base_address;
    i32 memory_size; // Note: right now max memory size is 2GB
    
    b32 blocks_used[512];
    MemoryBlock blocks[512];
    
    MemoryArenas[50]; // TODO: we could visualize memory usage with these
    
    i32 nr_of_memory_blocks;
    i32 memory_block_size;
};
*/

struct MemoryArena
{
    ShortString name;
    
    b32 consecutive_blocks;
    
    u16 current_block_index;
    u16 nr_of_blocks;
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

void free_memory_blocks(i32 block_index, i32 nr_of_blocks)
{
    // TODO: implement this!
    
    // mark all blocks as unused
}

i32 get_memory_blocks(i32 nr_of_blocks)
{
    // TODO: implement this!
    
    // set consecutive_count to 0
    // loop through blocks_used (until nr_of_memory_blocks)
    // if consecutive_count ==0: 
    //     if block is not used, remember its index. set consecutive_count to 1
    // if consecutive_count > 0: 
    //     if block is used, set consecutive_count to 0
    //     if block is unused, incrmeent consecutive_count;
    // if consecutive_count == nr_of_blocks:
    //     break and mark all blocks as used (and set set bytes_used to 0)
    //     set next_block_index, previous_block_index
    // return block index
}

i32 get_memory_block()
{
    // TODO: implement this!
    
    // loop through blocks_used (until nr_of_memory_blocks)
    // if block is not used, mark it as used
    // set bytes_used to 0
    // set next_block_index, previous_block_index
    // return block index
}

void free_memory_block(i32 block_index)
{
    // TODO: implement this!
    
    // mark block as unused
}

void * push_struct(MemoryArena * memory_arena, i32 size_struct)
{
    // TODO: implement this!
    
    // see if there is enough room in the current block
    //     if so, advance the 'used' (and align), return the address
    // if not, get a new block (TODO: maybe we should make this a function that takes the MemoryArena?)
    //     set next_block_index and previous_block_index
    //     advance the 'used' (and align?), return the address
}

void free_memory_arena(MemoryArena * memory_arena)
{
    // TODO: implement this!
    
    // Note: the memory arena might contain more than one non-consecutive blocks or consecutive blocks!
    
    // If !consecutive_blocks:
    //     get the current block
    //     get the previous block
    //     free the current block
    //     if previous block exists, repeat
    // If consecutive_blocks:
    //     free all consecutive blocks (and don't look are previous blocks)
}


Memory global_memory;

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        world->total_memory = input->memory.size;
        world->nr_of_memory_blocks = 512;
        world->block_size = input->memory.size / world->nr_of_memory_blocks;
        
        world->recorded_memory = false;

        // Filling with fake memory
        i32 * my_array = (i32 *)input->memory.address; // + input->memory.size
        i32 * my_array2 = (i32 *)((i32)input->memory.address + 10 * 1024 * 1024); // + input->memory.size
        
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
        for (i32 memory_block_index = 0; memory_block_index < world->nr_of_memory_blocks; memory_block_index++)
        {
            u8 * first_memory_address_in_block = (u8*)((i32)input->memory.address + memory_block_index * world->block_size);
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
        ShortString pointer_length;
        
        int_to_string((i32)input->memory.address, &address);
        int_to_string((i32)input->memory.size, &size);
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