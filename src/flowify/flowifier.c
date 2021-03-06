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

FlowElement * flowify_parameters(Flowifier * flowifier, Node * function_call_arguments_node, Node * function_parameters_node);
void flowify_expressions(Flowifier * flowifier, FlowElement * parent_element);
void flowify_statements(Flowifier * flowifier, FlowElement * parent_element);

FlowElement * flowify_expression(Flowifier * flowifier, Node * expression_node, b32 is_statement = false)
{
    // TODO: we should flowify the expression! Meaning: we should track the dataflow as well (for now we only track control flow)
    
    FlowElement * new_expression_element = 0;
    
    if (expression_node) 
    {
        if (expression_node->type == Node_Expr_Assign ||
            expression_node->type == Node_Expr_AssignOp_Multiply ||
            expression_node->type == Node_Expr_AssignOp_Divide ||
            expression_node->type == Node_Expr_AssignOp_Plus ||
            expression_node->type == Node_Expr_AssignOp_Minus ||
            expression_node->type == Node_Expr_AssignOp_Concat)
        {
            Node * assignee_node = expression_node->first_child;
            Node * right_side_expression_node = assignee_node->next_sibling;
            
            FlowElement * assignment_expression_element = new_element(flowifier, expression_node, FlowElement_Assignment);
            
            FlowElement * assignee_element = flowify_expression(flowifier, assignee_node);
            add_child_element(assignee_element, assignment_expression_element);
            
            // Note: we set the ast-node of the operator itself to the whole assignment-expression (because the operator itself is not an ast-node by itself)
            FlowElement * assignment_operator_element = new_element(flowifier, expression_node, FlowElement_AssignmentOperator);
            // TODO: we use the identifier of the expression (which is filled with the operator itself) as the "source_text" of this element! (little dirty)
            assignment_operator_element->source_text = expression_node->identifier;
            add_child_element(assignment_operator_element, assignment_expression_element);

            FlowElement * right_side_expression_element = flowify_expression(flowifier, right_side_expression_node);
            add_child_element(right_side_expression_element, assignment_expression_element);
            
            new_expression_element = assignment_expression_element;
        }
        else if (expression_node->type == Node_Expr_PreInc ||
                 expression_node->type == Node_Expr_PreDec)
        {
            Node * assignee_node = expression_node->first_child;
            
            FlowElement * unary_pre_operation_element = new_element(flowifier, expression_node, FlowElement_UnaryPreOperation);
            
            // Note: we set the ast-node of the operator itself to the whole unary-expression (because the operator itself is not an ast-node by itself)
            FlowElement * unary_operator_element = new_element(flowifier, expression_node, FlowElement_UnaryOperator);
            // TODO: we use the identifier of the expression (which is filled with the operator itself) as the "source_text" of this element! (little dirty)
            unary_operator_element->source_text = expression_node->identifier;
            add_child_element(unary_operator_element, unary_pre_operation_element);
            
            FlowElement * assignee_element = flowify_expression(flowifier, assignee_node);
            add_child_element(assignee_element, unary_pre_operation_element);
            
            new_expression_element = unary_pre_operation_element;
        }
        else if (expression_node->type == Node_Expr_PostInc ||
                 expression_node->type == Node_Expr_PostDec)
        {
            Node * assignee_node = expression_node->first_child;
            
            FlowElement * unary_post_operation_element = new_element(flowifier, expression_node, FlowElement_UnaryPostOperation);
            
            FlowElement * assignee_element = flowify_expression(flowifier, assignee_node);
            add_child_element(assignee_element, unary_post_operation_element);
            
            // Note: we set the ast-node of the operator itself to the whole unary-expression (because the operator itself is not an ast-node by itself)
            FlowElement * unary_operator_element = new_element(flowifier, expression_node, FlowElement_UnaryOperator);
            // TODO: we use the identifier of the expression (which is filled with the operator itself) as the "source_text" of this element! (little dirty)
            unary_operator_element->source_text = expression_node->identifier;
            add_child_element(unary_operator_element, unary_post_operation_element);
            
            new_expression_element = unary_post_operation_element;
        }
        else if (expression_node->type == Node_Expr_BinaryOp_Multiply ||
                 expression_node->type == Node_Expr_BinaryOp_Divide ||
                 expression_node->type == Node_Expr_BinaryOp_Plus ||
                 expression_node->type == Node_Expr_BinaryOp_Minus ||
                 expression_node->type == Node_Expr_BinaryOp_Concat ||
                 expression_node->type == Node_Expr_BinaryOp_Smaller ||
                 expression_node->type == Node_Expr_BinaryOp_Greater ||
                 expression_node->type == Node_Expr_BinaryOp_SmallerOrEqual ||
                 expression_node->type == Node_Expr_BinaryOp_GreaterOrEqual ||
                 expression_node->type == Node_Expr_BinaryOp_NotEqual ||
                 expression_node->type == Node_Expr_BinaryOp_Equal ||
                 expression_node->type == Node_Expr_BinaryOp_NotExactlyEqual ||
                 expression_node->type == Node_Expr_BinaryOp_ExactlyEqual ||
                 expression_node->type == Node_Expr_BinaryOp_LogicalAnd ||
                 expression_node->type == Node_Expr_BinaryOp_LogicalOr
                 )
        {
            FlowElement * binary_op_expression_element = new_element(flowifier, expression_node, FlowElement_BinaryOperation);
            
            FlowElement * left_operand_element = flowify_expression(flowifier, expression_node->first_child);
            add_child_element(left_operand_element, binary_op_expression_element);

            FlowElement * binary_operator_element = new_element(flowifier, expression_node, FlowElement_BinaryOperator);
            // TODO: we use the identifier of the expression (which is filled with the operator itself) as the "source_text" of this element! (little dirty)
            binary_operator_element->source_text = expression_node->identifier;
            add_child_element(binary_operator_element, binary_op_expression_element);
            
            FlowElement * right_operand_element = flowify_expression(flowifier, expression_node->first_child->next_sibling);
            add_child_element(right_operand_element, binary_op_expression_element);
            
            new_expression_element = binary_op_expression_element;
        }
        else if (expression_node->type == Node_Expr_Variable_Array_Access)
        {
            FlowElement * array_expression_element = new_element(flowifier, expression_node, FlowElement_ArrayAccess);
        
            FlowElement * array_identifier_element = new_element(flowifier, expression_node, FlowElement_Variable);
            array_identifier_element->source_text = expression_node->identifier;
            add_child_element(array_identifier_element, array_expression_element);
            
            FlowElement * array_key_expression_element = flowify_expression(flowifier, expression_node->first_child);
            add_child_element(array_key_expression_element, array_expression_element);
            
            new_expression_element = array_expression_element;
        }
        else if (expression_node->type == Node_Expr_FuncCall)
        {
            Node * function_call_node = expression_node;
            Node * function_call_arguments_node = function_call_node->first_child;
            
            // Function Call
            
            FlowElement * function_call_element = new_element(flowifier, function_call_node, FlowElement_FunctionCall);

            FlowElement * function_call_identifier = new_element(flowifier, function_call_node, FlowElement_FunctionCallIdentifier);
            // TODO: we use the identifier of the expression (which is filled with identifier of the function call) as the "source_text" of this element! (little dirty)
            function_call_identifier->source_text = function_call_node->identifier;
            add_child_element(function_call_identifier, function_call_element);
                
            FlowElement * function_call_arguments = new_element(flowifier, function_call_arguments_node, FlowElement_FunctionCallArguments);
            add_child_element(function_call_arguments, function_call_element);
            flowify_expressions(flowifier, function_call_arguments);
            
            // Function
            
            // FIXME: we should CLONE the function element, since this INSTANCE will be placed somewhere else
            //        and connected to (its previous_sibling, next_sibling and parent will be) different elements compared
            //        to another call of the same function!
            //log("Trying to find function");
            //log(function_call_node->identifier);
            FlowElement * function_element = get_function_element(flowifier, function_call_node->identifier);

            if (function_element && !function_element->parent) // FIXME: only adding if the function has no parent yet (to prevent infinite recursion)
            {
                // Right now, a is always be collapsed by default
                function_element->is_collapsed = true;

                Node * function_node = function_element->ast_node;
                Node * function_parameters_node = function_node->first_child;
                
                Node * function_call_arguments_node = function_call_arguments->ast_node;
            
                FlowElement * parameters_element = flowify_parameters(flowifier, function_call_arguments_node, function_parameters_node);
                
                add_child_element(parameters_element, function_call_element);
                add_child_element(function_element, function_call_element);
            }
            else
            {
                // log("Unknown function:");
                // log(identifier);
                
                // TODO: is it corrent that the hidden element has no corresponding ast-node?
                FlowElement * hidden_function_element = new_element(flowifier, 0, FlowElement_Hidden);
                add_child_element(hidden_function_element, function_call_element);
                
                // Right now, a hidden function is always be collapsed by default
                hidden_function_element->is_collapsed = true;
            }
            
            new_expression_element = function_call_element;
        }
        else if (expression_node->type == Node_Expr_Variable)
        {
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Variable);
        }
        else if (expression_node->type == Node_Scalar_Number)
        {
            // TODO: add the (constant) value to the flow element
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Scalar);
        }
        else if (expression_node->type == Node_Scalar_String)
        {
            // TODO: add the (constant) value to the flow element
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Scalar);
        }
        else if (expression_node->type == Node_Scalar_EmptyArray)
        {
            // TODO: add the (constant) value to the flow element
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Scalar);
        }
        else if (expression_node->type == Node_Scalar_True)
        {
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Scalar);
        }
        else if (expression_node->type == Node_Scalar_False)
        {
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Scalar);
        }
        else if (expression_node->type == Node_Scalar_Null)
        {
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Scalar);
        }
        else
        {
            // Unknown expression
            // TODO: log that we found an unknown expression!
            new_expression_element = new_element(flowifier, expression_node, FlowElement_Unknown);
        }
    }
    else {
        log("ERROR: expression node is null!!");
    }
    
    new_expression_element->is_statement = is_statement;
    
    return new_expression_element;
}

