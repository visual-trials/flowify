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
 
#if BUILD_FOR_NATIVE_PLATFORM
    #include "render_win32.cpp"
#else
    #include "render_js.cpp"
#endif

#include "input.cpp"

// FIXME: mouse position should also be a pos_2d, right?

struct pos_2d
{
    i32 x;
    i32 y;
};

struct size_2d
{
    i32 width;
    i32 height;
};

struct entity
{
    // TODO: shape
    pos_2d  pos;
    size_2d size;
    color4  line_color;
    color4  fill_color;
    i32     line_width;
    
    b32          has_text;
    short_string text;
};

#define MAX_ENTITIES 10

struct world_data
{
    entity entities[MAX_ENTITIES];
    i32 nr_of_entities;
    
    i32 increment;
    
    i32 y_offset;
    
    u32 first_entity_index;
    u32 second_entity_index;
    u32 third_entity_index;
    
    // TODO: create separate camera/user-interaction struct?
    b32 an_entity_is_selected;
    u32 selected_entity_index;
    
};

world_data allocated_world;  // FIXME: allocate this properly!
world_data * world;

i32 create_new_entity_index()
{
    // TODO: check MAX_ENTITIES!
    return world->nr_of_entities++;
}

extern "C" {
    
    void init_world()
    {
        world = &allocated_world;  // FIXME: allocate this properly!
        
        world->increment = 0;
        world->y_offset = 0;
        world->nr_of_entities = 0;
        world->an_entity_is_selected = false;
        
        world->third_entity_index = create_new_entity_index();
        entity * third_entity = world->entities + world->third_entity_index;
        
        third_entity->line_color.r = 255;
        third_entity->line_color.g = 0;
        third_entity->line_color.b = 0;
        third_entity->line_color.a = 255;
        
        third_entity->fill_color.r = 255;
        third_entity->fill_color.g = 255;
        third_entity->fill_color.b = 0;
        third_entity->fill_color.a = 255;
        
        third_entity->line_width = 3;
        
        third_entity->pos.x = 200;
        third_entity->pos.y = world->y_offset + 50;
        
        third_entity->size.width = 40;
        third_entity->size.height = 40;
        
        
        copy_cstring_to_short_string("My", &third_entity->text);
        third_entity->has_text = true;
        
        world->first_entity_index = create_new_entity_index();
        entity * first_entity = world->entities + world->first_entity_index;
        
        first_entity->line_color.r = 0;
        first_entity->line_color.g = 255;
        first_entity->line_color.b = 0;
        first_entity->line_color.a = 255;
        
        first_entity->fill_color.r = 0;
        first_entity->fill_color.g = 0;
        first_entity->fill_color.b = 255;
        first_entity->fill_color.a = 128;
        
        first_entity->line_width = 5;
        
        first_entity->size.width = 100;
        first_entity->size.height = 100;
        
        first_entity->pos.x = 10;
        first_entity->pos.y = 10;
            
        world->second_entity_index = create_new_entity_index();
        entity * second_entity = world->entities + world->second_entity_index;
        
        *second_entity = *first_entity;
        
        second_entity->pos.x = 10;
        second_entity->pos.y = 10;
    }
    
    void update_frame()
    {
        mouse_input mouse = global_input.mouse;
        
        world->increment++;

        i32 offset;
        if (world->increment % 512 < 256) {
            offset = world->increment % 256;
        }
        else {
            offset = 256 - (world->increment % 256);
        }
        
        entity * first_entity = world->entities + world->first_entity_index;
        first_entity->pos.x = offset + 10;
        
        entity * second_entity = world->entities + world->second_entity_index;
        second_entity->pos.y = offset + 10;
        
        entity * third_entity = world->entities + world->third_entity_index;
        third_entity->pos.y = world->y_offset + 50;
        
        
        // Mouse driven movement/placement
        
        if (mouse.mouse_wheel_has_moved)
        {
            world->y_offset += mouse.mouse_wheel_delta * 10;
        }

        if (mouse.left_mouse_button_has_gone_down)
        {
            // TODO: determine which entity is selected, using positions and sizes of all entities
            
            world->selected_entity_index = create_new_entity_index();
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
            
            current_entity->pos.x = mouse.mouse_position_left;
            current_entity->pos.y = mouse.mouse_position_top;
            
        }
        
        if (mouse.left_mouse_button_has_gone_up)
        {
            world->an_entity_is_selected = true;
        }
        
        if (mouse.left_mouse_button_is_down)
        {
            if (world->an_entity_is_selected)
            {
                entity * current_entity = world->entities + world->selected_entity_index;
                
                current_entity->pos.x = mouse.mouse_position_left;
                current_entity->pos.y = mouse.mouse_position_top;
            }
        }
        
    }
    
    void render_frame()
    {
        for (i32 entity_index = 0; entity_index < world->nr_of_entities; entity_index++)
        {
            entity * current_entity = world->entities + entity_index;
            
            draw_rectangle(current_entity->pos.x, current_entity->pos.y + world->y_offset, 
                           current_entity->size.width, current_entity->size.height, 
                           current_entity->line_color, current_entity->fill_color, 
                           current_entity->line_width);
            if (current_entity->has_text)
            {
                color4 font_color;
                font_color.r = 0;
                font_color.g = 0;
                font_color.b = 0;
                font_color.a = 255;
                draw_text(current_entity->pos.x, current_entity->pos.y, &current_entity->text, 10, font_color);
            }
        }
        
        // TODO: use entities with a text-property instead
        
        color4 font_color;
        font_color.r = 0;
        font_color.g = 0;
        font_color.b = 0;
        font_color.a = 255;
        
        short_string temp_string;
        draw_text(200, 240, copy_cstring_to_short_string("My first text!", &temp_string), 10, font_color);
        
        
        if (global_input.keyboard.ctrl_key_is_down) {
            draw_text(400, 180, copy_cstring_to_short_string("Ctrl", &temp_string), 10, font_color);
        }
        
        if (global_input.keyboard.alt_key_is_down) {
            draw_text(500, 180, copy_cstring_to_short_string("Alt", &temp_string), 10, font_color);
        }
        
        if (global_input.keyboard.shift_key_is_down) {
            draw_text(600, 180, copy_cstring_to_short_string("Shift", &temp_string), 10, font_color);
        }
        
        short_string decimal_string;
        if (global_input.keyboard.key_is_down) {
            draw_text(500, 220, int_to_string(global_input.keyboard.key_that_is_down, &decimal_string), 10, font_color);
        }
        
        short_string character;
        character.data[0] = ' ';
        character.length = 1;
        for (i32 letter_index = 0; letter_index < 10; letter_index++)
        {
            u8 ch = '0' + letter_index;
            character.data[0] = ch;
            if (global_input.keyboard.keys_that_are_down[ch])
            {
                draw_text(350 + letter_index * 10, 150, &character, 10, font_color);
            }
        }
        for (i32 letter_index = 0; letter_index < 26; letter_index++)
        {
            u8 ch = 'A' + letter_index;
            character.data[0] = ch;
            if (global_input.keyboard.keys_that_are_down[ch])
            {
                draw_text(350 + letter_index * 10, 170, &character, 10, font_color);
            }
        }
        
    }
}