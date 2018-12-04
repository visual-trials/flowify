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
    Token_Comma,
    
    Token_String,
    Token_Number,
    Token_Float,
    Token_Identifier,
    Token_VariableIdentifier,  // starts with $
    
    Token_If,
    Token_Else,
    Token_For,
    
    Token_Function,
    Token_Break,
    Token_Continue,
    Token_Return,
    
    Token_Assign,
    Token_AssignMultiply,
    Token_AssignDivide,
    Token_AssignPlus,
    Token_AssignMinus,
    
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
    
    Token_EndOfStream    
};

struct Token
{
    i32 type; // TODO: somehow we can't use TokenType as the type in win32. Using i32 instead.
    
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
    
    // Statements
    Node_Stmt_If,
    Node_Stmt_If_Cond,
    Node_Stmt_If_Then,
    Node_Stmt_If_Else,
    
    Node_Stmt_For,
    Node_Stmt_For_Init,
    Node_Stmt_For_Cond,
    Node_Stmt_For_Update,
    Node_Stmt_For_Body,
    
    Node_Stmt_Function,
    Node_Stmt_Function_Args,
    Node_Stmt_Function_Body,
    
    Node_Stmt_Return,
    Node_Stmt_Break,
    Node_Stmt_Continue,
    
    Node_Stmt_Expr,
    
    // Expressions
    Node_Expr_PreInc,
    Node_Expr_PreDec,
    Node_Expr_PostInc,
    Node_Expr_PostDec,
    
    Node_Expr_AssignOp_Multiply,
    Node_Expr_AssignOp_Divide,
    Node_Expr_AssignOp_Plus,
    Node_Expr_AssignOp_Minus,
    Node_Expr_AssignOp_Concat,
    
    Node_Expr_BinaryOp_Multiply,
    Node_Expr_BinaryOp_Divide,
    Node_Expr_BinaryOp_Plus,
    Node_Expr_BinaryOp_Minus,
    Node_Expr_BinaryOp_Smaller,
    Node_Expr_BinaryOp_Greater,
    Node_Expr_BinaryOp_Equal,
    
    Node_Expr_Assign,
    
    Node_Expr_FuncCall,
    
    Node_Expr_Variable,
    
    // Scalars
    Node_Scalar_Number,
    Node_Scalar_Float,
    Node_Scalar_String
};