FlowElement * flowify_child_expression_or_passthrough(Flowifier * flowifier, Node * parent_node, b32 is_statement = false)
{
    if (parent_node && parent_node->first_child)
    {
        FlowElement * expression_element = flowify_expression(flowifier, parent_node->first_child, is_statement);
        return expression_element;
    }
    else
    {
        FlowElement * passthrough_element = new_element(flowifier, 0, FlowElement_PassThrough);
        return passthrough_element;
    }
}

void flowify_child_statements_or_passthrough(Flowifier * flowifier, Node * body_node, FlowElement * body_element)
{
    if (body_node && body_node->first_child)
    {
        flowify_statements(flowifier, body_element);
    }
    else
    {
        FlowElement * body_passthrough_element = new_element(flowifier, 0, FlowElement_PassThrough);
        add_child_element(body_passthrough_element, body_element);
    }
}

void flowify_expressions(Flowifier * flowifier, FlowElement * parent_element)
{
    Node * parent_node = parent_element->ast_node;
    
    Node * expression_node = parent_node->first_child;
    if (expression_node) // There are expressions (in the parent)
    {
        do // Loop through all expressions (in the parent)
        {
            FlowElement * new_expression_element = flowify_expression(flowifier, expression_node);
                
            add_child_element(new_expression_element, parent_element);
        }
        while((expression_node = expression_node->next_sibling));
    }
}

