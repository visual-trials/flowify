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
    FlowElement_FunctionParameterAssignments,
    FlowElement_FunctionBody,
    
    FlowElement_If,
    FlowElement_IfKeyword,
    FlowElement_IfCond,
    FlowElement_IfThen,
    FlowElement_IfElse,
    FlowElement_IfJoin,
    
    FlowElement_For,
    FlowElement_ForKeyword,
    FlowElement_ForInit,
    FlowElement_ForCond,
    FlowElement_ForBody,
    FlowElement_ForUpdate,
    FlowElement_ForDone,
    
    FlowElement_Foreach,
    FlowElement_ForeachKeyword,
    FlowElement_ForeachInit,
    FlowElement_ForeachCond,
    FlowElement_ForeachAsKeyword,
    FlowElement_ForeachKeyVar,
    FlowElement_ForeachArrowKeyword,
    FlowElement_ForeachValueVar,
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
    FlowElement_AssignmentOperator,
    FlowElement_ArrayAccess,    // TODO: this is called Expr_ArrayDimFetch in the parser. We should probably use 'Fetch' here too
    FlowElement_UnaryPreOperation,
    FlowElement_UnaryPostOperation,
    FlowElement_UnaryOperator,
    FlowElement_BinaryOperation,
    FlowElement_BinaryOperator,
    FlowElement_FunctionCall,
    FlowElement_FunctionCallIdentifier,
    FlowElement_FunctionCallArguments,
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
    "FunctionParameterAssignments",
    "FunctionBody",
    
    "If",
    "IfKeyword",
    "IfCond",
    "IfThen",
    "IfElse",
    "IfJoin",
    
    "For",
    "ForKeyword",
    "ForInit",
    "ForCond",
    "ForBody",
    "ForUpdate",
    "ForDone",
    
    "Foreach",
    "ForeachKeyword",
    "ForeachInit",
    "ForeachCond",
    "ForeachAsKeyword",
    "ForeachKeyVar",
    "ForeachArrowKeyword",
    "ForeachValueVar",
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
    "AssignmentOperator",
    "ArrayAccess",
    "UnaryPreOperation",
    "UnaryPostOperation",
    "UnaryOperator",
    "BinaryOperation",
    "BinaryOperator",
    "FunctionCall",
    "FunctionCallIdentifier",
    "FunctionCallArguments",
    "Variable",
    "Scalar"
};

struct FlowElement
{
    i32 index;
    FlowElementType type;
    
    Node * ast_node;
    String source_text;
    
    b32 is_statement;
    
    // These 5 FlowElements represent a parental and sibling "hierarchy" (mostly used for "looping" through all elements)
    FlowElement * first_child;
    FlowElement * last_child;
    i32 nr_of_children;
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
    
    b32 is_collapsed;
    
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
    i32 acted_upon_element_index;
    i32 selected_element_index;
    i32 hovered_element_index;
};

struct FlowMargin
{
    i32 vertical;
    i32 horizontal;
};

struct Flowifier
{
    // Note: this can't be a ConsecutiveDynamicArray because we keep pointers of the elements
    FragmentedDynamicArray flow_elements;

    BasicRenderer renderer;
    LaneRenderer lane_renderer;
    
    DrawableLane * current_lane;
    
    FlowElement * first_function;
    FlowElement * latest_function;
    
    Parser * parser;
    
    b32 use_variable_color_variations;
    b32 has_absolute_positions;
    FlowInteraction interaction;
    
    // Colors and line widths
    
    Color4 text_color;
    Color4 highlighted_color;
    
    DrawStyle lane_style;
    DrawStyle help_rectangle_style;
    DrawStyle hovered_style;
    DrawStyle selected_style;
    DrawStyle function_style;
    DrawStyle scalar_style;
    DrawStyleEvenOdd expression_style;
    DrawStyle variable_style;
    DrawStyle variable_styles[20]; // FIXME: make sure nr_of_variable_colors is always smaller than (or equal to) the size of this array!
    i32 nr_of_variable_colors;
    DrawStyle detail_style;
    
    b32 show_help_rectangles;
    
    // Font
    Font font;
    i32 character_width;
    i32 character_height;
    u8 ascii_characters[256]; // TODO: this is now used to create Strings for "source_text" using only 1 character
        
    // Layout, sizes
    
    FlowMargin expression_margin;
    FlowMargin variable_margin;
    FlowMargin statement_margin;
    
    i32 if_middle_margin;
    i32 for_middle_margin;
    i32 for_right_margin;
    
    i32 default_element_width;
    i32 default_element_height;
};

FlowElement * new_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type);
FlowElement * new_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type, b32 is_statement);