// TODO: Keep this in sync with the enum above!
// TODO: DON'T FORGET THE COMMAS!!
const char * node_type_names[] = {
    "Unknown",
    
    "Root",
    
    // Statements
    "Stmt_If",
    "Stmt_If_Cond",
    "Stmt_If_Then",
    "Stmt_If_Else",
    
    "Stmt_For",
    "Stmt_For_Init",
    "Stmt_For_Cond",
    "Stmt_For_Update",
    "Stmt_For_Body",
    
    "Stmt_Function",
    "Stmt_Function_Args",
    "Stmt_Function_Body",
    
    "Stmt_Return",
    "Stmt_Break",
    "Stmt_Continue",
    
    "Stmt_Expr",
    
    // Expressions
    "Expr_PreInc",
    "Expr_PreDec",
    "Expr_PostInc",
    "Expr_PostDec",
    
    "Expr_AssignOp_Multiply",
    "Expr_AssignOp_Divide",
    "Expr_AssignOp_Plus",
    "Expr_AssignOp_Minus",
    "Expr_AssignOp_Concat",
    
    "Expr_BinaryOp_Multiply",
    "Expr_BinaryOp_Divide",
    "Expr_BinaryOp_Plus",
    "Expr_BinaryOp_Minus",
    "Expr_BinaryOp_Smaller",
    "Expr_BinaryOp_Greater",
    "Expr_BinaryOp_Equal",
    
    "Expr_Assign",
    
    "Expr_FuncCall",
    
    "Expr_Variable",
    
    // Scalars
    "Scalar_Number",
    "Scalar_Float",
    "Scalar_String"
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
        case ',': {token.type = Token_Comma;} break;
        
        case '=': {
            if (tokenizer->at[0] == '=')
            {
                tokenizer->at++;
                token.type = Token_Equal;
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
            }
            else if (tokenizer->at[0] == '+')
            {
                tokenizer->at++;
                token.type = Token_PlusPlus;
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
            }
            else if (tokenizer->at[0] == '-')
            {
                tokenizer->at++;
                token.type = Token_MinusMinus;
            }
            else
            {
                token.type = Token_Minus;
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
    
    // log("-------");
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

b32 accept_token(Parser * parser, i32 token_type)  // TODO: somehow we can't use TokenType as the type in win32. Using i32 instead.
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

b32 expect_token(Parser * parser, i32 token_type)  // TODO: somehow we can't use TokenType as the type in win32. Using i32 instead.
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

void parse_arguments(Parser * parser, Node * parent_node);
Node * parse_expression(Parser * parser);

void parse_variable_assignment(Parser * parser, Node * sub_expression_node)
{
    // Left side of the expression (a variable)
    Node * variable_node = new_node(parser);
    variable_node->type = Node_Expr_Variable;

    sub_expression_node->first_child = variable_node;

    // Right side of the expression (an expression)
    Node * child_expression_node = parse_expression(parser);

    sub_expression_node->first_child->next_sibling = child_expression_node;
}

Node * parse_sub_expression(Parser * parser)
{
    Node * sub_expression_node = new_node(parser);
    
    if (accept_token(parser, Token_OpenParenteses))
    {
        Node * child_expression = parse_expression(parser);
        
        expect_token(parser, Token_CloseParenteses);
        
        sub_expression_node = child_expression;
    }
    else if (accept_token(parser, Token_PlusPlus))
    {
        expect_token(parser, Token_VariableIdentifier);
        
        // TODO: use token to set variable name inside the Node_Expr_Variable!
        Token * variable_token = get_latest_token(parser);
        
        sub_expression_node->type = Node_Expr_PreInc;
            
        Node * variable_node = new_node(parser);
        variable_node->type = Node_Expr_Variable;
            
        sub_expression_node->first_child = variable_node;
    }
    else if (accept_token(parser, Token_MinusMinus))
    {
        expect_token(parser, Token_VariableIdentifier);
        
        // TODO: use token to set variable name inside the Node_Expr_Variable!
        Token * variable_token = get_latest_token(parser);
        
        sub_expression_node->type = Node_Expr_PreDec;
            
        Node * variable_node = new_node(parser);
        variable_node->type = Node_Expr_Variable;
            
        sub_expression_node->first_child = variable_node;
    }
    else if (accept_token(parser, Token_VariableIdentifier))
    {
        // TODO: use token to set variable name inside the Node_Expr_Variable!
        Token * variable_token = get_latest_token(parser);
        
        if (accept_token(parser, Token_PlusPlus))
        {
            // TODO: we should only allow '++' *right* behind a variableIdentifier!
            sub_expression_node->type = Node_Expr_PostInc;
                
            Node * variable_node = new_node(parser);
            variable_node->type = Node_Expr_Variable;
                
            sub_expression_node->first_child = variable_node;
        }
        else if (accept_token(parser, Token_MinusMinus))
        {
            // TODO: we should only allow '++' *right* behind a variableIdentifier!
            sub_expression_node->type = Node_Expr_PostDec;
                
            Node * variable_node = new_node(parser);
            variable_node->type = Node_Expr_Variable;
                
            sub_expression_node->first_child = variable_node;
        }
        else if (accept_token(parser, Token_AssignMultiply))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Multiply;
            parse_variable_assignment(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_AssignDivide))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Divide;
            parse_variable_assignment(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_AssignPlus))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Plus;
            parse_variable_assignment(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_AssignMinus))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Minus;
            parse_variable_assignment(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_Assign))
        {
            sub_expression_node->type = Node_Expr_Assign;
            parse_variable_assignment(parser, sub_expression_node);
        }
        else
        {
            // If the variable was not followed by anything, we assume the expression only contains the variable
            sub_expression_node->type = Node_Expr_Variable;
        }
    }
    else if (accept_token(parser, Token_Number))
    {
        // TODO: use token to set number inside Node!
        Token * token = get_latest_token(parser);
        
        sub_expression_node->type = Node_Scalar_Number;
    }
    else if (accept_token(parser, Token_Float))
    {
        // TODO: use token to set number inside Node!
        Token * token = get_latest_token(parser);
        
        sub_expression_node->type = Node_Scalar_Float;
    }
    else if (accept_token(parser, Token_String))
    {
        // TODO: use token to set number inside Node!
        Token * token = get_latest_token(parser);
        
        sub_expression_node->type = Node_Scalar_String;
    }
    else if (accept_token(parser, Token_Identifier))
    {
        // TODO: use token to set function name inside the Node_Expr_FuncCall!
        Token * function_call_token = get_latest_token(parser);

        sub_expression_node->type = Node_Expr_FuncCall;
        parse_arguments(parser, sub_expression_node);
    }
    else
    {
        log("ERROR: unknown sub expression!");
        sub_expression_node = 0;  // TODO: we should "free" this sub_expression_node (but an error occured so it might nog matter)
        return sub_expression_node;
    }
    
    return sub_expression_node;
   
}

