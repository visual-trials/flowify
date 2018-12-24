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

#define MAX_ENTITIES 10

struct Entity
{
    // TODO: shape
    Pos2d pos;
    Size2d size;
    Color4 line_color;
    Color4 fill_color;
    i32 line_width;
};

struct WorldData
{
    Entity entities[MAX_ENTITIES];
    i32 nr_of_entities;
    
    i32 increment;
    
    u32 first_entity_index;
    u32 second_entity_index;
    u32 third_entity_index;
    
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->increment = 0;
        world->nr_of_entities = 0;
        
        world->third_entity_index = world->nr_of_entities++;
        Entity * third_entity = world->entities + world->third_entity_index;
        
        Color4 black =  {  0,   0,   0, 255};
        Color4 red =    {255,   0,   0, 255};
        Color4 blue =   {  0,   0, 255, 150};
        Color4 green =  {  0, 255,   0, 255};
        Color4 yellow = {255, 255,   0, 255};
        Color4 no_color = {};

        // First entity
        
        world->first_entity_index = world->nr_of_entities++;
        Entity * first_entity = world->entities + world->first_entity_index;
        
        first_entity->line_color = green;
        first_entity->fill_color = blue;
        
        first_entity->line_width = 5;
        
        first_entity->size.width = 100;
        first_entity->size.height = 100;
        
        first_entity->pos.x = 10;
        first_entity->pos.y = 10;
            
        // Second entity
            
        world->second_entity_index = world->nr_of_entities++;
        Entity * second_entity = world->entities + world->second_entity_index;
        
        *second_entity = *first_entity;
        
        second_entity->pos.x = 10;
        second_entity->pos.y = 10;

        // Third entity
        
        third_entity->line_color = red;
        third_entity->fill_color = yellow;
        
        third_entity->line_width = 3;
        
        third_entity->pos.x = 200;
        third_entity->pos.y = 50;
        
        third_entity->size.width = 40;
        third_entity->size.height = 40;
        
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        
        world->increment++;

        i32 offset;
        if (world->increment % 512 < 256)
        {
            offset = world->increment % 256;
        }
        else
        {
            offset = 256 - (world->increment % 256);
        }
        
        Entity * first_entity = world->entities + world->first_entity_index;
        first_entity->pos.x = offset + 10;
        
        Entity * second_entity = world->entities + world->second_entity_index;
        second_entity->pos.y = offset + 10;
        
        Entity * third_entity = world->entities + world->third_entity_index;
        third_entity->pos.y = 50;
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;

        for (i32 entity_index = 0; entity_index < world->nr_of_entities; entity_index++)
        {
            Entity * current_entity = world->entities + entity_index;
            
            draw_rectangle(current_entity->pos, current_entity->size,
                           current_entity->line_color, current_entity->fill_color, 
                           current_entity->line_width);
        }
        
        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
    }
    
}