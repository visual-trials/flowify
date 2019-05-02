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

struct Rect2d
{
    Pos2d position;
    Size2d size;
};

enum Direction
{
    Direction_TopToBottom,
    Direction_LeftToRight,
    Direction_BottomToTop,
    Direction_RightToLeft
};

struct DirectionalRect2d
{
    Pos2d position;
    Size2d size;
    Direction direction;
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
    rectangles.first.size.width = rectangle.size.width;
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

Rect2d shrink_rect_by_size(Rect2d rect, Size2d size)
{
    Rect2d shrunk_rect = rect;
    
    shrunk_rect.position.x += size.width;
    shrunk_rect.position.y += size.height;
    
    shrunk_rect.size.width -= size.width + size.width;
    shrunk_rect.size.height -= size.height + size.height;
    
    return shrunk_rect;
}
