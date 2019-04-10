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

void draw_interaction_rectangle(Flowifier * flowifier, FlowElement * flow_element)
{
    if (flowifier->interaction.hovered_element_index == flow_element->index)
    {
        draw_rectangle(flow_element->rect_abs, flowifier->hovered_color, flowifier->hovered_fill, flowifier->hovered_line_width);
    }
    if (flowifier->interaction.selected_element_index == flow_element->index)
    {
        draw_rectangle(flow_element->rect_abs, flowifier->selected_color, flowifier->selected_fill, flowifier->selected_line_width);
    }
    if (flowifier->show_help_rectangles)
    {
        draw_rectangle(flow_element->rect_abs, flowifier->help_rectangle_color, flowifier->help_rectangle_fill, flowifier->help_rectangle_line_width);
    }
}

void draw_lane_segments_for_3_rectangles(Rect2d top_rect, Rect2d middle_rect, Rect2d bottom_rect, i32 bending_radius, i32 line_width, Color4 line_color, Color4 rect_color, Color4 bend_color)
{
    Color4 no_color = {};
    
    LaneSegment3 lane_segments = get_3_lane_segments_from_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius);
    
    LaneSegment lane_segment = {};
    
    if (lane_segments.has_valid_top_segment)
    {
        lane_segment = lane_segments.top;
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                          lane_segment.left_bottom, lane_segment.right_bottom, 
                          lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                          line_color, rect_color, line_width);
    }
    
    lane_segment = lane_segments.middle;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, rect_color, line_width);

    if (lane_segments.has_valid_bottom_segment)
    {
        
        lane_segment = lane_segments.bottom;
        draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                          lane_segment.left_bottom, lane_segment.right_bottom, 
                          lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                          line_color, bend_color, line_width);
    }
    
}

void draw_lane_segments_for_4_rectangles(Rect2d top_or_bottom_rect, b32 is_top_rect, Rect2d left_rect, Rect2d right_rect, Rect2d middle_rect, i32 bending_radius, i32 line_width, Color4 line_color, Color4 rect_color, Color4 bend_color)
{
    Color4 no_color = {};
    
    LaneSegment3LR lane_segments = get_3_lane_segments_from_4_rectangles(top_or_bottom_rect, is_top_rect, middle_rect, left_rect, right_rect, bending_radius);
    
    LaneSegment lane_segment = {};
    
    lane_segment = lane_segments.top_or_bottom;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, bend_color, line_width);

    // TODO: should we always draw all 3 segments? Could one or some of them be invalid?
                      
    lane_segment = lane_segments.left;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, bend_color, line_width);
                      
    lane_segment = lane_segments.right;
    draw_lane_segment(lane_segment.left_top,  lane_segment.right_top, 
                      lane_segment.left_bottom, lane_segment.right_bottom, 
                      lane_segment.left_middle_y, lane_segment.right_middle_y, lane_segment.bending_radius, 
                      line_color, bend_color, line_width);
}

