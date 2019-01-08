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
 
#define MAX_ENTITIES 10

struct Entity
{
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
    
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->nr_of_entities = 0;

        // First entity
        
        Entity * first_entity = world->entities + world->nr_of_entities++;
        
        Color4 violet = {255, 50, 255, 255};
        Color4 light_violet = {255, 200, 255, 255};

        first_entity->line_color = violet;
        first_entity->fill_color = light_violet;
        
        first_entity->line_width = 1;
        
        first_entity->size.width = 300;
        first_entity->size.height = 300;
        
        first_entity->pos.x = 100;
        first_entity->pos.y = 150;
            
        copy_cstring_to_short_string("This is a much larger text. And won't fit on a single line!", &first_entity->text);
        first_entity->has_text = true;
        first_entity->text_font.height = 26;
        first_entity->text_font.family = Font_CourierNew;
        
        // Second entity
        
        Entity * second_entity = world->entities + world->nr_of_entities++;
        
        Color4 blue = {0, 0, 255, 255};
        Color4 light_blue = {150, 150, 255, 255};
        
        second_entity->line_color = blue;
        second_entity->fill_color = light_blue;
        
        second_entity->line_width = 2;
        
        second_entity->pos.x = 400;
        second_entity->pos.y = 30;
        
        second_entity->size.width = 100;
        second_entity->size.height = 50;
        
        copy_cstring_to_short_string("Small text", &second_entity->text);
        second_entity->has_text = true;
        second_entity->text_font.height = 13;
        second_entity->text_font.family = Font_Arial;
        
    }
    
    void update_frame()
    {
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;

        Color4 no_color = {};
        Color4 black = {0, 0, 0, 255};
        Color4 red = {255, 0, 0, 255};
        
        for (i32 entity_index = 0; entity_index < world->nr_of_entities; entity_index++)
        {
            Entity * current_entity = world->entities + entity_index;
            
            draw_rectangle(current_entity->pos, current_entity->size,
                           current_entity->line_color, current_entity->fill_color, 
                           current_entity->line_width);
                           
            if (current_entity->has_text)
            {
                Size2dFloat text_size_float = get_text_size_float(&current_entity->text, current_entity->text_font);
                
                Size2d text_size_int = {};
                text_size_int.width = text_size_float.width;
                text_size_int.height = text_size_float.height;
                
                draw_text(current_entity->pos, &current_entity->text, current_entity->text_font, black);
                
                draw_rectangle(current_entity->pos, text_size_int, red, no_color, 1);
                               
                ShortString width_string = {};
                float_to_string(text_size_float.width, &width_string);
                
                ShortString height_string = {};
                float_to_string(text_size_float.height, &height_string);
                
                ShortString height_aspect_string = {};
                float_to_string(text_size_float.height / current_entity->text_font.height, &height_aspect_string);
                
                draw_text((Pos2d){current_entity->pos.x, current_entity->pos.y + 50}, &width_string, current_entity->text_font, black);
                draw_text((Pos2d){current_entity->pos.x, current_entity->pos.y + 50 + text_size_int.height}, &height_string, current_entity->text_font, black);
                draw_text((Pos2d){current_entity->pos.x, current_entity->pos.y + 50 + text_size_int.height * 2}, &height_aspect_string, current_entity->text_font, black);
            }
        }

        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
    }
    
}