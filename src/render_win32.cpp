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
    HDC dc;
    HBITMAP alpha_bitmap;
    BLENDFUNCTION blend_function;
    
    color4 color;
    i32 x;
    i32 y;
    i32 width;
    i32 height;
};

void init_blend(i32 x, i32 y, i32 width, i32 height, color4 color, blend_info * blend_info)
{
    
    blend_info->color = color;
    blend_info->x = x;
    blend_info->y = y;
    blend_info->width = width;
    blend_info->height = height;
    
    blend_info->dc = CreateCompatibleDC(backbuffer_dc);

    blend_info->blend_function.BlendOp = AC_SRC_OVER;
    blend_info->blend_function.BlendFlags = 0;
    blend_info->blend_function.SourceConstantAlpha = blend_info->color.a;
    blend_info->blend_function.AlphaFormat = 0;

    // FIXME: what if width or height are bigger than the width or height of the backbuffer?
    blend_info->alpha_bitmap = CreateCompatibleBitmap(backbuffer_dc, blend_info->width, blend_info->height);
    
    SelectObject(blend_info->dc, blend_info->alpha_bitmap);
}

void end_blend(blend_info * blend_info)
{
    AlphaBlend(backbuffer_dc, blend_info->x, blend_info->y, blend_info->width, blend_info->height, 
               blend_info->dc, 0, 0, blend_info->width, blend_info->height, 
               blend_info->blend_function);

    DeleteObject(blend_info->alpha_bitmap);
    DeleteDC(blend_info->dc);
}

void draw_rounded_rectangle(i32 x, i32 y, i32 width, i32 height, i32 r, color4 line_color, color4 fill_color, i32 line_width)
{
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    
    i32 d = r * 2;

    if (fill_color.a != 255)
    {
        blend_info blend_info;
        i32 x_blend = x;
        i32 y_blend = y;
        init_blend(x_blend, y_blend, width, height, fill_color, &blend_info);
        {
            SelectObject(blend_info.dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.dc, brush);

            RoundRect(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
        }
        end_blend(&blend_info);
    }
    else
    {
        SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
        SelectObject(backbuffer_dc, brush);

        RoundRect(backbuffer_dc, x, y, x + width, y + height, d, d);
    }
    
    if (line_color.a != 255)
    {
        blend_info blend_info;
        i32 x_blend = x;
        i32 y_blend = y;
        init_blend(x_blend, y_blend, width, height, line_color, &blend_info);
        {
            SelectObject(blend_info.dc, pen);
            SelectObject(blend_info.dc, GetStockObject(NULL_BRUSH));

            RoundRect(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
        }
        end_blend(&blend_info);
    }
    else
    {
        SelectObject(backbuffer_dc, pen);
        SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

        RoundRect(backbuffer_dc, x, y, x + width, y + height, d, d);
    }
    
    DeleteObject(pen);
    DeleteObject(brush);
}

void draw_rectangle(i32 x, i32 y, i32 width, i32 height, color4 line_color, color4 fill_color, i32 line_width)
{
    // FIXME: when doing alpha, take into account the line_width makes the reactangle bigger!
    
    // FIXME: when setting the alpha of the pen, the entire rectangle get darker!
    
    // FIXME: when not drawing the line (only the fill), you can see one pixel-line at the bottom and right are not blending right!

    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    
    if (fill_color.a != 255)
    {
        blend_info blend_info;
        i32 x_blend = x;
        i32 y_blend = y;
        init_blend(x_blend, y_blend, width, height, fill_color, &blend_info);
        {
            SelectObject(blend_info.dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.dc, brush);

            Rectangle(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
        }
        end_blend(&blend_info);
    }
    else {
        SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
        SelectObject(backbuffer_dc, brush);

        Rectangle(backbuffer_dc, x, y, x + width, y + height);
    }
    
    if (line_color.a != 255)
    {
        blend_info blend_info;
        i32 x_blend = x;
        i32 y_blend = y;
        init_blend(x_blend, y_blend, width, height, line_color, &blend_info);
        {
            SelectObject(blend_info.dc, pen);
            SelectObject(blend_info.dc, GetStockObject(NULL_BRUSH));

            Rectangle(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
        }
        end_blend(&blend_info);
    }
    else {
        SelectObject(backbuffer_dc, pen);
        SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

        Rectangle(backbuffer_dc, x, y, x + width, y + height);
    }
    
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