Node * parse_expression(Parser * parser)
{
    Node * expression_node = new_node(parser);
    
    Node * left_sub_expression = parse_sub_expression(parser);
    
    if (!left_sub_expression)
    {
        // if no expression was found, returning 0 (so the caller known no expression was found)
        expression_node = 0;  // TODO: we should "free" this expression_node (but an error occured so it might nog matter)
        return expression_node;
    }
    
    if (accept_token(parser, Token_Greater))
    {
        expression_node->type = Node_Expr_BinaryOp_Greater;
        Node * right_sub_expression = parse_sub_expression(parser);
        expression_node->first_child = left_sub_expression;
        left_sub_expression->next_sibling = right_sub_expression;
    }
    else if (accept_token(parser, Token_Smaller))
    {
        expression_node->type = Node_Expr_BinaryOp_Smaller;
        Node * right_sub_expression = parse_sub_expression(parser);
        expression_node->first_child = left_sub_expression;
        left_sub_expression->next_sibling = right_sub_expression;
    }
    else if (accept_token(parser, Token_Equal))
    {
        expression_node->type = Node_Expr_BinaryOp_Equal;
        Node * right_sub_expression = parse_sub_expression(parser);
        expression_node->first_child = left_sub_expression;
        left_sub_expression->next_sibling = right_sub_expression;
    }
    else if (accept_token(parser, Token_Multiply))
    {
        expression_node->type = Node_Expr_BinaryOp_Multiply;
        Node * right_sub_expression = parse_sub_expression(parser);
        expression_node->first_child = left_sub_expression;
        left_sub_expression->next_sibling = right_sub_expression;
    }
    else if (accept_token(parser, Token_Divide))
    {
        expression_node->type = Node_Expr_BinaryOp_Divide;
        Node * right_sub_expression = parse_sub_expression(parser);
        expression_node->first_child = left_sub_expression;
        left_sub_expression->next_sibling = right_sub_expression;
    }
    else if (accept_token(parser, Token_Plus))
    {
        expression_node->type = Node_Expr_BinaryOp_Plus;
        Node * right_sub_expression = parse_sub_expression(parser);
        expression_node->first_child = left_sub_expression;
        left_sub_expression->next_sibling = right_sub_expression;
    }
    else if (accept_token(parser, Token_Minus))
    {
        expression_node->type = Node_Expr_BinaryOp_Minus;
        Node * right_sub_expression = parse_sub_expression(parser);
        expression_node->first_child = left_sub_expression;
        left_sub_expression->next_sibling = right_sub_expression;
    }
    else
    {
        expression_node = left_sub_expression;
    }
    
    return expression_node;
}

