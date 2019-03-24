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

#include "tokenizer.c"

// Parser

/*

= Grammar used =

program = "<?php" statements ["?>"]

block = "{" statements "}"

statements = { statement }

statement = 
      "if" "(" expr ")" block [ else block ]
    | "for" "(" expr ";" expr ";" expr ")" block
    | "foreach" "(" expr "as" [ var "=>" ] ["&"]var ")" block
    | "function" identifier arguments block
    | "continue" ";"
    | "break" ";"
    | "return" expr ";"
    | expr ";"

arguments =
    "(" [ [&]expr { ", " ["&"]expr } ] ")"

expr =
    sub_expr "<" sub_expr
    sub_expr ">" sub_expr
    sub_expr "-" sub_expr
    sub_expr "+" sub_expr
    sub_expr "*" sub_expr
    sub_expr "==" sub_expr
    sub_expr
    
sub_expr :=
    "(" expr ")"
    "++" var
    "--" var
    var "[" expr "]"
    var "++"
    var "--"
    var "*=" expr
    var "/=" expr
    var "+=" expr
    var "-=" expr
    var "="  expr
    var
    number
    float
    single_quoted_string
    identifier arguments
    
*/

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
    
    Node_Stmt_Foreach,
    Node_Stmt_Foreach_Array,
    Node_Stmt_Foreach_Key_Var,
    Node_Stmt_Foreach_Value_Var,
    Node_Stmt_Foreach_Body,
    
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
    
    "Stmt_Foreach",
    "Stmt_Foreach_Array",
    "Stmt_Foreach_Key_Var",
    "Stmt_Foreach_Value_Var",
    "Stmt_Foreach_Body",
    
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
    
    String identifier;
    String value;
    
    i32 first_token_index;
    i32 last_token_index;
    
    HighlightedLinePart highlighted_line_part;
    
    Node * first_child;
    Node * last_child;
    
    Node * next_sibling;
};

struct Parser
{
    Tokenizer * tokenizer;
    i32 current_token_index;

    DynamicArray nodes;
};

void init_parser(Parser * parser, Tokenizer * tokenizer)
{
    parser->current_token_index = 0;
    parser->tokenizer = tokenizer;

    init_dynamic_array(&parser->nodes, sizeof(Node), (Color4){255,0,255,255}, cstring_to_string("Parser"));
}

void next_token(Parser * parser)
{
    assert(parser->current_token_index < parser->tokenizer->tokens.nr_of_items);
    
    parser->current_token_index++;
}

i32 latest_eaten_token_index(Parser * parser)
{
    assert(parser->current_token_index - 1 >= 0);
    assert(parser->current_token_index - 1 < parser->tokenizer->tokens.nr_of_items);
    
    return parser->current_token_index - 1;
}

Token * latest_eaten_token(Parser * parser)
{
    Tokenizer * tokenizer = parser->tokenizer;
    
    Token * tokens = (Token *)tokenizer->tokens.items;
    Token * token = &tokens[latest_eaten_token_index(parser)];
    
    return token;
}

b32 accept_token(Parser * parser, i32 token_type)  // TODO: somehow we can't use TokenType as the type in win32. Using i32 instead.
{
    Tokenizer * tokenizer = parser->tokenizer;

    Token * tokens = (Token *)tokenizer->tokens.items;
    Token * token = &tokens[parser->current_token_index];
    
    if (token->type == token_type)
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
    Node empty_node = {};
    Node * new_node = (Node *)add_to_array(&parser->nodes, &empty_node);
    
    new_node->first_child = 0;
    new_node->last_child = 0;
    new_node->next_sibling = 0;
    new_node->type = Node_Unknown;
    return new_node;
}

void add_child_node(Node * child_node, Node * parent_node)
{
    if (!parent_node->first_child)
    {
        parent_node->first_child = child_node;
    }
    else
    {
        parent_node->last_child->next_sibling = child_node;
    }
    parent_node->last_child = child_node;
}

String get_source_text_from_ast_node(Parser * parser, Node * node)
{
    String source_text = {};
    
    if (node)
    {
        Tokenizer * tokenizer = parser->tokenizer;
        
        Token * tokens = (Token *)tokenizer->tokens.items;
        Token * start_token = &tokens[node->first_token_index];
        Token * end_token = &tokens[node->last_token_index];
        
        source_text.data = start_token->text.data;
        source_text.length = end_token->text.data + end_token->text.length - start_token->text.data;
    }
    
    return source_text;
}

void parse_arguments(Parser * parser, Node * parent_node);
Node * parse_expression(Parser * parser);

