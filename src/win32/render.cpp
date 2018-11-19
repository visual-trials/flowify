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
        (f32)color.r/(f32)255, 
        (f32)color.g/(f32)255, 
        (f32)color.b/(f32)255, 
        (f32)color.a/(f32)255
    ), brush);
};
 
// TODO: don't we want to pass two positions to most of these functions? Instead of pos + size?
// TODO: don't we want f32 for colors?
 
void draw_rectangle(Pos2d position, Size2d size, Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    D2D1_RECT_F rectangle = D2D1::RectF(position.x, position.y, position.x + size.width, position.y + size.height);
    
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

void draw_line(Pos2d start_position, Pos2d end_position, Color4 line_color, i32 line_width)
{
    D2D1_POINT_2F start_pos, end_pos;
    ID2D1SolidColorBrush * line_brush = 0;
    
	start_pos.x = start_position.x;
	start_pos.y = start_position.y;
    
	end_pos.x = end_position.x;
	end_pos.y = end_position.y;
    
    get_brush(line_color, &line_brush);
    
    render_target->DrawLine(start_pos, end_pos, line_brush, line_width);
    
    release_brush(line_brush);
}

void draw_rounded_rectangle(Pos2d position, Size2d size, i32 r, Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    D2D1_RECT_F rectangle = D2D1::RectF(position.x, position.y, position.x + size.width, position.y + size.height);
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

void draw_lane_segment(Pos2d left_top_position, Pos2d right_top_position, 
                       Pos2d left_bottom_position, Pos2d right_bottom_position, i32 radius,
                       Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    ID2D1PathGeometry * path_geometry = 0;
    ID2D1GeometrySink * sink = 0;
    
    // FIXME: shouldn't we have middle_y as argument?
    i32 middle_y = left_top_position.y + (left_bottom_position.y - left_top_position.y) / 2;
    
    D2D1_POINT_2F left_top = D2D1::Point2F(left_top_position.x, left_top_position.y);
    D2D1_POINT_2F left_top_down = D2D1::Point2F(left_top_position.x, middle_y - radius);
    D2D1_POINT_2F left_top_arc_end;
    D2D1_POINT_2F left_middle_line_end;
    D2D1_POINT_2F left_bottom_arc_end = D2D1::Point2F(left_bottom_position.x, middle_y + radius);
    D2D1_POINT_2F left_bottom = D2D1::Point2F(left_bottom_position.x, left_bottom_position.y);
    
    D2D1_ARC_SEGMENT left_top_arc_segment;
    D2D1_ARC_SEGMENT left_bottom_arc_segment;
    
    D2D1_POINT_2F right_bottom = D2D1::Point2F(right_bottom_position.x, left_bottom_position.y);
    D2D1_POINT_2F right_bottom_up = D2D1::Point2F(right_bottom_position.x, middle_y + radius);
    D2D1_POINT_2F right_bottom_arc_end;
    D2D1_POINT_2F right_middle_line_end;
    D2D1_POINT_2F right_top_arc_end = D2D1::Point2F(right_top_position.x, middle_y - radius);
    D2D1_POINT_2F right_top = D2D1::Point2F(right_top_position.x, left_top_position.y);
    
    D2D1_ARC_SEGMENT right_top_arc_segment;
    D2D1_ARC_SEGMENT right_bottom_arc_segment;
    
    b32 left_side_is_straight = false;
    b32 right_side_is_straight = false;
    if (left_bottom_position.x < left_top_position.x)
    {
        left_top_arc_end = D2D1::Point2F(left_top_position.x - radius, middle_y);
        left_middle_line_end = D2D1::Point2F(left_bottom_position.x + radius, middle_y);
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
    else if (left_bottom_position.x > left_top_position.x)
    {
        left_top_arc_end = D2D1::Point2F(left_top_position.x + radius, middle_y);
        left_middle_line_end = D2D1::Point2F(left_bottom_position.x - radius, middle_y);
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
    
    if (right_bottom_position.x < right_top_position.x)
    {
        right_bottom_arc_end = D2D1::Point2F(right_bottom_position.x + radius, middle_y);
        right_middle_line_end = D2D1::Point2F(right_top_position.x - radius, middle_y);
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
    else if (right_bottom_position.x > right_top_position.x)
    {
        right_bottom_arc_end = D2D1::Point2F(right_bottom_position.x - radius, middle_y);
        right_middle_line_end = D2D1::Point2F(right_top_position.x + radius, middle_y);
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
void draw_ellipse(Pos2d position, Size2d size, 
                  Color4 line_color, Color4 fill_color, i32 line_width)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(position.x + (f32)size.width/(f32)2, position.y + (f32)size.height/(f32)2), (f32)size.width/(f32)2, (f32)size.height/(f32)2);
    
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

Size2d get_text_size(ShortString * text, Font font)
{
    u16 wide_text[MAX_LENGTH_SHORT_STRING];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCCH)text->data, text->length, (LPWSTR)wide_text, MAX_LENGTH_SHORT_STRING);

    IDWriteTextFormat * text_format;
    
    // TODO check result
    // TODO: shouldn't we release the text format? Do we want to (re)create it here every time?
    HRESULT text_format_result = direct_write_factory->CreateTextFormat(
        font_families[font.family],
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font.height,
        L"", //locale
        &text_format
    );
            
    IDWriteTextLayout * text_layout;
            
    // TODO check result
    // TODO: shouldn't we release the text layout? Do we want to (re)create it here every time?
    HRESULT text_layout_result = direct_write_factory->CreateTextLayout(
        (LPWSTR)wide_text,
        text->length,
        text_format,
        0, // We do not set a max width, and we do not use word wrapping
        0, // We do not set a max height, and we do not use word wrapping
        &text_layout
    );
    text_layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    
    DWRITE_TEXT_METRICS text_metrics;
    // TODO check result
    HRESULT metrics_result = text_layout->GetMetrics(&text_metrics);
    
    Size2d text_size = {};
    text_size.width = text_metrics.widthIncludingTrailingWhitespace;
    text_size.height = text_metrics.height;
    
    return text_size;
}

Size2dFloat get_text_size_float(ShortString * text, Font font)
{
    u16 wide_text[MAX_LENGTH_SHORT_STRING];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCCH)text->data, text->length, (LPWSTR)wide_text, MAX_LENGTH_SHORT_STRING);

    IDWriteTextFormat * text_format;
    
    // TODO check result
    // TODO: shouldn't we release the text format? Do we want to (re)create it here every time?
    HRESULT text_format_result = direct_write_factory->CreateTextFormat(
        font_families[font.family],
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font.height,
        L"", //locale
        &text_format
    );
            
    IDWriteTextLayout * text_layout;
            
    // TODO check result
    // TODO: shouldn't we release the text layout? Do we want to (re)create it here every time?
    HRESULT text_layout_result = direct_write_factory->CreateTextLayout(
        (LPWSTR)wide_text,
        text->length,
        text_format,
        0, // We do not set a max width, and we do not use word wrapping
        0, // We do not set a max height, and we do not use word wrapping
        &text_layout
    );
    text_layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    
    DWRITE_TEXT_METRICS text_metrics;
    // TODO check result
    HRESULT metrics_result = text_layout->GetMetrics(&text_metrics);
    
    Size2dFloat text_size = {};
    text_size.width = text_metrics.widthIncludingTrailingWhitespace;
    text_size.height = text_metrics.height;
    
    return text_size;
}

void draw_text(Pos2d position, ShortString * text, Font font, Color4 font_color)
{
    ID2D1SolidColorBrush * font_brush = 0;
    get_brush(font_color, &font_brush);

    u16 wide_text[MAX_LENGTH_SHORT_STRING];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCCH)text->data, text->length, (LPWSTR)wide_text, MAX_LENGTH_SHORT_STRING);

    IDWriteTextFormat * text_format;
    
    // TODO check result
    // TODO: shouldn't we release the text format? Do we want to (re)create it here every time?
    HRESULT text_format_result = direct_write_factory->CreateTextFormat(
        font_families[font.family],
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font.height,
        L"", //locale
        &text_format
    );
            
    IDWriteTextLayout * text_layout;
            
    // TODO check result
    // TODO: shouldn't we release the text layout? Do we want to (re)create it here every time?
    HRESULT text_layout_result = direct_write_factory->CreateTextLayout(
        (LPWSTR)wide_text,
        text->length,
        text_format,
        0, // We do not set a max width, and we do not use word wrapping
        0, // We do not set a max height, and we do not use word wrapping
        &text_layout
    );
    text_layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    
    render_target->DrawTextLayout(
        D2D1::Point2F(position.x, position.y),
        text_layout,
        font_brush
    );
    
    release_brush(font_brush);
}

void draw_text_c(Pos2d position, const char * cstring, Font font, Color4 font_color)
{
    ShortString text;
    copy_cstring_to_short_string(cstring, &text);
    draw_text(position, &text, font, font_color);
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

