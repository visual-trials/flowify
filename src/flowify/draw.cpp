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

void draw_joining_element(FlowElement * left_element, FlowElement * right_element, 
                          FlowElement * joining_element, FlowElement * element_next_in_flow, 
                          b32 show_help_rectangles)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    Color4 no_color         = {};
    Color4 fill_color = unselected_color;
    if (joining_element->is_selected)
    {
        fill_color = selected_color;
    }
        
    i32 line_width = 4;
    i32 bending_radius = 20;
    Rect2d no_rect = {-1,-1,-1,-1};
    
    // Right element position + size
    Rect2d right_rect = {};
    // FIXME: we need the position and size of the LAST statement in the then-element, NOT of the then-element itself!
    right_rect.position = right_element->absolute_position;
    right_rect.size = right_element->size;
    
    // Left element position + size
    Rect2d left_rect = {};
    // FIXME: we need the position and size of the LAST statement in the else-element, NOT of the else-element itself!
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
    
    draw_lane_segments_for_4_rectangles(bottom_rect, false, left_rect, right_rect, middle_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    draw_lane_segments_for_3_rectangles(no_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    
    if (show_help_rectangles)
    {
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, rectangle_fill, 2);
    }
}

void draw_splitting_element(FlowElement * left_element, FlowElement * right_element, FlowElement * splitting_element, FlowElement * element_previous_in_flow, b32 show_help_rectangles)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    Color4 no_color         = {};
    Color4 fill_color = unselected_color;
    if (splitting_element->is_selected)
    {
        fill_color = selected_color;
    }
        
    i32 line_width = 4;
    i32 bending_radius = 20;
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
    // FIXME: we need the position and size of the FIRST statement in the then-element, NOT of the then-element itself!
    right_rect.position = right_element->absolute_position;
    right_rect.size = right_element->size;
    
    // If-else rectangle (left)
    Rect2d left_rect = {};
    // FIXME: we need the position and size of the FIRST statement in the else-element, NOT of the else-element itself!
    left_rect.position = left_element->absolute_position;
    left_rect.size = left_element->size;

    draw_lane_segments_for_4_rectangles(top_rect, true, left_rect, right_rect, middle_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    // FIXME: we somehow need the previous element, BEFORE the start-if as top_rect!
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, no_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    
    if (show_help_rectangles)
    {
        draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, rectangle_fill, 2);
    }
}

void draw_element_rectangle(FlowElement * flow_element)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    
    Rect2d rect = {};
    rect.position = flow_element->absolute_position;
    rect.size = flow_element->size;
    
    draw_rectangle(rect.position, rect.size, rectangle_color, rectangle_fill, 2);
}

void draw_straight_element(FlowElement * flow_element, FlowElement * element_previous_in_flow, 
                           FlowElement * element_next_in_flow, b32 show_help_rectangles)
{
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)

    // Colors
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 no_color         = {};
    Color4 fill_color = unselected_color;
    if (flow_element->is_selected)
    {
        fill_color = selected_color;
    }
        
    i32 line_width = 4;
    i32 bending_radius = 20;
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
    
    draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
    if (show_help_rectangles)
    {
        draw_element_rectangle(flow_element);
    }
}

