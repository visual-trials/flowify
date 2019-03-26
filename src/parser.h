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

#include "tokenizer.c"

// TODO: Keep this in sync with node_type_names below!
enum NodeType
{
    Node_Unknown,
 
    Node_Root, 
    
    // Statements
    Node_Stmt_If,
    Node_Stmt_If_Cond,
    Node_Stmt_If_Then,
    Node_Stmt_If_Else,
    
    Node_Stmt_For,
    Node_Stmt_For_Init,
    Node_Stmt_For_Cond,
    Node_Stmt_For_Update,
    Node_Stmt_For_Body,
    
    Node_Stmt_Foreach,
    Node_Stmt_Foreach_Array,
    Node_Stmt_Foreach_Key_Var,
    Node_Stmt_Foreach_Value_Var,
    Node_Stmt_Foreach_Body,
    
    Node_Stmt_Function,
    Node_Stmt_Function_Params,
    Node_Stmt_Function_Body,
    
    Node_Stmt_Return,
    Node_Stmt_Break,
    Node_Stmt_Continue,
    
    Node_Stmt_Expr,
    
    // Expressions
    Node_Expr_PreInc,
    Node_Expr_PreDec,
    Node_Expr_PostInc,
    Node_Expr_PostDec,
    
    Node_Expr_AssignOp_Multiply,
    Node_Expr_AssignOp_Divide,
    Node_Expr_AssignOp_Plus,
    Node_Expr_AssignOp_Minus,
    Node_Expr_AssignOp_Concat,
    
    Node_Expr_BinaryOp_Multiply,
    Node_Expr_BinaryOp_Divide,
    Node_Expr_BinaryOp_Plus,
    Node_Expr_BinaryOp_Minus,
    Node_Expr_BinaryOp_Smaller,
    Node_Expr_BinaryOp_Greater,
    Node_Expr_BinaryOp_Equal,
    
    Node_Expr_Assign,
    
    Node_Expr_FuncCall,
    Node_Expr_FuncCall_Args,
    
    Node_Expr_Variable,
    
    Node_Expr_Array_Access, // TODO: find a better name for this: $myArray['key']
    
    // Scalars
    Node_Scalar_Number,
    Node_Scalar_Float,
    Node_Scalar_String
};

// TODO: Keep this in sync with the enum above!
// TODO: DON'T FORGET THE COMMAS!!
const char * node_type_names[] = {
    "Unknown",
    
    "Root",
    
    // Statements
    "Stmt_If",
    "Stmt_If_Cond",
    "Stmt_If_Then",
    "Stmt_If_Else",
    
    "Stmt_For",
    "Stmt_For_Init",
    "Stmt_For_Cond",
    "Stmt_For_Update",
    "Stmt_For_Body",
    
    "Stmt_Foreach",
    "Stmt_Foreach_Array",
    "Stmt_Foreach_Key_Var",
    "Stmt_Foreach_Value_Var",
    "Stmt_Foreach_Body",
    
    "Stmt_Function",
    "Stmt_Function_Params",
    "Stmt_Function_Body",
    
    "Stmt_Return",
    "Stmt_Break",
    "Stmt_Continue",
    
    "Stmt_Expr",
    
    // Expressions
    "Expr_PreInc",
    "Expr_PreDec",
    "Expr_PostInc",
    "Expr_PostDec",
    
    "Expr_AssignOp_Multiply",
    "Expr_AssignOp_Divide",
    "Expr_AssignOp_Plus",
    "Expr_AssignOp_Minus",
    "Expr_AssignOp_Concat",
    
    "Expr_BinaryOp_Multiply",
    "Expr_BinaryOp_Divide",
    "Expr_BinaryOp_Plus",
    "Expr_BinaryOp_Minus",
    "Expr_BinaryOp_Smaller",
    "Expr_BinaryOp_Greater",
    "Expr_BinaryOp_Equal",
    
    "Expr_Assign",
    
    "Expr_FuncCall",
    "Expr_FuncCall_Args",
    
    "Expr_Variable",
    
    "Expr_Array_Access",
    
    // Scalars
    "Scalar_Number",
    "Scalar_Float",
    "Scalar_String"
};

struct Node
{
    NodeType type;
    
    String identifier;
    String value;
    
    i32 first_token_index;
    i32 last_token_index;
    
    HighlightedLinePart highlighted_line_part;
    
    Node * first_child;
    Node * last_child;
    
    Node * next_sibling;
};

struct Parser
{
    Tokenizer * tokenizer;
    i32 current_token_index;

    DynamicArray nodes;
};

void init_parser(Parser * parser, Tokenizer * tokenizer)
{
    parser->current_token_index = 0;
    parser->tokenizer = tokenizer;

    init_dynamic_array(&parser->nodes, sizeof(Node), (Color4){255,0,255,255}, cstring_to_string("Parser"));
}

