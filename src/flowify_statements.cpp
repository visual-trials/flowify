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
    
    // Control flow elements
    FlowElement_Root,
    FlowElement_Function,
    FlowElement_If,
    FlowElement_For,
    
    // Data flow elements
    FlowElement_Assignment,
    FlowElement_BinaryOperator,
    FlowElement_FunctionCall,
    FlowElement_Scalar,
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

struct Flowifier
{
    FlowElement flow_elements[100]; // TODO: allocate this properly
    i32 nr_of_flow_elements;
};

FlowElement * new_flow_element(Flowifier * flowifier, Node * ast_node, FlowElementType flow_element_type = FlowElement_Unknown)
{
    FlowElement * new_flow_element = &flowifier->flow_elements[flowifier->nr_of_flow_elements++];
    new_flow_element->ast_node = ast_node;
    new_flow_element->first_child = 0;
    new_flow_element->next_sibling = 0;
    new_flow_element->type = flow_element_type;
    return new_flow_element;
}

void flowify_statements(Flowifier * flowifier, FlowElement * parent_element)
{
    
    Node * parent_node = parent_element->ast_node;
    
    Node * child_node = parent_node->first_child;
    FlowElement * previous_child_element = 0;
    
    if (child_node)
    {
        do
        {
            if (child_node->type == Node_Stmt_Expr)
            {
                
                // TODO: we should flowify the expression! (for now we create a dummy element)
                
                // FIXME: this is not always an FlowElement_Assignment!
                FlowElement * new_child_element = new_flow_element(flowifier, child_node, FlowElement_Assignment);
                
                if (!parent_element->first_child)
                {
                    parent_element->first_child = new_child_element;
                }
                else 
                {
                    previous_child_element->next_sibling = new_child_element;
                }
                previous_child_element = new_child_element;
            }
        }
        while((child_node = child_node->next_sibling));
    }
    
    log_int(flowifier->nr_of_flow_elements);
    
}