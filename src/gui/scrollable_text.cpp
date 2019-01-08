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
 
struct HighlightedLinePart
{
    i32 line_index;
    i32 start_character_index;
    i32 length;
    
    HighlightedLinePart * next_highlighted_line_part;
};

struct ScrollableText
{
    b32 is_active;  // TODO: implement this
    
    DynamicArray lines;
    
    i32 line_offset;
    
    // TODO: we probably want to use a DynamicArray here too!
    FragmentedMemoryArena highlighted_line_parts_memory_arena;
    HighlightedLinePart * first_highlighted_line_part;
    
    Font font;
    i32 line_margin;
    
    i32 left_margin;
    i32 top_margin;
    i32 bottom_margin;
    i32 right_margin;
    
    b32 draw_line_numbers;
    i32 line_numbers_width;
    
    Pos2d position;
    Size2d size;
    
    i32 nr_of_lines_to_show;
    i32 max_line_width_in_characters;
};

HighlightedLinePart * add_new_highlighted_line_part(ScrollableText * scrollable_text)
{
    HighlightedLinePart * new_highlighted_line_part = (HighlightedLinePart *)push_struct(&scrollable_text->highlighted_line_parts_memory_arena, sizeof(HighlightedLinePart));
    
    new_highlighted_line_part->line_index = 0;
    new_highlighted_line_part->start_character_index = 0;
    new_highlighted_line_part->length = 0;
    new_highlighted_line_part->next_highlighted_line_part = 0;
    
    HighlightedLinePart * old_first_line_part = scrollable_text->first_highlighted_line_part;
    new_highlighted_line_part->next_highlighted_line_part = old_first_line_part;
    scrollable_text->first_highlighted_line_part = new_highlighted_line_part;
    
    return new_highlighted_line_part;
}

void remove_highlighted_line_parts(ScrollableText * scrollable_text)
{
    reset_fragmented_memory_arena(&scrollable_text->highlighted_line_parts_memory_arena);
    scrollable_text->first_highlighted_line_part = 0;
}

// TODO: we should probably create a function to de-allocate all memory inside the scrollable_text
void init_scrollable_text(ScrollableText * scrollable_text, b32 draw_line_numbers = true)
{
    
    // TODO: get a description and color as parameter and pass it to create_dynamic_array!
    
    init_dynamic_array(&scrollable_text->lines, sizeof(String), (Color4){255,100,100,255}, cstring_to_string("Scrollable Text"));
    
    scrollable_text->line_offset = 0;
    
    if (!scrollable_text->highlighted_line_parts_memory_arena.memory)
    {
        scrollable_text->highlighted_line_parts_memory_arena = new_fragmented_memory_arena(&global_memory, (Color4){100,255,100,255}, cstring_to_string("Highlighted parts"));
        scrollable_text->first_highlighted_line_part = 0;
    }
    else
    {
        remove_highlighted_line_parts(scrollable_text);
    }
    
    Font font = {};
    font.height = 20;
    font.family = Font_CourierNew;
    scrollable_text->font = font;
    
    scrollable_text->line_margin = 4;

    // TODO: do we want to use nr-of-characters or percentage-of-screen for margins?
    scrollable_text->left_margin = 10;
    scrollable_text->top_margin = 10;
    scrollable_text->right_margin = 10;
    scrollable_text->bottom_margin = 10;
    
    scrollable_text->draw_line_numbers = draw_line_numbers;
    if (draw_line_numbers)
    {
        scrollable_text->line_numbers_width = 120; // TODO: we should properly calculate this
    }
    else
    {
        scrollable_text->line_numbers_width = 0;
    }
    
    // These should be calculated each update
    scrollable_text->position.x = 0;
    scrollable_text->position.y = 0;
    
    scrollable_text->size.width = 100;
    scrollable_text->size.height = 100;
}

// TODO: we might want to pass a pointer to the DynamicArray instead (not ScrollableText)
void split_string_into_scrollable_lines(String string, ScrollableText * scrollable_text)
{
    String line_string = {};
    line_string.data = string.data; // first line starts at beginning of the string
    line_string.length = 0;
    
    reset_dynamic_array(&scrollable_text->lines);
    
    i32 position = 0;
    i32 start_of_line = 0;
    while (position < string.length)
    {
        char ch = string.data[position++];
        
        if (ch == '\n')
        {
            // TODO: somewhere we need to remove the newline from either start_of_line or length!
            line_string.length = (position - 1) - start_of_line; // the -1 is because we do not include the newline to the line-text
            
            // We put the line into the array (aka index) of lines (note: the index is dynamically sized)
            add_to_array(&scrollable_text->lines, &line_string);
            
            // Starting a new line string
            start_of_line = position;
            
            line_string.data = (u8 *)((i32)string.data + start_of_line);
            line_string.length = 0;
        }
    }
    line_string.length = position - start_of_line;
    add_to_array(&scrollable_text->lines, &line_string);
    
    scrollable_text->line_offset = 0;
}

