// https://wasdk.github.io/WasmFiddle/?ksvlc

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

extern void jsLog(char * text_data, int text_length);    
extern void jsLogInt(int log_integer);

void log232(char * text)
{
    jsLog(text, string_length(text));
}

void draw_rectangle(struct color4 line_color, struct color4 fill_color)
{
    jsLogInt(line_color.a);
}

void init_world()
{
}

void update_frame()
{
    log232((char*)"First");
    log232((char*)"Second");
    log232((char*)"Third");
    log232((char*)"Fourth");
}

void render_frame()
{
    struct color4 line_color;
    line_color.r = 255;
    line_color.g = 0;
    line_color.b = 0;
    line_color.a = 255;
    
    struct color4 fill_color;
    fill_color.r = 255;
    fill_color.g = 255;
    fill_color.b = 0;
    fill_color.a = 255;

    for (int entity_index = 0; entity_index < 1; entity_index++)
    {
        draw_rectangle(line_color, fill_color);
    }
}
