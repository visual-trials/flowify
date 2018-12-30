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
#include "memory.cpp"
#include "input.cpp"
#include "render.cpp"

#include "parser.cpp"
#include "flowify_statements.cpp"

#include "examples.h"

struct WorldData
{
    String program_text;
    ScrollableText scrollable_program_text;

    Tokenizer tokenizer;
    Parser parser;
    Flowifier flowifier;
    
    DynamicString flowify_dump_text;
    ScrollableText scrollable_flowify_dump;
    
    FlowElement * root_element;
    
    const char * program_texts[10];
    i32 nr_of_program_texts;
    i32 current_program_text_index;
    
    i32 iteration;
    i32 selected_element_index;
    
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
        
        init_scrollable_text(scrollable_program_text);
        init_tokenizer(tokenizer);
        init_parser(parser, tokenizer);
        init_flowifier(flowifier);
        // TODO: we probably want reset_dynamic_string to create the dynamic string if the arena doesn't exist. But how to deal with the color?
        // TODO: use/implement init_dynamic_string
        if (!world->flowify_dump_text.memory_arena.memory)
        {
            world->flowify_dump_text = create_dynamic_string((Color4){70,150,255,255}, cstring_to_string("Flowify dump text"));
        }
        else
        {
            reset_dynamic_string(&world->flowify_dump_text);
        }
        init_scrollable_text(scrollable_flowify_dump, false);
        
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
        layout_elements(root_element);
        world->root_element = root_element;
        
        dump_element_tree(root_element, &world->flowify_dump_text);
        
        split_string_into_scrollable_lines(world->flowify_dump_text.string, scrollable_flowify_dump);
    }
    
    void init_world()
    {
        WorldData * world = &global_world;
        Memory * memory = &global_memory;
        
        world->iteration = 0;
        world->selected_element_index = 1;  // FIXME: HACK
        
        world->program_texts[0] = simple_assign_program_text;
        world->program_texts[1] = i_plus_plus_program_text;
        world->program_texts[2] = simple_functions_program_text;
        world->program_texts[3] = simple_if_program_text;
        world->program_texts[4] = simple_if_else_program_text;
        world->program_texts[5] = simple_for_continue_break_program_text;
        world->program_texts[6] = fibonacci_iterative_program_text;
        world->program_texts[7] = fibonacci_recursive_early_return_program_text;
        world->nr_of_program_texts = 8;
        
        world->current_program_text_index = 4;
        
        world->verbose_memory_usage = true;

        load_program_text(world->program_texts[world->current_program_text_index], world);
        
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;

        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        
        // The screen size can change, so we have to update the position and size of the scrollables.
        scrollable_program_text->position.x = 100;
        scrollable_program_text->position.y = 50; // TODO: we should properly account for the height of the text above this

        scrollable_program_text->size.width = input->screen.width - scrollable_program_text->position.x;
        scrollable_program_text->size.height = input->screen.height - scrollable_program_text->position.y;
        
        update_scrollable_text(scrollable_program_text, input);
        
        ScrollableText * scrollable_flowify_dump = &world->scrollable_flowify_dump;
        
        scrollable_flowify_dump->position.x = input->screen.width / 2 - 50; // TODO: calculate by percentage?
        scrollable_flowify_dump->position.y = 50; // TODO: where do we want to let this begin?

        scrollable_flowify_dump->size.width = input->screen.width - scrollable_flowify_dump->position.x;
        scrollable_flowify_dump->size.height = input->screen.height - scrollable_flowify_dump->position.y - 50;
        
        update_scrollable_text(scrollable_flowify_dump, input);
        
        world->iteration++;
        
        // FIXME: we should not set is_selected on the flow element itself. Instead we should keep a record of the *element identifier* we want to select
        //        whenever we draw the element we check for a *match* between the  selected *element identifier* and the identifier of the element.
        
        FlowElement * flow_elements = (FlowElement *)world->flowifier.flow_elements.items;
        i32 nr_of_flow_elements = world->flowifier.flow_elements.nr_of_items;
        
        if (world->iteration > 60) // every second
        {
            world->iteration = 0;
            
            FlowElement * selected_flow_element = &flow_elements[world->selected_element_index];
            selected_flow_element->is_selected = false;
            
            world->selected_element_index++;
            while (world->selected_element_index < nr_of_flow_elements)
            {
                
                FlowElement * newly_selected_flow_element = &flow_elements[world->selected_element_index];
                if (newly_selected_flow_element->has_lane_segments)
                {
                    newly_selected_flow_element->is_selected = true;
                    break;
                }
                world->selected_element_index++;
            }
        }
        if (world->selected_element_index >= nr_of_flow_elements)
        {
            world->selected_element_index = 0;
            while (world->selected_element_index < nr_of_flow_elements)
            {
                FlowElement * newly_selected_flow_element = &flow_elements[world->selected_element_index];
                if (newly_selected_flow_element->has_lane_segments)
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
        
        // FIXME: hack!
        FlowElement * root_element = world->root_element;
        Input * input = &global_input;
        Pos2d position;
        position.x = input->screen.width - root_element->size.width - 100;
        position.y = 50;
        draw_elements(root_element, position, world->show_help_rectangles);
        
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

        if (nr_of_flow_elements > 0)
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
        
        do_memory_usage(memory, input, &world->verbose_memory_usage);
        do_frame_timing(&global_input, &world->verbose_frame_times);
        do_physical_pixels_switch(&global_input);
    }
}
