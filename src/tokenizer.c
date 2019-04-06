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
 
// TODO: Keep this in sync with token_type_names below!
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
    
    Token_Period,
    Token_Colon,
    Token_Semicolon,
    Token_Comma,
    Token_Ampersand, // &
    
    Token_SingleQuotedString,
    Token_Number,
    Token_Float,
    Token_Identifier,
    Token_VariableIdentifier,  // starts with $
    
    Token_True,
    Token_False,
    Token_Null,
    
    Token_If,
    Token_Else,
    Token_For,
    Token_Foreach,
    Token_As,
    Token_Arrow,  // TODO: What should be the name of this: "=>"
    
    Token_Function,
    Token_Break,
    Token_Continue,
    Token_Return,
    
    Token_Assign,
    Token_AssignMultiply,
    Token_AssignDivide,
    Token_AssignPlus,
    Token_AssignMinus,
    Token_AssignConcat,
    
    Token_Multiply,
    Token_Divide,
    Token_Plus,
    Token_PlusPlus,
    Token_Minus,
    Token_MinusMinus,
    
    Token_Smaller,
    Token_SmallerOrEqual,
    Token_Greater,
    Token_GreaterOrEqual,
    
    Token_Equal,
    Token_NotEqual,
    Token_ExactlyEqual,
    Token_NotExactlyEqual,
    
    Token_LogicalAnd,
    Token_LogicalOr,
    
    Token_EndOfStream    
};

// TODO: Keep this in sync with the enum above!
// TODO: DON'T FORGET THE COMMAS!!
const char * token_type_names[] = {
    "Unknown",
    
    "StartOfPhp",
    "EndOfPhp",
    
    "OpenParenteses",
    "CloseParenteses",
    
    "OpenBracket",
    "CloseBracket",
    
    "OpenBrace",
    "CloseBrace",
    
    "Period",
    "Colon",
    "Semicolon",
    "Comma",
    "Ampersand", // &
    
    "SingleQuotedString",
    "Number",
    "Float",
    "Identifier",
    "VariableIdentifier",  // starts with $
    
    "True",
    "False",
    "Null",
    
    "If",
    "Else",
    "For",
    "Foreach",
    "As",
    "Arrow",  // TODO: What should be the name of this: "=>"
    
    "Function",
    "Break",
    "Continue",
    "Return",
    
    "Assign",
    "AssignMultiply",
    "AssignDivide",
    "AssignPlus",
    "AssignMinus",
    "AssignConcat",
    
    "Multiply",
    "Divide",
    "Plus",
    "PlusPlus",
    "Minus",
    "MinusMinus",
    
    "Smaller",
    "SmallerOrEqual",
    "Greater",
    "GreaterOrEqual",
    
    "Equal",
    "NotEqual",
    "ExactlyEqual",
    "NotExactlyEqual",
    
    "LogicalAnd",
    "LogicalOr",
    
    "EndOfStream"
};

struct Token
{
    i32 type; // TODO: somehow we can't use TokenType as the type in win32. Using i32 instead.
    
    String text;
    
    i32 line_index;
    // TODO: maybe we can calculate this once using split_string_into_scrollable_lines(): 
    // i32 character_in_line_index;
};

struct Tokenizer
{
    u8 * at;
    
    DynamicArray tokens;
    
    i32 current_line_index;
};

