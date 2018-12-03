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

struct ScrollableText
{
    b32 is_active;  // TODO: implement this
    
    String lines[1000];
    i32 nr_of_lines;
    i32 line_offset;
    
    Font font;
    i32 line_margin;
    
    i32 left_margin;
    i32 top_margin;
    i32 bottom_margin;
    i32 right_margin;
    
    Pos2d position;
    Size2d size;
    
    i32 nr_of_lines_to_show;
    i32 max_line_width_in_characters;
};

void init_scrollable_text(ScrollableText * scrollable_text)
{
    // TODO: put below in init_scrollable_text?
    scrollable_text->nr_of_lines = 0;
    scrollable_text->line_offset = 0;
    
    Font font = {};
    font.height = 20;
    font.family = Font_CourierNew;
    scrollable_text->font = font;
    
    scrollable_text->line_margin = 4;

    // TODO: do we want to use nr-of-characters or percentage-of-screen for margins?
    scrollable_text->left_margin = 130;  // TODO: should be lower (calculate width of line numbers each frame)
    scrollable_text->top_margin = 120;   // TODO: should be lower! (assume no top-text)
    scrollable_text->right_margin = 10;
    scrollable_text->bottom_margin = 10;
    
    // These should be calculated each update
    scrollable_text->position.x = 0;
    scrollable_text->position.y = 0;
    
    scrollable_text->size.width = 100;
    scrollable_text->size.height = 100;
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
    
    scrollable_text->nr_of_lines_to_show = (i32)(((f32)input->screen.height - (f32)scrollable_text->top_margin - scrollable_text->bottom_margin) / 
                                       ((f32)scrollable_text->font.height + (f32)scrollable_text->line_margin));

    ShortString white_space;
    copy_char_to_string(' ', &white_space);
    Size2d white_space_size = get_text_size(&white_space, scrollable_text->font);
    scrollable_text->max_line_width_in_characters = (i32)((f32)(input->screen.width - scrollable_text->left_margin - scrollable_text->right_margin) / (f32)white_space_size.width);
        
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
    
    
    Color4 black = {};
    black.a = 255;
    
    Color4 grey = {};
    grey.a = 100;
    
    Font font = scrollable_text->font;
    i32 line_margin = scrollable_text->line_margin;
    i32 nr_of_lines_to_show = scrollable_text->nr_of_lines_to_show;
    
    if (scrollable_text->nr_of_lines > 0)
    {
        ShortString line_nr_text;
        
        for (i32 line_on_screen_index = 0; line_on_screen_index < nr_of_lines_to_show; line_on_screen_index++)
        {
            i32 file_line_index = scrollable_text->line_offset + line_on_screen_index;
            
            if (file_line_index >= 0 && file_line_index < scrollable_text->nr_of_lines)
            {
                // Line text
                Pos2d position;
                position.x = scrollable_text->left_margin;
                position.y = scrollable_text->top_margin + line_on_screen_index * (font.height + line_margin);
                
                String line_text = scrollable_text->lines[file_line_index];
                if (line_text.length > scrollable_text->max_line_width_in_characters)
                {
                    line_text.length = scrollable_text->max_line_width_in_characters;
                }
                draw_text(position, &line_text, font, black);
                
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
