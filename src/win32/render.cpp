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

// Note: keep this in sync with the enum FontFamily in render.cpp
const wchar_t* font_families[] = { 
    L"Arial",
    L"Courier New"
};

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
    
    D2D1_RECT_F rectangle = D2D1::RectF(x, y, x + width,y + height);
    
    if (fill_color.a)
    {
        get_brush(fill_color, &fill_brush);
        render_target->FillRectangle(&rectangle, fill_brush);
        release_brush(fill_brush);
    }
    
    if (line_color.a)
    {
        get_brush(line_color, &line_brush);
        render_target->DrawRectangle(&rectangle, line_brush, line_width);
        release_brush(line_brush);
    }

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
    
    D2D1_RECT_F rectangle = D2D1::RectF(x, y, x + width,y + height);
    D2D1_ROUNDED_RECT rounded_rectangle = D2D1::RoundedRect(rectangle, r, r);
    
    if (fill_color.a)
    {
        get_brush(fill_color, &fill_brush);
        render_target->FillRoundedRectangle(&rounded_rectangle, fill_brush);
        release_brush(fill_brush);
    }
    
    if (line_color.a)
    {
        get_brush(line_color, &line_brush);
        render_target->DrawRoundedRectangle(&rounded_rectangle, line_brush, line_width);
        release_brush(line_brush);
    }
    
}

