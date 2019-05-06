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
    
    ConsecutiveDynamicArray lines;
    
    i32 widest_line;
    
    // TODO: we probably want to use a DynamicArray here too!
    FragmentedMemoryArena highlighted_line_parts_memory_arena;
    HighlightedLinePart * first_highlighted_line_part;
    b32 position_based_on_highlighted_line_parts;
    
    Font font;
    i32 line_margin;
    
    // Note: these are margins within the inside_rect (not in the screen_rect)
    i32 left_margin;
    i32 top_margin;
    i32 bottom_margin;
    i32 right_margin;
    
    b32 draw_line_numbers;
    i32 line_numbers_width;
    
    Window * window;
    
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
void init_scrollable_text(ScrollableText * scrollable_text, Window * window, b32 draw_line_numbers = true)
{
    
    // TODO: get a description and color as parameter and pass it to create_dynamic_array!
    
    init_consecutive_dynamic_array(&scrollable_text->lines, sizeof(String), (Color4){255,100,100,255}, cstring_to_string("Scrollable Text"));
    
    scrollable_text->window = window;
    
    scrollable_text->widest_line = 0;
    
    if (!scrollable_text->highlighted_line_parts_memory_arena.memory)
    {
        scrollable_text->highlighted_line_parts_memory_arena = new_fragmented_memory_arena(&global_memory, (Color4){100,255,100,255}, cstring_to_string("Highlighted parts"));
        scrollable_text->first_highlighted_line_part = 0;
    }
    else
    {
        remove_highlighted_line_parts(scrollable_text);
    }
    scrollable_text->position_based_on_highlighted_line_parts = false;
    
    Font font = {};
    font.height = 20;
    font.family = Font_CourierNew;
    scrollable_text->font = font;
    
    scrollable_text->line_margin = 4;

    // Note: these are margins within the inside_rect (not in the screen_rect)
    scrollable_text->left_margin = 10;
    scrollable_text->top_margin = 10;
    scrollable_text->right_margin = 10;
    scrollable_text->bottom_margin = 10;
    
    window->has_vertical_scrollbar = true;
    window->vertical_scrollbar_width = 15;
    
    window->has_horizontal_scrollbar = false;
    window->horizontal_scrollbar_width = 15;

    ShortString white_space;
    copy_char_to_string(' ', &white_space);
    Size2d white_space_size = get_text_size(&white_space, scrollable_text->font);

    scrollable_text->draw_line_numbers = draw_line_numbers;
    if (draw_line_numbers)
    {
        i32 nr_of_white_space_between_numbers_and_text = 3;
        i32 nr_of_digits_need_for_max_line_nr = 4;  // TODO: we should properly calculate this
        scrollable_text->line_numbers_width = white_space_size.width * (nr_of_digits_need_for_max_line_nr + nr_of_white_space_between_numbers_and_text); 
    }
    else
    {
        scrollable_text->line_numbers_width = 0;
    }
    
    // TODO: call init_window()
    
    // These should be calculated each update
    window->inside_rect.position.x = 0;
    window->inside_rect.position.y = 0;
    
    // Initially the size of the inside rectangle is the same as the screen rectangle
    window->inside_rect.size = window->screen_rect.size;
    window->inside_rect.size = window->screen_rect.size;
}

