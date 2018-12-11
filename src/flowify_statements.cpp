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
    
    FlowElement_If,
    FlowElement_IfStart,
    FlowElement_IfThen,
    FlowElement_IfElse,
    FlowElement_IfEnd,
    
    FlowElement_For,
    
    // Data flow elements
    FlowElement_Assignment,
    FlowElement_BinaryOperator,
    FlowElement_FunctionCall,   // TODO: is this redundant?
    FlowElement_Scalar,
};
// TODO: Keep this in sync with the enum above!
// TODO: DON'T FORGET THE COMMAS!!
const char * flow_element_type_names[] = {
    "Unknown",
    
    // Control flow elements
    "Root",
    "Function",
    
    "If",
    "IfStart",
    "IfThen",
    "IfElse",
    "IfEnd",
    
    "For",
    
    // Data flow elements
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
    new_flow_element->is_selected = false;
    return new_flow_element;
}

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element);

FlowElement * flowify_statement(Flowifier * flowifier, Node * statement_node)
{
    FlowElement * new_statement_element = 0;
    
    if (statement_node->type == Node_Stmt_Expr)
    {
        // FIXME: this is not always an FlowElement_Assignment!
        FlowElementType element_type = FlowElement_Assignment;
        if (statement_node->first_child && statement_node->first_child->first_child)
        {
            if (statement_node->first_child->first_child->next_sibling)
            {
                if (statement_node->first_child->first_child->next_sibling->type == Node_Expr_PostInc)
                {
                    // FIXME: hack!
                    element_type = FlowElement_BinaryOperator;
                }
                else if (statement_node->first_child->first_child->next_sibling->type == Node_Expr_FuncCall)
                {
                    // FIXME: hack!
                    element_type = FlowElement_FunctionCall;
                    
                    // FIXME: flowify the function itself!
                }
            }
        }
        
        // TODO: we should flowify the expression! (for now we create a dummy element)
        
        new_statement_element = new_flow_element(flowifier, statement_node, element_type);
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
        flowify_statements(flowifier, if_then_element);
        
        FlowElement * if_else_element = new_flow_element(flowifier, if_else_node, FlowElement_IfElse);
        if (if_else_node)
        {
            flowify_statements(flowifier, if_else_element);
        }
        else
        {
            // TODO: what to do with the if_else_element if no if_else_node is available?
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
    
    return new_statement_element;
}

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element)
{
    Node * parent_node = parent_element->ast_node;
    Node * child_node = 0;
    FlowElement * previous_child_element = 0;
    
    // Functions
    child_node = parent_node->first_child;
    if (child_node && child_node->type == Node_Stmt_Function)
    {
        do
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
        while((child_node = child_node->next_sibling));
    }
    
    // Non-functions
    child_node = parent_node->first_child;
    previous_child_element = 0;
    if (child_node && child_node->type != Node_Stmt_Function)
    {
        do
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
        while((child_node = child_node->next_sibling));
    }
}

void layout_elements(FlowElement * flow_element)
{
    if (flow_element->type == FlowElement_Assignment)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 100;
    }
    else if (flow_element->type == FlowElement_BinaryOperator)
    {
        flow_element->size.width = 200;
        flow_element->size.height = 100;
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_start_element = flow_element->first_child;
        FlowElement * if_then_element = if_start_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_end_element = if_else_element->next_sibling;
        
        layout_elements(if_then_element);
        layout_elements(if_else_element);
        
        i32 middle_margin = 80;

        flow_element->size.width = if_then_element->size.width + middle_margin + if_else_element->size.width;
        if_start_element->size.width = flow_element->size.width;
        if_end_element->size.width = flow_element->size.width;
        
        if_start_element->size.height = 100;
        if_end_element->size.height = 100;
        // TODO: determine whether if_then_element or if_else_element is higher
        flow_element->size.height = if_start_element->size.height + if_then_element->size.height + if_end_element->size.height;
        
        if_start_element->position.x = 0;
        if_then_element->position.x = 0;
        if_else_element->position.x = if_then_element->size.width + middle_margin;
        if_end_element->position.x = 0;
        
        if_start_element->position.y = 0;
        if_then_element->position.y = if_start_element->size.height;
        if_else_element->position.y = if_start_element->size.height;
        // TODO: determine whether if_then_element or if_else_element is higher
        if_end_element->position.y = if_start_element->size.height + if_then_element->size.height;
        
        // TODO: change/extend either the else- or the then- height
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionCall ||
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
        
        if (flow_element->type == FlowElement_Root ||
            flow_element->type == FlowElement_FunctionCall)
        {
            top_margin = 20;
            bottom_margin = 20;
            
            left_margin = 20;
            right_margin = 20;
        }
        
        i32 summed_children_height = 0;
        i32 largest_child_width = 40; // TODO: using a minimum width by setting this variable beforehand
        
        FlowElement * child_element = flow_element->first_child;
        
        if (child_element)
        {
            do
            {
                layout_elements(child_element);
                
                // TODO: the child can be wider or narrower at the bottom or top!
                Size2d child_size = child_element->size;
                
                child_element->position.x = left_margin;
                child_element->position.y = top_margin + summed_children_height;
                
                summed_children_height += child_size.height;
                if (child_size.width > largest_child_width)
                {
                    largest_child_width = child_size.width;
                }
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
    Color4 no_color         = {};
    i32 line_width = 4;
    
    Color4 function_line_color = { 200, 200, 200, 255};
    Color4 function_fill_color = { 240, 240, 240, 255};
    i32 function_line_width = 2;
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_Assignment || flow_element->type == FlowElement_BinaryOperator)
    {
        // Colors
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        
        // Size and positions
        Size2d size = flow_element->size;
        Size2d previous_size = size;
        Size2d next_size = size;
        if (flow_element->previous_sibling)
        {
            if (flow_element->previous_sibling->size.width >  size.width)
            {
                previous_size = flow_element->previous_sibling->size;
            }
        }
        if (flow_element->next_sibling)
        {
            if (flow_element->next_sibling->size.width >  size.width)
            {
                next_size = flow_element->next_sibling->size;
            }
        }
        
        Pos2d position = parent_position;
        position.x += flow_element->position.x;
        position.y += flow_element->position.y;
        
        Pos2d left_top = {};
        Pos2d right_top = {};
        Pos2d left_bottom = {};
        Pos2d right_bottom = {};

        // Top lane segment
        left_top = position;
        right_top = left_top;
        right_top.x += previous_size.width;
        
        left_bottom = left_top;
        left_bottom.y += flow_element->size.height / 2;
        
        right_bottom = left_bottom;
        right_bottom.x += flow_element->size.width;
        
        draw_lane_segment(left_top,  right_top, left_bottom, right_bottom, 
                          20, line_color, fill_color, line_width);
                          
        // Bottom lane segment
        
        left_top = left_bottom;
        right_top = right_bottom;
        
        left_bottom = position;
        left_bottom.y += flow_element->size.height;
        
        right_bottom = left_bottom;
        right_bottom.x += next_size.width;
        
        draw_lane_segment(left_top,  right_top, left_bottom, right_bottom, 
                          20, line_color, fill_color, line_width);
    }
    else if (flow_element->type == FlowElement_If)
    {
        Pos2d position = parent_position;
        position.x += flow_element->position.x;
        position.y += flow_element->position.y;
        
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
        
        // If-start position + size
        Pos2d position = add_position_to_position(flow_element->position, parent_position);
        Size2d size = flow_element->size;
        
        // If-then position + size (left)
        Pos2d left_position = add_position_to_position(flow_element->next_sibling->position, parent_position);
        i32 left_width = flow_element->next_sibling->size.width;
        
        // If-else position + size (right)
        Pos2d right_position = add_position_to_position(flow_element->next_sibling->next_sibling->position, parent_position);
        i32 right_width = flow_element->next_sibling->next_sibling->size.width;
        
        Pos2d top_position = {};
        i32 top_width = 0;
        
        Pos2d bottom_position = {};
        i32 bottom_width = 0;
        
        LaneSegment lane_segment = {};
        
        // TODO: maybe we should not reverse engineer middle_margin this way
        i32 middle_margin = size.width - (left_width + right_width);
        
        Pos2d split_point = position;
        split_point.x += left_width + middle_margin / 2;
        split_point.y += size.height / 2;
        
        // Top lane segment
        
        top_position = position;
        top_width = size.width;
        
        bottom_position = position;
        bottom_position.y += size.height / 2;
        bottom_width = size.width;
        
        lane_segment = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, lane_segment.left_bottom, lane_segment.right_bottom, 
                          20, line_color, fill_color, line_width);
        
        // Left lane segment

        top_position = position;
        top_position.y += size.height / 2;
        top_width = split_point.x - top_position.x;
        
        bottom_position = position;
        bottom_position.y += size.height;
        bottom_width = left_width;

        lane_segment = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, lane_segment.left_bottom, lane_segment.right_bottom, 
                          20, line_color, fill_color, line_width);
        
        // Right lane segment
        
        top_position = split_point;
        top_width = right_position.x + right_width - split_point.x;
        
        bottom_position = right_position;
        bottom_width = right_width;
        
        lane_segment = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, lane_segment.left_bottom, lane_segment.right_bottom, 
                          20, line_color, fill_color, line_width);
                          
    }
    else if (flow_element->type == FlowElement_IfEnd)
    {
        // Colors
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        
        // If-end positions + size
        Pos2d position = add_position_to_position(flow_element->position, parent_position);
        Size2d size = flow_element->size;
        
        // If-then position + size (left)
        Pos2d left_position = add_position_to_position(flow_element->previous_sibling->previous_sibling->position, parent_position);
        i32 left_width = flow_element->previous_sibling->previous_sibling->size.width;
        
        // If-else position + size (right)
        Pos2d right_position = add_position_to_position(flow_element->previous_sibling->position, parent_position);
        i32 right_width = flow_element->previous_sibling->size.width;
        
        Pos2d top_position = {};
        i32 top_width = 0;
        
        Pos2d bottom_position = {};
        i32 bottom_width = 0;
        
        LaneSegment lane_segment = {};
        
        // TODO: maybe we should not reverse engineer middle_margin this way
        i32 middle_margin = size.width - (left_width + right_width);
        
        Pos2d join_point = position;
        join_point.x += left_width + middle_margin / 2;
        join_point.y += size.height / 2;

        // Left lane segment

        top_position = position;
        top_width = left_width;
        
        bottom_position = position;
        bottom_position.y += size.height / 2;
        bottom_width = join_point.x - top_position.x;

        lane_segment = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, lane_segment.left_bottom, lane_segment.right_bottom, 
                          20, line_color, fill_color, line_width);
        
        // Right lane segment
        
        top_position = position;
        top_position.x += left_width + middle_margin;
        top_width = right_width; // right_position.x + right_width - split_point.x;
        
        bottom_position = join_point;
        bottom_width = right_position.x + right_width - join_point.x;
        
        lane_segment = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, lane_segment.left_bottom, lane_segment.right_bottom, 
                          20, line_color, fill_color, line_width);
                          
        // Bottom lane segment
        
        top_position = position;
        top_position.y += size.height / 2;
        top_width = size.width;
        
        bottom_position = position;
        bottom_position.y += size.height;
        bottom_width = size.width;
        
        lane_segment = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, lane_segment.left_bottom, lane_segment.right_bottom, 
                          20, line_color, fill_color, line_width);
        
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionCall ||
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
        
        if (flow_element->type == FlowElement_Root ||
            flow_element->type == FlowElement_FunctionCall)
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
