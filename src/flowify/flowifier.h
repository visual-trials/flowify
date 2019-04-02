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
    FlowElement_IfKeyword,
    FlowElement_IfCond,
    FlowElement_IfSplit,
    FlowElement_IfThen,
    FlowElement_IfElse,
    FlowElement_IfJoin,
    
    FlowElement_For,
    FlowElement_ForKeyword,
    FlowElement_ForInit,
    FlowElement_ForJoin,
    FlowElement_ForCond,
    FlowElement_ForSplit,
    FlowElement_ForBody,
    FlowElement_ForUpdate,
    FlowElement_ForDone,
    
    FlowElement_Foreach,
    FlowElement_ForeachKeyword,
    FlowElement_ForeachInit,
    FlowElement_ForeachJoin,
    FlowElement_ForeachCond,
    FlowElement_ForeachArray,
    FlowElement_ForeachAsKeyword,
    FlowElement_ForeachKeyVar,
    FlowElement_ForeachArrowKeyword,
    FlowElement_ForeachValueVar,
    FlowElement_ForeachSplit,
    FlowElement_ForeachBody,
    FlowElement_ForeachDone,
    
    FlowElement_PassThrough,
    FlowElement_PassBack,
    
    FlowElement_Return,
    FlowElement_ReturnKeyword,
    FlowElement_Break,
    FlowElement_Continue,
    
    // Data flow elements
    FlowElement_Hidden,         // TODO: using this to signify the contents of a function we don't know it's implementation of
    FlowElement_Assignment,
    FlowElement_Assignee,
    FlowElement_AssignmentOperator,
    FlowElement_UnaryOperator,
    FlowElement_BinaryOperation,
    FlowElement_BinaryOperator,
    FlowElement_FunctionCall,   // TODO: is this redundant?
    FlowElement_Variable,
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
    "IfKeyword",
    "IfCond",
    "IfSplit",
    "IfThen",
    "IfElse",
    "IfJoin",
    
    "For",
    "ForKeyword",
    "ForStart",
    "ForInit",
    "ForJoin",
    "ForCond",
    "ForSplit",
    "ForBody",
    "ForUpdate",
    "ForDone",
    
    "Foreach",
    "ForeachKeyword",
    "ForeachStart",
    "ForeachJoin",
    "ForeachCond",
    "ForeachArray",
    "ForeachAsKeyword",
    "ForeachKeyVar",
    "ForeachArrowKeyword",
    "ForeachValueVar",
    "ForeachSplit",
    "ForeachBody",
    "ForeachDone",
    
    "PassThrough",
    "PassBack",
    
    "Return",
    "ReturnKeyword",
    "Break",
    "Continue",
    
    // Data flow elements
    "Hidden",
    "Assignment",
    "Assignee",
    "AssignmentOperator",
    "UnaryOperator",
    "BinaryOperation",
    "BinaryOperator",
    "FunctionCall",   // TODO: is this redundant?
    "Variable",
    "Scalar"
};

struct FlowElement
{
    i32 index;
    FlowElementType type;
    
    Node * ast_node;
    String source_text;
    
    // These 5 FlowElements represent a parental and sibling "hierarchy" (mostly used for "looping" through all elements)
    FlowElement * first_child;
    FlowElement * last_child;
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
    b32 is_highlightable;
    
    Rect2d rect;
    Rect2d rect_abs;
    
    // TODO: i32 scale;
    
    // TODO: Pos2d left_top_absolute_position;
    // TODO: Pos2d bottom_right_absolute_position;
    // TODO: i32 absolute_scale;
    
    // HitShape * hit_box;               or is this part of the shape(s)???
    
    HighlightedLinePart highlighted_line_part; // This is for highlighting in the dump
};

struct FlowInteraction
{
    i32 highlighted_element_index;
    i32 selected_element_index;
    i32 hovered_element_index;
};

struct FlowMargin
{
    i32 vertical;
    i32 horizontal;
};

struct FlowStyle
{
    Color4 line_color;
    Color4 fill_color;
    i32 corner_radius;
    i32 line_width;
};

struct FlowStyleEvenOdd
{
    Color4 line_color;
    Color4 even_fill_color;
    Color4 odd_fill_color;
    i32 corner_radius;
    i32 line_width;
};

struct Flowifier
{
    DynamicArray flow_elements;
    
    FlowElement * first_function;
    FlowElement * latest_function;
    
    Parser * parser;
    
