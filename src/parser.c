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
    "(" [ expr { ", " expr } ] ")"
    
parameters =
    "(" [ [&]var { ", " ["&"]var } ] ")"

expr =
    sub_expr "<" sub_expr
    sub_expr ">" sub_expr
    sub_expr "-" sub_expr
    sub_expr "+" sub_expr
    sub_expr "*" sub_expr
    sub_expr "." sub_expr
    sub_expr "==" sub_expr
    sub_expr

var_array_access :=
    var "[" expr "]"

sub_expr :=
    "(" expr ")"
    "++" var                        // TODO: is var_array_access also allowed here?
    "--" var                        // TODO: is var_array_access also allowed here?
    var(_array_access) "++"
    var(_array_access) "--"
    var(_array_access) "*=" expr
    var(_array_access) "/=" expr
    var(_array_access) "+=" expr
    var(_array_access) "-=" expr
    var(_array_access) ".=" expr
    var(_array_access) "="  expr
    var(_array_access)
    number
    float
    single_quoted_string
    "[" "]"                         // TODO: we should allow for non-empty arrays!
    null
    identifier arguments
    
*/

Node * parse_arguments(Parser * parser, NodeType node_type);
Node * parse_expression(Parser * parser);
Node * parse_sub_expression(Parser * parser);

Node * parse_child_expression_node(Parser * parser, NodeType node_type)
{
    Node * child_node = start_node(parser, node_type);

    Node * child_expression_node = parse_expression(parser);
    
    add_child_node(child_expression_node, child_node);
    
    end_node(parser, child_node);
    
    return child_node;
}

Node * parse_variable(Parser * parser, NodeType node_type, b32 allow_variable_reference = false)
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
    
    return variable_node;
}

Node * parse_variable_assignment(Parser * parser, NodeType node_type, Node * variable_node)
{
    Node * variable_assignment_node = start_node(parser, node_type);
    // We set the first_token_index of the variable_assignment_node to the first_token_index of the variable_node
    variable_assignment_node->first_token_index = variable_node->first_token_index;
    
    // Left side of the expression (a variable)
    add_child_node(variable_node, variable_assignment_node);

    // The assignment operator acts as the identifier of the assignment-expression (note: we assume its has just been comsumed, so we get the latest token)
    Token * assignment_operator_token = latest_eaten_token(parser);
    variable_assignment_node->identifier = assignment_operator_token->text;

    // Right side of the expression (an expression)
    Node * child_expression_node = parse_expression(parser);
    add_child_node(child_expression_node, variable_assignment_node);
    
    end_node(parser, variable_assignment_node);
    
    return variable_assignment_node;
}

Node * parse_binary_op_expression(Parser * parser, NodeType node_type, Node * left_sub_expression)
{
    Node * binary_expression_node = start_node(parser, node_type);
    // We set the first_token_index of the binary_expression to the first_token_index of the left_sub_expression
    binary_expression_node->first_token_index = left_sub_expression->first_token_index;
    
    // The binary operator acts as the identifier of the binary-expression (note: we assume its has just been comsumed, so we get the latest token)
    Token * binary_operator_token = latest_eaten_token(parser);
    binary_expression_node->identifier = binary_operator_token->text;
    
    Node * right_sub_expression = parse_sub_expression(parser);
    add_child_node(left_sub_expression, binary_expression_node);
    add_child_node(right_sub_expression, binary_expression_node);
    
    end_node(parser, binary_expression_node);
    
    return binary_expression_node;
}

