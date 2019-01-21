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
 
struct Pos2d
{
    i32 x;
    i32 y;
};

struct Size2d
{
    i32 width;
    i32 height;
};

struct HorLine
{
   Pos2d position;
   i32 width;
};

struct VertLine
{
   Pos2d position;
   i32 height;
};

struct HorLine2
{
   HorLine top_line;
   HorLine bottom_line;
};

struct Rect2d
{
    Pos2d position;
    Size2d size;
};

struct Margins
{
    i32 left;
    i32 right;
    i32 top;
    i32 bottom;
};

struct Rectangle2
{
    Rect2d first;
    Rect2d second;
};

struct Size2dFloat
{
    f32 width;
    f32 height;
};

// TODO: use this instead
struct LaneSegment_New
{
    HorLine top_line;
    HorLine bottom_line;
    
    i32 left_bend_y;
    i32 right_bend_y;
    
    i32 bending_radius;
};

// TODO: deprecate this
struct LaneSegment
{
    Pos2d left_top;
    Pos2d right_top;
    
    Pos2d left_bottom;
    Pos2d right_bottom;
    
    i32 left_middle_y;
    i32 right_middle_y;
    
    i32 bending_radius;
};

struct LaneSegment3
{
    LaneSegment top;
    b32 has_valid_top_segment;
    
    LaneSegment middle;
    
    LaneSegment bottom;
    b32 has_valid_bottom_segment;
};

struct LaneSegment3LR
{
    LaneSegment left;
    LaneSegment right;
    LaneSegment top_or_bottom;
    b32 has_valid_bottom_segment;
};

HorLine get_top_line_from_rect(Rect2d rect)
{
    HorLine top_line = {};
    
    top_line.position = rect.position;
    top_line.width = rect.size.width;
    
    return top_line;
}

HorLine get_bottom_line_from_rect(Rect2d rect)
{
    HorLine bottom_line = {};
    
    bottom_line.position.x = rect.position.x;
    bottom_line.position.y = rect.position.y + rect.size.height;
    bottom_line.width = rect.size.width;
    
    return bottom_line;
}

VertLine get_left_line_from_rect(Rect2d rect)
{
    VertLine left_line = {};
    
    left_line.position = rect.position;
    left_line.height = rect.size.height;
    
    return left_line;
}

VertLine get_right_line_from_rect(Rect2d rect)
{
    VertLine right_line = {};
    
    right_line.position.x = rect.position.x + rect.size.width;
    right_line.position.y = rect.position.y;
    right_line.height = rect.size.height;
    
    return right_line;
}

Rect2d shrink_rect_by_size(Rect2d rect, Size2d size)
{
    Rect2d shrunk_rect = rect;
    
    shrunk_rect.position.x += size.width;
    shrunk_rect.position.y += size.height;
    
    shrunk_rect.size.width -= size.width + size.width;
    shrunk_rect.size.height -= size.height + size.height;
    
    return shrunk_rect;
}

HorLine2 get_hor_lines_between_rects(Rect2d top_rect, Rect2d bottom_rect)
{
    HorLine2 horizontal_lines = {};
    
    horizontal_lines.top_line = get_bottom_line_from_rect(top_rect);
    horizontal_lines.bottom_line = get_top_line_from_rect(bottom_rect);
    
    return horizontal_lines;
}

