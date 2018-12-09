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
    
    // TODO: what to do here?
    LaneSegment line_segment;
    
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
    new_flow_element->type = flow_element_type;
    return new_flow_element;
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
            if (child_node->type == Node_Stmt_Expr)
            {
                
                // TODO: we should flowify the expression! (for now we create a dummy element)
                
                // FIXME: this is not always an FlowElement_Assignment!
                FlowElement * new_child_element = new_flow_element(flowifier, child_node, FlowElement_Assignment);
                
                if (!parent_element->first_child)
                {
                    parent_element->first_child = new_child_element;
                }
                else 
                {
                    previous_child_element->next_sibling = new_child_element;
                }
                previous_child_element = new_child_element;
            }
        }
        while((child_node = child_node->next_sibling));
    }
    
    log_int(flowifier->nr_of_flow_elements);
    
}

void layout_elements(FlowElement * flow_element)
{

    // TODO: implement this!
    
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
