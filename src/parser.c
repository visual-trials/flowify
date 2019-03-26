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

#include "parser.h"

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

void parse_arguments(Parser * parser, Node * parent_node);
Node * parse_expression(Parser * parser);

Node * parse_child_expression_node(Parser * parser, NodeType node_type, Node * parent_node)
{
    Node * child_node = start_node(parser, node_type);

    Node * child_expression_node = parse_expression(parser);
    
    add_child_node(child_expression_node, child_node);
    
    end_node(parser, child_node);
    
    add_child_node(child_node, parent_node);
    
    return child_node;
}

Node * parse_child_variable_node(Parser * parser, NodeType node_type, Node * parent_node, b32 allow_variable_reference = false)
{
    Node * variable_node = start_node(parser, node_type);
    
    if (allow_variable_reference)
    {
        if (accept_token(parser, Token_Ampersand))
        {
            // FIXME: do something with the &: set "variable_node->by_reference = true"
        }
    }
   
    expect_token(parser, Token_VariableIdentifier);
    Token * variable_token = latest_eaten_token(parser);
    
    variable_node->identifier = variable_token->text;
    
    end_node(parser, variable_node);
    
    add_child_node(variable_node, parent_node);
    
    return variable_node;
}

Node * parse_variable_assignment(Parser * parser, NodeType node_type, Token * variable_token, i32 token_index_offset)
{
    Node * sub_expression_node = start_node(parser, node_type, token_index_offset);
    
    // Left side of the expression (a variable)
    Node * variable_node = start_node(parser, Node_Expr_Variable, token_index_offset); // The sub expression starts with the variable, so it has the same starting token
    variable_node->identifier = variable_token->text;
    end_node(parser, variable_node);  // We assume the variable only takes one token
    
    add_child_node(variable_node, sub_expression_node);

    // Right side of the expression (an expression)
    Node * child_expression_node = parse_expression(parser);

    add_child_node(child_expression_node, sub_expression_node);
    
    return sub_expression_node;
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
        sub_expression_node = start_node(parser, Node_Expr_PreInc, StartOnLatestToken);
        
        parse_child_variable_node(parser, Node_Expr_Variable, sub_expression_node);
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_MinusMinus))
    {
        sub_expression_node = start_node(parser, Node_Expr_PreDec, StartOnLatestToken);
        
        parse_child_variable_node(parser, Node_Expr_Variable, sub_expression_node);
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_VariableIdentifier))
    {
        // sub_expression_node = start_node_on_latest_token(parser);
        
        Token * variable_token = latest_eaten_token(parser);
        
        if (accept_token(parser, Token_OpenBracket))
        {
            i32 first_token_index = latest_eaten_token_index(parser);
            
            // FIXME: we lose the current sub_expression_node! (which we just created)
            sub_expression_node = parse_expression(parser);
            sub_expression_node->first_token_index = first_token_index;
            // FIXME: type is not set here!
            
            expect_token(parser, Token_CloseBracket);
            
            sub_expression_node->last_token_index = latest_eaten_token_index(parser);
        }
        else if (accept_token(parser, Token_PlusPlus))
        {
            // TODO: we should only allow '++' *right* behind a variableIdentifier!
            sub_expression_node = start_node(parser, Node_Expr_PostInc, -1);
                
            // The variable_node starts with the variable, and we assume the variable only takes one token
            // FIXME: we should call a function to set the identifier here! (parse_child_variable_node?)
            Node * variable_node = start_node(parser, Node_Expr_Variable, -1);
            end_node(parser, variable_node);
            
            add_child_node(variable_node, sub_expression_node);
            
            end_node(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_MinusMinus))
        {
            // TODO: we should only allow '--' *right* behind a variableIdentifier!
            // FIXME: we should call a function to set the identifier here! (parse_child_variable_node?)
            sub_expression_node = start_node(parser, Node_Expr_PostDec, StartOnLatestToken);
            // sub_expression_node->type = Node_Expr_PostDec;
                
            Node * variable_node = start_node(parser, Node_Expr_Variable, -1);
            end_node(parser, variable_node);

            add_child_node(variable_node, sub_expression_node);
            
            end_node(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_AssignMultiply))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Multiply, variable_token, -2);
        }
        else if (accept_token(parser, Token_AssignDivide))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Divide, variable_token, -2);
        }
        else if (accept_token(parser, Token_AssignPlus))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Plus, variable_token, -2);
        }
        else if (accept_token(parser, Token_AssignMinus))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Minus, variable_token, -2);
        }
        else if (accept_token(parser, Token_Assign))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_Assign, variable_token, -2);
        }
        else
        {
            // If the variable was not followed by anything, we assume the expression only contains the variable
            sub_expression_node = start_node(parser, Node_Expr_Variable, -1);
            end_node(parser, sub_expression_node);
        }
        
    }
    else if (accept_token(parser, Token_Number))
    {
        sub_expression_node = start_node(parser, Node_Scalar_Number, StartOnLatestToken);
        
        Token * number_token = latest_eaten_token(parser);
        sub_expression_node->value = number_token->text;
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_Float))
    {
        sub_expression_node = start_node(parser, Node_Scalar_Float, StartOnLatestToken);
        
        Token * float_token = latest_eaten_token(parser);
        sub_expression_node->value = float_token->text;
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_SingleQuotedString))
    {
        sub_expression_node = start_node(parser, Node_Scalar_String, StartOnLatestToken);
        
        Token * string_token = latest_eaten_token(parser);
        sub_expression_node->value = string_token->text;
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_Identifier))
    {
        sub_expression_node = start_node(parser, Node_Expr_FuncCall, StartOnLatestToken);
        
        Token * function_call_identifier_token = latest_eaten_token(parser);
        sub_expression_node->identifier = function_call_identifier_token->text;
        
        parse_arguments(parser, sub_expression_node);
        
        end_node(parser, sub_expression_node);
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

Node * parse_binary_op_expression(Parser * parser, NodeType node_type, Node * left_sub_expression)
{
    Node * expression_node = start_node(parser, node_type);
    
    Node * right_sub_expression = parse_sub_expression(parser);
    add_child_node(left_sub_expression, expression_node);
    add_child_node(right_sub_expression, expression_node);
    
    end_node(parser, expression_node);
    
    return expression_node;
}

Node * parse_expression(Parser * parser)
{
    Node * expression_node = 0;
    
    Node * left_sub_expression = parse_sub_expression(parser);
    
    if (!left_sub_expression)
    {
        // if no expression was found, returning 0 (so the caller known no expression was found)
        log("ERROR: found no left sub expression");
        expression_node = 0;  // TODO: we should "free" this expression_node (but an error occured so it might nog matter)
        return expression_node;
    }
    
    if (accept_token(parser, Token_Greater))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Greater, left_sub_expression);
    }
    else if (accept_token(parser, Token_Smaller))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Smaller, left_sub_expression);
    }
    else if (accept_token(parser, Token_Equal))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Equal, left_sub_expression);
    }
    else if (accept_token(parser, Token_Multiply))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Multiply, left_sub_expression);
    }
    else if (accept_token(parser, Token_Divide))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Divide, left_sub_expression);
    }
    else if (accept_token(parser, Token_Plus))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Plus, left_sub_expression);
    }
    else if (accept_token(parser, Token_Minus))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Minus, left_sub_expression);
    }
    else
    {
        expression_node = left_sub_expression;
    }
    
    return expression_node;
}

