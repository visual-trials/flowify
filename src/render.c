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
    String * text; // FIXME: there is no need to use a pointer here!
    Font font;
    Color4 color;
};

struct DrawableRect
{
    Rect2d rect;
    Color4 line_color;
    Color4 fill_color;
    i32 line_width;
};

struct DrawableRoundedRect
{
    Rect2d rect;
    i32 radius;
    Color4 line_color;
    Color4 fill_color;
    i32 line_width;
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
    
    // FIXME: make this a DrawStyle!
    i32 bending_radius;
    Color4 line_color;
    Color4 fill_color;
    i32 line_width;
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
    DynamicArray left_path_parts;
    DynamicArray right_path_parts;
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
    init_dynamic_array(&lane_renderer->left_path_parts, sizeof(DrawablePathPart), (Color4){50,100,150,255}, cstring_to_string("Left path parts"));
    init_dynamic_array(&lane_renderer->right_path_parts, sizeof(DrawablePathPart), (Color4){50,100,150,255}, cstring_to_string("Right path parts"));
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

void push_text(BasicRenderer * renderer, Pos2d position, String * text, Font font, Color4 color)
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

void push_rectangle(BasicRenderer * renderer, Rect2d rect, Color4 line_color, Color4 fill_color, i32 line_width)
{
    DrawableEntry * drawable_entry = (DrawableEntry *)push_struct(&renderer->draw_arena, sizeof(DrawableEntry));
    drawable_entry->type = Drawable_Rect;
    drawable_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableRect * drawable_rect = (DrawableRect *)push_struct(&renderer->draw_arena, sizeof(DrawableRect));
    drawable_entry->item_to_draw = drawable_rect;
    
    drawable_rect->rect = rect;
    drawable_rect->line_color = line_color;
    drawable_rect->fill_color = fill_color;
    drawable_rect->line_width = line_width;
    
    add_drawable_entry(renderer, drawable_entry);
}

void push_rounded_rectangle(BasicRenderer * renderer, Rect2d rect, i32 radius, Color4 line_color, Color4 fill_color, i32 line_width)
{
    DrawableEntry * drawable_entry = (DrawableEntry *)push_struct(&renderer->draw_arena, sizeof(DrawableEntry));
    drawable_entry->type = Drawable_RoundedRect;
    drawable_entry->next_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->first_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    drawable_entry->last_child_entry = 0; // TODO: we should let push_struct reset the memory of the struct!
    
    DrawableRoundedRect * drawable_rounded_rect = (DrawableRoundedRect *)push_struct(&renderer->draw_arena, sizeof(DrawableRoundedRect));
    drawable_entry->item_to_draw = drawable_rounded_rect;
    
    drawable_rounded_rect->rect = rect;
    drawable_rounded_rect->radius = radius;
    drawable_rounded_rect->line_color = line_color;
    drawable_rounded_rect->fill_color = fill_color;
    drawable_rounded_rect->line_width = line_width;
    
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
    
    drawable_lane->bending_radius = lane_style.corner_radius;
    drawable_lane->line_color = lane_style.line_color;
    drawable_lane->fill_color = lane_style.fill_color;
    drawable_lane->line_width = lane_style.line_width;
        
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

void push_path_part(LaneRenderer * lane_renderer, DrawablePathPart path_part, b32 is_right)
{
    if (is_right) {
        add_to_array(&lane_renderer->right_path_parts, &path_part);
    }
    else {
        add_to_array(&lane_renderer->left_path_parts, &path_part);
    }
}

void draw_lane_using_directional_rects(LaneRenderer * lane_renderer,
               DirectionalRect2d * lane_parts, i32 lane_parts_count,
               b32 partial_rect_at_start, b32 is_right_side_at_start, 
               b32 partial_rect_at_end, b32 is_right_side_at_end, 
               i32 radius, Color4 line_color, Color4 fill_color, i32 line_width)
{
    
    DrawablePathPart path_part = {};
    
    b32 is_right = false;
    
    path_part.position.x = 300;
    path_part.position.y = 200;
    path_part.part_type = PathPart_Move;
    push_path_part(lane_renderer, path_part, is_right);
    
    path_part.position.x = 200;
    path_part.position.y = 300;
    path_part.part_type = PathPart_Line;
    push_path_part(lane_renderer, path_part, is_right);
    
    DrawablePathPart * left_path_parts = (DrawablePathPart *)lane_renderer->left_path_parts.items;
    DrawablePathPart * right_path_parts = (DrawablePathPart *)lane_renderer->right_path_parts.items;
    draw_lane_paths(left_path_parts, lane_renderer->left_path_parts.nr_of_items, 
                    right_path_parts, lane_renderer->right_path_parts.nr_of_items, 
                    line_color, fill_color, line_width);
    
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
                                   rounded_rect->radius, 
                                   rounded_rect->line_color, 
                                   rounded_rect->fill_color, 
                                   rounded_rect->line_width);
        }
    }
    else if (drawable_entry->type == Drawable_Rect)
    {
        DrawableRect * rect = (DrawableRect *)drawable_entry->item_to_draw;
        if (rect_is_inside_screen(rect->rect))
        {
            draw_rectangle(rect->rect, 
                           rect->line_color, 
                           rect->fill_color, 
                           rect->line_width);
        }
    }
    else if (drawable_entry->type == Drawable_Text)
    {
        DrawableText * text = (DrawableText *)drawable_entry->item_to_draw;
        // FIXME: we need the size of the text to draw to determine whether the text is on the screen
        Rect2d text_rect = {};
        text_rect.position = text->position;
        text_rect.size.height = 50; // FIXME: hack!
        text_rect.size.width = text->text->length * 10; // FIXME: hack!
        if (rect_is_inside_screen(text_rect))
        {
            draw_text(text->position, text->text, text->font, text->color);
        }
    }
    else if (drawable_entry->type == Drawable_Lane)
    {
        DrawableLane * lane = (DrawableLane *)drawable_entry->item_to_draw;
        i32 bending_radius = lane->bending_radius;
        Color4 line_color = lane->line_color;
        Color4 fill_color = lane->fill_color;
        i32 line_width = lane->line_width;
    
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
                directional_rect.direction = lane_part->direction;
                
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
                directional_rect.direction = lane_part->direction;
                
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
            
            draw_lane(directional_rects, directional_rects_index, 
                      add_rect_at_start, lane->is_right_side_at_split, 
                      add_rect_at_end, lane->is_right_side_at_join, 
                      bending_radius, line_color, fill_color, line_width);
                      
            draw_lane_using_directional_rects(lane_renderer,
                      directional_rects, directional_rects_index, 
                      add_rect_at_start, lane->is_right_side_at_split, 
                      add_rect_at_end, lane->is_right_side_at_join, 
                      bending_radius, line_color, fill_color, line_width);
            
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
