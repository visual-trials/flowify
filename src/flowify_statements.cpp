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
    FlowElement_IfStart,
    FlowElement_IfThen,
    FlowElement_IfElse,
    FlowElement_IfEnd,
    
    FlowElement_For,
    
    FlowElement_PassThrough,
    
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
    "IfStart",
    "IfThen",
    "IfElse",
    "IfEnd",
    
    "For",
    
    "PassThrough",
    
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
    FlowElementType type;
    
    Node * ast_node;
    
    FlowElement * first_child;
    
    FlowElement * next_sibling;
    FlowElement * previous_sibling;
    
    FlowElement * next_function; // TODO: we should probably not store this in FlowElement
    
    b32 is_selected;
    
    Size2d size;
    Pos2d position;
    // TODO: i32 scale;
    
    Pos2d left_top_absolute_position;
    Pos2d bottom_right_absolute_position;
    // TODO: i32 absolute_scale;
    
    // HitShape * hit_box;               or is this part of the shape(s)???
    
    HighlightedLinePart highlighted_line_part; // This is for highlighting in the dump
};

struct Flowifier
{
    FlowElement flow_elements[100]; // TODO: allocate this properly
    i32 nr_of_flow_elements;
    
    FlowElement * first_function;
    FlowElement * latest_function;
};

FlowElement * new_flow_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type = FlowElement_Unknown)
{
    FlowElement * new_flow_element = &flowifier->flow_elements[flowifier->nr_of_flow_elements++];
    new_flow_element->ast_node = ast_node;
    new_flow_element->first_child = 0;
    new_flow_element->next_sibling = 0;
    new_flow_element->previous_sibling = 0;
    new_flow_element->next_function = 0;
    new_flow_element->type = flow_element_type;
    new_flow_element->position.x = 0;
    new_flow_element->position.y = 0;
    new_flow_element->size.width = 0;
    new_flow_element->size.height = 0;

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

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element);

