/*

   Copyright 2019 Jeffrey Hullekes

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
 
void push_text(BasicRenderer * renderer, Pos2d position, String text, Font font, Color4 color)
{
    DrawableEntry * drawable_entry = (DrawableEntry *)push_struct(&renderer->draw_arena, sizeof(DrawableEntry));
    drawable_entry->type = Drawable_Text;
    drawable_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableText * drawable_text = (DrawableText *)push_struct(&renderer->draw_arena, sizeof(DrawableText));
    drawable_entry->item_to_draw = drawable_text;
    
    drawable_text->position = position;
    drawable_text->text = text;
    drawable_text->font = font;
    drawable_text->color = color;
    
    add_drawable_entry(renderer, drawable_entry);
}

void push_rectangle(BasicRenderer * renderer, Rect2d rect, DrawStyle draw_style)
{
    DrawableEntry * drawable_entry = (DrawableEntry *)push_struct(&renderer->draw_arena, sizeof(DrawableEntry));
    drawable_entry->type = Drawable_Rect;
    drawable_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableRect * drawable_rect = (DrawableRect *)push_struct(&renderer->draw_arena, sizeof(DrawableRect));
    drawable_entry->item_to_draw = drawable_rect;
    
    drawable_rect->rect = rect;
    drawable_rect->draw_style = draw_style;
    
    add_drawable_entry(renderer, drawable_entry);
}

void push_rounded_rectangle(BasicRenderer * renderer, Rect2d rect, DrawStyle draw_style)
{
    DrawableEntry * drawable_entry = (DrawableEntry *)push_struct(&renderer->draw_arena, sizeof(DrawableEntry));
    drawable_entry->type = Drawable_RoundedRect;
    drawable_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableRoundedRect * drawable_rounded_rect = (DrawableRoundedRect *)push_struct(&renderer->draw_arena, sizeof(DrawableRoundedRect));
    drawable_entry->item_to_draw = drawable_rounded_rect;
    
    drawable_rounded_rect->rect = rect;
    drawable_rounded_rect->draw_style = draw_style;
    
    add_drawable_entry(renderer, drawable_entry);
}

DrawableLane * push_lane(BasicRenderer * renderer, DrawStyle lane_style, b32 add_to_last_lane = true)
{
    DrawableEntry * drawable_entry = (DrawableEntry *)push_struct(&renderer->draw_arena, sizeof(DrawableEntry));
    drawable_entry->type = Drawable_Lane;
    drawable_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableLane * drawable_lane = (DrawableLane *)push_struct(&renderer->draw_arena, sizeof(DrawableLane));
    drawable_entry->item_to_draw = drawable_lane;
    
    // TODO: we should let push_struct reset the memory of the struct!
    drawable_lane->bounding_rect = (Rect2d){};
    
    drawable_lane->first_part = 0;
    drawable_lane->last_part = 0;
    
    drawable_lane->splitting_from_lane = 0;
    drawable_lane->is_right_side_at_split = false;
    drawable_lane->splitting_point = (Pos2d){};
    
    drawable_lane->is_splitter_at_end = false;
    
    drawable_lane->joining_towards_lane = 0;
    drawable_lane->is_right_side_at_join = false;
    drawable_lane->joining_point = (Pos2d){};
    
    drawable_lane->is_joiner_at_beginning = false;
    
    drawable_lane->draw_style = lane_style;
        
    if (add_to_last_lane && renderer->last_lane_entry)
    {
        // We just want to extend the last_lane_entry, we don't want to add an entry in the regular list of entries
        add_child_drawable_entry(drawable_entry, renderer->last_lane_entry);
    }
    else
    {
        add_drawable_entry(renderer, drawable_entry);
    }
    
    return drawable_lane;
}

// TODO: we might want to add fill_color as an argument here
void push_lane_part_to_lane(BasicRenderer * renderer, DrawableLane * lane, Rect2d rect, Direction direction)
{
    assert(lane);
    
    DrawableLanePart * drawable_lane_part = (DrawableLanePart *)push_struct(&renderer->draw_arena, sizeof(DrawableLanePart));
    drawable_lane_part->rect = rect;
    drawable_lane_part->next_part = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_lane_part->direction = direction;
    
    if (!lane->first_part)
    {
        lane->first_part = drawable_lane_part;
    }
    else
    {
        lane->last_part->next_part = drawable_lane_part;
    }
    lane->last_part = drawable_lane_part;
}

void push_path_part(LaneRenderer * lane_renderer, i32 x, i32 y, PathPartType type, b32 is_right)
{
    DrawablePathPart path_part = (DrawablePathPart){ x, y, type };
    if (is_right) {
        add_to_array(&lane_renderer->right_path_parts, &path_part);
    }
    else {
        add_to_array(&lane_renderer->left_path_parts, &path_part);
    }
}

void push_left_parts(LaneRenderer * lane_renderer, 
                     i32 left_top_x, i32 top_y, i32 left_middle_y, 
                     i32 left_bottom_x, i32 bottom_y, i32 radius, 
                     b32 skip_top_part_when_border = false, 
                     b32 skip_bottom_part_when_border = false) {

    PathPartType top_line_type = PathPart_Line;
    if (skip_top_part_when_border) {
        top_line_type = PathPart_LineWhenBackground;
    }
    PathPartType bottom_line_type = PathPart_Line;
    if (skip_bottom_part_when_border) {
        bottom_line_type = PathPart_LineWhenBackground;
    }
    
    // Draw left side
    
    if (!lane_renderer->left_path_parts.nr_of_items) {
        push_path_part(lane_renderer, left_top_x, top_y, PathPart_LineWhenBackground, false);
    }
    
    if (left_bottom_x < left_top_x) {
        // bottom is to the left of the top
        if (left_top_x - left_bottom_x < radius * 2) {
            radius = (left_top_x - left_bottom_x) / 2;
        }
        push_path_part(lane_renderer, left_top_x, left_middle_y - radius, top_line_type, false);
        push_path_part(lane_renderer, left_top_x - radius, left_middle_y, PathPart_Arc_DownToLeft, false);
        push_path_part(lane_renderer, left_bottom_x + radius, left_middle_y, PathPart_Line, false);
        push_path_part(lane_renderer, left_bottom_x, left_middle_y + radius, PathPart_Arc_LeftToDown, false);
        push_path_part(lane_renderer, left_bottom_x, bottom_y, bottom_line_type, false);
    }
    else if (left_bottom_x > left_top_x) {
        // bottom is to the right of the top
        if (left_bottom_x - left_top_x < radius * 2) {
            radius = (left_bottom_x - left_top_x) / 2;
        }
        push_path_part(lane_renderer, left_top_x, left_middle_y - radius, top_line_type, false);
        push_path_part(lane_renderer, left_top_x + radius, left_middle_y, PathPart_Arc_DownToRight, false);
        push_path_part(lane_renderer, left_bottom_x - radius, left_middle_y, PathPart_Line, false);
        push_path_part(lane_renderer, left_bottom_x, left_middle_y + radius, PathPart_Arc_RightToDown, false);
        push_path_part(lane_renderer, left_bottom_x, bottom_y, bottom_line_type, false);
    }
    else {
        // straight vertical line
        push_path_part(lane_renderer, left_bottom_x, bottom_y, PathPart_Line, false);
    }
}

void push_right_parts(LaneRenderer * lane_renderer, 
                     i32 right_top_x, i32 top_y, i32 right_middle_y, 
                     i32 right_bottom_x, i32 bottom_y, i32 radius, 
                     b32 skip_top_part_when_border = false, 
                     b32 skip_bottom_part_when_border = false) {
    
    PathPartType top_line_type = PathPart_Line;
    if (skip_top_part_when_border) {
        top_line_type = PathPart_LineWhenBackground;
    }
    PathPartType bottom_line_type = PathPart_Line;
    if (skip_bottom_part_when_border) {
        bottom_line_type = PathPart_LineWhenBackground;
    }
    
    // Right side (bottom to top)
    
    if (!lane_renderer->right_path_parts.nr_of_items) {
        push_path_part(lane_renderer, right_bottom_x, bottom_y, PathPart_LineWhenBackground, true);
    }
            
    if (right_bottom_x < right_top_x) {
        // bottom is to the left of the top
        if (right_top_x - right_bottom_x < radius * 2) {
            radius = (right_top_x - right_bottom_x) / 2;
        }
        push_path_part(lane_renderer, right_bottom_x, right_middle_y + radius, bottom_line_type, true);
        push_path_part(lane_renderer, right_bottom_x + radius, right_middle_y, PathPart_Arc_UpToRight, true);
        push_path_part(lane_renderer, right_top_x - radius, right_middle_y, PathPart_Line, true);
        push_path_part(lane_renderer, right_top_x, right_middle_y - radius, PathPart_Arc_RightToUp, true);
        push_path_part(lane_renderer, right_top_x, top_y, top_line_type, true);
    }
    else if (right_bottom_x > right_top_x) {
        // bottom is to the right of the top
        if (right_bottom_x - right_top_x < radius * 2) {
            radius = (right_bottom_x - right_top_x) / 2;
        }
        push_path_part(lane_renderer, right_bottom_x, right_middle_y + radius, bottom_line_type, true);
        push_path_part(lane_renderer, right_bottom_x - radius, right_middle_y, PathPart_Arc_UpToLeft, true);
        push_path_part(lane_renderer, right_top_x + radius, right_middle_y, PathPart_Line, true);
        push_path_part(lane_renderer, right_top_x, right_middle_y - radius, PathPart_Arc_LeftToUp, true);
        push_path_part(lane_renderer, right_top_x, top_y, top_line_type, true);
    }
    else {
        // straight vertical line
        push_path_part(lane_renderer, right_top_x, top_y, PathPart_Line, true);
    }
}

void push_parts_on_one_side_of_corner(LaneRenderer * lane_renderer, 
                                      i32 first_x, i32 first_y, Direction first_direction, 
                                      i32 second_x, i32 second_y, Direction second_direction, 
                                      i32 radius, b32 is_right, 
                                      b32 skip_top_part_when_border = false, 
                                      b32 skip_bottom_part_when_border = false) {
    
    if (!is_right && !lane_renderer->left_path_parts.nr_of_items) {
        push_path_part(lane_renderer, first_x, first_y, PathPart_LineWhenBackground, is_right);
    }
    if (is_right && !lane_renderer->right_path_parts.nr_of_items) {
        push_path_part(lane_renderer, first_x, first_y, PathPart_LineWhenBackground, is_right);
    }

    // FIXME: adjust radius to make it fit!
    
    // Forwards
    if (first_direction == Direction_TopToBottom && second_direction == Direction_LeftToRight) {
        push_path_part(lane_renderer, first_x, second_y - radius, PathPart_Line, is_right);
        push_path_part(lane_renderer, first_x + radius, second_y, PathPart_Arc_DownToRight, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    else if (first_direction == Direction_LeftToRight && second_direction == Direction_BottomToTop) {
        push_path_part(lane_renderer, second_x - radius, first_y, PathPart_Line, is_right);
        push_path_part(lane_renderer, second_x, first_y - radius, PathPart_Arc_RightToUp, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    else if (first_direction == Direction_BottomToTop && second_direction == Direction_RightToLeft) {
        push_path_part(lane_renderer, first_x, second_y + radius, PathPart_Line, is_right);
        push_path_part(lane_renderer, first_x - radius, second_y, PathPart_Arc_UpToLeft, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    else if (first_direction == Direction_RightToLeft && second_direction == Direction_TopToBottom) {
        push_path_part(lane_renderer, second_x + radius, first_y, PathPart_Line, is_right);
        push_path_part(lane_renderer, second_x, first_y + radius, PathPart_Arc_LeftToDown, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    // Backwards
    else if (first_direction == Direction_TopToBottom && second_direction == Direction_RightToLeft) {
        push_path_part(lane_renderer, first_x, second_y + radius, PathPart_Line, is_right);
        push_path_part(lane_renderer, first_x + radius, second_y, PathPart_Arc_UpToRight, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    else if (first_direction == Direction_RightToLeft && second_direction == Direction_BottomToTop) {
        push_path_part(lane_renderer, second_x - radius, first_y, PathPart_Line, is_right);
        push_path_part(lane_renderer, second_x, first_y + radius, PathPart_Arc_RightToDown, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    else if (first_direction == Direction_BottomToTop && second_direction == Direction_LeftToRight) {
        push_path_part(lane_renderer, first_x, second_y - radius, PathPart_Line, is_right);
        push_path_part(lane_renderer, first_x - radius, second_y, PathPart_Arc_DownToLeft, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    else if (first_direction == Direction_LeftToRight && second_direction == Direction_TopToBottom) {
        push_path_part(lane_renderer, second_x + radius, first_y, PathPart_Line, is_right);
        push_path_part(lane_renderer, second_x, first_y - radius, PathPart_Arc_LeftToUp, is_right);
        push_path_part(lane_renderer, second_x, second_y, PathPart_Line, is_right);
    }
    else {
        log("ERROR: unsupported combination of directions!");
    }
}

// FIXME: put this in geometry.c
Pos2d get_half_way_side_point(DirectionalRect2d directional_rect, b32 is_right)
{
    Pos2d half_way_point = {};
    
    Direction direction = directional_rect.direction;
    
    if (!is_right) // left
    {
        if (direction == Direction_TopToBottom) {
            // Half-way point is on the left-middle
            half_way_point.x = directional_rect.position.x;
            half_way_point.y = directional_rect.position.y + directional_rect.size.height / 2;
        }
        else if (direction == Direction_LeftToRight) {
            // Half-way point is on the middle-bottom
            half_way_point.x = directional_rect.position.x + directional_rect.size.width / 2;
            half_way_point.y = directional_rect.position.y + directional_rect.size.height;
        }
        else if (direction == Direction_BottomToTop) {
            // Half-way point is on the right-middle
            half_way_point.x = directional_rect.position.x + directional_rect.size.width;
            half_way_point.y = directional_rect.position.y + directional_rect.size.height / 2;
        }
        else if (direction == Direction_RightToLeft) {
            // Half-way point is on the middle-top
            half_way_point.x = directional_rect.position.x + directional_rect.size.width / 2;
            half_way_point.y = directional_rect.position.y;
        }
    }
    else // right
    {
        if (direction == Direction_BottomToTop) {
            // Half-way point is on the left-middle
            half_way_point.x = directional_rect.position.x;
            half_way_point.y = directional_rect.position.y + directional_rect.size.height / 2;
        }
        else if (direction == Direction_RightToLeft) {
            // Half-way point is on the middle-bottom
            half_way_point.x = directional_rect.position.x + directional_rect.size.width / 2;
            half_way_point.y = directional_rect.position.y + directional_rect.size.height;
        }
        else if (direction == Direction_TopToBottom) {
            // Half-way point is on the right-middle
            half_way_point.x = directional_rect.position.x + directional_rect.size.width;
            half_way_point.y = directional_rect.position.y + directional_rect.size.height / 2;
        }
        else if (direction == Direction_LeftToRight) {
            // Half-way point is on the middle-top
            half_way_point.x = directional_rect.position.x + directional_rect.size.width / 2;
            half_way_point.y = directional_rect.position.y;
        }
    }
    
    return half_way_point;
}

void draw_lane_using_directional_rects(LaneRenderer * lane_renderer,
               DirectionalRect2d * lane_parts, i32 lane_parts_count,
               b32 partial_rect_at_start, b32 is_right_side_at_start, b32 is_joiner_at_beginning,
               b32 partial_rect_at_end, b32 is_right_side_at_end, b32 is_splitter_at_end,
               DrawStyle lane_style)
{
    if (lane_parts_count <= 0) {
        return;
    }
    
    i32 radius = lane_style.corner_radius;
    
    init_lane_renderer(lane_renderer);
        
    // Left side (top to bottom)
    
    // Check if first rect should be drawn from its top to the half of its height
    // This is only done if this rect wasn't part of the previous lane (indicated by a partial rect that was added at the start)
    // and this lane wasn't a joiner at the beginning (because then this first half would be drawn by the lane that joins with us.
    if (!partial_rect_at_start && !is_joiner_at_beginning) {
        DirectionalRect2d lane_part = lane_parts[0];
        
        push_left_parts(lane_renderer,
                        lane_part.position.x, lane_part.position.y, 
                        lane_part.position.y + lane_part.size.height / 4, // TODO: 1 / 4 as middle_y (we shouldnt use push_left_parts, we only draw a straight (half) lane_part)
                        lane_part.position.x, lane_part.position.y + lane_part.size.height / 2, radius);
    }
    
    DirectionalRect2d empty_lane_part = {};
    
    DirectionalRect2d previous_lane_part = empty_lane_part;
    for (i32 lane_part_index = 0; lane_part_index < lane_parts_count; lane_part_index++) {
        DirectionalRect2d lane_part = lane_parts[lane_part_index];
        
        if (lane_part_index > 0) {
            
            b32 is_right = false;
            
            // If either the current lane part of the previous lane part is not top-to-bottom
            // it means we have to draw a corner
            if (lane_part.direction != Direction_TopToBottom || previous_lane_part.direction != Direction_TopToBottom) {
                
                Pos2d start_side_point = get_half_way_side_point(previous_lane_part, is_right);
                Pos2d end_side_point = get_half_way_side_point(lane_part, is_right);
                
                push_parts_on_one_side_of_corner(lane_renderer, start_side_point.x, start_side_point.y, previous_lane_part.direction, 
                                                                end_side_point.x, end_side_point.y, lane_part.direction, radius, is_right);
            }
            else {
            
                Pos2d start_side_point = get_half_way_side_point(previous_lane_part, is_right);
                Pos2d end_side_point = get_half_way_side_point(lane_part, is_right);

                
                b32 skip_top_part_when_border = false;
                b32 skip_bottom_part_when_border = false;
                
                i32 distance_between_rects = lane_part.position.y - (previous_lane_part.position.y + previous_lane_part.size.height);
                i32 left_middle_y = lane_part.position.y - distance_between_rects / 2;
                
                if (lane_part_index == 1 && partial_rect_at_start && is_right_side_at_start) {
                    // We are at the beginning of a lane that begins from a splitter (its right side)
                    // We should not *stroke* the left side of the previous lane_part,
                    // which is the last part of the previous lane: a splitter.
                    // TODO: now forcing left_middle_y to be right below the top lane_part. We might want to add an extra line/move instead (towards a real splitting point)
                    left_middle_y = previous_lane_part.position.y + previous_lane_part.size.height + radius;
                    skip_top_part_when_border = true;
                }
                
                if (lane_part_index == lane_parts_count - 1 && partial_rect_at_end && is_right_side_at_end) {
                    // We are at the end of a lane that ends in a joiner (its right side)
                    // We should not *stroke* the left side of the current lane_part,
                    // which is the first part of the next lane: a joiner.
                    // TODO: now forcing left_middle_y to be right above the bottom lane_part. We might want to add an extra line/move instead (towards a real joining point)
                    left_middle_y = lane_part.position.y - radius;
                    skip_bottom_part_when_border = true;
                }
                
                push_left_parts(lane_renderer, start_side_point.x, start_side_point.y, 
                                left_middle_y, end_side_point.x, end_side_point.y, 
                                radius, skip_top_part_when_border, skip_bottom_part_when_border);
            }
            
        }
        else {
            // the case lane_part_index == 0 is handled above
        }
        
        previous_lane_part = lane_part;
    }
        
    if (!partial_rect_at_end && !is_splitter_at_end) {
        DirectionalRect2d lane_part = lane_parts[lane_parts_count - 1];
        
        push_left_parts(lane_renderer, lane_part.position.x, lane_part.position.y + lane_part.size.height / 2, 
                lane_part.position.y + lane_part.size.height * 3 / 4, // TODO: 3 / 4 as middleY (we shouldnt use push_left_parts, we only draw a straight (half) lane_part)
                lane_part.position.x, lane_part.position.y + lane_part.size.height, radius);
    }
    
    // Right side (bottom to top)
    if (!partial_rect_at_end && !is_splitter_at_end) {
        DirectionalRect2d lane_part = lane_parts[lane_parts_count - 1];
        
        push_right_parts(lane_renderer, lane_part.position.x + lane_part.size.width, lane_part.position.y + lane_part.size.height / 2, 
                  lane_part.position.y + lane_part.size.height * 3 / 4, // TODO: 3 / 4 as middleY (we shouldnt use push_right_parts, we only draw a straight (half) lane_part)
                  lane_part.position.x + lane_part.size.width, lane_part.position.y + lane_part.size.height, radius);
    }
    
    previous_lane_part = empty_lane_part;
    for (i32 lane_part_index = lane_parts_count - 1; lane_part_index >= 0; lane_part_index--) {
        DirectionalRect2d lane_part = lane_parts[lane_part_index];
        
        b32 is_right = true;
        
        if (lane_part_index < lane_parts_count - 1) {
            
            if (lane_part.direction != Direction_TopToBottom || previous_lane_part.direction != Direction_TopToBottom) {
                
                Pos2d start_side_point = get_half_way_side_point(previous_lane_part, is_right);
                Pos2d end_side_point = get_half_way_side_point(lane_part, is_right);
                
                push_parts_on_one_side_of_corner(lane_renderer, start_side_point.x, start_side_point.y, previous_lane_part.direction, 
                                                                end_side_point.x, end_side_point.y, lane_part.direction, radius, is_right);
            }
            else {
            
                Pos2d start_side_point = get_half_way_side_point(previous_lane_part, is_right);
                Pos2d end_side_point = get_half_way_side_point(lane_part, is_right);
                
                b32 skip_top_part_when_border = false;
                b32 skip_bottom_part_when_border = false;
                
                // TODO: where do we want the right_middle_y to be?
                i32 distance_between_rects = previous_lane_part.position.y - (lane_part.position.y + lane_part.size.height);
                i32 right_middle_y = previous_lane_part.position.y - distance_between_rects / 2;
                
                if (lane_part_index == 0 && partial_rect_at_start && !is_right_side_at_start) {
                    // We are at the beginning of a lane that begins from a splitter (its left side)
                    // We should not *stroke* the right side of the previous lane_part,
                    // which is the last part of the previous lane: a splitter.
                    // TODO: now forcing right_middle_y to be right below the top lane_part. We might want to add an extra line/move instead (towards a real splitting point)
                    right_middle_y = lane_part.position.y + lane_part.size.height + radius;
                    skip_top_part_when_border = true;
                }
                
                if (lane_part_index == lane_parts_count - 2 && partial_rect_at_end && !is_right_side_at_end) {
                    // We are at the end of a lane that ends in a joiner (its left side)
                    // We should not *stroke* the right side of the current lane_part,
                    // which is the first part of the next lane: a joiner.
                    // TODO: now forcing right_middle_y to be right above the bottom lane_part. We might want to add an extra line/move instead (towards a real joining point)
                    right_middle_y = previous_lane_part.position.y - radius;
                    skip_bottom_part_when_border = true;
                }
                
                push_right_parts(lane_renderer, end_side_point.x, end_side_point.y, 
                                 right_middle_y, start_side_point.x, start_side_point.y, 
                                 radius, skip_top_part_when_border, skip_bottom_part_when_border);
            }
        }
        else {
            // the case lane_parts_count - 1 is handled above
        }
                    
        previous_lane_part = lane_part;
    }
    
    if (!partial_rect_at_start && !is_joiner_at_beginning) {
        DirectionalRect2d lane_part = lane_parts[0];
        
        push_right_parts(lane_renderer, lane_part.position.x + lane_part.size.width, lane_part.position.y, 
                         lane_part.position.y + lane_part.size.height / 4, // TODO: 1 / 4 as middleY (we shouldnt use push_right_parts, we only draw a straight (half) lane_part)
                         lane_part.position.x + previous_lane_part.size.width, lane_part.position.y + lane_part.size.height / 2, radius);
    }
    

    // Do the drawing
    
    DrawablePathPart * left_path_parts = (DrawablePathPart *)lane_renderer->left_path_parts.items;
    DrawablePathPart * right_path_parts = (DrawablePathPart *)lane_renderer->right_path_parts.items;
    draw_lane_paths(left_path_parts, lane_renderer->left_path_parts.nr_of_items, 
                    right_path_parts, lane_renderer->right_path_parts.nr_of_items, 
                    lane_style);
    
}

void draw_an_entry(LaneRenderer * lane_renderer, DrawableEntry * drawable_entry)
{
    // FIXME: it's probably a better idea to check whether an entry is on the screen when trying to push it
    
    if (drawable_entry->type == Drawable_RoundedRect)
    {
        DrawableRoundedRect * rounded_rect = (DrawableRoundedRect *)drawable_entry->item_to_draw;
        if (rect_is_inside_screen(rounded_rect->rect))
        {
            draw_rounded_rectangle(rounded_rect->rect, 
                                   rounded_rect->draw_style);
        }
    }
    else if (drawable_entry->type == Drawable_Rect)
    {
        DrawableRect * rect = (DrawableRect *)drawable_entry->item_to_draw;
        if (rect_is_inside_screen(rect->rect))
        {
            draw_rectangle(rect->rect, 
                           rect->draw_style);
        }
    }
    else if (drawable_entry->type == Drawable_Text)
    {
        DrawableText * text = (DrawableText *)drawable_entry->item_to_draw;
        // FIXME: we need the size of the text to draw to determine whether the text is on the screen
        Rect2d text_rect = {};
        text_rect.position = text->position;
        text_rect.size.height = 50; // FIXME: hack!
        text_rect.size.width = text->text.length * 10; // FIXME: hack!
        if (rect_is_inside_screen(text_rect))
        {
            draw_text(text->position, text->text, text->font, text->color);
        }
    }
    else if (drawable_entry->type == Drawable_Lane)
    {
        DrawableLane * lane = (DrawableLane *)drawable_entry->item_to_draw;
    
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
                directional_rect.direction = Direction_TopToBottom; // We assume the first rect is always top-to-bottom
                
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
                directional_rect.direction = Direction_TopToBottom; // We assume the first rect is always top-to-bottom
                
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
            
            b32 partial_rect_at_start = add_rect_at_start;
            b32 partial_rect_at_end = add_rect_at_end;
            
            draw_lane_using_directional_rects(lane_renderer,
                      directional_rects, directional_rects_index, 
                      partial_rect_at_start, lane->is_right_side_at_split, lane->is_joiner_at_beginning,
                      partial_rect_at_end, lane->is_right_side_at_join, lane->is_splitter_at_end,
                      lane->draw_style);
            
        }
    }
}

void draw_entries(LaneRenderer * lane_renderer, DrawableEntry * drawable_entry)
{
    while (drawable_entry)
    {
        draw_an_entry(lane_renderer, drawable_entry);
        
        // If there are childs, first draw them, only after that we draw next entries (siblings)
        if (drawable_entry->first_child_entry)
        {
            draw_entries(lane_renderer, drawable_entry->first_child_entry);
        }
        
        drawable_entry = drawable_entry->next_entry;
    }
}
