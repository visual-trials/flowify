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

FlowStyle get_style_by_oddness(FlowStyleEvenOdd style_even_odd, b32 is_odd)
{
    FlowStyle style = {};
    style.line_color = style_even_odd.line_color;
    style.corner_radius = style_even_odd.corner_radius;
    style.line_width = style_even_odd.line_width;
    
    if (is_odd)
    {
        style.fill_color = style_even_odd.odd_fill_color;
    }
    else
    {
        style.fill_color = style_even_odd.even_fill_color;
    }
    
    return style;
}

// Drawable entries

void add_child_draw_entry(DrawableEntry * child_draw_entry, DrawableEntry * parent_draw_entry)
{
    if (!parent_draw_entry->first_child_entry)
    {
        parent_draw_entry->first_child_entry = child_draw_entry;
    }
    else
    {
        parent_draw_entry->last_child_entry->next_entry = child_draw_entry;
    }
    parent_draw_entry->last_child_entry = child_draw_entry;
}

void add_draw_entry(Flowifier * flowifier, DrawableEntry * draw_entry)
{
    // TODO: right now, if we want a lane entry to be added to the regular list of draw entries,
    //       we have to set flowifier->last_lane_entry to 0 before calling this function
    if (draw_entry->type == Drawable_Lane)
    {
        if (flowifier->last_lane_entry)
        {
            // We just want to extend the last_lane_entry, we don't want to add an entry in the regular list of entries
            add_child_draw_entry(draw_entry, flowifier->last_lane_entry);
            return;
        }
        else
        {
            flowifier->last_lane_entry = draw_entry;
        }
    }
    
    if (!flowifier->first_draw_entry)
    {
        flowifier->first_draw_entry = draw_entry;
    }
    else
    {
        flowifier->last_draw_entry->next_entry = draw_entry;
    }
    
    flowifier->last_draw_entry = draw_entry;
}

void push_text(Flowifier * flowifier, Pos2d position, String * text, Font font, Color4 color)
{
    DrawableEntry * draw_entry = (DrawableEntry *)push_struct(&flowifier->draw_arena, sizeof(DrawableEntry));
    draw_entry->type = Drawable_Text;
    draw_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableText * text_to_draw = (DrawableText *)push_struct(&flowifier->draw_arena, sizeof(DrawableText));
    draw_entry->item_to_draw = text_to_draw;
    
    text_to_draw->position = position;
    text_to_draw->text = text;
    text_to_draw->font = font;
    text_to_draw->color = color;
    
    add_draw_entry(flowifier, draw_entry);
}

void push_rectangle(Flowifier * flowifier, Rect2d rect, Color4 line_color, Color4 fill_color, i32 line_width)
{
    DrawableEntry * draw_entry = (DrawableEntry *)push_struct(&flowifier->draw_arena, sizeof(DrawableEntry));
    draw_entry->type = Drawable_Rect;
    draw_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableRect * draw_rect = (DrawableRect *)push_struct(&flowifier->draw_arena, sizeof(DrawableRect));
    draw_entry->item_to_draw = draw_rect;
    
    draw_rect->rect = rect;
    draw_rect->line_color = line_color;
    draw_rect->fill_color = fill_color;
    draw_rect->line_width = line_width;
    
    add_draw_entry(flowifier, draw_entry);
}

void push_rounded_rectangle(Flowifier * flowifier, Rect2d rect, i32 radius, Color4 line_color, Color4 fill_color, i32 line_width)
{
    DrawableEntry * draw_entry = (DrawableEntry *)push_struct(&flowifier->draw_arena, sizeof(DrawableEntry));
    draw_entry->type = Drawable_RoundedRect;
    draw_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableRoundedRect * draw_rounded_rect = (DrawableRoundedRect *)push_struct(&flowifier->draw_arena, sizeof(DrawableRoundedRect));
    draw_entry->item_to_draw = draw_rounded_rect;
    
    draw_rounded_rect->rect = rect;
    draw_rounded_rect->radius = radius;
    draw_rounded_rect->line_color = line_color;
    draw_rounded_rect->fill_color = fill_color;
    draw_rounded_rect->line_width = line_width;
    
    add_draw_entry(flowifier, draw_entry);
}

DrawableLane * push_lane(Flowifier * flowifier)
{
    DrawableEntry * draw_entry = (DrawableEntry *)push_struct(&flowifier->draw_arena, sizeof(DrawableEntry));
    draw_entry->type = Drawable_Lane;
    draw_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableLane * draw_lane = (DrawableLane *)push_struct(&flowifier->draw_arena, sizeof(DrawableLane));
    draw_entry->item_to_draw = draw_lane;
    
    // TODO: we should let push_struct reset the memory of the struct!
    draw_lane->bounding_rect = (Rect2d){};
    
    draw_lane->first_part = 0;
    draw_lane->last_part = 0;
    
    draw_lane->splitting_from_lane = 0;
    draw_lane->is_right_side_at_split = false;
    draw_lane->splitting_point = (Pos2d){};
    
    draw_lane->is_splitter_at_end = false;
    
    draw_lane->joining_towards_lane = 0;
    draw_lane->is_right_side_at_join = false;
    draw_lane->joining_point = (Pos2d){};
    
    draw_lane->is_joiner_at_beginning = false;
    
    draw_lane->bending_radius = flowifier->bending_radius;
    draw_lane->line_color = flowifier->line_color;
    draw_lane->fill_color = flowifier->unhighlighted_color;
    draw_lane->line_width = flowifier->line_width;
        
    add_draw_entry(flowifier, draw_entry);
    
    return draw_lane;
}