FlowElement * flowify_statement(Flowifier * flowifier, Node * statement_node)
{
    FlowElement * new_statement_element = 0;
    
    if (statement_node->type == Node_Stmt_Expr)
    {
        
        // TODO: we should flowify the expression! (for now we create a dummy element)
        
        FlowElementType element_type = FlowElement_Assignment;
        if (statement_node->first_child && statement_node->first_child->first_child)
        {
            if (statement_node->first_child->first_child->next_sibling)
            {
                if (statement_node->first_child->first_child->next_sibling->type == Node_Expr_PostInc)
                {
                    // FIXME: hack!
                    new_statement_element = new_flow_element(flowifier, statement_node, FlowElement_BinaryOperator);
                }
                else if (statement_node->first_child->first_child->next_sibling->type == Node_Expr_FuncCall)
                {
                    String identifier = statement_node->first_child->first_child->next_sibling->identifier;
                    
                    FlowElement * function_element = get_function_element(flowifier, identifier);
                    
                    if (function_element)
                    {
                        new_statement_element = new_flow_element(flowifier, statement_node, FlowElement_FunctionCall);
                        new_statement_element->first_child = function_element;
                    }
                    else {
                        // log("Unknown function:");
                        // log(identifier);
                        new_statement_element = new_flow_element(flowifier, statement_node, FlowElement_FunctionCall);
                        
                        FlowElement * hidden_element = new_flow_element(flowifier, statement_node, FlowElement_Hidden);
                        new_statement_element->first_child = hidden_element;
                    }

                }
                else
                {
                    // FIXME: hack!
                    new_statement_element = new_flow_element(flowifier, statement_node, FlowElement_Assignment);
                }
            }
            else
            {
                // FIXME: hack!
                new_statement_element = new_flow_element(flowifier, statement_node, FlowElement_Assignment);
            }
        }
        else
        {
            // FIXME: this is not always an FlowElement_Assignment!
            new_statement_element = new_flow_element(flowifier, statement_node, FlowElement_Assignment);
        }
        
        
    }
    else if (statement_node->type == Node_Stmt_If)
    {
        FlowElement * if_element = new_flow_element(flowifier, statement_node, FlowElement_If);
        
        Node * if_cond_node = statement_node->first_child;
        Node * if_then_node = if_cond_node->next_sibling;
        Node * if_else_node = if_then_node->next_sibling;
        
        // TODO: not sure if if_start_element correponds to if_cond_node
        FlowElement * if_start_element = new_flow_element(flowifier, if_cond_node, FlowElement_IfStart); 
        
        FlowElement * if_then_element = new_flow_element(flowifier, if_then_node, FlowElement_IfThen);
        if (if_then_node && if_then_node->first_child)
        {
            flowify_statements(flowifier, if_then_element);
        }
        else
        {
            FlowElement * then_passthrough_element = new_flow_element(flowifier, 0, FlowElement_PassThrough);
            if_then_element->first_child = then_passthrough_element;
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
        }
        
        FlowElement * if_end_element = new_flow_element(flowifier, 0, FlowElement_IfEnd);
        
        if_element->first_child = if_start_element;
        
        if_start_element->next_sibling = if_then_element;
        if_then_element->previous_sibling = if_start_element;
        
        if_then_element->next_sibling = if_else_element;
        if_else_element->previous_sibling = if_then_element;
        
        if_else_element->next_sibling = if_end_element;
        if_end_element->previous_sibling = if_else_element;
        
        new_statement_element = if_element;
    }
    else if (statement_node->type == Node_Stmt_Function)
    {
        FlowElement * function_element = new_flow_element(flowifier, statement_node, FlowElement_Function);
        
        // TODO: flowify function arguments
        
        Node * function_body_node = statement_node->first_child->next_sibling;
        
        FlowElement * function_body_element = new_flow_element(flowifier, function_body_node, FlowElement_FunctionBody);
        
        flowify_statements(flowifier, function_body_element);
        
        function_element->first_child = function_body_element;
        
        new_statement_element = function_element;
    }
    else if (statement_node->type == Node_Stmt_Return)
    {
        FlowElement * return_element = new_flow_element(flowifier, statement_node, FlowElement_Return);
        
        // TODO: flowify return statement
        
        new_statement_element = return_element;
    }
    else if (statement_node->type == Node_Stmt_Break)
    {
        FlowElement * break_element = new_flow_element(flowifier, statement_node, FlowElement_Break);
        
        // TODO: flowify break statement
        
        new_statement_element = break_element;
    }
    else if (statement_node->type == Node_Stmt_Continue)
    {
        FlowElement * continue_element = new_flow_element(flowifier, statement_node, FlowElement_Continue);
        
        // TODO: flowify continue statement
        
        new_statement_element = continue_element;
    }
    
    return new_statement_element;
}

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element)
{
    Node * parent_node = parent_element->ast_node;
    Node * child_node = 0;
    FlowElement * previous_child_element = 0;
    
    // Functions
    child_node = parent_node->first_child;
    if (child_node)
    {
        do
        {
            if (child_node->type == Node_Stmt_Function)
            {
                FlowElement * new_function_element = flowify_statement(flowifier, child_node);

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
        while((child_node = child_node->next_sibling));
    }
    
    // Non-functions
    child_node = parent_node->first_child;
    previous_child_element = 0;
    if (child_node)
    {
        do
        {
            if (child_node->type != Node_Stmt_Function)
            {
                FlowElement * new_child_element = flowify_statement(flowifier, child_node);
                    
                if (!parent_element->first_child)
                {
                    parent_element->first_child = new_child_element;
                }
                else 
                {
                    previous_child_element->next_sibling = new_child_element;
                    new_child_element->previous_sibling = previous_child_element;
                }
                previous_child_element = new_child_element;
            }
        }
        while((child_node = child_node->next_sibling));
    }
}

void layout_elements(FlowElement * flow_element)
{
    if (flow_element->type == FlowElement_Hidden)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 20;
    }
    else if (flow_element->type == FlowElement_PassThrough)
    {
        flow_element->size.width = 40;
        flow_element->size.height = 80;
    }
    else if (flow_element->type == FlowElement_Assignment)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 80;
    }
    else if (flow_element->type == FlowElement_BinaryOperator)
    {
        flow_element->size.width = 200;
        flow_element->size.height = 80;
    }
    else if (flow_element->type == FlowElement_Return)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 40;
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_start_element = flow_element->first_child;
        FlowElement * if_then_element = if_start_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_end_element = if_else_element->next_sibling;
        
        layout_elements(if_then_element);
        layout_elements(if_else_element);
        
        i32 then_else_height = if_then_element->size.height;
        if (if_else_element->size.height > then_else_height)
        {
            then_else_height = if_else_element->size.height;
        }
        
        i32 middle_margin = 80;
        i32 vertical_margin = 50;

        if_start_element->position.x = 0;
        if_start_element->position.y = 0;
        if_start_element->size.height = 50;
        if_start_element->size.width = if_then_element->size.width + middle_margin + if_else_element->size.width;
        
        if_else_element->position.x = 0;
        if_else_element->position.y = if_start_element->size.height + vertical_margin;
        
        if_then_element->position.x = if_else_element->size.width + middle_margin;
        if_then_element->position.y = if_start_element->size.height + vertical_margin;
        
        if_end_element->position.x = 0;
        if_end_element->position.y = if_start_element->size.height + vertical_margin + then_else_height + vertical_margin;
        if_end_element->size.width = if_then_element->size.width + middle_margin + if_else_element->size.width;
        if_end_element->size.height = 50;
        
        flow_element->size.height = if_start_element->size.height + vertical_margin + then_else_height + vertical_margin + if_end_element->size.height;
        flow_element->size.width = if_then_element->size.width + middle_margin + if_else_element->size.width;
        
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
             flow_element->type == FlowElement_IfElse)
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

