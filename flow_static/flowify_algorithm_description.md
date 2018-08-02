
# Algorithm

## There are two types of flowElements:

1. Childless elements
    * Variable declaration and assignment
    * Primitive function
    * Operator
    * Variable passthrough
    * Variable splitter
    * Variable joiner
2. Elements with childs
    * Root
        * RootBody/Lane (statements)
        * RootEndBody/Lane *(contains joiners: 'return')*
    * Function
        * FunctionCallBody/Lane (statements)
        * FunctionCallEndBody/Lane *(contains joiners: 'return')*
        * Can return a variable (or have multiple returns)
        * Can have references to variables in and out
        * Optionally: input, body, output children
    * If
        * Condition (expression) *(contains splitters)*
        * ThenBody/Lane (statements)
        * ElseBody/Lane (statements)
        * EndBody/Lane *(contains joiners)*
    * For
        * Init *(contains joiners)*
        * ForStep
            * Condition (expressions) *(contains splitters)*
            * IterationBody/Lane (statements)
            * Update *(can contain joiners: 'continue')*
        * Done *(can contain joiners: 'break')*
        * Passback
    * Try
        * TryBody/Lane
        * CatchBody/Lane

Bodies/Lanes end with terminator-statements:
* continue [n]
* break [n]
* return [variable]
* throw \<exception\>
* goto \<label\>
        
## Data structures inside flowElements

### Front-end

* Element-data
    * id as elementId
    * astIndentifier as identifier for visual info
    * type as indicator for visual type
    * children for elements inside containers 
* Connection-data
    * visually connecting elements
    * type as indicator of visual type

### Forwards

* Structure of the AST
* endsWith
* OpenEndings

### Backwards

#### Control flow data
* previousElement (statement(s)?)
* parentElement (statements)
* exitingParentElement (statements?)
    * Note: openEnds have a different exitingParent than their normal parent
* firstChild (statement?)
* lastChild (statement?)

#### Variable finding data
* varsInScopeAvailable (*array[] boolean*)
    * a variable was declared before this moment and is available in this Body/Lane
* varsInScopeChanged  (*array[] boolean*)
    * a variable was changed or declared inside this Lane or inside its children
* lastChangedVariable (*array[] flowElement*)
    * contains the last changed variable inside a direct parent (*cache*)

## Running the AST forwards

* Whenever there is a **variable-use** we first determine whether this is an implicit declaration
    * We do this by checking the varsInScopeAvailable for the Body/Lane
* If the **variable was not known yet**, it should be declared at the beginning of the function
    * We insert the variable inside the containing function and then build a path to it (which is always a direct connection). 
    * The varsInScopeAvailable and varsInScopeChanged are set for all elements inside the function/program.
        * Note: for block-scope languages this is restricted to the block.
* If the **variable is known** (and used) we build a path towards it. The **type of path** depends on whether it is a **use** (*use*) or **re-assignment** (*identity*).
* If the variable is **re-assigned**, we also create a connection towards the new variable (*assign*) and set its property 'isVariable' to true. We also set **varsInScopeChanged** for its parent.
* When **flowifying a Body/Lane** (usually a series of statements or expressions)
    * at the start we make a copy of the varsInScopeAvailable of the parent
    * after flowifying each statement, we do the following:
        * if **onlyHasOpenEndings is not true** for the child
            * we set the *exitingParent* of the child to the current element
             (but not for the openEnds inside the child)
            * we add the varsInScopeChanged of the child to the varsInScopeChanged of the *exitingParent*, unless the variable is not available inside this *exitingParent*.
    * when we **process/catch openEnds** (those inside the child)
        * we set the *exitingParent* for those elements (which could be the child itself) to the 'receiving' parent
        * we add the varsInScopeChanged of the child to the varsInScopeChanged of the *exitingParent*, unless the variable is not available inside this *exitingParent*.
* Depending on the type of Element (If/For/Do/While/Catch/Function) we set **previousElement/Lane** and **parentElement/Lane**. We also set the **firstChild** and **lastChild** when flowifying a list of statements.
    * When dealing with **nested expressions**, we should also make sure that preivousELement is set the correct way.
