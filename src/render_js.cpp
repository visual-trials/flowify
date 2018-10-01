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

#include "render.h"

// TODO: put this function in a common place
i32 string_length(char * string)
{
    i32 count = 0;
    while (*string++)
    {
        count++;
    }
    return count;
}

extern "C" { 
    extern void jsDrawRect(i32 x, i32 y, i32 width, i32 height, 
                           i32 line_color_rgb, i32 line_color_alpha, i32 fill_color_rgb, i32 fill_color_alpha, i32 line_width);
                           
    extern void jsDrawText(i32 x, i32 y, char * text, i32 length);
    
    extern void jsLog(char * text, i32 length);
}

void draw_rectangle(i32 x, i32 y, i32 width, i32 height, 
                    color4 line_color, color4 fill_color, i32 line_width)
{
    i32 line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    i32 line_color_alpha = (i32)line_color.a;
    
    i32 fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    i32 fill_color_alpha = (i32)fill_color.a;
    
    jsDrawRect(x, y, width, height, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}

void draw_text(i32 x, i32 y, char * text) // FIXME: use length-strings instead of null-terminated strings!
{
    // FIXME: add color, font, size etc
    jsDrawText(x, y, text, string_length(text));
}

void log(char * text)
{
    jsLog(text, string_length(text));
}

