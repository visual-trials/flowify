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

enum FlowElementType
{
    FlowElement_Unknown,
    
    FlowElement_StatementExpression, // TODO: what to call these?
    
};
 
struct FlowElement
{
    FlowElementType type;
    
    Node * ast_node;
    
    FlowElement * first_child;
    FlowElement * next_sibling;
};

// TODO: maybe we should create a struct for each type of FlowElement?
//    Function/Root (RoundedRectangle/Container)
//    ExpressionStatement ((Sub)LaneSegment)
//    IfStatement (Head/Split/2-Lanes/Join/Tail-Segments)
//    ForStatement (...)
struct FlowFunction // TODO: maybe call this FlowBody? or FlowBlock?
{
    FlowElement basic;
    
    
    
};

struct Flowifier
{
    FlowElement flow_elements[100]; // TODO: allocate this properly
    i32 nr_of_flow_elements;
};

FlowElement * new_flow_element(Flowifier * flowifier)
{
    FlowElement * new_flow_element = &flowifier->flow_elements[flowifier->nr_of_flow_elements++];
    new_flow_element->ast_node = 0;
    new_flow_element->first_child = 0;
    new_flow_element->next_sibling = 0;
    new_flow_element->type = FlowElement_Unknown;
    return new_flow_element;
}

void flowify_statements(Flowifier * flowifier, Node * parent_node, FlowElement * parent_element)
{
    
    Node * child = parent_node->first_child;
    FlowElement * child_element = 0;
    
    if (child)
    {
        
        child_element = new_flow_element(flowifier);
        parent_element->first_child = child_element;
        
        while((child = child->next_sibling))
        {
            
            FlowElement * new_child_element = new_flow_element(flowifier);
            child_element->next_sibling = new_child_element;
            child_element = new_child_element;
        }
    }
    
    log_int(flowifier->nr_of_flow_elements);
    
}