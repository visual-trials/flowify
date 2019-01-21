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
 
#if DO_ASSERTIONS
#define assert(expression) if(!(expression)) {abort("Assertion failed!", __FILE__, __LINE__);}
#define assert_message(expression, message) if(!(expression)) {abort(message, __FILE__, __LINE__);}
#else
#define assert(expression)
#define assert_message(expression, message)
#endif

#include "../generic.h"
#include "../string.c"
#include "../geometry.c"
#include "../memory.c"
#include "../input.c"

#include "browser_render.c"

#include "../imgui/imgui.h"
#include "../imgui/menu.c"
#include "../imgui/debug.c"
#include "../imgui/scrollable_text.c"

#include INCLUDE_PROJECT_FILE
