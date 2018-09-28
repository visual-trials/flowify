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
 
#if BUILD_FOR_NATIVE_PLATFORM
    #include "render_win32.cpp"
#else
    #include "render_js.cpp"
#endif

extern "C" {
    void draw_frame(int increment)
    {
        clear_rectangle(0, 0, 640, 480);
        
        color4 fill_color = {};
        fill_color.b = 255;
        fill_color.a = 128;
        
        color4 line_color = {};
        line_color.g = 255;
        line_color.a = 255;
        
        int line_width = 3;
        
        draw_rectangle(200, 50, 40, 40, (color4){255,0,0,255}, (color4){255,255,0,255}, line_width);
        
        int offset;
        if (increment % 512 < 256) {
            offset = increment % 256;
        }
        else {
            offset = 256 - (increment % 256);
        }
        draw_rectangle(offset + 10, 10, 100, 100, line_color, fill_color, line_width);
        draw_rectangle(10, offset + 10, 100, 100, line_color, fill_color, line_width);
    }
}