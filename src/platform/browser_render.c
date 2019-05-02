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
 
// Note: Keep this list in sync with the other font-lists!
const f32 textHeightToFontSizeRatio[] = {
    1.1499023, // Arial
    1.1328125  // Courier New
};

extern "C" { 
    extern void jsClipRect(i32 x, i32 y, i32 width, i32 height);
    
    extern void jsUnClipRect();
                           
    extern void jsDrawRoundedRect(i32 x, i32 y, i32 width, i32 height, i32 r,
                                  i32 line_color_rgb, i32 line_color_alpha, 
                                  i32 fill_color_rgb, i32 fill_color_alpha, 
                                  i32 line_width);
                                  
    extern void jsDrawLane(DirectionalRect2d * lane_parts_index, i32 lane_parts_count, 
                           b32 partial_rect_at_start, b32 is_right_side_at_start, 
                           b32 partial_rect_at_end, b32 is_right_side_at_end, 
                           i32 radius, i32 line_color_rgb, i32 line_color_alpha, 
                           i32 fill_color_rgb, i32 fill_color_alpha, 
                           i32 line_width);
                                  
    extern void jsDrawRect(i32 x, i32 y, i32 width, i32 height, 
                           i32 line_color_rgb, i32 line_color_alpha, 
                           i32 fill_color_rgb, i32 fill_color_alpha, 
                           i32 line_width);
                           
    extern void jsDrawEllipse(i32 x, i32 y, i32 width, i32 height, 
                           i32 line_color_rgb, i32 line_color_alpha, 
                           i32 fill_color_rgb, i32 fill_color_alpha, 
                           i32 line_width);
                           
    extern void jsDrawLine(i32 x_start, i32 y_start, i32 x_end, i32 y_end, 
                           i32 line_color_rgb, i32 line_color_alpha, 
                           i32 line_width, b32 round_cap);
                           
    extern void jsDrawText(i32 x, i32 y, 
                           u8 * string_data, i32 string_length,
                           i32 font_height, i32 font_family_index,
                           i32 font_color_rgb, i32 font_color_alpha);
                           
    extern f32 jsGetTextWidth(u8 * string_data, i32 string_length,
                              i32 font_height, i32 font_family_index);
                           
    extern void jsLog(u8 * text_data, i32 text_length);
    
    extern void jsAbort(u8 * text_data, i32 text_length, u8 * file_name_data, i32 file_name_length, i32 line_number);
    
    extern void jsSetUsingPhysicalPixels(b32 using_physical_pixels);
}

void log(const char * text);

void clip_rectangle(Pos2d position, Size2d size)
{
    jsClipRect(position.x, position.y, size.width, size.height);
}

void unclip_rectangle()
{
    jsUnClipRect();
}

void draw_rounded_rectangle(Pos2d position, Size2d size, i32 r,
                            Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawRoundedRect(position.x, position.y, size.width, size.height, r, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_rounded_rectangle(Rect2d rect, i32 r, Color4 line_color, Color4 fill_color, i32 line_width)
{
    draw_rounded_rectangle(rect.position, rect.size, r, line_color, fill_color, line_width);
}

void draw_lane(DirectionalRect2d * lane_parts, i32 lane_parts_count,
               b32 partial_rect_at_start, b32 is_right_side_at_start, 
               b32 partial_rect_at_end, b32 is_right_side_at_end, 
               i32 radius, Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawLane(lane_parts, lane_parts_count, 
               partial_rect_at_start, is_right_side_at_start, 
               partial_rect_at_end, is_right_side_at_end, 
               radius, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_rectangle(Pos2d position, Size2d size, 
                    Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawRect(position.x, position.y, size.width, size.height, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_rectangle(Rect2d rect, Color4 line_color, Color4 fill_color, i32 line_width)
{
    draw_rectangle(rect.position, rect.size, line_color, fill_color, line_width);
}

void draw_line(Pos2d start_position, Pos2d end_position, 
                    Color4 line_color, i32 line_width, b32 round_cap = false)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    jsDrawLine(start_position.x, start_position.y, end_position.x, end_position.y, line_color_rgb, line_color_alpha, line_width, round_cap);
}

void draw_ellipse(Pos2d position, Size2d size, 
                  Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawEllipse(position.x, position.y, size.width, size.height, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

Size2d get_text_size(String * text, Font font)
{
    f32 text_width = jsGetTextWidth(text->data, text->length, font.height, font.family);
    
    Size2d text_size = {};
    text_size.width = (i32)text_width;
    text_size.height = (i32)((f32)font.height * textHeightToFontSizeRatio[font.family]); // Note: this is a workaround. The browser doens't give us this info!
    
    return text_size;
}

Size2d get_text_size(ShortString * text, Font font)
{
    String text_string = {};
    text_string.data = text->data;
    text_string.length = text->length;
    return get_text_size(&text_string, font);
}

Size2dFloat get_text_size_float(ShortString * text, Font font)
{
    f32 text_width = jsGetTextWidth(text->data, text->length, font.height, font.family);
    
    Size2dFloat text_size = {};
    text_size.width = text_width;
    text_size.height = (f32)font.height * textHeightToFontSizeRatio[font.family]; // Note: this is a workaround. The browser doens't give us this info!
    
    return text_size;
}

void draw_text(Pos2d position, ShortString * text, Font font, Color4 font_color)
{
    i32 font_color_rgb = font_color.r + font_color.g * 256 + font_color.b * 256 * 256; 
    i32 font_color_alpha = (i32)font_color.a;
    
    jsDrawText(position.x, position.y, text->data, text->length, font.height, font.family, font_color_rgb, font_color_alpha);
}

void draw_text(Pos2d position, String * text, Font font, Color4 font_color)
{
    i32 font_color_rgb = font_color.r + font_color.g * 256 + font_color.b * 256 * 256; 
    i32 font_color_alpha = (i32)font_color.a;

    jsDrawText(position.x, position.y, text->data, text->length, font.height, font.family, font_color_rgb, font_color_alpha);
}

void draw_text_c(Pos2d position, const char * cstring, Font font, Color4 font_color)
{
    ShortString text;
    copy_cstring_to_short_string(cstring, &text);
    draw_text(position, &text, font, font_color);
}

void log(ShortString * text)
{
    jsLog(text->data, text->length);
}

void log(String text)
{
    jsLog(text.data, text.length);
}

void log(const char * text)
{
    String text_string;
    text_string.data = (u8 *)text;
    text_string.length = cstring_length(text);
    log(text_string);
}

void log_int(i32 integer)
{
    ShortString decimal_number;
    int_to_string(integer, &decimal_number);
    log(&decimal_number);
}

void abort(const char * text, const char * file_name, i32 line_number)
{
    i32 text_length = cstring_length(text);
    i32 file_name_length = cstring_length(file_name);
    jsAbort((u8 *)text, text_length, (u8 *)file_name, file_name_length, line_number);
}

void set_using_physical_pixels(b32 using_physical_pixels)
{
    jsSetUsingPhysicalPixels(using_physical_pixels);
}
