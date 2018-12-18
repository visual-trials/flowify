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

struct Rectangle
{
    Pos2d position;
    Size2d size;
};

struct Size2dFloat
{
    f32 width;
    f32 height;
};

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

struct LaneSegment2
{
    LaneSegment top;
    LaneSegment bottom;
    b32 has_valid_bottom_segment;
};

struct LaneSegment3
{
    LaneSegment left;
    LaneSegment right;
    LaneSegment top_or_bottom;
};

LaneSegment lane_segment_from_positions_and_widths(Pos2d top_position, i32 top_width, 
                                                   Pos2d bottom_position, i32 bottom_width)

{
    LaneSegment lane_segment = {};
    
    lane_segment.left_top = top_position;
    
    lane_segment.right_top = top_position;
    lane_segment.right_top.x += top_width;
    
    lane_segment.left_bottom = bottom_position;
    
    lane_segment.right_bottom = bottom_position;
    lane_segment.right_bottom.x += bottom_width;
    
    return lane_segment;
}

Rectangle shrink_rect_by_size(Rectangle rect, Size2d size)
{
    Rectangle shrunk_rect = rect;
    
    shrunk_rect.position.x += size.width;
    shrunk_rect.position.y += size.height;
    
    shrunk_rect.size.width -= size.width + size.width;
    shrunk_rect.size.height -= size.height + size.height;
    
    return shrunk_rect;
}

LaneSegment3 get_3_lane_segments_from_4_rectangles(Rectangle top_or_bottom_rect, 
                                                   b32 is_top_rect, 
                                                   Rectangle left_rect, 
                                                   Rectangle right_rect, 
                                                   Rectangle middle_rect, 
                                                   i32 bending_radius)
{
    LaneSegment3 lane_segments = {};
    
    // TODO: implement this!
    
    return lane_segments;
}

