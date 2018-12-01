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
    
    String text; // TODO: do want store explicitly the line number and column in the source text? 
};

struct Tokenizer
{
    u8 * at;
    
    Token tokens[1000]; // TODO: allocate this properly!
    i32 nr_of_tokens;
};

// TODO: Keep this in sync with node_type_names below!
enum NodeType
{
    Node_Unknown,
 
    Node_Root, 
    
    // Node_Stmt_Expr, // TODO: in php this is used. We use Stmt_Assign.
    Node_Stmt_Assign,
    Node_Stmt_If,
    Node_Stmt_FunctionDecl,
    Node_Stmt_FunctionCall,
    
    // Node_Expr_Assign, // TODO: in php this is used. We use Stmt_Assign.
    Node_Expr_BinPlus,
    Node_Expr_BinMinus,
    
    Node_Variable,
    Node_Number,
    Node_String
};

// TODO: Keep this in sync with the enum above!
const char * node_type_names[] = {
    "Unknown",
    
    "Root",
    
    // "Stmt_Expr", // TODO: in php this is used. We use Stmt_Assign.
    "Stmt_Assign",
    "Stmt_If",
    "Stmt_FunctionDecl",
    "Stmt_FunctionCall",
    
    // "Expr_Assign", // TODO: in php this is used. We use Stmt_Assign.
    "Expr_BinPlus",
    "Expr_BinMinus",
    
    "Variable",
    "Number",
    "String"
};

struct Node
{
    NodeType type;
    
    // TODO: store from-to tokens (and/or positions in text)
    
    Node * next_sibling;
    Node * first_child;
};

struct Parser
{
    Tokenizer * tokenizer;
    i32 current_token_index;

    Node nodes[1000]; // TODO: allocate this properly!
    i32 nr_of_nodes;
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

void tokenize (Tokenizer * tokenizer)
{
    b32 tokenizing = true;
    while(tokenizing)
    {
        Token token = get_token(tokenizer);
        tokenizer->tokens[tokenizer->nr_of_tokens++] = token;
        if (token.type == Token_EndOfStream)
        {
            tokenizing = false;
        }
    }
}

// Parser

void next_token(Parser * parser)
{
    // FIXME: we should check if we reached the last token!
    parser->current_token_index++;
}

Token * get_latest_token(Parser * parser)
{
    Tokenizer * tokenizer = parser->tokenizer;
    
    // FIXME: check bounds!    
    Token * token = &tokenizer->tokens[parser->current_token_index - 1];
    return token;
}

b32 accept_token(Parser * parser, TokenType token_type)
{
    Tokenizer * tokenizer = parser->tokenizer;

    Token token = tokenizer->tokens[parser->current_token_index];
    if (token.type == token_type)
    {
        next_token(parser);
        return true;
    }
    return false;
}

b32 expect_token(Parser * parser, TokenType token_type)
{
    if (accept_token(parser, token_type))
    {
        return true;
    }
    // FIXME: give a better error!
    log("ERROR: required token not found!");
    return false;
}

Node * new_node(Parser * parser)
{
    Node * new_node = &parser->nodes[parser->nr_of_nodes++];
    new_node->first_child = 0;
    new_node->next_sibling = 0;
    new_node->type = Node_Unknown;
    return new_node;
}

Node * parse_expression(Parser * parser)
{
    Node * expression_node = new_node(parser);
    
    if (accept_token(parser, Token_Number))
    {
        // TODO: use token to set number inside Node!
        Token * token = get_latest_token(parser);
        
        expression_node->type = Node_Number;
    }
    // TODO: implement more variants of expressions
    else
    {
        // if no expression was found, returning 0 (so the caller known no expression was found)
        expression_node = 0;  // TODO: we should "free" this expression_node (but an error occured so it might nog matter)
        return expression_node;
    }
    
    return expression_node;
}

Node * parse_statement(Parser * parser)
{
    Node * statement_node = new_node(parser);
    if (accept_token(parser, Token_VariableIdentifier))
    {
        // TODO: use token to set variable name inside the Node_Variable!
        Token * variable_token = get_latest_token(parser);
        
        statement_node->type = Node_Stmt_Assign;
        
        expect_token(parser, Token_Equals); 
        Node * expression_node = parse_expression(parser);
        expect_token(parser, Token_Semicolon); 
        
        Node * variable_node = new_node(parser);
        variable_node->type = Node_Variable;
        
        statement_node->first_child = variable_node;
        
        statement_node->first_child->next_sibling = expression_node;
    }
    else {
        // if no statement was found, returning 0 (so the caller known no statement was found)
        statement_node = 0;  // TODO: we should "free" this statement_node (but an error occured so it might nog matter)
        return statement_node;
    }
    // TODO implement more variants of statements
    
    return statement_node;
}

Node * parse_program(Parser * parser)
{
    Node * root_node = new_node(parser);
    root_node->type = Node_Root;
    
    Node * previous_sibling = 0;
    if (expect_token(parser, Token_StartOfPhp))
    {
        
        while (!accept_token(parser, Token_EndOfStream))  // TODO: also stop if '}' is reached (but only in "block") 
        {
            Node * statement_node = parse_statement(parser);
            
            if (!statement_node)
            {
                // No statement was found, even though it was expected. We break the loop.
                break;
            }
            
            if (!root_node->first_child)
            {
                root_node->first_child = statement_node;
                previous_sibling = statement_node;
            }
            else
            {
                previous_sibling->next_sibling = statement_node;
                previous_sibling = statement_node;
            }
        } 
            
    }
    else {
        // TODO: Program doesn't start with StartOfPhp
    }
    
    return root_node;
}

void dump_tree(Node * node, String * dump_text, i32 depth = 0)
{
    const char indent[] = "    "; 
    String indent_string = {};
    copy_cstring_to_string(indent, &indent_string);
    
    for (i32 indentation_index = 0; indentation_index < depth; indentation_index++)
    {
        append_string(dump_text, &indent_string);
    }

    String node_type_string = {};
    copy_cstring_to_string(node_type_names[node->type], &node_type_string);
    append_string(dump_text, &node_type_string);
    
    dump_text->data[dump_text->length] = '\n';
    dump_text->length++;
    
    if (node->first_child)
    {
        dump_tree(node->first_child, dump_text, depth + 1);
    }

    if (node->next_sibling)
    {
        dump_tree(node->next_sibling, dump_text, depth);
    }
}