// TODO: we might want to pass a pointer to the DynamicArray instead (not ScrollableText)
void split_string_into_scrollable_lines(String string, ScrollableText * scrollable_text)
{
    String line_string = {};
    line_string.data = string.data; // first line starts at beginning of the string
    line_string.length = 0;
    
    reset_consecutive_dynamic_array(&scrollable_text->lines);
    
    i32 position = 0;
    i32 start_of_line = 0;
    i32 widest_line = 0;
    while (position < string.length)
    {
        char ch = string.data[position++];
        
        if (ch == '\n')
        {
            // TODO: somewhere we need to remove the newline from either start_of_line or length!
            line_string.length = (position - 1) - start_of_line; // the -1 is because we do not include the newline to the line-text
            
            if (line_string.length > widest_line)
            {
                widest_line = line_string.length;
            }
            
            // We put the line into the array (aka index) of lines (note: the index is dynamically sized)
            add_to_array(&scrollable_text->lines, &line_string);
            
            // Starting a new line string
            start_of_line = position;
            
            line_string.data = (u8 *)((i32)string.data + start_of_line);
            line_string.length = 0;
        }
    }
    line_string.length = position - start_of_line;
    
    if (line_string.length > widest_line)
    {
        widest_line = line_string.length;
    }
    
    add_to_array(&scrollable_text->lines, &line_string);
    
    scrollable_text->widest_line = widest_line;
    
    // TODO: call reset_window ?
    scrollable_text->window->inside_rect.position.x = 0;
    scrollable_text->window->inside_rect.position.y = 0;
}

void update_scrollable_text(ScrollableText * scrollable_text, Input * input)
{
    // Updating scrollable text
    MouseInput * mouse = &input->mouse;
    KeyboardInput * keyboard = &input->keyboard;
    
    Window * window = scrollable_text->window;
    
    u8 sequence_keys_up_down[MAX_KEY_SEQUENCE_PER_FRAME * 2];
    i32 sequence_keys_length;

    // TODO: implement keys_that_have_gone_down[255] in input.c as general way of retrieving this info 
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
    
    // TODO: take into account vertical_scrollbar_width and horizontal_scrollbar_width
    
    i32 line_height = scrollable_text->font.height + scrollable_text->line_margin;
    
    // TODO: use line_height here too
    scrollable_text->nr_of_lines_to_show = (i32)((f32)(window->screen_rect.size.height - scrollable_text->top_margin - scrollable_text->bottom_margin) / 
                                       ((f32)scrollable_text->font.height + (f32)scrollable_text->line_margin));

    ShortString white_space;
    copy_char_to_string(' ', &white_space);
    Size2d white_space_size = get_text_size(&white_space, scrollable_text->font);
    scrollable_text->max_line_width_in_characters = (i32)((f32)(window->screen_rect.size.width - scrollable_text->left_margin - scrollable_text->line_numbers_width - scrollable_text->right_margin) / (f32)white_space_size.width);

    // Size of inside rect
    
    i32 nr_of_characters_in_widest_line = scrollable_text->widest_line;
    i32 inside_width = scrollable_text->left_margin + scrollable_text->line_numbers_width + nr_of_characters_in_widest_line * white_space_size.width + scrollable_text->right_margin;
    i32 inside_height = scrollable_text->top_margin + scrollable_text->lines.nr_of_items * line_height + scrollable_text->bottom_margin;
    window->inside_rect.size.width = inside_width;
    window->inside_rect.size.height = inside_height;

    
    // Positioning of inside rect inside screen_rect
    
    if (scrollable_text->position_based_on_highlighted_line_parts)
    {
        // Position based on highlighted line parts
        
        Pos2d absolute_base_position = {};
        absolute_base_position.x = window->screen_rect.position.x + window->inside_rect.position.x;
        absolute_base_position.y = window->screen_rect.position.y + window->inside_rect.position.y;
        
        i32 min_part_position_y = 30000; // FIXME: we need a MAX number here!
        i32 max_part_position_y = 0;
        
        Font font = scrollable_text->font;
        i32 line_margin = scrollable_text->line_margin;
        
        HighlightedLinePart * line_part = scrollable_text->first_highlighted_line_part;
        while(line_part) 
        {
            i32 x_position_part = (i32)line_part->start_character_index * white_space_size.width;
            i32 x_width_part = (i32)line_part->length * white_space_size.width;
            
            // FIXME: check if line_part is on screen! (vertical AND horizontal!)
            if (true)
            {
                Pos2d part_position_in_inside_rect = {};
                // FIXME: x is not used atm
                part_position_in_inside_rect.x = x_position_part + scrollable_text->left_margin + scrollable_text->line_numbers_width;
                part_position_in_inside_rect.y = scrollable_text->top_margin + line_part->line_index * (font.height + line_margin);
                
                // FIXME: not used atm
                Size2d part_size = {};
                part_size.width = x_width_part;
                part_size.height = font.height + line_margin;
                
                if (part_position_in_inside_rect.y < min_part_position_y)
                {
                    min_part_position_y = part_position_in_inside_rect.y;
                }
                if (part_position_in_inside_rect.y > max_part_position_y)
                {
                    max_part_position_y = part_position_in_inside_rect.y;
                }
                
                // draw_rectangle(position, size, no_color, selected_color, 1);
            }
            
            line_part = line_part->next_highlighted_line_part;
        }
        
        if (scrollable_text->first_highlighted_line_part)
        {
            if (min_part_position_y < -window->inside_rect.position.y + line_height * 3)
            {
                // If the first part is above the inside rect we change the position of the top of the inside rect
                window->inside_rect.position.y = - min_part_position_y + line_height * 3;
            }
            if (min_part_position_y > -window->inside_rect.position.y + scrollable_text->nr_of_lines_to_show * line_height - line_height * 3)
            {
                // If the first part is below the inside rect we change the position of the bottom of the inside rect
                window->inside_rect.position.y = - min_part_position_y + scrollable_text->nr_of_lines_to_show * line_height - line_height * 3;
            }
            
        }
        
    }
    else
    {
        // Mouse
        
        // TODO: put this in update_window()
        if (mouse->wheel_has_moved)
        {
            // TODO: account for a "Mac" mouse! (which has a 'continous' wheel)
            if (mouse->wheel_delta > 0)
            {
                window->inside_rect.position.y += 3 * line_height;
            }
            
            if (mouse->wheel_delta < 0)
            {
                window->inside_rect.position.y -= 3 * line_height;
            }
        }
        
        // Keyboard
        
        if (arrow_down_pressed)
        {
            window->inside_rect.position.y -= 1 * line_height;
        }

        if (arrow_up_pressed)
        {
            window->inside_rect.position.y += 1 * line_height;
        }
        
        if (page_down_pressed)
        {
            window->inside_rect.position.y -= (scrollable_text->nr_of_lines_to_show - 2) * line_height;
        }

        if (page_up_pressed)
        {
            window->inside_rect.position.y += (scrollable_text->nr_of_lines_to_show - 2) * line_height;
        }
    }

    // Check if outside screen_rect
    
    if (window->inside_rect.position.y < window->screen_rect.size.height - window->inside_rect.size.height)
    {
        window->inside_rect.position.y = window->screen_rect.size.height - window->inside_rect.size.height;
    }
    
    if (window->inside_rect.position.y > 0)
    {
        window->inside_rect.position.y = 0;
    }
}

