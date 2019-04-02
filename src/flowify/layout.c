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

Size2d get_size_based_on_source_text(Flowifier * flowifier, FlowElement * flow_element, FlowMargin margin)
{
    Size2d size = {};
    size.height = flowifier->character_height + margin.vertical * 2;
    
    if (flow_element->source_text.length)
    {
        size.width = flow_element->source_text.length * flowifier->character_width + margin.horizontal * 2;
    }
    else
    {
        size.width = flowifier->default_element_width;
    }
    
    return size;
}

// TODO: add ability to top, center or bottom vertically align
Size2d layout_horizontally(Rect2d * first_rect, Rect2d * second_rect, i32 in_between_distance, FlowMargin margin)
{
    Size2d outer_size = {};
    
    // Outer width and horizontal positions
    i32 outer_width = margin.horizontal;
    first_rect->position.x = outer_width;
    outer_width += first_rect->size.width + in_between_distance;
    second_rect->position.x = outer_width;
    outer_width += second_rect->size.width + margin.horizontal;
    outer_size.width = outer_width;
    
    // Outer height
    i32 largest_height = first_rect->size.height;
    if (second_rect->size.height > largest_height)
    {
        largest_height = second_rect->size.height;
    }
    outer_size.height = margin.vertical + largest_height + margin.vertical;
    
    // Vertical positions (center aligned)
    first_rect->position.y = margin.vertical + (largest_height / 2) - (first_rect->size.height / 2);
    second_rect->position.y = margin.vertical + (largest_height / 2) - (second_rect->size.height / 2);
    
    return outer_size;
}

// TODO: add ability to top, center or bottom vertically align
Size2d layout_horizontally(Rect2d * first_rect, Rect2d * second_rect, Rect2d * third_rect, i32 in_between_distance, FlowMargin margin)
{
    Size2d outer_size = {};
    
    // Outer width and horizontal positions
    i32 outer_width = margin.horizontal;
    first_rect->position.x = outer_width;
    outer_width += first_rect->size.width + in_between_distance;
    second_rect->position.x = outer_width;
    outer_width += second_rect->size.width + in_between_distance;
    third_rect->position.x = outer_width;
    outer_width += third_rect->size.width + margin.horizontal;
    outer_size.width = outer_width;
    
    // Outer height
    i32 largest_height = first_rect->size.height;
    if (second_rect->size.height > largest_height)
    {
        largest_height = second_rect->size.height;
    }
    if (third_rect->size.height > largest_height)
    {
        largest_height = third_rect->size.height;
    }
    outer_size.height = margin.vertical + largest_height + margin.vertical;
    
    // Vertical positions (center aligned)
    first_rect->position.y = margin.vertical + (largest_height / 2) - (first_rect->size.height / 2);
    second_rect->position.y = margin.vertical + (largest_height / 2) - (second_rect->size.height / 2);
    third_rect->position.y = margin.vertical + (largest_height / 2) - (third_rect->size.height / 2);
    
    return outer_size;
}

