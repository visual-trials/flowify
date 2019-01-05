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

enum FlowElementType
{
    FlowElement_Unknown,
    
    // Control flow elements
    FlowElement_Root,
    FlowElement_Function,
    FlowElement_FunctionBody,
    
    FlowElement_If,
    FlowElement_IfCond,
    FlowElement_IfSplit,
    FlowElement_IfThen,
    FlowElement_IfElse,
    FlowElement_IfJoin,
    
    FlowElement_For,
    FlowElement_ForStart,
    FlowElement_ForInit,
    FlowElement_ForJoin,
    FlowElement_ForCond,
    FlowElement_ForSplit,
    FlowElement_ForBody,
    FlowElement_ForUpdate,
    FlowElement_ForDone,
    
    FlowElement_PassThrough,
    FlowElement_PassBack,
    
    FlowElement_Return,
    FlowElement_Break,
    FlowElement_Continue,
    
    // Data flow elements
    FlowElement_Hidden,         // TODO: using this to signify the contents of a function we don't know it's implementation of
    FlowElement_Assignment,
    FlowElement_BinaryOperator,
    FlowElement_FunctionCall,   // TODO: is this redundant?
    FlowElement_Scalar
};
// TODO: Keep this in sync with the enum above!
// TODO: DON'T FORGET THE COMMAS!!
const char * flow_element_type_names[] = {
    "Unknown",
    
    // Control flow elements
    "Root",
    "Function",
    "FunctionBody",
    
    "If",
    "IfCond",
    "IfSplit",
    "IfThen",
    "IfElse",
    "IfJoin",
    
    "For",
    "ForStart",
    "ForInit",
    "ForJoin",
    "ForCond",
    "ForSplit",
    "ForBody",
    "ForUpdate",
    "ForDone",
    
    "PassThrough",
    "PassBack",
    
    "Return",
    "Break",
    "Continue",
    
    // Data flow elements
    "Hidden",
    "Assignment",
    "BinaryOperator",
    "FunctionCall",   // TODO: is this redundant?
    "Scalar"
};

struct FlowElement
{
    // FIXME: Just testing memory!! Remove this!    ShortString dummy[10];
    
    FlowElementType type;
    
    Node * ast_node;
    
    // These 4 FlowElements represent a parental and sibling "hierarchy" (mostly used for "looping" through all elements)
    FlowElement * first_child;
    FlowElement * parent;
    
    FlowElement * next_sibling;
    FlowElement * previous_sibling;
    
    // These 4 FlowElements represent the control-flow between elements (first 2 are only used by non-split and non-join and non-composite elements, e.g. "pure rectangle elements")
    FlowElement * next_in_flow;
    FlowElement * previous_in_flow;
    
    FlowElement * first_in_flow;  // helper, for example: the if-start if first_in_flow of the if-element (so the statement before the if knows it should "connect" with the first_in_flow of the if-statment element)
    FlowElement * last_in_flow; // helper, for example: the if-end if last_in_flow of the if-element
    
    // This FlowElement represents a chained-list of all functions
    FlowElement * next_function; // TODO: we should probably not store this in FlowElement
    
    b32 has_lane_segments;
    b32 is_selected;
    
    Size2d size;
    Pos2d position;
    // TODO: i32 scale;
    Pos2d absolute_position;
    
    // TODO: Pos2d left_top_absolute_position;
    // TODO: Pos2d bottom_right_absolute_position;
    // TODO: i32 absolute_scale;
    
    // HitShape * hit_box;               or is this part of the shape(s)???
    
    HighlightedLinePart highlighted_line_part; // This is for highlighting in the dump
};

struct Flowifier
{
    DynamicArray flow_elements;
    
    FlowElement * first_function;
    FlowElement * latest_function;
};

void init_flowifier(Flowifier * flowifier)
{
    flowifier->first_function = 0;
    flowifier->latest_function = 0;
    
    init_dynamic_array(&flowifier->flow_elements, sizeof(FlowElement), (Color4){0,255,255,255}, cstring_to_string("Flowifier"));
}

