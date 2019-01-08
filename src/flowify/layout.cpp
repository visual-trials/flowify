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

void layout_elements(FlowElement * flow_element)
{
    // FIXME: we should get this from Flowifier!
    i32 bending_radius = 20;
    
    if (flow_element->type == FlowElement_Hidden)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 20;
    }
    else if (flow_element->type == FlowElement_PassThrough)
    {
        flow_element->size.width = 40;
        flow_element->size.height = 80;
    }
    else if (flow_element->type == FlowElement_Assignment)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 80;
        flow_element->is_selectable = true;
    }
    else if (flow_element->type == FlowElement_BinaryOperator)
    {
        flow_element->size.width = 200;
        flow_element->size.height = 80;
        flow_element->is_selectable = true;
    }
    else if (flow_element->type == FlowElement_Return)
    {
        flow_element->size.width = 100;
        flow_element->size.height = 40;
        flow_element->is_selectable = true;
    }
    else if (flow_element->type == FlowElement_If)
    {
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;
        
        layout_elements(if_then_element);
        layout_elements(if_else_element);
        
        i32 then_else_height = if_then_element->size.height;
        if (if_else_element->size.height > then_else_height)
        {
            then_else_height = if_else_element->size.height;
        }
        
        i32 middle_margin = 80;
        i32 vertical_margin = 150;

        Pos2d start_position = {0,0};
        
        Pos2d current_position = start_position;
        
        if_cond_element->position = current_position;
        if_cond_element->size.height = 80;
        if_cond_element->size.width = 100;
        if_cond_element->is_selectable = true;
        
        current_position.y += if_cond_element->size.height;
        
        if_split_element->position = current_position;
        if_split_element->size.height = 20;
        if_split_element->size.width = 100; // if_then_element->size.width + middle_margin + if_else_element->size.width;
        
        current_position.y += if_split_element->size.height + vertical_margin;
        
        Pos2d current_position_right = current_position;
        Pos2d current_position_left = current_position;
        
        current_position_right.x += if_else_element->size.width + middle_margin;
        
        if_else_element->position = current_position_left;
        
        if_then_element->position = current_position_right;
        
        current_position.y += then_else_height + vertical_margin;
        
        if_join_element->position = current_position;
        if_join_element->size.width = 100; //if_then_element->size.width + middle_margin + if_else_element->size.width;
        if_join_element->size.height = 20;
        
        current_position.y += if_join_element->size.height;
        
        i32 total_height = current_position.y - start_position.y;
        i32 total_width = current_position_right.x + if_then_element->size.width - start_position.x;
        
        flow_element->size.height = total_height;
        flow_element->size.width = total_width;
    }
    else if (flow_element->type == FlowElement_For)
    {
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
        
        layout_elements(for_body_element);
        
        // TODO: we should layout for_init to get its (proper) width and height
        for_init_element->size.width = 100;
        for_init_element->size.height = 80;
        for_init_element->is_selectable = true;
        /*
        TODO: create a function that gets the text from the start-token until the end token and puts it in a string
        i32 nr_of_tokens = for_init_element->ast_node->last_token_index - for_init_element->ast_node->first_token_index;
        for_init_element->size.width = nr_of_tokens * 50;
        */
        
        // TODO: we should layout for_cond to get its (proper) width and height
        for_cond_element->size.height = 80;
        for_cond_element->size.width = 100;
        for_cond_element->is_selectable = true;
        
        // TODO: we should layout for_update to get its (proper) width and height
        for_update_element->size.height = 80;
        for_update_element->size.width = 100;
        for_update_element->is_selectable = true;
        
        i32 for_body_height = for_body_element->size.height;
        
        i32 passback_width = 50; // TODO: should actually depend on the number of data lines going through it
        i32 passback_height = 0;
        
        i32 passthrough_width = 50;
        
        i32 width_center_elements = 100; // for_start, for_join, for_split, for_done
        i32 middle_margin = 4 * bending_radius;
        i32 right_margin = 100;
        i32 vertical_margin = 50;

        Pos2d start_position = {0,0};

        // TODO: we want for_init and for_passthough to be aligned at their right-side! (and be to the left of the center)
        
        Pos2d current_position = start_position;
        
        current_position.x += for_init_element->size.width + 2 * bending_radius - width_center_elements / 2;
        // TODO: we are creating some space at the top here. But we probably want the entire For-element to be move to the left, so we don't need this vertical space.
        current_position.y += 100;

        for_start_element->position = current_position;
        for_start_element->size.height = 20;
        for_start_element->size.width = width_center_elements;
        
        current_position.y += for_start_element->size.height + vertical_margin + vertical_margin;
        current_position.x -= for_init_element->size.width + 2 * bending_radius - width_center_elements / 2;
        
        for_init_element->position = current_position;
        
        current_position.y += for_init_element->size.height + passback_width + bending_radius + bending_radius + bending_radius;
        current_position.x += for_init_element->size.width + 2 * bending_radius - width_center_elements / 2;
        
        for_join_element->position = current_position;
        for_join_element->size.height = 20;
        for_join_element->size.width = width_center_elements;
        
        current_position.y += for_join_element->size.height + vertical_margin;
        
        for_cond_element->position = current_position;
        
        current_position.y += for_cond_element->size.height + vertical_margin;
        
        for_split_element->position = current_position;
        for_split_element->size.height = 20;
        for_split_element->size.width = width_center_elements;
        
        current_position.y += for_split_element->size.height + vertical_margin + vertical_margin + vertical_margin;
        
        current_position.x -= passthrough_width + 2 * bending_radius - width_center_elements / 2;
        
        Pos2d current_position_right = current_position;
        Pos2d current_position_left = current_position;

        current_position_right.x += passthrough_width + middle_margin;
        for_body_element->position = current_position_right;
        
        current_position_right.y += for_body_element->size.height;
        
        for_update_element->position = current_position_right;
        
        current_position_right.y += for_update_element->size.height + vertical_margin;
        
        for_passright_element->position.y = current_position_right.y;
        for_passright_element->position.x = current_position_right.x + for_body_element->size.width + right_margin / 2; // TODO: use bending radius here?
        for_passright_element->size.height = passback_width;
        for_passright_element->size.width = passback_height;
        
        current_position_right.y += for_passright_element->size.height + vertical_margin;
        
        for_passup_element->position.y = for_split_element->position.y;
        for_passup_element->position.x = current_position_right.x + for_body_element->size.width + right_margin;
        for_passup_element->size.height = passback_height;
        for_passup_element->size.width = passback_width;
        
        for_passleft_element->position.y = for_init_element->position.y + for_init_element->size.height - passback_width - passback_height - bending_radius;
        for_passleft_element->position.x = current_position_right.x + for_body_element->size.width + right_margin / 2; // TODO: use bending radius here?
        for_passleft_element->size.height = passback_width;
        for_passleft_element->size.width = passback_height;
        
        for_passdown_element->position.y = for_init_element->position.y + for_init_element->size.height - passback_height;
        for_passdown_element->position.x = for_init_element->position.x + for_init_element->size.width + middle_margin;
        for_passdown_element->size.height = passback_height;
        for_passdown_element->size.width = passback_width;
        
        // FIXME: we are assuming the body + update is always vertically larger than the for_passthrough_element
        current_position_left.y = current_position_right.y;
        
        for_passthrough_element->position = current_position_left;
        for_passthrough_element->size.height = 20;
        for_passthrough_element->size.width = passthrough_width;
        
        current_position_left.y += for_passthrough_element->size.height + bending_radius * 4;
        current_position_left.x += passthrough_width + 2 * bending_radius - width_center_elements / 2;
        
        for_done_element->position = current_position_left;
        for_done_element->size.height = 20;
        for_done_element->size.width = width_center_elements;
        
        current_position_left.y += for_done_element->size.height;
        
        i32 total_height = current_position_left.y - start_position.y;
        i32 total_width = for_passup_element->position.x + for_passup_element->size.width - start_position.x;
        
        flow_element->size.height = total_height;
        flow_element->size.width = total_width;
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        FlowElement * function_element = flow_element->first_child;
        
        i32 top_margin = 20;
        i32 bottom_margin = 20;
        
        i32 left_margin = 20;
        i32 right_margin = 20;
            
        layout_elements(function_element);
        
        function_element->position.x = left_margin;
        function_element->position.y = top_margin;
        
        flow_element->size.width = left_margin + function_element->size.width + right_margin;
        flow_element->size.height = top_margin + function_element->size.height + bottom_margin; 
        flow_element->is_selectable = true;
        
        // flow_element->size = function_element->size;
    }
    else if (flow_element->type == FlowElement_Function)
    {
        FlowElement * function_body_element = flow_element->first_child;
        
        layout_elements(function_body_element);
        
        flow_element->size = function_body_element->size;
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForBody)
    {
        // FIXME: position shouldnt be set here!
        flow_element->position.x = 0;
        flow_element->position.y = 0;
        
        i32 top_margin = 0;
        i32 bottom_margin = 0;
        
        i32 left_margin = 0;
        i32 right_margin = 0;
        
        if (flow_element->type == FlowElement_Root)
        {
            top_margin = 20;
            bottom_margin = 20;
            
            left_margin = 20;
            right_margin = 20;
        }
        
        i32 summed_children_height = 0;
        i32 largest_child_width = 0;
        
        i32 verical_margin = 0; // TODO: we need vertical margin when connections between elements have to run horizintal
        
        FlowElement * child_element = flow_element->first_child;
        
        b32 is_first_element = true;
        if (child_element)
        {
            do
            {
                layout_elements(child_element);
                
                // TODO: the child can be wider or narrower at the bottom or top!
                Size2d child_size = child_element->size;
                
                if (!is_first_element)
                {
                    summed_children_height += verical_margin;
                }
                
                child_element->position.x = left_margin;
                child_element->position.y = top_margin + summed_children_height;
                
                summed_children_height += child_size.height;
                if (child_size.width > largest_child_width)
                {
                    largest_child_width = child_size.width;
                }
                
                is_first_element = false;
            }
            while ((child_element = child_element->next_sibling));
            
        }
    
        flow_element->size.width = left_margin + largest_child_width + right_margin;
        flow_element->size.height = top_margin + summed_children_height + bottom_margin; 
        
    }
}