void layout_elements(Flowifier * flowifier, FlowElement * flow_element)
{
    i32 bending_radius = flowifier->bending_radius;
    i32 default_element_width = flowifier->default_element_width;
    i32 default_element_height = flowifier->default_element_height;
    i32 if_middle_margin = flowifier->if_middle_margin;
    i32 for_middle_margin = flowifier->for_middle_margin;
    i32 for_right_margin = flowifier->for_right_margin;
    i32 foreach_middle_margin = flowifier->for_middle_margin; // TODO: duplicate
    i32 foreach_right_margin = flowifier->for_right_margin; // TODO: duplicate
    
    if (flow_element->type == FlowElement_Hidden)
    {
        flow_element->rect.size.width = default_element_width;
        flow_element->rect.size.height = default_element_height / 2;
    }
    else if (flow_element->type == FlowElement_PassThrough) // TODO: is this ever used?
    {
        flow_element->rect.size.width = default_element_width / 2;
        flow_element->rect.size.height = default_element_height;
    }
    else if (flow_element->type == FlowElement_IfCond ||
             flow_element->type == FlowElement_ForCond)
    {
        FlowElement * keyword_element = flow_element->first_child;
        FlowElement * cond_expression_element = keyword_element->next_sibling;
        
        keyword_element->rect.size = get_size_based_on_source_text(flowifier, keyword_element, flowifier->variable_margin);
        
        layout_elements(flowifier, cond_expression_element);
        
        i32 in_between_distance = 0; // FIXME: put this in Flowifier!

        flow_element->rect.size = layout_horizontally(&keyword_element->rect, &cond_expression_element->rect, 
                                                      in_between_distance, flowifier->expression_margin);
                                                      
        flow_element->is_highlightable = true;
    }
    else if (flow_element->type == FlowElement_ForeachCond)
    {
        FlowElement * foreach_keyword_element = flow_element->first_child;
        FlowElement * foreach_array_element = foreach_keyword_element->next_sibling;
        FlowElement * as_keyword_element = foreach_array_element->next_sibling;
        FlowElement * foreach_key_var_element = 0;
        FlowElement * arrow_keyword_element = 0;
        FlowElement * foreach_value_var_element = as_keyword_element->next_sibling;
        if (foreach_value_var_element->type == FlowElement_ForeachKeyVar)
        {
            foreach_key_var_element = foreach_value_var_element;
            arrow_keyword_element = foreach_key_var_element->next_sibling;
            foreach_value_var_element = arrow_keyword_element->next_sibling;
        }
        
        foreach_keyword_element->rect.size = get_size_based_on_source_text(flowifier, foreach_keyword_element, flowifier->variable_margin);
        
        // FIXME: we skip layouting foreach_array_element completely here (and take the expression inside it), do we need it at all?
        layout_elements(flowifier, foreach_array_element->first_child);
        
        i32 in_between_distance = 0; // FIXME: put this in Flowifier!

        flow_element->rect.size = layout_horizontally(&foreach_keyword_element->rect, &foreach_array_element->rect, 
                                                      in_between_distance, flowifier->expression_margin);
                                                      
        flow_element->is_highlightable = true;
    }
    else if (flow_element->type == FlowElement_Assignment)
    {
        FlowElement * assignee_element = flow_element->first_child;
        FlowElement * assignment_operator_element = assignee_element->next_sibling;
        FlowElement * right_side_expression_element = assignment_operator_element->next_sibling;
        
        layout_elements(flowifier, assignee_element);
        layout_elements(flowifier, assignment_operator_element);
        layout_elements(flowifier, right_side_expression_element);

        i32 in_between_distance = 0; // FIXME: put this in Flowifier!

        flow_element->rect.size = layout_horizontally(&assignee_element->rect, &assignment_operator_element->rect, &right_side_expression_element->rect, 
                                                      in_between_distance, flowifier->expression_margin);
        
        flow_element->is_highlightable = true;
    }
    else if (flow_element->type == FlowElement_UnaryOperator ||
             flow_element->type == FlowElement_BinaryOperator ||
             flow_element->type == FlowElement_AssignmentOperator)
    {
        flow_element->rect.size = get_size_based_on_source_text(flowifier, flow_element, flowifier->expression_margin);
        flow_element->is_highlightable = true;
    }
    else if (flow_element->type == FlowElement_Variable ||
             flow_element->type == FlowElement_Scalar ||
             flow_element->type == FlowElement_Assignee)
    {
        flow_element->rect.size = get_size_based_on_source_text(flowifier, flow_element, flowifier->variable_margin);
        flow_element->is_highlightable = true;
    }
    else if (flow_element->type == FlowElement_ArrayAccess)
    {
        // FIXME: implement this!
        flow_element->rect.size = get_size_based_on_source_text(flowifier, flow_element, flowifier->variable_margin);
        flow_element->is_highlightable = true;
    }
    
    else if (flow_element->type == FlowElement_BinaryOperation)
    {
        FlowElement * left_side_expression_element = flow_element->first_child;
        FlowElement * binary_operator_element = left_side_expression_element->next_sibling;
        FlowElement * right_side_expression_element = binary_operator_element->next_sibling;
        
        layout_elements(flowifier, left_side_expression_element);
        layout_elements(flowifier, binary_operator_element);
        layout_elements(flowifier, right_side_expression_element);

        i32 in_between_distance = 0; // FIXME: put this in Flowifier!

        flow_element->rect.size = layout_horizontally(&left_side_expression_element->rect, &binary_operator_element->rect, &right_side_expression_element->rect, 
                                                      in_between_distance, flowifier->expression_margin);
        
        flow_element->is_highlightable = true;
        
    }
    else if (flow_element->type == FlowElement_Return)
    {
        FlowElement * return_keyword_element = flow_element->first_child;
        FlowElement * return_expression_element = return_keyword_element->next_sibling;
        
        return_keyword_element->rect.size = get_size_based_on_source_text(flowifier, return_keyword_element, flowifier->variable_margin);
        
        layout_elements(flowifier, return_expression_element);
        
        i32 in_between_distance = 0; // FIXME: put this in Flowifier!

        flow_element->rect.size = layout_horizontally(&return_keyword_element->rect, &return_expression_element->rect, 
                                                      in_between_distance, flowifier->expression_margin);
                                                      
        flow_element->is_highlightable = true;
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;
        
        // FIXME: we should compute this based on the max width of: the splitter, if-then and if-else (+ several bending radius)
        //        note that the top and bottom vertical margin could be different!
        i32 vertical_margin = 150; 
        
        // Sizing
        
        layout_elements(flowifier, if_cond_element);
        layout_elements(flowifier, if_then_element);
        layout_elements(flowifier, if_else_element);
        
        i32 then_else_height = if_then_element->rect.size.height;
        if (if_else_element->rect.size.height > then_else_height)
        {
            then_else_height = if_else_element->rect.size.height;
        }
        
        if_split_element->rect.size.height = 2 * bending_radius;
        if_split_element->rect.size.width = default_element_width;

        if_join_element->rect.size.width = default_element_width;
        if_join_element->rect.size.height = 2 * bending_radius;
        
        // Positioning
        
        Pos2d start_position = {0,0};
        
        Pos2d current_position = start_position;
        
        if_cond_element->rect.position = current_position;
        if_cond_element->is_highlightable = true;
        
        current_position.y += if_cond_element->rect.size.height;
        
        if_split_element->rect.position = current_position;
        
        current_position.y += if_split_element->rect.size.height + vertical_margin;
        
        Pos2d current_position_right = current_position;
        Pos2d current_position_left = current_position;
        
        current_position_right.x += if_else_element->rect.size.width + if_middle_margin;
        
        if_else_element->rect.position = current_position_left;
        
        if_then_element->rect.position = current_position_right;
        
        current_position.y += then_else_height + vertical_margin;
        
        if_join_element->rect.position = current_position;
        
        current_position.y += if_join_element->rect.size.height;
        
        i32 total_height = current_position.y - start_position.y;
        i32 total_width = current_position_right.x + if_then_element->rect.size.width - start_position.x;
        
        flow_element->rect.size.height = total_height;
        flow_element->rect.size.width = total_width;
    }
    else if (flow_element->type == FlowElement_For)
    {
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
        
        i32 for_body_height = for_body_element->rect.size.height;
        
        i32 for_passback_width = 50; // TODO: should actually depend on the number of data lines going through it
        i32 for_passback_height = 0;
        
        i32 for_passthrough_width = 50; // TODO: should actually depend on the number of data lines going through it
        
        i32 width_center_elements = default_element_width; // for_join, for_split, for_done
        i32 middle_margin = 4 * bending_radius;
        i32 vertical_margin = 50; // FIXME: need to calculate this properly!

        // Sizing
        
        layout_elements(flowifier, for_init_element);
        layout_elements(flowifier, for_cond_element);
        layout_elements(flowifier, for_body_element);
        layout_elements(flowifier, for_update_element);
        
        for_init_element->is_highlightable = true;
        for_cond_element->is_highlightable = true;
        for_update_element->is_highlightable = true;
        
        for_join_element->rect.size.height = 2 * bending_radius;
        for_join_element->rect.size.width = width_center_elements;
        
        for_split_element->rect.size.height = 2 * bending_radius;
        for_split_element->rect.size.width = width_center_elements;

        for_passright_element->rect.size.height = for_passback_width;
        for_passright_element->rect.size.width = for_passback_height;
        
        for_passup_element->rect.size.height = for_passback_height;
        for_passup_element->rect.size.width = for_passback_width;
        
        for_passleft_element->rect.size.height = for_passback_width;
        for_passleft_element->rect.size.width = for_passback_height;
        
        for_passdown_element->rect.size.height = for_passback_height;
        for_passdown_element->rect.size.width = for_passback_width;
        
        for_done_element->rect.size.height = 2 * bending_radius;
        for_done_element->rect.size.width = for_init_element->rect.size.width; // we set the width done-element to be the same as the init-element
        
        // Positioning
        
        Pos2d start_position = {0,0};

        // TODO: we want for_init and for_passthough to be aligned at their right-side! (and be to the left of the center)
        
        Pos2d current_position = start_position;
        
        current_position.x += for_init_element->rect.size.width + 2 * bending_radius - width_center_elements / 2;
        // TODO: we are creating some space at the top here. But we probably want the entire For-element to be move to the left, so we don't need this vertical space.
        current_position.y += 50;  // FIXME: need to calculate this properly!

        // current_position.y += for_start_element->rect.size.height + vertical_margin + vertical_margin;
        current_position.x -= for_init_element->rect.size.width + 2 * bending_radius - width_center_elements / 2;
        
        for_init_element->rect.position = current_position;
        
        current_position.y += for_init_element->rect.size.height + for_passback_width + bending_radius + bending_radius + bending_radius;
        current_position.x += for_init_element->rect.size.width + 2 * bending_radius - width_center_elements / 2;
        
        for_join_element->rect.position = current_position;
        
        current_position.y += for_join_element->rect.size.height + vertical_margin;
        
        for_cond_element->rect.position = current_position;
        // TODO: do we want to center cond? for_cond_element->rect.position.x += width_center_elements / 2 - for_cond_element->rect.size.width / 2;
        
        current_position.y += for_cond_element->rect.size.height + vertical_margin;
        
        for_split_element->rect.position = current_position;
        
        current_position.y += for_split_element->rect.size.height + vertical_margin + vertical_margin + vertical_margin;
        
        current_position.x -= for_passthrough_width + 2 * bending_radius - width_center_elements / 2;
        
        Pos2d current_position_right = current_position;
        Pos2d current_position_left = current_position;

        current_position_right.x += for_passthrough_width + middle_margin;
        for_body_element->rect.position = current_position_right;
        
        current_position_right.y += for_body_element->rect.size.height;
        
        for_update_element->rect.position = current_position_right;
        
        current_position_right.y += for_update_element->rect.size.height + vertical_margin;
        
        for_passright_element->rect.position.y = current_position_right.y;
        for_passright_element->rect.position.x = current_position_right.x + for_body_element->rect.size.width + for_right_margin / 2; // TODO: use bending radius here?
        
        current_position_right.y += for_passright_element->rect.size.height; // We don't want margin here: + vertical_margin;
        
        for_passup_element->rect.position.y = for_split_element->rect.position.y;
        for_passup_element->rect.position.x = current_position_right.x + for_body_element->rect.size.width + for_right_margin;
        
        for_passleft_element->rect.position.y = for_init_element->rect.position.y + for_init_element->rect.size.height - for_passback_width - for_passback_height - bending_radius;
        for_passleft_element->rect.position.x = current_position_right.x + for_body_element->rect.size.width + for_right_margin / 2; // TODO: use bending radius here?
        
        for_passdown_element->rect.position.y = for_init_element->rect.position.y + for_init_element->rect.size.height - for_passback_height;
        for_passdown_element->rect.position.x = for_init_element->rect.position.x + for_init_element->rect.size.width + for_middle_margin;
        
        // FIXME: we are assuming the body + update is always vertically larger than the for_done_element
        current_position_left.y = current_position_right.y;
        
        // We vertically-align the done element with the init-element
        for_done_element->rect.position.x = for_init_element->rect.position.x;
        for_done_element->rect.position.y = current_position_left.y;
        
        current_position_left.y += for_done_element->rect.size.height;
        
        i32 total_height = current_position_left.y - start_position.y;
        i32 total_width = for_passup_element->rect.position.x + for_passup_element->rect.size.width - start_position.x;
        
        flow_element->rect.size.height = total_height;
        flow_element->rect.size.width = total_width;
    }
    else if (flow_element->type == FlowElement_Foreach)
    {
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
        
        i32 foreach_body_height = foreach_body_element->rect.size.height;
        
        i32 foreach_passback_width = 50; // TODO: should actually depend on the number of data lines going through it
        i32 foreach_passback_height = 0;
        
        i32 foreach_passthrough_width = 50; // TODO: should actually depend on the number of data lines going through it
        
        i32 width_center_elements = default_element_width; // foreach_init, foreach_join, foreach_split, foreach_done
        i32 middle_margin = 4 * bending_radius;
        i32 vertical_margin = 50; // FIXME: need to calculate this properly!
        
        // Sizing
        
        layout_elements(flowifier, foreach_cond_element);
        layout_elements(flowifier, foreach_body_element);
        
        foreach_cond_element->is_highlightable = true;
        
        foreach_init_element->rect.size.height = 2 * bending_radius;
        foreach_init_element->rect.size.width = default_element_width;
        
        foreach_join_element->rect.size.height = 2 * bending_radius;
        foreach_join_element->rect.size.width = width_center_elements;
        
        foreach_split_element->rect.size.height = 2 * bending_radius;
        foreach_split_element->rect.size.width = width_center_elements;

        foreach_passright_element->rect.size.height = foreach_passback_width;
        foreach_passright_element->rect.size.width = foreach_passback_height;
        
        foreach_passup_element->rect.size.height = foreach_passback_height;
        foreach_passup_element->rect.size.width = foreach_passback_width;
        
        foreach_passleft_element->rect.size.height = foreach_passback_width;
        foreach_passleft_element->rect.size.width = foreach_passback_height;
        
        foreach_passdown_element->rect.size.height = foreach_passback_height;
        foreach_passdown_element->rect.size.width = foreach_passback_width;
        
        foreach_done_element->rect.size.height = 2 * bending_radius;
        foreach_done_element->rect.size.width = foreach_init_element->rect.size.width; // we set the width done-element to be the same as the init-element
        
        // Positioning
        
        Pos2d start_position = {0,0};

        // TODO: we want foreach_init and foreach_passthough to be aligned at their right-side! (and be to the left of the center)
        
        Pos2d current_position = start_position;
        
        current_position.x += foreach_init_element->rect.size.width + 2 * bending_radius - width_center_elements / 2;
        // TODO: we are creating some space at the top here. But we probably want the entire For-element to be move to the left, so we don't need this vertical space.
        current_position.y += 50;  // FIXME: need to calculate this properly!

        foreach_init_element->rect.position = current_position;
        
        current_position.y += foreach_init_element->rect.size.height + foreach_passback_width + bending_radius + bending_radius + bending_radius;
        current_position.x += foreach_init_element->rect.size.width + 2 * bending_radius - width_center_elements / 2;
        
        foreach_join_element->rect.position = current_position;
        
        current_position.y += foreach_join_element->rect.size.height + vertical_margin;
        
        foreach_cond_element->rect.position = current_position;
        // TODO: do we want to center cond? foreach_cond_element->rect.position.x += width_center_elements / 2 - foreach_cond_element->rect.size.width / 2;
        
        current_position.y += foreach_cond_element->rect.size.height + vertical_margin;
        
        foreach_split_element->rect.position = current_position;
        
        current_position.y += foreach_split_element->rect.size.height + vertical_margin + vertical_margin + vertical_margin;
        
        current_position.x -= foreach_passthrough_width + 2 * bending_radius - width_center_elements / 2;
        
        Pos2d current_position_right = current_position;
        Pos2d current_position_left = current_position;
        
        current_position_right.x += foreach_passthrough_width + middle_margin;
        foreach_body_element->rect.position = current_position_right;
        
        current_position_right.y += foreach_body_element->rect.size.height;
        
        foreach_passright_element->rect.position.y = current_position_right.y;
        foreach_passright_element->rect.position.x = current_position_right.x + foreach_body_element->rect.size.width + foreach_right_margin / 2; // TODO: use bending radius here?
        
        current_position_right.y += foreach_passright_element->rect.size.height; // We don't want margin here: + vertical_margin;
        
        foreach_passup_element->rect.position.y = foreach_split_element->rect.position.y;
        foreach_passup_element->rect.position.x = current_position_right.x + foreach_body_element->rect.size.width + foreach_right_margin;
        
        foreach_passleft_element->rect.position.y = foreach_init_element->rect.position.y + foreach_init_element->rect.size.height - foreach_passback_width - foreach_passback_height - bending_radius;
        foreach_passleft_element->rect.position.x = current_position_right.x + foreach_body_element->rect.size.width + foreach_right_margin / 2; // TODO: use bending radius here?
        
        foreach_passdown_element->rect.position.y = foreach_init_element->rect.position.y + foreach_init_element->rect.size.height - foreach_passback_height;
        foreach_passdown_element->rect.position.x = foreach_init_element->rect.position.x + foreach_init_element->rect.size.width + foreach_middle_margin;
        
        // FIXME: we are assuming the body + update is always vertically larger than the foreach_done_element
        current_position_left.y = current_position_right.y;
        
        // We vertically-align the done element with the init-element
        foreach_done_element->rect.position.x = foreach_init_element->rect.position.x;
        foreach_done_element->rect.position.y = current_position_left.y;
        
        current_position_left.y += foreach_done_element->rect.size.height;
        
        i32 total_height = current_position_left.y - start_position.y;
        i32 total_width = foreach_passup_element->rect.position.x + foreach_passup_element->rect.size.width - start_position.x;
        
        flow_element->rect.size.width = total_width;
        flow_element->rect.size.height = total_height;
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        FlowElement * function_element = flow_element->first_child;

         // FIXME: workaround to create more vertical margin (extra bending_radius added)
        i32 top_margin = bending_radius + bending_radius;
        i32 bottom_margin = bending_radius + bending_radius;
        
        i32 left_margin = bending_radius;
        i32 right_margin = bending_radius;
            
        layout_elements(flowifier, function_element);
        
        function_element->rect.position.x = left_margin;
        function_element->rect.position.y = top_margin;
        
        flow_element->rect.size.width = left_margin + function_element->rect.size.width + right_margin;
        flow_element->rect.size.height = top_margin + function_element->rect.size.height + bottom_margin;
        flow_element->is_highlightable = true;
        
        // flow_element->rect.size = function_element->rect.size;
    }
    else if (flow_element->type == FlowElement_Function)
    {
        FlowElement * function_body_element = flow_element->first_child;
        
        layout_elements(flowifier, function_body_element);
        
        flow_element->rect.size = function_body_element->rect.size;
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
        // FIXME: position shouldnt be set here!
        flow_element->rect.position.x = 0;
        flow_element->rect.position.y = 0;
        
        i32 top_margin = 0;
        i32 bottom_margin = 0;
        
        i32 left_margin = 0;
        i32 right_margin = 0;
        
        if (flow_element->type == FlowElement_Root)
        {
            top_margin = 20; // FIXME: get from Flowifier!
            bottom_margin = 20; // FIXME: get from Flowifier!
            
            left_margin = 20; // FIXME: get from Flowifier!
            right_margin = 20; // FIXME: get from Flowifier!
        }
        
        i32 summed_children_height = 0;
        i32 largest_child_width = 0;
        
        i32 verical_margin = 0; // TODO: we need vertical margin when connections between elements have to run horizontal
        
        FlowElement * child_element = flow_element->first_child;
        
        b32 is_first_element = true;
        if (child_element)
        {
            do
            {
                layout_elements(flowifier, child_element);
                
                // TODO: the child can be wider or narrower at the bottom or top!
                Size2d child_size = child_element->rect.size;
                
                if (!is_first_element)
                {
                    summed_children_height += verical_margin;
                }
                
                child_element->rect.position.x = left_margin;
                child_element->rect.position.y = top_margin + summed_children_height;
                
                summed_children_height += child_size.height;
                if (child_size.width > largest_child_width)
                {
                    largest_child_width = child_size.width;
                }
                
                is_first_element = false;
            }
            while ((child_element = child_element->next_sibling));
            
        }
    
        flow_element->rect.size.width = left_margin + largest_child_width + right_margin;
        flow_element->rect.size.height = top_margin + summed_children_height + bottom_margin; 
        
    }
}

void absolute_layout_elements(Flowifier * flowifier, FlowElement * flow_element, Pos2d absolute_parent_position)
{
    flow_element->rect_abs.size = flow_element->rect.size;
    flow_element->rect_abs.position = add_position_to_position(flow_element->rect.position, absolute_parent_position);
    
    FlowElement * child_element = flow_element->first_child;
    if (child_element)
    {
        do
        {
            absolute_layout_elements(flowifier, child_element, flow_element->rect_abs.position);
        }
        while ((child_element = child_element->next_sibling));
    }
    
    if (flow_element->type == FlowElement_Root)
    {
        flowifier->has_absolute_positions = true;
    }
}