Node * add_child_expression_node(Parser * parser, NodeType node_type, Node * parent_node)
{
    Node * child_node = new_node(parser);
    
    child_node->type = node_type;
    child_node->first_token_index = parser->current_token_index;

    Node * child_expression_node = parse_expression(parser);
    
    add_child_node(child_expression_node, child_node);
    
    child_node->last_token_index = latest_eaten_token_index(parser);
    
    add_child_node(child_node, parent_node);
    
    return child_node;
}

Node * add_child_variable_node(Parser * parser, NodeType node_type, Node * parent_node, b32 allow_variable_reference = false)
{
    i32 first_token_index = parser->current_token_index;
    
    if (allow_variable_reference)
    {
        if (accept_token(parser, Token_Ampersand))
        {
            // FIXME: do something with the &: set "variable_node->by_reference = true"
        }
    }
   
    expect_token(parser, Token_VariableIdentifier);
    Token * variable_token = latest_eaten_token(parser);
    
    Node * variable_node = new_node(parser);
    variable_node->type = node_type;
    variable_node->identifier = variable_token->text;
    
    variable_node->first_token_index = first_token_index;
    variable_node->last_token_index = latest_eaten_token_index(parser);
    
    add_child_node(variable_node, parent_node);
    
    return variable_node;
}

void parse_variable_assignment(Parser * parser, Node * sub_expression_node, Token * variable_token)
{
    // Left side of the expression (a variable)
    Node * variable_node = new_node(parser);
    variable_node->type = Node_Expr_Variable;
    variable_node->identifier = variable_token->text;

    variable_node->first_token_index = sub_expression_node->first_token_index; // The sub expression starts with the variable, so we take its first_token_index
    variable_node->last_token_index = sub_expression_node->first_token_index; // We assume the variable only takes one token
    
    add_child_node(variable_node, sub_expression_node);

    // Right side of the expression (an expression)
    Node * child_expression_node = parse_expression(parser);

    add_child_node(child_expression_node, sub_expression_node);
}