FlowElement * flowify_parameters(Flowifier * flowifier, Node * function_call_arguments_node, Node * function_parameters_node)
{
    FlowElement * parameters_element = new_element(flowifier, function_call_arguments_node, FlowElement_FunctionParameterAssignments);
        
    FlowElement * previous_assignment_element = 0;
    Node * function_call_argument_node = function_call_arguments_node->first_child;
    Node * function_parameter_node = function_parameters_node->first_child;
    if (function_call_argument_node && function_parameter_node) // There is at least one argument AND one parameter
    {
        // TODO: make this a while loop instead!
        
        do // Loop through all parameters and arguments simultaniously
        {
            // Note: right now the assignment-expression is represented by the function-call-argument (not the function-parameter)
            FlowElement * assignment_expression_element = new_element(flowifier, function_call_argument_node, FlowElement_Assignment);
            
            // TODO: these are variables-only. No need to call flowify_expression here!
            FlowElement * assignee_element = flowify_expression(flowifier, function_parameter_node);
            add_child_element(assignee_element, assignment_expression_element);
            
            // Note: we set the ast-node of the operator itself to the function-call-argument-expression (because the operator itself is not an ast-node by itself)
            FlowElement * assignment_operator_element = new_element(flowifier, function_call_argument_node, FlowElement_AssignmentOperator);
            // TODO: we cannot the identifier of the expression as the "source_text" here! 
            // we can't use actual source code, so we "create" some on-the-fly (that we don't have to allocate)
            String equal_sign = {};
            equal_sign.data = &flowifier->ascii_characters['='];
            equal_sign.length = 1;
            assignment_operator_element->source_text = equal_sign;
            add_child_element(assignment_operator_element, assignment_expression_element);

            FlowElement * right_side_expression_element = flowify_expression(flowifier, function_call_argument_node);
            add_child_element(right_side_expression_element, assignment_expression_element);
            
            add_child_element(assignment_expression_element, parameters_element);
            
            previous_assignment_element = assignment_expression_element;
            
            function_call_argument_node = function_call_argument_node->next_sibling;
            function_parameter_node = function_parameter_node->next_sibling;
        }
        while(function_call_argument_node && function_parameter_node);
    }
    
    // TODO: not supporting optional arguments!
    
    return parameters_element;
}