Color4 lighten_color(Color4 dark_color, i32 factor)
{
    Color4 light_color;
    
    light_color.r = 255 - (255 - dark_color.r) / factor;
    light_color.g = 255 - (255 - dark_color.g) / factor;
    light_color.b = 255 - (255 - dark_color.b) / factor;
    light_color.a = dark_color.a;
    
    return light_color;
}

void init_flowifier(Flowifier * flowifier, Parser * parser)
{
    // TODO: right now we store functions in one linked list (maybe use an array/hashmap)
    flowifier->first_function = 0;
    flowifier->latest_function = 0;
    
    init_basic_renderer(&flowifier->renderer);
    init_lane_renderer(&flowifier->lane_renderer);
    
    // TODO: do we want to keep this one? Or should it be in the renderer/lane_renderer? (probably keep them in Flowifier)
    flowifier->current_lane = 0;
    
    flowifier->parser = parser;
    
    flowifier->interaction.highlighted_element_index = 0;
    flowifier->interaction.selected_element_index = 0;
    flowifier->interaction.acted_upon_element_index = 0;
    flowifier->interaction.hovered_element_index = 0;
    
    flowifier->use_variable_color_variations = true;
    flowifier->has_absolute_positions = false;
    
    flowifier->text_color = (Color4){  0,   0,   0, 255};
    
    flowifier->highlighted_color   = (Color4){180, 255, 180, 255};
    
    flowifier->lane_style.line_color = (Color4){  0,   0,   0, 255};
    flowifier->lane_style.fill_color = (Color4){180, 180, 255, 255};
    flowifier->lane_style.corner_radius = 16;
    flowifier->lane_style.line_width = 2;
    
    flowifier->show_help_rectangles = false;
    flowifier->help_rectangle_style.line_color = (Color4){255, 0, 0, 255};
    flowifier->help_rectangle_style.fill_color = (Color4){255, 0, 0, 20};
    flowifier->help_rectangle_style.line_width = 2;
    
    // FIXME: set fill/stroke to 0 alpha for now: this alpha seems to cause significant performance drains (when its a very large rect)
    flowifier->hovered_style.line_color = (Color4){0, 0, 255, 255};
    flowifier->hovered_style.fill_color = (Color4){0, 0, 255, 0}; 
    flowifier->hovered_style.line_width = 2;
    
    // FIXME: set fill/stroke to 0 alpha for now: this alpha seems to cause significant performance drains (when its a very large rect)
    flowifier->selected_style.line_color = (Color4){255, 0, 255, 255};
    flowifier->selected_style.fill_color = (Color4){255, 0, 255, 0};
    flowifier->selected_style.line_width = 2;
    
    flowifier->function_style.line_color = (Color4){ 200, 200, 200, 255};
    flowifier->function_style.fill_color = (Color4){ 255, 255, 255, 255};
    flowifier->function_style.line_width = 2;
    flowifier->function_style.corner_radius = 10;
    
    flowifier->expression_style.line_color = (Color4){ 200, 255, 200, 255};
    flowifier->expression_style.even_fill_color = (Color4){ 235, 255, 235, 255};
    flowifier->expression_style.odd_fill_color  = (Color4){ 245, 255, 245, 255};
    flowifier->expression_style.corner_radius = 10;
    flowifier->expression_style.line_width = 2;
    
    flowifier->variable_style.line_color = (Color4){ 255, 200, 200, 255};
    flowifier->variable_style.fill_color = (Color4){ 255, 235, 235, 255};
    flowifier->variable_style.corner_radius = 10;
    flowifier->variable_style.line_width = 2;
    
    Color4 red = {230, 25, 75, 255};
    Color4 orange = {245, 130, 48, 255};
    Color4 yellow = {255, 225, 25, 255};
    Color4 lime = {210, 245, 60, 255};
    Color4 green = {60, 180, 75, 255};
    Color4 cyan = {70, 240, 240, 255};
    Color4 blue = {0, 130, 200, 255};
    Color4 purple = {145, 30, 180, 255};
    Color4 magenta = {240, 50, 230, 255};
    
    i32 nr_of_colors = 8;
    flowifier->nr_of_variable_colors = nr_of_colors;
    // FIXME: make sure nr_of_colors is smaller than this array!
    for (i32 variable_style_index = 0; variable_style_index < nr_of_colors; variable_style_index++)
    {
        Color4 light_color = { 255, 255, 255, 255 };
        Color4 dark_color = { 200, 200, 200, 255 };
        
        flowifier->variable_styles[variable_style_index] = flowifier->variable_style;
        
        if (variable_style_index == 0)
        {
            dark_color = lighten_color(orange, 2);
            light_color = lighten_color(orange, 4);
        }
        else if (variable_style_index == 1)
        {
            dark_color = lighten_color(yellow, 2);
            light_color = lighten_color(yellow, 4);
        }
        else if (variable_style_index == 2)
        {
            dark_color = lighten_color(lime, 2);
            light_color = lighten_color(lime, 4);
        }
        else if (variable_style_index == 3)
        {
            dark_color = lighten_color(green, 2);
            light_color = lighten_color(green, 4);
        }
        else if (variable_style_index == 4)
        {
            dark_color = lighten_color(cyan, 2);
            light_color = lighten_color(cyan, 4);
        }
        else if (variable_style_index == 5)
        {
            dark_color = lighten_color(blue, 2);
            light_color = lighten_color(blue, 4);
        }
        else if (variable_style_index == 6)
        {
            dark_color = lighten_color(purple, 2);
            light_color = lighten_color(purple, 4);
        }
        else if (variable_style_index == 7)
        {
            dark_color = lighten_color(magenta, 2);
            light_color = lighten_color(magenta, 4);
        }
        
        flowifier->variable_styles[variable_style_index].fill_color = light_color;
        flowifier->variable_styles[variable_style_index].line_color = dark_color;
    }
    
    flowifier->scalar_style.line_color = lighten_color(red, 2); // (Color4){ 255, 150, 150, 255};
    flowifier->scalar_style.fill_color = lighten_color(red, 4); //(Color4){ 255, 200, 200, 255};
    flowifier->scalar_style.corner_radius = 10;
    flowifier->scalar_style.line_width = 2;
    
    flowifier->detail_style.line_color = (Color4){ 200, 200, 200, 255};
    flowifier->detail_style.fill_color = (Color4){ 255, 255, 255, 200};
    flowifier->detail_style.line_width = 2;
    
    flowifier->font.height = 18;
    flowifier->font.family = Font_CourierNew;
    
    ShortString white_space;
    copy_char_to_string(' ', &white_space);
    Size2d white_space_size = get_text_size(&white_space, flowifier->font);
    flowifier->character_width = white_space_size.width;
    flowifier->character_height = white_space_size.height;
    for (i32 ascii_character_index = 0; ascii_character_index < 256; ascii_character_index++)
    {
        flowifier->ascii_characters[ascii_character_index] = (u8)ascii_character_index;
    }
    
    flowifier->expression_margin.vertical = 10;
    flowifier->expression_margin.horizontal = 14;
    
    flowifier->variable_margin.vertical = 10;
    flowifier->variable_margin.horizontal = 14;
    
    flowifier->statement_margin.vertical = 20;
    flowifier->statement_margin.horizontal = 20;
    
    flowifier->if_middle_margin = 80;
    flowifier->for_middle_margin = 80;
    flowifier->for_right_margin = 100;
    
    flowifier->default_element_width = 100;
    flowifier->default_element_height = 64;
    
    init_fragmented_dynamic_array(&flowifier->flow_elements, sizeof(FlowElement), (Color4){0,255,255,255}, cstring_to_string("Flowifier"));
    
    // Note: we start elements with index = 1 (by creating a dummy here). That way we can use index 0 as being no-index.
    FlowElement * dummy_element = new_element(flowifier, 0, FlowElement_Unknown);
}