void parse_arguments(Parser * parser, Node * parent_node)
{
    expect_token(parser, Token_OpenParenteses);
    
    Node * previous_sibling;
    while(!accept_token(parser, Token_CloseParenteses))
    {
        Node * argument_node = parse_expression(parser);
        if (argument_node)
        {
            if (!parent_node->first_child)
            {
                parent_node->first_child = argument_node;
                previous_sibling = argument_node;
            }
            else
            {
                previous_sibling->next_sibling = argument_node;
                previous_sibling = argument_node;
            }
            
            // TODO: Only allow next argument if it comma separated
            accept_token(parser, Token_Comma);
        }
        else
        {
            log("ERROR: invalid argument (not an expression) in function call or declaration");
            break;
        }
    }
}

void parse_block(Parser * parser, Node * parent_node);

Node * parse_statement(Parser * parser)
{
    Node * statement_node = new_node(parser);
    if (accept_token(parser, Token_If))
    {
        statement_node->type = Node_Stmt_If;
        
        // Note: we do not allow single-line bodies atm (without braces)

        // If-condition
        if (!expect_token(parser, Token_OpenParenteses))
        {
            log("ERROR: Expected Token_OpenParenteses but did NOT get it after if-statement!");
        }
        Node * condition_node = new_node(parser);
        condition_node->type = Node_Stmt_If_Cond;
        statement_node->first_child = condition_node;
        
        Node * condition_expression_node = parse_expression(parser);
        condition_node->first_child = condition_expression_node;
        expect_token(parser, Token_CloseParenteses);
        
        // If-then
        Node * then_node = new_node(parser);
        then_node->type = Node_Stmt_If_Then;
        parse_block(parser, then_node);
        
        condition_node->next_sibling = then_node;
        
        if (accept_token(parser, Token_Else))
        {
            // If-else
            Node * else_node = new_node(parser);
            else_node->type = Node_Stmt_If_Else;
            parse_block(parser, else_node);
            
            then_node->next_sibling = else_node;
        }
        
        // Note: if-statemets (or any other block-ending statements) do not require a Semocolon at the end!
    }
    else if (accept_token(parser, Token_For))
    {
        statement_node->type = Node_Stmt_For;
        
        expect_token(parser, Token_OpenParenteses);
        
        Node * init_node = new_node(parser);
        init_node->type = Node_Stmt_For_Init;
        statement_node->first_child = init_node;
        
        Node * init_expression_node = parse_expression(parser);
        init_node->first_child = init_expression_node;
        
        expect_token(parser, Token_Semicolon);
            
        Node * condition_node = new_node(parser);
        condition_node->type = Node_Stmt_For_Cond;
        init_node->next_sibling = condition_node;
        
        Node * condition_expression_node = parse_expression(parser);
        condition_node->first_child = condition_expression_node;
        
        expect_token(parser, Token_Semicolon);
        
        Node * update_node = new_node(parser);
        update_node->type = Node_Stmt_For_Update;
        condition_node->next_sibling = update_node;
        
        Node * update_expression_node = parse_expression(parser);
        update_node->first_child = update_expression_node;
        
        expect_token(parser, Token_CloseParenteses);
        
        // For body
        Node * for_body_node = new_node(parser);
        for_body_node->type = Node_Stmt_For_Body;
        parse_block(parser, for_body_node);
        
        update_node->next_sibling = for_body_node;
        
    }
    else if (accept_token(parser, Token_Function))
    {
        if (expect_token(parser, Token_Identifier))
        {
            // TODO: use token to set function name inside the Node_Expr_Function!
            Token * function_token = get_latest_token(parser);

            statement_node->type = Node_Stmt_Function;
            
            Node * function_arguments_node = new_node(parser);
            function_arguments_node->type = Node_Stmt_Function_Args;
            statement_node->first_child = function_arguments_node;
            
            parse_arguments(parser, function_arguments_node);

            // Function body
            Node * function_body_node = new_node(parser);
            function_body_node->type = Node_Stmt_Function_Body;
            parse_block(parser, function_body_node);
            
            function_arguments_node->next_sibling = function_body_node;
        }
        else
        {
            log("ERROR: no function name found!");
        }
    }
    else if (accept_token(parser, Token_Continue))
    {
        statement_node->type = Node_Stmt_Continue;
        expect_token(parser, Token_Semicolon); 
    }
    else if (accept_token(parser, Token_Break))
    {
        statement_node->type = Node_Stmt_Break;
        expect_token(parser, Token_Semicolon); 
    }
    else if (accept_token(parser, Token_Return))
    {
        statement_node->type = Node_Stmt_Return;
        
        Node * expression_node = parse_expression(parser);
        statement_node->first_child = expression_node;
        
        expect_token(parser, Token_Semicolon); 
    }
    else
    {
        // We assume its a statement with only an expression
        statement_node->type = Node_Stmt_Expr;
        
        Node * expression_node = parse_expression(parser);
        if (!expression_node)
        {
            // We found no expression, so we probably got an error. We return 0;
            log("ERROR: found no expression where expression was expected");
            
            // TODO: create a helper-function that log the current position!
            Tokenizer * tokenizer = parser->tokenizer;
            Token token = tokenizer->tokens[parser->current_token_index];
            log("Next token starts with:");
            log(token.text);
            
            statement_node = 0; // TODO: we should "free" this expression_node (but an error occured so it might nog matter)
            return statement_node;
        }
        statement_node->first_child = expression_node;
        
        expect_token(parser, Token_Semicolon); 
    }
    // TODO implement more variants of statements
    
    return statement_node;
}

