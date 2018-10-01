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
 
#if BUILD_FOR_NATIVE_PLATFORM
    #include "render_win32.cpp"
#else
    #include "render_js.cpp"
#endif

#include "input.cpp"

// FIXME: mouse position should also be a pos_2d, right?

struct pos_2d
{
    i32 x;
    i32 y;
};

struct size_2d
{
    i32 width;
    i32 height;
};

struct entity
{
    // TODO: shape
    pos_2d  pos;
    size_2d size;
    color4  line_color;
    color4  fill_color;
    i32     line_width;
};

/*
entity rectangle[1];
i32 nr_of_rectangles = 0;
b32 rectangle_is_selected = 0;
i32 selected_rectangle_index = 0;
*/

/*
i32 last_x;
i32 last_y;
i32 last_width;
i32 last_height;
color4 last_line_color;
color4 last_fill_color;
i32 last_line_width;
b32 has_position;
*/

struct world_data
{
    entity entities[10];
    i32 nr_of_entities;
    
    i32 increment;
};

world_data allocated_world;  // FIXME: allocate this properly!
world_data * world;

extern "C" {
    void init_world()
    {
        world = &allocated_world;  // FIXME: allocate this properly!
        
        world->increment = 0;
    
        world->nr_of_entities = 0;
    }
    
    void update_frame()
    {
        world->increment++;
        
    }
    
    void render_frame()
    {
        i32 increment = world->increment;
        
        i32 nr_of_entities = world->nr_of_entities;
        entity * entities = world->entities;
        
        color4 line_color = {};
        color4 fill_color = {};
        
        line_color.r = 255;
        line_color.g = 0;
        line_color.b = 0;
        line_color.a = 255;
        
        fill_color.r = 255;
        fill_color.g = 255;
        fill_color.b = 0;
        fill_color.a = 255;
        
        int line_width = 5;
        
        draw_rectangle(200, 50, 40, 40, line_color, fill_color, 3);
        
        line_color.r = 0;
        line_color.g = 255;
        line_color.b = 0;
        line_color.a = 255;
        
        fill_color.r = 0;
        fill_color.g = 0;
        fill_color.b = 255;
        fill_color.a = 128;
        
        int offset;
        if (increment % 512 < 256) {
            offset = increment % 256;
        }
        else {
            offset = 256 - (increment % 256);
        }
        draw_rectangle(offset + 10, 10, 100, 100, line_color, fill_color, line_width);
        draw_rectangle(10, offset + 10, 100, 100, line_color, fill_color, line_width);
        
        // Mouse driven draws
        
        mouse_input mouse = global_input.mouse;
        
        if (mouse.left_mouse_button_has_gone_down)
        {
            world->nr_of_entities = 1;
            
            entity * current_entity = entities + (world->nr_of_entities - 1);
            
            current_entity->line_color.r = 50;
            current_entity->line_color.g = 50;
            current_entity->line_color.b = 50;
            current_entity->line_color.a = 255;
            
            current_entity->fill_color.r = 240;
            current_entity->fill_color.g = 200;
            current_entity->fill_color.b = 255;
            current_entity->fill_color.a = 255;
            
            current_entity->line_width = 2;
            
            current_entity->size.width = 150;
            current_entity->size.height = 150;
            
            current_entity->pos.x = mouse.mouse_position_left;
            current_entity->pos.y = mouse.mouse_position_top;
            
        }
        if (mouse.left_mouse_button_is_down)
        {
            entity * current_entity = entities + (world->nr_of_entities - 1);
            
            current_entity->pos.x = mouse.mouse_position_left;
            current_entity->pos.y = mouse.mouse_position_top;
        }
        
        if (world->nr_of_entities == 1)
        {
            entity * current_entity = entities + (world->nr_of_entities - 1);
            
            draw_rectangle(current_entity->pos.x, current_entity->pos.y, 
                           current_entity->size.width, current_entity->size.height, 
                           current_entity->line_color, current_entity->fill_color, 
                           current_entity->line_width);
        }
        
        /*
        if (mouse.left_mouse_button_has_gone_down)
        {
            nr_of_rectangles = 1;
            
            i32 current_rectangle_index = nr_of_rectangles - 1;
            entity * current_rectangle = &rectangle[current_rectangle_index];

            current_rectangle->line_color.r = 50;
            current_rectangle->line_color.g = 50;
            current_rectangle->line_color.b = 50;
            current_rectangle->line_color.a = 255;
            
            current_rectangle->fill_color.r = 240;
            current_rectangle->fill_color.g = 200;
            current_rectangle->fill_color.b = 255;
            current_rectangle->fill_color.a = 255;
            
            current_rectangle->line_width = 2;
            
            current_rectangle->pos.x = mouse.mouse_position_left;
            current_rectangle->pos.y = mouse.mouse_position_top;
                
            current_rectangle->size.width = 150;
            current_rectangle->size.height = 150;
            
            rectangle_is_selected = true;
            selected_rectangle_index = current_rectangle_index;
        }
        */
        
        /*
        if (mouse.left_mouse_button_has_gone_up)
        {
            rectangle_is_selected = true;
        }
        */
        
        /*
        if (mouse.right_mouse_button_has_gone_down)
        {
            rectangle_is_selected = false;
            nr_of_rectangles = 0;
        }
        */
        
        /*
        if (mouse.left_mouse_button_is_down)
        {
            if (rectangle_is_selected)
            {
                entity * current_rectangle = &rectangle[selected_rectangle_index];
                
                current_rectangle->pos.x = mouse.mouse_position_left;
                current_rectangle->pos.y = mouse.mouse_position_top;

            }
        }
        */

        
        /*
        for (int i = 0; i < nr_of_rectangles; i++)
        {
//            jsLogInt(i*8);
            entity * current_rectangle = &rectangle[i];
            
            draw_rectangle(current_rectangle->pos.x, current_rectangle->pos.y, 
                           current_rectangle->size.width, current_rectangle->size.height, 
                           current_rectangle->line_color, current_rectangle->fill_color, 
                           current_rectangle->line_width);
        }
        */
        
    }
}