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
 
#include "parser.c"

#include "flowify/flowifier.h"
#include "flowify/interact.c"
#include "flowify/flowifier.c"
#include "flowify/layout.c"
#include "flowify/draw.c"

#include "examples.h"

struct WorldData
{
    Rect2d flow_rect;
    Rect2d title_rect;
    Rect2d code_rect;
    
    b32 show_code;
    String program_text;
    String program_name;
    ScrollableText scrollable_program_text;
    Window program_text_window;

    Tokenizer tokenizer;
    Parser parser;
    Flowifier flowifier;
    
    // DynamicString flowify_dump_text;
    // ScrollableText scrollable_flowify_dump;
    // Window flowify_dump_window;
    
    DynamicString flowify_detail_text;
    ScrollableText scrollable_flowify_detail;
    Window flowify_detail_window;
    
    Margins screen_margins;
    i32 middle_margin;
    i32 title_height;
    f32 program_text_fraction_of_screen;
    // f32 flowify_dump_fraction_of_screen;
    
    FlowElement * root_element;
    
    b32 help_is_expanded;
    
    b32 menu_is_expanded;
    const char * program_texts[10];
    const char * program_names[10];
    i32 nr_of_program_texts;
    i32 current_program_text_index;
    
    i32 iteration;
    
    // TODO: Window flowify_window;
    i32 flowify_vertical_offset;
    i32 flowify_horizontal_offset;
    
    // FIXME: ugly HACK!
    b32 mouse_dragging;
    Pos2d last_mouse_position;
    
    // FIXME: ugly HACK!
    b32 touch_dragging;
    Pos2d last_touch_position;
    
    b32 verbose_memory_usage;
    b32 verbose_frame_times;
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void center_root_element(WorldData * world, b32 center_only_horizontally = false)
    {
        // Centering root element inside flow_rect (unless the root-element is too big)
        Size2d root_size = world->root_element->rect.size;
        
        // The root element fits horizontally side the flow_rect, so we center it
        world->flowify_horizontal_offset = world->flow_rect.position.x + world->flow_rect.size.width / 2 - root_size.width / 2;
        if  (world->flowify_horizontal_offset < 120)
        {
            world->flowify_horizontal_offset = 120;
        }
        
        if (!center_only_horizontally)
        {
            // The root element fits horizontally side the flow_rect, so we center it
            world->flowify_vertical_offset = world->flow_rect.position.y + world->flow_rect.size.height / 2 - root_size.height / 2;
            if (world->flowify_vertical_offset < 50)
            {
                world->flowify_vertical_offset = 50;
            }
        }
    }
    
    void load_program_text(const char * program_text, const char * program_name, WorldData * world)
    {
        Tokenizer * tokenizer = &world->tokenizer;
        Parser * parser = &world->parser;
        Flowifier * flowifier = &world->flowifier;
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        // ScrollableText * scrollable_flowify_dump = &world->scrollable_flowify_dump;
        // DynamicString * flowify_dump_text = &world->flowify_dump_text;
        
        init_scrollable_text(scrollable_program_text, &world->program_text_window);
        world->program_text_window.has_vertical_scrollbar = false;
        scrollable_program_text->position_based_on_highlighted_line_parts = true;
        
        init_tokenizer(tokenizer);
        init_parser(parser, tokenizer);
        init_flowifier(flowifier, parser);
        // init_dynamic_string(flowify_dump_text, (Color4){70,150,255,255}, cstring_to_string("Flowify dump text"));
        // init_scrollable_text(scrollable_flowify_dump, &world->flowify_dump_window, false);
        
        world->program_text.data = (u8 *)program_text;
        world->program_text.length = cstring_length(program_text);
        
        world->program_name.data = (u8 *)program_name;
        world->program_name.length = cstring_length(program_name);
        
        split_string_into_scrollable_lines(world->program_text, scrollable_program_text);

        // Tokenize
        tokenize(tokenizer, (u8 *)program_text);

        //Parse
        Node * root_node = parse_program(parser);
        
        // Flowify
        FlowElement * root_element = flowify_root(flowifier, root_node);
        
        // TODO: maybe put root_element inside the Flowifier-struct?
        world->root_element = root_element;
        
        // dump_element_tree(root_element, &world->flowify_dump_text);
        
        // split_string_into_scrollable_lines(world->flowify_dump_text.string, scrollable_flowify_dump);
        
        layout_elements(&world->flowifier, world->root_element);
        
        center_root_element(world);
    }
    
