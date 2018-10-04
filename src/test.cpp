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
 
struct color4
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

int string_length(char * string)
{
    int count = 0;
    while (*string++)
    {
        count++;
    }
    return count;
}

extern "C" { 
    extern void jsLog(char * text_data, int text_length);    
    extern void jsLogInt(int log_integer);
}

void log(char * text)
{
    jsLog(text, string_length(text));
}

void draw_rectangle(color4 line_color, color4 fill_color)
{
    jsLogInt(line_color.a);
}

extern "C" {
    
    void init_world()
    {
    }
    
    void update_frame()
    {
        log((char*)"First");
        log((char*)"Second");
        log((char*)"Third");
        log((char*)"Fourth");
    }
    
    void render_frame()
    {
        color4 line_color;
        line_color.r = 255;
        line_color.g = 0;
        line_color.b = 0;
        line_color.a = 255;
        
        color4 fill_color;
        fill_color.r = 255;
        fill_color.g = 255;
        fill_color.b = 0;
        fill_color.a = 255;

        for (int entity_index = 0; entity_index < 1; entity_index++)
        {
            draw_rectangle(line_color, fill_color);
        }
    }
}