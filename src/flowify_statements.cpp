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
    "For",
    
    // Data flow elements
    "Assignment",
    "BinaryOperator",
    "FunctionCall",   // TODO: is this redundant?
    "Scalar"
};

struct LaneSegment
{
    Pos2d left_top;
    Pos2d right_top;
    
    Pos2d left_bottom;
    Pos2d right_bottom;
};

struct FlowElement
{
    FlowElementType type;
    
    Node * ast_node;
    
    FlowElement * first_child;
    
    FlowElement * next_sibling;
    FlowElement * previous_sibling;

    b32 is_selected;
    
    // TODO: i32 scale;
    
    // TODO: should we use this? Or something more subtle? Or add something?
    Size2d size;
    Pos2d position;
    
    Size2d absolute_size;
    Pos2d absolute_position;
    
    // TODO: what to do here?
    LaneSegment lane_segment;
    
    // TODO: do we want to add position and size here this way?
    // LaneSegment * lane_segment;
    // Rectangle * rounded_rectangle;

    // TODO: we could consist of several sub-shapes (like a if-statement consists of if-start, if-cond, if-then, if-else, if-end)
    
    // HitShape * hit_box;               or is this part of the shape(s)???
    
    HighlightedLinePart highlighted_line_part; // This is for highlighting in the dump
};

// TODO: maybe we should create a struct for each type of FlowElement?
//    Function/Root (RoundedRectangle/Container)
//    ExpressionStatement ((Sub)LaneSegment)
//    IfStatement (Head/Split/2-Lanes/Join/Tail-Segments)
//    ForStatement (...)

struct Flowifier
{
    FlowElement flow_elements[100]; // TODO: allocate this properly
    i32 nr_of_flow_elements;
};

FlowElement * new_flow_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type = FlowElement_Unknown)
{
    FlowElement * new_flow_element = &flowifier->flow_elements[flowifier->nr_of_flow_elements++];
    new_flow_element->ast_node = ast_node;
    new_flow_element->first_child = 0;
    new_flow_element->next_sibling = 0;
    new_flow_element->previous_sibling = 0;
    new_flow_element->type = flow_element_type;
    new_flow_element->is_selected = false;
    return new_flow_element;
}

FlowElement * flowify_statement(Flowifier * flowifier, Node * statement_node)
{
    FlowElement * new_child_element = 0;
    
    if (statement_node->type == Node_Stmt_Expr)
    {
        // FIXME: this is not always an FlowElement_Assignment!
        FlowElementType element_type = FlowElement_Assignment;
        if (statement_node->first_child && statement_node->first_child && statement_node->first_child->first_child)
        {
            if (statement_node->first_child->first_child->next_sibling)
            {
                if (statement_node->first_child->first_child->next_sibling->type == Node_Expr_PostInc)
                {
                    // FIXME: hack!
                    element_type = FlowElement_BinaryOperator;
                }
            }
        }
        // TODO: we should flowify the expression! (for now we create a dummy element)
        
        new_child_element = new_flow_element(flowifier, statement_node, element_type);
    }
    else if (statement_node->type == Node_Stmt_If)
    {
        // TODO: implement this
    }
    
    return new_child_element;
}

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element)
{
    Node * parent_node = parent_element->ast_node;
    
    Node * child_node = parent_node->first_child;
    FlowElement * previous_child_element = 0;
    
    if (child_node)
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
        // TODO: 
    }
    else if (flow_element->type == FlowElement_Root)
    {
        // FIXME: position shouldnt be set here!
        flow_element->position.x = 0;
        flow_element->position.y = 0;
        
        i32 top_margin = 20;
        i32 bottom_margin = 20;
        
        i32 left_margin = 20;
        i32 right_margin = 20;
        
        i32 summed_children_height = 0;
        i32 largest_child_width = 0;
        
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
    else if (flow_element->type == FlowElement_Root)
    {
        Pos2d position = parent_position;
        position.x += flow_element->position.x;
        position.y += flow_element->position.y;
        
        /*
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        */
        
        draw_rounded_rectangle(position, flow_element->size, 20, 
                               function_line_color, function_fill_color, function_line_width);
                               
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