HorLine2 widen_horizontal_lines(HorLine2 horizontal_lines, i32 bending_radius, b32 move_down)
{
    i32 vertical_gap_between_lines = horizontal_lines.bottom_line.position.y - horizontal_lines.top_line.position.y;
    
    i32 gap_needed = bending_radius + bending_radius;
    if (vertical_gap_between_lines < gap_needed)
    {
        i32 extra_vertical_gap_needed = gap_needed - vertical_gap_between_lines;
        
        if (move_down)
        {
            // We are moving the bottom line downwards. (btw this means our rect is downwards).
            horizontal_lines.bottom_line.position.y += extra_vertical_gap_needed;
            
            // This also means we are *not* moving the top line upwards, so we have to move it to the side(s)
            if (horizontal_lines.bottom_line.position.x + horizontal_lines.bottom_line.width > horizontal_lines.top_line.position.x + horizontal_lines.top_line.width)
            {
                // The right side of the bottom line is to the right of the right side of the of the top line. This means the bend is above the rect.
                // So we should not bend. Meaning we should keep a straight line towards the top of the rect.
                horizontal_lines.top_line.width += (horizontal_lines.bottom_line.position.x + horizontal_lines.bottom_line.width) - (horizontal_lines.top_line.position.x + horizontal_lines.top_line.width);
            }
            
            if (horizontal_lines.bottom_line.position.x < horizontal_lines.top_line.position.x)
            {
                // The left side of the bottom line is to the left of the left side of the top line. This means the bend is above the rect.
                // So we should not bend. Meaning we should keep a straight line towards the top of the rect.
                horizontal_lines.top_line.width += horizontal_lines.top_line.position.x - horizontal_lines.bottom_line.position.x;
                horizontal_lines.top_line.position.x = horizontal_lines.bottom_line.position.x;
            }
        }
        else
        {
            // We are moving the top line upwards. (btw this means our rect is upwards).
            horizontal_lines.top_line.position.y -= extra_vertical_gap_needed;
            
            // This also means we are *not* moving the bottom line downwards, so we have to move it to the side(s)
            if (horizontal_lines.top_line.position.x + horizontal_lines.top_line.width > horizontal_lines.bottom_line.position.x + horizontal_lines.bottom_line.width)
            {
                // The right side of the top line is to the right of the right side of the of the bottom line. This means the bend is below the rect.
                // So we should not bend. Meaning we should keep a straight line towards the bottom of the rect.
                horizontal_lines.bottom_line.width += (horizontal_lines.top_line.position.x + horizontal_lines.top_line.width) - (horizontal_lines.bottom_line.position.x + horizontal_lines.bottom_line.width);
            }
            
            if (horizontal_lines.top_line.position.x < horizontal_lines.bottom_line.position.x)
            {
                // The left side of the top line is to the left of the left side of the bottom line. This means the bend is below the rect.
                // So we should not bend. Meaning we should keep a straight line towards the bottom of the rect.
                horizontal_lines.bottom_line.width += horizontal_lines.bottom_line.position.x - horizontal_lines.top_line.position.x;
                horizontal_lines.bottom_line.position.x = horizontal_lines.top_line.position.x;
            }
            
        }
    }
    
    return horizontal_lines;
}

LaneSegment lane_segment_from_horizontal_lines(HorLine2 horizontal_lines, b32 most_room_left, b32 most_room_right, i32 bending_radius)
{
    LaneSegment lane_segment = {};
    
    HorLine top_line = horizontal_lines.top_line;
    HorLine bottom_line = horizontal_lines.bottom_line;
  
    lane_segment.left_top = top_line.position;
    
    lane_segment.right_top = top_line.position;
    lane_segment.right_top.x += top_line.width;
    
    lane_segment.left_bottom = bottom_line.position;
    
    lane_segment.right_bottom = bottom_line.position;
    lane_segment.right_bottom.x += bottom_line.width;

    lane_segment.bending_radius = bending_radius;

    if (most_room_left)
    {
        if (bottom_line.position.x < top_line.position.x)
        {
            lane_segment.left_middle_y = lane_segment.left_top.y + bending_radius;
        }
        else
        {
            lane_segment.left_middle_y = lane_segment.left_bottom.y - bending_radius;
        }
    }
    else
    {
        if (bottom_line.position.x < top_line.position.x)
        {
            lane_segment.left_middle_y = lane_segment.left_bottom.y - bending_radius;
        }
        else
        {
            lane_segment.left_middle_y = lane_segment.left_top.y + bending_radius;
        }
    }
        
    if (most_room_right)
    {
        if (bottom_line.position.x + bottom_line.width < top_line.position.x + top_line.width)
        {
            lane_segment.right_middle_y = lane_segment.right_bottom.y - bending_radius;
        }
        else
        {
            lane_segment.right_middle_y = lane_segment.right_top.y + bending_radius;
        }
    }
    else
    {
        if (bottom_line.position.x + bottom_line.width < top_line.position.x + top_line.width)
        {
            lane_segment.right_middle_y = lane_segment.right_top.y + bending_radius;
        }
        else
        {
            lane_segment.right_middle_y = lane_segment.right_bottom.y - bending_radius;
        }
    }    
  
    return lane_segment;
}

