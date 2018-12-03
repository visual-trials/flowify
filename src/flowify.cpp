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
    
    String dump_text;
    
    ScrollableText scrollable_ast_dump;  // TODO: allocate this properly!
};

WorldData global_world = {};  // FIXME: allocate this properly!

// FIXME: CAREFUL WE ARE AT THE LIMIT!!!
u8 global_dump_text[2000]; // TODO: allocate this properly!

const char * simple_assign_program_text = 
    "<?php\n"
    "\n"
    "$answer = 42;\n"
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

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        init_scrollable_text(scrollable_program_text);

        ScrollableText * scrollable_ast_dump = &world->scrollable_ast_dump;
        init_scrollable_text(scrollable_ast_dump, false);
        
        const char * text_to_parse = fibonacci_iterative_program_text;
        
        world->program_text.data = (u8 *)text_to_parse;
        world->program_text.length = cstring_length(text_to_parse);
        
        scrollable_program_text->nr_of_lines = split_string_into_lines(world->program_text, scrollable_program_text->lines);
        scrollable_program_text->line_offset = 0;

        Tokenizer tokenizer = {};
        tokenizer.at = (u8 *)text_to_parse;

        tokenize(&tokenizer);

        Parser parser = {};
        parser.tokenizer = &tokenizer;
        
        Node * root_node = parse_program(&parser);
        
        world->dump_text.length = 0;
        world->dump_text.data = global_dump_text;
        dump_tree(root_node, &world->dump_text);
        
        scrollable_ast_dump->nr_of_lines = split_string_into_lines(world->dump_text, scrollable_ast_dump->lines);
        scrollable_ast_dump->line_offset = 0;
        
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;

        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        
        // The screen size can change, so we have to update the position and size of the scrollables.
        scrollable_program_text->position.x = 0;
        scrollable_program_text->position.y = 110; // TODO: we should properly account for the height of the text above this

        scrollable_program_text->size.width = input->screen.width - scrollable_program_text->position.x;
        scrollable_program_text->size.height = input->screen.height - scrollable_program_text->position.y;
        
        update_scrollable_text(scrollable_program_text, input);
        
        ScrollableText * scrollable_ast_dump = &world->scrollable_ast_dump;
        
        scrollable_ast_dump->position.x = 600; // TODO: calculate by percentage?
        scrollable_ast_dump->position.y = 110; // TODO: where do we want to let this begin?

        scrollable_ast_dump->size.width = input->screen.width - scrollable_ast_dump->position.x;
        scrollable_ast_dump->size.height = input->screen.height - scrollable_ast_dump->position.y;
        
        update_scrollable_text(scrollable_ast_dump, input);
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        ScrollableText * scrollable_ast_dump = &world->scrollable_ast_dump;
        
        draw_scrollable_text(scrollable_program_text);
        draw_scrollable_text(scrollable_ast_dump);
        
        do_physical_pixels_switch(&global_input);
    }
}