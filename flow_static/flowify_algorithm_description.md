
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
    * Function
        * FunctionCallBody/Lane (statements)
        * Can return a variable (or have multiple returns)
        * Can have references to variables in and out
        * Optionally: input, body, output children
    * If
        * Condition
        * ThenBody/Lane (statements)
        * ElseBody/Lane (statements)
    * For
        * Init
        * ForStep
            * Condition (expressions)
            * IterationBody/Lane (statements)
            * Update
        * Done
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
* previousElement/Lane (statement(s)?)
* parentElement/Lane (statements)
* firstChild (statement?)
* lastChild (statement?)

#### Containing childless elements
* splitterVars (*array[] FlowElement*)
* joinerVars (*array[] FlowElement*)
* passthroughVars (*array[] FlowElement*)
* assignedVars (*array[] FlowElement*)

#### Helper data
* varsInScopeStart (*array[] boolean*)
* varsInScope (or varsInScopeChanged)  (*array[] boolean*)

## Running the AST forwards

* Whenever there is a **variable-use** we first determine whether this is an implicit declaration
    * We do this by checking the varsInScope available for the Body/Lane
* If the **variable was not known yet**, it should be declared at the beginning of the function
    * We insert the variable inside the containing function and then build a path to it (which is always a direct connection)
* If the **variable is known** (and used) we build a path towards it. The **type of path** depends on whether it is a **use** (*use*) or **re-assignment** (*identity*).
* If the variable is **re-assigned**, we also create a connection towards the new variable (*assign*) and add it to $assignedVars.
* When **flowifying a Body/Lane** (usually a series of statements or expressions)
    * at the start we fill varsInScopeStart with the varsInScope of the parent
    * after flowifying each statement, we add the $assignedVars from each flowified statement (child-element) to the $varsInScope
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
* We check if for this variable the varsInScopeStart is different from the varsInScope. If it is, the var in somewhere in this Lane
    * We **loop though all the Lane's children backwards** (or start the last and use preivousELement each time). We do the following for each child
        * If the variableName is in **splitterVars/ joinerVars/ passthroughVars/ assignedVars**, we connect to that var (and are done)
        * If in the child, for this variable the varsInScopeStart is different from the varsInScope, then we recurse into this child as Lane
* If the variable was not inside the Lane, we go to the previousElement
    * If there a multiple previousElements, we recurse into all/both
        * Note: there can only be multiple previousElements, if we just 'entered' a Lane that **joins Lanes**, these are
            * Function: joining 'returns'
            * Catch: joining 'throws'
            * Done/For: joining 'breaks'
            * UpdateBody: joining 'continues'
            * If: joining 'normals' (then/else)
            * For/Condition: joining 'normals' (back/init?) 
    * We should detrmine whether the varsInScopeStart of the Lane where Lanes were **split** differ from the varsInScope of the Lane where they were joined.
        * Only if the variable was changed OR if the join was assymetric, should you build a path through all previousElements