Node * parse_sub_expression(Parser * parser)
{
    Node * sub_expression_node = 0;
    
    if (accept_token(parser, Token_OpenParenteses))
    {
        i32 first_token_index = latest_eaten_token_index(parser);
        
        sub_expression_node = parse_expression(parser);
        sub_expression_node->first_token_index = first_token_index;
        
        expect_token(parser, Token_CloseParenteses);
        
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_PlusPlus))
    {
        sub_expression_node = new_node(parser);
        sub_expression_node->first_token_index = latest_eaten_token_index(parser);
        sub_expression_node->type = Node_Expr_PreInc;
        add_child_variable_node(parser, Node_Expr_Variable, sub_expression_node);
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_MinusMinus))
    {
        sub_expression_node = new_node(parser);
        sub_expression_node->first_token_index = latest_eaten_token_index(parser);
        sub_expression_node->type = Node_Expr_PreDec;
        add_child_variable_node(parser, Node_Expr_Variable, sub_expression_node);
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_VariableIdentifier))
    {
        sub_expression_node = new_node(parser);
        sub_expression_node->first_token_index = latest_eaten_token_index(parser);
        
        Token * variable_token = latest_eaten_token(parser);
        
        if (accept_token(parser, Token_OpenBracket))
        {
            i32 first_token_index = latest_eaten_token_index(parser);
            
            sub_expression_node = parse_expression(parser);
            sub_expression_node->first_token_index = first_token_index;
            
            expect_token(parser, Token_CloseBracket);
            
            sub_expression_node->last_token_index = latest_eaten_token_index(parser);
        }
        else if (accept_token(parser, Token_PlusPlus))
        {
            // TODO: we should only allow '++' *right* behind a variableIdentifier!
            sub_expression_node->type = Node_Expr_PostInc;
                
            Node * variable_node = new_node(parser);
            variable_node->type = Node_Expr_Variable;
                
            variable_node->first_token_index = sub_expression_node->first_token_index; // The sub expression starts with the variable, so we take its first_token_index
            variable_node->last_token_index = sub_expression_node->first_token_index; // We assume the variable only takes one token
            
            add_child_node(variable_node, sub_expression_node);
        }
        else if (accept_token(parser, Token_MinusMinus))
        {
            // TODO: we should only allow '--' *right* behind a variableIdentifier!
            sub_expression_node->type = Node_Expr_PostDec;
                
            Node * variable_node = new_node(parser);
            variable_node->type = Node_Expr_Variable;
                
            variable_node->first_token_index = sub_expression_node->first_token_index; // The sub expression starts with the variable, so we take its first_token_index
            variable_node->last_token_index = sub_expression_node->first_token_index; // We assume the variable only takes one token

            add_child_node(variable_node, sub_expression_node);
        }
        else if (accept_token(parser, Token_AssignMultiply))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Multiply;
            parse_variable_assignment(parser, sub_expression_node, variable_token);
        }
        else if (accept_token(parser, Token_AssignDivide))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Divide;
            parse_variable_assignment(parser, sub_expression_node, variable_token);
        }
        else if (accept_token(parser, Token_AssignPlus))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Plus;
            parse_variable_assignment(parser, sub_expression_node, variable_token);
        }
        else if (accept_token(parser, Token_AssignMinus))
        {
            sub_expression_node->type = Node_Expr_AssignOp_Minus;
            parse_variable_assignment(parser, sub_expression_node, variable_token);
        }
        else if (accept_token(parser, Token_Assign))
        {
            sub_expression_node->type = Node_Expr_Assign;
            parse_variable_assignment(parser, sub_expression_node, variable_token);
        }
        else
        {
            // If the variable was not followed by anything, we assume the expression only contains the variable
            sub_expression_node->type = Node_Expr_Variable;
        }
        
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Number))
    {
        sub_expression_node = new_node(parser);
        sub_expression_node->first_token_index = latest_eaten_token_index(parser);
        
        Token * number_token = latest_eaten_token(parser);
        
        sub_expression_node->type = Node_Scalar_Number;
        sub_expression_node->value = number_token->text;
        
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Float))
    {
        sub_expression_node = new_node(parser);
        sub_expression_node->first_token_index = latest_eaten_token_index(parser);
        
        Token * float_token = latest_eaten_token(parser);
        
        sub_expression_node->type = Node_Scalar_Float;
        sub_expression_node->value = float_token->text;
        
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_SingleQuotedString))
    {
        sub_expression_node = new_node(parser);
        sub_expression_node->first_token_index = latest_eaten_token_index(parser);
        
        Token * string_token = latest_eaten_token(parser);
        
        sub_expression_node->type = Node_Scalar_String;
        sub_expression_node->value = string_token->text;
        
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Identifier))
    {
        sub_expression_node = new_node(parser);
        sub_expression_node->first_token_index = latest_eaten_token_index(parser);
        
        Token * function_call_identifier_token = latest_eaten_token(parser);

        sub_expression_node->type = Node_Expr_FuncCall;
        sub_expression_node->identifier = function_call_identifier_token->text;
        parse_arguments(parser, sub_expression_node);
        
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else
    {
        Token * latest_token = latest_eaten_token(parser);
        log("ERROR: unknown sub expression!");
        log(latest_token->text);
        log_int(latest_token->line_index + 1);
        sub_expression_node = 0;  // TODO: we should "free" this sub_expression_node (but an error occured so it might nog matter)
        return sub_expression_node;
    }
    
    return sub_expression_node;
   
}

