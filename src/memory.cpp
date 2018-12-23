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

struct MemoryArena;

struct MemoryBlock
{
    // TODO: maybe add a size_in_bytes?
    i32 bytes_used;
    
    MemoryArena * memory_arena; // TODO: do we really want to do point back this way? We might better get the color via the Memory route?
    
    u16 next_block_index;
    u16 previous_block_index;
};

struct Memory;

struct MemoryArena
{
    Memory * memory;
    
    Color4 color;
    
    b32 consecutive_blocks;
    
    u16 current_block_index;
    u16 nr_of_blocks;
};

#define NR_OF_MEMORY_BLOCKS 512

struct Memory
{
    void * base_address;
    i32 size; // Note: right now max memory size is 2GB
    
    b32 blocks_used[NR_OF_MEMORY_BLOCKS];
    MemoryBlock blocks[NR_OF_MEMORY_BLOCKS];
    
    MemoryArena arenas[20]; // TODO: we could visualize memory usage with these
    i32 nr_of_arenas;
    
    i32 nr_of_blocks;
    i32 block_size;
};

Memory global_memory = {};

void init_memory(Memory * memory)
{
    memory->nr_of_blocks = NR_OF_MEMORY_BLOCKS;
    memory->block_size = memory->size / memory->nr_of_blocks;
    for (i32 block_index = 0; block_index <  memory->nr_of_blocks; block_index++)
    {
        memory->blocks_used[block_index] = false;
    }
    memory->nr_of_arenas = 0;
}

void move_block(Memory * memory, u16 from_block_index, u16 to_block_index)
{
    
    memory->blocks_used[to_block_index] = true;
    memory->blocks[to_block_index].bytes_used = memory->blocks[from_block_index].bytes_used;
    memory->blocks[to_block_index].previous_block_index = memory->blocks[from_block_index].previous_block_index;
    memory->blocks[to_block_index].next_block_index = memory->blocks[from_block_index].next_block_index;
    memory->blocks[to_block_index].memory_arena = memory->blocks[from_block_index].memory_arena;
    
    // Copy all block data from the old blocks to the new blocks
    u8 * from_block_address = (u8 *)((i32)memory->base_address + memory->block_size * from_block_index);
    u8 * to_block_address = (u8 *)((i32)memory->base_address + memory->block_size * to_block_index);
    // TODO: copying bytes (u8) is very slow!
    for (i32 byte_index = 0; byte_index < memory->blocks[from_block_index].bytes_used; byte_index++)
    {
        from_block_address[byte_index] = to_block_address[byte_index];
    }
        
    memory->blocks_used[from_block_index] = false;
}

i32 increase_consecutive_memory_blocks(MemoryArena * memory_arena, i32 required_nr_of_blocks)
{
    Memory * memory = memory_arena->memory;
    
    u16 original_block_index = memory_arena->current_block_index;
    u16 original_nr_of_blocks = memory_arena->nr_of_blocks;
        
    u16 found_block_index = 0;
    u16 consecutive_count = 0;
    u16 block_index = 0;
    // Note: we are not using index 0!
    for (block_index = 1; block_index < memory->nr_of_blocks; block_index++)
    {
        if (!memory->blocks_used[block_index])
        {
            if (!found_block_index)
            {
                found_block_index = block_index; // We record the first free block that starts the series of free blocks
            }
            consecutive_count++;
        }
        else
        {
            consecutive_count = 0;
            found_block_index = 0;
        }
        
        if (consecutive_count == required_nr_of_blocks)
        {
            break;
        }
            
    }
    
    if (found_block_index)
    {
        
        u16 previous_block_index = 0;
        for (block_index = found_block_index; block_index < found_block_index + required_nr_of_blocks; block_index++)
        {
            // TODO: maybe put this in a helper function?
            memory->blocks_used[block_index] = true;
            memory->blocks[block_index].bytes_used = memory->block_size; // TODO: is this correct? Shouldn't the last block be partially used?
            memory->blocks[block_index].previous_block_index = previous_block_index;
            memory->blocks[block_index].next_block_index = 0;
            memory->blocks[block_index].memory_arena = memory_arena;
            if (previous_block_index)
            {
                memory->blocks[previous_block_index].next_block_index = block_index;
            }
            previous_block_index = block_index;
        }
        
        // TODO: check if we could simply EXTEND the old blocks!
        b32 was_extend = false;
        
        if (!was_extend)
        {
            // Copy-ing the old data to the new blocks
            u16 to_block_index = found_block_index;
            u16 from_block_index = original_block_index;
            for (u16 from_block_index = original_block_index; from_block_index < original_block_index + original_nr_of_blocks; from_block_index++)
            {
                move_block(memory, from_block_index, to_block_index);
                to_block_index++;
            }
        }
        
        memory_arena->current_block_index = found_block_index;
        memory_arena->nr_of_blocks = required_nr_of_blocks;
        
        return found_block_index;
    }
    else
    {
        // If we didn't found the room for the blocks, we return index = 0
        return 0;
    }

}

