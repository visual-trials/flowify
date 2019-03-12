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

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element);

FlowElement * flowify_expression(Flowifier * flowifier, Node * expression_node)
{
    // TODO: we should flowify the expression! (for now we create a dummy element)
    
    FlowElement * new_expression_element = 0;
    
    if (expression_node && expression_node->first_child)
    {
        if (expression_node->first_child->next_sibling)
        {
            if (expression_node->first_child->next_sibling->type == Node_Expr_PostInc)
            {
                // FIXME: hack!
                new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_BinaryOperator);
            }
            else if (expression_node->first_child->next_sibling->type == Node_Expr_FuncCall)
            {
                String identifier = expression_node->first_child->next_sibling->identifier;
                
                // FIXME: should we not CLONE the function element, since this INSTANCE will be places somewhere else
                //        and connected to (its previous_in_flow and next_in_flow will be) different compared
                //        to another call of the same function!
                FlowElement * function_element = get_function_element(flowifier, identifier);
                
                if (function_element)
                {
                    new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_FunctionCall);
                    new_expression_element->first_child = function_element;
                    // TODO: set parent?
                    new_expression_element->first_in_flow = function_element->first_in_flow;
                    new_expression_element->last_in_flow = function_element->last_in_flow;
                }
                else {
                    // log("Unknown function:");
                    // log(identifier);
                    new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_FunctionCall);
                    
                    // TODO: is it corrent that the hidden element has no corresponding ast-node?
                    FlowElement * hidden_element = new_flow_element(flowifier, 0, FlowElement_Hidden);
                    new_expression_element->first_child = hidden_element;
                    // TODO: set parent?
                    new_expression_element->first_in_flow = hidden_element;
                    new_expression_element->last_in_flow = hidden_element;
                }

            }
            else
            {
                // FIXME: hack!
                new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_Assignment);
            }
        }
        else
        {
            // FIXME: hack!
            new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_Assignment);
        }
    }
    else
    {
        // FIXME: this is not always an FlowElement_Assignment!
        new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_Assignment);
    }
    
    // FIXME: HACK. We want to do this properly for each expression type
    if (!new_expression_element->first_in_flow)
    {
        new_expression_element->first_in_flow = new_expression_element;
    }
    if (!new_expression_element->last_in_flow)
    {
        new_expression_element->last_in_flow = new_expression_element;
    }
    
    return new_expression_element;
}