Node * parse_sub_expression(Parser * parser)
{
    Node * sub_expression_node = 0;
    
    if (accept_token(parser, Token_OpenParenteses))
    {
        // TODO: do we want this expression node the include or exclude the parentheses?
        
        i32 first_token_index = latest_eaten_token_index(parser);
        
        sub_expression_node = parse_expression(parser);
        sub_expression_node->first_token_index = first_token_index;
        
        expect_token(parser, Token_CloseParenteses);
        
        sub_expression_node->last_token_index = latest_eaten_token_index(parser);
    }
    else if (accept_token(parser, Token_PlusPlus))
    {
        sub_expression_node = start_node(parser, Node_Expr_PreInc, StartOnLatestToken);
        
        // The unary operator acts as the identifier of the unary-expression (note: we assume its has just been comsumed, so we get the latest token)
        Token * unary_operator_token = latest_eaten_token(parser);
        sub_expression_node->identifier = unary_operator_token->text;
    
        Node * variable_node = parse_variable(parser, Node_Expr_Variable, true);
        add_child_node(variable_node, sub_expression_node);
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_MinusMinus))
    {
        sub_expression_node = start_node(parser, Node_Expr_PreDec, StartOnLatestToken);
        
        // The unary operator acts as the identifier of the unary-expression (note: we assume its has just been comsumed, so we get the latest token)
        Token * unary_operator_token = latest_eaten_token(parser);
        sub_expression_node->identifier = unary_operator_token->text;
        
        Node * variable_node = parse_variable(parser, Node_Expr_Variable, true);
        add_child_node(variable_node, sub_expression_node);
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_VariableIdentifier))
    {
        Token * variable_token = latest_eaten_token(parser);
        
        Node * variable_node = start_node(parser, Node_Expr_Variable, StartOnLatestToken);
        variable_node->identifier = variable_token->text;
            
        if (accept_token(parser, Token_OpenBracket))
        {
            variable_node->type = Node_Expr_Variable_Array_Access;
            
            Node * access_expression_node = parse_expression(parser);
            add_child_node(access_expression_node, variable_node);
            
            expect_token(parser, Token_CloseBracket);
        }
        end_node(parser, variable_node);
        
        if (accept_token(parser, Token_PlusPlus))
        {
            // TODO: we should only allow '++' *right* behind a variableIdentifier!
            sub_expression_node = start_node(parser, Node_Expr_PostInc, StartOnTokenBeforeLatestToken);
                
            // The unary operator acts as the identifier of the unary-expression (note: we assume its has just been comsumed, so we get the latest token)
            Token * unary_operator_token = latest_eaten_token(parser);
            sub_expression_node->identifier = unary_operator_token->text;
        
            add_child_node(variable_node, sub_expression_node);
            
            end_node(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_MinusMinus))
        {
            // TODO: we should only allow '--' *right* behind a variableIdentifier!
            sub_expression_node = start_node(parser, Node_Expr_PostDec, StartOnTokenBeforeLatestToken);
                
            // The unary operator acts as the identifier of the unary-expression (note: we assume its has just been comsumed, so we get the latest token)
            Token * unary_operator_token = latest_eaten_token(parser);
            sub_expression_node->identifier = unary_operator_token->text;
        
            add_child_node(variable_node, sub_expression_node);
            
            end_node(parser, sub_expression_node);
        }
        else if (accept_token(parser, Token_AssignMultiply))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Multiply, variable_node);
        }
        else if (accept_token(parser, Token_AssignDivide))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Divide, variable_node);
        }
        else if (accept_token(parser, Token_AssignPlus))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Plus, variable_node);
        }
        else if (accept_token(parser, Token_AssignMinus))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Minus, variable_node);
        }
        else if (accept_token(parser, Token_AssignConcat))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_AssignOp_Concat, variable_node);
        }
        else if (accept_token(parser, Token_Assign))
        {
            sub_expression_node = parse_variable_assignment(parser, Node_Expr_Assign, variable_node);
        }
        else
        {
            // If the variable was not followed by anything, we assume the expression only contains the variable
            sub_expression_node = variable_node;
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
    else if (accept_token(parser, Token_OpenBracket))
    {
        sub_expression_node = start_node(parser, Node_Scalar_EmptyArray, StartOnLatestToken);
        
        Token * open_bracket_token = latest_eaten_token(parser);
        sub_expression_node->value = open_bracket_token->text;
        
        // TODO: we should allow for non-empty arrays
        
        expect_token(parser, Token_CloseBracket);
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_True))
    {
        sub_expression_node = start_node(parser, Node_Scalar_True, StartOnLatestToken);
        
        Token * true_token = latest_eaten_token(parser);
        sub_expression_node->value = true_token->text;
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_False))
    {
        sub_expression_node = start_node(parser, Node_Scalar_False, StartOnLatestToken);
        
        Token * false_token = latest_eaten_token(parser);
        sub_expression_node->value = false_token->text;
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_Null))
    {
        sub_expression_node = start_node(parser, Node_Scalar_Null, StartOnLatestToken);
        
        Token * null_token = latest_eaten_token(parser);
        sub_expression_node->value = null_token->text;
        
        end_node(parser, sub_expression_node);
    }
    else if (accept_token(parser, Token_Identifier))
    {
        sub_expression_node = start_node(parser, Node_Expr_FuncCall, StartOnLatestToken);
        
        Token * function_call_identifier_token = latest_eaten_token(parser);
        sub_expression_node->identifier = function_call_identifier_token->text;
        
        Node * arguments_node = parse_arguments(parser, Node_Expr_FuncCall_Args);
        add_child_node(arguments_node, sub_expression_node);
        
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
    else if (accept_token(parser, Token_NotEqual))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_NotEqual, left_sub_expression);
    }
    else if (accept_token(parser, Token_ExactlyEqual))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_ExactlyEqual, left_sub_expression);
    }
    else if (accept_token(parser, Token_NotExactlyEqual))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_NotExactlyEqual, left_sub_expression);
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
    else if (accept_token(parser, Token_Period))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_Concat, left_sub_expression);
    }
    else if (accept_token(parser, Token_LogicalAnd))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_LogicalAnd, left_sub_expression);
    }
    else if (accept_token(parser, Token_LogicalOr))
    {
        expression_node = parse_binary_op_expression(parser, Node_Expr_BinaryOp_LogicalOr, left_sub_expression);
    }
    
    else
    {
        expression_node = left_sub_expression;
    }
    
    return expression_node;
}