void absolute_layout_elements(FlowElement * flow_element, Pos2d absolute_parent_position)
{
    if (flow_element->type == FlowElement_PassThrough || 
        flow_element->type == FlowElement_PassBack || 
        flow_element->type == FlowElement_Assignment || 
        flow_element->type == FlowElement_BinaryOperator ||
        flow_element->type == FlowElement_Return)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
    }
    else if (flow_element->type == FlowElement_If)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * if_cond_element = flow_element->first_child;
        FlowElement * if_split_element = if_cond_element->next_sibling;
        FlowElement * if_then_element = if_split_element->next_sibling;
        FlowElement * if_else_element = if_then_element->next_sibling;
        FlowElement * if_join_element = if_else_element->next_sibling;

        absolute_layout_elements(if_cond_element, flow_element->absolute_position);
        absolute_layout_elements(if_split_element, flow_element->absolute_position);
        absolute_layout_elements(if_then_element, flow_element->absolute_position);
        absolute_layout_elements(if_else_element, flow_element->absolute_position);
        absolute_layout_elements(if_join_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_IfCond ||
             flow_element->type == FlowElement_IfSplit ||
             flow_element->type == FlowElement_IfJoin
             )
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
    }
    else if (flow_element->type == FlowElement_For)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
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

        absolute_layout_elements(for_start_element, flow_element->absolute_position);
        absolute_layout_elements(for_init_element, flow_element->absolute_position);
        absolute_layout_elements(for_join_element, flow_element->absolute_position);
        absolute_layout_elements(for_cond_element, flow_element->absolute_position);
        absolute_layout_elements(for_split_element, flow_element->absolute_position);
        absolute_layout_elements(for_body_element, flow_element->absolute_position);
        absolute_layout_elements(for_update_element, flow_element->absolute_position);
        absolute_layout_elements(for_passright_element, flow_element->absolute_position);
        absolute_layout_elements(for_passup_element, flow_element->absolute_position);
        absolute_layout_elements(for_passleft_element, flow_element->absolute_position);
        absolute_layout_elements(for_passdown_element, flow_element->absolute_position);
        absolute_layout_elements(for_passthrough_element, flow_element->absolute_position);
        absolute_layout_elements(for_done_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_ForStart ||
             flow_element->type == FlowElement_ForInit ||
             flow_element->type == FlowElement_ForJoin ||
             flow_element->type == FlowElement_ForCond ||
             flow_element->type == FlowElement_ForSplit ||
             flow_element->type == FlowElement_ForUpdate ||
             flow_element->type == FlowElement_ForDone
             )
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
    }
    else if (flow_element->type == FlowElement_FunctionCall)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * function_element = flow_element->first_child;

        absolute_layout_elements(function_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_Function)
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * function_body_element = flow_element->first_child;

        absolute_layout_elements(function_body_element, flow_element->absolute_position);
    }
    else if (flow_element->type == FlowElement_Root ||
             flow_element->type == FlowElement_FunctionBody ||
             flow_element->type == FlowElement_IfThen ||
             flow_element->type == FlowElement_IfElse ||
             flow_element->type == FlowElement_ForBody
             )
    {
        flow_element->absolute_position = add_position_to_position(flow_element->position, absolute_parent_position);
        
        FlowElement * child_element = flow_element->first_child;
        if (child_element)
        {
            do
            {
                absolute_layout_elements(child_element, flow_element->absolute_position);
            }
            while ((child_element = child_element->next_sibling));
        }
    }
    
}