void free_consecutive_memory_blocks(Memory * memory, u16 start_block_index, i32 nr_of_blocks)
{
    u16 previous_block_index = 0;
    for (u16 block_index = start_block_index; block_index < start_block_index + nr_of_blocks; block_index++)
    {
        memory->blocks_used[block_index] = false;
        u16 previous_block_index = memory->blocks[block_index].previous_block_index;
        if (previous_block_index)
        {
            memory->blocks[previous_block_index].next_block_index = 0;
        }
    }
}

u16 add_memory_block(MemoryArena * memory_arena)
{
    Memory * memory = memory_arena->memory;

    u16 previous_block_index = memory_arena->current_block_index;
    
    u16 block_index = 0;
    // Note: we are not using index 0!
    for (block_index = 1; block_index < memory->nr_of_blocks; block_index++)
    {
        if (!memory->blocks_used[block_index])
        {
            // TODO: maybe put this in a helper function?
            memory->blocks_used[block_index] = true;
            memory->blocks[block_index].bytes_used = 0;
            memory->blocks[block_index].previous_block_index = previous_block_index;
            memory->blocks[block_index].next_block_index = 0;
            memory->blocks[block_index].memory_arena = memory_arena;
            if (previous_block_index)
            {
                memory->blocks[previous_block_index].next_block_index = block_index;
            }
            
            memory_arena->current_block_index = block_index;
            
            return block_index;
        }
    }
    
    return block_index;
}

void free_memory_block(Memory * memory, u16 block_index)
{
    memory->blocks_used[block_index] = false;
    u16 previous_block_index = memory->blocks[block_index].previous_block_index;
    if (previous_block_index)
    {
        memory->blocks[previous_block_index].next_block_index = 0;
    }    
}

MemoryArena * new_memory_arena(Memory * memory, b32 consecutive_blocks, Color4 color, i32 initial_nr_of_blocks = 1)
{
    // TODO: check bounds!
    MemoryArena * memory_arena = &memory->arenas[memory->nr_of_arenas++];
    
    memory_arena->memory = memory;
    memory_arena->consecutive_blocks = consecutive_blocks;
    memory_arena->color = color;
    memory_arena->current_block_index = 0;
    
    memory_arena->nr_of_blocks = 0; // Only used when consecutive_blocks is true
    if (!memory_arena->consecutive_blocks)
    {
        // We reserve the first block if consecutive_blocks is false
        // TODO: what if new_block_index is 0? (meaning: no more free block)
        // TODO: listen to initial_nr_of_blocks here too?
        u16 new_block_index = add_memory_block(memory_arena);
    }
    else
    {
        increase_consecutive_memory_blocks(memory_arena, initial_nr_of_blocks);
    }
    
    return memory_arena;
}

void reset_memory_arena(MemoryArena * memory_arena)
{
    Memory * memory = memory_arena->memory;
    
    // Note: the memory arena might contain more than one non-consecutive blocks or a sequence of consecutive blocks!
    
    if (!memory_arena->consecutive_blocks)
    {
        u16 current_block_index = memory_arena->current_block_index;
        while (current_block_index)
        {
            MemoryBlock * memory_block = &memory->blocks[current_block_index];
            
            u16 previous_block_index = memory_block->previous_block_index;
            
            free_memory_block(memory, current_block_index);
            
            current_block_index = previous_block_index;
        }
        memory_arena->current_block_index = 0;
    }
    else
    {
        free_consecutive_memory_blocks(memory, memory_arena->current_block_index, memory_arena->nr_of_blocks);
        memory_arena->current_block_index = 0;
        memory_arena->nr_of_blocks = 0;
    }
}

void * push_struct(MemoryArena * memory_arena, i32 size_struct)
{
    Memory * memory = memory_arena->memory;
    
    if (!memory_arena->consecutive_blocks)
    {
        MemoryBlock * memory_block = &memory->blocks[memory_arena->current_block_index];
        if (memory_block->bytes_used + size_struct > memory->block_size)
        {
            // If not enough room in the current block, then get a new block
            u16 new_block_index = add_memory_block(memory_arena);
            memory_block = &memory->blocks[memory_arena->current_block_index];
        }
        
        // TODO: we assume we have a new block! (we should check this, or at least assert it)
        
        // FIXME: alignment!! (only align bytes_used, right?)
        // FIXME: alignment!! (only align bytes_used, right?)
        // FIXME: alignment!! (only align bytes_used, right?)
        void * struct_address = (void *)((i32)memory->base_address + memory->block_size * memory_arena->current_block_index + memory_block->bytes_used);
        memory_block->bytes_used += size_struct;
        
        return struct_address;
    }
    else
    {
        // TODO: implement this!
    }

    return 0;
}

extern "C" {

    // Memory
    void set_address_and_size_dynamic_memory(i32 base_address, i32 memory_size)
    {
        global_memory.base_address = (void *)base_address;
        global_memory.size = memory_size;

        init_memory(&global_memory);
    }

}