FlowElement * new_flow_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type = FlowElement_Unknown)
{
    FlowElement empty_flow_element = {};
    
    FlowElement * new_flow_element = (FlowElement *)add_to_array(&flowifier->flow_elements, &empty_flow_element);
    
    new_flow_element->ast_node = ast_node;
    
    new_flow_element->first_child = 0;
    new_flow_element->parent = 0;
    new_flow_element->next_sibling = 0;
    new_flow_element->previous_sibling = 0;
    
    new_flow_element->next_in_flow = 0;
    new_flow_element->previous_in_flow = 0;
    new_flow_element->first_in_flow = 0;
    new_flow_element->last_in_flow = 0;
    
    new_flow_element->next_function = 0;
    
    new_flow_element->type = flow_element_type;
    
    new_flow_element->position.x = 0;
    new_flow_element->position.y = 0;
    
    new_flow_element->size.width = 0;
    new_flow_element->size.height = 0;
    
    new_flow_element->absolute_position.x = 0;
    new_flow_element->absolute_position.y = 0;

    new_flow_element->has_lane_segments = false;
    
    new_flow_element->is_selected = false;
    
    return new_flow_element;
}

FlowElement * get_function_element(Flowifier * flowifier, String identifier)
{
    FlowElement * function_element = flowifier->first_function;
    if (function_element)
    {
        do
        {
            if (equals(function_element->ast_node->identifier, identifier))
            {
                return function_element;
            }
        }
        while((function_element = function_element->next_function));
    }
    return 0;
}

void add_sibling(FlowElement * existing_sibling, FlowElement * new_sibling)
{
    existing_sibling->next_sibling = new_sibling;
    new_sibling->previous_sibling = existing_sibling;
}

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
                
                FlowElement * function_element = get_function_element(flowifier, identifier);
                
                if (function_element)
                {
                    new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_FunctionCall);
                    new_expression_element->first_child = function_element;
                    // TODO: set parent?
                }
                else {
                    // log("Unknown function:");
                    // log(identifier);
                    new_expression_element = new_flow_element(flowifier, expression_node, FlowElement_FunctionCall);
                    
                    FlowElement * hidden_element = new_flow_element(flowifier, expression_node, FlowElement_Hidden);
                    new_expression_element->first_child = hidden_element;
                    // TODO: set parent?
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
        new_statement_element->first_in_flow = new_expression_element;
        new_statement_element->last_in_flow = new_expression_element;
        
    }
    else if (statement_node->type == Node_Stmt_If)
    {
        FlowElement * if_element = new_flow_element(flowifier, statement_node, FlowElement_If);
        
        Node * if_cond_node = statement_node->first_child;
        Node * if_then_node = if_cond_node->next_sibling;
        Node * if_else_node = if_then_node->next_sibling;
        
        // TODO: not sure if if_split_element correponds to if_cond_node
        //       No, it doesn't. We need a if_split_element AND a if_cond_element. The if_cond_element corresponds to the if_cond_node.
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
        }
        
        // TODO: to which ast-node does this correpond (if any)?
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
        }
        
        // TODO: to which ast-node does this correpond (if any)?
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
        function_element->first_in_flow = function_element;
        function_element->last_in_flow = function_element;
        
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
                }
                else 
                {
                    if (previous_statement_element)
                    {
                        previous_statement_element->next_sibling = new_statement_element;
                    }
                    new_statement_element->previous_sibling = previous_statement_element;
                    
                }
                
                if (previous_statement_element)
                {
                    previous_statement_element->next_in_flow = new_statement_element->first_in_flow;
                }
                new_statement_element->previous_in_flow = previous_statement_element->last_in_flow;
                    
                previous_statement_element = new_statement_element;
            }
        }
        while((statement_node = statement_node->next_sibling));
    }
}

