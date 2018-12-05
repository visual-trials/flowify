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
#include "parser.cpp"

struct WorldData
{
    String program_text;
    
    ScrollableText scrollable_program_text;  // TODO: allocate this properly!
    
    Tokenizer tokenizer;
    Parser parser;
    
    String dump_text;
    
    ScrollableText scrollable_ast_dump;  // TODO: allocate this properly!
    
    const char * program_texts[10];
    i32 nr_of_program_texts;
    i32 current_program_text_index;
    
    i32 iteration;
    i32 selected_token_index;
    
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

// FIXME: CAREFUL WE ARE AT THE LIMIT!!!
u8 global_dump_text[2000]; // TODO: allocate this properly!

const char * simple_assign_program_text = 
    "<?php\n"
    "\n"
    "$answer = 42;\n"
;

const char * i_plus_plus_program_text = 
    "<?php\n"
    "\n"
    "$i = 7;\n"
    "\n"
    "$b = $i++;\n"
    "\n"
    "$c = ++$b;"
;

const char * simple_functions_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "$extraVar = (19 + 51) * (47 + 31);\n"
    "\n"
    "$c = $myVar + $b;\n"
    "\n"
    "$d = dechex($c);\n"
    "\n"
    "$e = 128;\n"
    "\n"
    "$sq = square($c);\n"
    "\n"
    "$f = average2($b, $e);\n"
    "\n"
    "function square ($toBeSquared) {\n"
    "    $square = $toBeSquared * $toBeSquared;\n"
    "\n"
    "    return  $square;\n"
    "}\n"
    "\n"
    "function average2 ($first, $second) {\n"
    "    $average = ($first + $second) / 2;\n"
    "\n"
    "    return $average;\n"
    "}"
;

const char * simple_if_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "if ($b > 50) {\n"
    "	$b = 50;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

const char * simple_if_else_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "\n"
    "if ($b > 50) {\n"
    "   $b = 50;\n"
    "}\n"
    "else {\n"
    "    $b = 30;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

const char * simple_for_continue_break_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "for ($i = 0; $i < 10; $i = $i + 1) {\n"
    "    if ($b < 20) {\n"
    "        break;\n"
    "    }\n"
    "    if ($i > 5) {\n"
    "        continue;\n"
    "    }\n"
    "    $b = $b - 5;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

const char * fibonacci_iterative_program_text = 
    "<?php\n"
    "\n"
    "$fib = fibonacci_iterative(10);\n"
    "\n"
    "function fibonacci_iterative($n)\n"
    "{\n"
    "    $a = 0;\n"
    "    $b = 1;\n"
    "    for ($i = 0; $i < $n; $i++) {\n"
    "        $c = $a;\n"
    "        $a = $b;\n"
    "        $b += $c;\n"
    "    }\n"
    "    return $a;\n"
    "}\n"
;

const char * fibonacci_recursive_early_return_program_text = 
    "<?php\n"
    "\n"
    "$fib = fibonacci_recursive(10);\n"
    "\n"
    "function fibonacci_recursive($n)\n"
    "{\n"
    "    if ($n == 0) {\n"
    "        return 0;\n"
    "    }\n"
    "    if ($n == 1) {\n"
    "        return 1;\n"
    "    }\n"
    "    return fibonacci_recursive($n - 1) + fibonacci_recursive($n - 2);\n"
    "}"
;

extern "C" {
    
    
    void load_program_text(const char * program_text, WorldData * world)
    {
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        init_scrollable_text(scrollable_program_text);

        ScrollableText * scrollable_ast_dump = &world->scrollable_ast_dump;
        init_scrollable_text(scrollable_ast_dump, false);
        
        world->program_text.data = (u8 *)program_text;
        world->program_text.length = cstring_length(program_text);
        
        scrollable_program_text->nr_of_lines = split_string_into_lines(world->program_text, scrollable_program_text->lines);
        scrollable_program_text->line_offset = 0;

        // TODO: we need a ZeroStruct function/macro!
        world->tokenizer.nr_of_tokens = 0;
        world->tokenizer.current_line_index = 0;
        world->tokenizer.at = (u8 *)program_text;
        
        Tokenizer * tokenizer = &world->tokenizer;

        tokenize(tokenizer);

        // TODO: we need a ZeroStruct function/macro!
        world->parser.current_token_index = 0;
        world->parser.nr_of_nodes = 0;
        world->parser.tokenizer = tokenizer;
        
        Parser * parser = &world->parser;
        
        Node * root_node = parse_program(parser);
        
        world->dump_text.length = 0;
        world->dump_text.data = global_dump_text;
        dump_tree(root_node, &world->dump_text);
        
        scrollable_ast_dump->nr_of_lines = split_string_into_lines(world->dump_text, scrollable_ast_dump->lines);
        scrollable_ast_dump->line_offset = 0;
    }
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        world->iteration = 0;
        world->selected_token_index = 0;
        
        world->program_texts[0] = simple_assign_program_text;
        world->program_texts[1] = i_plus_plus_program_text;
        world->program_texts[2] = simple_functions_program_text;
        world->program_texts[3] = simple_if_program_text;
        world->program_texts[4] = simple_if_else_program_text;
        world->program_texts[5] = simple_for_continue_break_program_text;
        world->program_texts[6] = fibonacci_iterative_program_text;
        world->program_texts[7] = fibonacci_recursive_early_return_program_text;
        world->nr_of_program_texts = 8;
        
        world->current_program_text_index = 0;
        
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
        
        ScrollableText * scrollable_ast_dump = &world->scrollable_ast_dump;
        
        scrollable_ast_dump->position.x = input->screen.width / 2 - 50; // TODO: calculate by percentage?
        scrollable_ast_dump->position.y = 50; // TODO: where do we want to let this begin?

        scrollable_ast_dump->size.width = input->screen.width - scrollable_ast_dump->position.x;
        scrollable_ast_dump->size.height = input->screen.height - scrollable_ast_dump->position.y - 50;
        
        update_scrollable_text(scrollable_ast_dump, input);
        
        world->iteration++;
        if (world->iteration > 60) // every second
        {
            world->iteration = 0;
            
            world->selected_token_index++;
        }
        if (world->selected_token_index >= world->tokenizer.nr_of_tokens)
        {
            world->selected_token_index = 0;
        }        
        
    }
    
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
        
        // FIXME: hardcoded for now!
        i32 selected_node_index = 3;
        if (world->parser.nr_of_nodes > 0)
        {
            Node node = world->parser.nodes[selected_node_index];
            
            ShortString decimal_string;
            
            int_to_string(node.first_token_index, &decimal_string);
            draw_text((Pos2d){100,100}, &decimal_string, font, black);
            
            int_to_string(node.last_token_index, &decimal_string);
            draw_text((Pos2d){100,140}, &decimal_string, font, black);
            
        }
        
        draw_scrollable_text(scrollable_program_text);
        draw_scrollable_text(scrollable_ast_dump);
        
        draw_and_update_button_menu(world);

        do_frame_timing(&global_input, &world->verbose_frame_times);
        do_physical_pixels_switch(&global_input);
    }
}