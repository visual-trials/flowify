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

#define MAX_ENTITIES 10

struct Entity
{
    // TODO: shape
    Pos2d pos;
    Size2d size;
    Color4 line_color;
    Color4 fill_color;
    i32 line_width;
    
    b32 has_text;
    ShortString text;
    Font text_font;
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
        
        third_entity->line_color.r = 0;
        third_entity->line_color.g = 0;
        third_entity->line_color.b = 255;
        third_entity->line_color.a = 255;
        
        third_entity->fill_color.r = 150;
        third_entity->fill_color.g = 150;
        third_entity->fill_color.b = 255;
        third_entity->fill_color.a = 255;
        
        third_entity->line_width = 2;
        
        third_entity->pos.x = 400;
        third_entity->pos.y = 30;
        
        third_entity->size.width = 100;
        third_entity->size.height = 50;
        
        copy_cstring_to_short_string("Small text", &third_entity->text);
        third_entity->has_text = true;
        third_entity->text_font.height = 10;
        third_entity->text_font.family = Font_Arial;
        
        world->first_entity_index = world->nr_of_entities++;
        Entity * first_entity = world->entities + world->first_entity_index;
        
        first_entity->line_color.r = 255;
        first_entity->line_color.g = 50;
        first_entity->line_color.b = 255;
        first_entity->line_color.a = 0;
        
        first_entity->fill_color.r = 255;
        first_entity->fill_color.g = 200;
        first_entity->fill_color.b = 255;
        first_entity->fill_color.a = 255;
        
        first_entity->line_width = 1;
        
        first_entity->size.width = 300;
        first_entity->size.height = 300;
        
        first_entity->pos.x = 100;
        first_entity->pos.y = 150;
            
        copy_cstring_to_short_string("This is a much larger text. And won't fit on a single line!", &first_entity->text);
        first_entity->has_text = true;
        first_entity->text_font.height = 20;
        first_entity->text_font.family = Font_CourierNew;
        
        /*
        world->second_entity_index = world->nr_of_entities++;
        Entity * second_entity = world->entities + world->second_entity_index;
        
        *second_entity = *first_entity;
        
        second_entity->pos.x = 10;
        second_entity->pos.y = 10;
        */
    }
    
    void update_frame()
    {
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;

        Color4 no_color = {};
        Color4 red = {};
        red.r = 255;
        red.a = 255;
        
        for (i32 entity_index = 0; entity_index < world->nr_of_entities; entity_index++)
        {
            Entity * current_entity = world->entities + entity_index;
            
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
                
                Size2dFloat text_size = get_text_size_float(&current_entity->text, current_entity->text_font);
                draw_text(current_entity->pos.x, current_entity->pos.y, &current_entity->text, current_entity->text_font, font_color);
                
                draw_rectangle(current_entity->pos.x, current_entity->pos.y, 
                               text_size.width, text_size.height,
                               red, no_color, 1);
                               
                ShortString width_string = {};
                ShortString height_string = {};
                float_to_string(text_size.width, &width_string);
                float_to_string(text_size.height, &height_string);
                draw_text(current_entity->pos.x, current_entity->pos.y + 50, &width_string, current_entity->text_font, font_color);
                draw_text(current_entity->pos.x, current_entity->pos.y + 50 + text_size.height, &height_string, current_entity->text_font, font_color);
                
            }
        }

        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
    }
    
}