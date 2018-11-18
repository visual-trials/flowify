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

struct Pos2d
{
    i32 x;
    i32 y;
};

struct Size2d
{
    i32 width;
    i32 height;
};

struct Size2dFloat
{
    f32 width;
    f32 height;
};

#define MAX_LENGTH_SHORT_STRING 100

struct ShortString
{
    u8 data[MAX_LENGTH_SHORT_STRING];
    i32 length;
};

// TODO: put these function in a more common place

// TODO: can we make one common array_length here?
i32 array_length(i32 * array)
{
    return sizeof(array)/sizeof(array[0]);
}

i32 array_length(u8 * array)
{
    return sizeof(array)/sizeof(array[0]);
}

i32 cstring_length(u8 * string)
{
    i32 count = 0;
    while (*string++)
    {
        count++;
    }
    return count;
}

void append_string(ShortString * dest, ShortString * src)
{
    for (i32 i = 0; i < src->length; i++)
    {
        dest->data[dest->length + i] = src->data[i];
    }
    dest->length = dest->length + src->length;
}

void copy_string(ShortString * src, ShortString * dest)
{
    for (i32 i = 0; i < src->length; i++)
    {
        dest->data[i] = src->data[i];
    }
    dest->length = src->length;
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

ShortString * copy_cstring_to_short_string(const char * src, ShortString * dest)
{
    i32 length = copy_cstring(src, dest->data, MAX_LENGTH_SHORT_STRING);
    dest->length = length;
    
    return dest;
}

ShortString * copy_char_to_string(char ch, ShortString * dest)
{
    dest->data[0] = ch;
    dest->length = 1;
    
    return dest;
}

ShortString * int_to_string(i32 number, ShortString * decimal_string)
{
    if (number == 0)
    {
        decimal_string->data[0] = '0';
        decimal_string->length = 1;
        return decimal_string;
    }
    
    i32 negative_digit_offset = 0;
    if (number < 0)
    {
        negative_digit_offset = 1;  // the '-' takes one character, so everything moves one character to the right
        number = -number;
        decimal_string->data[0] = '-';
    }
    
    i32 left_over = number;
    i32 nr_of_digits = 0;
    while (left_over > 0)
    {
        i32 decimal_digit = left_over % 10;
        left_over = (left_over - decimal_digit) / 10;
        nr_of_digits++;
    }
    
    left_over = number;
    i32 digit_index = nr_of_digits - 1;
    while (left_over > 0)
    {
        i32 decimal_digit = left_over % 10;
        
        decimal_string->data[negative_digit_offset + digit_index] = '0' + decimal_digit;
        
        left_over = (left_over - decimal_digit) / 10;
        digit_index--;
    }
    decimal_string->length = negative_digit_offset + nr_of_digits;
    return decimal_string;
}

ShortString * float_to_string(f32 number, ShortString * decimal_string)
{
    if (number == 0)
    {
        decimal_string->data[0] = '0';
        decimal_string->length = 1;
        return decimal_string;
    }
    
    i32 negative_digit_offset = 0;
    if (number < 0)
    {
        negative_digit_offset = 1;  // the '-' takes one character, so everything moves one character to the right
        number = -number;
        decimal_string->data[0] = '-';
        decimal_string->length = 1;
    }
    
    i32 number_left_part = (i32)number;
    f32 number_float_right_part = number - (f32)number_left_part;
    i32 number_right_part = number_float_right_part * 10000000; // TODO: only the 7 digits are processed after the period
    
    ShortString string_left_part = {};
    int_to_string(number_left_part, &string_left_part);

    ShortString period = {};
    copy_char_to_string('.', &period);
    
    ShortString string_right_part = {};
    int_to_string(number_right_part, &string_right_part);
    
    append_string(decimal_string, &string_left_part);
    append_string(decimal_string, &period);
    append_string(decimal_string, &string_right_part);
    
    return decimal_string;
}
