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

DrawStyle get_style_by_oddness(DrawStyleEvenOdd style_even_odd, b32 is_odd)
{
    DrawStyle style = {};
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

void push_interaction_rectangle(Flowifier * flowifier, FlowElement * flow_element)
{
    if (flowifier->interaction.hovered_element_index == flow_element->index)
    {
        push_rectangle(&flowifier->renderer, flow_element->rect_abs, flowifier->hovered_style);
    }
    /*
    // FIXME: disabled for now
    if (flowifier->interaction.selected_element_index == flow_element->index)
    {
        push_rectangle(&flowifier->renderer, flow_element->rect_abs, flowifier->selected_style);
    }
    */
    // FIXME: disabled for now
    if (flowifier->show_help_rectangles)
    {
        push_rectangle(&flowifier->renderer, flow_element->rect_abs, flowifier->help_rectangle_style);
    }
}

void push_rectangle_element(Flowifier * flowifier, FlowElement * flow_element, DrawStyle draw_style, b32 draw_rectangle, b32 draw_source_text)
{
    // FIXME: is this the right way?
    if (flow_element->is_collapsed)
    {
        push_rectangle(&flowifier->renderer, flow_element->rect_abs, draw_style);
        return;
    }
    
    if (flowifier->interaction.highlighted_element_index == flow_element->index)
    {
        draw_style.fill_color = flowifier->highlighted_color;
    }
    
    if (draw_rectangle)
    {
        push_rounded_rectangle(&flowifier->renderer, flow_element->rect_abs, draw_style);
    }
    
    if (draw_source_text && flow_element->source_text.length)
    {
        // FIXME: combine this with get_size_based_on_source_text() in layout.c!
        Size2d source_text_size = {};
        source_text_size.height = flowifier->character_height;
        // FIXME: hack: emulating 'kerning' here: space between characters
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
        
        push_text(&flowifier->renderer, text_position, flow_element->source_text, flowifier->font, flowifier->text_color);
    }
    
    push_interaction_rectangle(flowifier, flow_element);
}

// TODO: maybe use a different name for this function?
void push_straight_element(Flowifier * flowifier, FlowElement * flow_element)
{
    push_lane_part_to_lane(&flowifier->renderer, flowifier->current_lane, flow_element->rect_abs, Direction_TopToBottom);
    push_interaction_rectangle(flowifier, flow_element);
}

void draw_entries(DrawableEntry * drawable_entry);

// TODO: rename this to push_elements_to_be_draw (or something) and add root-funtion: draw_elements
void draw_elements(Flowifier * flowifier, FlowElement * flow_element)
{
    assert(flow_element);

    // FIXME: is this the right way?
    if (flow_element->is_collapsed)
    {
        // FIXME: what style should we use here?
        push_rectangle_element(flowifier, flow_element, flowifier->variable_style, true, false);
        return;
    }

    BasicRenderer * renderer = &flowifier->renderer;
    
    if (flow_element->type == FlowElement_Root)
    {
        assert(renderer->draw_arena.memory);
        
        init_basic_renderer(&flowifier->renderer);
        
        flowifier->current_lane = 0;
    }
    
    // TODO: add is_position_of and position_originates_from

    Rect2d no_rect = {-1,-1,-1,-1};
    
    DrawStyle lane_style = flowifier->lane_style;
    
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
        
        
        DrawStyle variable_style = flowifier->variable_style;
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * array_identifier_element = flow_element->first_child;
        FlowElement * array_key_expression_element = array_identifier_element->next_sibling;
        
        push_rectangle_element(flowifier, array_identifier_element, flowifier->variable_style, true, true);
        
        draw_elements(flowifier, array_key_expression_element);
    }
    else if (flow_element->type == FlowElement_IfCond ||
             flow_element->type == FlowElement_ForCond)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
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
        DrawableLane * then_lane = push_lane(renderer, lane_style);
        DrawableLane * then_lane_end = 0;
        DrawableLane * else_lane = push_lane(renderer, lane_style);
        DrawableLane * else_lane_end = 0;
        DrawableLane * end_if_lane = push_lane(renderer, lane_style);
        
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
        
        push_straight_element(flowifier, if_join_element);
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
        DrawableLane * cond_lane = push_lane(renderer, lane_style);
        DrawableLane * cond_lane_end = 0;
        DrawableLane * body_lane = push_lane(renderer, lane_style);
        DrawableLane * body_lane_end = 0;
        DrawableLane * end_for_lane = push_lane(renderer, lane_style);
        
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
        
        push_lane_part_to_lane(renderer, flowifier->current_lane, passright_rect, Direction_LeftToRight);
        push_lane_part_to_lane(renderer, flowifier->current_lane, passup_rect, Direction_BottomToTop);
        push_lane_part_to_lane(renderer, flowifier->current_lane, passleft_rect, Direction_RightToLeft);
        push_lane_part_to_lane(renderer, flowifier->current_lane, passdown_rect, Direction_TopToBottom);
        
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
        DrawableLane * cond_lane = push_lane(renderer, lane_style);
        DrawableLane * cond_lane_end = 0;
        DrawableLane * body_lane = push_lane(renderer, lane_style);
        DrawableLane * body_lane_end = 0;
        DrawableLane * end_foreach_lane = push_lane(renderer, lane_style);
        
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
        
        push_lane_part_to_lane(renderer, flowifier->current_lane, passright_rect, Direction_LeftToRight);
        push_lane_part_to_lane(renderer, flowifier->current_lane, passup_rect, Direction_BottomToTop);
        push_lane_part_to_lane(renderer, flowifier->current_lane, passleft_rect, Direction_RightToLeft);
        push_lane_part_to_lane(renderer, flowifier->current_lane, passdown_rect, Direction_TopToBottom);
        
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
        DrawStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        Rect2d rect = function_call_element->rect_abs;
        
        if (function_call_element->is_statement)
        {
            push_straight_element(flowifier, function_call_element);
            
            // FIXME: We should add a Stmt_Expr element to all statements instead (and give it margins only for function-calls!)
            //        Other statements (previous_in_flow and next_in_flow) can then also correctly connect with that element!
            i32 bending_radius = flowifier->lane_style.corner_radius;
            rect.position.x += bending_radius;
            rect.position.y += bending_radius;
            rect.size.width -= bending_radius + bending_radius;
            rect.size.height -= bending_radius + bending_radius;
        }
        
        push_rounded_rectangle(renderer, rect, flowifier->function_style);

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
            b32 add_to_last_lane = false; // this is to make sure we start with a new set of lane-entries in the draw-list
            flowifier->current_lane = push_lane(renderer, lane_style, add_to_last_lane);

            FlowElement * parameters_element = function_call_arguments->next_sibling;
            FlowElement * function_element = parameters_element->next_sibling;

            if (!function_element->is_collapsed)
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
            push_rounded_rectangle(renderer, flow_element->rect_abs, flowifier->function_style);
                                   
            b32 add_to_last_lane = false; // this is to make sure we start with a new set of lane-entries in the draw-list
            flowifier->current_lane = push_lane(renderer, lane_style, add_to_last_lane);
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
    else
    {
        if (flow_element->type == FlowElement_Unknown)
        {
            // Now ignoring the FlowElement_Unknown here
        }
        else
        {
            log("Found an invalid element type!");
            log_int(flow_element->type);
            assert(false);
        }
        
    }
    
    // TODO: this will double-draw in case of primitive elements
    push_interaction_rectangle(flowifier, flow_element);


    // FIXME: we should probably do this outside this function!
    
    if (flow_element->type == FlowElement_Root)
    {
        DrawableEntry * drawable_entry = renderer->first_drawable_entry;
        
        draw_entries(&flowifier->lane_renderer, drawable_entry);
    }
}

