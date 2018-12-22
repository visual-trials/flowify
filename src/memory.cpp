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
    i32 bytes_used;
    
    u16 next_block_index;
    u16 previous_block_index;
};

struct MemoryArena
{
    ShortString name;
    
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

void init_memory()
{
    global_memory.nr_of_blocks = NR_OF_MEMORY_BLOCKS;
    global_memory.block_size = global_memory.size / global_memory.nr_of_blocks;
    for (i32 block_index = 0; block_index <  global_memory.nr_of_blocks; block_index++)
    {
        global_memory.blocks_used[block_index] = false;
    }
    global_memory.nr_of_arenas = 0;
}

void free_memory_blocks(i32 block_index, i32 nr_of_blocks)
{
    // TODO: implement this!
    
    // mark all blocks as unused
}

i32 reserve_memory_blocks(i32 nr_of_blocks)
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

i32 reserve_memory_block()
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

extern "C" {

    // Memory
    void set_address_and_size_dynamic_memory(i32 base_address, i32 memory_size)
    {
        global_memory.base_address = (void *)base_address;
        global_memory.size = memory_size;

        init_memory();
    }

}