Node * parse_expression(Parser * parser)
{
    Node * expression_node = 0;
    
    Node * left_sub_expression = parse_sub_expression(parser);
    
    if (!left_sub_expression)
    {
        // if no expression was found, returning 0 (so the caller known no expression was found)
        expression_node = 0;  // TODO: we should "free" this expression_node (but an error occured so it might nog matter)
        return expression_node;
    }
    
    if (accept_token(parser, Token_Greater))
    {
        expression_node = new_node(parser);
        expression_node->first_token_index = left_sub_expression->first_token_index;
        expression_node->type = Node_Expr_BinaryOp_Greater;
        Node * right_sub_expression = parse_sub_expression(parser);
        add_child_node(left_sub_expression, expression_node);
        add_child_node(right_sub_expression, expression_node);
        expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Smaller))
    {
        expression_node = new_node(parser);
        expression_node->first_token_index = left_sub_expression->first_token_index;
        expression_node->type = Node_Expr_BinaryOp_Smaller;
        Node * right_sub_expression = parse_sub_expression(parser);
        add_child_node(left_sub_expression, expression_node);
        add_child_node(right_sub_expression, expression_node);
        expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Equal))
    {
        expression_node = new_node(parser);
        expression_node->first_token_index = left_sub_expression->first_token_index;
        expression_node->type = Node_Expr_BinaryOp_Equal;
        Node * right_sub_expression = parse_sub_expression(parser);
        add_child_node(left_sub_expression, expression_node);
        add_child_node(right_sub_expression, expression_node);
        expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Multiply))
    {
        expression_node = new_node(parser);
        expression_node->first_token_index = left_sub_expression->first_token_index;
        expression_node->type = Node_Expr_BinaryOp_Multiply;
        Node * right_sub_expression = parse_sub_expression(parser);
        add_child_node(left_sub_expression, expression_node);
        add_child_node(right_sub_expression, expression_node);
        expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Divide))
    {
        expression_node = new_node(parser);
        expression_node->first_token_index = left_sub_expression->first_token_index;
        expression_node->type = Node_Expr_BinaryOp_Divide;
        Node * right_sub_expression = parse_sub_expression(parser);
        add_child_node(left_sub_expression, expression_node);
        add_child_node(right_sub_expression, expression_node);
        expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Plus))
    {
        expression_node = new_node(parser);
        expression_node->first_token_index = left_sub_expression->first_token_index;
        expression_node->type = Node_Expr_BinaryOp_Plus;
        Node * right_sub_expression = parse_sub_expression(parser);
        add_child_node(left_sub_expression, expression_node);
        add_child_node(right_sub_expression, expression_node);
        expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Minus))
    {
        expression_node = new_node(parser);
        expression_node->first_token_index = left_sub_expression->first_token_index;
        expression_node->type = Node_Expr_BinaryOp_Minus;
        Node * right_sub_expression = parse_sub_expression(parser);
        add_child_node(left_sub_expression, expression_node);
        add_child_node(right_sub_expression, expression_node);
        expression_node->last_token_index = latest_eaten_token_index(parser);
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
    
    parent_node->first_token_index = latest_eaten_token_index(parser);
    
    while(!accept_token(parser, Token_CloseParenteses))
    {
        if (accept_token(parser, Token_Ampersand))
        {
            // FIXME: do something with the &
        }
        
        Node * argument_node = parse_expression(parser);
        if (argument_node)
        {
            add_child_node(argument_node, parent_node);
            
            // TODO: Only allow next argument if it comma separated
            accept_token(parser, Token_Comma);
        }
        else
        {
            log("ERROR: invalid argument (not an expression) in function call or declaration");
            break;
        }
    }
    parent_node->last_token_index = latest_eaten_token_index(parser);
}

void parse_block(Parser * parser, Node * parent_node);