FlowElement * flowify_statement(Flowifier * flowifier, Node * statement_node)
{
    FlowElement * new_statement_element = 0;
    
    if (statement_node->type == Node_Stmt_Expr)
    {
        Node * expression_node = statement_node->first_child;
        
        b32 is_statement = true;
        FlowElement * new_expression_element = flowify_expression(flowifier, expression_node, is_statement);
        
        new_statement_element = new_expression_element;
    }
    else if (statement_node->type == Node_Stmt_If)
    {
        FlowElement * if_element = new_element(flowifier, statement_node, FlowElement_If);
        
        Node * if_cond_node = statement_node->first_child;
        Node * if_then_node = if_cond_node->next_sibling;
        Node * if_else_node = if_then_node->next_sibling;
        
        FlowElement * if_cond_element = new_element(flowifier, if_cond_node, FlowElement_IfCond); 
        // Note: we set the ast-node of the if-keyword itself to the whole if-cond-expression (because the keyword itself is not an ast-node by itself)
        FlowElement * if_keyword_element = new_element(flowifier, if_cond_node, FlowElement_IfKeyword);
        // TODO: we use the identifier of the if-cond-expression (which is filled with the keyword itself) as the "source_text" of this element! (little dirty)
        if_keyword_element->source_text = if_cond_node->identifier;
        add_child_element(if_keyword_element, if_cond_element);
        FlowElement * cond_expression_element = flowify_child_expression_or_passthrough(flowifier, if_cond_node);
        add_child_element(cond_expression_element, if_cond_element);
        
//        FlowElement * if_split_element = new_element(flowifier, if_cond_node, FlowElement_IfSplit); 
        
        FlowElement * if_then_element = new_element(flowifier, if_then_node, FlowElement_IfThen);
        flowify_child_statements_or_passthrough(flowifier, if_then_node, if_then_element);
        
        FlowElement * if_else_element = new_element(flowifier, if_else_node, FlowElement_IfElse);
        flowify_child_statements_or_passthrough(flowifier, if_else_node, if_else_element);
        
        FlowElement * if_join_element = new_element(flowifier, 0, FlowElement_IfJoin);
        
        add_child_element(if_cond_element, if_element);
//        add_child_element(if_split_element, if_element);
        add_child_element(if_then_element, if_element);
        add_child_element(if_else_element, if_element);
        add_child_element(if_join_element, if_element);
        
        new_statement_element = if_element;
    }
    else if (statement_node->type == Node_Stmt_For)
    {
        FlowElement * for_element = new_element(flowifier, statement_node, FlowElement_For);
        
        Node * for_init_node = statement_node->first_child;
        Node * for_cond_node = for_init_node->next_sibling;
        Node * for_update_node = for_cond_node->next_sibling;
        Node * for_body_node = for_update_node->next_sibling;
        
        FlowElement * for_init_element = new_element(flowifier, for_init_node, FlowElement_ForInit); 
        b32 is_statement = true;
        FlowElement * init_expression_element = flowify_child_expression_or_passthrough(flowifier, for_init_node, is_statement);
        add_child_element(init_expression_element, for_init_element);
        
//        FlowElement * for_join_element = new_element(flowifier, 0, FlowElement_ForJoin); 
        
        FlowElement * for_cond_element = new_element(flowifier, for_cond_node, FlowElement_ForCond); 
        // Note: we set the ast-node of the for-keyword itself to the whole for-cond-expression (because the keyword itself is not an ast-node by itself)
        FlowElement * for_keyword_element = new_element(flowifier, for_cond_node, FlowElement_ForKeyword);
        // TODO: we use the identifier of the for-cond-expression (which is filled with the keyword itself) as the "source_text" of this element! (little dirty)
        for_keyword_element->source_text = for_cond_node->identifier;
        add_child_element(for_keyword_element, for_cond_element);
        FlowElement * cond_expression_element = flowify_child_expression_or_passthrough(flowifier, for_cond_node);
        add_child_element(cond_expression_element, for_cond_element);
        
//        FlowElement * for_split_element = new_element(flowifier, 0, FlowElement_ForSplit); 
        
        FlowElement * for_body_element = new_element(flowifier, for_body_node, FlowElement_ForBody); 
        flowify_child_statements_or_passthrough(flowifier, for_body_node, for_body_element);
        
        FlowElement * for_update_element = new_element(flowifier, for_update_node, FlowElement_ForUpdate); 
        is_statement = true;
        FlowElement * update_expression_element = flowify_child_expression_or_passthrough(flowifier, for_update_node, is_statement);
        add_child_element(update_expression_element, for_update_element);
        
        FlowElement * for_passright_element = new_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * for_passup_element = new_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * for_passleft_element = new_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * for_passdown_element = new_element(flowifier, 0, FlowElement_PassBack);
            
        FlowElement * for_done_element = new_element(flowifier, 0, FlowElement_ForDone); 
        
        add_child_element(for_init_element, for_element);
//        add_child_element(for_join_element, for_element);
        add_child_element(for_cond_element, for_element);
//        add_child_element(for_split_element, for_element);
        add_child_element(for_body_element, for_element);
        add_child_element(for_update_element, for_element);
        add_child_element(for_passright_element, for_element);
        add_child_element(for_passup_element, for_element);
        add_child_element(for_passleft_element, for_element);
        add_child_element(for_passdown_element, for_element);
        add_child_element(for_done_element, for_element);
        
        new_statement_element = for_element;
    }
    else if (statement_node->type == Node_Stmt_Foreach)
    {
        FlowElement * foreach_element = new_element(flowifier, statement_node, FlowElement_Foreach);
        
        Node * foreach_cond_node = statement_node->first_child;
        
        Node * foreach_array_node = foreach_cond_node->first_child;
        Node * foreach_value_var_node = foreach_array_node->next_sibling;
        Node * foreach_key_var_node = 0;
        if (foreach_value_var_node->type == Node_Stmt_Foreach_Key_Var)
        {
            foreach_key_var_node = foreach_value_var_node;
            foreach_value_var_node = foreach_key_var_node->next_sibling;
        }
        Node * foreach_body_node = foreach_cond_node->next_sibling;
        
        FlowElement * foreach_init_element = new_element(flowifier, 0, FlowElement_ForeachInit); 
        
//        FlowElement * foreach_join_element = new_element(flowifier, 0, FlowElement_ForeachJoin); 
        
        FlowElement * foreach_cond_element = new_element(flowifier, foreach_cond_node, FlowElement_ForeachCond); 
        
        FlowElement * foreach_keyword_element = new_element(flowifier, foreach_cond_node, FlowElement_ForeachKeyword);  // Note: we set the ast-node of the foreach-keyword itself to the whole foreach-cond-expression (because the keyword itself is not an ast-node by itself)
        foreach_keyword_element->source_text = foreach_cond_node->identifier;  // TODO: we use the identifier of the foreach-cond-expression (which is filled with the keyword itself) as the "source_text" of this element! (little dirty)
        add_child_element(foreach_keyword_element, foreach_cond_element);
        FlowElement * foreach_array_expression_element = flowify_child_expression_or_passthrough(flowifier, foreach_array_node);
        add_child_element(foreach_array_expression_element, foreach_cond_element);
        
        FlowElement * as_keyword_element = new_element(flowifier, foreach_array_node, FlowElement_ForeachAsKeyword);  // Note: we set the ast-node of the as-keyword itself to the foreach-array-expression (because the keyword itself is not an ast-node by itself)
        as_keyword_element->source_text = foreach_array_node->identifier;  // TODO: we use the identifier of the foreach-array-expression (which is filled with the keyword "as") as the "source_text" of this element! (little dirty)
        add_child_element(as_keyword_element, foreach_cond_element);
        
        if (foreach_key_var_node)
        {
            FlowElement * foreach_key_var_element = new_element(flowifier, foreach_key_var_node, FlowElement_ForeachKeyVar); 
            add_child_element(foreach_key_var_element, foreach_cond_element);
            
            FlowElement * arrow_keyword_element = new_element(flowifier, foreach_key_var_node, FlowElement_ForeachArrowKeyword);  // Note: we set the ast-node of the arrow-keyword itself to the foreach-key_var-expression (because the keyword itself is not an ast-node by itself)
            arrow_keyword_element->source_text = foreach_key_var_node->identifier;  // TODO: we use the identifier of the foreach-key_var-expression (which is filled with the keyword "=>") as the "source_text" of this element! (little dirty)
            add_child_element(arrow_keyword_element, foreach_cond_element);
        }
        FlowElement * foreach_value_var_element = new_element(flowifier, foreach_value_var_node, FlowElement_ForeachValueVar); 
        add_child_element(foreach_value_var_element, foreach_cond_element);
        
//        FlowElement * foreach_split_element = new_element(flowifier, 0, FlowElement_ForeachSplit); 
        
        FlowElement * foreach_body_element = new_element(flowifier, foreach_body_node, FlowElement_ForeachBody); 
        flowify_child_statements_or_passthrough(flowifier, foreach_body_node, foreach_body_element);
        
        FlowElement * foreach_passright_element = new_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * foreach_passup_element = new_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * foreach_passleft_element = new_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * foreach_passdown_element = new_element(flowifier, 0, FlowElement_PassBack);
            
        FlowElement * foreach_done_element = new_element(flowifier, 0, FlowElement_ForeachDone); 
        
        add_child_element(foreach_init_element, foreach_element);        
//        add_child_element(foreach_join_element, foreach_element);
        add_child_element(foreach_cond_element, foreach_element);
//        add_child_element(foreach_split_element, foreach_element);
        add_child_element(foreach_body_element, foreach_element);
        add_child_element(foreach_passright_element, foreach_element);
        add_child_element(foreach_passup_element, foreach_element);
        add_child_element(foreach_passleft_element, foreach_element);
        add_child_element(foreach_passdown_element, foreach_element);
        add_child_element(foreach_done_element, foreach_element);
        
        new_statement_element = foreach_element;
    }
    else if (statement_node->type == Node_Stmt_Return)
    {
        FlowElement * return_element = new_element(flowifier, statement_node, FlowElement_Return);
        
        // Note: we set the ast-node of the return-keyword itself to the whole return-statement (because the keyword itself is not an ast-node by itself)
        FlowElement * return_keyword_element = new_element(flowifier, statement_node, FlowElement_ReturnKeyword);
        // TODO: we use the identifier of the statement (which is filled with the keyword itself) as the "source_text" of this element! (little dirty)
        return_keyword_element->source_text = statement_node->identifier;
        add_child_element(return_keyword_element, return_element);
        
        // Only add expression-element if the return-statement has an expression (as its child)
        if (statement_node->first_child)
        {
            Node * expression_node = statement_node->first_child;
            FlowElement * new_expression_element = flowify_expression(flowifier, expression_node);
            add_child_element(new_expression_element, return_element);
        }
        
        new_statement_element = return_element;
    }
    else if (statement_node->type == Node_Stmt_Break)
    {
        FlowElement * break_element = new_element(flowifier, statement_node, FlowElement_Break);
        
        // TODO: flowify break statement
        
        new_statement_element = break_element;
    }
    else if (statement_node->type == Node_Stmt_Continue)
    {
        FlowElement * continue_element = new_element(flowifier, statement_node, FlowElement_Continue);
        
        // TODO: flowify continue statement
        
        new_statement_element = continue_element;
    }
    
    return new_statement_element;
}

