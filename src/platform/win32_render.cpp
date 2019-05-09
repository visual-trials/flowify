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

// Note: keep this in sync with the enum FontFamily in generic.h
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

void get_round_cap_stroke_style(ID2D1StrokeStyle ** stroke_style)
{
    d2d_factory->CreateStrokeStyle(
        D2D1::StrokeStyleProperties(
            D2D1_CAP_STYLE_ROUND,  // The start cap.
            D2D1_CAP_STYLE_ROUND,  // The end cap.
            D2D1_CAP_STYLE_ROUND, // The dash cap.
            D2D1_LINE_JOIN_ROUND, // The line join.
            10.0f, // The miter limit.
            D2D1_DASH_STYLE_SOLID, // The dash style.
            0.0f // The dash offset.
        ), 
        NULL, 
        0, 
        stroke_style
    );
}

// TODO: don't we want to pass two positions to most of these functions? Instead of pos + size?
// TODO: don't we want f32 for colors?

void clip_rectangle(Pos2d position, Size2d size)
{
    render_target->PushAxisAlignedClip(
        D2D1::RectF(position.x, position.y, position.x + size.width, position.y + size.height),
        D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
    );
}
 
void unclip_rectangle()
{
    render_target->PopAxisAlignedClip();
}

void draw_rectangle(Pos2d position, Size2d size, DrawStyle draw_style)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    D2D1_RECT_F rectangle = D2D1::RectF(position.x, position.y, position.x + size.width, position.y + size.height);
    
    if (draw_style.fill_color.a)
    {
        get_brush(draw_style.fill_color, &fill_brush);
        render_target->FillRectangle(&rectangle, fill_brush);
        release_brush(fill_brush);
    }
    
    if (draw_style.line_color.a)
    {
        get_brush(draw_style.line_color, &line_brush);
        render_target->DrawRectangle(&rectangle, line_brush, draw_style.line_width);
        release_brush(line_brush);
    }

}

void draw_rectangle(Rect2d rect, DrawStyle draw_style)
{
    draw_rectangle(rect.position, rect.size, draw_style);
}


b32 draw_path(DrawablePathPart * path_parts, i32 path_parts_count, b32 is_background, ID2D1GeometrySink * sink, b32 figure_is_started = false)
{
    i32 previous_x = 0;
    i32 previous_y = 0;
    for (i32 part_index = 0; part_index < path_parts_count; part_index++)
    {
        DrawablePathPart path_part = path_parts[part_index];
        PathPartType type = path_part.part_type;
        i32 x = path_part.position.x;
        i32 y = path_part.position.y;
        
        D2D1_POINT_2F position = D2D1::Point2F(x, y);
        D2D1_ARC_SEGMENT arc_segment;
    
        i32 radius = previous_x - x;
        if (radius < 0)
        {
            radius = -radius;
        }
        
        D2D1_SIZE_F size_arc = D2D1::SizeF(radius, radius);

            
        if (type == PathPart_Move)
        {
            if (figure_is_started)
            {
                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                figure_is_started = false;
            }
            sink->BeginFigure(position, D2D1_FIGURE_BEGIN_FILLED);
            figure_is_started = true;
        }
        else if (type == PathPart_Line)
        {
            assert(figure_is_started);
            sink->AddLine(position);
        }
        else if (type == PathPart_LineWhenBackground)
        {
            if (is_background)
            {
                // FIXME: we currently start with a LineWhenBackhround. But we *SHOULD* start with a move instead!
                // assert(figure_is_started);
                if (!figure_is_started)
                {
                    sink->BeginFigure(position, D2D1_FIGURE_BEGIN_FILLED);
                    figure_is_started = true;
                }
                sink->AddLine(position);
            }
            else {
                if (figure_is_started)
                {
                    sink->EndFigure(D2D1_FIGURE_END_OPEN);
                    figure_is_started = false;
                }
                sink->BeginFigure(position, D2D1_FIGURE_BEGIN_FILLED);
                figure_is_started = true;
            }
        }
        else if (type == PathPart_Arc_DownToLeft)
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_CLOCKWISE,  // The 90 doesn't seem to matter. Probably because we have no straight part in the Arc
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
        else if (type == PathPart_Arc_DownToRight)
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
        else if( type == PathPart_Arc_UpToLeft)
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
        else if (type == PathPart_Arc_UpToRight)
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
        else if (type == PathPart_Arc_LeftToUp) 
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
        else if (type == PathPart_Arc_LeftToDown) 
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
        else if (type == PathPart_Arc_RightToUp) 
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
        else if (type == PathPart_Arc_RightToDown)
        {
            arc_segment = D2D1::ArcSegment(position, size_arc,
                                           90, D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                           D2D1_ARC_SIZE_SMALL ); // means: smaller than 180 degrees
            sink->AddArc(arc_segment);
        }
                    
        previous_x = x;
        previous_y = y;
    }
    
    return figure_is_started;
}