Node * parse_arguments(Parser * parser, NodeType node_type)
{
    Node * arguments_node = start_node(parser, node_type);
    
    expect_token(parser, Token_OpenParenteses);
    
    while(!accept_token(parser, Token_CloseParenteses))
    {
        Node * argument_node = parse_expression(parser);
        if (argument_node)
        {
            add_child_node(argument_node, arguments_node);
            
            // TODO: Only allow next argument if it comma separated
            accept_token(parser, Token_Comma);
        }
        else
        {
            log("ERROR: invalid argument (not an expression) in function call");
            break;
        }
    }
    
    end_node(parser, arguments_node);
    
    return arguments_node;
}

Node * parse_parameters(Parser * parser, NodeType node_type)
{
    Node * parameters_node = start_node(parser, node_type);
    
    expect_token(parser, Token_OpenParenteses);
    
    while(!accept_token(parser, Token_CloseParenteses))
    {
        Node * parameter_node = parse_variable(parser, Node_Expr_Variable, true);
        if (parameter_node)
        {
            add_child_node(parameter_node, parameters_node);
            
            // TODO: Only allow next parameter if it comma separated
            accept_token(parser, Token_Comma);
        }
        else
        {
            log("ERROR: invalid parameter (not a variable) in function declaration");
            break;
        }
    }
    
    end_node(parser, parameters_node);
    
    return parameters_node;
}

Node * parse_block(Parser * parser, NodeType node_type);


