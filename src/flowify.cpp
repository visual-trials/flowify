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
 
#include "parser.cpp"

#include "flowify/flow.h"
#include "flowify/interact.cpp"
#include "flowify/flow.cpp"
#include "flowify/layout.cpp"
#include "flowify/draw.cpp"

#include "examples.h"

struct WorldData
{
    Rect2d title_rect;
    
    String program_text;
    ScrollableText scrollable_program_text;
    Window program_text_window;

    Tokenizer tokenizer;
    Parser parser;
    Flowifier flowifier;
    
    DynamicString flowify_dump_text;
    ScrollableText scrollable_flowify_dump;
    Window flowify_dump_window;
    
    DynamicString flowify_detail_text;
    ScrollableText scrollable_flowify_detail;
    Window flowify_detail_window;
    
    Margins screen_margins;
    i32 middle_margin;
    i32 title_height;
    f32 program_text_fraction_of_screen;
    f32 flowify_dump_fraction_of_screen;
    
    FlowElement * root_element;
    
    const char * program_texts[10];
    i32 nr_of_program_texts;
    i32 current_program_text_index;
    
    i32 iteration;
    
    // TODO: Window flowify_window;
    i32 flowify_vertical_offset;
    i32 flowify_horizontal_offset;
    
    // FIXME: ugly HACK!
    b32 mouse_dragging;
    Pos2d last_mouse_position;
    
    // FIXME: ugly HACK!
    b32 touch_dragging;
    Pos2d last_touch_position;
    
    b32 verbose_memory_usage;
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    
    void load_program_text(const char * program_text, WorldData * world)
    {
        Tokenizer * tokenizer = &world->tokenizer;
        Parser * parser = &world->parser;
        Flowifier * flowifier = &world->flowifier;
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        ScrollableText * scrollable_flowify_dump = &world->scrollable_flowify_dump;
        DynamicString * flowify_dump_text = &world->flowify_dump_text;
        
        init_scrollable_text(scrollable_program_text, &world->program_text_window);
        init_tokenizer(tokenizer);
        init_parser(parser, tokenizer);
        init_flowifier(flowifier, parser);
        init_dynamic_string(flowify_dump_text, (Color4){70,150,255,255}, cstring_to_string("Flowify dump text"));
        init_scrollable_text(scrollable_flowify_dump, &world->flowify_dump_window, false);
        
        world->program_text.data = (u8 *)program_text;
        world->program_text.length = cstring_length(program_text);
        
        split_string_into_scrollable_lines(world->program_text, scrollable_program_text);

        // Tokenize
        tokenize(tokenizer, (u8 *)program_text);

        //Parse
        Node * root_node = parse_program(parser);
        
        // Flowify
        // FIXME: create the root_element inside flowify_statements (and put it in the Flowifier-struct
        FlowElement * root_element = new_flow_element(flowifier, root_node, FlowElement_Root);
        flowify_statements(flowifier, root_element);
        
        // FIXME: put root_element inside the Flowifier-struct!
        world->root_element = root_element;
        
        dump_element_tree(root_element, &world->flowify_dump_text);
        
        split_string_into_scrollable_lines(world->flowify_dump_text.string, scrollable_flowify_dump);
    }
    
    void update_window_dimensions(WorldData * world, Screen * screen)
    {
        Rect2d full_screen_rect = {}; // also meaning: position = 0,0
        full_screen_rect.size.width = screen->width;
        full_screen_rect.size.height = screen->height;
        
        Rect2d available_screen_rect = shrink_rect_by_margins(full_screen_rect, world->screen_margins);
        Rectangle2 title_and_text_rects = split_rect_vertically(available_screen_rect, world->title_height);
        Rectangle2 horizontal_rects = split_rect_horizontally_fraction(title_and_text_rects.second, world->flowify_dump_fraction_of_screen, world->middle_margin);
        Rectangle2 dump_and_flowify_rects = split_rect_horizontally_fraction(horizontal_rects.second, world->program_text_fraction_of_screen / (1 - world->flowify_dump_fraction_of_screen), world->middle_margin);
        
        world->title_rect = title_and_text_rects.first;
        world->flowify_dump_window.screen_rect = horizontal_rects.first;
        world->program_text_window.screen_rect = dump_and_flowify_rects.first;
        // TODO: store rect in flowify-window!
    }
        
