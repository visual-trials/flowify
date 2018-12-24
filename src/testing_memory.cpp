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
    b32 increased_memory;
    
    MemoryArena * consecutive_memory_arena;
    
    b32 verbose_frame_times;
    b32 verbose_memory_usage;
};

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
        
        MemoryArena * memory_arena = new_memory_arena(memory, false, (Color4){0,255,0,255});
        
        LargeStruct * large_struct = (LargeStruct *)push_struct(memory_arena, sizeof(LargeStruct));

        log_int(sizeof(LargeStruct));
        
        log_int((i32)large_struct);
        
        world->consecutive_memory_arena = new_memory_arena(memory, true, (Color4){200,0,255,255}, 10);
        
        LargeStruct * large_struct2 = (LargeStruct *)push_struct(memory_arena, sizeof(LargeStruct));
        
        log_int((i32)large_struct2);
        
        // reset_memory_arena(memory_arena);
        
        world->verbose_memory_usage = true;
    }
    
    void update_frame()
    {
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        Memory * memory = &global_memory;
        Screen * screen = &input->screen;
        
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
        
        draw_text((Pos2d){100,100}, &address, font, black);
        draw_text((Pos2d){100,140}, &size, font, black);
        draw_text((Pos2d){100,180}, &pointer_length, font, black);
        
        
        // Button to increase memory
        ShortString label;
        copy_char_to_string('=', &label);
        ShortString label_active;
        copy_char_to_string('*', &label_active);
        
        Size2d size_button = {50,50};
        Pos2d position_button = {};
        position_button.x = screen->width - size_button.width - 20;
        position_button.y = 100;
        
        b32 button_is_pressed = do_button(position_button, size_button, &label, world->increased_memory, input, &label_active);
        
        if (button_is_pressed && !world->increased_memory)
        {
            increase_consecutive_memory_blocks(world->consecutive_memory_arena, 20);
            world->increased_memory = true;
        }
        
        // Draw memory usage
        do_memory_usage(memory, input, &world->verbose_memory_usage);
        
        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
    }
}