Node * parse_statement(Parser * parser)
{
    if (accept_token(parser, Token_If))
    {
        Node * if_node = start_node(parser, Node_Stmt_If, StartOnLatestToken);
        
        // Note: we do not allow single-line bodies atm (without braces)

        // TODO: currently we set "if" as identifier of the if-cond expression to (later on) draw this text. We probably want to signify a if-keyword a different way.
        Token * if_token = latest_eaten_token(parser);
        
        // If-condition
        expect_token(parser, Token_OpenParenteses);
        
        Node * condition_node = parse_child_expression_node(parser, Node_Stmt_If_Cond);
        condition_node->identifier = if_token->text; // TODO: this is a little weird: the 'if'-token is not really part of the cond-expression
        add_child_node(condition_node, if_node);
       
        expect_token(parser, Token_CloseParenteses);
        
        // If-then
        Node * then_node = parse_block(parser, Node_Stmt_If_Then);
        add_child_node(then_node, if_node);
        
        if (accept_token(parser, Token_Else))
        {
            // If-else
            Node * else_node = parse_block(parser, Node_Stmt_If_Else);
            add_child_node(else_node, if_node);
        }
        
        // Note: if-statemets (or any other block-ending statements) do not require a Semocolon at the end!
        
        end_node(parser, if_node);
        
        return if_node;
    }
    else if (accept_token(parser, Token_For))
    {
        // For
        Node * for_node = start_node(parser, Node_Stmt_For, StartOnLatestToken);
        
        // TODO: currently we set "for" as identifier of the for-cond expression to (later on) draw this text. We probably want to signify a for-keyword a different way.
        Token * for_token = latest_eaten_token(parser);
        
        expect_token(parser, Token_OpenParenteses);
        
        // For_Init
        Node * init_node = parse_child_expression_node(parser, Node_Stmt_For_Init);
        add_child_node(init_node, for_node);
        
        expect_token(parser, Token_Semicolon);
        
        // For_Cond
        Node * cond_node = parse_child_expression_node(parser, Node_Stmt_For_Cond);
        cond_node->identifier = for_token->text; // TODO: this is a little weird: the 'for'-token is not really part of the cond-expression
        add_child_node(cond_node, for_node);
        
        expect_token(parser, Token_Semicolon);
        
        // For_Update
        Node * update_node = parse_child_expression_node(parser, Node_Stmt_For_Update);
        add_child_node(update_node, for_node);
        
        expect_token(parser, Token_CloseParenteses);
        
        // For_Body
        Node * for_body_node = parse_block(parser, Node_Stmt_For_Body);
        add_child_node(for_body_node, for_node);
        
        end_node(parser, for_node);
        
        return for_node;
    }
    else if (accept_token(parser, Token_Foreach))
    {
        // Foreach
        Node * foreach_node = start_node(parser, Node_Stmt_Foreach, StartOnLatestToken);
        
        Token * foreach_token = latest_eaten_token(parser);  // TODO: currently we set "foreach" as identifier of the foreach-cond expression to (later on) draw this text. We probably want to signify a foreach-keyword a different way.
        
        expect_token(parser, Token_OpenParenteses);
        
        // ForeachCond
        Node * foreach_cond_node = start_node(parser, Node_Stmt_Foreach_Cond, StartOnLatestToken);
        foreach_cond_node->identifier = foreach_token->text; // TODO: this is a little weird: the 'for'-token is not really part of the cond-expression
        add_child_node(foreach_cond_node, foreach_node);
        
        // Foreach_Array
        Node * foreach_array_node = parse_child_expression_node(parser, Node_Stmt_Foreach_Array);
        add_child_node(foreach_array_node, foreach_cond_node);
        
        expect_token(parser, Token_As);

        Token * as_token = latest_eaten_token(parser);  // TODO: currently we set "as" as identifier of the foreach-array expression to (later on) draw this text. We probably want to signify a as-keyword a different way.
        foreach_array_node->identifier = as_token->text; // TODO: this is a little weird: the 'as'-token is not really part of the array-expression
        
        // Foreach_Value_Var
        
        // We always expect a variable, which (by default) is the Foreach_Value_Var
        Node * foreach_first_var_node = parse_variable(parser, Node_Stmt_Foreach_Value_Var, true);
        add_child_node(foreach_first_var_node, foreach_cond_node);
        
        if (accept_token(parser, Token_Arrow)) {
            
            // If there is an "=>", the first variable was the Key_Var instead, the next variable becomes the Value_Var
            foreach_first_var_node->type = Node_Stmt_Foreach_Key_Var;  // Foreach_Value_Var --becomes--> Foreach_Key_Var

            Token * arrow_token = latest_eaten_token(parser);  // TODO: currently we set "=>" as identifier of the foreach-key_var expression to (later on) draw this text. We probably want to signify a "=>"-keyword a different way.
            foreach_first_var_node->identifier = arrow_token->text; // TODO: this is a little weird: the 'as'-token is not really part of the array-expression
        
            Node * foreach_second_var_node = parse_variable(parser, Node_Stmt_Foreach_Value_Var, true);
            add_child_node(foreach_second_var_node, foreach_cond_node);
        }
        
        expect_token(parser, Token_CloseParenteses);
        end_node(parser, foreach_cond_node);
        
        // Foreach_Body
        Node * foreach_body_node = parse_block(parser, Node_Stmt_Foreach_Body);
        add_child_node(foreach_body_node, foreach_node);
        
        end_node(parser, foreach_node);
        
        return foreach_node;
    }
    else if (accept_token(parser, Token_Function))
    {
        if (expect_token(parser, Token_Identifier))
        {
            Token * function_identifier_token = latest_eaten_token(parser);

            Node * function_node = start_node(parser, Node_Stmt_Function, StartOnTokenBeforeLatestToken);
        
            function_node->identifier = function_identifier_token->text;
            
            Node * function_parameters_node = parse_parameters(parser, Node_Stmt_Function_Params);
            add_child_node(function_parameters_node, function_node);

            // Function body
            Node * function_body_node = parse_block(parser, Node_Stmt_Function_Body);
            add_child_node(function_body_node, function_node);
            
            end_node(parser, function_node);
        
            return function_node;
        }
        else
        {
            log("ERROR: no function name found!");
        }
        
    }
    else if (accept_token(parser, Token_Continue))
    {
        Node * continue_node = start_node(parser, Node_Stmt_Continue, StartOnLatestToken);
        
        expect_token(parser, Token_Semicolon); 
        
        end_node(parser, continue_node);
        
        return continue_node;
    }
    else if (accept_token(parser, Token_Break))
    {
        Node * break_node = start_node(parser, Node_Stmt_Break, StartOnLatestToken);
        
        expect_token(parser, Token_Semicolon); 
        
        end_node(parser, break_node);
        
        return break_node;
    }
    else if (accept_token(parser, Token_Return))
    {
        Node * return_node = start_node(parser, Node_Stmt_Return, StartOnLatestToken);
        
        // TODO: currently we set "return" as identifier of the return expression to (later on) draw this text. We probably want to signify a return a different way.
        Token * return_token = latest_eaten_token(parser);
        return_node->identifier = return_token->text;
            
        if (accept_token(parser, Token_Semicolon))
        {
            // The return statement has no expression.
            
            // TODO: should we add a dummy child to the return_node?
            
        }
        else
        {
            Node * return_expression_node = parse_expression(parser);
            
            add_child_node(return_expression_node, return_node);
            
            expect_token(parser, Token_Semicolon); 
        }
        end_node(parser, return_node);
        
        return return_node;
    }
    else
    {
        // We assume its a statement with only an expression
        
        // Note: the expression hasn't started yet, so no need to do StartOnLatestToken here
        Node * statement_expression_node = start_node(parser, Node_Stmt_Expr);
        
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
            
            return 0;
        }
        add_child_node(expression_node, statement_expression_node);
        
        expect_token(parser, Token_Semicolon); 
        
        end_node(parser, statement_expression_node);
        
        return statement_expression_node;
    }
    // TODO implement more variants of statements
    
    // TODO: if we reach this point, we didn't find a statement, which is an ERROR.
    return 0;
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

Node * parse_block(Parser * parser, NodeType node_type)
{
    Node * block_node = start_node(parser, node_type);
    
    if (expect_token(parser, Token_OpenBrace))
    {
        parse_statements(parser, block_node);
        // Note: Token_CloseBrace is already eaten by parse_statements
    }
    else 
    {
        log("ERROR: did NOT found open brace!");
    }

    end_node(parser, block_node);
    
    return block_node;
}

Node * parse_program(Parser * parser)
{
    Node * root_node = 0;
    
    if (expect_token(parser, Token_StartOfPhp))
    {
        root_node = start_node(parser, Node_Root);
        
        parse_statements(parser, root_node);    
        
        end_node(parser, root_node);
    }
    else {
        // TODO: Program doesn't start with StartOfPhp
    }
    
    return root_node;
}
