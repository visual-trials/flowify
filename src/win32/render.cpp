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
 
#include <d2d1.h>
#include <dwrite.h>

ID2D1HwndRenderTarget * render_target;
ID2D1Factory * d2d_factory;
IDWriteFactory * direct_write_factory;


// TODO: make this inline
void release_brush(ID2D1SolidColorBrush * brush)
{
    if (brush)
    {
        brush->Release();
        brush = 0;
    }
}

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
 
// TODO: don't we want to pass two positions to most of these functions? Instead of pos + size?
// TODO: don't we want r32 for colors?
 
void draw_rectangle(i32 x, i32 y, i32 width, i32 height, Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    get_brush(line_color, &line_brush);
    get_brush(fill_color, &fill_brush);
    
    D2D1_RECT_F rectangle = D2D1::RectF(x + 0.5, y + 0.5, x + width + 0.5,y + height + 0.5);
    
    render_target->FillRectangle(&rectangle, fill_brush);
    render_target->DrawRectangle(&rectangle, line_brush, line_width);

    release_brush(line_brush);
    release_brush(fill_brush);
}

void draw_line(i32 x_start, i32 y_start, i32 x_end, i32 y_end, Color4 line_color, i32 line_width)
{
    D2D1_POINT_2F start_position, end_position;
    ID2D1SolidColorBrush * line_brush = 0;
    
	start_position.x = x_start;
	start_position.y = y_start;
    
	end_position.x = x_end;
	end_position.y = y_end;
    
    get_brush(line_color, &line_brush);
    
    render_target->DrawLine(start_position, end_position, line_brush, line_width);
    
    release_brush(line_brush);
}

void draw_rounded_rectangle(i32 x, i32 y, i32 width, i32 height, i32 r, Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    get_brush(line_color, &line_brush);
    get_brush(fill_color, &fill_brush);
    
    D2D1_RECT_F rectangle = D2D1::RectF(x + 0.5, y + 0.5, x + width + 0.5,y + height + 0.5);
    D2D1_ROUNDED_RECT rounded_rectangle = D2D1::RoundedRect(rectangle, r, r);
    
    render_target->FillRoundedRectangle(&rounded_rectangle, fill_brush);
    render_target->DrawRoundedRectangle(&rounded_rectangle, line_brush, line_width);

    release_brush(line_brush);
    release_brush(fill_brush);
}

// TODO: shouldn't we use radius x and radius y instead of using width and height?
void draw_ellipse(i32 x, i32 y, i32 width, i32 height, 
                  Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    get_brush(line_color, &line_brush);
    get_brush(fill_color, &fill_brush);
    
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(x + 0.5 + (r32)width/(r32)2, y + 0.5 + (r32)height/(r32)2), (r32)width/(r32)2, (r32)height/(r32)2);
    
    render_target->FillEllipse(&ellipse, fill_brush);
    render_target->DrawEllipse(&ellipse, line_brush, line_width);
    
    release_brush(line_brush);
    release_brush(fill_brush);
}

void draw_text(i32 x, i32 y, ShortString * text, i32 font_height, Color4 font_color)
{
    ID2D1SolidColorBrush * font_brush = 0;
    get_brush(font_color, &font_brush);

    u16 wide_text[MAX_LENGTH_SHORT_STRING];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCCH)text->data, text->length, (LPWSTR)wide_text, MAX_LENGTH_SHORT_STRING);

    IDWriteTextFormat * text_format;
    
    // FIXME check result
    // TODO: shouldn't we release the text format? Do we want to (re)create it here every time?
    HRESULT text_format_result = direct_write_factory->CreateTextFormat(
            L"Arial",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            font_height * 1.3, // TODO: see canvas.js: we use this constant now
            L"", //locale
            &text_format
    );
            
    render_target->DrawText(
            (LPWSTR)wide_text,
            text->length,
            text_format,
            D2D1::RectF(x + 0.5, y + 0.5, x + 0.5 + 200, y + 0.5 + 100), // FIXME: how to determine size of text?
            font_brush
    );
            
    release_brush(font_brush);
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

// Init functions

b32 create_factory_d2d()
{
    d2d_factory = 0;
    HRESULT d2d_factory_result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
    if (FAILED(d2d_factory_result))
    {
        return false;
    }
    return true;
}

b32 create_write_factory_d2d()
{
    HRESULT direct_write_factory_result = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(direct_write_factory),
            reinterpret_cast<IUnknown **>(&direct_write_factory)
    );
    if (FAILED(direct_write_factory_result))
    {
        return false;
    }
    return true;
}

void create_render_target_d2d(HWND window)
{
    RECT window_rect;
    GetClientRect(window, &window_rect);
    
    render_target = 0;
    // TODO: check render_target_result and return true or false accordingly
    HRESULT render_target_result = d2d_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            window,
            D2D1::SizeU(
                window_rect.right - window_rect.left,
                window_rect.bottom - window_rect.top),
            D2D1_PRESENT_OPTIONS_IMMEDIATELY
        ),
        &render_target
    );
}

void resize_d2d(HWND window)
{
    // TODO: we probably don't want to resize by re-creating the render_target each time!
    if (render_target)
    {
        render_target->Release();

        create_render_target_d2d(window);
    }
}

b32 init_d2d(HWND window)
{
    if (!create_factory_d2d())
    {
        return false;
    }
    
    if(!create_write_factory_d2d())
    {
        return false;
    }
    
    create_render_target_d2d(window);
    return true;
}    

void uninit_d2d()
{
    if (render_target)
    {
        render_target->Release();
        render_target = 0;
    }
    if (d2d_factory)
    {
        d2d_factory->Release();
        d2d_factory = 0;
    }
    if (direct_write_factory)
    {
        direct_write_factory->Release();
        direct_write_factory = 0;
    }
}


extern "C" {
    
void render_frame();

}

void render_d2d(HWND window)
{
    if (render_target)
    {
        render_target->BeginDraw();

        render_target->Clear( D2D1::ColorF(D2D1::ColorF::White) );

        render_frame();

        // TODO: check result
        render_target->EndDraw();
        
    }
}

