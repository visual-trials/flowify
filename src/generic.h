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
 
#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32     b32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

struct Color4
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

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
    
    i32 gap_needed = bending_radius + bending_radius + bending_radius + bending_radius;
    if (vertical_gap_between_lines < gap_needed)
    {
        i32 extra_vertical_gap_needed = gap_needed - vertical_gap_between_lines;
        i32 extra_vertical_gap_needed_top_line = (i32)((f32)extra_vertical_gap_needed / (f32)2);
        i32 extra_vertical_gap_needed_bottom_line = extra_vertical_gap_needed - extra_vertical_gap_needed_top_line;
        
        if (move_down)
        {
            // We are moving the bottom line downwards. (btw this means our rect is downwards).
            horizontal_lines.bottom_line.position.y += extra_vertical_gap_needed_bottom_line;
            
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
            horizontal_lines.top_line.position.y -= extra_vertical_gap_needed_top_line;
            
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

#define MAX_LENGTH_SHORT_STRING 100

struct ShortString
{
    u8 data[MAX_LENGTH_SHORT_STRING];
    i32 length;
};

struct String
{
    u8 * data;
    i32 length;
};

// TODO: can we make one common array_length here?
inline i32 array_length(i32 * array)
{
    return sizeof(array)/sizeof(array[0]);
}

inline i32 array_length(u8 * array)
{
    return sizeof(array)/sizeof(array[0]);
}

i32 cstring_length(const char * cstring)
{
    i32 count = 0;
    while (*cstring++)
    {
        count++;
    }
    return count;
}

void append_string(ShortString * dest, ShortString * src)
{
    i32 nr_of_characters_to_append = src->length;
    if (dest->length + nr_of_characters_to_append > MAX_LENGTH_SHORT_STRING)
    {
        nr_of_characters_to_append = MAX_LENGTH_SHORT_STRING - dest->length;
    }
    
    for (i32 i = 0; i < nr_of_characters_to_append; i++)
    {
        dest->data[dest->length + i] = src->data[i];
    }
    dest->length = dest->length + nr_of_characters_to_append;
}

// TODO: we don't need a pointer to the source here. Better not to take the pointer, we don't want to change the source anyway!
void append_string(String * dest, String * src)
{
    for (i32 i = 0; i < src->length; i++)
    {
        dest->data[dest->length + i] = src->data[i];
    }
    dest->length = dest->length + src->length;
}

b32 equals(String string1, String string2)
{
    if (string1.length != string2.length)
    {
        return false;
    }
    
    b32 equals = true;
    for (i32 i = 0; i < string1.length; i++)
    {
        if (string1.data[i] != string2.data[i])
        {
            return false;
        }
    }
    return true;
}

b32 equals(String string1, const char * cstring2)
{
    i32 cstring2_length = cstring_length(cstring2);
    
    if (string1.length != cstring2_length)
    {
        return false;
    }
    
    b32 equals = true;
    for (i32 i = 0; i < string1.length; i++)
    {
        if (string1.data[i] != cstring2[i])
        {
            return false;
        }
    }
    return true;
}

void copy_string(ShortString * src, ShortString * dest)
{
    for (i32 i = 0; i < src->length; i++)
    {
        dest->data[i] = src->data[i];
    }
    dest->length = src->length;
}

i32 copy_cstring(const char * src, u8 * dest, i32 max_length = -1)
{
    i32 count = 0;
    while (char ch = *src++)
    {
        if (max_length >= 0 && count >= max_length)
        {
            break;
        }
        *dest++ = ch;
        count++;
    }
    *dest = 0;
    return count;
}

ShortString * copy_cstring_to_short_string(const char * src, ShortString * dest)
{
    i32 length = copy_cstring(src, dest->data, MAX_LENGTH_SHORT_STRING);
    dest->length = length;
    
    return dest;
}

String * copy_cstring_to_string(const char * src, String * dest)
{
    i32 length = copy_cstring(src, dest->data);
    dest->length = length;
    
    return dest;
}

ShortString * copy_char_to_string(char ch, ShortString * dest)
{
    dest->data[0] = ch;
    dest->length = 1;
    
    return dest;
}

ShortString * int_to_string(i32 number, ShortString * decimal_string)
{
    if (number == 0)
    {
        decimal_string->data[0] = '0';
        decimal_string->length = 1;
        return decimal_string;
    }
    
    i32 negative_digit_offset = 0;
    if (number < 0)
    {
        negative_digit_offset = 1;  // the '-' takes one character, so everything moves one character to the right
        number = -number;
        decimal_string->data[0] = '-';
    }
    
    i32 left_over = number;
    i32 nr_of_digits = 0;
    while (left_over > 0)
    {
        i32 decimal_digit = left_over % 10;
        left_over = (left_over - decimal_digit) / 10;
        nr_of_digits++;
    }
    
    left_over = number;
    i32 digit_index = nr_of_digits - 1;
    while (left_over > 0)
    {
        i32 decimal_digit = left_over % 10;
        
        decimal_string->data[negative_digit_offset + digit_index] = '0' + decimal_digit;
        
        left_over = (left_over - decimal_digit) / 10;
        digit_index--;
    }
    decimal_string->length = negative_digit_offset + nr_of_digits;
    return decimal_string;
}

ShortString * float_to_string(f32 number, ShortString * decimal_string)
{
    if (number == 0)
    {
        decimal_string->data[0] = '0';
        decimal_string->length = 1;
        return decimal_string;
    }
    
    i32 negative_digit_offset = 0;
    if (number < 0)
    {
        negative_digit_offset = 1;  // the '-' takes one character, so everything moves one character to the right
        number = -number;
        decimal_string->data[0] = '-';
        decimal_string->length = 1;
    }
    
    i32 number_left_part = (i32)number;
    f32 number_float_right_part = number - (f32)number_left_part;
    i32 number_right_part = number_float_right_part * 10000000; // TODO: only the 7 digits are processed after the period
    
    ShortString string_left_part = {};
    int_to_string(number_left_part, &string_left_part);

    ShortString period = {};
    copy_char_to_string('.', &period);
    
    ShortString string_right_part = {};
    int_to_string(number_right_part, &string_right_part);
    
    append_string(decimal_string, &string_left_part);
    append_string(decimal_string, &period);
    append_string(decimal_string, &string_right_part);
    
    return decimal_string;
}