FlowElement * flowify_statement(Flowifier * flowifier, Node * statement_node)
{
    FlowElement * new_statement_element = 0;
    
    if (statement_node->type == Node_Stmt_Expr)
    {
        Node * expression_node = statement_node->first_child;
        
        FlowElement * new_expression_element = flowify_expression(flowifier, expression_node);
        
        // TODO: should we wrap the expression element inside a statement element?
        new_statement_element = new_expression_element;
        new_statement_element->first_in_flow = new_expression_element->first_in_flow;
        new_statement_element->last_in_flow = new_expression_element->last_in_flow;
        
    }
    else if (statement_node->type == Node_Stmt_If)
    {
        FlowElement * if_element = new_flow_element(flowifier, statement_node, FlowElement_If);
        
        Node * if_cond_node = statement_node->first_child;
        Node * if_then_node = if_cond_node->next_sibling;
        Node * if_else_node = if_then_node->next_sibling;
        
        FlowElement * if_cond_element = new_flow_element(flowifier, if_cond_node, FlowElement_IfCond); 
        if (if_cond_node && if_cond_node->first_child)
        {
            FlowElement * cond_expression_element = flowify_expression(flowifier, if_cond_node->first_child);
            if_cond_element->first_child = cond_expression_element;
            cond_expression_element->parent = if_cond_element;
        }
        else
        {
            FlowElement * cond_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            if_cond_element->first_child = cond_passthrough_element;
            cond_passthrough_element->parent = if_cond_element;
            
            // TODO: whould we set first_in_flow and last_in_flow?
        }
        
        FlowElement * if_split_element = new_flow_element(flowifier, if_cond_node, FlowElement_IfSplit); 
        
        FlowElement * if_then_element = new_flow_element(flowifier, if_then_node, FlowElement_IfThen);
        if (if_then_node && if_then_node->first_child)
        {
            flowify_statements(flowifier, if_then_element);
        }
        else
        {
            FlowElement * then_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            if_then_element->first_child = then_passthrough_element;
            then_passthrough_element->parent = if_then_element;
            if_then_element->first_in_flow = then_passthrough_element;
            if_then_element->last_in_flow = then_passthrough_element;
        }
        
        FlowElement * if_else_element = new_flow_element(flowifier, if_else_node, FlowElement_IfElse);
        if (if_else_node && if_else_node->first_child)
        {
            flowify_statements(flowifier, if_else_element);
        }
        else
        {
            FlowElement * else_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            if_else_element->first_child = else_passthrough_element;
            else_passthrough_element->parent = if_else_element;
            if_else_element->first_in_flow = else_passthrough_element;
            if_else_element->last_in_flow = else_passthrough_element;
        }
        
        FlowElement * if_join_element = new_flow_element(flowifier, 0, FlowElement_IfJoin);
        
        if_element->first_child = if_cond_element;
        if_cond_element->parent = if_element;  // TODO: only setting parent on first_child?
        
        add_sibling(if_cond_element, if_split_element);
        add_sibling(if_split_element, if_then_element);
        add_sibling(if_then_element, if_else_element);
        add_sibling(if_else_element, if_join_element);
        
        if_element->first_in_flow = if_cond_element;
        if_element->last_in_flow = if_join_element;
        
        new_statement_element = if_element;
    }
    else if (statement_node->type == Node_Stmt_For)
    {
        FlowElement * for_element = new_flow_element(flowifier, statement_node, FlowElement_For);
        
        Node * for_init_node = statement_node->first_child;
        Node * for_cond_node = for_init_node->next_sibling;
        Node * for_update_node = for_cond_node->next_sibling;
        Node * for_body_node = for_update_node->next_sibling;
        
        FlowElement * for_start_element = new_flow_element(flowifier, 0, FlowElement_ForStart); 
        
        FlowElement * for_init_element = new_flow_element(flowifier, for_init_node, FlowElement_ForInit); 
        if (for_init_node && for_init_node->first_child)
        {
            FlowElement * init_expression_element = flowify_expression(flowifier, for_init_node->first_child);
            for_init_element->first_child = init_expression_element;
            init_expression_element->parent = for_init_element;
        }
        else
        {
            FlowElement * init_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            for_init_element->first_child = init_passthrough_element;
            init_passthrough_element->parent = for_init_element;
            
            // TODO: whould we set first_in_flow and last_in_flow?
        }
        
        FlowElement * for_join_element = new_flow_element(flowifier, 0, FlowElement_ForJoin); 
        
        FlowElement * for_cond_element = new_flow_element(flowifier, for_cond_node, FlowElement_ForCond); 
        if (for_cond_node && for_cond_node->first_child)
        {
            FlowElement * cond_expression_element = flowify_expression(flowifier, for_cond_node->first_child);
            for_cond_element->first_child = cond_expression_element;
            cond_expression_element->parent = for_cond_element;
        }
        else
        {
            FlowElement * cond_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            for_cond_element->first_child = cond_passthrough_element;
            cond_passthrough_element->parent = for_cond_element;
            
            // TODO: whould we set first_in_flow and last_in_flow?
        }
        
        FlowElement * for_split_element = new_flow_element(flowifier, 0, FlowElement_ForSplit); 
        
        FlowElement * for_body_element = new_flow_element(flowifier, for_body_node, FlowElement_ForBody); 
        if (for_body_node && for_body_node->first_child)
        {
            flowify_statements(flowifier, for_body_element);
        }
        else
        {
            FlowElement * body_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            for_body_element->first_child = body_passthrough_element;
            body_passthrough_element->parent = for_body_element;
            
            for_body_element->first_in_flow = body_passthrough_element;
            for_body_element->last_in_flow = body_passthrough_element;
        }
        
        FlowElement * for_update_element = new_flow_element(flowifier, for_update_node, FlowElement_ForUpdate); 
        if (for_update_node && for_update_node->first_child)
        {
            FlowElement * update_expression_element = flowify_expression(flowifier, for_update_node->first_child);
            for_update_element->first_child = update_expression_element;
            update_expression_element->parent = for_update_element;
        }
        else
        {
            FlowElement * update_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            for_update_element->first_child = update_passthrough_element;
            update_passthrough_element->parent = for_update_element;
            
            // TODO: whould we set first_in_flow and last_in_flow?
        }
        
        FlowElement * for_passright_element = new_flow_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * for_passup_element = new_flow_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * for_passleft_element = new_flow_element(flowifier, 0, FlowElement_PassBack);
        FlowElement * for_passdown_element = new_flow_element(flowifier, 0, FlowElement_PassBack);
            
        FlowElement * for_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
        
        FlowElement * for_done_element = new_flow_element(flowifier, 0, FlowElement_ForDone); 
        
        for_element->first_child = for_start_element;
        for_start_element->parent = for_element;  // TODO: only setting parent on first_child?
        
        add_sibling(for_start_element, for_init_element);
        add_sibling(for_init_element, for_join_element);
        add_sibling(for_join_element, for_cond_element);
        add_sibling(for_cond_element, for_split_element);
        add_sibling(for_split_element, for_body_element);
        add_sibling(for_body_element, for_update_element);
        add_sibling(for_update_element, for_passright_element);
        add_sibling(for_passright_element, for_passup_element);
        add_sibling(for_passup_element, for_passleft_element);
        add_sibling(for_passleft_element, for_passdown_element);
        add_sibling(for_passdown_element, for_passthrough_element);
        add_sibling(for_passthrough_element, for_done_element);
        
        for_element->first_in_flow = for_start_element;
        for_element->last_in_flow = for_done_element;
        
        new_statement_element = for_element;
    }
    else if (statement_node->type == Node_Stmt_Function)
    {
        FlowElement * function_element = new_flow_element(flowifier, statement_node, FlowElement_Function);
        
        // TODO: flowify function arguments
        
        Node * function_body_node = statement_node->first_child->next_sibling;
        
        FlowElement * function_body_element = new_flow_element(flowifier, function_body_node, FlowElement_FunctionBody);
        
        flowify_statements(flowifier, function_body_element);
        
        function_element->first_child = function_body_element;
        function_body_element->parent = function_element;
        
        // TODO: we might want to say that the first_in_flow of a function is actually the first statement element in the function?
        // Note that flowify_statements(...) (see above) already set first_in_flow and last_in_flow (to the first and last statement 
        // in the body resp.) of the function_body_element.
        function_element->first_in_flow = function_body_element->first_in_flow;
        function_element->last_in_flow = function_body_element->last_in_flow;
        
        new_statement_element = function_element;
    }
    else if (statement_node->type == Node_Stmt_Return)
    {
        FlowElement * return_element = new_flow_element(flowifier, statement_node, FlowElement_Return);
        
        // TODO: flowify return statement
        
        return_element->first_in_flow = return_element;
        return_element->last_in_flow = return_element;
        
        new_statement_element = return_element;
    }
    else if (statement_node->type == Node_Stmt_Break)
    {
        FlowElement * break_element = new_flow_element(flowifier, statement_node, FlowElement_Break);
        
        // TODO: flowify break statement
        
        break_element->first_in_flow = break_element;
        break_element->last_in_flow = break_element;
        
        new_statement_element = break_element;
    }
    else if (statement_node->type == Node_Stmt_Continue)
    {
        FlowElement * continue_element = new_flow_element(flowifier, statement_node, FlowElement_Continue);
        
        // TODO: flowify continue statement
        
        continue_element->first_in_flow = continue_element;
        continue_element->last_in_flow = continue_element;
        
        new_statement_element = continue_element;
    }
    
    return new_statement_element;
}

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element)
{
    Node * parent_node = parent_element->ast_node;
    Node * statement_node = 0;
    
    // Functions
    statement_node = parent_node->first_child;
    if (statement_node)
    {
        do
        {
            if (statement_node->type == Node_Stmt_Function)
            {
                log("Found function!");
                FlowElement * new_function_element = flowify_statement(flowifier, statement_node);

                // TODO: right now we store functions in one (global) linked list
                if (!flowifier->first_function)
                {
                    flowifier->first_function = new_function_element;
                }
                else 
                {
                    flowifier->latest_function->next_function = new_function_element;
                }
                flowifier->latest_function = new_function_element;
            }
        }
        while((statement_node = statement_node->next_sibling));
    }
    
    // Non-functions
    statement_node = parent_node->first_child;
    FlowElement * previous_statement_element = 0;
    parent_element->first_in_flow = parent_element;
    parent_element->last_in_flow = parent_element;
    if (statement_node) // There are statements (in the parent)
    {
        do // Loop through all statements (in the parent)
        {
            if (statement_node->type != Node_Stmt_Function)
            {
                FlowElement * new_statement_element = flowify_statement(flowifier, statement_node);
                    
                if (!parent_element->first_child)
                {
                    parent_element->first_child = new_statement_element;
                    new_statement_element->parent = parent_element;  // TODO: only setting parent on first_child?
                    
                    // The first element is found, we assume its also the first_in_flow
                    parent_element->first_in_flow = new_statement_element;
                    // We have only found one child (yet) so we assume its also the last_in_flow
                    parent_element->last_in_flow = new_statement_element;
                }
                else 
                {
                    if (previous_statement_element)
                    {
                        previous_statement_element->next_sibling = new_statement_element;
                    }
                    new_statement_element->previous_sibling = previous_statement_element;
                    
                    // We found a new element and we already have children, so we only set last_in_flow
                    parent_element->last_in_flow = new_statement_element;
                }
                
                if (previous_statement_element)
                {
                    previous_statement_element->last_in_flow->next_in_flow = new_statement_element->first_in_flow;
                    new_statement_element->first_in_flow->previous_in_flow = previous_statement_element->last_in_flow;
                }
                    
                previous_statement_element = new_statement_element;
            }
        }
        while((statement_node = statement_node->next_sibling));
    }
}