void layout_elements(FlowElement * flow_element)
{
    // FIXME: we should get this from Flowifier!
    i32 bending_radius = 20;
    
    if (flow_element->type == FlowElement_Hidden)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 20;
    }
    else if (flow_element->type == FlowElement_PassThrough)
    {
        flow_element->size.width = 40;
        flow_element->size.height = 80;
        flow_element->has_lane_segments = true;
    }
    else if (flow_element->type == FlowElement_Assignment)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 80;
        flow_element->has_lane_segments = true;
    }
    else if (flow_element->type == FlowElement_BinaryOperator)
    {
        flow_element->size.width = 200;
        flow_element->size.height = 80;
        flow_element->has_lane_segments = true;
    }
    else if (flow_element->type == FlowElement_Return)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 40;
        flow_element->has_lane_segments = true;
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;
        
        layout_elements(if_then_element);
        layout_elements(if_else_element);
        
        i32 then_else_height = if_then_element->size.height;
        if (if_else_element->size.height > then_else_height)
        {
            then_else_height = if_else_element->size.height;
        }
        
        i32 middle_margin = 80;
        i32 vertical_margin = 150;

        Pos2d start_position = {0,0};
        
        Pos2d current_position = start_position;
        
        if_cond_element->position = current_position;
        if_cond_element->size.height = 80;
        if_cond_element->size.width = 100;
        if_cond_element->has_lane_segments = true;
        
        current_position.y += if_cond_element->size.height;
        
        if_split_element->position = current_position;
        if_split_element->size.height = 20;
        if_split_element->size.width = 100; // if_then_element->size.width + middle_margin + if_else_element->size.width;
        if_split_element->has_lane_segments = true;
        
        current_position.y += if_split_element->size.height + vertical_margin;
        
        Pos2d current_position_right = current_position;
        Pos2d current_position_left = current_position;
        
        current_position_right.x += if_else_element->size.width + middle_margin;
        
        if_else_element->position = current_position_left;
        
        if_then_element->position = current_position_right;
        
        current_position.y += then_else_height + vertical_margin;
        
        if_join_element->position = current_position;
        if_join_element->size.width = 100; //if_then_element->size.width + middle_margin + if_else_element->size.width;
        if_join_element->size.height = 20;
        if_join_element->has_lane_segments = true;
        
        current_position.y += if_join_element->size.height;
        
        i32 total_height = current_position.y - start_position.y;
        i32 total_width = current_position_right.x + if_then_element->size.width - start_position.x;
        
        flow_element->size.height = total_height;
        flow_element->size.width = total_width;
    }
    else if (flow_element->type == FlowElement_For)
    {
        FlowElement * for_start_element = flow_element->first_child;
        FlowElement * for_init_element = for_start_element->next_sibling;
        FlowElement * for_join_element = for_init_element->next_sibling;
        FlowElement * for_cond_element = for_join_element->next_sibling;
        FlowElement * for_split_element = for_cond_element->next_sibling;
        FlowElement * for_body_element = for_split_element->next_sibling;
        FlowElement * for_update_element = for_body_element->next_sibling;
        FlowElement * for_passright_element = for_update_element->next_sibling;
        FlowElement * for_passup_element = for_passright_element->next_sibling;
        FlowElement * for_passleft_element = for_passup_element->next_sibling;
        FlowElement * for_passdown_element = for_passleft_element->next_sibling;
        FlowElement * for_passthrough_element = for_passdown_element->next_sibling;
        FlowElement * for_done_element = for_passthrough_element->next_sibling;
        
        layout_elements(for_body_element);
        
        // TODO: we should layout for_init to get its (proper) width and height
        for_init_element->size.width = 100;
        for_init_element->size.height = 80;
        
        // TODO: we should layout for_cond to get its (proper) width and height
        for_cond_element->size.height = 80;
        for_cond_element->size.width = 100;
        
        // TODO: we should layout for_update to get its (proper) width and height
        for_update_element->size.height = 80;
        for_update_element->size.width = 100;
        
        i32 for_body_height = for_body_element->size.height;
        
        i32 passback_width = 50; // TODO: should actually depend on the number of data lines going through it
        i32 passback_height = 0;
        
        i32 passthrough_width = 50;
        
        i32 width_center_elements = 100; // for_start, for_join, for_split, for_done
        i32 middle_margin = 4 * bending_radius;
        i32 right_margin = 100;
        i32 vertical_margin = 50;

        Pos2d start_position = {0,0};

        // TODO: we want for_init and for_passthough to be aligned at their right-side! (and be to the left of the center)
        
        Pos2d current_position = start_position;
        
        current_position.x += for_init_element->size.width + 2 * bending_radius - width_center_elements / 2;
        // TODO: we are creating some space at the top here. But we probably want the entire For-element to be move to the left, so we don't need this vertical space.
        current_position.y += 100;

        for_start_element->position = current_position;
        for_start_element->size.height = 20;
        for_start_element->size.width = width_center_elements;
        for_start_element->has_lane_segments = true;
        
        current_position.y += for_start_element->size.height + vertical_margin + vertical_margin;
        current_position.x -= for_init_element->size.width + 2 * bending_radius - width_center_elements / 2;
        
        for_init_element->position = current_position;
        for_init_element->has_lane_segments = true;
        
        current_position.y += for_init_element->size.height + passback_width + bending_radius + bending_radius + bending_radius;
        current_position.x += for_init_element->size.width + 2 * bending_radius - width_center_elements / 2;
        
        for_join_element->position = current_position;
        for_join_element->size.height = 20;
        for_join_element->size.width = width_center_elements;
        for_join_element->has_lane_segments = true;
        
        current_position.y += for_join_element->size.height + vertical_margin;
        
        for_cond_element->position = current_position;
        for_cond_element->has_lane_segments = true;
        
        current_position.y += for_cond_element->size.height + vertical_margin;
        
        for_split_element->position = current_position;
        for_split_element->size.height = 20;
        for_split_element->size.width = width_center_elements;
        for_split_element->has_lane_segments = true;
        
        current_position.y += for_split_element->size.height + vertical_margin + vertical_margin + vertical_margin;
        
        current_position.x -= passthrough_width + 2 * bending_radius - width_center_elements / 2;
        
        Pos2d current_position_right = current_position;
        Pos2d current_position_left = current_position;

        for_passthrough_element->position = current_position_left;
        for_passthrough_element->size.height = 20;
        for_passthrough_element->size.width = passthrough_width;
        
        current_position_right.x += for_passthrough_element->size.width + middle_margin;
        for_body_element->position = current_position_right;
        
        current_position_right.y += for_body_element->size.height;
        
        for_update_element->position = current_position_right;
        for_update_element->has_lane_segments = true;
        
        current_position_right.y += for_update_element->size.height + vertical_margin;
        
        for_passright_element->position.y = current_position_right.y;
        for_passright_element->position.x = current_position_right.x + for_body_element->size.width + right_margin / 2; // TODO: use bending radius here?
        for_passright_element->size.height = passback_width;
        for_passright_element->size.width = passback_height;
        
        current_position_right.y += for_passright_element->size.height + vertical_margin;
        
        for_passup_element->position.y = for_split_element->position.y;
        for_passup_element->position.x = current_position_right.x + for_body_element->size.width + right_margin;
        for_passup_element->size.height = passback_height;
        for_passup_element->size.width = passback_width;
        
        for_passleft_element->position.y = for_init_element->position.y + for_init_element->size.height - passback_width - passback_height - bending_radius;
        for_passleft_element->position.x = current_position_right.x + for_body_element->size.width + right_margin / 2; // TODO: use bending radius here?
        for_passleft_element->size.height = passback_width;
        for_passleft_element->size.width = passback_height;
        
        for_passdown_element->position.y = for_init_element->position.y + for_init_element->size.height - passback_height;
        for_passdown_element->position.x = for_init_element->position.x + for_init_element->size.width + middle_margin;
        for_passdown_element->size.height = passback_height;
        for_passdown_element->size.width = passback_width;
        
        // FIXME: we are assuming the body + update is always vertically larger than the for_passthrough_element
        current_position_left.y = current_position_right.y;
        
        for_done_element->position = current_position_left;
        for_done_element->size.height = 20;
        for_done_element->size.width = width_center_elements;
        
        current_position_left.y += for_done_element->size.height;
        
        i32 total_height = current_position_left.y - start_position.y;
        i32 total_width = for_passup_element->position.x + for_passup_element->size.width - start_position.x;
        
        flow_element->size.height = total_height;
        flow_element->size.width = total_width;
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        FlowElement * function_element = flow_element->first_child;
        
        i32 top_margin = 20;
        i32 bottom_margin = 20;
        
        i32 left_margin = 20;
        i32 right_margin = 20;
            
        layout_elements(function_element);
        
        function_element->position.x = left_margin;
        function_element->position.y = top_margin;
        
        flow_element->size.width = left_margin + function_element->size.width + right_margin;
        flow_element->size.height = top_margin + function_element->size.height + bottom_margin; 
        flow_element->has_lane_segments = true;
        
        // flow_element->size = function_element->size;
    }
    else if (flow_element->type == FlowElement_Function)
    {
        FlowElement * function_body_element = flow_element->first_child;
        
        layout_elements(function_body_element);
        
        flow_element->size = function_body_element->size;
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForBody)
    {
        // FIXME: position shouldnt be set here!
        flow_element->position.x = 0;
        flow_element->position.y = 0;
        
        i32 top_margin = 0;
        i32 bottom_margin = 0;
        
        i32 left_margin = 0;
        i32 right_margin = 0;
        
        if (flow_element->type == FlowElement_Root)
        {
            top_margin = 20;
            bottom_margin = 20;
            
            left_margin = 20;
            right_margin = 20;
        }
        
        i32 summed_children_height = 0;
        i32 largest_child_width = 0;
        
        i32 verical_margin = 0; // TODO: we need vertical margin when connections between elements have to run horizintal
        
        FlowElement * child_element = flow_element->first_child;
        
        b32 is_first_element = true;
        if (child_element)
        {
            do
            {
                layout_elements(child_element);
                
                // TODO: the child can be wider or narrower at the bottom or top!
                Size2d child_size = child_element->size;
                
                if (!is_first_element)
                {
                    summed_children_height += verical_margin;
                }
                
                child_element->position.x = left_margin;
                child_element->position.y = top_margin + summed_children_height;
                
                summed_children_height += child_size.height;
                if (child_size.width > largest_child_width)
                {
                    largest_child_width = child_size.width;
                }
                
                is_first_element = false;
            }
            while ((child_element = child_element->next_sibling));
            
        }
    
        flow_element->size.width = left_margin + largest_child_width + right_margin;
        flow_element->size.height = top_margin + summed_children_height + bottom_margin; 
        
    }
}