// TODO: we might want to add fill_color as an argument here
void push_lane_part_to_current_lane(Flowifier * flowifier, Rect2d rect, Direction direction)
{
    assert(flowifier->current_lane);
    
    DrawableLane * current_lane = flowifier->current_lane;
    
    DrawableLanePart * draw_lane_part = (DrawableLanePart *)push_struct(&flowifier->draw_arena, sizeof(DrawableLanePart));
    draw_lane_part->rect = rect;
    draw_lane_part->next_part = 0; // TODO: we should let push_struct reset the memory of the struct!
    draw_lane_part->direction = direction;
    
    if (!current_lane->first_part)
    {
        current_lane->first_part = draw_lane_part;
    }
    else
    {
        current_lane->last_part->next_part = draw_lane_part;
    }
    current_lane->last_part = draw_lane_part;
}

void push_interaction_rectangle(Flowifier * flowifier, FlowElement * flow_element)
{
    if (flowifier->interaction.hovered_element_index == flow_element->index)
    {
        push_rectangle(flowifier, flow_element->rect_abs, flowifier->hovered_color, flowifier->hovered_fill, flowifier->hovered_line_width);
    }
    /*
    // FIXME: disabled for now
    if (flowifier->interaction.selected_element_index == flow_element->index)
    {
        push_rectangle(flowifier, flow_element->rect_abs, flowifier->selected_color, flowifier->selected_fill, flowifier->selected_line_width);
    }
    */
    /*
    // FIXME: disabled for now
    if (flowifier->show_help_rectangles)
    {
        push_rectangle(flowifier, flow_element->rect_abs, flowifier->help_rectangle_color, flowifier->help_rectangle_fill, flowifier->help_rectangle_line_width);
    }
    */
}

void push_rectangle_element(Flowifier * flowifier, FlowElement * flow_element, FlowStyle style, b32 draw_rectangle, b32 draw_source_text)
{
    
    Color4 fill_color = style.fill_color;
    if (flowifier->interaction.highlighted_element_index == flow_element->index)
    {
        fill_color = flowifier->highlighted_color;
    }
    
    if (draw_rectangle)
    {
        push_rounded_rectangle(flowifier, flow_element->rect_abs, style.corner_radius, style.line_color, fill_color, style.line_width);
    }
    
    if (draw_source_text && flow_element->source_text.length)
    {
        // FIXME: combine this with get_size_based_on_source_text() in layout.c!
        Size2d source_text_size = {};
        source_text_size.height = flowifier->character_height;
        // FIXME: hack: eulating 'kerning' here: space between characters
        source_text_size.width = flow_element->source_text.length * (flowifier->character_width + 1) - 1;
        /*
        // FIXME: this is slow!!
        Size2d source_text_size = get_text_size(&flow_element->source_text, flowifier->font);
        */
        
        // TODO: create a function: draw_text_in_rect()
        Pos2d text_position = {};
        text_position = flow_element->rect_abs.position;
        text_position.x += (flow_element->rect_abs.size.width - source_text_size.width) / 2;
        text_position.y += (flow_element->rect_abs.size.height - source_text_size.height) / 2;
        
        push_text(flowifier, text_position, &flow_element->source_text, flowifier->font, flowifier->text_color);
    }
    
    push_interaction_rectangle(flowifier, flow_element);
}

// TODO: maybe use a different name for this function?
void push_straight_element(Flowifier * flowifier, FlowElement * flow_element)
{
    push_lane_part_to_current_lane(flowifier, flow_element->rect_abs, Direction_TopToBottom);
    push_interaction_rectangle(flowifier, flow_element);
}

void draw_entries(DrawableEntry * draw_entry);