    void init_world()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        world->iteration = 0;
        
        world->flowify_vertical_offset = 0;
        world->flowify_horizontal_offset = 0;
        
        world->program_texts[0] = simple_assign_program_text;
        world->program_texts[1] = i_plus_plus_program_text;
        world->program_texts[2] = simple_functions_program_text;
        world->program_texts[3] = simple_if_program_text;
        world->program_texts[4] = simple_if_else_program_text;
        world->program_texts[5] = simple_for;
        // TODO: in the end, we probably want this example instead: world->program_texts[5] = simple_for_continue_break_program_text;
        world->program_texts[6] = fibonacci_iterative_program_text;
        world->program_texts[7] = fibonacci_recursive_early_return_program_text;
        world->nr_of_program_texts = 8;
        
        world->current_program_text_index = 2;
        
        world->verbose_memory_usage = true;

        world->screen_margins.left = 100;
        world->screen_margins.top = 20;
        world->screen_margins.right = 100;
        world->screen_margins.bottom = 20;
        
        world->middle_margin = 20;
        world->flowify_dump_fraction_of_screen = 0.5;
        world->program_text_fraction_of_screen = 0.5;
        world->title_height = 30;
        
        world->mouse_dragging = false;
        world->last_mouse_position.x = 0;
        world->last_mouse_position.y = 0;
        
        world->touch_dragging = false;
        world->last_touch_position.x = 0;
        world->last_touch_position.y = 0;
        
        update_window_dimensions(world, &input->screen);
        
        load_program_text(world->program_texts[world->current_program_text_index], world);
        
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        Flowifier * flowifier = &world->flowifier;

        FlowElement * flow_elements = (FlowElement *)flowifier->flow_elements.items;
        i32 nr_of_flow_elements = flowifier->flow_elements.nr_of_items;
        
        // Process input
        if (flowifier->has_absolute_positions)
        {
            i32 old_selected_element_index = flowifier->interaction.selected_element_index;
            process_interactions(flowifier, input, world->root_element);
            if (old_selected_element_index != flowifier->interaction.selected_element_index)
            {
                FlowElement * newly_selected_flow_element = &flow_elements[flowifier->interaction.selected_element_index];
                
                init_scrollable_text(&world->scrollable_flowify_detail, &world->flowify_detail_window, false);
                
                init_dynamic_string(&world->flowify_detail_text, (Color4){255,70,150,255}, cstring_to_string("Flowify detail text"));

                generate_element_detail(newly_selected_flow_element, &world->flowify_detail_text);

                split_string_into_scrollable_lines(world->flowify_detail_text.string, &world->scrollable_flowify_detail);
            }
        }
        
        // Update world
        update_window_dimensions(world, &input->screen);
        
        update_scrollable_text(&world->scrollable_program_text, input);
        update_scrollable_text(&world->scrollable_flowify_dump, input);
        
        layout_elements(flowifier, world->root_element);
        
        world->iteration++;
        