void draw_lane_paths(DrawablePathPart * left_path_parts_index, i32 left_path_parts_count, 
                     DrawablePathPart * right_path_parts_index, i32 right_path_parts_count, 
                     DrawStyle draw_style)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    ID2D1PathGeometry * path_geometry = 0;
    ID2D1GeometrySink * sink = 0;
    
    // Drawing the lane as left and right paths
    if (draw_style.fill_color.a)
    {
        d2d_factory->CreatePathGeometry(&path_geometry);
        path_geometry->Open(&sink);
        sink->SetFillMode(D2D1_FILL_MODE_WINDING);

        b32 is_background = true;
        b32 figure_is_started = draw_path(left_path_parts_index, left_path_parts_count, is_background, sink);
        figure_is_started = draw_path(right_path_parts_index, right_path_parts_count, is_background, sink, figure_is_started);
        if (figure_is_started)
        {
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        }
        
        sink->Close();
        sink->Release();
        
        get_brush(draw_style.fill_color, &fill_brush);
        render_target->FillGeometry(path_geometry, fill_brush);
        release_brush(fill_brush);
        
        path_geometry->Release();
    }
    
    if (draw_style.line_color.a)
    {
        d2d_factory->CreatePathGeometry(&path_geometry);
        path_geometry->Open(&sink);
        sink->SetFillMode(D2D1_FILL_MODE_WINDING);

        b32 is_background = false;
        b32 figure_is_started = draw_path(left_path_parts_index, left_path_parts_count, is_background, sink);
        if (figure_is_started)
        {
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
        }
        
        figure_is_started = draw_path(right_path_parts_index, right_path_parts_count, is_background, sink);
        if (figure_is_started)
        {
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
        }
        
        sink->Close();
        sink->Release();
        
        get_brush(draw_style.line_color, &line_brush);
        render_target->DrawGeometry(path_geometry, line_brush, draw_style.line_width);    
        release_brush(line_brush);
        
        path_geometry->Release();
    }

}
               
void draw_line(Pos2d start_position, Pos2d end_position, Color4 line_color, i32 line_width, b32 round_cap = false)
{
    D2D1_POINT_2F start_pos, end_pos;
    ID2D1SolidColorBrush * line_brush = 0;
    
	start_pos.x = start_position.x;
	start_pos.y = start_position.y;
    
	end_pos.x = end_position.x;
	end_pos.y = end_position.y;
    
    get_brush(line_color, &line_brush);
    
    if (round_cap)
    {
        ID2D1StrokeStyle * stroke_style;
        get_round_cap_stroke_style(&stroke_style);
        render_target->DrawLine(start_pos, end_pos, line_brush, line_width, stroke_style);
    }
    else
    {
        render_target->DrawLine(start_pos, end_pos, line_brush, line_width);
    }
    
    release_brush(line_brush);
}

void draw_rounded_rectangle(Pos2d position, Size2d size, DrawStyle draw_style)
{
    ID2D1SolidColorBrush * line_brush = 0;
    ID2D1SolidColorBrush * fill_brush = 0;
    
    D2D1_RECT_F rectangle = D2D1::RectF(position.x, position.y, position.x + size.width, position.y + size.height);
    D2D1_ROUNDED_RECT rounded_rectangle = D2D1::RoundedRect(rectangle, draw_style.corner_radius, draw_style.corner_radius);
    
    if (draw_style.fill_color.a)
    {
        get_brush(draw_style.fill_color, &fill_brush);
        render_target->FillRoundedRectangle(&rounded_rectangle, fill_brush);
        release_brush(fill_brush);
    }
    
    if (draw_style.line_color.a)
    {
        get_brush(draw_style.line_color, &line_brush);
        render_target->DrawRoundedRectangle(&rounded_rectangle, line_brush, draw_style.line_width);
        release_brush(line_brush);
    }
    
}

void draw_rounded_rectangle(Rect2d rect, DrawStyle draw_style)
{
    draw_rounded_rectangle(rect.position, rect.size, draw_style);
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

Size2d get_text_size(String text, Font font)
{
    u16 wide_text[MAX_LENGTH_SHORT_STRING];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCCH)text.data, text.length, (LPWSTR)wide_text, MAX_LENGTH_SHORT_STRING);

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
        text.length,
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

Size2d get_text_size(ShortString * text, Font font)
{
    String text_string = {};
    text_string.data = text->data;
    text_string.length = text->length;
    
    return get_text_size(text_string, font);
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

void draw_text(Pos2d position, String text, Font font, Color4 font_color)
{
    ID2D1SolidColorBrush * font_brush = 0;
    get_brush(font_color, &font_brush);

    u16 wide_text[MAX_LENGTH_SHORT_STRING];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCCH)text.data, text.length, (LPWSTR)wide_text, MAX_LENGTH_SHORT_STRING);

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
        text.length,
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

void draw_text(Pos2d position, ShortString * text, Font font, Color4 font_color)
{
    String text_string;
    text_string.data = text->data;
    text_string.length = text->length;
    
    draw_text(position, text_string, font, font_color);
}

void draw_text_c(Pos2d position, const char * cstring, Font font, Color4 font_color)
{
    ShortString text;
    copy_cstring_to_short_string(cstring, &text);
    draw_text(position, &text, font, font_color);
}

void log(ShortString * text)
{
    char cstring[200];  // TODO: this might not be big enough!
    i32 max_length = text->length;
    if (max_length > sizeof(cstring) - 1)
    {
        max_length = sizeof(cstring) - 1;
    }
    copy_cstring((const char *)text->data, (u8 *)cstring, max_length);
    cstring[max_length] = '\0';
    OutputDebugStringA((LPCSTR)cstring);
}

void log(String text)
{
    char cstring[200];  // TODO: this might not be big enough!
    i32 max_length = text.length;
    if (max_length > sizeof(cstring) - 1)
    {
        max_length = sizeof(cstring) - 1;
    }
    copy_cstring((const char *)text.data, (u8 *)cstring, max_length);
    cstring[max_length] = '\0';
    OutputDebugStringA((LPCSTR)cstring);
}

void log(const char * text)
{
    OutputDebugStringA((LPCSTR)text);
}

void log_int(i32 integer)
{
    ShortString decimal_number;
    int_to_string(integer, &decimal_number);
    log(&decimal_number);
}

void set_using_physical_pixels(b32 using_physical_pixels)
{
    // TODO: not implemented yet
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
