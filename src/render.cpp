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
    #include "win32/render.cpp"
#else
    #include "browser/render.cpp"
#endif


void draw_cross(i32 x, i32 y, i32 distance_from_center, i32 line_length, Color4 line_color, i32 line_width)
{
    draw_line(x - distance_from_center, y, 
              x - distance_from_center - line_length, y, 
              line_color, line_width);
    
    draw_line(x + distance_from_center, y, 
              x + distance_from_center + line_length, y, 
              line_color, line_width);
    
    draw_line(x, y - distance_from_center, 
              x, y - distance_from_center - line_length, 
              line_color, line_width);
              
    draw_line(x, y + distance_from_center, 
              x, y + distance_from_center + line_length, 
              line_color, line_width);
}

