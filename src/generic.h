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
 
#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32     b32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

struct Color4
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

// NOTE: always keep this in sync with my.fontFamilies in canvas.js and font_families in win32/render.cpp!
enum FontFamily
{
    Font_Arial = 0x00,
    Font_CourierNew = 0x01
};

struct Font
{
    FontFamily family;
    i32 height;  // font.height is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
};