    void update_window_dimensions(WorldData * world, Screen * screen)
    {
        Rect2d full_screen_rect = {}; // also meaning: position = 0,0
        full_screen_rect.size.width = screen->width;
        full_screen_rect.size.height = screen->height;
        
        Rect2d available_screen_rect = shrink_rect_by_margins(full_screen_rect, world->screen_margins);
        f32 flow_code_fraction = 0.6;
        if (!world->show_code)
        {
            flow_code_fraction = 1.0;
        }
        Rectangle2 left_and_right_screen_rects = split_rect_horizontally_fraction(available_screen_rect, flow_code_fraction, world->middle_margin);
        world->flow_rect = left_and_right_screen_rects.first;
        world->code_rect = left_and_right_screen_rects.second;
        Rectangle2 title_and_code_rects = split_rect_vertically(world->code_rect, 100);
        //Rectangle2 dump_and_flowify_rects = split_rect_horizontally_fraction(horizontal_rects.second, world->program_text_fraction_of_screen / (1 - world->flowify_dump_fraction_of_screen), world->middle_margin);
        
        world->title_rect = title_and_code_rects.first;
        // world->flowify_dump_window.screen_rect = horizontal_rects.first;
        world->program_text_window.screen_rect = title_and_code_rects.second;
        // TODO: store rect in flowify-window!
    }
        
    void init_world()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;

        world->iteration = 0;
        
        world->flowify_vertical_offset = 0;
        world->flowify_horizontal_offset = 0;
        
        world->help_is_expanded = false;
        world->menu_is_expanded = false;
        
        world->program_texts[0] = simple_assign_program_text;
        world->program_names[0] = simple_assign_program_name;
        
        world->program_texts[1] = i_plus_plus_program_text;
        world->program_names[1] = i_plus_plus_program_name;
        
        world->program_texts[2] = simple_math_program_text;
        world->program_names[2] = simple_math_program_name;
        
        world->program_texts[3] = simple_if_program_text;
        world->program_names[3] = simple_if_program_name;
        
        world->program_texts[4] = simple_if_else_program_text;
        world->program_names[4] = simple_if_else_program_name;
        
        world->program_texts[5] = simple_for_program_text;
        world->program_names[5] = simple_for_program_name;
        
        // TODO: in the end, we probably want this example instead: world->program_texts[5] = simple_for_continue_break_program_text;
        world->program_texts[6] = fibonacci_iterative_program_text;
        world->program_names[6] = fibonacci_iterative_program_name;
        
        world->program_texts[7] = fibonacci_recursive_early_return_program_text;
        world->program_names[7] = fibonacci_recursive_early_return_program_name;
        
        world->program_texts[8] = large_example_program_text;
        world->program_names[8] = large_example_program_name;
        
        world->nr_of_program_texts = 9;
        
        world->current_program_text_index = 5;
        
        world->verbose_memory_usage = false;
        world->verbose_frame_times = false;

        world->screen_margins.left = 20;
        world->screen_margins.top = 20;
        world->screen_margins.right = 20;
        world->screen_margins.bottom = 20;
        
        world->middle_margin = 20;
        // world->flowify_dump_fraction_of_screen = 0.5;
        world->program_text_fraction_of_screen = 0.5;
        world->title_height = 30;
        
        world->mouse_dragging = false;
        world->last_mouse_position.x = 0;
        world->last_mouse_position.y = 0;
        
