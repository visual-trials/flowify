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
 
extern "C" { 
    extern void jsDrawRect(int x, int y, int width, int height, 
                           int line_color_rgb, int line_color_alpha, int fill_color_rgb, int fill_color_alpha, int line_width);
    // FIXME: enable logging strings!
    extern void jsLogInt(int log_integer);
}

void draw_rectangle(int x, int y, int width, int height, 
                    color4 line_color, color4 fill_color, int line_width)
{
    int line_color_rgb = line_color.r + line_color.g * 256 + line_color.b * 256 * 256; 
    int line_color_alpha = (int)line_color.a;
    
    int fill_color_rgb = fill_color.r + fill_color.g * 256 + fill_color.b * 256 * 256;
    int fill_color_alpha = (int)fill_color.a;
    
    jsDrawRect(x, y, width, height, line_color_rgb, line_color_alpha, fill_color_rgb, fill_color_alpha, line_width);
}