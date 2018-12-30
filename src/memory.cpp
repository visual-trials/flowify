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

#define NR_OF_MEMORY_BLOCKS 512

struct MemoryBlock
{
    i32 bytes_used;

    // Note: later on, we might want to keep a record of the memory arenas (using MemoryArenaType-enum and a pointer)
    
    Color4 color;
    String description;
    
    u16 next_block_index;
    u16 previous_block_index;
};

struct Memory
{
    void * base_address;
    i32 size; // Note: right now max memory size is 2GB
    
    b32 blocks_used[NR_OF_MEMORY_BLOCKS];
    MemoryBlock blocks[NR_OF_MEMORY_BLOCKS];
    
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
}

void * memory_copy(void * destination, void * source, i32 nr_of_bytes)
{
    // TODO: copying byte-by-byte is slow. copy using u32/u64 instead.
    u8 * destination_bytes = (u8 *)destination;
    u8 * source_bytes = (u8 *)source;
    for (i32 bytes_copied = 0; bytes_copied < nr_of_bytes; bytes_copied++)
    {
        destination_bytes[bytes_copied] = source_bytes[bytes_copied];
    }
    return destination;
}

void move_memory_block(Memory * memory, u16 from_block_index, u16 to_block_index)
{
    memory->blocks_used[to_block_index] = true;
    memory->blocks[to_block_index].bytes_used = memory->blocks[from_block_index].bytes_used;
    memory->blocks[to_block_index].previous_block_index = memory->blocks[from_block_index].previous_block_index;
    memory->blocks[to_block_index].next_block_index = memory->blocks[from_block_index].next_block_index;
    memory->blocks[to_block_index].color = memory->blocks[from_block_index].color;
    memory->blocks[to_block_index].description = memory->blocks[from_block_index].description;
    
    // Copy all block data from the old blocks to the new blocks
    u8 * from_block_address = (u8 *)((i32)memory->base_address + memory->block_size * from_block_index);
    u8 * to_block_address = (u8 *)((i32)memory->base_address + memory->block_size * to_block_index);
    
    memory_copy(to_block_address, from_block_address, memory->blocks[from_block_index].bytes_used);
        
    memory->blocks_used[from_block_index] = false;
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

// Consecutive Memory Arena

struct ConsecutiveMemoryArena
{
    Memory * memory;
    
    Color4 color;
    ShortString description;
    
    u16 first_block_index;
    u16 nr_of_blocks;
};

u16 increase_consecutive_memory_blocks(ConsecutiveMemoryArena * memory_arena, i32 required_nr_of_blocks);

ConsecutiveMemoryArena new_consecutive_memory_arena(Memory * memory, Color4 color, String description, i32 initial_nr_of_blocks)
{
    ConsecutiveMemoryArena memory_arena = {};
    
    memory_arena.memory = memory;
    memory_arena.color = color;
    copy_string(description, &memory_arena.description);
    memory_arena.first_block_index = 0;
    
    memory_arena.nr_of_blocks = 0;
    increase_consecutive_memory_blocks(&memory_arena, initial_nr_of_blocks);
    
    return memory_arena;
}

u16 increase_consecutive_memory_blocks(ConsecutiveMemoryArena * memory_arena, i32 required_nr_of_blocks)
{
    Memory * memory = memory_arena->memory;
    
    u16 original_block_index = memory_arena->first_block_index;
    u16 original_nr_of_blocks = memory_arena->nr_of_blocks;
        
    u16 found_block_index = 0;
    u16 candidate_block_index = 0;
    u16 consecutive_count = 0;
    // Note: we are not using block index 0!
    for (u16 block_index = 1; block_index < memory->nr_of_blocks; block_index++)
    {
        if (!memory->blocks_used[block_index])
        {
            if (!candidate_block_index)
            {
                candidate_block_index = block_index; // We record the first free block that starts the series of free blocks
            }
            consecutive_count++;
        }
        else
        {
            consecutive_count = 0;
            candidate_block_index = 0;
        }
        
        if (consecutive_count == required_nr_of_blocks)
        {
            found_block_index = candidate_block_index;
            break;
        }
            
    }
    
    if (found_block_index)
    {
        
        u16 previous_block_index = 0;
        for (u16 block_index = found_block_index; block_index < found_block_index + required_nr_of_blocks; block_index++)
        {
            memory->blocks_used[block_index] = true;
            
            // TODO: is this correct? Shouldn't the last block be partially used?
            memory->blocks[block_index].bytes_used = memory->block_size; 
            memory->blocks[block_index].previous_block_index = previous_block_index;
            memory->blocks[block_index].next_block_index = 0;
            memory->blocks[block_index].color = memory_arena->color;
            memory->blocks[block_index].description = shortstring_to_string(&memory_arena->description);
            if (previous_block_index)
            {
                memory->blocks[previous_block_index].next_block_index = block_index;
            }
            previous_block_index = block_index;
        }
        
        // TODO: check if we could simply EXTEND the old blocks!
        b32 was_extended = false;
        
        if (!was_extended)
        {
            // Copy-ing the old data to the new blocks
            u16 to_block_index = found_block_index;
            u16 from_block_index = original_block_index;
            for (u16 from_block_index = original_block_index; from_block_index < original_block_index + original_nr_of_blocks; from_block_index++)
            {
                move_memory_block(memory, from_block_index, to_block_index);
                to_block_index++;
            }
        }
        
        memory_arena->first_block_index = found_block_index;
        memory_arena->nr_of_blocks = required_nr_of_blocks;
        
        return found_block_index;
    }
    else
    {
        // If we didn't found the room for the blocks, we return index = 0
        return 0;
    }

}

void free_consecutive_memory_blocks(ConsecutiveMemoryArena * memory_arena)
{
    Memory * memory = memory_arena->memory;
    
    u16 first_block_index = memory_arena->first_block_index;
    i32 nr_of_blocks = memory_arena->nr_of_blocks;
    
    u16 previous_block_index = 0;
    for (u16 block_index = first_block_index; block_index < first_block_index + nr_of_blocks; block_index++)
    {
        memory->blocks_used[block_index] = false;
        u16 previous_block_index = memory->blocks[block_index].previous_block_index;
        if (previous_block_index)
        {
            memory->blocks[previous_block_index].next_block_index = 0;
        }
    }
}

void reset_consecutive_memory_arena(ConsecutiveMemoryArena * memory_arena, i32 initial_nr_of_blocks = 0)
{
    Memory * memory = memory_arena->memory;
    
    free_consecutive_memory_blocks(memory_arena);
    memory_arena->first_block_index = 0;
    memory_arena->nr_of_blocks = 0;
    increase_consecutive_memory_blocks(memory_arena, initial_nr_of_blocks);
}

// Fragmented Memory Arena

struct FragmentedMemoryArena
{
    Memory * memory;
    
    Color4 color;
    ShortString description;
    
    u16 last_block_index;
};

u16 add_memory_block(FragmentedMemoryArena * memory_arena);

FragmentedMemoryArena new_fragmented_memory_arena(Memory * memory, Color4 color, String description, b32 create_initial_block = false)
{
    FragmentedMemoryArena memory_arena = {};
    
    memory_arena.memory = memory;
    memory_arena.color = color;
    copy_string(description, &memory_arena.description);
    memory_arena.last_block_index = 0;
    
    if (create_initial_block)
    {
        // TODO: what if new_block_index is 0? (meaning: no more free block)
        u16 new_block_index = add_memory_block(&memory_arena);
    }
    
    return memory_arena;
}

u16 add_memory_block(FragmentedMemoryArena * memory_arena)
{
    Memory * memory = memory_arena->memory;

    u16 last_block_index = memory_arena->last_block_index;
    
    u16 block_index = 0;
    // Note: we are not using block index 0!
    for (block_index = 1; block_index < memory->nr_of_blocks; block_index++)
    {
        if (!memory->blocks_used[block_index])
        {
            memory->blocks_used[block_index] = true;
            memory->blocks[block_index].bytes_used = 0;
            memory->blocks[block_index].previous_block_index = last_block_index;
            memory->blocks[block_index].next_block_index = 0;
            memory->blocks[block_index].color = memory_arena->color;
            memory->blocks[block_index].description = shortstring_to_string(&memory_arena->description);
            if (last_block_index)
            {
                memory->blocks[last_block_index].next_block_index = block_index;
            }
            
            memory_arena->last_block_index = block_index;
            
            return block_index;
        }
    }
    
    return block_index;
}

void * push_struct(FragmentedMemoryArena * memory_arena, i32 size_struct)
{
    Memory * memory = memory_arena->memory;
    
    MemoryBlock * memory_block = 0;
    if (!memory_arena->last_block_index || memory->blocks[memory_arena->last_block_index].bytes_used + size_struct > memory->block_size)
    {
        // If not enough room in the current block (or no block present), then get a new block
        add_memory_block(memory_arena);
    }
    memory_block = &memory->blocks[memory_arena->last_block_index];
    
    // TODO: we assume we have a new block! (we should check this, or at least assert it)
    
    void * struct_address = (void *)((i32)memory->base_address + memory->block_size * memory_arena->last_block_index + memory_block->bytes_used);
    memory_block->bytes_used += size_struct;
    
    // Add alignment offset (if needed)
    i32 alignment = 4;
    i32 alignment_mask = alignment - 1; // 0x3
    if (memory_block->bytes_used & alignment_mask) // meaning: if it isn't dividable by 4
    {
        memory_block->bytes_used += alignment - (memory_block->bytes_used & alignment_mask);
    }
    
    return struct_address;
}

void reset_fragmented_memory_arena(FragmentedMemoryArena * memory_arena, b32 create_initial_block = false)
{
    Memory * memory = memory_arena->memory;
    
    u16 current_block_index = memory_arena->last_block_index;
    while (current_block_index)
    {
        MemoryBlock * memory_block = &memory->blocks[current_block_index];
        
        u16 previous_block_index = memory_block->previous_block_index;
        
        free_memory_block(memory, current_block_index);
        
        current_block_index = previous_block_index;
    }
    
    memory_arena->last_block_index = 0;
    
    if (create_initial_block)
    {
        // TODO: what if new_block_index is 0? (meaning: no more free block)
        u16 new_block_index = add_memory_block(memory_arena);
    }
}

// Dynamic Array

struct DynamicArray
{
    void * items;
    i32 nr_of_items;
    i32 item_size; // nr of bytes per item
    
    ConsecutiveMemoryArena memory_arena;
};

DynamicArray create_dynamic_array(i32 item_size, Color4 color, String description)
{
    DynamicArray dynamic_array = {};
    
    // TODO: we should check if the item_size is not larger than the block_size!
    dynamic_array.item_size = item_size;
    
    // TODO: maybe we want to reserve 1 block or allow an initial amount of items and reserve memory for those
    dynamic_array.memory_arena = new_consecutive_memory_arena(&global_memory, color, description, 0);
    
    dynamic_array.nr_of_items = 0;
    dynamic_array.items = 0;  // TODO: this needs to be set if memory is reserved (now nothing is reserved)
    
    return dynamic_array;
}

void reset_dynamic_array(DynamicArray * dynamic_array)
{
    // TODO: maybe we want to reserve 1 block or allow an initial amount of items and reserve memory for those
    reset_consecutive_memory_arena(&dynamic_array->memory_arena);
    
    dynamic_array->nr_of_items = 0;
    dynamic_array->items = 0;  // TODO: this needs to be set if memory is reserved (now nothing is reserved)
}

void init_dynamic_array(DynamicArray * dynamic_array, i32 item_size, Color4 color, String description)
{
    if (!dynamic_array->memory_arena.memory)
    {
        *dynamic_array = create_dynamic_array(item_size, color, description);
    }
    else
    {
        reset_dynamic_array(dynamic_array);
    }
}

void * add_to_array(DynamicArray * dynamic_array, void * item)
{
    ConsecutiveMemoryArena * memory_arena = &dynamic_array->memory_arena;
    Memory * memory = memory_arena->memory;
    
    i32 available_memory_size = memory_arena->nr_of_blocks * memory->block_size;
    i32 required_memory_size = dynamic_array->item_size * (dynamic_array->nr_of_items + 1);
    if (required_memory_size > available_memory_size)
    {
        // TODO: maybe its better to give increase_consecutive_memory_blocks the nr_of_required_bytes instead of the nr_of_required_blocks?
        // TODO: and also let it return the pointer to the start of the first block
        i32 required_nr_of_blocks = (i32)((f32)required_memory_size / (f32)memory->block_size) + 1; // first round down, then add one
        increase_consecutive_memory_blocks(memory_arena, required_nr_of_blocks);
        
        void * items = (void *)((i32)memory->base_address + memory->block_size * memory_arena->first_block_index);
        
        dynamic_array->items = items;
    }
    
    // TODO: should we increase .bytes_used on the memory block?
    
    void * destination = (void *)((i32)dynamic_array->items + (dynamic_array->nr_of_items * dynamic_array->item_size));
    memory_copy(destination, item, dynamic_array->item_size);
    dynamic_array->nr_of_items++;
    
    return destination;
}

// Dynamic String

struct DynamicString
{
    String string; // Contains .data (u8 *) and .length (i32)
    
    ConsecutiveMemoryArena memory_arena;
    i32 max_length;
};

DynamicString create_dynamic_string(Color4 color, String description)
{
    DynamicString dynamic_string = {};
    
    // TODO: maybe we want to reserve 1 block or allow an initial amount of items and reserve memory for those
    dynamic_string.memory_arena = new_consecutive_memory_arena(&global_memory, color, description, 0); 
    
    dynamic_string.string.length = 0;
    dynamic_string.string.data = 0;  // TODO: this needs to be set if memory is reserved (now nothing is reserved)
    
    return dynamic_string;
}

void reset_dynamic_string(DynamicString * dynamic_string)
{
    // TODO: maybe we want to reserve 1 block or allow an initial amount of items and reserve memory for those
    reset_consecutive_memory_arena(&dynamic_string->memory_arena);
    
    dynamic_string->string.length = 0;
    dynamic_string->string.data = 0;  // TODO: this needs to be set if memory is reserved (now nothing is reserved)
}

void init_dynamic_string(DynamicString * dynamic_string, Color4 color, String description)
{
    if (!dynamic_string->memory_arena.memory)
    {
        *dynamic_string = create_dynamic_string(color, description);
    }
    else
    {
        reset_dynamic_string(dynamic_string);
    }
}

void append_string(DynamicString * dynamic_string, String string)
{
    ConsecutiveMemoryArena * memory_arena = &dynamic_string->memory_arena;
    Memory * memory = memory_arena->memory;
    
    i32 available_memory_size = memory_arena->nr_of_blocks * memory->block_size;
    i32 required_memory_size = dynamic_string->string.length + string.length;
    if (required_memory_size > available_memory_size)
    {
        // TODO: maybe its better to give increase_consecutive_memory_blocks the nr_of_required_bytes instead of the nr_of_required_blocks?
        // TODO: and also let it return the pointer to the start of the first block
        i32 required_nr_of_blocks = (i32)((f32)required_memory_size / (f32)memory->block_size) + 1; // first round down, then add one
        increase_consecutive_memory_blocks(memory_arena, required_nr_of_blocks);
        
        u8 * data = (u8 *)((i32)memory->base_address + memory->block_size * memory_arena->first_block_index);
        
        dynamic_string->string.data = data;
    }
    
    // TODO: should we increase .bytes_used on the memory block?
    
    // TODO: maybe use the existing append_string(String * dest, String * src) ?
    u8 * destination = (u8 *)((i32)dynamic_string->string.data + dynamic_string->string.length);
    memory_copy(destination, string.data, string.length);
    dynamic_string->string.length += string.length;
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