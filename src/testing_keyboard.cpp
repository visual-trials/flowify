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
    u8 key_sequence[MAX_KEY_SEQUENCE];
    
    keyboard_layout keyboard_layout;
};

world_data global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        world_data * world = &global_world;
        
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
        world_data * world = &global_world;
        keyboard_input * keyboard = &global_input.keyboard;
        
        for (i32 frame_sequence_index = 0; frame_sequence_index < keyboard->sequence_keys_length; frame_sequence_index++)
        {
            b32 is_down = (b32)keyboard->sequence_keys_up_down[frame_sequence_index * 2];
            u8 key_code = keyboard->sequence_keys_up_down[frame_sequence_index * 2 + 1];
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
    
    void render_frame()
    {
        world_data * world = &global_world;
        input * input = &global_input;
        
        draw_sequence(world);
        
        draw_keyboard(&input->keyboard, &world->keyboard_layout);
    }
    
}