        if (flowifier->interaction.highlighted_element_index > 0 && world->iteration > 60) // every second (and if it is a valid selected element)
        {
            world->iteration = 0;
            
            flowifier->interaction.highlighted_element_index++;
            while (flowifier->interaction.highlighted_element_index < nr_of_flow_elements)
            {
                FlowElement * newly_highlighted_flow_element = &flow_elements[flowifier->interaction.highlighted_element_index];
                if (newly_highlighted_flow_element->is_highlightable)
                {
                    break;
                }
                flowifier->interaction.highlighted_element_index++;
            }
        }
        if (flowifier->interaction.highlighted_element_index == 0 || flowifier->interaction.highlighted_element_index >= nr_of_flow_elements)
        {
            flowifier->interaction.highlighted_element_index = 1;
            while (flowifier->interaction.highlighted_element_index < nr_of_flow_elements)
            {
                FlowElement * newly_highlighted_flow_element = &flow_elements[flowifier->interaction.highlighted_element_index];
                if (newly_highlighted_flow_element->is_highlightable)
                {
                    break;
                }
                flowifier->interaction.highlighted_element_index++;
            }
        }
        
    }
    
    // TODO: make this a more general function (add arguments nr_of_buttons and current_button_index)
    //       then return button_pressed_index (so you can load the program text for that index)
    void draw_and_update_button_menu(WorldData * world)
    {
        // Draw (and update) button menu
        
        Size2d size_button = {50, 50};
        Pos2d position_button = {20, 20};
        i32 margin_between_buttons = 20;
        
        for (i32 program_text_index = 0; program_text_index < world->nr_of_program_texts; program_text_index++)
        {
            b32 button_is_active = false;
            if (program_text_index == world->current_program_text_index)
            {
                button_is_active = true;
            }
            
            position_button.y = 20 + program_text_index * (margin_between_buttons + size_button.height);
            b32 button_is_pressed = do_integer_button(position_button, size_button, program_text_index + 1, button_is_active, &global_input);
            
            if (button_is_pressed)
            {
                world->current_program_text_index = program_text_index;
                load_program_text(world->program_texts[world->current_program_text_index], world);
            }
        }
        
        
        FlowElement * root_element = world->root_element;
        Input * input = &global_input;
        MouseInput * mouse = &input->mouse;
        TouchesInput * touch = &input->touch;
        Flowifier * flowifier = &world->flowifier;
        
        if (mouse->wheel_has_moved)
        {
            // TODO: account for a "Mac" mouse! (which has a 'continous' wheel)
            if (mouse->wheel_delta > 0)
            {
                world->flowify_vertical_offset += 30;
            }
            
            if (mouse->wheel_delta < 0)
            {
                world->flowify_vertical_offset -= 30;
            }
        }
        
        // FIXME: ugly HACK!
        Pos2d mouse_delta_position = {};
        if (mouse->left_button_is_down)
        {
            if (world->mouse_dragging)
            {
                mouse_delta_position.x = mouse->position.x - world->last_mouse_position.x;
                mouse_delta_position.y = mouse->position.y - world->last_mouse_position.y;
            }
            world->flowify_horizontal_offset += mouse_delta_position.x;
            world->flowify_vertical_offset += mouse_delta_position.y;
            
            world->last_mouse_position = mouse->position;
            world->mouse_dragging = true;
        }
        else
        {
            world->mouse_dragging = false;
        }
        
        // FIXME: ugly HACK!
        Pos2d touch_delta_position = {};
        if (touch->touch_count == 1)
        {
            if (world->touch_dragging)
            {
                touch_delta_position.x = touch->touches[0].position.x - world->last_touch_position.x;
                touch_delta_position.y = touch->touches[0].position.y - world->last_touch_position.y;
            }
            world->flowify_horizontal_offset += touch_delta_position.x;
            world->flowify_vertical_offset += touch_delta_position.y;
            
            // FIXME: we are not checking if the touch was actually ended!
            world->last_touch_position = touch->touches[0].position;
            world->touch_dragging = true;
        }
        else
        {
            world->touch_dragging = false;
        }
        
        
        
        Pos2d absolute_position;
        // FIXME: hack!
        // Aligned right: absolute_position.x = input->screen.width - root_element->size.width - 100 + world->flowify_horizontal_offset; 
        absolute_position.x = 100 + world->flowify_horizontal_offset; 
        absolute_position.y = 50 + world->flowify_vertical_offset; 
        absolute_layout_elements(flowifier, root_element, absolute_position);
        
        draw_elements(flowifier, root_element);
        
        // FIXME: hack!
        if (flowifier->interaction.selected_element_index)
        {
            FlowElement * flow_elements = (FlowElement *)flowifier->flow_elements.items;
            FlowElement * selected_flow_element = &flow_elements[flowifier->interaction.selected_element_index];
            
            Rect2d detail_rect = {};
            
            detail_rect.position = world->root_element->absolute_position;
            detail_rect.size.width = 500; // FIXME: we should choose a different width here right?
            detail_rect.size.height = 644; // FIXME: we should choose a different width here right?
            
            detail_rect.position.x += world->root_element->size.width + 20;
            detail_rect.position.y = selected_flow_element->absolute_position.y;
            //selected_flow_element->absolute_position;
            //selected_flow_element->size
            
            draw_rounded_rectangle(detail_rect.position, detail_rect.size, flowifier->bending_radius, 
                                   flowifier->detail_line_color, flowifier->detail_fill_color, flowifier->detail_line_width);

            // FIXME: update_scrollable_text should be called in update_frame!!
            world->flowify_detail_window.screen_rect = detail_rect;
            update_scrollable_text(&world->scrollable_flowify_detail, input);
            
            draw_scrollable_text(&world->scrollable_flowify_detail);
        }
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        Memory * memory = &global_memory;
        Flowifier * flowifier = &world->flowifier;
        
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        ScrollableText * scrollable_flowify_dump = &world->scrollable_flowify_dump;
        
        Font font = {};
        font.height = 20;
        font.family = Font_CourierNew;

        Color4 black = {};
        black.a = 255;
        
        Token * tokens = (Token *)world->tokenizer.tokens.items;
        
        FlowElement * flow_elements = (FlowElement *)flowifier->flow_elements.items;
        i32 nr_of_flow_elements = flowifier->flow_elements.nr_of_items;
        
        String * lines = (String *)scrollable_program_text->lines.items;

        if (nr_of_flow_elements > 0 && flowifier->interaction.highlighted_element_index > 0)
        {
            FlowElement * highlighted_flow_element = &flow_elements[flowifier->interaction.highlighted_element_index];
            Node * node = highlighted_flow_element->ast_node;
            
            remove_highlighted_line_parts(scrollable_program_text);
            for (i32 token_index = node->first_token_index; token_index <= node->last_token_index; token_index++)
            {
                Token token = tokens[token_index];
                
                if (token.type != Token_EndOfStream)
                {
                    String program_line_text = lines[token.line_index];
                    
                    i32 character_in_line_index = (i32)token.text.data - (i32)program_line_text.data;

                    HighlightedLinePart * highlighted_line_part = add_new_highlighted_line_part(scrollable_program_text);

                    highlighted_line_part->line_index = token.line_index;
                    highlighted_line_part->start_character_index = (u16)character_in_line_index;
                    highlighted_line_part->length = (u16)token.text.length;
                }
            }

            remove_highlighted_line_parts(scrollable_flowify_dump); 
            
            HighlightedLinePart * highlighted_line_part = add_new_highlighted_line_part(scrollable_flowify_dump);
            highlighted_line_part->line_index = highlighted_flow_element->highlighted_line_part.line_index;
            highlighted_line_part->start_character_index = highlighted_flow_element->highlighted_line_part.start_character_index;
            highlighted_line_part->length = highlighted_flow_element->highlighted_line_part.length;
        }
        
        draw_scrollable_text(scrollable_program_text);
        // Turned off for the moment: draw_scrollable_text(scrollable_flowify_dump);
        
        draw_and_update_button_menu(world);

        // Button for toggling showing help rectangles
        {
            Size2d size_button = {50, 50};
            Pos2d position_button = {};
            position_button.x = global_input.screen.width - size_button.width - 20;
            position_button.y = 400;
            
            ShortString label;
            copy_cstring_to_short_string("Help", &label);
            ShortString label_active;
            copy_cstring_to_short_string("[   ]", &label_active);

            b32 button_is_pressed = do_button(position_button, size_button, &label, flowifier->show_help_rectangles, &global_input, &label_active);

            if (button_is_pressed)
            {
                flowifier->show_help_rectangles = !flowifier->show_help_rectangles;
            }
        }
        
        // do_memory_usage(memory, input, &world->verbose_memory_usage);
        do_frame_timing(&global_input, &world->verbose_frame_times);
        do_physical_pixels_switch(&global_input);
    }
}