void draw_lane_segment(i32 left_top_x, i32 right_top_x, i32 top_y, 
                       i32 left_bottom_x, i32 right_bottom_x, i32 bottom_y, i32 radius,
                       Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    ID2D1PathGeometry * path_geometry = 0;
    ID2D1GeometrySink * sink = 0;
    
    // FIXME: shouldn't we have middle_y as argument?
    i32 middle_y = top_y + (bottom_y - top_y) / 2;
    
    D2D1_POINT_2F left_top = D2D1::Point2F(left_top_x, top_y);
    D2D1_POINT_2F left_top_down = D2D1::Point2F(left_top_x, middle_y - radius);
    D2D1_POINT_2F left_top_arc_end;
    D2D1_POINT_2F left_middle_line_end;
    D2D1_POINT_2F left_bottom_arc_end = D2D1::Point2F(left_bottom_x, middle_y + radius);
    D2D1_POINT_2F left_bottom = D2D1::Point2F(left_bottom_x, bottom_y);
    
    D2D1_ARC_SEGMENT left_top_arc_segment;
    D2D1_ARC_SEGMENT left_bottom_arc_segment;
    
    D2D1_POINT_2F right_bottom = D2D1::Point2F(right_bottom_x, bottom_y);
    D2D1_POINT_2F right_bottom_up = D2D1::Point2F(right_bottom_x, middle_y + radius);
    D2D1_POINT_2F right_bottom_arc_end;
    D2D1_POINT_2F right_middle_line_end;
    D2D1_POINT_2F right_top_arc_end = D2D1::Point2F(right_top_x, middle_y - radius);
    D2D1_POINT_2F right_top = D2D1::Point2F(right_top_x, top_y);
    
    D2D1_ARC_SEGMENT right_top_arc_segment;
    D2D1_ARC_SEGMENT right_bottom_arc_segment;
    
    b32 left_side_is_straight = false;
    b32 right_side_is_straight = false;
    if (left_bottom_x < left_top_x)
    {
        left_top_arc_end = D2D1::Point2F(left_top_x - radius, middle_y);
        left_middle_line_end = D2D1::Point2F(left_bottom_x + radius, middle_y);
        left_top_arc_segment = D2D1::ArcSegment(
            left_top_arc_end,
            D2D1::SizeF(radius, radius),
            90,
            D2D1_SWEEP_DIRECTION_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
        left_bottom_arc_segment = D2D1::ArcSegment(
            left_bottom_arc_end,
            D2D1::SizeF(radius, radius),
            180,
            D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
    }
    else if (left_bottom_x > left_top_x)
    {
        left_top_arc_end = D2D1::Point2F(left_top_x + radius, middle_y);
        left_middle_line_end = D2D1::Point2F(left_bottom_x - radius, middle_y);
        left_top_arc_segment = D2D1::ArcSegment(
            left_top_arc_end,
            D2D1::SizeF(radius, radius),
            90,
            D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
        left_bottom_arc_segment = D2D1::ArcSegment(
            left_bottom_arc_end,
            D2D1::SizeF(radius, radius),
            180,
            D2D1_SWEEP_DIRECTION_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
    }
    else {
        left_side_is_straight = true;
    }
    
    if (right_bottom_x < right_top_x)
    {
        right_bottom_arc_end = D2D1::Point2F(right_bottom_x + radius, middle_y);
        right_middle_line_end = D2D1::Point2F(right_top_x - radius, middle_y);
        right_bottom_arc_segment = D2D1::ArcSegment(
            right_bottom_arc_end,
            D2D1::SizeF(radius, radius),
            270,
            D2D1_SWEEP_DIRECTION_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
        right_top_arc_segment = D2D1::ArcSegment(
            right_top_arc_end,
            D2D1::SizeF(radius, radius),
            0,
            D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
    }
    else if (right_bottom_x > right_top_x)
    {
        right_bottom_arc_end = D2D1::Point2F(right_bottom_x - radius, middle_y);
        right_middle_line_end = D2D1::Point2F(right_top_x + radius, middle_y);
        right_bottom_arc_segment = D2D1::ArcSegment(
            right_bottom_arc_end,
            D2D1::SizeF(radius, radius),
            270,
            D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
        right_top_arc_segment = D2D1::ArcSegment(
            right_top_arc_end,
            D2D1::SizeF(radius, radius),
            0,
            D2D1_SWEEP_DIRECTION_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL  // means: smaller than 180 degrees
        );  
    }
    else {
        right_side_is_straight = true;
    }
    
    if (fill_color.a)
    {
        d2d_factory->CreatePathGeometry(&path_geometry);
        path_geometry->Open(&sink);
        sink->SetFillMode(D2D1_FILL_MODE_WINDING);
        
        // Left side
        sink->BeginFigure(left_top, D2D1_FIGURE_BEGIN_FILLED);
        if (!left_side_is_straight)
        {
            sink->AddLine(left_top_down);
            sink->AddArc(left_top_arc_segment);
            sink->AddLine(left_middle_line_end);
            sink->AddArc(left_bottom_arc_segment);
        }
        sink->AddLine(left_bottom);

        // Right side
        sink->AddLine(right_bottom);
        if (!right_side_is_straight)
        {
            sink->AddLine(right_bottom_up);
            sink->AddArc(right_bottom_arc_segment);
            sink->AddLine(right_middle_line_end);
            sink->AddArc(right_top_arc_segment);
        }
        sink->AddLine(right_top);
        
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
        sink->Release();
        
        get_brush(fill_color, &fill_brush);
        render_target->FillGeometry(path_geometry, fill_brush);
        release_brush(fill_brush);
        
        path_geometry->Release();
    }
    
    if (line_color.a)
    {
        // Left side
        d2d_factory->CreatePathGeometry(&path_geometry);
        path_geometry->Open(&sink);
        sink->SetFillMode(D2D1_FILL_MODE_WINDING);
        
        sink->BeginFigure(left_top, D2D1_FIGURE_BEGIN_FILLED);
        if (!left_side_is_straight)
        {
            sink->AddLine(left_top_down);
            sink->AddArc(left_top_arc_segment);
            sink->AddLine(left_middle_line_end);
            sink->AddArc(left_bottom_arc_segment);
        }
        sink->AddLine(left_bottom);
        
        sink->EndFigure(D2D1_FIGURE_END_OPEN);
        sink->Close();
        sink->Release();
        
        get_brush(line_color, &line_brush);
        render_target->DrawGeometry(path_geometry, line_brush, line_width);    
        release_brush(line_brush);
        
        path_geometry->Release();
        
        // Right side
        
        d2d_factory->CreatePathGeometry(&path_geometry);
        path_geometry->Open(&sink);
        sink->SetFillMode(D2D1_FILL_MODE_WINDING);
        
        sink->BeginFigure(right_bottom, D2D1_FIGURE_BEGIN_FILLED);
        if (!right_side_is_straight)
        {
            sink->AddLine(right_bottom_up);
            sink->AddArc(right_bottom_arc_segment);
            sink->AddLine(right_middle_line_end);
            sink->AddArc(right_top_arc_segment);
        }
        sink->AddLine(right_top);
        
        sink->EndFigure(D2D1_FIGURE_END_OPEN);
        sink->Close();
        sink->Release();
        
        get_brush(line_color, &line_brush);
        render_target->DrawGeometry(path_geometry, line_brush, line_width);    
        release_brush(line_brush);
        
        path_geometry->Release();
        
    }
    
}

// TODO: shouldn't we use radius x and radius y instead of using width and height?
void draw_ellipse(i32 x, i32 y, i32 width, i32 height, 
                  Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(x + (r32)width/(r32)2, y + (r32)height/(r32)2), (r32)width/(r32)2, (r32)height/(r32)2);
    
    if (fill_color.a)
    {
        get_brush(fill_color, &fill_brush);
        render_target->FillEllipse(&ellipse, fill_brush);
        release_brush(fill_brush);
    }
    if (line_color.a)
    {
        get_brush(line_color, &line_brush);
        render_target->DrawEllipse(&ellipse, line_brush, line_width);
        release_brush(line_brush);
    }
    
}

void draw_text(i32 x, i32 y, ShortString * text, Font font, Color4 font_color)
{
    ID2D1SolidColorBrush * font_brush = 0;
    get_brush(font_color, &font_brush);

    u16 wide_text[MAX_LENGTH_SHORT_STRING];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCCH)text->data, text->length, (LPWSTR)wide_text, MAX_LENGTH_SHORT_STRING);

    IDWriteTextFormat * text_format;
    
    // FIXME check result
    // TODO: shouldn't we release the text format? Do we want to (re)create it here every time?
    HRESULT text_format_result = direct_write_factory->CreateTextFormat(
            font_families[font.family],
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            font.height * 1.3, // TODO: see canvas.js: we use this constant now
            L"", //locale
            &text_format
    );
            
    render_target->DrawText(
            (LPWSTR)wide_text,
            text->length,
            text_format,
            D2D1::RectF(x, y, x + 200, y + 100), // FIXME: how to determine size of text?
            font_brush
    );
            
    release_brush(font_brush);
}

void draw_text_c(i32 x, i32 y, const char * cstring, Font font, Color4 font_color)
{
    ShortString text;
    copy_cstring_to_short_string(cstring, &text);
    draw_text(x, y, &text, font, font_color);
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
                window_rect.bottom - window_rect.top)
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

inline LARGE_INTEGER get_clock_counter(void);

void render_d2d(HWND window)
{
    if (render_target)
    {
        render_target->BeginDraw();

        render_target->Clear( D2D1::ColorF(D2D1::ColorF::White) );

        render_frame();

        // This is a workaround to measure the drawing time and NOT count 
        // the waiting time (until vsync). The EndDraw waits for the vsync so 
        // we can't measure time AFTER that. That's why we do a Flush first.
        render_target->Flush();
        clock_counter_after_render = get_clock_counter();
        
        // TODO: check result
        render_target->EndDraw();
    }
}

