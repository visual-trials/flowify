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
 
// NOTE: always keep this in sync with my.fontFamilies in canvas.js and font_families in win32/render.cpp!
enum FontFamily
{
    Font_Arial = 0x00,
    Font_CourierNew = 0x01
};

struct Font
{
    FontFamily family;
    i32 height;  // font.height is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
};

#if BUILD_FOR_NATIVE_PLATFORM
    #include "win32/render.cpp"
#else
    #include "browser/render.cpp"
#endif

void draw_lane_segments_for_3_rectangles(Rect2d top_rect, Rect2d middle_rect, Rect2d bottom_rect, i32 bending_radius, i32 line_width, Color4 line_color, Color4 rect_color, Color4 bend_color)
{
    Color4 no_color = {};
    
    LaneSegment3 lane_segments = get_3_lane_segments_from_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius);
    
    LaneSegment lane_segment = {};
    
    if (lane_segments.has_valid_top_segment)
    {
        lane_segment = lane_segments.top;
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                          lane_segment.left_bottom, lane_segment.right_bottom, 
                          lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                          line_color, rect_color, line_width);
    }
    
    lane_segment = lane_segments.middle;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, rect_color, line_width);

    if (lane_segments.has_valid_bottom_segment)
    {
        lane_segment = lane_segments.bottom;
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                          lane_segment.left_bottom, lane_segment.right_bottom, 
                          lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                          line_color, bend_color, line_width);
    }
    
}

void draw_lane_segments_for_4_rectangles(Rect2d top_or_bottom_rect, b32 is_top_rect, Rect2d left_rect, Rect2d right_rect, Rect2d middle_rect, i32 bending_radius, i32 line_width, Color4 line_color, Color4 rect_color, Color4 bend_color)
{
    Color4 no_color = {};
    
    LaneSegment3LR lane_segments = get_3_lane_segments_from_4_rectangles(top_or_bottom_rect, is_top_rect, middle_rect, left_rect, right_rect, bending_radius);
    
    LaneSegment lane_segment = {};
    
    lane_segment = lane_segments.top_or_bottom;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, bend_color, line_width);

    // TODO: should we always draw all 3 segments? Could one or some of them be invalid?
                      
    lane_segment = lane_segments.left;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, bend_color, line_width);
                      
    lane_segment = lane_segments.right;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, bend_color, line_width);
}



void draw_cross(Pos2d position, i32 distance_from_center, i32 line_length, Color4 line_color, i32 line_width)
{
    draw_line((Pos2d){position.x - distance_from_center, position.y}, 
              (Pos2d){position.x - distance_from_center - line_length, position.y}, 
              line_color, line_width);
    
    draw_line((Pos2d){position.x + distance_from_center, position.y}, 
              (Pos2d){position.x + distance_from_center + line_length, position.y}, 
              line_color, line_width);
    
    draw_line((Pos2d){position.x, position.y - distance_from_center}, 
              (Pos2d){position.x, position.y - distance_from_center - line_length}, 
              line_color, line_width);
              
    draw_line((Pos2d){position.x, position.y + distance_from_center}, 
              (Pos2d){position.x, position.y + distance_from_center + line_length}, 
              line_color, line_width);
}

struct HoveredOrPressed
{
    b32 is_hovered;
    b32 is_pressed;
};

HoveredOrPressed check_hovered_or_pressed(Pos2d position, Size2d size, Input * input)
{
    HoveredOrPressed result = {};
    
    MouseInput * mouse_input = &input->mouse;
    TouchesInput * touches_input = &input->touch;
    
    if (mouse_input->position.x >= position.x && mouse_input->position.x <= position.x + size.width &&
        mouse_input->position.y >= position.y && mouse_input->position.y <= position.y + size.height)
    {
        result.is_hovered = true;
        if (mouse_input->left_button_has_gone_down)
        {
            result.is_pressed = true;
        }
    }
    
    if (touches_input->touch_count == 1)
    {
        TouchInput * touch_input = &touches_input->touches[0];
        
        if (touch_input->has_started)
        {
            if (touch_input->position.x >= position.x && touch_input->position.x <= position.x + size.width &&
                touch_input->position.y >= position.y && touch_input->position.y <= position.y + size.height)
            {
                result.is_pressed = true;
            }
        }
    }
    
    return result;
}

b32 do_button(Pos2d position, Size2d size, ShortString * label, b32 is_active, Input * input, ShortString * label_active = 0)
{
    
    HoveredOrPressed hovered_or_pressed = check_hovered_or_pressed(position, size, input);
    
    Color4 line_color = {  0,   0,   0, 255};
    Color4 fill_color = {255, 255, 255, 255};
    
    if (is_active)
    {
        if (label_active)
        {
            label = label_active;
            if (hovered_or_pressed.is_hovered)
            {
                fill_color.r = 200;
                fill_color.g = 200;
                fill_color.b = 200;
            }
        }
        else
        {
            fill_color.r = 80;
            fill_color.g = 80;
            fill_color.b = 255;
        }
    }
    else if (hovered_or_pressed.is_hovered)
    {
        fill_color.r = 200;
        fill_color.g = 200;
        fill_color.b = 200;
    }
    
    i32 line_width = 1;
    
    draw_rounded_rectangle(position, size, 5, line_color, fill_color, line_width);
    
    Font font = {};
    font.height = 13;
    font.family = Font_Arial;
    
    Size2d text_size = get_text_size(label, font);
    
    Pos2d text_position = {};
    text_position.x = position.x + (size.width - text_size.width) / 2;
    text_position.y = position.y + (size.height - text_size.height) / 2;
    draw_text(text_position, label, font, line_color);
    
    return hovered_or_pressed.is_pressed;
}