void draw_elements(FlowElement * flow_element, b32 show_help_rectangles)
{
    // TODO: add is_position_of and position_originates_from
    // TODO: set colors properly
    
    // TODO: maybe we want to have a drawer-variable (Drawer-struct), containing all color/line_width/bending_radius settings)
    
    Color4 line_color       = {  0,   0,   0, 255};
    Color4 unselected_color = {180, 180, 255, 255};
    Color4 selected_color   = {180, 255, 180, 255};
    Color4 rectangle_color  = {255, 0, 0, 255};
    Color4 rectangle_fill   = {255, 0, 0, 50};
    Color4 no_color         = {};
    i32 line_width = 4;
    i32 bending_radius = 20;
    
    Rect2d no_rect = {-1,-1,-1,-1};
    
    Color4 function_line_color = { 200, 200, 200, 255};
    Color4 function_fill_color = { 240, 240, 240, 255};
    i32 function_line_width = 2;
    
    // TODO: we probably want flags here!
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_Assignment || 
        flow_element->type == FlowElement_BinaryOperator ||
        flow_element->type == FlowElement_Return)
    {
        draw_straight_element(flow_element, flow_element->previous_in_flow, flow_element->next_in_flow, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_element = flow_element;
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;

        draw_straight_element(if_cond_element, if_cond_element->previous_in_flow, if_split_element, show_help_rectangles);
        draw_splitting_element(if_else_element, if_then_element, if_split_element, if_cond_element, show_help_rectangles);
        draw_elements(if_then_element, show_help_rectangles);
        draw_elements(if_else_element, show_help_rectangles);
        draw_joining_element(if_else_element, if_then_element, if_join_element, if_element->next_in_flow, show_help_rectangles);
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

        draw_straight_element(for_start_element, for_start_element->previous_in_flow, for_init_element, show_help_rectangles);
        
        draw_straight_element(for_init_element, for_start_element, 0, show_help_rectangles);
        
        draw_joining_element(for_init_element, for_passdown_element, for_join_element, for_cond_element, show_help_rectangles);
        
        draw_straight_element(for_cond_element, for_join_element, for_split_element, show_help_rectangles);
        draw_splitting_element(for_passthrough_element, for_body_element, for_split_element, for_cond_element, show_help_rectangles);
        draw_elements(for_body_element, show_help_rectangles);
        // TODO: The previous_in_flow and next_in_flow of the update_element are not correct (previous should be last statement in for-body, next should be?)
        draw_straight_element(for_update_element, for_body_element, 0, show_help_rectangles);
        
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
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);
        
        vert_line = get_left_line_from_rect(passright_rect);
        hor_line = get_bottom_line_from_rect(passup_rect);
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);
        
        hor_line = get_bottom_line_from_rect(passup_rect);
        vert_line = get_left_line_from_rect(passleft_rect);
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);

        vert_line = get_left_line_from_rect(passleft_rect);
        hor_line = get_bottom_line_from_rect(passdown_rect);
        draw_cornered_lane_segment(hor_line, vert_line, bending_radius, line_color, unselected_color, line_width);

        if (show_help_rectangles)
        {
            draw_element_rectangle(for_passright_element);
            draw_element_rectangle(for_passup_element);
            draw_element_rectangle(for_passleft_element);
            draw_element_rectangle(for_passdown_element);
        }
        
        draw_straight_element(for_passthrough_element, 0, for_done_element, show_help_rectangles);
        draw_straight_element(for_done_element, for_passthrough_element, for_done_element->next_in_flow, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        // Colors
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        
        // FIXME: HACK!
        fill_color.r -= 30;
        fill_color.g -= 30;
        fill_color.b -= 30;
        
        
        // Positions and sizes
        Rect2d top_rect = {-1,-1,-1,-1};
        Rect2d middle_rect = {-1,-1,-1,-1};
        Rect2d bottom_rect = {-1,-1,-1,-1};
        
        Pos2d position = flow_element->absolute_position;
        Size2d size = flow_element->size;
        middle_rect.position = position;
        middle_rect.size = size;
        
        if (flow_element->previous_in_flow)
        {
            top_rect.position = flow_element->previous_in_flow->absolute_position;
            top_rect.size = flow_element->previous_in_flow->size;
        }
        if (flow_element->next_in_flow)
        {
            bottom_rect.position = flow_element->next_in_flow->absolute_position;
            bottom_rect.size = flow_element->next_in_flow->size;
        }
        
        draw_lane_segments_for_3_rectangles(top_rect, middle_rect, bottom_rect, bending_radius, line_width, line_color, fill_color, fill_color);
        
        if (show_help_rectangles)
        {
            draw_rectangle(middle_rect.position, middle_rect.size, rectangle_color, rectangle_fill, 2);
        }
        
        // Drawing the function itself
        
        FlowElement * function_element = flow_element->first_child;

        draw_elements(function_element, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_Function)
    {
        Pos2d position = flow_element->absolute_position;
        
        FlowElement * function_body_element = flow_element->first_child;

        draw_elements(function_body_element, show_help_rectangles);
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForBody
             )
    {
        Pos2d position = flow_element->absolute_position;
        
        /*
        Color4 fill_color = unselected_color;
        if (flow_element->is_selected)
        {
            fill_color = selected_color;
        }
        */
        
        if (flow_element->type == FlowElement_Root)
        {
            draw_rounded_rectangle(position, flow_element->size, 20, 
                                   function_line_color, function_fill_color, function_line_width);
        }
                               
        FlowElement * child_element = flow_element->first_child;
        if (child_element)
        {
            do
            {
                draw_elements(child_element, show_help_rectangles);
            }
            while ((child_element = child_element->next_sibling));
        }
        
    }
}
