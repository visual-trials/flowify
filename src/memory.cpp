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

struct MemoryBlock
{
    // TODO: maybe add a size_in_bytes?
    i32 bytes_used;
    
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

// TODO: should this be called: reserve_consecutive_memory_blocks?
i32 reserve_memory_blocks(Memory * memory, i32 nr_of_blocks)
{
    // TODO: implement this!
    
    // set consecutive_count to 0
    // loop through blocks_used (until nr_of_memory_blocks)
    // if consecutive_count == 0: 
    //     if block is not used, remember its index. set consecutive_count to 1
    // if consecutive_count > 0: 
    //     if block is used, set consecutive_count to 0
    //     if block is unused, incrmeent consecutive_count;
    // if consecutive_count == nr_of_blocks:
    //     break and mark all blocks as used (and set set bytes_used to 0)
    //     set next_block_index, previous_block_index
    // return block index
}

void free_memory_blocks(i32 block_index, i32 nr_of_blocks)
{
    // TODO: implement this!
    
    // mark all blocks as unused
}

u16 reserve_memory_block(Memory * memory, u16 previous_block_index)
{
    u16 block_index = 0;
    // Note: we are not using index 0!
    for (block_index = 1; block_index < memory->nr_of_blocks; block_index++)
    {
        if (!memory->blocks_used[block_index])
        {
            memory->blocks_used[block_index] = true;
            memory->blocks[block_index].bytes_used = 0;
            memory->blocks[block_index].previous_block_index = previous_block_index;
            memory->blocks[block_index].next_block_index = 0;
            if (previous_block_index)
            {
                memory->blocks[previous_block_index].next_block_index = block_index;
            }
            
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

MemoryArena * new_memory_arena(Memory * memory, b32 consecutive_blocks, Color4 color)
{
    // TODO: check bounds!
    MemoryArena * memory_arena = &memory->arenas[memory->nr_of_arenas++];
    
    memory_arena->memory = memory;
    memory_arena->consecutive_blocks = consecutive_blocks;
    memory_arena->color = color;
    
    memory_arena->nr_of_blocks = 0; // Only used when consecutive_blocks is true
    if (!memory_arena->consecutive_blocks)
    {
        // We reserve the first block if consecutive_blocks is false
        // TODO: what if current_block_index is 0? (meaning: no more free block)
        memory_arena->current_block_index = reserve_memory_block(memory, 0);
    }
    else
    {
        // TODO: whould we reserve blocks it consecutive_blocks is false
    }
    
    return memory_arena;
}

void free_blocks_in_arena(MemoryArena * memory_arena)
{
    Memory * memory = memory_arena->memory;
    
    // Note: the memory arena might contain more than one non-consecutive blocks or a sequence of consecutive blocks!
    
    if (!memory_arena->consecutive_blocks)
    {
        u16 current_block_index = memory_arena->current_block_index;
        while (current_block_index)
        {
            MemoryBlock * memory_block = &memory->blocks[memory_arena->current_block_index];
            
            u16 previous_block_index = memory_block->previous_block_index;
            
            free_memory_block(memory, current_block_index);
            
            current_block_index = previous_block_index;
        }
        memory_arena->current_block_index = 0;
    }
    else
    {
        // TODO: implement this!
        //     free all consecutive blocks (and don't look are previous blocks)
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
            // Ff not enough room in the current block, then get a new block
            u16 new_block_index = reserve_memory_block(memory, memory_arena->current_block_index);
            memory_arena->current_block_index = new_block_index;
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