LaneSegment3LR get_3_lane_segments_from_4_rectangles(Rect2d top_or_bottom_rect, 
                                                     b32 is_top_rect, 
                                                     Rect2d middle_rect, 
                                                     Rect2d left_rect, 
                                                     Rect2d right_rect, 
                                                     i32 bending_radius)
{
    LaneSegment3LR lane_segments = {};
    
    if (is_top_rect)
    {
        Rect2d top_rect = top_or_bottom_rect;

        HorLine2 lines_between_top_and_middle_rects = get_hor_lines_between_rects(top_rect, middle_rect);
        
        // TODO: is there a better way of saying that the top_rect is invalid?
        if (top_rect.size.height >= 0)
        {
            b32 move_down = true;
            lines_between_top_and_middle_rects = widen_horizontal_lines(lines_between_top_and_middle_rects, bending_radius, move_down);
        }
        
        HorLine left_plus_right_line = {};
        HorLine left_line = {};
        HorLine right_line = {};
        
        {
            Pos2d split_point = {};
            
            i32 lowest_y = left_rect.position.y;
            if (right_rect.position.y < lowest_y)
            {
                lowest_y = right_rect.position.y;
            }
            i32 horizontal_distance = right_rect.position.x - (left_rect.position.x + left_rect.size.width);
            i32 middle_x = right_rect.position.x - (i32)((f32)horizontal_distance / (f32)2);
            
            split_point.x = middle_x;
            split_point.y = lowest_y - bending_radius - bending_radius;
            
            left_plus_right_line.position.x = left_rect.position.x;
            left_plus_right_line.position.y = split_point.y;
            left_plus_right_line.width = right_rect.position.x + right_rect.size.width - left_rect.position.x;
            
            left_line.position.x = left_rect.position.x;
            left_line.position.y = split_point.y;
            left_line.width = split_point.x - left_rect.position.x;
            
            right_line.position.x = split_point.x;
            right_line.position.y = split_point.y;
            right_line.width = right_rect.position.x + right_rect.size.width - split_point.x;
        }
        
        HorLine2 top_segment_horizontal_lines = {};
        HorLine2 left_bottom_segment_horizontal_lines = {};
        HorLine2 right_bottom_segment_horizontal_lines = {};
        
        top_segment_horizontal_lines.top_line = get_bottom_line_from_rect(middle_rect);
        top_segment_horizontal_lines.bottom_line = left_plus_right_line;
        
        left_bottom_segment_horizontal_lines.top_line = left_line;
        left_bottom_segment_horizontal_lines.bottom_line = get_top_line_from_rect(left_rect);
        
        right_bottom_segment_horizontal_lines.top_line = right_line;
        right_bottom_segment_horizontal_lines.bottom_line = get_top_line_from_rect(right_rect);
    
        // Top lane segment
        lane_segments.top_or_bottom = lane_segment_from_horizontal_lines(top_segment_horizontal_lines, true, true, bending_radius);
        
        // Left bottom lane segment
        lane_segments.left = lane_segment_from_horizontal_lines(left_bottom_segment_horizontal_lines, true, false, bending_radius);
        
        // Right bottom lane segment
        lane_segments.right = lane_segment_from_horizontal_lines(right_bottom_segment_horizontal_lines, false, true, bending_radius);
    }
    else
    {
        Rect2d bottom_rect = top_or_bottom_rect;
        
        HorLine left_plus_right_line = {};
        HorLine left_line = {};
        HorLine right_line = {};
        
        {
            Pos2d join_point = {};
            
            i32 highest_y = left_rect.position.y + left_rect.size.height;
            if (right_rect.position.y + right_rect.size.height > highest_y)
            {
                highest_y = right_rect.position.y + right_rect.size.height;
            }
            i32 horizontal_distance = right_rect.position.x - (left_rect.position.x + left_rect.size.width);
            i32 middle_x = right_rect.position.x - (i32)((f32)horizontal_distance / (f32)2);
            
            join_point.x = middle_x;
            join_point.y = highest_y + bending_radius + bending_radius;
            
            left_plus_right_line.position.x = left_rect.position.x;
            left_plus_right_line.position.y = join_point.y;
            left_plus_right_line.width = right_rect.position.x + right_rect.size.width - left_rect.position.x;
            
            left_line.position.x = left_rect.position.x;
            left_line.position.y = join_point.y;
            left_line.width = join_point.x - left_rect.position.x;
            
            right_line.position.x = join_point.x;
            right_line.position.y = join_point.y;
            right_line.width = right_rect.position.x + right_rect.size.width - join_point.x;
        }
        
        HorLine2 lines_between_middle_and_bottom_rects = get_hor_lines_between_rects(middle_rect, bottom_rect);
        
        // TODO: is there a better way of saying that the bottom_rect is invalid?
        if (bottom_rect.size.height >= 0)
        {
            b32 move_down = false;
            lines_between_middle_and_bottom_rects = widen_horizontal_lines(lines_between_middle_and_bottom_rects, bending_radius, move_down);
            lane_segments.has_valid_bottom_segment = true;
        }
        else
        {
            // If there is no bottom rect, then we should not draw a bend towards it. Meaning: no bottom segment
            lane_segments.has_valid_bottom_segment = false;
        }
        
        HorLine2 left_top_segment_horizontal_lines = {};
        HorLine2 right_top_segment_horizontal_lines = {};
        HorLine2 bottom_segment_horizontal_lines = {};
        
        bottom_segment_horizontal_lines.top_line = left_plus_right_line;
        bottom_segment_horizontal_lines.bottom_line = get_top_line_from_rect(middle_rect);
        
        left_top_segment_horizontal_lines.top_line = get_bottom_line_from_rect(left_rect);
        left_top_segment_horizontal_lines.bottom_line = left_line;
        
        right_top_segment_horizontal_lines.top_line = get_bottom_line_from_rect(right_rect);
        right_top_segment_horizontal_lines.bottom_line = right_line;
        
        // Left top lane segment
        lane_segments.left = lane_segment_from_horizontal_lines(left_top_segment_horizontal_lines, true, false, bending_radius);
        
        // Right top lane segment
        lane_segments.right = lane_segment_from_horizontal_lines(right_top_segment_horizontal_lines, false, true, bending_radius);
        
        // Bottom lane segment
        lane_segments.top_or_bottom = lane_segment_from_horizontal_lines(bottom_segment_horizontal_lines, true, true, bending_radius);
    }
    
    return lane_segments;
}

