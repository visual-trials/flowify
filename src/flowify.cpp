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

const char * simple_assign_program_text = 
    "<?php\n"
    "\n"
    "$answer = 42;\n"
;

const char * simple_if_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "\n"
    "if ($b > 50) {\n"
    "	$b = 50;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

extern "C" {
    
    void init_world()
    {
    }
    
    void update_frame()
    {
    }
    
    void render_frame()
    {
    }
}