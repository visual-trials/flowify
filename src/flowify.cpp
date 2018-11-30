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
    Token_Semicolon,
    
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
    u8 * at;
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

b32 is_alpha(char ch)
{
    b32 is_alpha = (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')));

    return is_alpha;
}

b32 is_number(char ch)
{
    b32 is_number = ((ch >= '0') && (ch <= '9'));
    
    return is_number;
}

void eat_all_white_spaces(Tokenizer * tokenizer)
{
    while(true)
    {
        if(is_white_space(tokenizer->at[0]))
        {
            tokenizer->at++;
        }
        else if((tokenizer->at[0] == '/') && (tokenizer->at[1] == '/'))
        {
            tokenizer->at += 2;
            while(tokenizer->at[0] && !is_end_of_line(tokenizer->at[0]))
            {
                tokenizer->at++;
            }
        }
        else if((tokenizer->at[0] == '/') && (tokenizer->at[1] == '*'))
        {
            tokenizer->at += 2;
            while(tokenizer->at[0] && !((tokenizer->at[0] == '*') && (tokenizer->at[1] == '/')))
            {
                tokenizer->at++;
            }
            
            if(tokenizer->at[0] == '*')
            {
                tokenizer->at += 2;
            }
        }
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
    
    token.text.length = 1;
    token.text.data = tokenizer->at;
    char ch = tokenizer->at[0];
    tokenizer->at++;
    switch(ch)
    {
        case '\0': {token.type = Token_EndOfStream;} break;
        
        case '(': {token.type = Token_OpenParenteses;} break;
        case ')': {token.type = Token_CloseParenteses;} break;
        
        case '[': {token.type = Token_OpenBracket;} break;
        case ']': {token.type = Token_CloseBracket;} break;
        
        case '{': {token.type = Token_OpenBrace;} break;
        case '}': {token.type = Token_CloseBrace;} break;
        
        case ':': {token.type = Token_Colon;} break;
        case ';': {token.type = Token_Semicolon;} break;
        
        case '=': {token.type = Token_Equals;} break;
        case '+': {token.type = Token_Plus;} break;
        case '-': {token.type = Token_Minus;} break;
        
        case '<': {
            if (tokenizer->at[0] == '?')
            {
                tokenizer->at++;
                
                // TODO: right now all tags starting with '<?' are assumed to be StartPhp-tokens
                token.type = Token_StartOfPhp;
                
                // TODO: we assume the '<?'-tag only contains alpha characters
                while(is_alpha(tokenizer->at[0])) 
                {
                    tokenizer->at++;
                }
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_SmallerThan;
            }
            
        } break;
        
        case '>': {token.type = Token_GreaterThan;} break;
        
        case '?': {
            if (tokenizer->at[0] == '>')
            {
                tokenizer->at++;
                // TODO: we assume that all tags that end with '?>' are EndPhp-tokens
                token.type = Token_EndOfPhp;
            }
            else
            {
                // TODO: we do not support '??' and inline if's using '.. ? .. : ..'
                token.type = Token_Unknown;
            }
            
        } break;
        
        case '$':
        {
            token.type = Token_VariableIdentifier;
            // TODO: should a variable name always begin with a alpha character in php?
            while(is_alpha(tokenizer->at[0]) || is_number(tokenizer->at[0]) || (tokenizer->at[0] == '_'))
            {
                tokenizer->at++;
            }
            token.text.length = tokenizer->at - token.text.data;
            
        } break;
        
        default:
        {
            if (is_alpha(ch))
            {
                token.type = Token_Identifier;
                while(is_alpha(tokenizer->at[0]) || is_number(tokenizer->at[0]) || (tokenizer->at[0] == '_'))
                {
                    tokenizer->at++;
                }
                token.text.length = tokenizer->at - token.text.data;
            }
            if (is_number(ch))
            {
                // TODO: implement constant floats
                
                token.type = Token_Number;
                while(is_number(tokenizer->at[0]))
                {
                    tokenizer->at++;
                }
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Unknown;
            }
        }
        
    }
    
    return token;
};


// 


struct WorldData
{
    String program_text;
    String program_lines[1000]; // TODO: allocate this properly!
    
    Token tokens[1000]; // TODO: allocate this properly!
    i32 nr_of_tokens;
    
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

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        
        const char * text_to_parse = simple_if_program_text; //simple_assign_program_text;
        
        world->program_text.data = (u8 *)text_to_parse;
        world->program_text.length = cstring_length((u8 *)text_to_parse);
        
        world->nr_of_lines = split_string_into_lines(world->program_text, world->program_lines);

        world->nr_of_tokens = 0;        
        
        Tokenizer tokenizer = {};
        tokenizer.at = (u8 *)text_to_parse;

        b32 tokenizing = true;
        while(tokenizing)
        {
            Token token = get_token(&tokenizer);
            switch(token.type)
            {
                case Token_EndOfStream:
                {
                    tokenizing = false;
                } break;
                
                default:
                {
                    world->tokens[world->nr_of_tokens++] = token;
                } break;
            }
        }

        for (i32 token_index = 0; token_index < world->nr_of_tokens; token_index++)
        {
            Token token = world->tokens[token_index];
                    
            ShortString token_integer;
            int_to_string(token.type, &token_integer);
            log("Token found: ");
            log(&token.text);
            log(&token_integer);
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