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
};

struct LaneRenderer
{
    ConsecutiveMemoryArena left_path_arena;
    DrawablePathPart * left_lane_parts;
    i32 nr_of_left_path_parts;
    
    ConsecutiveMemoryArena right_path_arena;
    DrawablePathPart * right_lane_parts;
    i32 nr_of_right_path_parts;
};

void init_basic_renderer(BasicRenderer * basic_renderer)
{
    // FIXME: implement this!
}

void init_lane_renderer(LaneRenderer * lane_renderer)
{
    // FIXME: implement this!
}

void push_path_part(LaneRenderer * lane_renderer, DrawablePathPart path_part, b32 is_right)
{
    if (is_right) {
        // FIXME: rightPath.push({"type": type, "x": x, "y": y})
    }
    else {
        // FIXME: leftPath.push({"type": type, "x": x, "y": y})
    }
}

void draw_an_entry(DrawableEntry * drawable_entry)
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
                      
            DrawablePathPart left_path_parts_index[2];
            i32 left_path_parts_count = 2;
            
            left_path_parts_index[0].position.x = 200;
            left_path_parts_index[0].position.y = 200;
            left_path_parts_index[0].part_type = PathPart_Move;
            
            left_path_parts_index[1].position.x = 200;
            left_path_parts_index[1].position.y = 300;
            left_path_parts_index[1].part_type = PathPart_Line;
            
            DrawablePathPart right_path_parts_index[2];
            i32 right_path_parts_count = 0;
            
            draw_lane_paths(left_path_parts_index, left_path_parts_count, right_path_parts_index, right_path_parts_count, 
                            line_color, fill_color, line_width);
        }
    }
}

void draw_entries(DrawableEntry * drawable_entry)
{
    while (drawable_entry)
    {
        draw_an_entry(drawable_entry);
        
        // If there are childs, first draw them, only after that we draw next entries (siblings)
        if (drawable_entry->first_child_entry)
        {
            draw_entries(drawable_entry->first_child_entry);
        }
        
        drawable_entry = drawable_entry->next_entry;
    }
}
