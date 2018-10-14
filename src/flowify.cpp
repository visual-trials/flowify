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

#define MAX_KEYS_PER_KEYBOARD_ROW 20
#define MAX_ROWS_PER_KEYBOARD 6

struct keyboard_key
{
    u8 key_code;
    short_string key_name;  // TODO: this is memory-intensive!
    i32 width;
};

struct keyboard_row
{
    keyboard_key keys[MAX_KEYS_PER_KEYBOARD_ROW];
    i32 nr_of_keys;
    i32 height;
};

struct keyboard_layout
{
    keyboard_row rows[MAX_ROWS_PER_KEYBOARD];
    i32 nr_of_rows;
};

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
    
    keyboard_layout keyboard_layout;
};

world_data global_world = {};  // FIXME: allocate this properly!

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

        
        short_string rows[MAX_ROWS_PER_KEYBOARD];
        i32 row_heights[MAX_ROWS_PER_KEYBOARD];
        
        i32 nr_of_rows = 6;
        
        // TODO: how to deal with other keys here!? Maybe use an enum with their names iterated below? And their values represent the keyCodes?
        u8 row0[] = { Key_Escape, Key_Invisible, Key_F1, Key_F2, Key_F3, Key_F4, Key_Invisible, Key_F5, Key_F6, Key_F7, Key_F8, Key_Invisible, Key_F9, Key_F10, Key_F11, Key_F12, 0};
        u8 row1[] = { Key_Backtick, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', Key_Minus, Key_Equals, Key_Backspace, 0};
        u8 row2[] = { Key_Tab, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', Key_OpeningSquareBracket, Key_ClosingSquareBracket, Key_BackSlash, 0};
        u8 row3[] = { Key_CapsLock, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', Key_SemiColon, Key_SingleQuote, Key_Enter, 0};
        u8 row4[] = { Key_Shift, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', Key_Comma, Key_Period, Key_ForwardSlash, Key_Shift, 0};
        u8 row5[] = { Key_Control, Key_RightMeta, Key_Alt, Key_Space, Key_Alt, Key_RightMeta, Key_Clear, Key_Control, 0};
        
        copy_cstring_to_short_string((const char*)row0, &rows[0]);
        copy_cstring_to_short_string((const char*)row1, &rows[1]);
        copy_cstring_to_short_string((const char*)row2, &rows[2]);
        copy_cstring_to_short_string((const char*)row3, &rows[3]);
        copy_cstring_to_short_string((const char*)row4, &rows[4]);
        copy_cstring_to_short_string((const char*)row5, &rows[5]);
        
        const char * key_names[255] = {};

		key_names[Key_Invisible] = "";
        
		key_names[Key_Escape] = "Esc";
		key_names[Key_F1] = "F1";
		key_names[Key_F2] = "F2";
		key_names[Key_F3] = "F3";
		key_names[Key_F4] = "F4";
		key_names[Key_F5] = "F5";
		key_names[Key_F6] = "F6";
		key_names[Key_F7] = "F7";
		key_names[Key_F8] = "F8";
		key_names[Key_F9] = "F9";
		key_names[Key_F10] = "F10";
		key_names[Key_F11] = "F11";
		key_names[Key_F12] = "F12";
        
		key_names[Key_Backtick] = "`";
		key_names[Key_Minus] = "-";
        key_names[Key_Equals] = "=";
        key_names[Key_Backspace] = "Backspace";
        
		key_names[Key_Tab] = "Tab";
		key_names[Key_OpeningSquareBracket] = "[";
		key_names[Key_ClosingSquareBracket] = "]";
		key_names[Key_BackSlash] = "\\";
        
		key_names[Key_CapsLock] = "Capslock";
		key_names[Key_SemiColon] = ";";
		key_names[Key_SingleQuote] = "'";
		key_names[Key_Enter] = "Enter";
        
		key_names[Key_Shift] = "Shift";
		key_names[Key_LeftShift] = "Shift";
		key_names[Key_RightShift] = "Shift";
		key_names[Key_Comma] = ",";
		key_names[Key_Period] = ".";
		key_names[Key_ForwardSlash] = "/";
        
		key_names[Key_Control] = "Ctrl";
		key_names[Key_Alt] = "Alt";
		key_names[Key_LeftMeta] = "_";
		key_names[Key_RightMeta] = "_";
		key_names[Key_Clear] = "";
        
        i32 key_widths[255] = {};

        key_widths[Key_Invisible] = 27;
        
        key_widths[Key_Backspace] = 80;
        
        key_widths[Key_Tab] = 60;
        key_widths[Key_BackSlash] = 60;
        
        key_widths[Key_CapsLock] = 70;
        key_widths[Key_Enter] = 90;
        
        key_widths[Key_Shift] = 100;
        key_widths[Key_LeftShift] = 100;
        key_widths[Key_RightShift] = 100;
        
        key_widths[Key_Control] = 60;
        key_widths[Key_Alt] = 50;
        key_widths[Key_Space] = 250;
        key_widths[Key_LeftMeta] = 40;
        key_widths[Key_RightMeta] = 40;
        key_widths[Key_Clear] = 50;
        
        row_heights[0] = 30;
        row_heights[1] = 40;
        row_heights[2] = 40;
        row_heights[3] = 40;
        row_heights[4] = 40;
        row_heights[5] = 40;
        
        for (i32 row_index = 0; row_index < nr_of_rows; row_index++)
        {
            i32 row_length = rows[row_index].length;
            for (i32 column_index = 0; column_index < row_length; column_index++)
            {
                keyboard_key * key = &world->keyboard_layout.rows[row_index].keys[column_index];
                key->key_code = rows[row_index].data[column_index];
                if (key_names[key->key_code])
                {
                    copy_cstring_to_short_string(key_names[key->key_code], &key->key_name);
                }
                else 
                {
                    copy_char_to_string(key->key_code, &key->key_name);
                }
                
                if (key_widths[key->key_code]) 
                {
                    key->width = key_widths[key->key_code];
                }
                else
                {
                    key->width = 40; // default
                }
            }
            world->keyboard_layout.rows[row_index].nr_of_keys = row_length;
            world->keyboard_layout.rows[row_index].height = row_heights[row_index];
        }
        world->keyboard_layout.nr_of_rows = nr_of_rows;
        
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
    
    void draw_keyboard(keyboard_input * keyboard, keyboard_layout * keyboard_layout)
    {
        short_string temp_string;
        short_string character;

        color4 black;
        black.r = 0;
        black.g = 0;
        black.b = 0;
        black.a = 255;
        
        color4 white;
        white.r = 255;
        white.g = 255;
        white.b = 255;
        white.a = 255;

        i32 y_row = 150;
        for (i32 row_index = 0; row_index < keyboard_layout->nr_of_rows; row_index++)
        {
            i32 row_height = keyboard_layout->rows[row_index].height;
            
            i32 x_column = 350;
            for (i32 column_index = 0; column_index < keyboard_layout->rows[row_index].nr_of_keys; column_index++)
            {
                u8 key_code = keyboard_layout->rows[row_index].keys[column_index].key_code;
                short_string key_name = keyboard_layout->rows[row_index].keys[column_index].key_name;
                i32 key_width = keyboard_layout->rows[row_index].keys[column_index].width;
                
                i32 x = x_column;
                i32 y = y_row;
                if (key_code != Key_Invisible)
                {
                    i32 text_margin = 10;
                    if (row_height < 40)
                    {
                        text_margin = 5;
                    }
                    if (keyboard->keys_that_are_down[key_code])
                    {
                        draw_rounded_rectangle(x, y, key_width - 10, row_height - 10, 3, black, black, 1);
                        draw_text(x + text_margin, y + text_margin, &key_name, 10, white);
                    }
                    else {
                        draw_rounded_rectangle(x, y, key_width - 10, row_height - 10, 3, black, white, 1);
                        draw_text(x + text_margin, y + text_margin, &key_name, 10, black);
                    }
                }
                
                x_column += key_width;
            }
            
            y_row += row_height;
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
            draw_text(450 + sequence_index * 10, 100, &character, 10, font_color);
        }
        
    }
    
    void draw_touches()
    {
        touches_input * touch_info = &global_input.touch;
        short_string nr_of_touches_string;
        
        color4 green;
        green.r = 40;
        green.g = 200;
        green.b = 80;
        green.a = 255;
        
        color4 transparant = {};
        
        // int_to_string(touch_info->touch_count, &nr_of_touches_string);
        // draw_text(500 , 50, &nr_of_touches_string, 10, green);
        
        for (i32 touch_index = 0; touch_index < touch_info->touch_count; touch_index++)
        {
            touch_input touch = touch_info->touches[touch_index];
            draw_ellipse(touch.position_left - 25, touch.position_top - 25, 
                           50, 50,
                           green, transparant, 
                           6);
        }
    }
    
    void render_frame()
    {
        world_data * world = &global_world;
        input * input = &global_input;

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
        
        draw_sequence(world);
        
        draw_keyboard(&input->keyboard, &world->keyboard_layout);
        
        draw_touches();
        
    }
}