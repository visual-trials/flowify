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
 

/*
    TODO: we should use something else than GDI to render with alpha
    
    We are using basic GDI here. But alpha-blending is not really supported in GDI.
    To make blending work anyway we now use a workaround: an alpha buffer AND a backup buffer.
    Note: we only have to use this backup buffer when we are not drawing a pure rectangle.
    
    The procedure is as follows:
    
    1) We make a backup of the area we are going to draw/blend into (this goes into the backup buffer)
    2) We draw into the alpha buffer
    3) We then use AlphaBlend to blend our drawing to the back buffer.
           Note: The problem is that AlphaBlend has a 24 bpp source bitmap 
                 and cannot know which parts it shouln't blend. This causes it
                 to blend parts we didn't draw into. We need to restore those.
    4) We draw the same figure to the backup buffer, but set our brush/pen to WHITE
    5) We use TransparentBlt to copy back (aka restore) the pixels that are not WHITE
        Note: By doing this we restored most of the parts we blended incorrectly to
              But if the original image had white pixel in them, we didn't restore those pixel.
              So we have to use another color and do the same.
    6) We draw the same figure to the backup buffer, but set our brush/pen to BLACK
    7) We use TransparentBlt to copy back (aka restore) the pixels that are not BLACK
    
*/

struct blend_info
{
    HDC dc;
    HBITMAP alpha_bitmap;
    BLENDFUNCTION blend_function;
    
    b32 make_backup;
    HDC backup_dc;
    HBITMAP backup_bitmap;
    
    color4 color;
    i32 x;
    i32 y;
    i32 width;
    i32 height;
};

void init_blend(i32 x, i32 y, i32 width, i32 height, color4 color, blend_info * blend_info, b32 make_backup)
{
    
    blend_info->color = color;
    blend_info->x = x;
    blend_info->y = y;
    blend_info->width = width;
    blend_info->height = height;
    
    blend_info->make_backup = make_backup;
    
    blend_info->dc = CreateCompatibleDC(backbuffer_dc);

    blend_info->blend_function.BlendOp = AC_SRC_OVER;
    blend_info->blend_function.BlendFlags = 0;
    blend_info->blend_function.SourceConstantAlpha = blend_info->color.a;
    blend_info->blend_function.AlphaFormat = 0;

    // FIXME: what if width or height are bigger than the width or height of the backbuffer?
    blend_info->alpha_bitmap = CreateCompatibleBitmap(backbuffer_dc, blend_info->width, blend_info->height);
    
    SelectObject(blend_info->dc, blend_info->alpha_bitmap);
    
    if (make_backup)
    {
        blend_info->backup_dc = CreateCompatibleDC(backbuffer_dc);
        
        // FIXME: what if width or height are bigger than the width or height of the backbuffer?
        blend_info->backup_bitmap = CreateCompatibleBitmap(backbuffer_dc, blend_info->width, blend_info->height);
        
        SelectObject(blend_info->backup_dc, blend_info->backup_bitmap);
        
        // Creating a backup of the part of the image we are going to alpha blend into
        // TODO: we should check the result of BitBlt and log if something goes wrong
        BitBlt(blend_info->backup_dc, 0, 0, blend_info->width, blend_info->height, backbuffer_dc, blend_info->x, blend_info->y, SRCCOPY);
    }
}

void do_blend(blend_info * blend_info)
{
//    BitBlt(backbuffer_dc, blend_info->x, blend_info->y, blend_info->width, blend_info->height, 
//           blend_info->backup_dc, 0, 0, SRCCOPY);
    // TODO: we should check the result of AlphaBlend and log if something goes wrong
    AlphaBlend(backbuffer_dc, blend_info->x, blend_info->y, blend_info->width, blend_info->height, 
               blend_info->dc, 0, 0, blend_info->width, blend_info->height, 
               blend_info->blend_function);
}

void retore_blend_white(blend_info * blend_info)
{
    // TODO: we should check the result of TransparentBlt and log if something goes wrong
    TransparentBlt(backbuffer_dc, blend_info->x, blend_info->y, blend_info->width, blend_info->height,
                   blend_info->backup_dc, 0, 0, blend_info->width, blend_info->height,
                   RGB(255, 255, 255));
}

void retore_blend_black(blend_info * blend_info)
{
    // TODO: we should check the result of TransparentBlt and log if something goes wrong
    TransparentBlt(backbuffer_dc, blend_info->x, blend_info->y, blend_info->width, blend_info->height,
                   blend_info->backup_dc, 0, 0, blend_info->width, blend_info->height,
                   RGB(0, 0, 0));
}