void absolute_layout_elements(FlowElement * flow_element, Pos2d absolute_parent_position)
{
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_PassBack || 
        flow_element->type == FlowElement_Assignment || 
        flow_element->type == FlowElement_BinaryOperator ||
        flow_element->type == FlowElement_Return)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
    }
    else if (flow_element->type == FlowElement_If)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;

        absolute_layout_elements(if_cond_element, flow_element->absolute_position);
        absolute_layout_elements(if_split_element, flow_element->absolute_position);
        absolute_layout_elements(if_then_element, flow_element->absolute_position);
        absolute_layout_elements(if_else_element, flow_element->absolute_position);
        absolute_layout_elements(if_join_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_IfCond ||
             flow_element->type == FlowElement_IfSplit ||
             flow_element->type == FlowElement_IfJoin
             )
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
    }
    else if (flow_element->type == FlowElement_For)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * for_start_element = flow_element->first_child;
        FlowElement * for_init_element = for_start_element->next_sibling;
        FlowElement * for_join_element = for_init_element->next_sibling;
        FlowElement * for_cond_element = for_join_element->next_sibling;
        FlowElement * for_split_element = for_cond_element->next_sibling;
        FlowElement * for_body_element = for_split_element->next_sibling;
        FlowElement * for_update_element = for_body_element->next_sibling;
        FlowElement * for_passright_element = for_update_element->next_sibling;
        FlowElement * for_passup_element = for_passright_element->next_sibling;
        FlowElement * for_passleft_element = for_passup_element->next_sibling;
        FlowElement * for_passdown_element = for_passleft_element->next_sibling;
        FlowElement * for_passthrough_element = for_passdown_element->next_sibling;
        FlowElement * for_done_element = for_passthrough_element->next_sibling;

        absolute_layout_elements(for_start_element, flow_element->absolute_position);
        absolute_layout_elements(for_init_element, flow_element->absolute_position);
        absolute_layout_elements(for_join_element, flow_element->absolute_position);
        absolute_layout_elements(for_cond_element, flow_element->absolute_position);
        absolute_layout_elements(for_split_element, flow_element->absolute_position);
        absolute_layout_elements(for_body_element, flow_element->absolute_position);
        absolute_layout_elements(for_update_element, flow_element->absolute_position);
        absolute_layout_elements(for_passright_element, flow_element->absolute_position);
        absolute_layout_elements(for_passup_element, flow_element->absolute_position);
        absolute_layout_elements(for_passleft_element, flow_element->absolute_position);
        absolute_layout_elements(for_passdown_element, flow_element->absolute_position);
        absolute_layout_elements(for_passthrough_element, flow_element->absolute_position);
        absolute_layout_elements(for_done_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_ForStart ||
             flow_element->type == FlowElement_ForInit ||
             flow_element->type == FlowElement_ForJoin ||
             flow_element->type == FlowElement_ForCond ||
             flow_element->type == FlowElement_ForSplit ||
             flow_element->type == FlowElement_ForUpdate ||
             flow_element->type == FlowElement_ForDone
             )
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * function_element = flow_element->first_child;

        absolute_layout_elements(function_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_Function)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * function_body_element = flow_element->first_child;

        absolute_layout_elements(function_body_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForBody
             )
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * child_element = flow_element->first_child;
        if (child_element)
        {
            do
            {
                absolute_layout_elements(child_element, flow_element->absolute_position);
            }
            while ((child_element = child_element->next_sibling));
        }
    }
    
}

