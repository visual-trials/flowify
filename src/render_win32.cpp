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

void clear_rectangle(i32 x, i32 y, i32 width, i32 height)
{
    SelectObject(device_context, GetStockObject(WHITE_PEN));
    SelectObject(device_context, GetStockObject(WHITE_BRUSH));
    Rectangle(device_context, x, y, x + width, y + height);
}

void draw_rectangle(i32 x, i32 y, i32 width, i32 height, color4 line_color, color4 fill_color, i32 line_width)
{
    SelectObject(device_context, GetStockObject(DC_PEN));
    SelectObject(device_context, GetStockObject(DC_BRUSH));

    SetDCPenColor(device_context, RGB(line_color.r, line_color.g, line_color.b));
    SetDCBrushColor(device_context, RGB(fill_color.r, fill_color.g, fill_color.b));

    Rectangle(device_context, x, y, x + width, y + height);
}