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


// TODO: we should check whether these mouse or touch interactions are within the Window of the flowified elements
void process_interactions(Flowifier * flowifier, Input * input, FlowElement * flow_element)
{
    HoveredOrPressed element_hovered_or_pressed = check_hovered_or_pressed(flow_element->rect_abs.position, flow_element->rect_abs.size, input);
    
    if (flow_element->type == FlowElement_Root)
    {
        // Resetting hovered_element_index each time we start to process interactions
        flowifier->interaction.hovered_element_index = 0;

        // Unsetting the selected element if the screen is pressed, but not the root element
        // TODO: instead of the screen rectangle, we should probably use the Window-rectangle here
        Size2d screen_size = {input->screen.width, input->screen.height};
        Pos2d screen_position = {};
        HoveredOrPressed screen_hovered_or_pressed = check_hovered_or_pressed(screen_position, screen_size, input);
        
        if (screen_hovered_or_pressed.is_pressed && !element_hovered_or_pressed.is_pressed)
        {
            flowifier->interaction.selected_element_index = 0;
        }
    }
    
    if (element_hovered_or_pressed.is_hovered || element_hovered_or_pressed.is_pressed)
    {
        // If the element is hovered, we remember it as the hovered element
        if (element_hovered_or_pressed.is_hovered)
        {
            flowifier->interaction.hovered_element_index = flow_element->index;
        }
        // If the element is hovered, we remember it as the selected element
        if (element_hovered_or_pressed.is_pressed)
        {
            flowifier->interaction.selected_element_index = flow_element->index;
        }
        
        if (flow_element->first_child)
        {
            // If the element is hovered or pressed and it has children, we check them too.
            // The child may become the more specific element being hovered or selected
            process_interactions(flowifier, input, flow_element->first_child);
        }
    }
    else
    {
        // If the element is not hovered, we check its siblings
        if (flow_element->next_sibling)
        {
            process_interactions(flowifier, input, flow_element->next_sibling);
        }
    }
}