b32 do_integer_button(Pos2d position, Size2d size, i32 number, b32 is_active, Input * input)
{
    ShortString decimal_string;
    int_to_string(number, &decimal_string);
    
    return do_button(position, size, &decimal_string, is_active, input);
}

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
    
    MemoryArena * lines_memory_arena;
    i32 nr_of_lines;
    
    i32 line_offset;
    
    MemoryArena * highlighted_line_parts_memory_arena;
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
    HighlightedLinePart * new_highlighted_line_part = (HighlightedLinePart *)push_struct(scrollable_text->highlighted_line_parts_memory_arena, sizeof(HighlightedLinePart));
    
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
    reset_memory_arena(scrollable_text->highlighted_line_parts_memory_arena);
    scrollable_text->first_highlighted_line_part = 0;
}

// TODO: we should probably create a function to de-allocate all memory inside the scrollable_text
void init_scrollable_text(ScrollableText * scrollable_text, b32 draw_line_numbers = true)
{
    if (!scrollable_text->lines_memory_arena)
    {
        scrollable_text->lines_memory_arena = new_memory_arena(&global_memory, true, (Color4){255,100,100,255}, 0);
    }
    else
    {
        reset_memory_arena(scrollable_text->lines_memory_arena, 0);
    }
    scrollable_text->nr_of_lines = 0;
    scrollable_text->line_offset = 0;
    
    if (!scrollable_text->highlighted_line_parts_memory_arena)
    {
        scrollable_text->highlighted_line_parts_memory_arena = new_memory_arena(&global_memory, false, (Color4){100,255,100,255}, 0);
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

void split_string_into_scrollable_lines(String string, ScrollableText * scrollable_text)
{
    i32 file_line_index = 0;
    
    String line_string = {};
    line_string.data = string.data; // first line starts at beginning of the string
    line_string.length = 0;
    
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
            put_string_in_index(file_line_index, line_string, scrollable_text->lines_memory_arena);
            
            // Starting a new line string
            start_of_line = position;
            
            file_line_index++;
            
            line_string.data = (u8 *)((i32)string.data + start_of_line);
            line_string.length = 0;
        }
    }
    line_string.length = position - start_of_line;
    put_string_in_index(file_line_index, line_string, scrollable_text->lines_memory_arena);
    
    scrollable_text->nr_of_lines = file_line_index + 1;
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
    
    if (scrollable_text->line_offset > scrollable_text->nr_of_lines - scrollable_text->nr_of_lines_to_show)
    {
        scrollable_text->line_offset = scrollable_text->nr_of_lines - scrollable_text->nr_of_lines_to_show;
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
    
    if (scrollable_text->nr_of_lines > 0)
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
        
        for (i32 line_on_screen_index = 0; line_on_screen_index < nr_of_lines_to_show; line_on_screen_index++)
        {
            i32 file_line_index = scrollable_text->line_offset + line_on_screen_index;
            
            if (file_line_index >= 0 && file_line_index < scrollable_text->nr_of_lines)
            {
                // Line text
                Pos2d position;
                position.x = scrollable_text->position.x + scrollable_text->left_margin + scrollable_text->line_numbers_width;
                position.y = scrollable_text->position.y + scrollable_text->top_margin + line_on_screen_index * (font.height + line_margin);
                
                String line_text = get_string_by_index(file_line_index, scrollable_text->lines_memory_arena);
                
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

void do_physical_pixels_switch(Input * input)
{
    Screen * screen = &input->screen;
    if (screen->device_pixel_ratio != 1.0f)
    {
        Size2d size_button = {50, 50};
        
        Pos2d position_button = {};
        position_button.x = screen->width - size_button.width - 20;
        position_button.y = 20;
        
        ShortString label;
        copy_char_to_string('-', &label);
        ShortString label_active;
        copy_char_to_string('+', &label_active);
        
        b32 button_is_pressed = do_button(position_button, size_button, &label, screen->using_physical_pixels, input, &label_active);
        
        if (button_is_pressed)
        {
            set_using_physical_pixels(!screen->using_physical_pixels);
        }
    }
}

void do_memory_usage(Memory * memory, Input * input, b32 * is_verbose)
{
    Screen * screen = &input->screen;
    
    Font font;
    font.family = Font_Arial;
    font.height = 20;
    
    Color4 black = {0, 0, 0, 255};
    Color4 dark_blue = {0, 0, 100, 255};
    Color4 light_blue = {220, 220, 255, 255};
    Color4 no_color = {};
    
    if (!*is_verbose)
    {
        // TODO: make it small
    }
    
    i32 bar_height = 100;
    
    Pos2d start_position = {};
    start_position.x = 100;
    start_position.y = screen->height - bar_height - 50;
    
    for (i32 memory_block_index = 0; memory_block_index < memory->nr_of_blocks; memory_block_index++)
    {
        if (memory->blocks_used[memory_block_index])
        {
            i32 bytes_used = memory->blocks[memory_block_index].bytes_used;
            i32 block_size = memory->block_size;
            
            i32 percentage_used = (i32)(bar_height * (f32)((f32)bytes_used / (f32)block_size));
            
            Color4 color = memory->blocks[memory_block_index].memory_arena->color;
            
            draw_rectangle((Pos2d){start_position.x + memory_block_index * 2, start_position.y + bar_height - percentage_used}, (Size2d){2,percentage_used}, no_color, color, 1);
            draw_rectangle((Pos2d){start_position.x + memory_block_index * 2, start_position.y}, (Size2d){2, bar_height - percentage_used}, no_color, light_blue, 1);
        }
        else {
            draw_rectangle((Pos2d){start_position.x + memory_block_index * 2, start_position.y}, (Size2d){2, bar_height}, no_color, light_blue, 1);
        }
    }
    
    
    
}

void do_frame_timing(Input * input, b32 * is_verbose)
{
    Timing * timing = &input->timing;
    Screen * screen = &input->screen;
    
    i32 nr_of_bars_to_show = MAX_NR_OF_FRAMES_FOR_TIMING;
    i32 bar_width = 6;
    i32 margin_between_bars = 2;
    i32 normal_bar_height = 100;
    f32 normal_value = (f32)1 / (f32)60; // TODO: should we put 60(fps) in a global?
    
    if (!*is_verbose)
    {
        bar_width = 3;
        margin_between_bars = 0;
        normal_bar_height = 33;
        nr_of_bars_to_show = MAX_NR_OF_FRAMES_FOR_TIMING / 2;
    }
    
    Pos2d start_position = {};
    start_position.x = screen->width - nr_of_bars_to_show * (bar_width + margin_between_bars) - 50;
    start_position.y = screen->height - normal_bar_height - 50;
    
    Color4 input_color =     {  0, 150,   0, 255};
    Color4 updating_color =  {150,   0,   0, 255};
    Color4 rendering_color = {  0,   0, 150, 255};
    Color4 waiting_color =   {220, 220, 220, 255};
    Color4 light_color =     {150, 255, 150, 150};
    Color4 no_color = {};
    
    i32 line_width = 1;
    
    Size2d graph_size = {};
    graph_size.width = nr_of_bars_to_show * (bar_width + margin_between_bars);
    graph_size.height = normal_bar_height;
    
    draw_line(start_position, (Pos2d){start_position.x + graph_size.width, start_position.y}, light_color, 1);
    
    Pos2d bar_position = {};
    Size2d bar_size = {};
    
    i32 bar_start = start_position.y + normal_bar_height;
    
    bar_size.width = bar_width;
    bar_size.height = 0;
    
    for (i32 frame_index_offset = 0; frame_index_offset > -nr_of_bars_to_show; frame_index_offset--)
    {
        i32 frame_index = (timing->frame_index + frame_index_offset + MAX_NR_OF_FRAMES_FOR_TIMING) % MAX_NR_OF_FRAMES_FOR_TIMING;
        i32 x_left = start_position.x + (nr_of_bars_to_show - 1 + frame_index_offset) * (bar_width + margin_between_bars);
        
        f32 input_time = timing->frame_times[frame_index].input_time;
        f32 updating_time = timing->frame_times[frame_index].updating_time;
        f32 rendering_time = timing->frame_times[frame_index].rendering_time;
        f32 waiting_time = timing->frame_times[frame_index].waiting_time;

        bar_position.x = x_left;
        
        if (!*is_verbose)
        {
            bar_start = start_position.y + normal_bar_height;
            bar_size.height = ((input_time + updating_time + rendering_time) / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, rendering_color, line_width);
        }
        else
        {

            // Input time
            
            bar_start = start_position.y + normal_bar_height;
            bar_size.height = (input_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, input_color, line_width);
                           
            // Updating time
            
            bar_start = bar_start - bar_size.height;
            bar_size.height = (updating_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, updating_color, line_width);
                       
            // Rendering time
            
            bar_start = bar_start - bar_size.height;
            bar_size.height = (rendering_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, rendering_color, line_width);
                           
            // Waiting time
            
            bar_start = bar_start - bar_size.height;
            bar_size.height = (waiting_time / normal_value) * normal_bar_height;
            bar_position.y = bar_start - bar_size.height;
            
            draw_rectangle(bar_position, bar_size, no_color, waiting_color, line_width);
        }
    }
    
    HoveredOrPressed hovered_or_pressed = check_hovered_or_pressed(start_position, graph_size, input);
    
    if (hovered_or_pressed.is_pressed)
    {
        *is_verbose = !*is_verbose;
    }
}