void init_tokenizer(Tokenizer * tokenizer)
{
    tokenizer->at = 0;
    tokenizer->current_line_index = 0;

    init_dynamic_array(&tokenizer->tokens, sizeof(Token), (Color4){0,255,0,255}, cstring_to_string("Tokenizer"));
}

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
            // Note: right now we only count '\n' as being a *new* line (since we split on '\n')
            if (tokenizer->at[0] == '\n')
            {
                tokenizer->current_line_index++;
            }
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
                // Note: right now we only count '\n' as being a *new* line (since we split on '\n')
                if (tokenizer->at[0] == '\n')
                {
                    tokenizer->current_line_index++;
                }
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
    // TODO: we should create tokens for white space too (and seperate them between lines)
    eat_all_white_spaces(tokenizer);
    
    Token token = {};
    
    token.text.length = 1;
    token.text.data = tokenizer->at;
    token.line_index = tokenizer->current_line_index;
    
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
        case ',': {token.type = Token_Comma;} break;
        
        case '&': {
            if (tokenizer->at[0] == '&')   // "&&"
            {
                tokenizer->at++;
                token.type = Token_LogicalAnd;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Ampersand;
            }
            
        } break;
        
        case '|': {
            if (tokenizer->at[0] == '|')   // "||"
            {
                tokenizer->at++;
                token.type = Token_LogicalOr;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                // TODO: add the bitwise-or "|" here
                token.type = Token_Unknown;
            }
            
        } break;
        
        case '\'': {
            token.type = Token_SingleQuotedString;
            // TODO: we should allow for escaping inside a single quoted string
            while(tokenizer->at[0] && tokenizer->at[0] != '\'')
            {
                tokenizer->at++;
            }
            tokenizer->at++;
            token.text.length = tokenizer->at - token.text.data;
        } break;
        
        case '!': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                if (tokenizer->at[0] == '=')
                {
                    tokenizer->at++;
                    token.type = Token_NotExactlyEqual;
                    token.text.length = tokenizer->at - token.text.data;
                }
                else
                {
                    token.type = Token_NotEqual;
                    token.text.length = tokenizer->at - token.text.data;
                }
            }
            else
            {
                // TODO: implement other tokens starting with "!"
                token.type = Token_Unknown;
            }
        } break;
        
        case '=': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                if (tokenizer->at[0] == '=')
                {
                    tokenizer->at++;
                    token.type = Token_ExactlyEqual;
                    token.text.length = tokenizer->at - token.text.data;
                }
                else
                {
                    token.type = Token_Equal;
                    token.text.length = tokenizer->at - token.text.data;
                }
            }
            else if (tokenizer->at[0] == '>')   // "=>"
            {
                tokenizer->at++;
                token.type = Token_Arrow;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Assign;
            }
        } break;
        
        case '*': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_AssignMultiply;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Multiply;
            }
        } break;
        case '/': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_AssignDivide;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Divide;
            }
        } break;
        case '+': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_AssignPlus;
                token.text.length = tokenizer->at - token.text.data;
            }
            else if (tokenizer->at[0] == '+')
            {
                tokenizer->at++;
                token.type = Token_PlusPlus;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Plus;
            }
        } break;
        case '-': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_AssignMinus;
                token.text.length = tokenizer->at - token.text.data;
            }
            else if (tokenizer->at[0] == '-')
            {
                tokenizer->at++;
                token.type = Token_MinusMinus;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Minus;
            }
        } break;
        
        case '.': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_AssignConcat;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Period;
            }
        } break;
        
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
            else if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_SmallerOrEqual;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Smaller;
            }
            
        } break;
        
        case '>': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_GreaterOrEqual;
                token.text.length = tokenizer->at - token.text.data;
            }
            else
            {
                token.type = Token_Greater;
            }
        } break;
        
        case '?': {
            if (tokenizer->at[0] == '>')
            {
                tokenizer->at++;
                // TODO: we assume that all tags that end with '?>' are EndPhp-tokens
                token.type = Token_EndOfPhp;
                token.text.length = tokenizer->at - token.text.data;
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
                
                if (equals(token.text, "if"))
                {
                    token.type = Token_If;
                }
                else if (equals(token.text, "else"))
                {
                    token.type = Token_Else;
                }
                else if (equals(token.text, "for"))
                {
                    token.type = Token_For;
                }
                else if (equals(token.text, "foreach"))
                {
                    token.type = Token_Foreach;
                }
                else if (equals(token.text, "as"))
                {
                    token.type = Token_As;
                }
                else if (equals(token.text, "function"))
                {
                    token.type = Token_Function;
                }
                else if (equals(token.text, "break"))
                {
                    token.type = Token_Break;
                }
                else if (equals(token.text, "continue"))
                {
                    token.type = Token_Continue;
                }
                else if (equals(token.text, "return"))
                {
                    token.type = Token_Return;
                }
                else if (equals(token.text, "true"))
                {
                    token.type = Token_True;
                }
                else if (equals(token.text, "false"))
                {
                    token.type = Token_False;
                }
                else if (equals(token.text, "null"))
                {
                    token.type = Token_Null;
                }
                // TODO: add more keywords
                else
                {
                    token.type = Token_Identifier;
                }
            }
            else if (is_number(ch))
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
    
    if (false)
    {
        log(token.text);
    }
    
    return token;
};

void tokenize (Tokenizer * tokenizer, u8 * program_text)
{
    tokenizer->at = program_text;
    
    b32 tokenizing = true;
    while(tokenizing)
    {
        Token new_token = get_token(tokenizer);
        
        add_to_array(&tokenizer->tokens, &new_token);
        
        if (new_token.type == Token_EndOfStream)
        {
            tokenizing = false;
        }
    }
    
    // log("-------");
}
