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
 
#include "generic.h"
#include "input.cpp"
#include "render.cpp"

struct WorldData
{
};

WorldData global_world = {};  // FIXME: allocate this properly!

extern "C" {
    
    void init_world()
    {
        WorldData * world = &global_world;
    }
    
    void update_frame()
    {
        
        
    }
    
    void render_frame()
    {
        Input * input = &global_input;
        ShortString address;
        ShortString size;
        int_to_string(input->memory.address, &address);
        int_to_string(input->memory.size, &size);
        
        Font font;
        font.family = Font_Arial;
        font.height = 20;
        
        Color4 black = {0, 0, 0, 255};
        
        draw_text((Pos2d){100,100}, &address, font, black);
        draw_text((Pos2d){100,140}, &size, font, black);
    }
}