    b32 has_absolute_positions;
    FlowInteraction interaction;
    
    // Colors and line widths
    
    Color4 text_color;
    
    // FIXME: give these a better name: statement_line_color and statement_fill_color, statement_line_width
    Color4 line_color;
    // FIXME: isnt the unhighlighted_color == statement_fill_color?
    Color4 unhighlighted_color;
    i32 statement_corner_radius;
    i32 line_width;
    
    // TODO: should we add highlighted_color to the Style struct? or is the highlighted_color the same for alle types of elements?
    Color4 highlighted_color;
    
    b32 show_help_rectangles;
    
    // TODO: make a struct for 2 colors, 1 line width!
    Color4 help_rectangle_color;
    Color4 help_rectangle_fill;
    i32 help_rectangle_line_width;
    
    // TODO: make a struct for 2 colors, 1 line width!
    Color4 hovered_color;
    Color4 hovered_fill;
    i32 hovered_line_width;
    
    // TODO: make a struct for 2 colors, 1 line width!
    Color4 selected_color;
    Color4 selected_fill;
    i32 selected_line_width;
    
    // TODO: make a struct for 3 colors, 1 line width!
    Color4 function_line_color;
    Color4 function_even_fill_color;
    Color4 function_odd_fill_color;
    i32 function_line_width;
    
    FlowStyleEvenOdd expression_style;
    
    FlowStyle variable_style;
    
    FlowStyle scalar_style;
    
    // TODO: make a struct for 2 colors, 1 line width!
    Color4 detail_line_color;
    Color4 detail_fill_color;
    i32 detail_line_width;
    
    // Font
    Font font;
    i32 character_width;
    i32 character_height;
        
    // Layout, sizes
    
    FlowMargin expression_margin;
    
    FlowMargin variable_margin;
    
    FlowMargin statement_margin;
    
    i32 bending_radius;
    i32 if_middle_margin;
    i32 for_middle_margin;
    i32 for_right_margin;
    
    i32 default_element_width;
    i32 default_element_height;
};

FlowElement * new_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type);

void init_flowifier(Flowifier * flowifier, Parser * parser)
{
    flowifier->first_function = 0;
    flowifier->latest_function = 0;
    
    flowifier->parser = parser;
    
    flowifier->interaction.highlighted_element_index = 0;
    flowifier->interaction.selected_element_index = 0;
    flowifier->interaction.hovered_element_index = 0;
    
    flowifier->has_absolute_positions = false;
    
    flowifier->line_color          = (Color4){  0,   0,   0, 255};
    flowifier->text_color          = (Color4){  0,   0,   0, 255};
    flowifier->statement_corner_radius = 20;
    flowifier->line_width = 4;
    
    // FIXME: isnt the unhighlighted_color == statement_fill_color?
    flowifier->unhighlighted_color = (Color4){180, 180, 255, 255};
    flowifier->highlighted_color   = (Color4){180, 255, 180, 255};
    
    flowifier->show_help_rectangles = false;
    flowifier->help_rectangle_color     = (Color4){255, 0, 0, 255};
    flowifier->help_rectangle_fill      = (Color4){255, 0, 0, 50};
    flowifier->help_rectangle_line_width = 2;
    
    flowifier->hovered_color     = (Color4){0, 0, 255, 30};
    flowifier->hovered_fill      = (Color4){0, 0, 255, 10};
    flowifier->hovered_line_width = 2;
    
    flowifier->selected_color     = (Color4){255, 0, 255, 255};
    flowifier->selected_fill      = (Color4){255, 0, 255, 30};
    flowifier->selected_line_width = 2;
    
    flowifier->function_line_color = (Color4){ 200, 200, 200, 255};
    flowifier->function_even_fill_color = (Color4){ 235, 235, 235, 100}; // FIXME: remove alpha
    flowifier->function_odd_fill_color  = (Color4){ 245, 245, 245, 100}; // FIXME: remove alpha
    flowifier->function_line_width = 2;
    
    flowifier->expression_style.line_color = (Color4){ 200, 255, 200, 255};
    flowifier->expression_style.even_fill_color = (Color4){ 235, 255, 235, 255};
    flowifier->expression_style.odd_fill_color  = (Color4){ 245, 255, 245, 255};
    flowifier->expression_style.corner_radius = 10;
    flowifier->expression_style.line_width = 2;
    
    flowifier->variable_style.line_color = (Color4){ 255, 200, 200, 255};
    flowifier->variable_style.fill_color = (Color4){ 255, 235, 235, 255};
    flowifier->variable_style.corner_radius = 10;
    flowifier->variable_style.line_width = 2;
    
    flowifier->scalar_style.line_color = (Color4){ 255, 150, 150, 255};
    flowifier->scalar_style.fill_color = (Color4){ 255, 200, 200, 255};
    flowifier->scalar_style.corner_radius = 10;
    flowifier->scalar_style.line_width = 2;
    
    
    flowifier->detail_line_color = (Color4){ 200, 200, 200, 255};
    flowifier->detail_fill_color = (Color4){ 255, 255, 255, 200};
    flowifier->detail_line_width = 2;
    
    flowifier->font.height = 20;
    flowifier->font.family = Font_CourierNew;
    
    ShortString white_space;
    copy_char_to_string(' ', &white_space);
    Size2d white_space_size = get_text_size(&white_space, flowifier->font);
    flowifier->character_width = white_space_size.width;
    flowifier->character_height = white_space_size.height;
    
    flowifier->expression_margin.vertical = 10;
    flowifier->expression_margin.horizontal = 14;
    
    flowifier->variable_margin.vertical = 10;
    flowifier->variable_margin.horizontal = 14;
    
    flowifier->statement_margin.vertical = 20;
    flowifier->statement_margin.horizontal = 20;
    
    flowifier->bending_radius = 20;
    flowifier->if_middle_margin = 80;
    flowifier->for_middle_margin = 80;
    flowifier->for_right_margin = 100;
    
    flowifier->default_element_width = 100;
    flowifier->default_element_height = 80;
    
    init_dynamic_array(&flowifier->flow_elements, sizeof(FlowElement), (Color4){0,255,255,255}, cstring_to_string("Flowifier"));
    
    // Note: we start elements with index = 1 (by creating a dummy here). That way we can use index 0 as being no-index.
    FlowElement * dummy_element = new_element(flowifier, 0, FlowElement_Unknown);
}