// TODO: rename this to push_elements_to_be_draw (or something) and add root-funtion: draw_elements
void draw_elements(Flowifier * flowifier, FlowElement * flow_element)
{
    assert(flow_element);
    
    if (flow_element->type == FlowElement_Root)
    {
        assert(flowifier->draw_arena.memory);
        
        reset_fragmented_memory_arena(&flowifier->draw_arena, true);
        flowifier->first_draw_entry = 0;
        flowifier->last_draw_entry = 0;
        flowifier->last_lane_entry = 0;
        flowifier->current_lane = 0;
    }
    
    // TODO: add is_position_of and position_originates_from

    Rect2d no_rect = {-1,-1,-1,-1};
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_Hidden)
    {
        push_straight_element(flowifier, flow_element);
    }
    else if (flow_element->type == FlowElement_Variable ||
             flow_element->type == FlowElement_ForeachKeyVar ||
             flow_element->type == FlowElement_ForeachValueVar ||
             flow_element->type == FlowElement_UnaryOperator)
    {
        // FIXME: we should put bars on the sides of a UnaryExpression IF its a statement!!
        // FIXME: we should put bars on the sides of a UnaryExpression IF its a statement!!
        // FIXME: we should put bars on the sides of a UnaryExpression IF its a statement!!
        
        
        FlowStyle variable_style = flowifier->variable_style;
        
        if (flowifier->use_variable_color_variations)
        {
            i32 variable_index = get_number_based_on_string(flow_element->ast_node->identifier);
            
            i32 variable_style_index = variable_index % flowifier->nr_of_variable_colors;
            variable_style = flowifier->variable_styles[variable_style_index];
        }
        
        push_rectangle_element(flowifier, flow_element, variable_style, true, true);
    }
    else if (flow_element->type == FlowElement_Scalar)
    {
        push_rectangle_element(flowifier, flow_element, flowifier->scalar_style, true, true);
    }
    else if (flow_element->type == FlowElement_ForeachKeyword ||
             flow_element->type == FlowElement_ForeachAsKeyword ||
             flow_element->type == FlowElement_ForeachArrowKeyword)
    {
        // TODO: what kind of style do we need to pass here? (for now using scalar_style)
        push_rectangle_element(flowifier, flow_element, flowifier->scalar_style, false, true);
    }
    else if (flow_element->type == FlowElement_ArrayAccess)
    {
        i32 expression_depth = 1; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * array_identifier_element = flow_element->first_child;
        FlowElement * array_key_expression_element = array_identifier_element->next_sibling;
        
        push_rectangle_element(flowifier, array_identifier_element, flowifier->variable_style, true, true);
        
        draw_elements(flowifier, array_key_expression_element);
    }
    else if (flow_element->type == FlowElement_IfCond ||
             flow_element->type == FlowElement_ForCond)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * keyword_element = flow_element->first_child;
        FlowElement * cond_expression_element = keyword_element->next_sibling;
        
        // FIXME: this is already draw in the if-element: draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, false);
        
        push_rectangle_element(flowifier, keyword_element, expression_style, false, true);
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, cond_expression_element);
    }
    /*
    else if (flow_element->type == FlowElement_ForeachCond)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * foreach_keyword_element = flow_element->first_child;
        FlowElement * foreach_array_expression_element = foreach_keyword_element->next_sibling;
        FlowElement * as_keyword_element = foreach_array_expression_element->next_sibling;
        FlowElement * foreach_key_var_element = 0;
        FlowElement * arrow_keyword_element = 0;
        FlowElement * foreach_value_var_element = as_keyword_element->next_sibling;
        if (foreach_value_var_element->type == FlowElement_ForeachKeyVar)
        {
            foreach_key_var_element = foreach_value_var_element;
            arrow_keyword_element = foreach_key_var_element->next_sibling;
            foreach_value_var_element = arrow_keyword_element->next_sibling;
        }
        
        push_rectangle_element(flowifier, foreach_keyword_element, expression_style, false, true);
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, foreach_array_expression_element);
        
    }
    */
    else if (flow_element->type == FlowElement_Return)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * return_keyword_element = flow_element->first_child;
        FlowElement * return_expression_element = return_keyword_element->next_sibling;
        
        push_straight_element(flowifier, flow_element);
        
        push_rectangle_element(flowifier, return_keyword_element, expression_style, false, true);
        if (return_expression_element)
        {
            // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
            draw_elements(flowifier, return_expression_element);
        }
    }
    else if (flow_element->type == FlowElement_UnaryPostOperation)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * assignee_element = flow_element->first_child;
        FlowElement * unary_operator_element = assignee_element->next_sibling;

        if (flow_element->is_statement)
        {
            // TODO: use expression_style here too
            push_straight_element(flowifier, flow_element);
        }
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, assignee_element);
        push_rectangle_element(flowifier, unary_operator_element, expression_style, false, true);
    }
    else if (flow_element->type == FlowElement_UnaryPreOperation)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * unary_operator_element = flow_element->first_child;
        FlowElement * assignee_element = unary_operator_element->next_sibling;

        if (flow_element->is_statement)
        {
            // TODO: use expression_style here too
            push_straight_element(flowifier, flow_element);
        }
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        push_rectangle_element(flowifier, unary_operator_element, expression_style, false, true);
        draw_elements(flowifier, assignee_element);
    }
    else if (flow_element->type == FlowElement_BinaryOperation)
    {
        i32 expression_depth = 1; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * left_side_expression_element = flow_element->first_child;
        FlowElement * binary_operator_element = left_side_expression_element->next_sibling;
        FlowElement * right_side_expression_element = binary_operator_element->next_sibling;
        
        push_rectangle_element(flowifier, flow_element, expression_style, true, false);
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, left_side_expression_element);
        push_rectangle_element(flowifier, binary_operator_element, expression_style, false, true);
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, right_side_expression_element);
    }
    else if (flow_element->type == FlowElement_Assignment)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * assignee_element = flow_element->first_child;
        FlowElement * assignment_operator_element = assignee_element->next_sibling;
        FlowElement * right_side_expression_element = assignment_operator_element->next_sibling;
        
        // TODO: use expression_style here too
        push_straight_element(flowifier, flow_element);
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, assignee_element);
        push_rectangle_element(flowifier, assignment_operator_element, expression_style, false, true);
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, right_side_expression_element);
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_element = flow_element;
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_then_element = if_cond_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;

        // TODO: we  draw the if-cond in a way so that the side-lines are drawn AND the if-cond-expression is drawn
        // FIXME: what should we put here as next element? Maybe a rect from the left-top of the 'else' to the right-top of the 'then'?
        push_straight_element(flowifier, if_cond_element);
        draw_elements(flowifier, if_cond_element);

        DrawableLane * cond_lane_end = flowifier->current_lane;
        DrawableLane * then_lane = push_lane(flowifier);
        DrawableLane * then_lane_end = 0;
        DrawableLane * else_lane = push_lane(flowifier);
        DrawableLane * else_lane_end = 0;
        DrawableLane * end_if_lane = push_lane(flowifier);
        
        cond_lane_end->is_splitter_at_end = true;
        
        then_lane->splitting_from_lane = cond_lane_end;
        then_lane->is_right_side_at_split = true;
        
        else_lane->splitting_from_lane = cond_lane_end;
        else_lane->is_right_side_at_split = false;
        
        i32 horizontal_distance = if_then_element->rect_abs.position.x - (if_else_element->rect_abs.position.x + if_else_element->rect_abs.size.width);
        // TODO: maybe calculate y using vertical distance i32 vertical_distance
        then_lane->splitting_point.x = if_then_element->rect_abs.position.x - horizontal_distance / 2;
        // TODO: we should check if the else-statement is higher/lower aswell (not just the then-statement) using a min()-function
        then_lane->splitting_point.y = if_then_element->rect_abs.position.y;
        
        else_lane->splitting_point = then_lane->splitting_point;
        
        flowifier->current_lane = then_lane;
        draw_elements(flowifier, if_then_element);
        then_lane_end = flowifier->current_lane; 

        flowifier->current_lane = else_lane;
        draw_elements(flowifier, if_else_element);
        else_lane_end = flowifier->current_lane; 
        
        then_lane_end->joining_towards_lane = end_if_lane;
        then_lane_end->joining_point.x = then_lane->splitting_point.x; // TODO: is it always correct that the splitting and joining points have the same x (for an if-statement)?
        // TODO: we should check if the else-statement ends higher/lower aswell (not just the then-statement) using a max()-function
        then_lane_end->joining_point.y = if_then_element->rect_abs.position.y + if_then_element->rect_abs.size.height;
        then_lane_end->is_right_side_at_join = true;
        
        else_lane_end->joining_towards_lane = end_if_lane;
        else_lane_end->joining_point = then_lane_end->joining_point;
        else_lane_end->is_right_side_at_join = false;
        
        end_if_lane->is_joiner_at_beginning = true;
        
        flowifier->current_lane = end_if_lane;
    }
    else if (flow_element->type == FlowElement_For)
    {
        FlowElement * for_element = flow_element;
        FlowElement * for_init_element = flow_element->first_child;
        FlowElement * for_cond_element = for_init_element->next_sibling;
        FlowElement * for_body_element = for_cond_element->next_sibling;
        FlowElement * for_update_element = for_body_element->next_sibling;
        FlowElement * for_passright_element = for_update_element->next_sibling;
        FlowElement * for_passup_element = for_passright_element->next_sibling;
        FlowElement * for_passleft_element = for_passup_element->next_sibling;
        FlowElement * for_passdown_element = for_passleft_element->next_sibling;
        FlowElement * for_done_element = for_passdown_element->next_sibling;

        draw_elements(flowifier, for_init_element);
        
        DrawableLane * for_lane = flowifier->current_lane;
        DrawableLane * cond_lane = push_lane(flowifier);
        DrawableLane * cond_lane_end = 0;
        DrawableLane * body_lane = push_lane(flowifier);
        DrawableLane * body_lane_end = 0;
        DrawableLane * end_for_lane = push_lane(flowifier);
        
        // TODO: we  draw the if-cond in a way so that the side-lines are drawn AND the if-cond-expression is drawn
        // FIXME: this element is in between a join and split, so what to do with the previous_in_flow and next_in_flow?
        flowifier->current_lane = cond_lane;
        push_straight_element(flowifier, for_cond_element);
        draw_elements(flowifier, for_cond_element);
        cond_lane_end = flowifier->current_lane;
        
        cond_lane_end->is_splitter_at_end = true;
        
        body_lane->splitting_from_lane = cond_lane_end;
        body_lane->is_right_side_at_split = true;
        end_for_lane->splitting_from_lane = cond_lane_end;
        end_for_lane->is_right_side_at_split = false;
        
        i32 horizontal_distance = for_body_element->rect_abs.position.x - (for_done_element->rect_abs.position.x + for_done_element->rect_abs.size.width);

        // TODO: maybe calculate y using vertical distance i32 vertical_distance
        body_lane->splitting_point.x = for_body_element->rect_abs.position.x - horizontal_distance / 2;
        // TODO: we should check if the done-statement is higher/lower aswell (not just the body-statement) using a min()-function
        body_lane->splitting_point.y = for_body_element->rect_abs.position.y;
        
        end_for_lane->splitting_point = body_lane->splitting_point;
        
        flowifier->current_lane = body_lane;
        draw_elements(flowifier, for_body_element);
        
        draw_elements(flowifier, for_update_element);
        
        // Rect2d update_rect = for_update_element->rect_abs;
        
        Rect2d passright_rect = for_passright_element->rect_abs;
        
        Rect2d passup_rect = for_passup_element->rect_abs;
        
        Rect2d passleft_rect = for_passleft_element->rect_abs;
        
        Rect2d passdown_rect = for_passdown_element->rect_abs;
        
        push_lane_part_to_current_lane(flowifier, passright_rect, Direction_LeftToRight);
        push_lane_part_to_current_lane(flowifier, passup_rect, Direction_BottomToTop);
        push_lane_part_to_current_lane(flowifier, passleft_rect, Direction_RightToLeft);
        push_lane_part_to_current_lane(flowifier, passdown_rect, Direction_TopToBottom);
        
        push_interaction_rectangle(flowifier, for_passright_element);
        push_interaction_rectangle(flowifier, for_passup_element);
        push_interaction_rectangle(flowifier, for_passleft_element);
        push_interaction_rectangle(flowifier, for_passdown_element);
        
        body_lane_end = flowifier->current_lane;
        
        body_lane_end->joining_towards_lane = cond_lane;
        body_lane_end->joining_point.x = body_lane->splitting_point.x; // TODO: is it always correct that the splitting and joining points have the same x (for an for-statement)?
        // TODO: we should check if the body-statement ends higher/lower aswell (not just the for_init-statement) using a max()-function
        body_lane_end->joining_point.y = for_init_element->rect_abs.position.y + for_init_element->rect_abs.size.height;
        body_lane_end->is_right_side_at_join = true;
        
        for_lane->joining_towards_lane = cond_lane;
        for_lane->joining_point = body_lane->joining_point;
        for_lane->is_right_side_at_join = false;
        
        cond_lane->is_joiner_at_beginning = true;
        
        flowifier->current_lane = end_for_lane;
        
        push_straight_element(flowifier, for_done_element);
    }
    else if (flow_element->type == FlowElement_Foreach)
    {
        FlowElement * foreach_element = flow_element;
        FlowElement * foreach_init_element = flow_element->first_child;
        FlowElement * foreach_cond_element = foreach_init_element->next_sibling;
        FlowElement * foreach_body_element = foreach_cond_element->next_sibling;
        FlowElement * foreach_passright_element = foreach_body_element->next_sibling;
        FlowElement * foreach_passup_element = foreach_passright_element->next_sibling;
        FlowElement * foreach_passleft_element = foreach_passup_element->next_sibling;
        FlowElement * foreach_passdown_element = foreach_passleft_element->next_sibling;
        FlowElement * foreach_done_element = foreach_passdown_element->next_sibling;

        // FIXME: what should be the last element?
        push_straight_element(flowifier, foreach_init_element);
        
        DrawableLane * foreach_lane = flowifier->current_lane;
        DrawableLane * cond_lane = push_lane(flowifier);
        DrawableLane * cond_lane_end = 0;
        DrawableLane * body_lane = push_lane(flowifier);
        DrawableLane * body_lane_end = 0;
        DrawableLane * end_foreach_lane = push_lane(flowifier);
        
        // TODO: we  draw the if-cond in a way so that the side-lines are drawn AND the if-cond-expression is drawn
        // FIXME: this element is in between a join and split, so what to do with the previous_in_flow and next_in_flow?
        flowifier->current_lane = cond_lane;
        push_straight_element(flowifier, foreach_cond_element);
        draw_elements(flowifier, foreach_cond_element);
        cond_lane_end = flowifier->current_lane;
        
        cond_lane_end->is_splitter_at_end = true;
        
        body_lane->splitting_from_lane = cond_lane_end;
        body_lane->is_right_side_at_split = true;
        end_foreach_lane->splitting_from_lane = cond_lane_end;
        end_foreach_lane->is_right_side_at_split = false;
        
        i32 horizontal_distance = foreach_body_element->rect_abs.position.x - (foreach_done_element->rect_abs.position.x + foreach_done_element->rect_abs.size.width);

        // TODO: maybe calculate y using vertical distance i32 vertical_distance
        body_lane->splitting_point.x = foreach_body_element->rect_abs.position.x - horizontal_distance / 2;
        // TODO: we should check if the done-statement is higher/lower aswell (not just the body-statement) using a min()-function
        body_lane->splitting_point.y = foreach_body_element->rect_abs.position.y;
        
        end_foreach_lane->splitting_point = body_lane->splitting_point;
        
        flowifier->current_lane = body_lane;
        draw_elements(flowifier, foreach_body_element);
        
        // Rect2d last_body_element_rect = foreach_body_element->last_in_flow->rect_abs;
        
        Rect2d passright_rect = foreach_passright_element->rect_abs;
        
        Rect2d passup_rect = foreach_passup_element->rect_abs;
        
        Rect2d passleft_rect = foreach_passleft_element->rect_abs;
        
        Rect2d passdown_rect = foreach_passdown_element->rect_abs;
        
        push_lane_part_to_current_lane(flowifier, passright_rect, Direction_LeftToRight);
        push_lane_part_to_current_lane(flowifier, passup_rect, Direction_BottomToTop);
        push_lane_part_to_current_lane(flowifier, passleft_rect, Direction_RightToLeft);
        push_lane_part_to_current_lane(flowifier, passdown_rect, Direction_TopToBottom);
        
        push_interaction_rectangle(flowifier, foreach_passright_element);
        push_interaction_rectangle(flowifier, foreach_passup_element);
        push_interaction_rectangle(flowifier, foreach_passleft_element);
        push_interaction_rectangle(flowifier, foreach_passdown_element);
        
        body_lane_end = flowifier->current_lane;
        
        body_lane_end->joining_towards_lane = cond_lane;
        body_lane_end->joining_point.x = body_lane->splitting_point.x; // TODO: is it always correct that the splitting and joining points have the same x (for an for-statement)?
        // TODO: we should check if the body-statement ends higher/lower aswell (not just the for_init-statement) using a max()-function
        body_lane_end->joining_point.y = foreach_init_element->rect_abs.position.y + foreach_init_element->rect_abs.size.height;
        body_lane_end->is_right_side_at_join = true;
        
        foreach_lane->joining_towards_lane = cond_lane;
        foreach_lane->joining_point = body_lane->joining_point;
        foreach_lane->is_right_side_at_join = false;

        cond_lane->is_joiner_at_beginning = true;
        
        flowifier->current_lane = end_foreach_lane;
        
        push_straight_element(flowifier, foreach_done_element);
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        FlowElement * function_call_element = flow_element;
        FlowElement * function_call_identifier = function_call_element->first_child;
        FlowElement * function_call_arguments = function_call_identifier->next_sibling;
        FlowElement * function_element = function_call_arguments->next_sibling;
        
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        Rect2d rect = function_call_element->rect_abs;
        
        if (function_call_element->is_statement)
        {
            push_straight_element(flowifier, function_call_element);
            
            // FIXME: We should add a Stmt_Expr element to all statements instead (and give it margins only for function-calls!)
            //        Other statements (previous_in_flow and next_in_flow) can then also correctly connect with that element!
            i32 bending_radius = flowifier->bending_radius;
            rect.position.x += bending_radius;
            rect.position.y += bending_radius;
            rect.size.width -= bending_radius + bending_radius;
            rect.size.height -= bending_radius + bending_radius;
        }
        
        push_rounded_rectangle(flowifier, rect, flowifier->bending_radius, 
                               flowifier->function_line_color, flowifier->function_fill_color, flowifier->function_line_width);

        push_interaction_rectangle(flowifier, function_call_element);
        
        // Drawing the Function call
        
        push_rectangle_element(flowifier, function_call_identifier, expression_style, false, true);
        draw_elements(flowifier, function_call_arguments);
        
        // Drawing the Function itself (if not collapsed and if not hidden)
        
        if (function_call_arguments->next_sibling->type == FlowElement_Hidden)
        {
            FlowElement * hidden_function = function_call_arguments->next_sibling;
            
            // TODO: should we somehow show the hidden element? Does it have a layouting effect?
        }
        else
        {
            DrawableLane * parent_lane = flowifier->current_lane;
            flowifier->last_lane_entry = 0; // TODO: this is a way to make sure we start with a new set of lane-entries in the draw-list
            flowifier->current_lane = push_lane(flowifier);

            FlowElement * parameters_element = function_call_arguments->next_sibling;
            FlowElement * function_element = parameters_element->next_sibling;

            if (!function_call_element->is_collapsed)
            {
                // Parameter Assignments
                
                draw_elements(flowifier, parameters_element);
                    
                // Function
                
                draw_elements(flowifier, function_element);
            }
            else
            {
                // TODO: what to draw if the function is collapsed?
            }
            
            flowifier->current_lane = parent_lane;
        }
    }
    else if (flow_element->type == FlowElement_Function)
    {
        FlowElement * function_body_element = flow_element->first_child;

        draw_elements(flowifier, function_body_element);
    }
    else if (flow_element->type == FlowElement_FunctionCallArguments ||
             flow_element->type == FlowElement_FunctionParameterAssignments ||
             flow_element->type == FlowElement_ForeachCond)
    {
        FlowElement * child_element = flow_element->first_child;
        if (child_element)
        {
            do
            {
                draw_elements(flowifier, child_element);
            }
            while ((child_element = child_element->next_sibling));
        }
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForInit ||
             flow_element->type == FlowElement_ForBody ||
             flow_element->type == FlowElement_ForUpdate ||
             flow_element->type == FlowElement_ForeachBody
             )
    {
        if (flow_element->type == FlowElement_Root)
        {
            push_rounded_rectangle(flowifier, flow_element->rect_abs, flowifier->bending_radius, 
                                   flowifier->function_line_color, flowifier->function_fill_color, flowifier->function_line_width);
                                   
            flowifier->last_lane_entry = 0; // TODO: this is a way to make sure we start with a new set of lane-entries in the draw-list
            flowifier->current_lane = push_lane(flowifier);
        }
                               
        FlowElement * child_element = flow_element->first_child;
        if (child_element)
        {
            do
            {
                draw_elements(flowifier, child_element);
            }
            while ((child_element = child_element->next_sibling));
        }
        
    }
    
    // TODO: this will double-draw in case of primitive elements
    push_interaction_rectangle(flowifier, flow_element);


    // FIXME: we should probably do this outside this function!
    
    if (flow_element->type == FlowElement_Root)
    {
        DrawableEntry * draw_entry = flowifier->first_draw_entry;
        
        draw_entries(draw_entry);
    }
}