LaneSegment3 get_3_lane_segments_from_3_rectangles(Rect2d top_rect, 
                                                   Rect2d middle_rect, 
                                                   Rect2d bottom_rect,
                                                   i32 bending_radius)
{
    LaneSegment3 lane_segments = {};
    
    HorLine2 lines_between_top_and_middle_rects = get_hor_lines_between_rects(top_rect, middle_rect);
    HorLine2 lines_between_middle_and_bottom_rects = get_hor_lines_between_rects(middle_rect, bottom_rect);

    // TODO: is there a better way of saying that the top_rect is invalid?
    if (top_rect.size.height >= 0)
    {
        // FIXME: shouldn't this be false?
        b32 move_down = true;
        lines_between_top_and_middle_rects = widen_horizontal_lines(lines_between_top_and_middle_rects, bending_radius, move_down);
        lane_segments.has_valid_top_segment = true;
    }
    else
    {
        lane_segments.has_valid_top_segment = false;
    }
    
    // TODO: is there a better way of saying that the bottom_rect is invalid?
    if (bottom_rect.size.height >= 0)
    {
        // FIXME: shouldn't this be true?
        b32 move_down = false;
        lines_between_middle_and_bottom_rects = widen_horizontal_lines(lines_between_middle_and_bottom_rects, bending_radius, move_down);
        lane_segments.has_valid_bottom_segment = true;
    }
    else
    {
        // If there is no bottom rect, then we should not draw a bend towards it. Meaning: no bottom segment
        lane_segments.has_valid_bottom_segment = false;
    }
        
    HorLine2 top_segment_horizontal_lines = {};
    HorLine2 middle_segment_horizontal_lines = {};
    HorLine2 bottom_segment_horizontal_lines = {};
    
    top_segment_horizontal_lines.top_line = lines_between_top_and_middle_rects.top_line;
    top_segment_horizontal_lines.bottom_line = lines_between_top_and_middle_rects.bottom_line;
    
    middle_segment_horizontal_lines.top_line = lines_between_top_and_middle_rects.bottom_line;
    middle_segment_horizontal_lines.bottom_line = lines_between_middle_and_bottom_rects.top_line;
    
    bottom_segment_horizontal_lines.top_line = lines_between_middle_and_bottom_rects.top_line;
    bottom_segment_horizontal_lines.bottom_line = lines_between_middle_and_bottom_rects.bottom_line;
    
    // Top lane segment
    lane_segments.top = lane_segment_from_horizontal_lines(top_segment_horizontal_lines, true, true, bending_radius);
    
    // Middle lane segment
    lane_segments.middle = lane_segment_from_horizontal_lines(middle_segment_horizontal_lines, true, true, bending_radius);
    
    // Bottom lane segment
    lane_segments.bottom = lane_segment_from_horizontal_lines(bottom_segment_horizontal_lines, true, true, bending_radius);
                                                                         
    return lane_segments;
}