Node * parse_statement(Parser * parser)
{
    Node * statement_node = new_node(parser);
    if (accept_token(parser, Token_If))
    {
        statement_node->first_token_index = latest_eaten_token_index(parser);
        
        statement_node->type = Node_Stmt_If;
        
        // Note: we do not allow single-line bodies atm (without braces)

        // If-condition
        if (!expect_token(parser, Token_OpenParenteses))
        {
            log("ERROR: Expected Token_OpenParenteses but did NOT get it after if-statement!");
        }
        Node * condition_node = new_node(parser);
        condition_node->type = Node_Stmt_If_Cond;
        condition_node->first_token_index = parser->current_token_index;
        add_child_node(condition_node, statement_node);
        
        Node * condition_expression_node = parse_expression(parser);
        condition_node->last_token_index = latest_eaten_token_index(parser);
        add_child_node(condition_expression_node, condition_node);
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
        
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_For))
    {
        // For
        statement_node->first_token_index = latest_eaten_token_index(parser);
        statement_node->type = Node_Stmt_For;
        
        expect_token(parser, Token_OpenParenteses);
        
        // For_Init
        add_child_expression_node(parser, Node_Stmt_For_Init, statement_node);
        
        expect_token(parser, Token_Semicolon);
        
        // For_Cond
        add_child_expression_node(parser, Node_Stmt_For_Cond, statement_node);
        
        expect_token(parser, Token_Semicolon);
        
        // For_Update
        add_child_expression_node(parser, Node_Stmt_For_Update, statement_node);
        
        expect_token(parser, Token_CloseParenteses);
        
        // For_Body
        Node * for_body_node = new_node(parser);
        for_body_node->type = Node_Stmt_For_Body;
        parse_block(parser, for_body_node);
        
        add_child_node(for_body_node, statement_node);
        
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Foreach))
    {
        // Foreach
        statement_node->first_token_index = latest_eaten_token_index(parser);
        statement_node->type = Node_Stmt_Foreach;
        expect_token(parser, Token_OpenParenteses);
        
        // Foreach_Array
        add_child_expression_node(parser, Node_Stmt_Foreach_Array, statement_node);
        
        expect_token(parser, Token_As);

        // Foreach_Value_Var
        
        // We always expect a variable, which (by default) is the Foreach_Value_Var
        add_child_variable_node(parser, Node_Stmt_Foreach_Value_Var, statement_node, true);
        
        if (accept_token(parser, Token_Arrow)) {
            
            // If there is an "=>", the first variable was the Key_Var instead, the next variable becomes the Value_Var
            Node * key_var_node = statement_node->last_child;
            key_var_node->type = Node_Stmt_Foreach_Key_Var;  // Foreach_Value_Var --becomes--> Foreach_Key_Var

            add_child_variable_node(parser, Node_Stmt_Foreach_Value_Var, statement_node, true);
        }
        
        expect_token(parser, Token_CloseParenteses);
        
        // Foreach_Body
        Node * foreach_body_node = new_node(parser);
        foreach_body_node->type = Node_Stmt_Foreach_Body;
        parse_block(parser, foreach_body_node);
        
        add_child_node(foreach_body_node, statement_node);
        
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Function))
    {
        statement_node->first_token_index = latest_eaten_token_index(parser);
        
        if (expect_token(parser, Token_Identifier))
        {
            Token * function_identifier_token = latest_eaten_token(parser);

            statement_node->type = Node_Stmt_Function;
            statement_node->identifier = function_identifier_token->text;
            
            Node * function_arguments_node = new_node(parser);
            function_arguments_node->type = Node_Stmt_Function_Args;
            add_child_node(function_arguments_node, statement_node);
            
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
        
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Continue))
    {
        statement_node->first_token_index = latest_eaten_token_index(parser);
        
        statement_node->type = Node_Stmt_Continue;
        expect_token(parser, Token_Semicolon); 
        
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Break))
    {
        statement_node->first_token_index = latest_eaten_token_index(parser);
        
        statement_node->type = Node_Stmt_Break;
        expect_token(parser, Token_Semicolon); 
        
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_Return))
    {
        add_child_expression_node(parser, Node_Stmt_Return, statement_node);
        
        expect_token(parser, Token_Semicolon); 
            
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    else
    {
        statement_node->first_token_index = parser->current_token_index; // Note: the expression hasn't started yet, so no need to do - 1 here
        
        // We assume its a statement with only an expression
        statement_node->type = Node_Stmt_Expr;
        
        Node * expression_node = parse_expression(parser);
        if (!expression_node)
        {
            // We found no expression, so we probably got an error. We return 0;
            log("ERROR: found no expression where expression was expected");
            
            // TODO: create a helper-function that log the current position!
            Tokenizer * tokenizer = parser->tokenizer;
            Token * tokens = (Token *)tokenizer->tokens.items;
            Token * token = &tokens[parser->current_token_index];
            log("Next token starts with:");
            log(token->text);
            
            statement_node = 0; // TODO: we should "free" this expression_node (but an error occured so it might nog matter)
            return statement_node;
        }
        add_child_node(expression_node, statement_node);
        
        expect_token(parser, Token_Semicolon); 
        
        statement_node->last_token_index = latest_eaten_token_index(parser);
    }
    // TODO implement more variants of statements
    
    return statement_node;
}

void parse_statements(Parser * parser, Node * parent_node)
{
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

        add_child_node(statement_node, parent_node);
    }
    
    // TODO: if Token_EndOfPhp we should also retrieve/expect the Token_EndOfStream after that!
    
}

void parse_block(Parser * parser, Node * parent_node)
{
    if (expect_token(parser, Token_OpenBrace))
    {
        parent_node->first_token_index = latest_eaten_token_index(parser);
        
        parse_statements(parser, parent_node);    
        // Note: Token_CloseBrace is already eaten by parse_statements
        
        parent_node->last_token_index = latest_eaten_token_index(parser);
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
        root_node->first_token_index = latest_eaten_token_index(parser);
        parse_statements(parser, root_node);    
        root_node->last_token_index = latest_eaten_token_index(parser);
    }
    else {
        // TODO: Program doesn't start with StartOfPhp
    }
    
    return root_node;
}

i32 dump_parse_tree(Node * node, DynamicString * dump_text, i32 dump_line_index = 0, i32 depth = 0)
{
    String node_type_string = cstring_to_string(node_type_names[node->type]);
    for (i32 indentation_index = 0; indentation_index < depth; indentation_index++)
    {
        append_string(dump_text, cstring_to_string("    "));
    }
    append_string(dump_text, node_type_string);
    append_string(dump_text, cstring_to_string("\n"));
    
    node->highlighted_line_part.line_index = dump_line_index++;
    node->highlighted_line_part.start_character_index = (u16)(depth * 4);
    node->highlighted_line_part.length = (u16)node_type_string.length;
    
    if (node->first_child)
    {
        dump_line_index = dump_parse_tree(node->first_child, dump_text, dump_line_index, depth + 1);
    }

    if (node->next_sibling)
    {
        dump_line_index = dump_parse_tree(node->next_sibling, dump_text, dump_line_index, depth);
    }
    
    return dump_line_index;
}