void draw_joining_element(Flowifier * flowifier, FlowElement * left_element, FlowElement * right_element, 
                          FlowElement * joining_element, FlowElement * element_next_in_flow)
{
    Color4 fill_color = flowifier->unhighlighted_color;
    if (flowifier->interaction.highlighted_element_index == joining_element->index)
    {
        fill_color = flowifier->highlighted_color;
    }
        
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // Right element position + size
    Rect2d right_rect = right_element->rect_abs;
    
    // Left element position + size
    Rect2d left_rect = left_element->rect_abs;
    
    // Joining element positions + size
    Rect2d middle_rect = joining_element->rect_abs;
    
    Rect2d bottom_rect = {-1,-1,-1,-1};
    
    if (element_next_in_flow)
    {
        bottom_rect = element_next_in_flow->rect_abs;
    }
    
    draw_lane_segments_for_4_rectangles(bottom_rect, false, left_rect, right_rect, middle_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
    draw_lane_segments_for_3_rectangles(no_rect, middle_rect, bottom_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
                                       
    draw_interaction_rectangle(flowifier, joining_element);
    
}

void draw_splitting_element(Flowifier * flowifier, FlowElement * left_element, FlowElement * right_element, 
                            FlowElement * splitting_element, FlowElement * element_previous_in_flow)
{
    Color4 fill_color = flowifier->unhighlighted_color;
    if (flowifier->interaction.highlighted_element_index == splitting_element->index)
    {
        fill_color = flowifier->highlighted_color;
    }
        
    Rect2d no_rect = {-1,-1,-1,-1};

    Rect2d top_rect = no_rect;
    
    if (element_previous_in_flow)
    {
        top_rect = element_previous_in_flow->rect_abs;
    }
    
    // If-start rectangle (middle)
    Rect2d middle_rect = splitting_element->rect_abs;
    
    // If-then rectangle (right)
    Rect2d right_rect = right_element->rect_abs;
    
    // If-else rectangle (left)
    Rect2d left_rect = left_element->rect_abs;

    draw_lane_segments_for_4_rectangles(top_rect, true, left_rect, right_rect, middle_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
                                        
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, no_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
 
    draw_interaction_rectangle(flowifier, splitting_element);
}

void draw_straight_element(Flowifier * flowifier, FlowElement * flow_element, FlowElement * element_previous_in_flow, 
                           FlowElement * element_next_in_flow, b32 draw_source_text = true)
{
    Color4 fill_color = flowifier->unhighlighted_color;
    if (flowifier->interaction.highlighted_element_index == flow_element->index)
    {
        fill_color = flowifier->highlighted_color;
    }
    
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // Positions and sizes
    Rect2d top_rect = no_rect;
    Rect2d middle_rect = no_rect;
    Rect2d bottom_rect = no_rect;
    
    middle_rect = flow_element->rect_abs;
    
    if (element_previous_in_flow)
    {
        top_rect = element_previous_in_flow->rect_abs;
    }
    if (element_next_in_flow)
    {
        bottom_rect = element_next_in_flow->rect_abs;
    }
    
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
    
    if (draw_source_text && flow_element->source_text.length)
    {
        Size2d source_text_size = get_text_size(&flow_element->source_text, flowifier->font);
        
        // TODO: create a function: draw_text_in_rect()
        Pos2d text_position = {};
        text_position = middle_rect.position;
        text_position.x += (middle_rect.size.width - source_text_size.width) / 2;
        text_position.y += (middle_rect.size.height - source_text_size.height) / 2;
        
        draw_text(text_position, &flow_element->source_text, flowifier->font, flowifier->text_color);
    }
    
    draw_interaction_rectangle(flowifier, flow_element);
}

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

void draw_rectangle_element(Flowifier * flowifier, FlowElement * flow_element, FlowStyle style, b32 draw_rectangle, b32 draw_source_text)
{
    Color4 fill_color = style.fill_color;
    if (flowifier->interaction.highlighted_element_index == flow_element->index)
    {
        fill_color = flowifier->highlighted_color;
    }
    
    if (draw_rectangle)
    {
        draw_rounded_rectangle(flow_element->rect_abs, style.corner_radius, style.line_color, fill_color, style.line_width);
    }
    
    if (draw_source_text && flow_element->source_text.length)
    {
        Size2d source_text_size = get_text_size(&flow_element->source_text, flowifier->font);
        
        // TODO: create a function: draw_text_in_rect()
        Pos2d text_position = {};
        text_position = flow_element->rect_abs.position;
        text_position.x += (flow_element->rect_abs.size.width - source_text_size.width) / 2;
        text_position.y += (flow_element->rect_abs.size.height - source_text_size.height) / 2;
        
        draw_text(text_position, &flow_element->source_text, flowifier->font, flowifier->text_color);
    }
    
    draw_interaction_rectangle(flowifier, flow_element);
}

void draw_elements(Flowifier * flowifier, FlowElement * flow_element)
{
    // TODO: add is_position_of and position_originates_from
    
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_Hidden)
    {
        draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, false);
    }
    else if (flow_element->type == FlowElement_Variable ||
             flow_element->type == FlowElement_ForeachKeyVar ||
             flow_element->type == FlowElement_ForeachValueVar ||
             flow_element->type == FlowElement_UnaryOperator)
    {
        // FIXME: we should put bars on the sides of a UnaryExpression IF its a statement!!
        // FIXME: we should put bars on the sides of a UnaryExpression IF its a statement!!
        // FIXME: we should put bars on the sides of a UnaryExpression IF its a statement!!
        
        draw_rectangle_element(flowifier, flow_element, flowifier->variable_style, true, true);
    }
    else if (flow_element->type == FlowElement_Scalar)
    {
        draw_rectangle_element(flowifier, flow_element, flowifier->scalar_style, true, true);
    }
    else if (flow_element->type == FlowElement_ForeachKeyword ||
             flow_element->type == FlowElement_ForeachAsKeyword ||
             flow_element->type == FlowElement_ForeachArrowKeyword)
    {
        // TODO: what kind of style do we need to pass here? (for now using scalar_style)
        draw_rectangle_element(flowifier, flow_element, flowifier->scalar_style, false, true);
    }
    else if (flow_element->type == FlowElement_ArrayAccess)
    {
        i32 expression_depth = 1; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * array_identifier_element = flow_element->first_child;
        FlowElement * array_key_expression_element = array_identifier_element->next_sibling;
        
        draw_rectangle_element(flowifier, array_identifier_element, flowifier->variable_style, true, true);
        
        draw_elements(flowifier, array_key_expression_element);
    }
    else if (flow_element->type == FlowElement_IfCond ||
             flow_element->type == FlowElement_ForCond)
    {
        i32 expression_depth = 0; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * keyword_element = flow_element->first_child;
        FlowElement * cond_expression_element = keyword_element->next_sibling;
        
        draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, false);
        
        draw_rectangle_element(flowifier, keyword_element, expression_style, false, true);
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
        
        draw_rectangle_element(flowifier, foreach_keyword_element, expression_style, false, true);
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
        
        draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, false);
        
        draw_rectangle_element(flowifier, return_keyword_element, expression_style, false, true);
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, return_expression_element);
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
            draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, false);
        }
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, assignee_element);
        draw_rectangle_element(flowifier, unary_operator_element, expression_style, false, true);
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
            draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, false);
        }
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_rectangle_element(flowifier, unary_operator_element, expression_style, false, true);
        draw_elements(flowifier, assignee_element);
    }
    else if (flow_element->type == FlowElement_BinaryOperation)
    {
        i32 expression_depth = 1; // FIXME: fill this with the depth of the expression-stack! We should probably store this in FlowElement
        FlowStyle expression_style = get_style_by_oddness(flowifier->expression_style, expression_depth % 2);
        
        FlowElement * left_side_expression_element = flow_element->first_child;
        FlowElement * binary_operator_element = left_side_expression_element->next_sibling;
        FlowElement * right_side_expression_element = binary_operator_element->next_sibling;
        
        draw_rectangle_element(flowifier, flow_element, expression_style, true, false);
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, left_side_expression_element);
        draw_rectangle_element(flowifier, binary_operator_element, expression_style, false, true);
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
        draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, false);
        
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, assignee_element);
        draw_rectangle_element(flowifier, assignment_operator_element, expression_style, false, true);
        // FIXME: Either pass expression_depth here, or set this in FlowElement during flowification!
        draw_elements(flowifier, right_side_expression_element);
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_element = flow_element;
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;

        // TODO: we  draw the if-cond in a way so that the side-lines are drawn AND the if-cond-expression is drawn
        draw_straight_element(flowifier, if_cond_element, if_cond_element->previous_in_flow, if_split_element, false);
        draw_elements(flowifier, if_cond_element);
        
        draw_splitting_element(flowifier, if_else_element->first_in_flow, if_then_element->first_in_flow, if_split_element, if_cond_element);
        draw_elements(flowifier, if_then_element);
        draw_elements(flowifier, if_else_element);
        draw_joining_element(flowifier, if_else_element->last_in_flow, if_then_element->last_in_flow, if_join_element, if_element->next_in_flow);
    }
    else if (flow_element->type == FlowElement_For)
    {
        FlowElement * for_element = flow_element;
        FlowElement * for_init_element = flow_element->first_child;
        FlowElement * for_join_element = for_init_element->next_sibling;
        FlowElement * for_cond_element = for_join_element->next_sibling;
        FlowElement * for_split_element = for_cond_element->next_sibling;
        FlowElement * for_body_element = for_split_element->next_sibling;
        FlowElement * for_update_element = for_body_element->next_sibling;
        FlowElement * for_passright_element = for_update_element->next_sibling;
        FlowElement * for_passup_element = for_passright_element->next_sibling;
        FlowElement * for_passleft_element = for_passup_element->next_sibling;
        FlowElement * for_passdown_element = for_passleft_element->next_sibling;
        FlowElement * for_done_element = for_passdown_element->next_sibling;

        draw_elements(flowifier, for_init_element);
        
        draw_joining_element(flowifier, for_init_element, for_passdown_element, for_join_element, for_cond_element);
        
        // TODO: we  draw the if-cond in a way so that the side-lines are drawn AND the if-cond-expression is drawn
        draw_straight_element(flowifier, for_cond_element, for_join_element, for_split_element, false);
        draw_elements(flowifier, for_cond_element);
        
        draw_splitting_element(flowifier, for_done_element, for_body_element->first_in_flow, for_split_element, for_cond_element);
        
        draw_elements(flowifier, for_body_element);
        
        draw_elements(flowifier, for_update_element);
        
        Rect2d update_rect = for_update_element->rect_abs;
        
        Rect2d passright_rect = for_passright_element->rect_abs;
        
        Rect2d passup_rect = for_passup_element->rect_abs;
        
        Rect2d passleft_rect = for_passleft_element->rect_abs;
        
        Rect2d passdown_rect = for_passdown_element->rect_abs;
        
        HorLine hor_line = get_bottom_line_from_rect(update_rect);
        VertLine vert_line = get_left_line_from_rect(passright_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);
        
        vert_line = get_left_line_from_rect(passright_rect);
        hor_line = get_bottom_line_from_rect(passup_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);
        
        hor_line = get_bottom_line_from_rect(passup_rect);
        vert_line = get_left_line_from_rect(passleft_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);

        vert_line = get_left_line_from_rect(passleft_rect);
        hor_line = get_bottom_line_from_rect(passdown_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);

        draw_interaction_rectangle(flowifier, for_passright_element);
        draw_interaction_rectangle(flowifier, for_passup_element);
        draw_interaction_rectangle(flowifier, for_passleft_element);
        draw_interaction_rectangle(flowifier, for_passdown_element);
        
        draw_straight_element(flowifier, for_done_element, 0, for_done_element->next_in_flow, false);
    }
    else if (flow_element->type == FlowElement_Foreach)
    {
        FlowElement * foreach_element = flow_element;
        FlowElement * foreach_init_element = flow_element->first_child;
        FlowElement * foreach_join_element = foreach_init_element->next_sibling;
        FlowElement * foreach_cond_element = foreach_join_element->next_sibling;
        FlowElement * foreach_split_element = foreach_cond_element->next_sibling;
        FlowElement * foreach_body_element = foreach_split_element->next_sibling;
        FlowElement * foreach_passright_element = foreach_body_element->next_sibling;
        FlowElement * foreach_passup_element = foreach_passright_element->next_sibling;
        FlowElement * foreach_passleft_element = foreach_passup_element->next_sibling;
        FlowElement * foreach_passdown_element = foreach_passleft_element->next_sibling;
        FlowElement * foreach_done_element = foreach_passdown_element->next_sibling;

        draw_straight_element(flowifier, foreach_init_element, foreach_init_element->previous_in_flow, foreach_join_element, false);
        
        draw_joining_element(flowifier, foreach_init_element, foreach_passdown_element, foreach_join_element, foreach_cond_element);
        
        // TODO: we  draw the if-cond in a way so that the side-lines are drawn AND the if-cond-expression is drawn
        draw_straight_element(flowifier, foreach_cond_element, foreach_join_element, foreach_split_element, false);
        draw_elements(flowifier, foreach_cond_element);
        
        draw_splitting_element(flowifier, foreach_done_element, foreach_body_element->first_in_flow, foreach_split_element, foreach_cond_element);
        
        draw_elements(flowifier, foreach_body_element);
        
        Rect2d last_body_element_rect = foreach_body_element->last_in_flow->rect_abs;
        
        Rect2d passright_rect = foreach_passright_element->rect_abs;
        
        Rect2d passup_rect = foreach_passup_element->rect_abs;
        
        Rect2d passleft_rect = foreach_passleft_element->rect_abs;
        
        Rect2d passdown_rect = foreach_passdown_element->rect_abs;
        
        HorLine hor_line = get_bottom_line_from_rect(last_body_element_rect);
        VertLine vert_line = get_left_line_from_rect(passright_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);
        
        vert_line = get_left_line_from_rect(passright_rect);
        hor_line = get_bottom_line_from_rect(passup_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);
        
        hor_line = get_bottom_line_from_rect(passup_rect);
        vert_line = get_left_line_from_rect(passleft_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);

        vert_line = get_left_line_from_rect(passleft_rect);
        hor_line = get_bottom_line_from_rect(passdown_rect);
        draw_cornered_lane_segment(hor_line, vert_line, flowifier->bending_radius, 
                                   flowifier->line_color, flowifier->unhighlighted_color, flowifier->line_width);

        draw_interaction_rectangle(flowifier, foreach_passright_element);
        draw_interaction_rectangle(flowifier, foreach_passup_element);
        draw_interaction_rectangle(flowifier, foreach_passleft_element);
        draw_interaction_rectangle(flowifier, foreach_passdown_element);
        
        draw_straight_element(flowifier, foreach_done_element, 0, foreach_done_element->next_in_flow, false);
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
            draw_straight_element(flowifier, function_call_element, function_call_element->previous_in_flow, function_call_element->next_in_flow, false);
            
            // FIXME: We should add a Stmt_Expr element to all statements instead (and give it margins only for function-calls!)
            //        Other statements (previous_in_flow and next_in_flow) can then also correctly connect with that element!
            i32 bending_radius = flowifier->bending_radius;
            rect.position.x += bending_radius;
            rect.position.y += bending_radius;
            rect.size.width -= bending_radius + bending_radius;
            rect.size.height -= bending_radius + bending_radius;
        }
        
        draw_rounded_rectangle(rect, flowifier->bending_radius, 
                               flowifier->function_line_color, flowifier->function_fill_color, flowifier->function_line_width);

        draw_interaction_rectangle(flowifier, function_call_element);
        
        // Drawing the Function call
        
        draw_rectangle_element(flowifier, function_call_identifier, expression_style, false, true);
        draw_elements(flowifier, function_call_arguments);
        
        // Drawing the Function itself (if not collapsed and if not hidden)
        
        if (function_call_arguments->next_sibling->type == FlowElement_Hidden)
        {
            FlowElement * hidden_function = function_call_arguments->next_sibling;
            
            // TODO: should we somehow show the hidden element? Does it have a layouting effect?
        }
        else
        {
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
            draw_rounded_rectangle(flow_element->rect_abs, flowifier->bending_radius, 
                                   flowifier->function_line_color, flowifier->function_fill_color, flowifier->function_line_width);
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
    draw_interaction_rectangle(flowifier, flow_element);
    
}
