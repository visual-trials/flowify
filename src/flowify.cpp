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

struct WorldData
{
    String program_text;
    String program_lines[1000]; // TODO: allocate this properly!
    
    i32 nr_of_lines;
};

WorldData global_world = {};  // FIXME: allocate this properly!

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

enum TokenType
{
    Token_Unknown,
    
    Token_StartOfPhp,
    Token_EndOfPhp,
    
    Token_OpenParenteses,
    Token_CloseParenteses,
    
    Token_OpenBracket,
    Token_CloseBracket,
    
    Token_OpenBrace,
    Token_CloseBrace,
    
    Token_Colon,
    Token_SemiColon,
    
    Token_String,
    Token_Number,
    Token_Identifier,
    Token_VariableIdentifier,  // starts with $
    
    Token_Equals,
    Token_Plus,
    Token_Minus,
    
    Token_SmallerThan,
    Token_GreaterThan,
    
    Token_EndOfStream    
};

struct Token
{
    TokenType type;
    
    String text;
};

struct Tokenizer
{
    char * at;
};


b32 is_end_of_line(char ch)
{
    b32 is_end_of_line = ((ch == '\n') || (ch == '\r'));

    return is_end_of_line;
}

b32 is_white_space(char ch)
{
    b32 is_white_space = ((ch == ' ') || (ch == '\t') || (ch == '\v') || (ch == '\f') || is_end_of_line(ch));

    return is_white_space;
}

void eat_all_white_spaces(Tokenizer * tokenizer)
{
    while(true)
    {
        if(is_white_space(tokenizer->at[0]))
        {
            tokenizer->at++;
        }
        // TODO: implement comments // and /* */
        else
        {
            break;
        }
    }
};

Token get_token(Tokenizer * tokenizer)
{
    eat_all_white_spaces(tokenizer);
    
    Token token = {};
    
    
    
    
    return token;
};



extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        const char * text_to_parse = simple_assign_program_text;
        
        world->program_text.data = (u8 *)text_to_parse;
        world->program_text.length = cstring_length((u8 *)text_to_parse);
        
        world->nr_of_lines = split_string_into_lines(world->program_text, world->program_lines);
        
        Tokenizer tokenizer = {};
        tokenizer.at = (char *)text_to_parse;

        b32 parsing = true;
        while(parsing)
        {
            Token token = get_token(&tokenizer);
            switch(token.type)
            {
                case Token_EndOfStream:
                {
                    parsing = false;
                } break;
                
                default:
                {
                    // TODO: log unknown token
                } break;
            }
        }
        
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
    }
}