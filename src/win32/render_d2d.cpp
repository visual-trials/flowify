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
 
 
 // FIXME: don't we want to pass two positions to most of these functions? Instead of pos + size?
 
 // FIXME: don't we want r32 for colors?
 
// TODO: make this inline
void get_brush(Color4 color, ID2D1SolidColorBrush ** brush)
{
    render_target->CreateSolidColorBrush(D2D1::ColorF(
        (r32)color.r/(r32)255, 
        (r32)color.g/(r32)255, 
        (r32)color.b/(r32)255, 
        (r32)color.a/(r32)255
    ), brush);
};
 
void draw_rectangle(i32 x, i32 y, i32 width, i32 height, Color4 line_color, Color4 fill_color, i32 line_width)
{

    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    get_brush(line_color, &line_brush);
    get_brush(fill_color, &fill_brush);
    
    D2D1_RECT_F rectangle = D2D1::RectF(x + 0.5, y + 0.5, x + width + 0.5,y + height + 0.5);
    
    render_target->FillRectangle(&rectangle, fill_brush);
    render_target->DrawRectangle(&rectangle, line_brush);

    // FIXME: use the template!
    if (line_brush)
    {
        line_brush->Release();
    }
    
    if (fill_brush)
    {
        fill_brush->Release();
    }
    
    /*
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    
    SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
    SelectObject(backbuffer_dc, brush);

    Rectangle(backbuffer_dc, x, y, x + width, y + height);

    SelectObject(backbuffer_dc, pen);
    SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

    Rectangle(backbuffer_dc, x, y, x + width, y + height);
    
    DeleteObject(pen);
    DeleteObject(brush);
    */
}

void draw_line(i32 x_start, i32 y_start, i32 x_end, i32 y_end, Color4 line_color, i32 line_width)
{
    /*
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    
    SelectObject(backbuffer_dc, pen);
    SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

    MoveToEx(backbuffer_dc, x_start, y_start, (LPPOINT) NULL); 
    LineTo(backbuffer_dc, x_end, y_end); 
    
    DeleteObject(pen);
    */
}

void draw_rounded_rectangle(i32 x, i32 y, i32 width, i32 height, i32 r, Color4 line_color, Color4 fill_color, i32 line_width)
{
    /*
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    
    i32 d = r * 2;

    SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
    SelectObject(backbuffer_dc, brush);

    RoundRect(backbuffer_dc, x, y, x + width, y + height, d, d);
    
    SelectObject(backbuffer_dc, pen);
    SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

    RoundRect(backbuffer_dc, x, y, x + width, y + height, d, d);
    
    DeleteObject(pen);
    DeleteObject(brush);
    */
}

void draw_ellipse(i32 x, i32 y, i32 width, i32 height, 
                  Color4 line_color, Color4 fill_color, i32 line_width)
{
/*    
    HPEN pen = CreatePen(PS_SOLID, line_width, RGB(line_color.r, line_color.g, line_color.b));
    HBRUSH brush = CreateSolidBrush(RGB(fill_color.r, fill_color.g, fill_color.b));
    
    SelectObject(backbuffer_dc, GetStockObject(NULL_PEN));
    SelectObject(backbuffer_dc, brush);

    Ellipse(backbuffer_dc, x, y, x + width, y + height);
    
    SelectObject(backbuffer_dc, pen);
    SelectObject(backbuffer_dc, GetStockObject(NULL_BRUSH));

    Ellipse(backbuffer_dc, x, y, x + width, y + height);
    
    DeleteObject(pen);
    
    DeleteObject(brush);
    */
}

void draw_text(i32 x, i32 y, ShortString * text, i32 font_height, Color4 font_color)
{
    /*
    HFONT hFont = (HFONT)GetStockObject(ANSI_VAR_FONT); 
    SelectObject(backbuffer_dc, hFont);
    
    SetBkMode(backbuffer_dc, TRANSPARENT);
    SetTextColor(backbuffer_dc, RGB(font_color.r, font_color.g, font_color.b));

    TextOut(backbuffer_dc, x, y, (LPCSTR)text->data, text->length);
    */
}

void draw_text_c(i32 x, i32 y, const char * cstring, i32 font_height, Color4 font_color)
{
    ShortString text;
    copy_cstring_to_short_string(cstring, &text);
    draw_text(x, y, &text, font_height, font_color);
}

void log(u8 * text)
{
    OutputDebugStringA((LPCSTR)text);
}