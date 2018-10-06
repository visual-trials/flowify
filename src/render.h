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
 
#define internal static 
#define local_persist static 
#define global_variable static

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

typedef float  r32;
typedef double r64;

struct color4
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

#define MAX_LENGTH_SHORT_STRING 255

struct short_string
{
    u8 data[MAX_LENGTH_SHORT_STRING];
    i32 length;
};

// TODO: put this function in a more common place
i32 cstring_length(u8 * string)
{
    i32 count = 0;
    while (*string++)
    {
        count++;
    }
    return count;
}

i32 copy_cstring(const char * src, u8 * dest, i32 max_length)
{
    i32 count = 0;
    while (char ch = *src++)
    {
        if (count >= max_length)
        {
            break;
        }
        *dest++ = ch;
        count++;
    }
    *dest = 0;
    return count;
}

void copy_cstring_to_short_string(const char * src, short_string * dest)
{
    i32 length = copy_cstring(src, dest->data, MAX_LENGTH_SHORT_STRING);
    dest->length = length;
}