void end_blend(blend_info * blend_info)
{
    if (blend_info->make_backup)
    {
        DeleteObject(blend_info->backup_bitmap);
        DeleteDC(blend_info->backup_dc);
    }
    DeleteObject(blend_info->alpha_bitmap);
    DeleteDC(blend_info->dc);
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
        b32 make_backup = false;
        init_blend(x_blend, y_blend, width, height, fill_color, &blend_info, make_backup);
        {
            SelectObject(blend_info.dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.dc, brush);

            Rectangle(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            do_blend(&blend_info);
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
        b32 make_backup = false;
        init_blend(x_blend, y_blend, width, height, line_color, &blend_info, make_backup);
        {
            SelectObject(blend_info.dc, pen);
            SelectObject(blend_info.dc, GetStockObject(NULL_BRUSH));

            Rectangle(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            do_blend(&blend_info);
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

void draw_rounded_rectangle(i32 x, i32 y, i32 width, i32 height, i32 r, color4 line_color, color4 fill_color, i32 line_width)
{
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HPEN black_pen = CreatePen(PS_SOLID, line_width, RGB(0, 0, 0));
    HPEN white_pen = CreatePen(PS_SOLID, line_width, RGB(255, 255, 255));
    
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH white_brush = CreateSolidBrush(RGB(255, 255, 255));
    
    i32 d = r * 2;

    if (fill_color.a != 255)
    {
        blend_info blend_info;
        i32 x_blend = x;
        i32 y_blend = y;
        b32 make_backup = true;
        init_blend(x_blend, y_blend, width, height, fill_color, &blend_info, make_backup);
        {
            SelectObject(blend_info.dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.dc, brush);

            RoundRect(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
            
            do_blend(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.backup_dc, white_brush);

            RoundRect(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
            
            retore_blend_white(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.backup_dc, black_brush);

            RoundRect(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
            
            retore_blend_black(&blend_info);
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
        b32 make_backup = true;
        init_blend(x_blend, y_blend, width, height, line_color, &blend_info, make_backup);
        {
            SelectObject(blend_info.dc, pen);
            SelectObject(blend_info.dc, GetStockObject(NULL_BRUSH));

            RoundRect(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
            
            do_blend(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, white_pen);
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_BRUSH));

            RoundRect(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
            
            retore_blend_white(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, black_pen);
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_BRUSH));

            RoundRect(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend, d, d);
            
            retore_blend_black(&blend_info);
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

void draw_ellipse(i32 x, i32 y, i32 width, i32 height, 
                  color4 line_color, color4 fill_color, i32 line_width)
{
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HPEN black_pen = CreatePen(PS_SOLID, line_width, RGB(0, 0, 0));
    HPEN white_pen = CreatePen(PS_SOLID, line_width, RGB(255, 255, 255));
    
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH white_brush = CreateSolidBrush(RGB(255, 255, 255));
    
    if (fill_color.a != 255)
    {
        blend_info blend_info;
        i32 x_blend = x;
        i32 y_blend = y;
        b32 make_backup = true;
        init_blend(x_blend, y_blend, width, height, fill_color, &blend_info, make_backup);
        {
            SelectObject(blend_info.dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.dc, brush);

            Ellipse(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            do_blend(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.backup_dc, white_brush);

            Ellipse(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            retore_blend_white(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_PEN));
            SelectObject(blend_info.backup_dc, black_brush);

            Ellipse(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            retore_blend_black(&blend_info);
        }
        end_blend(&blend_info);
    }
    else {
        SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
        SelectObject(backbuffer_dc, brush);

        Ellipse(backbuffer_dc, x, y, x + width, y + height);
    }
    
    if (line_color.a != 255)
    {
        blend_info blend_info;
        i32 x_blend = x;
        i32 y_blend = y;
        b32 make_backup = true;
        init_blend(x_blend, y_blend, width, height, line_color, &blend_info, make_backup);
        {
            SelectObject(blend_info.dc, pen);
            SelectObject(blend_info.dc, GetStockObject(NULL_BRUSH));

            Ellipse(blend_info.dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            do_blend(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, white_pen);
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_BRUSH));

            Ellipse(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            retore_blend_white(&blend_info);
        }
        {
            SelectObject(blend_info.backup_dc, black_pen);
            SelectObject(blend_info.backup_dc, GetStockObject(NULL_BRUSH));

            Ellipse(blend_info.backup_dc, x - x_blend, y - y_blend, x + width- x_blend, y + height - y_blend);
            
            retore_blend_black(&blend_info);
        }
        end_blend(&blend_info);
    }
    else {
        SelectObject(backbuffer_dc, pen);
        SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

        Ellipse(backbuffer_dc, x, y, x + width, y + height);
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