* A Body or Lane (containing a series of statements) can be terminated by any of the 5 **lane-terminators**.
    * We set the endsWith of the Lane
    * We add the Lane to the OpenEnds of itself (?)
    * We keep a record of onlyHasOpenEndings and hasOpenEndings
    * An openEnding effectively "stretches" the Lane until the point where it is "catched" by an Element "interested" in those types of terminators.
* A **function-call** that has **openEndings of type '*return*'** will join these openEndings into a return variable.
* An **if** will do the following with OpenEndings
    * if **both** its **then- and else-body** have their **onlyHasOpenEndings** set to true, it will not join its lanes and will set its own onlyHasOpenEndings to true.
    * if **only** its **then-body** has its **onlyHasOpenEndings** set to true, it will not join its lanes, but will *not* set onlyHasOpenEndings to true.
    * if **only** its **else-body** has its **onlyHasOpenEndings** set to true, it will not join its lanes, but will *not* set onlyHasOpenEndings to true.
    * if either its then- or else-body has **some openEndings**, they are added to the openEndings of the if. It will however
        * Note: whether or not joining of its lanes takes place, affects **how previousElements** are set and whether varsInScope is copied back.
* A **for** will do the following with OpenEndings    
    * if the **iterBody/Lane** has its **onlyHasOpenEndings** set to true for **only 'return'- and 'throw'-terminators**, it will not join the iterBody/updateBody with the condition-element (via the back-element) and its done will not be connected. No Lanes will be joined until later (after the for).
    * if the **iterBody/Lane** has its **onlyHasOpenEndings** set to true for **only 'break'-terminators**, it will join with the done-body (and skip the updateBody/BackBody etc)
    * if the **iterBody/Lane** has its **onlyHasOpenEndings** set to true for **only 'continue'-terminators**, it will join all openEnds inside the updateBody
    * if there are some openEnds inside **iterBody/Lane** of type 'continue', we join them inside the updateBody
    * if there are some openEnds inside **iterBody/Lane** of type 'break', we join them with the done-body
    * if there are some openEnds inside **iterBody/Lane** of type 'return' or 'throw', we do not join them (until after the for)
    
    
    
## Building a path backwards

* At **variable-use** we take the **parentElement** (of the toElement) and call it the **laneElement** towards the buildingPath-function
    * Note: optionally we start at the previousElement (of the toElement)
* We check if for this variable the **varsInScopeChanged** is true. If it is, the var we want to connect to is somewhere in this Lane
    * We **loop though all the Lane's children backwards** (or start the last and use preivousELement each time). We do the following for each child
        * If the variableName is in **lastChangedVariable**, we connect to that var (and are done)
        * If in the child, for this variable the varsInScopeChanged is true, then we recurse into this child as Lane
* If the variable was not changed inside the Lane, we go to the previousElement
    * If there a multiple previousElements, we recurse into all/both
        * Note: there can only be multiple previousElements, if we just 'entered' a Lane that **joins Lanes**, these are
            * Function: joining 'returns'
            * Catch: joining 'throws'
            * Done/For: joining 'breaks'
            * UpdateBody: joining 'continues'
            * If: joining 'normals' (then/else)
            * For/Condition: joining 'normals' (back/init?) 
    * For elements that contain splitters (If/For) and for elements that can contain joiners (Catch/Function/Update/Done), we should detrmine whether the varsInScopeChanged is true.
        * Basicly, we need to know whether a var was changed between the **split** and **join** (by *either* side)
        * Only if the variable was **changed by either side** OR if the **join was assymetric**, should you build a path through all previousElements of **both sides**.
            * We do this by first adding a joiner and then building a path from that joining into both sides
            * When we reach a point where a splitter should be added (If/For) we add a one
            * If we start entering a parent again and varsInScopeChanged was not filled, we add a passthrough
            * Note: whenever we add a splitter, joiner or passthrough we:
                * add it to the lastChangedVariable of the (exiting?)parent
                * set varsInScopeChanged to all (exiting?)parents until the point where varsInScopeAvailable is not true for the variable anymore