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
    String source_text;
    
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
    
    b32 is_selectable;
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
    
    Parser * parser;
    
    // Colors
    Color4 line_color;
    Color4 unselected_color;
    Color4 selected_color;
    Color4 text_color;
    Color4 rectangle_color;
    Color4 rectangle_fill;
    Color4 function_line_color;
    Color4 function_fill_color;
    
    // Font
    Font font;
        
    // Layout, sizes
    i32 line_width;
    i32 bending_radius;
    i32 function_line_width;
};

void init_flowifier(Flowifier * flowifier, Parser * parser)
{
    flowifier->first_function = 0;
    flowifier->latest_function = 0;
    
    flowifier->parser = parser;
    
    flowifier->line_color       = (Color4){  0,   0,   0, 255};
    flowifier->unselected_color = (Color4){180, 180, 255, 255};
    flowifier->selected_color   = (Color4){180, 255, 180, 255};
    flowifier->text_color       = (Color4){  0,   0,   0, 255};
    flowifier->rectangle_color  = (Color4){255, 0, 0, 255};
    flowifier->rectangle_fill   = (Color4){255, 0, 0, 50};
    flowifier->function_line_color = (Color4){ 200, 200, 200, 255};
    flowifier->function_fill_color = (Color4){ 240, 240, 240, 255};
    
    flowifier->font.height = 20;
    flowifier->font.family = Font_CourierNew;
        
    flowifier->line_width = 4;
    flowifier->bending_radius = 20;
    flowifier->function_line_width = 2;
    
    init_dynamic_array(&flowifier->flow_elements, sizeof(FlowElement), (Color4){0,255,255,255}, cstring_to_string("Flowifier"));
}

FlowElement * new_flow_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type = FlowElement_Unknown)
{
    Parser * parser = flowifier->parser;
    
    FlowElement empty_flow_element = {};
    
    FlowElement * new_flow_element = (FlowElement *)add_to_array(&flowifier->flow_elements, &empty_flow_element);
    
    new_flow_element->ast_node = ast_node;
    new_flow_element->source_text = get_source_text_from_ast_node(parser, ast_node);
    
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

    new_flow_element->is_selectable = false;
    
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