void draw_elements(FlowElement * flow_element, Pos2d parent_position)
{
    // TODO: convert relative to absolute positions!
    // TODO: add is_position_of and position_originates_from
    // TODO: set colors properly
    
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 no_color         = {};
    i32 line_width = 4;
    i32 bending_radius = 20;
    
    Rectangle no_rect = {-1,-1,-1,-1};
    
    Color4 function_line_color = { 200, 200, 200, 255};
    Color4 function_fill_color = { 240, 240, 240, 255};
    i32 function_line_width = 2;
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_Assignment || 
        flow_element->type == FlowElement_BinaryOperator ||
        flow_element->type == FlowElement_Return)
    {
        // Colors
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }

        // Positions and sizes
        Rectangle top_rect = {-1,-1,-1,-1};
        Rectangle middle_rect = {-1,-1,-1,-1};
        Rectangle bottom_rect = {-1,-1,-1,-1};
        
        Pos2d position = add_position_to_position(flow_element->position, parent_position);
        Size2d size = flow_element->size;
        middle_rect.position = position;
        middle_rect.size = size;
        
        if (flow_element->previous_sibling)
        {
            top_rect.position = add_position_to_position(flow_element->previous_sibling->position, parent_position);
            top_rect.size = flow_element->previous_sibling->size;
        }
        if (flow_element->next_sibling)
        {
            bottom_rect.position = add_position_to_position(flow_element->next_sibling->position, parent_position);
            bottom_rect.size = flow_element->next_sibling->size;
        }
        
        draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, no_color, 2);
    }
    else if (flow_element->type == FlowElement_If)
    {
        Pos2d position = add_position_to_position(flow_element->position, parent_position);
        
        FlowElement * if_start_element = flow_element->first_child;
        FlowElement * if_then_element = if_start_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_end_element = if_else_element->next_sibling;

        draw_elements(if_start_element, position);
        draw_elements(if_then_element, position);
        draw_elements(if_else_element, position);
        draw_elements(if_end_element, position);
    }
    else if (flow_element->type == FlowElement_IfStart)
    {
        // Colors
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        
        FlowElement * if_start_element = flow_element;
        FlowElement * if_then_element = if_start_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_end_element = if_else_element->next_sibling;
        
        Rectangle top_rect = {-1,-1,-1,-1};
        
        // If-start rectangle (middle)
        Rectangle middle_rect = {};
        middle_rect.position = add_position_to_position(if_start_element->position, parent_position);
        middle_rect.size = if_start_element->size;
        
        // If-then rectangle (right)
        Rectangle right_rect = {};
        right_rect.position = add_position_to_position(if_then_element->position, parent_position);
        right_rect.size = if_then_element->size;
        
        // If-else rectangle (left)
        Rectangle left_rect = {};
        left_rect.position = add_position_to_position(if_else_element->position, parent_position);
        left_rect.size = if_else_element->size;

        draw_lane_segments_for_4_rectangles(top_rect, true, left_rect, right_rect, middle_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        // FIXME: we somehow need the previous element, BEFORE the start-if as top_rect!
        draw_lane_segments_for_3_rectangles(top_rect, middle_rect, no_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, no_color, 2);
    }
    else if (flow_element->type == FlowElement_IfEnd)
    {
        // Colors
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        
        FlowElement * if_end_element = flow_element;
        FlowElement * if_else_element = if_end_element->previous_sibling;
        FlowElement * if_then_element = if_else_element->previous_sibling;
        FlowElement * if_start_element = if_then_element->previous_sibling;
        
        // If-then position + size (right)
        Rectangle right_rect = {};
        right_rect.position = add_position_to_position(if_then_element->position, parent_position);
        right_rect.size = if_then_element->size;
        
        // If-else position + size (left)
        Rectangle left_rect = {};
        left_rect.position = add_position_to_position(if_else_element->position, parent_position);
        left_rect.size = if_else_element->size;
        
        // If-end positions + size
        Rectangle middle_rect = {};
        middle_rect.position = add_position_to_position(if_end_element->position, parent_position);
        middle_rect.size = if_end_element->size;
        
        Rectangle bottom_rect = {-1,-1,-1,-1};
        
        draw_lane_segments_for_4_rectangles(bottom_rect, false, left_rect, right_rect, middle_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        // FIXME: we somehow need the next element, AFTER the end-if as bottom_rect!
        draw_lane_segments_for_3_rectangles(no_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, no_color, 2);
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
        Rectangle top_rect = {-1,-1,-1,-1};
        Rectangle middle_rect = {-1,-1,-1,-1};
        Rectangle bottom_rect = {-1,-1,-1,-1};
        
        Pos2d position = add_position_to_position(flow_element->position, parent_position);
        Size2d size = flow_element->size;
        middle_rect.position = position;
        middle_rect.size = size;
        
        if (flow_element->previous_sibling)
        {
            top_rect.position = add_position_to_position(flow_element->previous_sibling->position, parent_position);
            top_rect.size = flow_element->previous_sibling->size;
        }
        if (flow_element->next_sibling)
        {
            bottom_rect.position = add_position_to_position(flow_element->next_sibling->position, parent_position);
            bottom_rect.size = flow_element->next_sibling->size;
        }
        
        draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, no_color, 2);
        
        // Drawing the function itself
        
        FlowElement * function_element = flow_element->first_child;

        draw_elements(function_element, position);
    }
    else if (flow_element->type == FlowElement_Function)
    {
        Pos2d position = add_position_to_position(flow_element->position, parent_position);
        
        FlowElement * function_body_element = flow_element->first_child;

        draw_elements(function_body_element, position);
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse)
    {
        Pos2d position = add_position_to_position(flow_element->position, parent_position);
        
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
                draw_elements(child_element, position);
            }
            while ((child_element = child_element->next_sibling));
        }
        
    }
}

i32 dump_element_tree(FlowElement * element, String * dump_text, i32 dump_line_index = 0, i32 depth = 0)
{
    u8 temp_string[100]; // TODO: use a temp-memory buffer instead
    
    String indent_string = {};
    indent_string.data = temp_string;
    copy_cstring_to_string("    ", &indent_string);
    
    for (i32 indentation_index = 0; indentation_index < depth; indentation_index++)
    {
        append_string(dump_text, &indent_string);
    }
    
    String element_type_string = {};
    element_type_string.data = temp_string;
    copy_cstring_to_string(flow_element_type_names[element->type], &element_type_string);
    
    append_string(dump_text, &element_type_string);
    
    dump_text->data[dump_text->length] = '\n';
    dump_text->length++;
    
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
