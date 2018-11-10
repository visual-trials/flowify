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

// FIXME: mouse position should also be a Pos2d, right?

struct Pos2d
{
    i32 x;
    i32 y;
};

struct Size2d
{
    i32 width;
    i32 height;
};

struct entity
{
    // TODO: shape
    Pos2d  pos;
    Size2d size;
    Color4 line_color;
    Color4 fill_color;
    i32 line_width;
    
    b32 has_text;
    ShortString text;
};

#define MAX_ENTITIES 10

struct WorldData
{
    entity entities[MAX_ENTITIES];
    i32 nr_of_entities;
    
    // TODO: create separate camera/user-interaction struct?
    b32 an_entity_is_selected;
    u32 selected_entity_index;
};

WorldData global_world = {};  // FIXME: allocate this properly!

i32 create_new_entity_index(WorldData * world)
{
    // TODO: check MAX_ENTITIES!
    return world->nr_of_entities++;
}

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->an_entity_is_selected = false;
        world->nr_of_entities = 0;
    }
    
    void update_frame()
    {
        MouseInput * mouse = &global_input.mouse;
        WorldData * world = &global_world;
        
        // Mouse driven movement/placement
        
        if (mouse->wheel_has_moved)
        {
            for (i32 entity_index = 0; entity_index < world->nr_of_entities; entity_index++)
            {
                world->entities[entity_index].pos.y += mouse->wheel_delta * 10;
            }
        }

        if (mouse->left_button_has_gone_down)
        {
            // TODO: determine which entity is selected, using positions and sizes of all entities
            
            if (world->nr_of_entities < MAX_ENTITIES)
            {
                world->selected_entity_index = create_new_entity_index(world);
                world->an_entity_is_selected = true;
                
                entity * current_entity = world->entities + world->selected_entity_index;
                
                current_entity->line_color.r = 50;
                current_entity->line_color.g = 50;
                current_entity->line_color.b = 50;
                current_entity->line_color.a = 255;
                
                current_entity->fill_color.r = 240;
                current_entity->fill_color.g = 200;
                current_entity->fill_color.b = 255;
                current_entity->fill_color.a = 255;
                
                current_entity->line_width = 2;
                
                current_entity->size.width = 150;
                current_entity->size.height = 150;
                
                current_entity->pos.x = mouse->x;
                current_entity->pos.y = mouse->y;
            }
            
        }
        
        if (mouse->left_button_has_gone_up)
        {
            world->an_entity_is_selected = true;
        }
        
        if (mouse->left_button_is_down)
        {
            if (world->an_entity_is_selected)
            {
                entity * current_entity = world->entities + world->selected_entity_index;
                
                current_entity->pos.x = mouse->x - current_entity->size.width / 2;
                current_entity->pos.y = mouse->y - current_entity->size.height / 2;
            }
        }
        
        
    }
    
    void render_frame()
    {
        MouseInput * mouse = &global_input.mouse;
        WorldData * world = &global_world;

        // Drawing all entities
        
        for (i32 entity_index = 0; entity_index < world->nr_of_entities; entity_index++)
        {
            entity * current_entity = world->entities + entity_index;
            
            draw_rectangle(current_entity->pos.x, current_entity->pos.y, 
                           current_entity->size.width, current_entity->size.height,
                           current_entity->line_color, current_entity->fill_color, 
                           current_entity->line_width);
            if (current_entity->has_text)
            {
                Color4 font_color;
                font_color.r = 0;
                font_color.g = 0;
                font_color.b = 0;
                font_color.a = 255;
                draw_text(current_entity->pos.x, current_entity->pos.y, &current_entity->text, 10, font_color);
            }
        }
        
        // Drawing a cross around the mouse-pointer
        
        Color4 line_color;
        line_color.r = 0;
        line_color.g = 0;
        line_color.b = 100;
        line_color.a = 255;
        
        i32 line_width = 1;
        
        i32 distance_from_center = 30;
        i32 line_length = 30;
        
        draw_cross(mouse->x, mouse->y, distance_from_center, line_length, line_color, line_width);
        
    }
}