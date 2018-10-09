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
#define MAX_KEY_SEQUENCE 30

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
    
    u8 key_sequence[MAX_KEY_SEQUENCE];
    
};

world_data global_world;  // FIXME: allocate this properly!

i32 create_new_entity_index(world_data * world)
{
    // TODO: check MAX_ENTITIES!
    return world->nr_of_entities++;
}

extern "C" {
    
    void init_world()
    {
        world_data * world = &global_world;
        
        world->increment = 0;
        world->y_offset = 0;
        world->nr_of_entities = 0;
        world->an_entity_is_selected = false;
        
        world->third_entity_index = create_new_entity_index(world);
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
        
        world->first_entity_index = create_new_entity_index(world);
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
            
        world->second_entity_index = create_new_entity_index(world);
        entity * second_entity = world->entities + world->second_entity_index;
        
        *second_entity = *first_entity;
        
        second_entity->pos.x = 10;
        second_entity->pos.y = 10;
        
        for (i32 sequence_index = 0; sequence_index < MAX_KEY_SEQUENCE; sequence_index++)
        {
            world->key_sequence[sequence_index] = ' ';
        }
    }
    
    void update_frame()
    {
        mouse_input * mouse = &global_input.mouse;
        world_data * world = &global_world;
        
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
        
        entity * first_entity = world->entities + world->first_entity_index;
        first_entity->pos.x = offset + 10;
        
        entity * second_entity = world->entities + world->second_entity_index;
        second_entity->pos.y = offset + 10;
        
        entity * third_entity = world->entities + world->third_entity_index;
        third_entity->pos.y = world->y_offset + 50;
        
        
        // Mouse driven movement/placement
        
        if (mouse->mouse_wheel_has_moved)
        {
            world->y_offset += mouse->mouse_wheel_delta * 10;
        }

        if (mouse->left_mouse_button_has_gone_down)
        {
            // TODO: determine which entity is selected, using positions and sizes of all entities
            
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
            
            current_entity->pos.x = mouse->mouse_position_left;
            current_entity->pos.y = mouse->mouse_position_top;
            
        }
        
        if (mouse->left_mouse_button_has_gone_up)
        {
            world->an_entity_is_selected = true;
        }
        
        if (mouse->left_mouse_button_is_down)
        {
            if (world->an_entity_is_selected)
            {
                entity * current_entity = world->entities + world->selected_entity_index;
                
                current_entity->pos.x = mouse->mouse_position_left;
                current_entity->pos.y = mouse->mouse_position_top;
            }
        }
        
        
        keyboard_input keyboard = global_input.keyboard;
        
        for (i32 frame_sequence_index = 0; frame_sequence_index < keyboard.sequence_keys_length; frame_sequence_index++)
        {
            b32 is_down = (b32)keyboard.sequence_keys_up_down[frame_sequence_index * 2];
            u8 key_code = keyboard.sequence_keys_up_down[frame_sequence_index * 2 + 1];
            if ((key_code >= '0' && key_code <= '9') || (key_code >= 'A' && key_code <= 'Z'))
            {
                if (is_down)
                {
                    // TODO: this is very slow!
                    for (i32 sequence_index = 0; sequence_index < MAX_KEY_SEQUENCE - 1; sequence_index++)
                    {
                        world->key_sequence[sequence_index] = world->key_sequence[sequence_index + 1];
                    }
                    world->key_sequence[MAX_KEY_SEQUENCE - 1] = key_code;
                }
                else
                {
                    // Ignoring up-keys atm
                }
            }
        }
        
        
    }
    
    void draw_keyboard(keyboard_input * keyboard)
    {
        short_string temp_string;
        short_string character;

        color4 font_color;
        font_color.r = 0;
        font_color.g = 0;
        font_color.b = 0;
        font_color.a = 255;

        if (keyboard->ctrl_key_is_down)
        {
            draw_text(400, 180, copy_cstring_to_short_string("Ctrl", &temp_string), 10, font_color);
        }
        
        if (keyboard->alt_key_is_down)
        {
            draw_text(500, 180, copy_cstring_to_short_string("Alt", &temp_string), 10, font_color);
        }
        
        if (keyboard->shift_key_is_down)
        {
            draw_text(600, 180, copy_cstring_to_short_string("Shift", &temp_string), 10, font_color);
        }
        
        for (i32 number_index = 0; number_index < 10; number_index++)
        {
            u8 ch = '0' + number_index;
            copy_char_to_string(ch, &character);
            if (keyboard->keys_that_are_down[ch])
            {
                draw_text(350 + number_index * 10, 150, &character, 10, font_color);
            }
        }
        for (i32 letter_index = 0; letter_index < 26; letter_index++)
        {
            u8 ch = 'A' + letter_index;
            copy_char_to_string(ch, &character);
            if (keyboard->keys_that_are_down[ch])
            {
                draw_text(350 + letter_index * 10, 170, &character, 10, font_color);
            }
        }
       
    }
    
    void draw_sequence (world_data * world)
    {
        short_string character;
        
        color4 font_color;
        font_color.r = 0;
        font_color.g = 0;
        font_color.b = 0;
        font_color.a = 255;
        
        copy_char_to_string(' ', &character);
        for (i32 sequence_index = 0; sequence_index < MAX_KEY_SEQUENCE; sequence_index++)
        {
            copy_char_to_string(world->key_sequence[sequence_index], &character);
            draw_text(320 + sequence_index * 10, 220, &character, 10, font_color);
        }
    }
    
    void render_frame()
    {
        world_data * world = &global_world;

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
        draw_text(200, 340, copy_cstring_to_short_string("My first text!", &temp_string), 10, font_color);
        
        //draw_sequence(world);
        
        keyboard_input * keyboard = &global_input.keyboard;
        
        draw_keyboard(keyboard);
        
    }
}