FlowElement * new_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type)
{
    Parser * parser = flowifier->parser;
    
    FlowElement empty_flow_element = {};
    
    i32 index = flowifier->flow_elements.nr_of_items;

    FlowElement * new_flow_element = (FlowElement *)add_to_array(&flowifier->flow_elements, &empty_flow_element);
    
    new_flow_element->index = index;
    new_flow_element->type = flow_element_type;
    
    new_flow_element->ast_node = ast_node;
    // FIXME: for the root element, the source_text has an extra character at the end (which doesn't look right)
    new_flow_element->source_text = get_source_text_from_ast_node(parser, ast_node);
    
    new_flow_element->first_child = 0;
    new_flow_element->last_child = 0;
    new_flow_element->parent = 0;
    new_flow_element->next_sibling = 0;
    new_flow_element->previous_sibling = 0;
    
    new_flow_element->next_in_flow = 0;
    new_flow_element->previous_in_flow = 0;
    new_flow_element->first_in_flow = 0;
    new_flow_element->last_in_flow = 0;
    
    new_flow_element->next_function = 0;
    
    new_flow_element->rect.position.x = 0;
    new_flow_element->rect.position.y = 0;
    
    new_flow_element->rect.size.width = 0;
    new_flow_element->rect.size.height = 0;
    
    new_flow_element->rect_abs.position.x = 0;
    new_flow_element->rect_abs.position.y = 0;
    
    new_flow_element->rect_abs.size.width = 0;
    new_flow_element->rect_abs.size.height = 0;

    new_flow_element->is_selectable = true;
    new_flow_element->is_highlightable = false;
    
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

void add_function_element(Flowifier * flowifier, FlowElement * function_element)
{
    if (!flowifier->first_function)
    {
        flowifier->first_function = function_element;
    }
    else 
    {
        flowifier->latest_function->next_function = function_element;
    }
    flowifier->latest_function = function_element;
}

void add_child_element(FlowElement * child_element, FlowElement * parent_element)
{
    if (!parent_element->first_child)
    {
        parent_element->first_child = child_element;
        child_element->parent = parent_element;
    }
    else
    {
        parent_element->last_child->next_sibling = child_element;
        child_element->previous_sibling = parent_element->last_child;
        child_element->parent = parent_element;
    }
    parent_element->last_child = child_element;
}

void append_cstring_detail(const char * title, const char * value, DynamicString * detail_text, b32 append_newline = true)
{
    append_string(detail_text, cstring_to_string(title));
    append_string(detail_text, cstring_to_string(": "));
    String value_string = cstring_to_string(value);
    append_string(detail_text, value_string);
    
    if (append_newline)
    {
        append_string(detail_text, cstring_to_string("\n"));
    }
}

void append_string_detail(const char * title, String value, DynamicString * detail_text, b32 append_newline = true)
{
    append_string(detail_text, cstring_to_string(title));
    append_string(detail_text, cstring_to_string(": "));
    append_string(detail_text, value);
    
    if (append_newline)
    {
        append_string(detail_text, cstring_to_string("\n"));
    }
}

void append_integer_detail(const char * title, i32 int_value, DynamicString * detail_text, b32 append_newline = true)
{
    ShortString decimal_string = {}; // TODO: this is hurting the stack a bit. Maybe pass it instead?
    
    append_string(detail_text, cstring_to_string(title));
    append_string(detail_text, cstring_to_string(": "));
    append_string(detail_text, int_to_string(int_value, &decimal_string));
    
    if (append_newline)
    {
        append_string(detail_text, cstring_to_string("\n"));
    }
}

void append_element_pointer_detail(const char * title, FlowElement * flow_element, DynamicString * detail_text, b32 append_newline = true)
{
    ShortString decimal_string = {}; // TODO: this is hurting the stack a bit. Maybe pass it instead?
    
    i32 int_value = 0;
    if (flow_element)
    {
        int_value = flow_element->index;
    }
    append_string(detail_text, cstring_to_string(title));
    append_string(detail_text, cstring_to_string(": "));
    append_string(detail_text, int_to_string(int_value, &decimal_string));
    
    if (int_value)
    {
        append_string(detail_text, cstring_to_string(" ("));
        append_string(detail_text, cstring_to_string(flow_element_type_names[flow_element->type]));
        append_string(detail_text, cstring_to_string(")"));
    }
    
    if (append_newline)
    {
        append_string(detail_text, cstring_to_string("\n"));
    }
}

void append_boolean_detail(const char * title, b32 bool_value, DynamicString * detail_text, b32 append_newline = true)
{
    append_string(detail_text, cstring_to_string(title));
    append_string(detail_text, cstring_to_string(": "));
    if (bool_value)
    {
        append_string(detail_text, cstring_to_string("true"));
    }
    else
    {
        append_string(detail_text, cstring_to_string("false"));
    }
    if (append_newline)
    {
        append_string(detail_text, cstring_to_string("\n"));
    }
    
}

void append_newline(DynamicString * detail_text)
{
    append_string(detail_text, cstring_to_string("\n"));
}

void generate_element_detail(FlowElement * element, DynamicString * detail_text)
{
    append_integer_detail("index", element->index, detail_text);
    append_cstring_detail("type", flow_element_type_names[element->type], detail_text);
    // TODO: we only want to see the first line (and/or be able to hover over this): append_string_detail("source", element->source_text, detail_text);
    append_newline(detail_text);
    
    append_integer_detail("x", element->rect.position.x, detail_text);
    append_integer_detail("y", element->rect.position.y, detail_text);
    append_newline(detail_text);
    
    append_integer_detail("width", element->rect.size.width, detail_text);
    append_integer_detail("height", element->rect.size.height, detail_text);
    append_newline(detail_text);
    
    append_integer_detail("abs.x", element->rect_abs.position.x, detail_text);
    append_integer_detail("abs.y", element->rect_abs.position.y, detail_text);
    append_newline(detail_text);
    
    append_boolean_detail("is_selectable", element->is_selectable, detail_text);
    append_boolean_detail("is_highlightable", element->is_highlightable, detail_text);
    append_newline(detail_text);

    append_element_pointer_detail("parent", element->parent, detail_text);
    append_element_pointer_detail("first_child", element->first_child, detail_text);
    append_newline(detail_text);
    
    append_element_pointer_detail("next_sibling", element->next_sibling, detail_text);
    append_element_pointer_detail("previous_sibling", element->previous_sibling, detail_text);
    append_newline(detail_text);
    
    append_element_pointer_detail("next_in_flow", element->next_in_flow, detail_text);
    append_element_pointer_detail("previous_in_flow", element->previous_in_flow, detail_text);
    append_newline(detail_text);
    
    append_element_pointer_detail("first_in_flow", element->first_in_flow, detail_text);
    append_element_pointer_detail("last_in_flow", element->last_in_flow, detail_text, false);
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