void update_scrollable_text(ScrollableText * scrollable_text, Input * input)
{
    // Updating scrollable text
    MouseInput * mouse = &input->mouse;
    KeyboardInput * keyboard = &input->keyboard;
    
    u8 sequence_keys_up_down[MAX_KEY_SEQUENCE_PER_FRAME * 2];
    i32 sequence_keys_length;

    // TODO: implement keys_that_have_gone_down[255] in input.cpp as general way of retrieving this info 
    //       OR a function that does this loop for one key you want to know (or a set of keys)
    
    // TODO: check if active, only then scroll
    // TODO: activate if hovered or pressed or touched
    // TODO: use touch to scroll (and activate)
    
    b32 arrow_up_pressed = false;
    b32 arrow_down_pressed = false;
    b32 page_up_pressed = false;
    b32 page_down_pressed = false;
    for (i32 sequence_key_index = 0; sequence_key_index < keyboard->sequence_keys_length; sequence_key_index++)
    {
        b32 is_down = (b32)keyboard->sequence_keys_up_down[sequence_key_index * 2];
        u8 key_code = keyboard->sequence_keys_up_down[sequence_key_index * 2 + 1];
        
        if (is_down)
        {
            if (key_code == Key_ArrowUp)
            {
                arrow_up_pressed = true;
            }
            
            if (key_code == Key_ArrowDown)
            {
                arrow_down_pressed = true;
            }
            
            if (key_code == Key_PageUp)
            {
                page_up_pressed = true;
            }
            
            if (key_code == Key_PageDown)
            {
                page_down_pressed = true;
            }
        }
    }
    
    scrollable_text->nr_of_lines_to_show = (i32)((f32)(scrollable_text->size.height - scrollable_text->top_margin - scrollable_text->bottom_margin) / 
                                       ((f32)scrollable_text->font.height + (f32)scrollable_text->line_margin));

    ShortString white_space;
    copy_char_to_string(' ', &white_space);
    Size2d white_space_size = get_text_size(&white_space, scrollable_text->font);
    scrollable_text->max_line_width_in_characters = (i32)((f32)(scrollable_text->size.width - scrollable_text->left_margin - scrollable_text->line_numbers_width - scrollable_text->right_margin) / (f32)white_space_size.width);
        
    if (mouse->wheel_has_moved)
    {
        // TODO: account for a "Mac" mouse! (which has a 'continous' wheel)
        if (mouse->wheel_delta > 0)
        {
            scrollable_text->line_offset -= 3;
        }
        
        if (mouse->wheel_delta < 0)
        {
            scrollable_text->line_offset += 3;
        }
    }
    
    if (arrow_down_pressed)
    {
        scrollable_text->line_offset += 1;
    }

    if (arrow_up_pressed)
    {
        scrollable_text->line_offset -= 1;
    }
    
    if (page_down_pressed)
    {
        scrollable_text->line_offset += scrollable_text->nr_of_lines_to_show - 2;
    }

    if (page_up_pressed)
    {
        scrollable_text->line_offset -= scrollable_text->nr_of_lines_to_show - 2;
    }
    
    if (scrollable_text->line_offset > scrollable_text->lines.nr_of_items - scrollable_text->nr_of_lines_to_show)
    {
        scrollable_text->line_offset = scrollable_text->lines.nr_of_items - scrollable_text->nr_of_lines_to_show;
    }
    
    if (scrollable_text->line_offset < 0)
    {
        scrollable_text->line_offset = 0;
    }
    
}

void draw_scrollable_text(ScrollableText * scrollable_text)
{
    
    // TODO: turn line numbers on/off
    // TODO: add scroll bars
    
    Color4 no_color = {};
    
    Color4 black = {};
    black.a = 255;
    
    Color4 grey = {};
    grey.a = 100;
    
    Color4 selected_color = {180, 255, 180, 255};

    Font font = scrollable_text->font;
    i32 line_margin = scrollable_text->line_margin;
    i32 nr_of_lines_to_show = scrollable_text->nr_of_lines_to_show;
    
    ShortString white_space_text;
    copy_char_to_string(' ', &white_space_text);
    Size2d white_space_size = get_text_size(&white_space_text, font);
    
    if (scrollable_text->lines.nr_of_items > 0)
    {
        HighlightedLinePart * line_part = scrollable_text->first_highlighted_line_part;
        while(line_part) 
        {
            i32 x_position_part = (i32)line_part->start_character_index * white_space_size.width;
            i32 x_width_part = (i32)line_part->length * white_space_size.width;
            
            i32 line_on_screen_index = line_part->line_index - scrollable_text->line_offset;
            // FIXME: check if line_part is on screen! (vertical AND horizontal!)
            if (true)
            {
                
                Pos2d position;
                position.x = x_position_part + scrollable_text->position.x + scrollable_text->left_margin + scrollable_text->line_numbers_width;
                position.y = scrollable_text->position.y + scrollable_text->top_margin + line_on_screen_index * (font.height + line_margin);
                
                Size2d size;
                size.width = x_width_part;
                size.height = font.height + line_margin;
                
                draw_rectangle(position, size, no_color, selected_color, 1);
            }
            
            line_part = line_part->next_highlighted_line_part;
        }
        
        ShortString line_nr_text;
        
        String * lines = (String *)scrollable_text->lines.items;
        
        for (i32 line_on_screen_index = 0; line_on_screen_index < nr_of_lines_to_show; line_on_screen_index++)
        {
            i32 file_line_index = scrollable_text->line_offset + line_on_screen_index;
            
            if (file_line_index >= 0 && file_line_index < scrollable_text->lines.nr_of_items)
            {
                // Line text
                Pos2d position;
                position.x = scrollable_text->position.x + scrollable_text->left_margin + scrollable_text->line_numbers_width;
                position.y = scrollable_text->position.y + scrollable_text->top_margin + line_on_screen_index * (font.height + line_margin);
                
                String line_text = lines[file_line_index];
                
                if (line_text.length > scrollable_text->max_line_width_in_characters)
                {
                    line_text.length = scrollable_text->max_line_width_in_characters;
                }
                draw_text(position, &line_text, font, black);
                
                if (scrollable_text->draw_line_numbers)
                {
                    // Line number
                    Pos2d position_line_nr = position;
                    int_to_string(file_line_index + 1, &line_nr_text);
                    Size2d line_nr_size = get_text_size(&line_nr_text, font);
                    position_line_nr.x -= 40 + line_nr_size.width;
                    draw_text(position_line_nr, &line_nr_text, font, grey);
                }
            }
        }
    }
    
}
