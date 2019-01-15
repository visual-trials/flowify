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

void process_interactions(Flowifier * flowifier, FlowElement * flow_element)
{
    
    // TODO: implement: check for mouse/touch interaction and store in Flowifier-struct
    
    if (flow_element->first_child)
    {
        process_interactions(flowifier, flow_element->first_child);
    }

    if (flow_element->next_sibling)
    {
        process_interactions(flowifier, flow_element->next_sibling);
    }
}