void draw_joining_element(FlowElement * left_element, FlowElement * right_element, 
                          FlowElement * joining_element, FlowElement * element_next_in_flow, 
                          b32 show_help_rectangles)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    Color4 no_color         = {};
    Color4 fill_color = unselected_color;
    if (joining_element->is_selected)
    {
        fill_color = selected_color;
    }
        
    i32 line_width = 4;
    i32 bending_radius = 20;
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // Right element position + size
    Rect2d right_rect = {};
    // FIXME: we need the position and size of the LAST statement in the then-element, NOT of the then-element itself!
    right_rect.position = right_element->absolute_position;
    right_rect.size = right_element->size;
    
    // Left element position + size
    Rect2d left_rect = {};
    // FIXME: we need the position and size of the LAST statement in the else-element, NOT of the else-element itself!
    left_rect.position = left_element->absolute_position;
    left_rect.size = left_element->size;
    
    // Joining element positions + size
    Rect2d middle_rect = {};
    middle_rect.position = joining_element->absolute_position;
    middle_rect.size = joining_element->size;
    
    Rect2d bottom_rect = {-1,-1,-1,-1};
    
    if (element_next_in_flow)
    {
        bottom_rect.position = element_next_in_flow->absolute_position;
        bottom_rect.size = element_next_in_flow->size;
    }
    
    draw_lane_segments_for_4_rectangles(bottom_rect, false, left_rect, right_rect, middle_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    draw_lane_segments_for_3_rectangles(no_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    
    if (show_help_rectangles)
    {
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, rectangle_fill, 2);
    }
}

