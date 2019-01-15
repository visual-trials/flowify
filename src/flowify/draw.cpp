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
        draw_rectangle(flow_element->absolute_position, flow_element->size, flowifier->hovered_color, flowifier->hovered_fill, flowifier->hovered_line_width);
    }
    if (flowifier->interaction.selected_element_index == flow_element->index)
    {
        draw_rectangle(flow_element->absolute_position, flow_element->size, flowifier->selected_color, flowifier->selected_fill, flowifier->selected_line_width);
    }
    if (flowifier->show_help_rectangles)
    {
        draw_rectangle(flow_element->absolute_position, flow_element->size, flowifier->help_rectangle_color, flowifier->help_rectangle_fill, flowifier->help_rectangle_line_width);
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
    if (joining_element->is_highlighted)
    {
        fill_color = flowifier->highlighted_color;
    }
        
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // Right element position + size
    Rect2d right_rect = {};
    right_rect.position = right_element->absolute_position;
    right_rect.size = right_element->size;
    
    // Left element position + size
    Rect2d left_rect = {};
    left_rect.position = left_element->absolute_position;
    left_rect.size = left_element->size;
    
    // Joining element positions + size
    Rect2d middle_rect = {};
    middle_rect.position = joining_element->absolute_position;
    middle_rect.size = joining_element->size;
    
    Rect2d bottom_rect = {-1,-1,-1,-1};
    
    if (element_next_in_flow)
    {
        bottom_rect.position = element_next_in_flow->absolute_position;
        bottom_rect.size = element_next_in_flow->size;
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
    if (splitting_element->is_highlighted)
    {
        fill_color = flowifier->highlighted_color;
    }
        
    Rect2d no_rect = {-1,-1,-1,-1};

    Rect2d top_rect = no_rect;
    
    if (element_previous_in_flow)
    {
        top_rect.position = element_previous_in_flow->absolute_position;
        top_rect.size = element_previous_in_flow->size;
    }
    
    // If-start rectangle (middle)
    Rect2d middle_rect = {};
    middle_rect.position = splitting_element->absolute_position;
    middle_rect.size = splitting_element->size;
    
    // If-then rectangle (right)
    Rect2d right_rect = {};
    right_rect.position = right_element->absolute_position;
    right_rect.size = right_element->size;
    
    // If-else rectangle (left)
    Rect2d left_rect = {};
    left_rect.position = left_element->absolute_position;
    left_rect.size = left_element->size;

    draw_lane_segments_for_4_rectangles(top_rect, true, left_rect, right_rect, middle_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
                                        
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, no_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
 
    draw_element_rectangle(flowifier, splitting_element);
}

void draw_straight_element(Flowifier * flowifier, FlowElement * flow_element, FlowElement * element_previous_in_flow, 
                           FlowElement * element_next_in_flow)
{
    Color4 fill_color = flowifier->unhighlighted_color;
    if (flow_element->is_highlighted)
    {
        fill_color = flowifier->highlighted_color;
    }
    
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // Positions and sizes
    Rect2d top_rect = no_rect;
    Rect2d middle_rect = no_rect;
    Rect2d bottom_rect = no_rect;
    
    Pos2d position = flow_element->absolute_position;
    Size2d size = flow_element->size;
    middle_rect.position = position;
    middle_rect.size = size;
    
    if (element_previous_in_flow)
    {
        top_rect.position = element_previous_in_flow->absolute_position;
        top_rect.size = element_previous_in_flow->size;
    }
    if (element_next_in_flow)
    {
        bottom_rect.position = element_next_in_flow->absolute_position;
        bottom_rect.size = element_next_in_flow->size;
    }
    
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, 
                                        flowifier->bending_radius, flowifier->line_width, 
                                        flowifier->line_color, fill_color, fill_color);
    
    if (flow_element->source_text.length)
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

void draw_elements(Flowifier * flowifier, FlowElement * flow_element)
{
    // TODO: add is_position_of and position_originates_from
    
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_Hidden ||
        flow_element->type == FlowElement_Assignment || 
        flow_element->type == FlowElement_BinaryOperator ||
        flow_element->type == FlowElement_Return)
    {
        draw_straight_element(flowifier, flow_element, flow_element->previous_in_flow, flow_element->next_in_flow);
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_element = flow_element;
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;

        draw_straight_element(flowifier, if_cond_element, if_cond_element->previous_in_flow, if_split_element);
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
        
        draw_straight_element(flowifier, for_cond_element, for_join_element, for_split_element);
        
        draw_splitting_element(flowifier, for_passthrough_element, for_body_element->first_in_flow, for_split_element, for_cond_element);
        
        draw_elements(flowifier, for_body_element);
        
        draw_straight_element(flowifier, for_update_element, for_body_element->last_in_flow, 0);
        
        // FIXME: we need a get_rect_from_element-function!
        Rect2d update_rect = {};
        update_rect.position = for_update_element->absolute_position;
        update_rect.size = for_update_element->size;
        
        Rect2d passright_rect = {};
        passright_rect.position = for_passright_element->absolute_position;
        passright_rect.size = for_passright_element->size;
        
        Rect2d passup_rect = {};
        passup_rect.position = for_passup_element->absolute_position;
        passup_rect.size = for_passup_element->size;
        
        Rect2d passleft_rect = {};
        passleft_rect.position = for_passleft_element->absolute_position;
        passleft_rect.size = for_passleft_element->size;
        
        Rect2d passdown_rect = {};
        passdown_rect.position = for_passdown_element->absolute_position;
        passdown_rect.size = for_passdown_element->size;
        
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
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        Pos2d position = flow_element->absolute_position;
        Size2d size = flow_element->size;
        
        // FIXME: workaround to create more vertical margin (here we substract from the size)
        position.y += flowifier->bending_radius;
        size.height -= flowifier->bending_radius * 2;
        
        if (flow_element->type == FlowElement_FunctionCall)
        {
            // TODO: determine fill color depending on odd/even depth
            draw_rounded_rectangle(position, size, flowifier->bending_radius, 
                                   flowifier->function_line_color, flowifier->function_even_fill_color, flowifier->function_line_width);
        }

        draw_element_rectangle(flowifier, flow_element);
        
        // Drawing the function itself
        
        FlowElement * function_element = flow_element->first_child;

        draw_elements(flowifier, function_element);
    }
    else if (flow_element->type == FlowElement_Function)
    {
        Pos2d position = flow_element->absolute_position;
        
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
        Pos2d position = flow_element->absolute_position;
        
        if (flow_element->type == FlowElement_Root)
        {
            // TODO: determine fill color depending on odd/even depth
            draw_rounded_rectangle(position, flow_element->size, flowifier->bending_radius, 
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
}
