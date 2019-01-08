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
 
#include "../parser.cpp"
#include "../examples.h"

struct WorldData
{
    String program_text;
    
    ScrollableText scrollable_program_text;
    Window program_text_window;
    
    Tokenizer tokenizer;
    Parser parser;
    
    DynamicString dump_text;
    
    ScrollableText scrollable_ast_dump;
    Window ast_dump_window;
    
    const char * program_texts[10];
    i32 nr_of_program_texts;
    i32 current_program_text_index;
    
    i32 iteration;
    i32 selected_token_index;
    i32 selected_node_index;
    
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    
    void load_program_text(const char * program_text, WorldData * world)
    {
        
        Tokenizer * tokenizer = &world->tokenizer;
        Parser * parser = &world->parser;
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        ScrollableText * scrollable_ast_dump = &world->scrollable_ast_dump;
        
        init_scrollable_text(scrollable_program_text, &world->program_text_window);
        init_tokenizer(tokenizer);
        init_parser(parser, tokenizer);
        init_dynamic_string(&world->dump_text, (Color4){70,150,255,255}, cstring_to_string("Ast dump text"));
        init_scrollable_text(scrollable_ast_dump, &world->ast_dump_window, false);
        
        world->program_text.data = (u8 *)program_text;
        world->program_text.length = cstring_length(program_text);
        
        split_string_into_scrollable_lines(world->program_text, scrollable_program_text);

        // Tokenize
        tokenize(tokenizer, (u8 *)program_text);

        //Parse
        Node * root_node = parse_program(parser);
        
        
        // Dump parse result
        dump_tree(root_node, &world->dump_text);
        
        split_string_into_scrollable_lines(world->dump_text.string, scrollable_ast_dump);
    }
    
    void init_world()
    {
        WorldData * world = &global_world;
        Memory * memory = &global_memory;
        
        world->iteration = 0;
        world->selected_token_index = 0;
        world->selected_node_index = 0;
        
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

        // TODO: call a function the divides the screen into parts
        world->program_text_window.screen_rect.position = (Pos2d){};
        world->program_text_window.screen_rect.size = (Size2d){};
        
        world->ast_dump_window.screen_rect.position = (Pos2d){};
        world->ast_dump_window.screen_rect.size = (Size2d){};
        
        load_program_text(world->program_texts[world->current_program_text_index], world);
        
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;

        // TODO: put this somewhere else
        i32 left_margin = 100;
        i32 top_margin = 50; // TODO: we should properly account for the height of the text above this
        i32 right_margin = 10;
        i32 bottom_margin = 10;
        
        // TODO: call a function the divides the screen into parts
        // The screen size can change, so we have to update the position and size of the windows of the scrollables.
        world->program_text_window.screen_rect.position.x = left_margin;
        world->program_text_window.screen_rect.position.y = top_margin; 

        world->program_text_window.screen_rect.size.width = (input->screen.width - left_margin - right_margin) * 0.4;
        world->program_text_window.screen_rect.size.height = input->screen.height - top_margin - bottom_margin;
        
        world->ast_dump_window.screen_rect.position.x = left_margin + (input->screen.width - left_margin - right_margin) * 0.4;
        world->ast_dump_window.screen_rect.position.y = top_margin;

        world->ast_dump_window.screen_rect.size.width = (input->screen.width - left_margin - right_margin) * 0.6;
        world->ast_dump_window.screen_rect.size.height = input->screen.height - top_margin - bottom_margin;
        
        
        update_scrollable_text(&world->scrollable_program_text, input);
        update_scrollable_text(&world->scrollable_ast_dump, input);
        
        world->iteration++;
        if (world->iteration > 60) // every second
        {
            world->iteration = 0;
            
            world->selected_token_index++;
            world->selected_node_index++;
        }
        if (world->selected_token_index >= world->tokenizer.tokens.nr_of_items)
        {
            world->selected_token_index = 0;
        }        
        if (world->selected_node_index >= world->parser.nodes.nr_of_items)
        {
            world->selected_node_index = 0;
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
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        ScrollableText * scrollable_ast_dump = &world->scrollable_ast_dump;
        
        Font font = {};
        font.height = 20;
        font.family = Font_CourierNew;

        Color4 black = {};
        black.a = 255;
        
        /*
        if (world->tokenizer.nr_of_tokens > 0)
        {
            Token token = world->tokenizer.tokens[world->selected_token_index];
            
            i32 character_in_line_index = (i32)token.text.data - (i32)scrollable_program_text->lines[token.line_index].data;

            HighlightedLinePart highlighted_line_part = {};
            highlighted_line_part.line_index = token.line_index;
            highlighted_line_part.start_character_index = (u16)character_in_line_index;
            highlighted_line_part.length = (u16)token.text.length;

            // FIXME: do not hardcode it this way!
            scrollable_program_text->highlighted_line_parts[0] = highlighted_line_part;
            scrollable_program_text->nr_of_highlighted_parts = 1;
            
            // TODO: we might want to draw the TokenType
        }
        */
        Token * tokens = (Token *)world->tokenizer.tokens.items;
        Node * nodes = (Node *)world->parser.nodes.items;
        String * lines = (String *)scrollable_program_text->lines.items;
        
        if (world->parser.nodes.nr_of_items > 0)
        {
            Node * node = &nodes[world->selected_node_index];
            
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
            
            remove_highlighted_line_parts(scrollable_ast_dump);
            
            HighlightedLinePart * highlighted_line_part = add_new_highlighted_line_part(scrollable_ast_dump);
            highlighted_line_part->line_index = node->highlighted_line_part.line_index;
            highlighted_line_part->start_character_index = node->highlighted_line_part.start_character_index;
            highlighted_line_part->length = node->highlighted_line_part.length;
        }
        
        // TODO: add clip-rectanglesn (inside draw_scrollable_text!)
        draw_scrollable_text(scrollable_program_text);
        draw_scrollable_text(scrollable_ast_dump);
        
        draw_and_update_button_menu(world);

        do_frame_timing(&global_input, &world->verbose_frame_times);
        do_physical_pixels_switch(&global_input);
    }
}