        world->touch_dragging = false;
        world->last_touch_position.x = 0;
        world->last_touch_position.y = 0;
        
        world->show_code = false;
        
        update_window_dimensions(world, &input->screen);
        
        load_program_text(world->program_texts[world->current_program_text_index], 
                          world->program_names[world->current_program_text_index], 
                          world);
    }
    
    void update_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        Flowifier * flowifier = &world->flowifier;

        i32 nr_of_flow_elements = flowifier->flow_elements.nr_of_index_entries;
        
        // Process input
        if (flowifier->has_absolute_positions)
        {
            i32 old_selected_element_index = flowifier->interaction.selected_element_index;
            
            process_interactions(flowifier, input, world->root_element);
            
            if (flowifier->interaction.acted_upon_element_index)
            {
                FlowElement * acted_upon_element = (FlowElement *)get_item_by_index(&flowifier->flow_elements, flowifier->interaction.acted_upon_element_index);
                
                // TODO: right now when a FunctionBody or FunctionCallIdentifier is double-clicked,
                //       we do as-if the FunctionCall is clicked. We probably shouldn't hardcode this here
                //       but instead set properties on these objects to indicatie what the 
                //       double-click-take-over-element should be.
                
                if (acted_upon_element->type == FlowElement_FunctionCallIdentifier)
                {
                    acted_upon_element = acted_upon_element->parent;
                    flowifier->interaction.acted_upon_element_index = acted_upon_element->index;
                }
                
                /*
                // FIXME: turned off for now
                if (acted_upon_element->type == FlowElement_FunctionBody)
                {
                    acted_upon_element = acted_upon_element->parent->parent;
                    flowifier->interaction.acted_upon_element_index = acted_upon_element->index;
                }
                */
                
                if (acted_upon_element->type == FlowElement_FunctionCall)
                {
                    acted_upon_element->is_collapsed = !acted_upon_element->is_collapsed;
                    if (acted_upon_element->is_collapsed)
                    {
                        // TODO: if its now collapsed, the whole root element may be completely off screen.
                        //       we want to re-position all elements, so at least this element becomes in-view
                        //       this is a little dirty, but it kinda works for now
                        
                        // FIXME: turned off for now: world->flowify_vertical_offset = -acted_upon_element->rect.position.y;
                    }
                }
                
                flowifier->interaction.acted_upon_element_index = 0;
            }

            // TODO: we should also re-generate the detail-string when the absolute position changes of the element
            // FIXME: turned off for now: if (old_selected_element_index != flowifier->interaction.selected_element_index)
            /*
            {
                FlowElement * newly_selected_flow_element = &flow_elements[flowifier->interaction.selected_element_index];
                
                init_scrollable_text(&world->scrollable_flowify_detail, &world->flowify_detail_window, false);
                
                init_dynamic_string(&world->flowify_detail_text, (Color4){255,70,150,255}, cstring_to_string("Flowify detail text"));

                generate_element_detail(newly_selected_flow_element, &world->flowify_detail_text);

                split_string_into_scrollable_lines(world->flowify_detail_text.string, &world->scrollable_flowify_detail);
            }
            */
        }
        
        // Update world
        update_window_dimensions(world, &input->screen);
        
        update_scrollable_text(&world->scrollable_program_text, input);
        // update_scrollable_text(&world->scrollable_flowify_dump, input);
        
        layout_elements(flowifier, world->root_element);

        // Up/Down the highlighted index
        
        KeyboardInput * keyboard = &input->keyboard;
        
        b32 arrow_up_pressed = false;
        b32 arrow_down_pressed = false;
        for (i32 sequence_key_index = 0; sequence_key_index < keyboard->sequence_keys_length; sequence_key_index++)
        {
            b32 is_down = (b32)keyboard->sequence_keys_up_down[sequence_key_index * 2];
            u8 key_code = keyboard->sequence_keys_up_down[sequence_key_index * 2 + 1];
            
            if (is_down)
            {
                if (key_code == Key_ArrowUp)
                {
                    arrow_up_pressed = true;
                }
                
                if (key_code == Key_ArrowDown)
                {
                    arrow_down_pressed = true;
                }
            }
        }
        
        if (arrow_up_pressed) {
            flowifier->interaction.highlighted_element_index--;
            flowifier->interaction.selected_element_index = flowifier->interaction.highlighted_element_index;
        }
        
        if (arrow_down_pressed) {
            flowifier->interaction.highlighted_element_index++;
            flowifier->interaction.selected_element_index = flowifier->interaction.highlighted_element_index;
        }
        
        if (flowifier->interaction.highlighted_element_index > 0 && world->iteration > 60) // every second (and if it is a valid selected element)
        {
            world->iteration = 0;
            
            flowifier->interaction.highlighted_element_index++;
            while (flowifier->interaction.highlighted_element_index < nr_of_flow_elements)
            {
                FlowElement * newly_highlighted_flow_element = (FlowElement *)get_item_by_index(&flowifier->flow_elements, flowifier->interaction.highlighted_element_index);
                if (newly_highlighted_flow_element->is_highlightable)
                {
                    break;
                }
                flowifier->interaction.highlighted_element_index++;
            }
        }
        if (flowifier->interaction.highlighted_element_index == 0 || flowifier->interaction.highlighted_element_index >= nr_of_flow_elements)
        {
            flowifier->interaction.highlighted_element_index = 1;
            while (flowifier->interaction.highlighted_element_index < nr_of_flow_elements)
            {
                FlowElement * newly_highlighted_flow_element = (FlowElement *)get_item_by_index(&flowifier->flow_elements, flowifier->interaction.highlighted_element_index);
                if (newly_highlighted_flow_element->is_highlightable)
                {
                    break;
                }
                flowifier->interaction.highlighted_element_index++;
            }
        }
        
        
    }
    
    // TODO: make this a more general function (add arguments nr_of_buttons and current_button_index)
    //       then return button_pressed_index (so you can load the program text for that index)
    void draw_and_update_button_menu(WorldData * world)
    {
        // Draw (and update) button menu
        
        Size2d size_menu_button = {50, 50};
        Pos2d position_menu_button = {20, 20};
        Pos2d position_menu = {20, 71};
        Size2d size_menu_item = {500, 50};
        i32 margin_between_menu_items = 0;
        
        Size2d size_help_button = {50, 50};
        Pos2d position_help_button = {}; 
        position_help_button.x = global_input.screen.width - size_help_button.width - 20;
        position_help_button.y = global_input.screen.height - size_help_button.height - 20;
        
        ShortString help_label;
        copy_cstring_to_short_string("help", &help_label);
        
        FlowElement * root_element = world->root_element;
        Input * input = &global_input;
        MouseInput * mouse = &input->mouse;
        TouchesInput * touch = &input->touch;
        Flowifier * flowifier = &world->flowifier;

        if (mouse->wheel_has_moved)
        {
            // TODO: account for a "Mac" mouse! (which has a 'continous' wheel)
            if (mouse->wheel_delta > 0)
            {
                world->flowify_vertical_offset += 30;
            }
            
            if (mouse->wheel_delta < 0)
            {
                world->flowify_vertical_offset -= 30;
            }
        }
        
        // FIXME: ugly HACK!
        Pos2d mouse_delta_position = {};
        if (mouse->left_button_is_down)
        {
            if (world->mouse_dragging)
            {
                mouse_delta_position.x = mouse->position.x - world->last_mouse_position.x;
                mouse_delta_position.y = mouse->position.y - world->last_mouse_position.y;
            }
            world->flowify_horizontal_offset += mouse_delta_position.x;
            world->flowify_vertical_offset += mouse_delta_position.y;
            
            world->last_mouse_position = mouse->position;
            world->mouse_dragging = true;
        }
        else
        {
            world->mouse_dragging = false;
        }
        
        // FIXME: ugly HACK!
        Pos2d touch_delta_position = {};
        if (touch->touch_count == 1)
        {
            if (world->touch_dragging)
            {
                touch_delta_position.x = touch->touches[0].position.x - world->last_touch_position.x;
                touch_delta_position.y = touch->touches[0].position.y - world->last_touch_position.y;
            }
            world->flowify_horizontal_offset += touch_delta_position.x;
            world->flowify_vertical_offset += touch_delta_position.y;
            
            // FIXME: we are not checking if the touch was actually ended!
            world->last_touch_position = touch->touches[0].position;
            world->touch_dragging = true;
        }
        else
        {
            world->touch_dragging = false;
        }
        
        
        
        Pos2d absolute_position;
        // FIXME: hack!
        // Aligned right: absolute_position.x = input->screen.width - root_element->rect.size.width - 100 + world->flowify_horizontal_offset; 
        absolute_position.x = world->flowify_horizontal_offset; 
        absolute_position.y = world->flowify_vertical_offset; 
        absolute_layout_elements(flowifier, root_element, absolute_position);
        
        draw_elements(flowifier, root_element);
        
        // FIXME: hack!
        /*
        if (flowifier->interaction.selected_element_index)
        {
            FlowElement * flow_elements = (FlowElement *)flowifier->flow_elements.items;
            FlowElement * selected_flow_element = &flow_elements[flowifier->interaction.selected_element_index];
            
            Rect2d detail_rect = {};
            
            detail_rect.position = world->root_element->rect_abs.position;
            detail_rect.size.width = 500; // TODO: we should choose a different width here right?
            detail_rect.size.height = 644; // TODO: we should choose a different width here right?
            
            detail_rect.position.x += world->root_element->rect.size.width + 20;
            // detail_rect.position.y = selected_flow_element->absolute_position.y + selected_flow_element->rect.size.height / 2 - detail_rect.size.height / 4;
            
            draw_rounded_rectangle(detail_rect.position, detail_rect.size, flowifier->detail_style);

            // FIXME: update_scrollable_text should be called in update_frame!!
            world->flowify_detail_window.screen_rect = detail_rect;
            // update_scrollable_text(&world->scrollable_flowify_detail, input);
            
            draw_scrollable_text(&world->scrollable_flowify_detail);
        }
        */
        
        // Help button
        
        b32 help_button_is_active = false;
        b32 help_button_is_pressed = do_button(position_help_button, size_help_button, &help_label, help_button_is_active, &global_input);
        
        if (help_button_is_pressed)
        {
            world->help_is_expanded = !world->help_is_expanded;
        }
        
        if (world->help_is_expanded)
        {
            Font font = {};
            font.height = 20;
            font.family = Font_CourierNew;
            i32 line_height = 30;

            Size2d size_help = {700, 500};
            Pos2d position_help = {};
            position_help.x = position_help_button.x - size_help.width + size_help_button.width;
            position_help.y = position_help_button.y - size_help.height;
            Color4 text_color = {   0,   0,   0, 255};
            Color4 line_color = { 100, 100, 100, 255};
            Color4 fill_color = { 255, 255, 255, 255};
            
            DrawStyle draw_style = {};
            draw_style.corner_radius = 5;
            draw_style.line_width = 1;
            draw_style.line_color = line_color;
            draw_style.fill_color = fill_color;
            
            draw_rounded_rectangle(position_help, size_help, draw_style);
            Pos2d current_position = position_help;
            
            current_position.x += 20;
            current_position.y += 20;
            
            ShortString help_text_item = {};
            
            copy_cstring_to_short_string("Mouse scroll:", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            copy_cstring_to_short_string("    move visualization up/down.", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            current_position.y += line_height / 2;
            
            copy_cstring_to_short_string("Mouse hover:", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            copy_cstring_to_short_string("    highlight element and (if code is expanded) its", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            copy_cstring_to_short_string("    corresponding code.", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            current_position.y += line_height / 2;
            
            copy_cstring_to_short_string("Mouse (click and) drag: ", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            copy_cstring_to_short_string("   move visualization up/down/left/right.", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            current_position.y += line_height / 2;
            
            copy_cstring_to_short_string("Mouse double-click on function:", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            copy_cstring_to_short_string("    expand/collapse the function body.", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            current_position.y += line_height / 2;
            
            copy_cstring_to_short_string("Mouse click on 'code' button:", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            copy_cstring_to_short_string("    expand/collapse the source code.", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            current_position.y += line_height / 2;
            
            copy_cstring_to_short_string("Mouse click on 'menu' button:", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            copy_cstring_to_short_string("    choose an example program.", &help_text_item);
            draw_text(current_position, &help_text_item, font, text_color);
            current_position.y += line_height;
            current_position.y += line_height / 2;
        }
        
        // Menu button
        b32 menu_button_is_active = false;
        b32 menu_button_is_pressed = do_integer_button(position_menu_button, size_menu_button, 0, menu_button_is_active, &global_input);
        
        if (menu_button_is_pressed)
        {
            world->menu_is_expanded = !world->menu_is_expanded;
        }
        
        if (world->menu_is_expanded)
        {
            for (i32 program_text_index = 0; program_text_index < world->nr_of_program_texts; program_text_index++)
            {
                b32 menu_item_button_is_active = false;
                if (program_text_index == world->current_program_text_index)
                {
                    menu_item_button_is_active = true;
                }
                
                ShortString program_name = {};
                copy_cstring_to_short_string(world->program_names[program_text_index], &program_name);

                Pos2d position_menu_item = position_menu;
                position_menu_item.y += program_text_index * size_menu_item.height;
                b32 menu_item_button_is_pressed = do_menu_item(position_menu_item, size_menu_item, &program_name, menu_item_button_is_active, &global_input);
                
                if (menu_item_button_is_pressed)
                {
                    world->current_program_text_index = program_text_index;
                    load_program_text(world->program_texts[world->current_program_text_index], 
                                      world->program_names[world->current_program_text_index],
                                      world);
                                      
                    world->menu_is_expanded = false;
                }
            }
            
            Size2d size_menu = size_menu_item;
            size_menu.height = size_menu_item.height * world->nr_of_program_texts;
            Color4 line_color = { 100, 100, 100, 255};
            Color4 fill_color = { 255, 255, 255,   0};
            
            DrawStyle draw_style = {};
            draw_style.corner_radius = 5;
            draw_style.line_width = 1;
            draw_style.line_color = line_color;
            draw_style.fill_color = fill_color;
            
            
            draw_rounded_rectangle(position_menu, size_menu, draw_style);
                
        }
        
    }
    
    void render_frame()
    {
        WorldData * world = &global_world;
        Input * input = &global_input;
        Memory * memory = &global_memory;
        Flowifier * flowifier = &world->flowifier;
        
        ScrollableText * scrollable_program_text = &world->scrollable_program_text;
        // ScrollableText * scrollable_flowify_dump = &world->scrollable_flowify_dump;
        
        Font font = {};
        font.height = 20;
        font.family = Font_CourierNew;

        Color4 black = {};
        black.a = 255;
        
        Token * tokens = (Token *)world->tokenizer.tokens.items;
        
        i32 nr_of_flow_elements = flowifier->flow_elements.nr_of_index_entries;
        
        String * lines = (String *)scrollable_program_text->lines.items;

        if (nr_of_flow_elements > 0 && flowifier->interaction.hovered_element_index > 0)
        {
            FlowElement * highlighted_flow_element = (FlowElement *)get_item_by_index(&flowifier->flow_elements, flowifier->interaction.hovered_element_index);
            Node * node = highlighted_flow_element->ast_node;
            
            remove_highlighted_line_parts(scrollable_program_text);
            
            if (node && 
                highlighted_flow_element->type != FlowElement_Root &&
                highlighted_flow_element->type != FlowElement_FunctionBody)
            {
                // Not highlighting Root or FunctionBody
                
                for (i32 token_index = node->first_token_index; token_index <= node->last_token_index; token_index++)
                {
                    Token token = tokens[token_index];
                    
                    if (token.type != Token_EndOfStream)
                    {
                        String program_line_text = lines[token.line_index];
                        
                        i32 character_in_line_index = (i32)token.text.data - (i32)program_line_text.data;

                        HighlightedLinePart * highlighted_line_part = add_new_highlighted_line_part(scrollable_program_text);

                        highlighted_line_part->line_index = token.line_index;
                        highlighted_line_part->start_character_index = (u16)character_in_line_index;
                        highlighted_line_part->length = (u16)token.text.length;
                    }
                }

                // remove_highlighted_line_parts(scrollable_flowify_dump); 
                
                // HighlightedLinePart * highlighted_line_part = add_new_highlighted_line_part(scrollable_flowify_dump);
                // highlighted_line_part->line_index = highlighted_flow_element->highlighted_line_part.line_index;
                // highlighted_line_part->start_character_index = highlighted_flow_element->highlighted_line_part.start_character_index;
                // highlighted_line_part->length = highlighted_flow_element->highlighted_line_part.length;
            }
        }
        
        // Turned off for the moment: 
        // draw_scrollable_text(scrollable_flowify_dump);
        
        draw_and_update_button_menu(world);

        if (world->show_code)
        {
            Color4 fill_color = { 255, 255, 255, 255 };
            Color4 line_color = { 200, 200, 200, 255 };
            
            DrawStyle code_style = {};
            code_style.line_width = 2;
            code_style.fill_color = fill_color;
            code_style.line_color = line_color;
            
            draw_rounded_rectangle(world->code_rect.position, world->code_rect.size, code_style);
                                   
            Size2d program_name_size = get_text_size(world->program_name, font);
            Pos2d program_name_position = {};
            program_name_position.x = world->title_rect.position.x + world->title_rect.size.width / 2 - program_name_size.width / 2;
            program_name_position.y = world->title_rect.position.y + world->title_rect.size.height / 2 - program_name_size.height / 2;
            draw_text(program_name_position, world->program_name, font, black);
            
            draw_scrollable_text(scrollable_program_text);
        }
        
        ShortString code_label;
        copy_cstring_to_short_string("code", &code_label);
        
        Size2d size_code_button = {50, 50};
        Pos2d position_code_button = {};
        position_code_button.x = global_input.screen.width - size_code_button.width - 20;
        position_code_button.y = 20;

        b32 code_button_is_pressed = do_button(position_code_button, size_code_button, &code_label, world->show_code, &global_input);

        if (code_button_is_pressed)
        {
            world->show_code = !world->show_code;
            update_window_dimensions(world, &input->screen);
            b32 center_only_horizontally = true;
            center_root_element(world, center_only_horizontally);
        }
            
        // Button for toggling showing help rectangles
        {
            Size2d size_button = {50, 50};
            Pos2d position_button = {};
            position_button.x = global_input.screen.width - size_button.width - 20;
            position_button.y = 400;
            
            ShortString label;
            copy_cstring_to_short_string("Help", &label);
            ShortString label_active;
            copy_cstring_to_short_string("[   ]", &label_active);

            b32 button_is_pressed = do_button(position_button, size_button, &label, flowifier->show_help_rectangles, &global_input, &label_active);

            if (button_is_pressed)
            {
                flowifier->show_help_rectangles = !flowifier->show_help_rectangles;
            }
        }
        
        // do_memory_usage(memory, input, &world->verbose_memory_usage);
        // do_frame_timing(&global_input, &world->verbose_frame_times);
        do_physical_pixels_switch(&global_input);
    }
}
