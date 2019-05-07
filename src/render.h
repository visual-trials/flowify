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
 
struct DrawStyle
{
    Color4 line_color;
    Color4 fill_color;
    i32 corner_radius;
    i32 line_width;
};

struct DrawStyleEvenOdd
{
    Color4 line_color;
    Color4 even_fill_color;
    Color4 odd_fill_color;
    i32 corner_radius;
    i32 line_width;
};

struct DrawableText
{
    Pos2d position;
    String text;
    Font font;
    Color4 color;
};

struct DrawableRect
{
    Rect2d rect;
    DrawStyle draw_style;
};

struct DrawableRoundedRect
{
    Rect2d rect;
    DrawStyle draw_style;
};

enum PathPartType
{
    PathPart_Move = 0,
    PathPart_Line = 1,
    PathPart_LineWhenBackground = 2,
    PathPart_Arc_DownToLeft = 3,
    PathPart_Arc_DownToRight = 4,
    PathPart_Arc_LeftToUp = 5,
    PathPart_Arc_LeftToDown = 6,
    PathPart_Arc_UpToLeft = 7,
    PathPart_Arc_UpToRight = 8,
    PathPart_Arc_RightToUp = 9,
    PathPart_Arc_RightToDown = 10
};

struct DrawablePathPart
{
    Pos2d position;
    PathPartType part_type;
};

struct DrawableLanePart
{
    Rect2d rect;
    Direction direction;
    
    DrawableLanePart * next_part;
    // TODO: maybe add fill_color (when a part is selected for example)
};

struct DrawableLane
{
    Rect2d bounding_rect; // We can use this to check whether any of the lane-parts is inside the screen (so we know we should draw the lane)
    
    DrawableLanePart * first_part;
    DrawableLanePart * last_part;
    
    // TODO: how to signify that the first/last part should not have a line on either the left or right side?
    
    DrawableLane * splitting_from_lane;
    b32 is_right_side_at_split;
    Pos2d splitting_point;
    
    b32 is_splitter_at_end;
    
    DrawableLane * joining_towards_lane;
    b32 is_right_side_at_join;
    Pos2d joining_point;
    
    b32 is_joiner_at_beginning;
    
    DrawStyle draw_style;
};

enum DrawableType
{
    Drawable_Text,
    Drawable_Rect,
    Drawable_RoundedRect,
    Drawable_Lane
};

struct DrawableEntry
{
    DrawableType type; // Lane / RoundedRect / Rect / Text
    void * item_to_draw;
    
    DrawableEntry * first_child_entry;
    DrawableEntry * last_child_entry;
    
    DrawableEntry * next_entry;
};

struct BasicRenderer
{
    FragmentedMemoryArena draw_arena;
    DrawableEntry * first_drawable_entry;
    DrawableEntry * last_drawable_entry;
    
    DrawableEntry * last_lane_entry;
};

struct LaneRenderer
{
    ConsecutiveDynamicArray left_path_parts;
    ConsecutiveDynamicArray right_path_parts;
};

void init_basic_renderer(BasicRenderer * basic_renderer)
{
    if (!basic_renderer->draw_arena.memory)
    {
        basic_renderer->draw_arena = new_fragmented_memory_arena(&global_memory, (Color4){50,100,50,255}, cstring_to_string("Basic renderer"), true);
    }
    else
    {
        reset_fragmented_memory_arena(&basic_renderer->draw_arena, true);
    }
    basic_renderer->first_drawable_entry = 0;
    basic_renderer->last_drawable_entry = 0;
    
    basic_renderer->last_lane_entry = 0;
}

void init_lane_renderer(LaneRenderer * lane_renderer)
{
    init_consecutive_dynamic_array(&lane_renderer->left_path_parts, sizeof(DrawablePathPart), (Color4){50,100,150,255}, cstring_to_string("Left path parts"));
    init_consecutive_dynamic_array(&lane_renderer->right_path_parts, sizeof(DrawablePathPart), (Color4){50,100,150,255}, cstring_to_string("Right path parts"));
}

void add_child_drawable_entry(DrawableEntry * child_drawable_entry, DrawableEntry * parent_drawable_entry)
{
    if (!parent_drawable_entry->first_child_entry)
    {
        parent_drawable_entry->first_child_entry = child_drawable_entry;
    }
    else
    {
        parent_drawable_entry->last_child_entry->next_entry = child_drawable_entry;
    }
    parent_drawable_entry->last_child_entry = child_drawable_entry;
}

void add_drawable_entry(BasicRenderer * renderer, DrawableEntry * drawable_entry)
{
    if (drawable_entry->type == Drawable_Lane)
    {
        renderer->last_lane_entry = drawable_entry;
    }
    
    if (!renderer->first_drawable_entry)
    {
        renderer->first_drawable_entry = drawable_entry;
    }
    else
    {
        renderer->last_drawable_entry->next_entry = drawable_entry;
    }
    
    renderer->last_drawable_entry = drawable_entry;
}