LaneSegment2 get_2_lane_segments_from_3_rectangles(Rectangle top_rect, 
                                                   Rectangle middle_rect, 
                                                   Rectangle bottom_rect,
                                                   i32 bending_radius)
{
    LaneSegment2 lane_segments = {};
    
    LaneSegment top_lane_segment = {};
    LaneSegment bottom_lane_segment = {};
    
    top_lane_segment.left_top.x = middle_rect.position.x;
    top_lane_segment.left_top.y = middle_rect.position.y;
    
    top_lane_segment.right_top.x = middle_rect.position.x + middle_rect.size.width;
    top_lane_segment.right_top.y = middle_rect.position.y;
    
    top_lane_segment.left_bottom.x = middle_rect.position.x;
    top_lane_segment.left_bottom.y = middle_rect.position.y + middle_rect.size.height;
    
    top_lane_segment.right_bottom.x = middle_rect.position.x + middle_rect.size.width;
    top_lane_segment.right_bottom.y = middle_rect.position.y + middle_rect.size.height;
    
    top_lane_segment.bending_radius = bending_radius;
    
    i32 top_rect_bottom_y = top_rect.position.y + top_rect.size.height;
    i32 middle_rect_top_y = middle_rect.position.y;
    
    if (top_rect.size.height >= 0)
    {
        i32 vertical_gap_between_top_and_middle_rects = middle_rect_top_y - top_rect_bottom_y;
        if (vertical_gap_between_top_and_middle_rects < bending_radius + bending_radius)
        {
            // The top of the middle rectangle is very close to (or is touching) the bottom of the top rectangle
            // So determine the new top rectangle bottom-y and new middle rectanhle top-y
            // bottom_rect_bottom_y = middle_rect.position.y - bending_radius - bending_radius;
            middle_rect_top_y = top_rect.position.y + top_rect.size.height + bending_radius + bending_radius;
        }
    }
    top_lane_segment.left_top.y = middle_rect_top_y;
    top_lane_segment.right_top.y = middle_rect_top_y;
    
    if (bottom_rect.size.height < 0)
    {
        lane_segments.has_valid_bottom_segment = false;
    }
    else
    {
        i32 middle_rect_bottom_y = middle_rect.position.y + middle_rect.size.height;
        i32 bottom_rect_top_y = bottom_rect.position.y;
        
        i32 vertical_gap_between_middle_and_bottom_rects = bottom_rect_top_y - middle_rect_bottom_y;
        if (vertical_gap_between_middle_and_bottom_rects < bending_radius + bending_radius)
        {
            // The top of the bottom rectangle is very close to (or is touching) the bottom of the middle rectangle
            // So determine the new middle rectangle bottom-y and new bottom rectanhle top-y
            middle_rect_bottom_y = bottom_rect.position.y - bending_radius - bending_radius;
            bottom_rect_top_y = middle_rect.position.y + middle_rect.size.height + bending_radius + bending_radius;
        }
        
        top_lane_segment.left_bottom.y = middle_rect_bottom_y;
        top_lane_segment.right_bottom.y = middle_rect_bottom_y;
        
    
        bottom_lane_segment.left_top.x = middle_rect.position.x;
        bottom_lane_segment.left_top.y = middle_rect_bottom_y; // middle_rect.position.y + middle_rect.size.height;
        
        bottom_lane_segment.right_top.x = middle_rect.position.x + middle_rect.size.width;
        bottom_lane_segment.right_top.y = middle_rect_bottom_y; // middle_rect.position.y + middle_rect.size.height;
        
        bottom_lane_segment.left_bottom.x = bottom_rect.position.x;
        bottom_lane_segment.left_bottom.y = bottom_rect_top_y; //bottom_rect.position.y;
        
        bottom_lane_segment.right_bottom.x = bottom_rect.position.x + bottom_rect.size.width;
        bottom_lane_segment.right_bottom.y = bottom_rect_top_y; // bottom_rect.position.y;
        
        bottom_lane_segment.bending_radius = bending_radius;

        if (bottom_rect.position.x < middle_rect.position.x)
        {
            bottom_lane_segment.left_middle_y = bottom_lane_segment.left_top.y + bending_radius;
        }
        else
        {
            bottom_lane_segment.left_middle_y = bottom_lane_segment.left_bottom.y - bending_radius;
        }
        
        if (bottom_rect.position.x + bottom_rect.size.width < middle_rect.position.x + middle_rect.size.width)
        {
            bottom_lane_segment.right_middle_y = bottom_lane_segment.right_bottom.y - bending_radius;
        }
        else
        {
            bottom_lane_segment.right_middle_y = bottom_lane_segment.right_top.y + bending_radius;
        }    
        
        lane_segments.has_valid_bottom_segment = true;
    }
    
    lane_segments.top = top_lane_segment;
    lane_segments.bottom = bottom_lane_segment;
    
    return lane_segments;
}

// TODO: rewrite this!
LaneSegment2 get_2_lane_segments_from_3_rectangles(Size2d top_size, 
                                                   Rectangle middle_rect, 
                                                   Size2d bottom_size)
{
    LaneSegment2 lane_segments = {};
    
    Pos2d position = middle_rect.position;
    Size2d size = middle_rect.size;
    
    Pos2d left_top = {};
    Pos2d right_top = {};
    Pos2d left_bottom = {};
    Pos2d right_bottom = {};

    // Top lane segment
    left_top = position;
    right_top = left_top;
    right_top.x += top_size.width;
    
    left_bottom = left_top;
    left_bottom.y += size.height / 2;
    
    right_bottom = left_bottom;
    right_bottom.x += size.width;
                
    lane_segments.top.left_top = left_top;
    lane_segments.top.right_top = right_top;
    lane_segments.top.left_bottom = left_bottom;
    lane_segments.top.right_bottom = right_bottom;
                
    // Bottom lane segment
    
    left_top = left_bottom;
    right_top = right_bottom;
    
    left_bottom = position;
    left_bottom.y += size.height;
    
    right_bottom = left_bottom;
    right_bottom.x += bottom_size.width;
    
    lane_segments.bottom.left_top = left_top;
    lane_segments.bottom.right_top = right_top;
    lane_segments.bottom.left_bottom = left_bottom;
    lane_segments.bottom.right_bottom = right_bottom;
                
    return lane_segments;
}