void draw_splitting_element(FlowElement * left_element, FlowElement * right_element, FlowElement * splitting_element, b32 show_help_rectangles)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    Color4 no_color         = {};
    Color4 fill_color = unselected_color;
    if (splitting_element->is_selected)
    {
        fill_color = selected_color;
    }
        
    i32 line_width = 4;
    i32 bending_radius = 20;
    Rect2d no_rect = {-1,-1,-1,-1};

        
    Rect2d top_rect = no_rect;
    
    // If-start rectangle (middle)
    Rect2d middle_rect = {};
    middle_rect.position = splitting_element->absolute_position;
    middle_rect.size = splitting_element->size;
    
    // If-then rectangle (right)
    Rect2d right_rect = {};
    // FIXME: we need the position and size of the FIRST statement in the then-element, NOT of the then-element itself!
    right_rect.position = right_element->absolute_position;
    right_rect.size = right_element->size;
    
    // If-else rectangle (left)
    Rect2d left_rect = {};
    // FIXME: we need the position and size of the FIRST statement in the else-element, NOT of the else-element itself!
    left_rect.position = left_element->absolute_position;
    left_rect.size = left_element->size;

    draw_lane_segments_for_4_rectangles(top_rect, true, left_rect, right_rect, middle_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    // FIXME: we somehow need the previous element, BEFORE the start-if as top_rect!
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, no_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    
    if (show_help_rectangles)
    {
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, rectangle_fill, 2);
    }
}

void draw_element_rectangle(FlowElement * flow_element)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    
    Rect2d rect = {};
    rect.position = flow_element->absolute_position;
    rect.size = flow_element->size;
    
    draw_rectangle(rect.position, rect.size, rectangle_color, rectangle_fill, 2);
}

