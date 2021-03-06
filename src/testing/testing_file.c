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
 
struct WorldData
{
    i32 file_length;
    String file_name;
    
    ScrollableText scrollable_file;
    Window scrollable_file_window;
    
    b32 verbose_memory_usage;
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;

        init_scrollable_text(&world->scrollable_file, &world->scrollable_file_window);
    }
        
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        ScrollableText * scrollable_file = &world->scrollable_file;
        
        // Preparing file info
        if (input->file.file_was_uploaded)
        {
            // FIXME: this might be erased, so the pointer in .data may become invalid. We have to copy the content!
            String file_contents = input->file.file_contents.string;
            world->file_length = file_contents.length;
            world->file_name = input->file.file_name.string;
            
            split_string_into_scrollable_lines(file_contents, scrollable_file);
        }
        
        Margins margins = {};
        margins.top = 110;
        
        Rect2d full_screen_rect = {}; // also meaning: position = 0,0
        full_screen_rect.size.width = input->screen.width;
        full_screen_rect.size.height = input->screen.height;
        
        Rect2d available_screen_rect = shrink_rect_by_margins(full_screen_rect, margins);

        world->scrollable_file_window.screen_rect = available_screen_rect;
        
        /*
        // The screen size can change, so we have to update the position and size of the scrollables.
        scrollable_file->position.x = 0;
        scrollable_file->position.y = 110; // TODO: we should properly account for the hieght of the text above

        scrollable_file->size.width = input->screen.width - scrollable_file->position.x;
        scrollable_file->size.height = input->screen.height - scrollable_file->position.y;
        */
        
        update_scrollable_text(scrollable_file, input);
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        
        ScrollableText * scrollable_file = &world->scrollable_file;
        
        Color4 black = {};
        black.a = 255;
        
        Font font = scrollable_file->font;
        
        ShortString help_text;
        copy_cstring_to_short_string("Press Ctrl + L to open file", &help_text);
        Size2d help_text_size = get_text_size(&help_text, font);
        i32 help_text_x = ((f32)input->screen.width / (f32)2) - ((f32)help_text_size.width / (f32)2);
        draw_text((Pos2d){help_text_x, 50}, &help_text, font, black);
        
        Size2d file_name_size = get_text_size(world->file_name, font);
        i32 file_name_x = ((f32)input->screen.width / (f32)2) - ((f32)file_name_size.width / (f32)2);
        draw_text((Pos2d){file_name_x, 80}, world->file_name, font, black);
        
        // TODO: show file-name and file-length
        // ShortString file_length_text;
        // int_to_string(world->file_length, &file_length_text);
        // draw_text((Pos2d){10,10}, &file_length_text, font, black);
        
        draw_scrollable_text(scrollable_file);
        
        
        // Draw frame timing
        do_frame_timing(&global_input, &world->verbose_frame_times);
        
        do_memory_usage(&global_memory, &global_input, &world->verbose_memory_usage);
        do_physical_pixels_switch(&global_input);
    }
}