FlowElement * new_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type)
{
    return new_element(flowifier, ast_node, flow_element_type, false);
}

FlowElement * new_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type, b32 is_statement)
{
    Parser * parser = flowifier->parser;
    
    FlowElement empty_flow_element = {};
    
    i32 index = flowifier->flow_elements.nr_of_index_entries;

    FlowElement * new_flow_element = (FlowElement *)add_to_indexed_array(&flowifier->flow_elements, &empty_flow_element);
    
    new_flow_element->index = index;
    new_flow_element->type = flow_element_type;
    
    new_flow_element->ast_node = ast_node;
    // FIXME: for the root element, the source_text has an extra character at the end (which doesn't look right)
    new_flow_element->source_text = get_source_text_from_ast_node(parser, ast_node);
    
    new_flow_element->is_statement = is_statement;
    
    new_flow_element->first_child = 0;
    new_flow_element->last_child = 0;
    new_flow_element->nr_of_children = 0;
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

    new_flow_element->is_collapsed = false;
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
    parent_element->nr_of_children++;
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
    
    /*
    append_integer_detail("abs.x", element->rect_abs.position.x, detail_text);
    append_integer_detail("abs.y", element->rect_abs.position.y, detail_text);
    append_newline(detail_text);
    */
    
    append_boolean_detail("is_statement", element->is_statement, detail_text);
    append_boolean_detail("is_collapsed", element->is_collapsed, detail_text);
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