void draw_scrollable_text(ScrollableText * scrollable_text)
{
    
    // TODO: turn line numbers on/off
    // TODO: add scroll bars
    
    Window * window = scrollable_text->window;
    
    clip_rectangle(window->screen_rect.position, window->screen_rect.size);
    
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
    
    Pos2d absolute_base_position = {};
    absolute_base_position.x = window->screen_rect.position.x + window->inside_rect.position.x;
    absolute_base_position.y = window->screen_rect.position.y + window->inside_rect.position.y;
    
    // TODO: this is DEBUG code
    if (false)
    {
        Color4 screen_rect_color = {0, 0, 255, 255};
        Color4 inside_rect_color = {255, 0, 0, 255};
        draw_rectangle(window->screen_rect.position, window->screen_rect.size, screen_rect_color, no_color, 1);
        draw_rectangle(absolute_base_position, window->inside_rect.size, inside_rect_color, no_color, 1);
    }
                
    if (scrollable_text->lines.nr_of_items > 0)
    {
        HighlightedLinePart * line_part = scrollable_text->first_highlighted_line_part;
        while(line_part) 
        {
            i32 x_position_part = (i32)line_part->start_character_index * white_space_size.width;
            i32 x_width_part = (i32)line_part->length * white_space_size.width;
            
            // FIXME: check if line_part is on screen! (vertical AND horizontal!)
            if (true)
            {
                Pos2d position;
                position.x = absolute_base_position.x + x_position_part + scrollable_text->left_margin + scrollable_text->line_numbers_width;
                position.y = absolute_base_position.y + scrollable_text->top_margin + line_part->line_index * (font.height + line_margin);
                
                Size2d size;
                size.width = x_width_part;
                size.height = font.height + line_margin;
                
                draw_rectangle(position, size, no_color, selected_color, 1);
            }
            
            line_part = line_part->next_highlighted_line_part;
        }
        
        ShortString line_nr_text;
        
        String * lines = (String *)scrollable_text->lines.items;
        
        // Note: we are drawing only lines that are on screen (or are close to being on screen)
        
        // Extra -1 and not counting top_margin
        i32 start_line_index = - 1 - window->inside_rect.position.y / (font.height + line_margin);
        if (start_line_index < 0)
        {
            start_line_index = 0;
        }
        // Extra +1 and not counting bottom_margin
        i32 end_line_index = + 1 + (window->screen_rect.size.height - window->inside_rect.position.y) / (font.height + line_margin);
        if (end_line_index > scrollable_text->lines.nr_of_items - 1)
        {
            end_line_index = scrollable_text->lines.nr_of_items - 1;
        }
        
        for (i32 file_line_index = start_line_index; file_line_index <= end_line_index; file_line_index++)
        {
            // Line text
            Pos2d position;
            position.x = absolute_base_position.x + scrollable_text->left_margin + scrollable_text->line_numbers_width;
            position.y = absolute_base_position.y + scrollable_text->top_margin + file_line_index * (font.height + line_margin);
            
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
// FIXME: this is SLOW!
                Size2d line_nr_size = get_text_size(&line_nr_text, font);
                position_line_nr.x -= white_space_size.width * 3 + line_nr_size.width;
                draw_text(position_line_nr, &line_nr_text, font, grey);
            }
        }
        
    }
    
    // TODO: put this in draw_window() or draw_scrollbars()
    
    if (window->has_vertical_scrollbar)
    {
        Rect2d vertical_bar_rect = {}; // TODO: draw the bar in which the scrollbar scrolls
        Rect2d vertical_scrollbar_rect = {};
        
        vertical_scrollbar_rect.size.width = window->vertical_scrollbar_width;
        
        vertical_scrollbar_rect.position = window->screen_rect.position;
        vertical_scrollbar_rect.position.x += window->screen_rect.size.width - vertical_scrollbar_rect.size.width;
        i32 scroll_distance = - window->inside_rect.position.y;
        f32 top_position_vertical_fraction = (f32)scroll_distance / (f32)window->inside_rect.size.height;
        if (top_position_vertical_fraction < 0)
        {
            top_position_vertical_fraction = 0;
        }
        
        f32 height_vertical_fraction = (f32)window->screen_rect.size.height / (f32)window->inside_rect.size.height;
        if (height_vertical_fraction > 1)
        {
            height_vertical_fraction = 1;
        }
        
        vertical_scrollbar_rect.position.y += window->screen_rect.size.height * top_position_vertical_fraction;
        
        vertical_scrollbar_rect.size.height = window->screen_rect.size.height * height_vertical_fraction;
        
        // TODO: put this color in the Window-struct
        Color4 scrollbar_color = {0, 0, 0, 50};
        
        // TODO: not drawing the scrollbar also changes the width!
        if (height_vertical_fraction != 1)
        {
            draw_rectangle(vertical_scrollbar_rect.position, vertical_scrollbar_rect.size, no_color, scrollbar_color, 1);
        }
    }
    
    if (window->has_horizontal_scrollbar)
    {
        // TODO: horizontal_scrollbar_width;
    }
    
    unclip_rectangle();

}