void next_token(Parser * parser)
{
    assert(parser->current_token_index < parser->tokenizer->tokens.nr_of_items);
    
    parser->current_token_index++;
}

i32 get_earlier_eaten_token_index(Parser * parser, i32 token_index_offset)
{
    assert(parser->current_token_index + token_index_offset >= 0);
    assert(parser->current_token_index + token_index_offset < parser->tokenizer->tokens.nr_of_items);
    
    return parser->current_token_index + token_index_offset;
}

i32 latest_eaten_token_index(Parser * parser)
{
    assert(parser->current_token_index - 1 >= 0);
    assert(parser->current_token_index - 1 < parser->tokenizer->tokens.nr_of_items);
    
    return parser->current_token_index - 1;
}

Token * latest_eaten_token(Parser * parser)
{
    Tokenizer * tokenizer = parser->tokenizer;
    
    Token * tokens = (Token *)tokenizer->tokens.items;
    Token * token = &tokens[latest_eaten_token_index(parser)];
    
    return token;
}

b32 accept_token(Parser * parser, i32 token_type)  // TODO: somehow we can't use TokenType as the type in win32. Using i32 instead.
{
    Tokenizer * tokenizer = parser->tokenizer;

    Token * tokens = (Token *)tokenizer->tokens.items;
    Token * token = &tokens[parser->current_token_index];
    
    if (token->type == token_type)
    {
        next_token(parser);
        return true;
    }
    return false;
}

b32 expect_token(Parser * parser, i32 token_type)  // TODO: somehow we can't use TokenType as the type in win32. Using i32 instead.
{
    if (accept_token(parser, token_type))
    {
        return true;
    }
    // FIXME: give a better error!
    log("ERROR: required token not found!");
    return false;
}

Node * new_node(Parser * parser)
{
    Node empty_node = {};
    Node * new_node = (Node *)add_to_array(&parser->nodes, &empty_node);
    
    new_node->first_child = 0;
    new_node->last_child = 0;
    new_node->next_sibling = 0;
    new_node->type = Node_Unknown;
    return new_node;
}

#define StartOnLatestToken -1
#define StartOnTokenBeforeLatestToken -2

Node * start_node(Parser * parser, NodeType node_type, i32 token_index_offset)
{
    Node * node = new_node(parser);
    node->first_token_index = get_earlier_eaten_token_index(parser, token_index_offset);
    node->type = node_type;
    return node;
}

Node * start_node(Parser * parser, NodeType node_type)
{
    Node * node = new_node(parser);
    node->first_token_index = parser->current_token_index;
    node->type = node_type;
    return node;
}

void end_node(Parser * parser, Node * node)
{
    node->last_token_index = latest_eaten_token_index(parser);
}

void end_node(Parser * parser, Node * node, i32 token_index_offset)
{
    node->last_token_index = get_earlier_eaten_token_index(parser, token_index_offset);
}

void add_child_node(Node * child_node, Node * parent_node)
{
    if (!parent_node->first_child)
    {
        parent_node->first_child = child_node;
    }
    else
    {
        parent_node->last_child->next_sibling = child_node;
    }
    parent_node->last_child = child_node;
}

String get_source_text_from_ast_node(Parser * parser, Node * node)
{
    String source_text = {};
    
    if (node)
    {
        Tokenizer * tokenizer = parser->tokenizer;
        
        Token * tokens = (Token *)tokenizer->tokens.items;
        Token * start_token = &tokens[node->first_token_index];
        Token * end_token = &tokens[node->last_token_index];
        
        source_text.data = start_token->text.data;
        source_text.length = end_token->text.data + end_token->text.length - start_token->text.data;
    }
    
    return source_text;
}

i32 dump_parse_tree(Node * node, DynamicString * dump_text, i32 dump_line_index = 0, i32 depth = 0)
{
    String node_type_string = cstring_to_string(node_type_names[node->type]);
    for (i32 indentation_index = 0; indentation_index < depth; indentation_index++)
    {
        append_string(dump_text, cstring_to_string("    "));
    }
    append_string(dump_text, node_type_string);
    append_string(dump_text, cstring_to_string("\n"));
    
    node->highlighted_line_part.line_index = dump_line_index++;
    node->highlighted_line_part.start_character_index = (u16)(depth * 4);
    node->highlighted_line_part.length = (u16)node_type_string.length;
    
    if (node->first_child)
    {
        dump_line_index = dump_parse_tree(node->first_child, dump_text, dump_line_index, depth + 1);
    }

    if (node->next_sibling)
    {
        dump_line_index = dump_parse_tree(node->next_sibling, dump_text, dump_line_index, depth);
    }
    
    return dump_line_index;
}