// Actual drawing

void draw_an_entry(DrawableEntry * draw_entry)
{
    // FIXME: it's probably a better idea to check whether an entry is on the screen when trying to push it
    
    if (draw_entry->type == Drawable_RoundedRect)
    {
        DrawableRoundedRect * rounded_rect = (DrawableRoundedRect *)draw_entry->item_to_draw;
        if (rect_is_inside_screen(rounded_rect->rect))
        {
            draw_rounded_rectangle(rounded_rect->rect, 
                                   rounded_rect->radius, 
                                   rounded_rect->line_color, 
                                   rounded_rect->fill_color, 
                                   rounded_rect->line_width);
        }
    }
    else if (draw_entry->type == Drawable_Rect)
    {
        DrawableRect * rect = (DrawableRect *)draw_entry->item_to_draw;
        if (rect_is_inside_screen(rect->rect))
        {
            draw_rectangle(rect->rect, 
                           rect->line_color, 
                           rect->fill_color, 
                           rect->line_width);
        }
    }
    else if (draw_entry->type == Drawable_Text)
    {
        DrawableText * text = (DrawableText *)draw_entry->item_to_draw;
        // FIXME: we need the size of the text to draw to determine whether the text is on the screen
        Rect2d text_rect = {};
        text_rect.position = text->position;
        text_rect.size.height = 50; // FIXME: hack!
        text_rect.size.width = text->text->length * 10; // FIXME: hack!
        if (rect_is_inside_screen(text_rect))
        {
            draw_text(text->position, text->text, text->font, text->color);
        }
    }
    else if (draw_entry->type == Drawable_Lane)
    {
        DrawableLane * lane = (DrawableLane *)draw_entry->item_to_draw;
        i32 bending_radius = lane->bending_radius;
        Color4 line_color = lane->line_color;
        Color4 fill_color = lane->fill_color;
        i32 line_width = lane->line_width;
    
        b32 some_lane_parts_are_on_screen = false;
        
        Rect2d rect_to_add_at_start = {};
        b32 add_rect_at_start = false;
        
        Rect2d rect_to_add_at_end = {};
        b32 add_rect_at_end = false;
        
        DrawableLanePart * first_lane_part = lane->first_part;
        if (first_lane_part)
        {
            
            if (lane->splitting_from_lane && lane->splitting_from_lane->last_part)
            {
                // We are a lane splitting from another, so we get a "piece" of its rectangle as starting-rect
                
                add_rect_at_start = true;
                
                Rect2d splitting_rect = lane->splitting_from_lane->last_part->rect; // This is the rect that is to be split (in 3 pieces actually)
                Pos2d splitting_point = lane->splitting_point;
                
                if (lane->is_right_side_at_split)
                {
                    // We take the lower-right part.
                    
                    // The left side of the (rect that is split off) starts (horizontally) at the splitting_point
                    rect_to_add_at_start.position.x = splitting_point.x;
                    // The width is the distance between the right side of the splitting rect and the splitting point
                    rect_to_add_at_start.size.width = splitting_rect.position.x + splitting_rect.size.width - splitting_point.x; 
                    
                    // We split the rect in half vertically and take the lower half.
                    rect_to_add_at_start.position.y = splitting_rect.position.y; // TODO not needed anymore: + splitting_rect.size.height / 2; 
                    rect_to_add_at_start.size.height = splitting_rect.size.height; // TODO not needed anymore:  / 2;
                }
                else
                {
                    // We take the lower-left part.
                    
                    // The left side of the (rect that is split off) starts (horizontally) at the start of the splitting rect
                    rect_to_add_at_start.position.x = splitting_rect.position.x;
                    // The width is the distance between the splitting point and the left side of the splitting rect
                    rect_to_add_at_start.size.width = splitting_point.x - splitting_rect.position.x; 
                    
                    // We split the rect in half vertically and take the lower half.
                    rect_to_add_at_start.position.y = splitting_rect.position.y; // TODO not needed anymore:  + splitting_rect.size.height / 2; 
                    rect_to_add_at_start.size.height = splitting_rect.size.height; // TODO not needed anymore:  / 2;
                }
                
                if (rect_is_inside_screen(rect_to_add_at_start))
                {
                    some_lane_parts_are_on_screen = true;
                }
                
            }
            
            if (lane->joining_towards_lane && lane->joining_towards_lane->first_part)
            {
                // We are a lane joning towards another, so we get a "piece" of its rectangle as ending-rect
                
                add_rect_at_end = true;
                
                Rect2d joining_rect = lane->joining_towards_lane->first_part->rect; // This is the rect that is to be split (in 3 pieces actually)
                Pos2d joining_point = lane->joining_point;
                
                if (lane->is_right_side_at_join)
                {
                    // We take the lower-right part.
                    
                    // The left side of the (rect that is split off) starts (horizontally) at the joining_point
                    rect_to_add_at_end.position.x = joining_point.x;
                    // The width is the distance between the right side of the joining rect and the joining point
                    rect_to_add_at_end.size.width = joining_rect.position.x + joining_rect.size.width - joining_point.x; 
                    
                    // We split the rect in half vertically and take the upper half.
                    rect_to_add_at_end.position.y = joining_rect.position.y; 
                    rect_to_add_at_end.size.height = joining_rect.size.height; // TODO not needed anymore:  / 2;
                }
                else
                {
                    // We take the lower-left part.

                    // The left side of the (rect that is split off) starts (horizontally) at the start of the joining rect
                    rect_to_add_at_end.position.x = joining_rect.position.x;
                    // The width is the distance between the joining point and the left side of the joining rect
                    rect_to_add_at_end.size.width = joining_point.x - joining_rect.position.x; 
                    
                    // We split the rect in half vertically and take the upper half.
                    rect_to_add_at_end.position.y = joining_rect.position.y; 
                    rect_to_add_at_end.size.height = joining_rect.size.height; // TODO not needed anymore:  / 2;
                }

                if (rect_is_inside_screen(rect_to_add_at_end))
                {
                    some_lane_parts_are_on_screen = true;
                }
                
            }
                
        }
        
        // TODO: it's probably more efficient to calculate this "inside screen" when adding parts to the lane_parts
        DrawableLanePart * lane_part = first_lane_part;
        while(lane_part)
        {
            if (rect_is_inside_screen(lane_part->rect)) {
                some_lane_parts_are_on_screen = true;
                break;
            }
            
            lane_part = lane_part->next_part;
        }
        
        if (some_lane_parts_are_on_screen)
        {
            // FIXME: allocate this properly!
            i32 directional_rects_index = 0;
            DirectionalRect2d directional_rects[100];
            
            if (add_rect_at_start)
            {
                DirectionalRect2d directional_rect = {};
                
                directional_rect.position = rect_to_add_at_start.position;
                directional_rect.size = rect_to_add_at_start.size;
                directional_rect.direction = lane_part->direction;
                
                assert(directional_rects_index < 100);
                directional_rects[directional_rects_index++] = directional_rect;
            }
            
            lane_part = first_lane_part;
            while(lane_part)
            {
                Rect2d rect = lane_part->rect;

                DirectionalRect2d directional_rect = {};
                
                directional_rect.position = rect.position;
                directional_rect.size = rect.size;
                directional_rect.direction = lane_part->direction;
                
                assert(directional_rects_index < 100);
                directional_rects[directional_rects_index++] = directional_rect;
                
                lane_part = lane_part->next_part;
            }
            
            if (add_rect_at_end)
            {
                DirectionalRect2d directional_rect = {};
                
                directional_rect.position = rect_to_add_at_end.position;
                directional_rect.size = rect_to_add_at_end.size;
                directional_rect.direction = lane_part->direction;
                
                assert(directional_rects_index < 100);
                directional_rects[directional_rects_index++] = directional_rect;
            }
            
            if (lane->is_joiner_at_beginning)
            {
                // We assume a lane cannot be both a joiner at the beginning AND have a rect added at its top
                assert(!add_rect_at_start);
                
                // We remove the top half of the first lane-part
                // TODO: not needed anymore: directional_rects[0].position.y += directional_rects[0].size.height / 2;
                // TODO: not needed anymore: directional_rects[0].size.height = directional_rects[0].size.height / 2;
            }
            
            if (lane->is_splitter_at_end)
            {
                // We assume a lane cannot be both a splitter at the end AND have a rect added at its end
                assert(!add_rect_at_end);
                
                // We remove the bottom half of the last lane-part
                // TODO: not needed anymore: directional_rects[directional_rects_index - 1].size.height = directional_rects[directional_rects_index - 1].size.height / 2;
            }
            
            draw_lane(directional_rects, directional_rects_index, 
                      add_rect_at_start, lane->is_right_side_at_split, 
                      add_rect_at_end, lane->is_right_side_at_join, 
                      bending_radius, line_color, fill_color, line_width);
        }
    }
}

void draw_entries(DrawableEntry * draw_entry)
{
    while (draw_entry)
    {
        draw_an_entry(draw_entry);
        
        // If there are childs, first draw them, only after that we draw next entries (siblings)
        if (draw_entry->first_child_entry)
        {
            draw_entries(draw_entry->first_child_entry);
        }
        
        draw_entry = draw_entry->next_entry;
    }
}
