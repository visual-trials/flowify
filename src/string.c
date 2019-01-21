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
 
#define MAX_LENGTH_SHORT_STRING 100

struct ShortString
{
    u8 data[MAX_LENGTH_SHORT_STRING];
    i32 length;
};

struct String
{
    u8 * data;
    i32 length;
};

// TODO: can we make one common array_length here?
inline i32 array_length(i32 * array)
{
    return sizeof(array)/sizeof(array[0]);
}

inline i32 array_length(u8 * array)
{
    return sizeof(array)/sizeof(array[0]);
}

i32 cstring_length(const char * cstring)
{
    i32 count = 0;
    while (*cstring++)
    {
        count++;
    }
    return count;
}

void append_string(ShortString * dest, ShortString * src)
{
    i32 nr_of_characters_to_append = src->length;
    if (dest->length + nr_of_characters_to_append > MAX_LENGTH_SHORT_STRING)
    {
        nr_of_characters_to_append = MAX_LENGTH_SHORT_STRING - dest->length;
    }
    
    for (i32 i = 0; i < nr_of_characters_to_append; i++)
    {
        dest->data[dest->length + i] = src->data[i];
    }
    dest->length = dest->length + nr_of_characters_to_append;
}

// TODO: we don't need a pointer to the source here. Better not to take the pointer, we don't want to change the source anyway!
void append_string(String * dest, String * src)
{
    for (i32 i = 0; i < src->length; i++)
    {
        dest->data[dest->length + i] = src->data[i];
    }
    dest->length = dest->length + src->length;
}

b32 equals(String string1, String string2)
{
    if (string1.length != string2.length)
    {
        return false;
    }
    
    b32 equals = true;
    for (i32 i = 0; i < string1.length; i++)
    {
        if (string1.data[i] != string2.data[i])
        {
            return false;
        }
    }
    return true;
}

b32 equals(String string1, const char * cstring2)
{
    i32 cstring2_length = cstring_length(cstring2);
    
    if (string1.length != cstring2_length)
    {
        return false;
    }
    
    b32 equals = true;
    for (i32 i = 0; i < string1.length; i++)
    {
        if (string1.data[i] != cstring2[i])
        {
            return false;
        }
    }
    return true;
}

void copy_string(ShortString * src, ShortString * dest)
{
    for (i32 i = 0; i < src->length; i++)
    {
        dest->data[i] = src->data[i];
    }
    dest->length = src->length;
}

void copy_string(String src, ShortString * dest)
{
    for (i32 i = 0; i < src.length; i++)
    {
        dest->data[i] = src.data[i];
    }
    dest->length = src.length;
}

String shortstring_to_string(ShortString * shortstring)
{
    String string = {};
    string.data = shortstring->data;
    string.length = shortstring->length;
    return string;
}

i32 copy_cstring(const char * src, u8 * dest, i32 max_length = -1)
{
    i32 count = 0;
    while (char ch = *src++)
    {
        if (max_length >= 0 && count >= max_length)
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

String cstring_to_string(const char * src)
{
    String string = {};
    string.length = cstring_length(src);
    string.data = (u8*)src;
    
    return string;
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