void parse_statements(Parser * parser, Node * parent_node)
{
    Node * previous_sibling = 0;
    
    // TODO: only accept EndOfPhp and EndOfStream for program, only accept CloseBrace for block (use an boolean argument to this function)
    while (!accept_token(parser, Token_EndOfStream) && 
           !accept_token(parser, Token_CloseBrace) && 
           !accept_token(parser, Token_EndOfPhp))
    {
        Node * statement_node = parse_statement(parser);
        
        if (!statement_node)
        {
            log("ERROR: No statement found!");
            // No statement was found, even though it was expected. We break the loop.
            // TODO: should we allow empty blocks?
            break;
        }
        
        if (!parent_node->first_child)
        {
            parent_node->first_child = statement_node;
            previous_sibling = statement_node;
        }
        else
        {
            previous_sibling->next_sibling = statement_node;
            previous_sibling = statement_node;
        }
    }
    
    // TODO: if Token_EndOfPhp we should also retrieve/expect the Token_EndOfStream after that!
    
}

void parse_block(Parser * parser, Node * parent_node)
{
    if (expect_token(parser, Token_OpenBrace))
    {
        parse_statements(parser, parent_node);    
        // Note: Token_CloseBrace is already eaten by parse_statements
    }
    else 
    {
        log("ERROR: did NOT found open brace!");
    }

    // TODO: we should probably return a boolean that parser went ok or not
}

Node * parse_program(Parser * parser)
{
    Node * root_node = new_node(parser);
    root_node->type = Node_Root;
    
    if (expect_token(parser, Token_StartOfPhp))
    {
        parse_statements(parser, root_node);    
    }
    else {
        // TODO: Program doesn't start with StartOfPhp
    }
    
    return root_node;
}

void dump_tree(Node * node, String * dump_text, i32 depth = 0)
{
    u8 temp_string[100]; // TODO: use a temp-memory buffer instead
    
    String indent_string = {};
    indent_string.data = temp_string;
    copy_cstring_to_string("    ", &indent_string);
    
    for (i32 indentation_index = 0; indentation_index < depth; indentation_index++)
    {
        append_string(dump_text, &indent_string);
    }
    
    String node_type_string = {};
    node_type_string.data = temp_string;
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