void draw_straight_element(FlowElement * flow_element, FlowElement * element_previous_in_flow, 
                           FlowElement * element_next_in_flow, b32 show_help_rectangles)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 no_color         = {};
    Color4 fill_color = unselected_color;
    if (flow_element->is_selected)
    {
        fill_color = selected_color;
    }
        
    i32 line_width = 4;
    i32 bending_radius = 20;
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // Positions and sizes
    Rect2d top_rect = no_rect;
    Rect2d middle_rect = no_rect;
    Rect2d bottom_rect = no_rect;
    
    Pos2d position = flow_element->absolute_position;
    Size2d size = flow_element->size;
    middle_rect.position = position;
    middle_rect.size = size;
    
    if (element_previous_in_flow)
    {
        top_rect.position = element_previous_in_flow->absolute_position;
        top_rect.size = element_previous_in_flow->size;
    }
    if (element_next_in_flow)
    {
        bottom_rect.position = element_next_in_flow->absolute_position;
        bottom_rect.size = element_next_in_flow->size;
    }
    
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    if (show_help_rectangles)
    {
        draw_element_rectangle(flow_element);
    }
}

void draw_elements(FlowElement * flow_element, b32 show_help_rectangles)
{
    // TODO: add is_position_of and position_originates_from
    // TODO: set colors properly
    
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)
    
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    Color4 no_color         = {};
    i32 line_width = 4;
    i32 bending_radius = 20;
    
    Rect2d no_rect = {-1,-1,-1,-1};
    
    Color4 function_line_color = { 200, 200, 200, 255};
    Color4 function_fill_color = { 240, 240, 240, 255};
    i32 function_line_width = 2;
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_Assignment || 
        flow_element->type == FlowElement_BinaryOperator ||
        flow_element->type == FlowElement_Return)
    {
        draw_straight_element(flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_element = flow_element;
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;

        draw_straight_element(if_cond_element, if_cond_element->previous_in_flow, if_split_element, show_help_rectangles);
        draw_splitting_element(if_else_element, if_then_element, if_split_element, show_help_rectangles);
        draw_elements(if_then_element, show_help_rectangles);
        draw_elements(if_else_element, show_help_rectangles);
        draw_joining_element(if_else_element, if_then_element, if_join_element, if_element->next_in_flow, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_For)
    {
        FlowElement * for_element = flow_element;
        FlowElement * for_start_element = flow_element->first_child;
        FlowElement * for_init_element = for_start_element->next_sibling;
        FlowElement * for_join_element = for_init_element->next_sibling;
        FlowElement * for_cond_element = for_join_element->next_sibling;
        FlowElement * for_split_element = for_cond_element->next_sibling;
        FlowElement * for_body_element = for_split_element->next_sibling;
        FlowElement * for_update_element = for_body_element->next_sibling;
        FlowElement * for_passright_element = for_update_element->next_sibling;
        FlowElement * for_passup_element = for_passright_element->next_sibling;
        FlowElement * for_passleft_element = for_passup_element->next_sibling;
        FlowElement * for_passdown_element = for_passleft_element->next_sibling;
        FlowElement * for_passthrough_element = for_passdown_element->next_sibling;
        FlowElement * for_done_element = for_passthrough_element->next_sibling;

        draw_straight_element(for_start_element, for_start_element->previous_in_flow, for_init_element, show_help_rectangles);
        
        draw_straight_element(for_init_element, for_start_element, 0, show_help_rectangles);
        
        draw_joining_element(for_init_element, for_passdown_element, for_join_element, for_cond_element, show_help_rectangles);
        
        draw_straight_element(for_cond_element, for_join_element, for_split_element, show_help_rectangles);
        draw_splitting_element(for_passthrough_element, for_body_element, for_split_element, show_help_rectangles);
        draw_elements(for_body_element, show_help_rectangles);
        // TODO: The previous_in_flow and next_in_flow of the update_element are not correct (previous should be last statement in for-body, next should be?)
        draw_straight_element(for_update_element, for_body_element, 0, show_help_rectangles);
        
        // FIXME: we need a get_rect_from_element-function!
        Rect2d update_rect = {};
        update_rect.position = for_update_element->absolute_position;
        update_rect.size = for_update_element->size;
        
        Rect2d passright_rect = {};
        passright_rect.position = for_passright_element->absolute_position;
        passright_rect.size = for_passright_element->size;
        
        Rect2d passup_rect = {};
        passup_rect.position = for_passup_element->absolute_position;
        passup_rect.size = for_passup_element->size;
        
        Rect2d passleft_rect = {};
        passleft_rect.position = for_passleft_element->absolute_position;
        passleft_rect.size = for_passleft_element->size;
        
        Rect2d passdown_rect = {};
        passdown_rect.position = for_passdown_element->absolute_position;
        passdown_rect.size = for_passdown_element->size;
        
        HorLine hor_line = get_bottom_line_from_rect(update_rect);
        VertLine vert_line = get_left_line_from_rect(passright_rect);
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);
        
        vert_line = get_left_line_from_rect(passright_rect);
        hor_line = get_bottom_line_from_rect(passup_rect);
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);
        
        hor_line = get_bottom_line_from_rect(passup_rect);
        vert_line = get_left_line_from_rect(passleft_rect);
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);

        vert_line = get_left_line_from_rect(passleft_rect);
        hor_line = get_bottom_line_from_rect(passdown_rect);
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);

        if (show_help_rectangles)
        {
            draw_element_rectangle(for_passright_element);
            draw_element_rectangle(for_passup_element);
            draw_element_rectangle(for_passleft_element);
            draw_element_rectangle(for_passdown_element);
        }
        
        draw_straight_element(for_passthrough_element, 0, for_done_element, show_help_rectangles);
        draw_straight_element(for_done_element, for_passthrough_element, for_done_element->next_in_flow, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        // Colors
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        
        // FIXME: HACK!
        fill_color.r -= 30;
        fill_color.g -= 30;
        fill_color.b -= 30;
        
        
        // Positions and sizes
        Rect2d top_rect = {-1,-1,-1,-1};
        Rect2d middle_rect = {-1,-1,-1,-1};
        Rect2d bottom_rect = {-1,-1,-1,-1};
        
        Pos2d position = flow_element->absolute_position;
        Size2d size = flow_element->size;
        middle_rect.position = position;
        middle_rect.size = size;
        
        if (flow_element->previous_in_flow)
        {
            top_rect.position = flow_element->previous_in_flow->absolute_position;
            top_rect.size = flow_element->previous_in_flow->size;
        }
        if (flow_element->next_in_flow)
        {
            bottom_rect.position = flow_element->next_in_flow->absolute_position;
            bottom_rect.size = flow_element->next_in_flow->size;
        }
        
        draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        
        if (show_help_rectangles)
        {
            draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, rectangle_fill, 2);
        }
        
        // Drawing the function itself
        
        FlowElement * function_element = flow_element->first_child;

        draw_elements(function_element, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_Function)
    {
        Pos2d position = flow_element->absolute_position;
        
        FlowElement * function_body_element = flow_element->first_child;

        draw_elements(function_body_element, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForBody
             )
    {
        Pos2d position = flow_element->absolute_position;
        
        /*
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        */
        
        if (flow_element->type == FlowElement_Root)
        {
            draw_rounded_rectangle(position, flow_element->size, 20, 
                                   function_line_color, function_fill_color, function_line_width);
        }
                               
        FlowElement * child_element = flow_element->first_child;
        if (child_element)
        {
            do
            {
                draw_elements(child_element, show_help_rectangles);
            }
            while ((child_element = child_element->next_sibling));
        }
        
    }
}

i32 dump_element_tree(FlowElement * element, DynamicString * dump_text, i32 dump_line_index = 0, i32 depth = 0)
{
    String element_type_string = cstring_to_string(flow_element_type_names[element->type]);
    for (i32 indentation_index = 0; indentation_index < depth; indentation_index++)
    {
        append_string(dump_text, cstring_to_string("    "));
    }
    append_string(dump_text, element_type_string);
    append_string(dump_text, cstring_to_string("\n"));
    
    element->highlighted_line_part.line_index = dump_line_index++;
    element->highlighted_line_part.start_character_index = (u16)(depth * 4);
    element->highlighted_line_part.length = (u16)element_type_string.length;
    
    if (element->first_child)
    {
        dump_line_index = dump_element_tree(element->first_child, dump_text, dump_line_index, depth + 1);
    }

    if (element->next_sibling)
    {
        dump_line_index = dump_element_tree(element->next_sibling, dump_text, dump_line_index, depth);
    }
    
    return dump_line_index;
}