void flowify_function(Flowifier * flowifier, Node * function_node)
{
    Node * function_parameters_node = function_node->first_child;
    Node * function_body_node = function_parameters_node->next_sibling;
   
    FlowElement * function_element = get_function_element(flowifier, function_node->identifier);
    assert(function_element);
    
    FlowElement * function_body_element = new_element(flowifier, function_body_node, FlowElement_FunctionBody);
    
    flowify_statements(flowifier, function_body_element);
    
    add_child_element(function_body_element, function_element);
}

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element)
{
    Node * parent_node = parent_element->ast_node;
    
    Node * statement_node = parent_node->first_child;
    FlowElement * previous_statement_element = 0;
    if (statement_node) // There are statements (in the parent)
    {
        do // Loop through all statements (in the parent)
        {
            if (statement_node->type == Node_Stmt_Function)
            {
                flowify_function(flowifier, statement_node);
            }
            else
            {
                FlowElement * new_statement_element = flowify_statement(flowifier, statement_node);
                    
                add_child_element(new_statement_element, parent_element);
                
                previous_statement_element = new_statement_element;
            }
        }
        while((statement_node = statement_node->next_sibling));
    }
}

// We create stub for all the function definitions
void find_and_create_stub_functions(Flowifier * flowifier, Node * parent_node)
{
    Node * statement_node = parent_node->first_child;
    if (statement_node)
    {
        do
        {
            if (statement_node->type == Node_Stmt_Function)
            {
                FlowElement * stub_function_element = new_element(flowifier, statement_node, FlowElement_Function);
                add_function_element(flowifier, stub_function_element);
                
                find_and_create_stub_functions(flowifier, statement_node);
            }
        }
        while((statement_node = statement_node->next_sibling));
    }
}

FlowElement * flowify_root(Flowifier * flowifier, Node * root_node)
{
    FlowElement * root_element = new_element(flowifier, root_node, FlowElement_Root);
    
    find_and_create_stub_functions(flowifier, root_node);
    flowify_statements(flowifier, root_element);
    
    return root_element;
}    