inline Pos2d add_size_to_position(Pos2d position, Size2d size)
{
    position.x += size.width;
    position.y += size.height;
    return position;
}

inline Pos2d add_position_to_position(Pos2d position, Pos2d added_position)
{
    position.x += added_position.x;
    position.y += added_position.y;
    return position;
}

Rectangle2 split_rect_horizontally(Rect2d rectangle, i32 left_width, i32 middle_margin = 0)
{
    Rectangle2 rectangles = {};
    
    rectangles.first.position.x = rectangle.position.x;
    rectangles.first.position.y = rectangle.position.y;
    rectangles.first.size.height = rectangle.size.height;
    rectangles.first.size.width = left_width;
    
    rectangles.second.position.x = rectangle.position.x + left_width + middle_margin;
    rectangles.second.position.y = rectangle.position.y;
    rectangles.second.size.height = rectangle.size.height;
    rectangles.second.size.width = rectangle.size.width - left_width - middle_margin;
    
    return rectangles;
}

Rectangle2 split_rect_horizontally_fraction(Rect2d rectangle, f32 left_fraction, i32 middle_margin = 0)
{
    i32 left_width = rectangle.size.width * left_fraction;
    return split_rect_horizontally(rectangle, left_width, middle_margin);
}

Rectangle2 split_rect_vertically(Rect2d rectangle, i32 top_height, i32 middle_margin = 0)
{
    Rectangle2 rectangles = {};
    
    rectangles.first.position.x = rectangle.position.x;
    rectangles.first.position.y = rectangle.position.y;
    rectangles.first.size.width = rectangle.size.height;
    rectangles.first.size.height = top_height;
    
    rectangles.second.position.x = rectangle.position.x;
    rectangles.second.position.y = rectangle.position.y + top_height + middle_margin;
    rectangles.second.size.width = rectangle.size.width;
    rectangles.second.size.height = rectangle.size.height - top_height - middle_margin;
    
    return rectangles;
}

Rectangle2 split_rect_vertically_fraction(Rect2d rectangle, f32 top_fraction, i32 middle_margin = 0)
{
    i32 top_height = rectangle.size.width * top_fraction;
    return split_rect_vertically(rectangle, top_height, middle_margin);
}

Rect2d shrink_rect_by_margins(Rect2d rectangle, Margins margins)
{
    rectangle.size.width -= margins.left + margins.right;
    rectangle.size.height -= margins.top + margins.bottom;
    
    rectangle.position.x += margins.left;
    rectangle.position.y += margins.top;
    
    return rectangle;
}