// FIXME: move first_token_index and last_token_index outside this function by using start_node and end_node
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
        // FIXME: shouldnt first_token_index and last_token_index be set for then_node?
        
        parse_block(parser, then_node);
        
        add_child_node(then_node, statement_node);
        
        if (accept_token(parser, Token_Else))
        {
            // If-else
            Node * else_node = new_node(parser);
            else_node->type = Node_Stmt_If_Else;
            // FIXME: shouldnt first_token_index and last_token_index be set for else_node?
            parse_block(parser, else_node);
            
            add_child_node(else_node, statement_node);
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
        parse_child_expression_node(parser, Node_Stmt_For_Init, statement_node);
        
        expect_token(parser, Token_Semicolon);
        
        // For_Cond
        parse_child_expression_node(parser, Node_Stmt_For_Cond, statement_node);
        
        expect_token(parser, Token_Semicolon);
        
        // For_Update
        parse_child_expression_node(parser, Node_Stmt_For_Update, statement_node);
        
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
        parse_child_expression_node(parser, Node_Stmt_Foreach_Array, statement_node);
        
        expect_token(parser, Token_As);

        // Foreach_Value_Var
        
        // We always expect a variable, which (by default) is the Foreach_Value_Var
        parse_child_variable_node(parser, Node_Stmt_Foreach_Value_Var, statement_node, true);
        
        if (accept_token(parser, Token_Arrow)) {
            
            // If there is an "=>", the first variable was the Key_Var instead, the next variable becomes the Value_Var
            Node * key_var_node = statement_node->last_child;
            key_var_node->type = Node_Stmt_Foreach_Key_Var;  // Foreach_Value_Var --becomes--> Foreach_Key_Var

            parse_child_variable_node(parser, Node_Stmt_Foreach_Value_Var, statement_node, true);
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
            
            add_child_node(function_body_node, statement_node);
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
        parse_child_expression_node(parser, Node_Stmt_Return, statement_node);
        
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
            
            Token * latest_token = latest_eaten_token(parser);
            log("Latest eaten token is:");
            log(latest_token->text);
            
            Tokenizer * tokenizer = parser->tokenizer;
            Token * tokens = (Token *)tokenizer->tokens.items;
            Token * token = &tokens[parser->current_token_index];
            log("Next token is:");
            log(token->text);
            
            log("Line number:");
            log_int(token->line_index + 1);
            
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
