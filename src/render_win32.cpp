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

void draw_rectangle(i32 x, i32 y, i32 width, i32 height, color4 line_color, color4 fill_color, i32 line_width)
{
    // FIXME: when doing alpha, take into account the line_width makes the reactangle bigger!
    
    // FIXME: when setting the alpha of the pen, the entire rectangle get darker!

    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    
    // GetStockObject(NULL_BRUSH)
    // GetStockObject(NULL_PEN)
    
    if (fill_color.a != 255)
    {
        HDC alphabuffer_dc = CreateCompatibleDC(backbuffer_dc);

        BLENDFUNCTION blend;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = fill_color.a;
        blend.AlphaFormat = 0;

        // FIXME: what if width or height are bigger than the width or height of the backbuffer?
        HBITMAP alphabuffer_bitmap = CreateCompatibleBitmap(backbuffer_dc, width, height);
        
        SelectObject(alphabuffer_dc, alphabuffer_bitmap);

        // Drawing to alphabuffer
        {
            SelectObject(alphabuffer_dc, GetStockObject(NULL_PEN));
            SelectObject(alphabuffer_dc, brush);

            Rectangle(alphabuffer_dc, 0, 0, width, height);
        }
        
        AlphaBlend(backbuffer_dc, x, y, width, height, alphabuffer_dc, 0, 0, width, height, blend);

        DeleteObject(alphabuffer_bitmap);
        DeleteDC(alphabuffer_dc);
    }
    else
    {
        SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
        SelectObject(backbuffer_dc, brush);

        Rectangle(backbuffer_dc, x, y, x + width, y + height);
    }
    
    if (line_color.a != 255)
    {
        HDC alphabuffer_dc = CreateCompatibleDC(backbuffer_dc);

        BLENDFUNCTION blend;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = fill_color.a;
        blend.AlphaFormat = 0;

        // FIXME: what if width or height are bigger than the width or height of the backbuffer?
        
        // FIXME: take into account the width of the line! So your rectangle exactly that much bigger (width aswell as height)
        //        and you should start to draw from half of the thickness (x and y)
        //        Problem: rounding issues (either with even or odd line_widths)
        HBITMAP alphabuffer_bitmap = CreateCompatibleBitmap(backbuffer_dc, width, height);
        
        SelectObject(alphabuffer_dc, alphabuffer_bitmap);

        // Drawing to alphabuffer
        {
            SelectObject(alphabuffer_dc, pen);
            SelectObject(alphabuffer_dc, GetStockObject(NULL_BRUSH));

            Rectangle(alphabuffer_dc, 0, 0, width, height);
        }
        
        AlphaBlend(backbuffer_dc, x, y, width, height, alphabuffer_dc, 0, 0, width, height, blend);

        DeleteObject(alphabuffer_bitmap);
        DeleteDC(alphabuffer_dc);
    }
    else
    {
        SelectObject(backbuffer_dc, pen);
        SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

        Rectangle(backbuffer_dc, x, y, x + width, y + height);
    }
    
    DeleteObject(pen);
    DeleteObject(brush);
}