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

struct blend_info
{
    HDC dc_to_draw_to;
    HBITMAP alpha_buffer_bitmap;
    BLENDFUNCTION blend_function;
    
    color4 color;
    i32 x;
    i32 y;
    i32 width;
    i32 height;
};

blend_info init_blend(i32 x, i32 y, i32 width, i32 height, color4 color)
{
    blend_info blend_info = {};
    
    blend_info.color = color;
    
    if (blend_info.color.a != 255)
    {
        blend_info.x = x;
        blend_info.y = y;
        blend_info.width = width;
        blend_info.height = height;
        
        blend_info.dc_to_draw_to = CreateCompatibleDC(backbuffer_dc);

        blend_info.blend_function.BlendOp = AC_SRC_OVER;
        blend_info.blend_function.BlendFlags = 0;
        blend_info.blend_function.SourceConstantAlpha = blend_info.color.a;
        blend_info.blend_function.AlphaFormat = 0;

        // FIXME: what if width or height are bigger than the width or height of the backbuffer?
        blend_info.alphabuffer_bitmap = CreateCompatibleBitmap(backbuffer_dc, blend_info.width, blend_info.height);
        
        SelectObject(blend_info.dc_to_draw_to, blend_info.alphabuffer_bitmap);
    }
    else {
        blend_info.dc = backbuffer_dc;
    }
    
    return blend_info;
}

void end_blend(blend_info * blend_info)
{
    if (blend_info->color.a != 255)
    {
        AlphaBlend(backbuffer_dc, blend_info->x, blend_info->y, blend_info->width, blend_info->height, 
                   blend_info->dc, 0, 0, blend_info->width, blend_info->height, 
                   blend_info->blend_function);

        DeleteObject(blend_info->bitmap);
        DeleteDC(blend_info->dc);
    }
    else
    {
        // Nothing to do when not blending
    }
}

void draw_rounded_rectangle(i32 x, i32 y, i32 width, i32 height, i32 r, color4 line_color, color4 fill_color, i32 line_width)
{
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));

    if (fill_color.a != 255)
    {
        // FIXME: implement rounded rectangle with alpha using dc = start_alpha_blend() and end_alpha_blend() functions
    }
    else
    {
        SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
        SelectObject(backbuffer_dc, brush);

        Rectangle(backbuffer_dc, x, y, x + width, y + height);
    }
    
    if (line_color.a != 255)
    {
        // FIXME: implement rounded rectangle with alpha using:
        //            blend_info = init_alpha_blend(line_color, width, height) 
        //            dc = blend_info.dc (which could be the back buffer or the alpha buffer)
        //            THEN: draw something
        //            end_alpha_blend(blend_info)
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

void draw_rectangle(i32 x, i32 y, i32 width, i32 height, color4 line_color, color4 fill_color, i32 line_width)
{
    // FIXME: when doing alpha, take into account the line_width makes the reactangle bigger!
    
    // FIXME: when setting the alpha of the pen, the entire rectangle get darker!

    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    
    // GetStockObject(NULL_BRUSH)
    // GetStockObject(NULL_PEN)
    
    blend_info = init_blend(x, y, width, height, fill_color);
    {
        SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
        SelectObject(backbuffer_dc, brush);

        Rectangle(backbuffer_dc, x, y, x + width, y + height);
    }
    end_blend(&blend_info);

    blend_info = init_blend(x, y, width, height, line_color);
    {
        SelectObject(backbuffer_dc, pen);
        SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

        Rectangle(backbuffer_dc, x, y, x + width, y + height);
    }
    end_blend(&blend_info);
    
/*
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
    */
    
    /*
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
    */
    
    DeleteObject(pen);
    DeleteObject(brush);
}

void draw_text(i32 x, i32 y, short_string * text, i32 font_height, color4 font_color)
{
    HFONT hFont = (HFONT)GetStockObject(ANSI_VAR_FONT); 
    SelectObject(backbuffer_dc, hFont);
    
    SetBkMode(backbuffer_dc, TRANSPARENT);
    SetTextColor(backbuffer_dc, RGB(font_color.r, font_color.g, font_color.b));

    TextOut(backbuffer_dc, x, y, (LPCSTR)text->data, text->length);
}

void draw_text_c(i32 x, i32 y, const char * cstring, i32 font_height, color4 font_color)
{
    short_string text;
    copy_cstring_to_short_string(cstring, &text);
    draw_text(x, y, &text, font_height, font_color);
}

void log(u8 * text)
{
    OutputDebugStringA((LPCSTR)text);
}