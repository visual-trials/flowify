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
    i32 selected_element_index;
    
    // TODO: Window flowify_window;
    i32 flowify_vertical_offset;
    
    b32 show_help_rectangles;
    
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
        FlowElement * root_element = new_flow_element(flowifier, root_node, FlowElement_Root);
        flowify_statements(flowifier, root_element);
        
        // TODO: should we do this in update_frame?
        layout_elements(flowifier, root_element);
        world->root_element = root_element;
        
        dump_element_tree(root_element, &world->flowify_dump_text);
        
        split_string_into_scrollable_lines(world->flowify_dump_text.string, scrollable_flowify_dump);
        
        // Note: we reset this, so selected_element_index never refers to a non-existing element (from a previous file parse/flowify)
        world->selected_element_index = -1;  // TODO: there is probably a nicer way of saying this value is invalid
    }
    
    void update_window_dimensions(WorldData * world, Screen * screen)
    {
        Rect2d full_screen_rect = {}; // also meaning: position = 0,0
        full_screen_rect.size.width = screen->width;
        full_screen_rect.size.height = screen->height;
        
        Rect2d available_screen_rect = shrink_rect_by_margins(full_screen_rect, world->screen_margins);
        Rectangle2 title_and_text_rects = split_rect_vertically(available_screen_rect, world->title_height);
        Rectangle2 horizontal_rects = split_rect_horizontally_fraction(title_and_text_rects.second, world->program_text_fraction_of_screen, world->middle_margin);
        Rectangle2 dump_and_flowify_rects = split_rect_horizontally_fraction(horizontal_rects.second, world->flowify_dump_fraction_of_screen / (1 - world->program_text_fraction_of_screen), world->middle_margin);
        
        world->title_rect = title_and_text_rects.first;
        world->program_text_window.screen_rect = horizontal_rects.first;
        world->flowify_dump_window.screen_rect = dump_and_flowify_rects.first;
        // TODO: store rect in flowify-window!
    }
        
    void init_world()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        world->iteration = 0;
        world->selected_element_index = -1;  // TODO: there is probably a nicer way of saying this value is invalid
        
        world->flowify_vertical_offset = 0;
        
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
        
        world->current_program_text_index = 5;
        
        world->verbose_memory_usage = true;

        world->screen_margins.left = 100;
        world->screen_margins.top = 20;
        world->screen_margins.right = 100;
        world->screen_margins.bottom = 20;
        
        world->middle_margin = 20;
        world->program_text_fraction_of_screen = 0.35;
        world->flowify_dump_fraction_of_screen = 0.35;
        world->title_height = 30;
        
        update_window_dimensions(world, &input->screen);
        
        load_program_text(world->program_texts[world->current_program_text_index], world);
        
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;

        update_window_dimensions(world, &input->screen);
        
        update_scrollable_text(&world->scrollable_program_text, input);
        update_scrollable_text(&world->scrollable_flowify_dump, input);
        
        world->iteration++;
        
        // FIXME: we should not set is_selected on the flow element itself. Instead we should keep a record of the *element identifier* we want to select
        //        whenever we draw the element we check for a *match* between the  selected *element identifier* and the identifier of the element.
        
        FlowElement * flow_elements = (FlowElement *)world->flowifier.flow_elements.items;
        i32 nr_of_flow_elements = world->flowifier.flow_elements.nr_of_items;
        
        if (world->selected_element_index >= 0 && world->iteration > 60) // every second (and if it is a valid selected element)
        {
            world->iteration = 0;
            
            FlowElement * selected_flow_element = &flow_elements[world->selected_element_index];
            selected_flow_element->is_selected = false;
            
            world->selected_element_index++;
            while (world->selected_element_index < nr_of_flow_elements)
            {
                
                FlowElement * newly_selected_flow_element = &flow_elements[world->selected_element_index];
                if (newly_selected_flow_element->is_selectable)
                {
                    newly_selected_flow_element->is_selected = true;
                    break;
                }
                world->selected_element_index++;
            }
        }
        if (world->selected_element_index < 0 || world->selected_element_index >= nr_of_flow_elements)
        {
            world->selected_element_index = 0;
            while (world->selected_element_index < nr_of_flow_elements)
            {
                FlowElement * newly_selected_flow_element = &flow_elements[world->selected_element_index];
                if (newly_selected_flow_element->is_selectable)
                {
                    newly_selected_flow_element->is_selected = true;
                    break;
                }
                world->selected_element_index++;
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
        
        Pos2d absolute_position;
        // FIXME: hack!
        absolute_position.x = input->screen.width - root_element->size.width - 100; 
        absolute_position.y = 50 + world->flowify_vertical_offset; 
        absolute_layout_elements(root_element, absolute_position);
        
        draw_elements(&world->flowifier, root_element, world->show_help_rectangles);
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        Memory * memory = &global_memory;
        
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        ScrollableText * scrollable_flowify_dump = &world->scrollable_flowify_dump;
        
        Font font = {};
        font.height = 20;
        font.family = Font_CourierNew;

        Color4 black = {};
        black.a = 255;
        
        Token * tokens = (Token *)world->tokenizer.tokens.items;
        
        FlowElement * flow_elements = (FlowElement *)world->flowifier.flow_elements.items;
        i32 nr_of_flow_elements = world->flowifier.flow_elements.nr_of_items;
        
        String * lines = (String *)scrollable_program_text->lines.items;

        if (nr_of_flow_elements > 0 && world->selected_element_index >= 0)
        {
            FlowElement * selected_flow_element = &flow_elements[world->selected_element_index];
            Node * node = selected_flow_element->ast_node;
            
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
            highlighted_line_part->line_index = selected_flow_element->highlighted_line_part.line_index;
            highlighted_line_part->start_character_index = selected_flow_element->highlighted_line_part.start_character_index;
            highlighted_line_part->length = selected_flow_element->highlighted_line_part.length;
        }
        
        draw_scrollable_text(scrollable_program_text);
        draw_scrollable_text(scrollable_flowify_dump);
        
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

            b32 button_is_pressed = do_button(position_button, size_button, &label, world->show_help_rectangles, &global_input, &label_active);

            if (button_is_pressed)
            {
                world->show_help_rectangles = !world->show_help_rectangles;
            }
        }
        
        // do_memory_usage(memory, input, &world->verbose_memory_usage);
        do_frame_timing(&global_input, &world->verbose_frame_times);
        do_physical_pixels_switch(&global_input);
    }
}
