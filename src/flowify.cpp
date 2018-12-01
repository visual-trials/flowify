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
    String program_lines[1000]; // TODO: allocate this properly!
    i32 nr_of_lines;
    
    String dump_text;
};

WorldData global_world = {};  // FIXME: allocate this properly!

// FIXME: CAREFUL WE ARE AT THE LIMIT!!!
u8 global_dump_text[1000]; // TODO: allocate this properly!

const char * simple_assign_program_text = 
    "<?php\n"
    "\n"
    "$answer = 42;\n"
;

const char * simple_if_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "\n"
    "if ($b > 50) {\n"
    "	$b = 50;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        const char * text_to_parse = simple_assign_program_text;
        
        world->program_text.data = (u8 *)text_to_parse;
        world->program_text.length = cstring_length((u8 *)text_to_parse);
        
        world->nr_of_lines = split_string_into_lines(world->program_text, world->program_lines);

        Tokenizer tokenizer = {};
        tokenizer.at = (u8 *)text_to_parse;

        tokenize(&tokenizer);

        Parser parser = {};
        parser.tokenizer = &tokenizer;
        
        Node * root_node = parse_program(&parser);
        
        world->dump_text.length = 0;
        world->dump_text.data = global_dump_text;
        dump_tree(root_node, &world->dump_text);
    }
    
    void update_frame()
    {
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        
        Color4 black = {};
        black.a = 255;
        
        Color4 grey = {};
        grey.a = 100;
        
        Font font = {};
        font.height = 20;
        font.family = Font_CourierNew;
        
        i32 line_margin = 4;
        
        i32 left_margin = 500;
        i32 top_margin = 200;
        
        ShortString line_nr_text;
            
        for (i32 line_index = 0; line_index < world->nr_of_lines; line_index++)
        {
            // Line text
            Pos2d position;
            position.x = left_margin;
            position.y = top_margin + line_index * (font.height + line_margin);
            
            String line_text = world->program_lines[line_index];
            draw_text(position, &line_text, font, black);
            
            // Line number
            Pos2d position_line_nr = position;
            int_to_string(line_index + 1, &line_nr_text);
            Size2d line_nr_size = get_text_size(&line_nr_text, font);
            position_line_nr.x -= 40 + line_nr_size.width;
            draw_text(position_line_nr, &line_nr_text, font, grey);
        }
        
        
        draw_text((Pos2d){100,100}, &world->dump_text, font, black);
        
        
    }
}