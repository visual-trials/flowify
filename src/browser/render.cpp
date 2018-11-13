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

extern "C" { 
    extern void jsDrawRoundedRect(i32 x, i32 y, i32 width, i32 height, i32 r,
                                  i32 line_color_rgb, i32 line_color_alpha, 
                                  i32 fill_color_rgb, i32 fill_color_alpha, 
                                  i32 line_width);
                                  
    extern void jsDrawLaneSegment(i32 left_top_x, i32 right_top_x, i32 top_y, 
                                  i32 left_bottom_x, i32 right_bottom_x, i32 bottom_y, i32 radius,
                                  i32 line_color_rgb, i32 line_color_alpha, 
                                  i32 fill_color_rgb, i32 fill_color_alpha, 
                                  i32 line_width);
                                  
    extern void jsDrawArcedCorner(i32 x, i32 y, i32 r, i32 rotation, i32 convex,
                                  i32 line_color_rgb, i32 line_color_alpha, 
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
                           i32 line_width);
                           
    extern void jsDrawText(i32 x, i32 y, 
                           u8 * string_data, i32 string_length,
                           i32 font_height, 
                           i32 font_color_rgb, i32 font_color_alpha);
                           // TODO: base font choice
                           //u8 * base_font_data, i32 base_font_length );
                           
    extern void jsLog(u8 * text_data, i32 text_length);
    
    extern void jsLogInt(i32 log_integer);
}

void draw_rounded_rectangle(i32 x, i32 y, i32 width, i32 height, i32 r,
                            Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawRoundedRect(x, y, width, height, r, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_lane_segment(i32 left_top_x, i32 right_top_x, i32 top_y, 
                       i32 left_bottom_x, i32 right_bottom_x, i32 bottom_y, i32 radius,
                       Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawLaneSegment(left_top_x, right_top_x, top_y, left_bottom_x, right_bottom_x, bottom_y, radius,
                      line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_arced_corner(i32 x, i32 y, i32 r, i32 rotation, i32 convex, 
                       Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawArcedCorner(x, y, r, rotation, convex, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_rectangle(i32 x, i32 y, i32 width, i32 height, 
                    Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawRect(x, y, width, height, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_line(i32 x_start, i32 y_start, i32 x_end, i32 y_end, 
                    Color4 line_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    jsDrawLine(x_start, y_start, x_end, y_end, line_color_rgb, line_color_alpha, line_width);
}

void draw_ellipse(i32 x, i32 y, i32 width, i32 height, 
                  Color4 line_color, Color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawEllipse(x, y, width, height, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_text(i32 x, i32 y, ShortString * text, i32 font_height, Color4 font_color)
{
    // u8 base_font[] = "Arial"; // FIXME: hardcoded (btw: do we really want to pass this each time?)
    
    i32 font_color_rgb = font_color.r + font_color.g * 256 + font_color.b * 256 * 256; 
    i32 font_color_alpha = (i32)font_color.a;
    
    jsDrawText(x, y, text->data, text->length, font_height, font_color_rgb, font_color_alpha); //, base_font, string_length(base_font));
}

void draw_text_c(i32 x, i32 y, const char * cstring, i32 font_height, Color4 font_color)
{
    ShortString text;
    copy_cstring_to_short_string(cstring, &text);
    draw_text(x, y, &text, font_height, font_color);
}

void log(ShortString * text)
{
    jsLog(text->data, text->length);
}

