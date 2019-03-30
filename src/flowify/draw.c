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

void draw_element_rectangle(Flowifier * flowifier, FlowElement * flow_element)
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
                                       
    draw_element_rectangle(flowifier, joining_element);
    
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
 
    draw_element_rectangle(flowifier, splitting_element);
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
    
    draw_element_rectangle(flowifier, flow_element);
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
    
    draw_element_rectangle(flowifier, flow_element);
}

void draw_elements(Flowifier * flowifier, FlowElement * flow_element)
{
    // TODO: add is_position_of and position_originates_from
    
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_Hidden)
    {
        draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow);
    }
    else if (flow_element->type == FlowElement_Variable ||
        flow_element->type == FlowElement_UnaryOperator)
    {
        draw_rectangle_element(flowifier, flow_element, flowifier->variable_style, true, true);
    }
    else if (flow_element->type == FlowElement_Scalar)
    {
        draw_rectangle_element(flowifier, flow_element, flowifier->scalar_style, true, true);
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
        
        draw_rectangle_element(flowifier, assignee_element, flowifier->variable_style, true, true);
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
        draw_straight_element(flowifier, if_cond_element, if_cond_element->previous_in_flow, if_split_element);
        draw_elements(flowifier, if_cond_element);
        
        draw_splitting_element(flowifier, if_else_element->first_in_flow, if_then_element->first_in_flow, if_split_element, if_cond_element);
        draw_elements(flowifier, if_then_element);
        draw_elements(flowifier, if_else_element);
        draw_joining_element(flowifier, if_else_element->last_in_flow, if_then_element->last_in_flow, if_join_element, if_element->next_in_flow);
    }
    else if (flow_element->type == FlowElement_For)
    {
        FlowElement * for_element = flow_element;
        FlowElement * for_start_element = flow_element->first_child;
        FlowElement * for_init_element = for_start_element->next_sibling;
        FlowElement * for_join_element = for_init_element->next_sibling;
        FlowElement * for_cond_element = for_join_element->next_sibling;
        FlowElement * for_split_element = for_cond_element->next_sibling;
        FlowElement * for_body_element = for_split_element->next_sibling;
        FlowElement * for_update_element = for_body_element->next_sibling;
        FlowElement * for_passright_element = for_update_element->next_sibling;
        FlowElement * for_passup_element = for_passright_element->next_sibling;
        FlowElement * for_passleft_element = for_passup_element->next_sibling;
        FlowElement * for_passdown_element = for_passleft_element->next_sibling;
        FlowElement * for_passthrough_element = for_passdown_element->next_sibling;
        FlowElement * for_done_element = for_passthrough_element->next_sibling;

        draw_straight_element(flowifier, for_start_element, for_start_element->previous_in_flow, for_init_element);
        
        draw_straight_element(flowifier, for_init_element, for_start_element, 0);
        
        draw_joining_element(flowifier, for_init_element, for_passdown_element, for_join_element, for_cond_element);
        
        // TODO: we  draw the if-cond in a way so that the side-lines are drawn AND the if-cond-expression is drawn
        draw_straight_element(flowifier, for_cond_element, for_join_element, for_split_element);
        draw_elements(flowifier, for_cond_element);
        
        draw_splitting_element(flowifier, for_passthrough_element, for_body_element->first_in_flow, for_split_element, for_cond_element);
        
        draw_elements(flowifier, for_body_element);
        
        draw_straight_element(flowifier, for_update_element, for_body_element->last_in_flow, 0);
        
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

        draw_element_rectangle(flowifier, for_passright_element);
        draw_element_rectangle(flowifier, for_passup_element);
        draw_element_rectangle(flowifier, for_passleft_element);
        draw_element_rectangle(flowifier, for_passdown_element);
        
        draw_straight_element(flowifier, for_passthrough_element, 0, for_done_element);
        draw_straight_element(flowifier, for_done_element, for_passthrough_element, for_done_element->next_in_flow);
    }
    else if (flow_element->type == FlowElement_Foreach)
    {
        // FIXME: draw foreach
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        Rect2d rect = flow_element->rect_abs;
        
        // FIXME: workaround to create more vertical margin (here we substract from the size)
        rect.position.y += flowifier->bending_radius;
        rect.size.height -= flowifier->bending_radius * 2;
        
        if (flow_element->type == FlowElement_FunctionCall)
        {
            // TODO: determine fill color depending on odd/even depth
            draw_rounded_rectangle(rect, flowifier->bending_radius, 
                                   flowifier->function_line_color, flowifier->function_even_fill_color, flowifier->function_line_width);
        }

        draw_element_rectangle(flowifier, flow_element);
        
        // Drawing the function itself
        
        FlowElement * function_element = flow_element->first_child;

        draw_elements(flowifier, function_element);
    }
    else if (flow_element->type == FlowElement_Function)
    {
        FlowElement * function_body_element = flow_element->first_child;

        draw_elements(flowifier, function_body_element);
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForBody
             )
    {
        if (flow_element->type == FlowElement_Root)
        {
            // TODO: determine fill color depending on odd/even depth
            draw_rounded_rectangle(flow_element->rect_abs, flowifier->bending_radius, 
                                   flowifier->function_line_color, flowifier->function_odd_fill_color, flowifier->function_line_width);
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
    draw_element_rectangle(flowifier, flow_element);
    
}