// TODO: rewrite this!
LaneSegment3 get_3_lane_segments_from_3_rectangles(Rectangle left_rect, 
                                                   Rectangle right_rect, 
                                                   Rectangle top_or_bottom_rect, 
                                                   b32 is_top_rect)
{
    LaneSegment3 lane_segments = {};
    
    Pos2d top_position = {};
    i32 top_width = 0;
    
    Pos2d bottom_position = {};
    i32 bottom_width = 0;
    
    i32 left_width = left_rect.size.width;
    i32 right_width = right_rect.size.width;
    
    // TODO: maybe we should not reverse engineer middle_margin this way
    i32 middle_margin = top_or_bottom_rect.size.width - (left_width + right_width);
    
    if (is_top_rect)
    {
        Rectangle top_rect = top_or_bottom_rect;
        
        Pos2d split_point = top_rect.position;
        split_point.x += left_width + middle_margin / 2;
        split_point.y += top_rect.size.height / 2;
        
        // Top lane segment
        
        top_position = top_rect.position;
        top_width = top_rect.size.width;
        
        bottom_position = top_rect.position;
        bottom_position.y += top_rect.size.height / 2;
        bottom_width = top_rect.size.width;
        
        lane_segments.top_or_bottom = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        
        // Left lane segment

        top_position = top_rect.position;
        top_position.y += top_rect.size.height / 2;
        top_width = split_point.x - top_position.x;
        
        bottom_position = top_rect.position;
        bottom_position.y += top_rect.size.height;
        bottom_width = left_width;

        lane_segments.left = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        
        // Right lane segment
        
        top_position = split_point;
        top_width = right_rect.position.x + right_width - split_point.x;
        
        bottom_position = right_rect.position;
        bottom_width = right_width;
        
        lane_segments.right = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
    }
    else
    {
        Rectangle bottom_rect = top_or_bottom_rect;
        
        Pos2d join_point = bottom_rect.position;
        join_point.x += left_width + middle_margin / 2;
        join_point.y += bottom_rect.size.height / 2;

        // Left lane segment

        top_position = bottom_rect.position;
        top_width = left_width;
        
        bottom_position = bottom_rect.position;
        bottom_position.y += bottom_rect.size.height / 2;
        bottom_width = join_point.x - top_position.x;

        lane_segments.left = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
        
        // Right lane segment
        
        top_position = bottom_rect.position;
        top_position.x += left_width + middle_margin;
        top_width = right_width; // right_position.x + right_width - split_point.x;
        
        bottom_position = join_point;
        bottom_width = right_rect.position.x + right_width - join_point.x;
        
        lane_segments.right = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
                          
        // Bottom lane segment
        
        top_position = bottom_rect.position;
        top_position.y += bottom_rect.size.height / 2;
        top_width = bottom_rect.size.width;
        
        bottom_position = bottom_rect.position;
        bottom_position.y += bottom_rect.size.height;
        bottom_width = bottom_rect.size.width;
        
        lane_segments.top_or_bottom = lane_segment_from_positions_and_widths(top_position, top_width, bottom_position, bottom_width);
    }
    
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

i32 split_string_into_lines(String string, String * lines)
{
    i32 nr_of_lines;
    
    i32 file_line_index = 0;
    lines[file_line_index].data = string.data;
    lines[file_line_index].length = 0;
    
    i32 position = 0;
    i32 start_of_line = 0;
    while (position < string.length)
    {
        char ch = string.data[position++];
        
        if (ch == '\n')
        {
            // TODO: somewhere we need to remove the newline from either start_of_line or length!
            lines[file_line_index].length = (position - 1) - start_of_line; // the -1 is because we do not include the newline to the line-text
            start_of_line = position;
            
            // FIXME: limit to length of lines[]!
            file_line_index++;
            
            lines[file_line_index].data = (u8 *)((i32)string.data + start_of_line);
            lines[file_line_index].length = 0;
        }
    }
    lines[file_line_index].length = position - start_of_line;
    nr_of_lines = file_line_index + 1;
    
    return nr_of_lines;
}
