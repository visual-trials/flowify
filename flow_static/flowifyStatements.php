<?php

function flowifyProgram($statements) {
    
    $astNodeIdentifier = getAstNodeIdentifier(null);

    $rootFlowElement = createFlowElement('root', 'root', null, $astNodeIdentifier);

    // TODO: should we do anything with the return value of the main function?
    $openEndings = flowifyStatements($statements, $rootFlowElement);
    
    // FIXME: we should do something with the $openEndings!
    
    return $rootFlowElement;
}
    
function flowifyFunction ($functionStatement, $flowCallArguments, $functionCallFlowElement) { 

    // With a body: (maybe make this work) 
    //    - the parameters should be in the function-input
    //    - the statements should be in the function-body
    //    - the return variable(s) should be in the function-output

    // Without body:(current)
    //    - Everything should be inside the function-call (no input/body/output)
    
    $functionCallFlowElement->varsInScope = [];
    
    // TODO: In order for recursion to be possible, the function itself should be available in functionsInScope!
    //       Currently we reset the functionsInScope to prevent never ending recursion
    $functionCallFlowElement->functionsInScope = [];
    
    $functionName = $functionCallFlowElement->name;
    
    $argumentNumber = 0;
    foreach ($flowCallArguments as $flowCallArgument) {

        $parameters = $functionStatement['params'];
        // FIXME: there might be optional arguments...
        if (count($parameters) === count($flowCallArguments)) { // TODO: this is checked for every argument (= redundant)
            $parameter = $parameters[$argumentNumber];

            $parameterVar = $parameter['var'];

            $parameterName = null;
            if ($parameterVar['nodeType'] === 'Expr_Variable') {
                $parameterName = $parameterVar['name'];
            }
            else {
                die("Found" . $parameterVar['nodeType'] . " as nodeType inside 'var' of param\n");
            }

            $astNodeIdentifier = getAstNodeIdentifier($parameterVar);

            // Adding the parameter to the function
            $parameterFlowElement = createAndAddChildlessFlowElementToParent('variable', $parameterName, null, $astNodeIdentifier, $functionCallFlowElement);

            // Connecting the callArgument (outside the function) to the parameter (inside the function)
            addFlowConnection($flowCallArgument, $parameterFlowElement);

            // Setting the parameter as a local var within the function body
            $functionCallFlowElement->varsInScope[$parameterName] = $parameterFlowElement;
            
            // $functionCallFlowElement->usedVars[$parameterName] = 'created';

        }
        else {
            die("The number of call arguments for function $functionName is different from the number of arguments inside the function!\n");
        }

        $argumentNumber++;
    }

    $statements = $functionStatement['stmts'];

    // TODO: We are assuming that the statements of a function will always have a null-endingType or 'return'-endingType (not 'continue' or 'break')
    //       If the parser does not gaurd against this, we should.
    
    $openEndingsFunctionCall = flowifyStatements($statements, $functionCallFlowElement);
    
    // START OF FUNCTION getReturnFlowElementFromOpenEndings

    // FIXME: we should probably ALSO join all other variables inside these openEndings, and put them in
    //        the varsInScope of the functionCallFlowElement. This is especially the case then variables
    //        are passed by reference (to the function) and might be used after the function call. We should
    //        therefore join all variables.
    
    {    
        // If there are multiple open endings of type 'return', we join them
        if (count($openEndingsFunctionCall->returns) > 1) {
            $differentVariables = [];
            // TODO: how do we really know that all return variables are actually different? Of they are the same,
            //       should we add a passthrough by the ones who are the same?
            foreach ($openEndingsFunctionCall->returns as $openEndingElement) {
                array_push($differentVariables, $openEndingElement->returnVar);
            }
            
            // Note: we use '*' to make sure there is no collision with existing variableNames (since they are not allowed in variable-identifiers)
            $conditionalJoinVariableFlowElement = joinVariables('*RETURN*', $differentVariables, $functionCallFlowElement);
            $returnFlowElement = $conditionalJoinVariableFlowElement;
            
            // TODO: should we add the conditionalJoinVariable/returnFlowElement to our scope?
            //$functionCallFlowElement->varsInScope[$variableName] = $conditionalJoinVariableFlowElement; 
        }
        else if (count($openEndingsFunctionCall->returns) === 1) {
            // Exactly one openEnding of type 'return' found in statements of function call, so we use its returnVar as returnFlowElement
            $openEndingElement = reset($openEndingsFunctionCall->returns);
            $returnFlowElement = $openEndingElement->returnVar;
        }
        else {
            // No return found in statements of function call, meaning no returnFlowElement
            $returnFlowElement = null;
        }
    }
    
    // END OF FUNCTION getReturnFlowElementFromOpenEndings
    
    return $returnFlowElement;

}

function flowifyStatements ($statements, $bodyFlowElement) {

    // 1)    First find all defined functions, so we known the nodes of them, when they are called
    
    // TODO: function definitions actually work a little different in PHP:
    //          http://www.php.net/manual/en/functions.user-defined.php
    //         - they are globally defined
    //         - when in a condition, the condition has to be 'run'/processed before it's use
    //         - can be defined inside functions (but then have to be run prior to use?)
    //         - can be in a namespace
    
    foreach ($statements as $statement) {

        $statementType = $statement['nodeType'];

        if ($statementType === 'Stmt_Function') {
            $identifierNode = $statement['name']; // TODO: we are assuming 'name' is always present

            if ($identifierNode['nodeType'] === 'Identifier') {
                $bodyFlowElement->functionsInScope[$identifierNode['name']] = $statement;
            }
            else {
                echo "Found '" . $identifierNode['nodeType'] . "' as nodeType inside 'name' of 'Stmt_Function'\n";
            }
        }
    }

    $openEndings = new OpenEndings;

    // 2) Then loop through all non-fuctions and do as if they are "executed"...
    foreach ($statements as $statement) {

        $statementType = $statement['nodeType'];

        // We already did 'Stmt_Function' above
        if ($statementType === 'Stmt_Function') {
            continue;
        }

        if ($statementType === 'Stmt_Expression') {

            $expression = $statement['expr'];

            $flowElement = flowifyExpression($expression, $bodyFlowElement);

            // TODO: should we do anything with this $flowElement?
            //       if an expression is a statement, where does the output of that expesssion/statement go?

        }
        else if ($statementType === 'Stmt_Return') {

            $expression = $statement['expr'];

            // TODO: maybe add a FlowElement-wrapper around the expression that is returned (but only if it contains any children).
            
            $returnFlowElement = flowifyExpression($expression, $bodyFlowElement);

            // Note: we are assuming that when we reach a 'return' statement, all statements that follow will be unreachable. That's why 
            //       we stop looping through all the left-over statements and this bodyFlowElement as an openEnding of type 'return'.
            
            addElementToOpenEndings($bodyFlowElement, $openEndings, 'return', $returnFlowElement);
            break;
        }
        else if ($statementType === 'Stmt_Break') {

            // Note: we are assuming that when we reach a 'break' statement, all statements that follow will be unreachable. That's why 
            //       we stop looping through all the left-over statements and this bodyFlowElement as an openEnding of type 'break'.
            
            addElementToOpenEndings($bodyFlowElement, $openEndings, 'break');
            break;
        }
        else if ($statementType === 'Stmt_Continue') {

            // Note: we are assuming that when we reach a 'continue' statement, all statements that follow will be unreachable. That's why 
            //       we stop looping through all the left-over statements and this bodyFlowElement as an openEnding of type 'continue'.
            
            addElementToOpenEndings($bodyFlowElement, $openEndings, 'continue');
            break;
        }
        else if($statementType === 'Stmt_If') {
            
            $ifOpenEndings = flowifyIfStatement($statement, $bodyFlowElement);
            
            $openEndings = combineOpenEndings($ifOpenEndings, $openEndings);
            
            if ($bodyFlowElement->onlyHasOpenEndings) {
                // If the if-statement only had openEndings we stop looping statements here, since they have become unreachable.
                break;
            }
        }
        else if($statementType === 'Stmt_For') {
            
            $forOpenEndings = flowifyForStatement($statement, $bodyFlowElement);
            
            $openEndings = combineOpenEndings($forOpenEndings, $openEndings);
            
            if ($bodyFlowElement->onlyHasOpenEndings) {
                // If the for-statement only had openEndings we stop looping statements here, since they have become unreachable.
                break;
            }
        }
        else {
            echo "statementType '".$statementType."' found in function body, but not supported!\n";
            echo print_r($statement, true);
            continue;
        }

    }

    // Note: if no 'return', 'break' or 'continue' was encountered, the endsWith will be null and 
    //       the onlyHasOpenEndings of the $bodyFlowElement will be false
    
    return $openEndings;

}

function flowifyIfStatement($ifStatement, $parentFlowElement) {
    
    $ifOpenEndings = new OpenEndings;

    $ifAstNodeIdentifier = getAstNodeIdentifier($ifStatement);
    $ifFlowElement = createAndAddFlowElementToParent('ifMain', 'if', null, $ifAstNodeIdentifier, $parentFlowElement);
    $ifFlowElement->canContainSplitters = true;
    
    {
        // == COND ==
        
        $conditionExpression = $ifStatement['cond'];
        
        $condAstNodeIdentifier = $ifAstNodeIdentifier . "_IfCond";
        $condFlowElement = createAndAddFlowElementToParent('ifCond', 'cond', null, $condAstNodeIdentifier, $ifFlowElement);
        
        $flowElement = flowifyExpression($conditionExpression, $condFlowElement);
        // addUsedVarsToParent($condFlowElement, $ifFlowElement);
        
        // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
        //       whether the then-statements or the else(if)-statements are executed. How to should
        //       we visualize this?
        
        $varsInScopeAfterCondBody = $condFlowElement->varsInScope; // copy!
        
        // == THEN ==
        
        $thenStatements = $ifStatement['stmts'];
        
        $thenAstNodeIdentifier = getAstNodeIdentifier($thenStatements);
        $thenBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'then', null, $thenAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
        
        // Note: we *copy* the varsInScope here. This is because the thenBody might replace vars in it's scope,
        //       These are however conditional-replacement when it comes to the if-statement. 
        //       Instead of the thenBody letting the vars in the if-scope to be replaced, we *add* it later to our varsInScope,
        //       by using a conditionalFlowElement.
        
        $thenBodyFlowElement->varsInScope = $ifFlowElement->varsInScope;  // copy!
        $thenBodyFlowElement->functionsInScope = &$ifFlowElement->functionsInScope;
        
        $thenOpenEndings = flowifyStatements($thenStatements, $thenBodyFlowElement);
        // addUsedVarsToParent($thenBodyFlowElement, $ifFlowElement);
        
        $ifOpenEndings = combineOpenEndings($thenOpenEndings, $ifOpenEndings);
        
        if ($thenBodyFlowElement->onlyHasOpenEndings) {
            // FIXME: what to do here?
        }
        
        // == ELSE ==
        
        $elseStatement = $ifStatement['else'];
        $elseBodyHasReturn = false;
        
        $elseBodyFlowElement = null;
        if ($elseStatement !== null) {
            
            if ($elseStatement['nodeType'] !== 'Stmt_Else') {
                die("Expected nodeType 'Stmt_Else' in else. Not found.");
            }
            
            // There is an else-statement, getting the body of statements in it
            $elseStatements = $elseStatement['stmts'];
            
            $elseAstNodeIdentifier = getAstNodeIdentifier($elseStatements);
            $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
            
            // Note: we *copy* the varsInScope here. This is because the elseBody might replace vars in it's scope,
            //       These are however conditional-replacement when it comes to the if-statement. 
            //       Instead of the elseBody letting the vars in the if-scope to be replaced, we *add* it later to our varsInScope,
            //       by using a conditionalFlowElement.
            
            $elseBodyFlowElement->varsInScope = $ifFlowElement->varsInScope;  // copy!
            $elseBodyFlowElement->functionsInScope = &$ifFlowElement->functionsInScope;
            
            // TODO: we don't have a return statement in then-bodies, so we call it $noReturnFlowElement here (but we shouldn't get it at all)
            $elseOpenEndings = flowifyStatements($elseStatements, $elseBodyFlowElement);
            // addUsedVarsToParent($elseBodyFlowElement, $ifFlowElement);
            
            $ifOpenEndings = combineOpenEndings($elseOpenEndings, $ifOpenEndings);
            
            if ($elseBodyFlowElement->onlyHasOpenEndings) {
                // FIXME: what to do here?
            }
        }
        else {
            
            // Add an elseBody if it doesn't exist yet
            // FIXME: what if an implicit else is never needed?
            $elseAstNodeIdentifier = $ifAstNodeIdentifier . "_ImplicitElse";
            // FIXME: this should be of type: 'ifElseImplicit'
            $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
            
            $elseBodyFlowElement->varsInScope = $ifFlowElement->varsInScope;  // copy!
            $elseBodyFlowElement->functionsInScope = &$ifFlowElement->functionsInScope;
        }
        
        if ($thenBodyFlowElement->onlyHasOpenEndings && $elseBodyFlowElement->onlyHasOpenEndings) {
            // Both the thenBody and the elseBody have only openEndings. This means the ifBody and its parent also only have openEndings 
            $ifFlowElement->onlyHasOpenEndings = true;
            $parentFlowElement->onlyHasOpenEndings = true;
            // FIXME: should we break here?
        }
        
        
        // FIXME: if either the thenBody- or the elseBody onlyHasOpenEndings, we should NOT JOIN here!
        
        
        // Adding a passthrough variable if either side has changed a variable, while the other has not
        // addPassThroughsBasedOnChange($thenBodyFlowElement, $elseBodyFlowElement, $varsInScopeAfterCondBody);
        
        // Joining variables between then and else, if they are different
        $varsInScopeAfterJoining = joinVariablesBasedOnDifference($thenBodyFlowElement, $elseBodyFlowElement, $ifFlowElement);

        // After joining, the ifFlowElement should get the joinedVars in it's scope, so elements after that can connect to the joinedVars
        $ifFlowElement->varsInScope = $varsInScopeAfterJoining; // copy!

        // Splitting variables if either side (then or else) has used it
        splitVariablesBasedOnUsage($varsInScopeAfterCondBody, $thenBodyFlowElement, $elseBodyFlowElement, null, $ifFlowElement);
        
            
        foreach ($thenBodyFlowElement->varsInScope as $variableName => $thenBodyVarInScopeElement) {
            
            // We check if we have the same variable in our parent's scope
            if (!array_key_exists($variableName, $parentFlowElement->varsInScope)) {
                
                // the variable exists in the thenBody scope, but not in the parent's scope, so it must have been declared in the thenBody
            
                // TODO: depending on the language, variables DECLARED in an then- or else- statement are or are not
                //       available in the scope outside the then- or else- statement.
                //       We assume here that the current language has BLOCK-scope, so any newly declared variables
                //       within the then- or else- statement, are NOT copied back into the if- scope!
                //       The problem: PHP does not have this kind of scoping behaviour, right? (should investigate)
            }

        }
        
        foreach ($elseBodyFlowElement->varsInScope as $variableName => $elseBodyVarInScopeElement) {
            
            // We check if we have the same variable in our parent's scope
            if (!array_key_exists($variableName, $parentFlowElement->varsInScope)) {
                
                // the variable exists in the elseBody scope, but not in the parent's scope, so it must have been declared in the elseBody
            
                // TODO: depending on the language, variables DECLARED in an then- or else- statement are or are not
                //       available in the scope outside the then- or else- statement.
                //       We assume here that the current language has BLOCK-scope, so any newly declared variables
                //       within the then- or else- statement, are NOT copied back into the if- scope!
                //       The problem: PHP does not have this kind of scoping behaviour, right? (should investigate)
            }

        }
        
        // TODO: what should be done if the SAME variable was declared in the thenBody AND the elseBody, but wasn't present in the parent's scope?
        
        
        // == ELSEIF ==
        
        // TODO: $elseIfStatements = $ifStatement['elseif']

    }
    
    // addUsedVarsToParent($ifFlowElement, $parentFlowElement);

    return $ifOpenEndings;
}


function flowifyForStatement($forStatement, $parentFlowElement) {
    
    // Note: We are assuming a programming language with FUNCTION scope here!
    //       The variables declared inside the init-statement will be part
    //       of the for's scope right now, because it uses the for's scope
    //       and will simply add it to it's scope.
    
    // Note: Php allows comma separated expressions (but ONLY inside the for-statement):
    //    http://php.net/manual/en/language.expressions.php#90327
    // TODO: make sure more than 1 expression is used for the 'init', 'cond' and 'loop' segments 
    
    $initExpression = $forStatement['init'][0]; // TODO: hardcoded to 1 statement/expression! Make sure there is always one!
    $conditionExpression = $forStatement['cond'][0]; // TODO: hardcoded to 1 statement/expression! Make sure there is always one!
    $updateExpression = $forStatement['loop'][0]; // TODO: hardcoded to 1 statement/expression! Make sure there is always one!
    $iterStatements = $forStatement['stmts'];
    
    
    $forAstNodeIdentifier = getAstNodeIdentifier($forStatement);
    // FIXME: change this from a ifMain for a forMain
    $forFlowElement = createAndAddFlowElementToParent('ifMain', 'for', null, $forAstNodeIdentifier, $parentFlowElement);

    {
            
        // == INIT ==
        
        $initAstNodeIdentifier = $forAstNodeIdentifier . "_ForInit";
        // FIXME: replace ifCond with forInit
        $initFlowElement = createAndAddFlowElementToParent('ifCond', 'init', null, $initAstNodeIdentifier, $forFlowElement);
        $flowElement = flowifyExpression($initExpression, $initFlowElement);
        // addUsedVarsToParent($initFlowElement, $forFlowElement);
            
        // == DONE ==
        
        $doneAstNodeIdentifier = $forAstNodeIdentifier . "_ImplicitDone";
        // FIXME: this should be of type: 'forDoneImplicit'
        $doneBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'done', null, $doneAstNodeIdentifier, $forFlowElement);
        
        // == BACK ==
        
        $backAstNodeIdentifier = $forAstNodeIdentifier . "_ImplicitBack";
        // FIXME: this should be of type: 'forBackImplicit'
        $backBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'back', null, $backAstNodeIdentifier, $forFlowElement);
        
     
        // == COND / ITER / UPDATE ==
        
        
        // STEP 1
        
        $forStepAstNodeIdentifier1 = $forAstNodeIdentifier . "_1";
        // FIXME: change this from a ifThen for a forStep
        $forStepFlowElement1 = createAndAddFlowElementToParent('ifThen', '#1', null, $forStepAstNodeIdentifier1, $forFlowElement, $useVarScopeFromParent = false);
        $forStepFlowElement1->varsInScope = $forFlowElement->varsInScope; // copy!
        $forStepFlowElement1->functionsInScope = &$forFlowElement->functionsInScope;
        $forStepFlowElement1->canContainSplitters = true;
        
        flowifyForIteration(
            $conditionExpression, 
            $iterStatements, 
            $updateExpression,
            $forAstNodeIdentifier,
            $doneBodyFlowElement,
            $backBodyFlowElement,
            $forFlowElement,
            //$doneBodyFlowElement,
            $forStepAstNodeIdentifier1,
            $forStepFlowElement1
        );
        // Since we now consider the loop to be finished, we take the doneBody and copy its varsInScope back to the varsInScope of the forStepFlowElement
        $forFlowElement->varsInScope = $doneBodyFlowElement->varsInScope; // copy back!

        // addUsedVarsToParent($forStepFlowElement1, $forFlowElement);
        
        /*
        // STEP 2
        
        $forStepAstNodeIdentifier2 = $forAstNodeIdentifier . "_2";
        // FIXME: change this from a ifThen for a forStep
        $forStepFlowElement2 = createAndAddFlowElementToParent('ifThen', '#2', null, $forStepAstNodeIdentifier2, $forFlowElement, $useVarScopeFromParent = false);
        $forStepFlowElement2->varsInScope = $forFlowElement->varsInScope; // copy!
        $forStepFlowElement2->functionsInScope = &$forFlowElement->functionsInScope;
        $forStepFlowElement2->canContainSplitters = true;
        
        flowifyForIteration(
            $conditionExpression, 
            $iterStatements, 
            $updateExpression,
            $forAstNodeIdentifier,
            $doneBodyFlowElement,
            $backBodyFlowElement,
            $forFlowElement,
            $forStepAstNodeIdentifier2,
            $forStepFlowElement2
        );
        $forFlowElement->varsInScope = $forStepFlowElement2->varsInScope; // copy back!
        
        // addUsedVarsToParent($forStepFlowElement2, $forFlowElement);
        
        */

        // TODO: implement continue statement (inside flowifyStatements)
        // TODO: implement break statement (inside flowifyStatements)
    

    }    
    
    // addUsedVarsToParent($forFlowElement, $parentFlowElement);

}

function flowifyForIteration (
        $conditionExpression, 
        $iterStatements, 
        $updateExpression,
        $forAstNodeIdentifier,
        $doneBodyFlowElement,
        $backBodyFlowElement,
        $forFlowElement,
        $forStepAstNodeIdentifier,
        $forStepFlowElement
    ) {

    // == COND ==
    
    // FIXME: replace ifCond with forCond
    $condBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'cond', null, $forAstNodeIdentifier . "_ForCond", $forStepFlowElement);
    $flowElement = flowifyExpression($conditionExpression, $condBodyFlowElement);
    // addUsedVarsToParent($condBodyFlowElement, $forStepFlowElement);

    $varsInScopeAfterCondBody = $forStepFlowElement->varsInScope; //copy!

    // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
    //       whether the iter-statements are executed. How to should we visualize this?
    
    // == ITER ==

    $iterAstNodeIdentifier = getAstNodeIdentifier($iterStatements);
    
    // FIXME: replace ifThen with iterBody
    $iterBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'iter', null, $iterAstNodeIdentifier, $forStepFlowElement);
    $iterOpenEndings = flowifyStatements($iterStatements, $iterBodyFlowElement);
    // addUsedVarsToParent($iterBodyFlowElement, $forStepFlowElement);
    // FIXME: do something with $iterOpenEndings!

    // FIXME: If iterOpenEndings contains a 'continue', we need to join the varsInScope of the continue-body (for example a then-body)
    //        with the varsInScope of the iterBodyFlowElement. We need to do this before the update.
    //        We also need to add a passthrough for that and split it.

    // FIXME: if we have multiple continues, we need to combine them first?
    
    // for the ONE 'continue' in iterOpenEndings do the following
    {

        if (count($iterOpenEndings->continues) > 0) {
            // 1) addPassThroughsBasedOnChange(continueBody, iterBodyFlowElement, varsInScopeBeforeIterBody?) --> should addPassThrough be a recurive function?
            //       if the iterBodyFlowElement has changed vars (after the continue-statement) and the continue has not (necessarily true),
            //       then this var should be passed through the continueBody
        
            // FIXME: HACK!
            $bodyThatEndsWithContinue = reset($iterOpenEndings->continues);
            addPassThroughsBasedOnChange($bodyThatEndsWithContinue, $iterBodyFlowElement, $varsInScopeAfterCondBody);
                
             // 2) joinVariablesBasedOnDifference(varsInScopeBeforeIterBody?, iterBodyFlowElement->varsInScope, updateBody?)
            //       the passthrough var in the continueBody should be joined with the (changed) var in the iterBodyFlowElement
            //       here we assume we add the joinVar inside the updateBody (which we haven't created yet!) so we probably
            //       have to do steps 1-3 between creating the updateBodyFlowElement and flowifying the updateExpression.
            
            $varsInScopeAfterJoining = joinVariablesBasedOnDifference($bodyThatEndsWithContinue, $iterBodyFlowElement, $iterBodyFlowElement);
       
            // FIXME: is this indeed the right way to do this? or should the join var be created in the forStepBody?
            $forStepFlowElement->varsInScope = $varsInScopeAfterJoining; // copy back!
            
            // 3) splitVariablesBasedOnUsage(varsInScopeBeforeIF_THAT_CONTAINS_THE_CONTINUE???, thenBodyCONTAINING_THE_CONTINUE, iterBodyFlowElement-part_AFTER_THE_IF_CONTAINING_THE_CONTINUE?)
            //       we somehow need to split at the right place. Maybe this place should be found (and executed) by the recursive addPassThroughsBasedOnChange?
                   
            // FIXME: we should somehow do this!
            
        }
        
        

        
        
    }
    
    // == UPDATE ==
    
    // FIXME: replace ifCond with forUpdate
    $updateBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'update', null, $forAstNodeIdentifier . "_ForUpdate", $forStepFlowElement);
    $flowElement = flowifyExpression($updateExpression, $updateBodyFlowElement);
    // $forStepFlowElement->usedVars = array_merge($updateBodyFlowElement->usedVars, $forStepFlowElement->usedVars);
    
    // Adding a passthrough variable if the iter/update side has changed a variable: the done-side then needs a passthrough
    addPassThroughsBasedOnChange($doneBodyFlowElement, $forStepFlowElement, $varsInScopeAfterCondBody);
    
    // Joining variables between beforeCondBody and afterForStep, if they are different
    $varsInScopeAfterJoining = joinVariablesBasedOnDifference($forFlowElement, $forStepFlowElement, $forStepFlowElement, $backBodyFlowElement, $updateExistingConnections = true);

    // FIXME: removed vars that were CREATED inside the loop! We need a better way to do this!
    $strippedVarsInScopeAfterJoining = [];
    foreach ($varsInScopeAfterCondBody as $variableName => $varInScopeAfterCondBody) {
        $strippedVarsInScopeAfterJoining[$variableName] = $varsInScopeAfterJoining[$variableName];
    }
    splitVariablesBasedOnUsage($strippedVarsInScopeAfterJoining, $doneBodyFlowElement, $iterBodyFlowElement, $updateBodyFlowElement, $forStepFlowElement);
    
}



function joinVariables($variableName, $differentVariables, $targetElement) {
    
    $conditionalJoinVariableAstNodeIdentifier = $targetElement->astNodeIdentifier . "_JOINED_" . $variableName;
    $conditionalJoinVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalJoinVariable', $variableName, null, $conditionalJoinVariableAstNodeIdentifier, $targetElement);
    foreach ($differentVariables as $differentVariableElement) {
        $flowConntectionId = addFlowConnection($differentVariableElement, $conditionalJoinVariableFlowElement, 'conditional');
    }
    return $conditionalJoinVariableFlowElement;
}

// FIXME: we should give this functoin the thenBody and elseBody, instead of their varsInScope
//        we can then give this thenBody or elseBody as the 'laneElement' to the buildPathBackwardsToElementFromVariable
function joinVariablesBasedOnDifference ($firstLane, $secondLane, $targetElement, $passBackBodyFlowElement = null, $updateExistingConnections = false) {

    $firstVarsInScope = $firstLane->varsInScope;
    $secondVarsInScope = $secondLane->varsInScope;
    
    // FIXME: we can most likely simplyfy the code below, since we only have two scopes (not an arbitrary amount anymore)
    //        we can probably get rid of 'doPassBack'
    //        whether we want to keep doPassBack depends on how we implement 'continue' in for loops
    
    $varsInScopeAfterJoining = $targetElement->varsInScope; // copy!
    
    $differentVariablesPerVariableName = [];
    foreach ($firstVarsInScope as $variableName => $variableFlowElement) {
        if (!array_key_exists($variableName, $differentVariablesPerVariableName)) {
            $differentVariablesPerVariableName[$variableName] = [];
        }
        // TODO: workaround for PHP not allowing numbers as keys
        // FIXME: if there are multiple lanes with the same variable, we lose the information about these lanes! We should keep each lane/variable combination!
        $differentVariablesPerVariableName[$variableName]['id:' . $variableFlowElement->id] = [ 'variable' => $variableFlowElement, 'lane' => $firstLane];
    }
    foreach ($secondVarsInScope as $variableName => $variableFlowElement) {
        if (!array_key_exists($variableName, $differentVariablesPerVariableName)) {
            $differentVariablesPerVariableName[$variableName] = [];
        }
        // We assume that the secondVarsInScope should be passed-back
        $variableFlowElement->doPassBack = true;
        // TODO: workaround for PHP not allowing numbers as keys
        // FIXME: if there are multiple lanes with the same variable, we lose the information about these lanes! We should keep each lane/variable combination!
        $differentVariablesPerVariableName[$variableName]['id:' . $variableFlowElement->id] = [ 'variable' => $variableFlowElement, 'lane' => $secondLane];
    }

    foreach ($differentVariablesPerVariableName as $variableName => $differentVariablesWithLanes) {
        // Only if we found more than 1 different variable should we join them
        if (count($differentVariablesWithLanes) > 1) {
            
            $differentVariablesWithLanesToBeJoined = [];
            if ($passBackBodyFlowElement !== null) {
                // We should do pass-back the variables when their doPassBack is set to true
                foreach (array_values($differentVariablesWithLanes) as $differentVariableWithLane) {
                    $differentVariable = $differentVariableWithLane['variable'];
                    if ($differentVariable->doPassBack) {
                        // We need to pass-back this variable. We first create a pass-back flowElement,
                        // add it to $passBackBodyFlowElement and connect it with $differentVariable
                        // we then add the pass-back flowElement to $differentVariablesWithLanesToBeJoined
                        
                        // FIXME: it is conceivable that the SAME variable(name) is pass-backed for more than ONE flowElementBodiesWithDifferentScope
                        //        we should have a pass-back body for each flowElementBodiesWithDifferentScope, or we need find another
                        //        way to make sure this passBackVariableAstNodeIdentifier is unique
                        $passBackVariableAstNodeIdentifier = $targetElement->astNodeIdentifier . "_PASSBACK_" . $variableName;
                        $passBackVariableFlowElement = createAndAddChildlessFlowElementToParent('passBackVariable', $variableName, null, $passBackVariableAstNodeIdentifier, $passBackBodyFlowElement);
                        addFlowConnection($differentVariable, $passBackVariableFlowElement, 'conditional');
                        
                        // TODO: is it possible that $differentVariable already had connections *from* itself to others? If so, we need to deal with those connections!
                        
                        array_push($differentVariablesWithLanesToBeJoined, [ 'variable' => $passBackVariableFlowElement, 'lane' => $differentVariableWithLane['lane'] ]);
                    }
                    else {
                        // No need to pass-back this variable, so it can be added to differentVariablesWithLanesToBeJoined
                        array_push($differentVariablesWithLanesToBeJoined, $differentVariableWithLane);
                    }
                }
                
            }
            else {
                // There is no passBackBodyFlowElement, so no need to check which variables have to be
                // pass-backed. We simply set differentVariablesWithLanesToBeJoined to add all 
                // original ones (without pass-back in between).
                foreach (array_values($differentVariablesWithLanes) as $differentVariableWithLane) {
                    array_push($differentVariablesWithLanesToBeJoined, $differentVariableWithLane);
                }
            }
            
            
            // FIXME: should we keep this? or can we put it into buildPathBackwardsToElementFromVariable?
            if ($updateExistingConnections) {
                // FIXME: also remove joinVariables?
                $differentVariablesToBeJoined = [];
                foreach (array_values($differentVariablesWithLanes) as $differentVariableWithLane) {
                    array_push($differentVariablesToBeJoined, $differentVariableWithLane['variable']);
                }
                $conditionalJoinVariableFlowElement = joinVariables($variableName, $differentVariablesToBeJoined, $targetElement);
                
                $varsInScopeAfterJoining[$variableName] = $conditionalJoinVariableFlowElement;
            
                foreach ($differentVariablesWithLanesToBeJoined as $differentVariableWithLane) {
                    $differentVariable = $differentVariableWithLane['variable'];
                    
                    if (count($differentVariable->connectionIdsFromThisElement) > 0) {
                        // There are connections from the variable, we have to update all the connections from it,
                        // since they should now point towards the newly created conditionalJoinVariableFlowElement
                        $updatedConnectionIdsFromThisElement = [];
                        foreach ($differentVariable->connectionIdsFromThisElement as $connectionIdFromVariable) {
                            $connectionToBeChanged = getConnectionById($connectionIdFromVariable);
                            $connectedToFlowElementId = $connectionToBeChanged->to;
                            // We don't want to change the connections we just created by calling joinVariables(). Those
                            // connections were connected to the conditionalJoinVariableFlowElement, so we skip those.
                            if ($connectedToFlowElementId !== $conditionalJoinVariableFlowElement->id) {
                                $connectionToBeChanged->from = $conditionalJoinVariableFlowElement->id;
                                // This connection is now between the $conditionalJoinVariableFlowElement and
                                // another element inside one of the flowElementBodies. It is *not* connected
                                // with $differentVariable anymore, so we don't add this connection to 
                                // $updatedConnectionIdsFromThisElement
                                
                                // We do however add this connection to the connectionIdsFromThisElement of
                                // $conditionalJoinVariableFlowElement
                                array_push($conditionalJoinVariableFlowElement->connectionIdsFromThisElement, $connectionIdFromVariable);
                            }
                            else {
                                // We are now changing this connection, so it should stay in connectionIdsFromThisElement
                                // of $differentVariable. We therefore add it to updatedConnectionIdsFromThisElement.
                                array_push($updatedConnectionIdsFromThisElement, $connectionToBeChanged->from);
                            }
                        }
                        $differentVariable->connectionIdsFromThisElement = $updatedConnectionIdsFromThisElement;
                    }
                }
            }
            else {
                $conditionalJoinVariableAstNodeIdentifier = $targetElement->astNodeIdentifier . "_JOINED_" . $variableName;
                $conditionalJoinVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalJoinVariable', $variableName, null, $conditionalJoinVariableAstNodeIdentifier, $targetElement);

                $varsInScopeAfterJoining[$variableName] = $conditionalJoinVariableFlowElement;
                
                $parentOfConditionalJoin = getParentElement($conditionalJoinVariableFlowElement);
                foreach ($differentVariablesWithLanesToBeJoined as $differentVariableWithLane) {
                    $differentVariable = $differentVariableWithLane['variable'];
                    $differentLane = $differentVariableWithLane['lane'];

                    buildPathBackwardsToElementFromVariable($differentLane, $conditionalJoinVariableFlowElement, $differentVariable, $variableName);
                }
            }
        }
    }

    // Resetting doPassBack to false
    foreach ($firstVarsInScope as $variableName => $variableFlowElement) {
        $variableFlowElement->doPassBack = false;
    }
    foreach ($secondVarsInScope as $variableName => $variableFlowElement) {
        $variableFlowElement->doPassBack = false;
    }

    return $varsInScopeAfterJoining;
}

function buildPathBackwardsToElementFromVariable($laneElement, $toElement, $fromVariable, $variableName) {
    
    $isAncestorOfFromVariable = isAncestorOf($laneElement, $fromVariable);
    if ($isAncestorOfFromVariable) {
        // Connect the fromVariable to the toElement
        // TODO: what connectionType should we use here?
        addFlowConnection($fromVariable, $toElement);
    }
    else {
        
        
        // TODO: we still need to check for open ends and deal with them...
        
        
        $parentOfLaneElement = getParentElement($laneElement);
        
        if ($parentOfLaneElement !== null) {
   
            if ($parentOfLaneElement->canContainSplitters) {
                
                {
                    // Adding a passthrough
                    
                    $passThroughVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
                    $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $laneElement);

                    
                    // Connecting the passthrough variable to the toElement to the
                    addFlowConnection($passThroughVariableFlowElement, $toElement);
                    
                    // Setting the toElement to the passThroughVariableFlowElement
                    $toElement = $passThroughVariableFlowElement;

                    // We add this passthrough variable to the scope of the laneElement
                    $laneElement->varsInScope[$variableName] = $passThroughVariableFlowElement;
                }

                
                {
                    // Getting (or if not available creating) a splitter
                    // Note: we could also do this in the next recursion!
                    
                    // TODO: get/add splitter!
                    
                }
                
            }
            else {
                // there is no need to split, we simply continue to the next parent
            }
            
            
            buildPathBackwardsToElementFromVariable($parentOfLaneElement, $toElement, $fromVariable, $variableName);
            
        }
        else {
            // We somehow reached the root node and apparently it is not an ancestor of the toElement (this should not be possible
            die("toElement is somehow not a child of the root element!");
        }
    }
    
}

function addPassThroughsBasedOnChange($thenBodyFlowElement, $elseBodyFlowElement, $varsInScopeBeforeChange) {
    
    // We loop through all the vars in varsInScopeBeforeChange
    foreach ($varsInScopeBeforeChange as $variableName => $varInScopeBeforeChange) {
        
        $varReplacedInThenBody = false;
        $varReplacedInElseBody = false;
        
        // We check if we have the same variable in our thenBody scope
        if (array_key_exists($variableName, $thenBodyFlowElement->varsInScope)) {
            // The var exists both in thenBodyFlowElement and in the parent's scope
            if ($thenBodyFlowElement->varsInScope[$variableName]->id !== $varsInScopeBeforeChange[$variableName]->id) {
                // The vars differ, so it must have been replaced (or extended) inside the thenBody. 
                $varReplacedInThenBody = true;
            }
        }
      
        // We check if we have the same variable in our elseBody scope
        if (array_key_exists($variableName, $elseBodyFlowElement->varsInScope)) {
            // The var exists both in elseBodyFlowElement and in the parent's scope
            if ($elseBodyFlowElement->varsInScope[$variableName]->id !== $varsInScopeBeforeChange[$variableName]->id) {
                // The vars differ, so it must have been replaced (or extended) inside the elseBody. 
                $varReplacedInElseBody = true;
            }
        }

        if ($varReplacedInThenBody && $varReplacedInElseBody) {
            // SInce both of them are replaced. we don't need to add a passthrough, so nothing to do here
        }
        else if ($varReplacedInThenBody) {
            // Only the thenBody has replaced the variable. We use the parent's variable as the (default) else variable
            
            // Adding the variable to the elseBody as a passthrough variable
            $passThroughVariableAstNodeIdentifier = $elseBodyFlowElement->astNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
            $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $elseBodyFlowElement);

            // Connecting the variable in the parent to the passthrough variable (inside the thenBody)
            addFlowConnection($varsInScopeBeforeChange[$variableName], $passThroughVariableFlowElement);

            // We add this passthrough variable to the scope of the elseBody
            $elseBodyFlowElement->varsInScope[$variableName] = $passThroughVariableFlowElement;
            
        }
        else if ($varReplacedInElseBody) {
            // Only the elseBody has replaced the variable. We use the parent's variable as the (default) then variable
            
            // Adding the variable to the thenBody as a passthrough variable
            $passThroughVariableAstNodeIdentifier = $thenBodyFlowElement->astNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
            $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $thenBodyFlowElement);

            // Connecting the variable in the parent to the passthrough variable (inside the thenBody)
            addFlowConnection($varsInScopeBeforeChange[$variableName], $passThroughVariableFlowElement);

            // We add this passthrough variable to the scope of the thenBody
            $thenBodyFlowElement->varsInScope[$variableName] = $passThroughVariableFlowElement;
            
        }
        else {
            // The variable wasn't replaced by either the thenBody or the elseBody, so nothing to do here
        }
        
    }
}

function splitVariablesBasedOnUsage($varsInScopeAfterCondBody, $thenBodyFlowElement, $elseBodyFlowElement, $secondElseBodyFlowElement, $parentFlowElement) {

    // FIXME: workaround with 'second' else body
    
    // FIXME: don't call it then and else, call it left/right or true/false
        
    foreach ($varsInScopeAfterCondBody as $variableName => $variableAfterCondBody) {

        // We also want to create a conditional *split* element between the condBody and thenBody, and between the condBody and elseBody
        // We need to know the connections going from the condBody into the thenBody and elseBody (for this variable)
        // We do this by looping all the connections from the condBody for this variable (in its 'connectionIdsFromThisElement')
        $conditionalSplitVariableFlowElement = null;
        $connectionIdToConditionalSplitVariable = null;
        $connectionTypeToConditionalSplitVariable = null;
        $updatedConnectionIdsFromThisElement = [];
        
        // TODO: could this be moved outside the foreach of the $varsInScopeAfterCondBody? Or are we adding elements to the thenBody and elseBody in this loop?
        // TODO: we should probably use a hashmap of all flowElements inside the thenBody and elseBody.
        $elementIdsInThenBody = getElementsIdsIn($thenBodyFlowElement);
        $elementIdsInElseBody = [];
        if ($elseBodyFlowElement !== null) {
            $elementIdsInElseBody = getElementsIdsIn($elseBodyFlowElement);
        }
        $elementIdsInSecondElseBody = [];
        if ($secondElseBodyFlowElement !== null) {
            $elementIdsInSecondElseBody = getElementsIdsIn($secondElseBodyFlowElement);
        }
            
        foreach ($variableAfterCondBody->connectionIdsFromThisElement as $connectionIdFromVariable) {
            // By default we want to keep the connections, so we take over the id in the loop
            $currentConnectionIdFromThisElement = $connectionIdFromVariable;
            $newlyAddedConnectionIdFromThisElement = null;
            
            $connectionToBeChanged = getConnectionById($connectionIdFromVariable);
            $flowElementIdInThenOrElseBody = $connectionToBeChanged->to;
            
            // Here we check whether the flowElement to which the variableAfterCondBody is connected to, is in the thenBody or elseBody.
            $variableConnectedWithThenBody = false;
            $variableConnectedWithElseBody = false;
            if (in_array($flowElementIdInThenOrElseBody, $elementIdsInThenBody)) {
                $variableConnectedWithThenBody = true;
            }
            if (in_array($flowElementIdInThenOrElseBody, $elementIdsInElseBody)) {
                $variableConnectedWithElseBody = true;
            }
            if (in_array($flowElementIdInThenOrElseBody, $elementIdsInSecondElseBody)) {
                $variableConnectedWithElseBody = true;
            }
                
            if ($variableConnectedWithThenBody || $variableConnectedWithElseBody) {
                if ($conditionalSplitVariableFlowElement === null) {
                    // Adding the conditionalSplitVariableFlowElement and adding a connection to connect from the variableAfterCondBody to it
                    $connectionTypeToConditionalSplitVariable = $connectionToBeChanged->type;
                    
                    // FIXME: use "*SPLIT*" and put it BEFORE the variable!
                    // FIXME: is this AST Identifier correct?
                    $conditionalSplitVariableAstNodeIdentifier = $parentFlowElement->astNodeIdentifier . "_" . $variableName . "_SPLIT";
                    // FIXME: should this be put into the ifBody or the condBody?
                    $conditionalSplitVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalSplitVariable', $variableName, null, $conditionalSplitVariableAstNodeIdentifier, $parentFlowElement);
                    
                    // Adding a connection from the variableAfterCondBody to the conditionalSplitVariableFlowElement
                    $connectionIdToConditionalSplitVariable = addFlowConnection($variableAfterCondBody, $conditionalSplitVariableFlowElement, $connectionToBeChanged->type); // Note: we use the original type
                    // This connection will effectively be added to $variableAfterCondBody->connectionIdsFromThisElement
                    $newlyAddedConnectionIdFromThisElement = $connectionIdToConditionalSplitVariable;
                }

                // We set the from in the connection to the flowElementIdInThenOrElseBody
                // FIXME: we should add to which SIDE the connection is connected: true-side or false-side (depending on THEN or ELSE)
                $connectionToBeChanged->from = $conditionalSplitVariableFlowElement->id; // TODO: should we do it this way?
                // we add the connection to the connectionIdsFromThisElement of the $conditionalSplitVariableFlowElement
                array_push($conditionalSplitVariableFlowElement->connectionIdsFromThisElement, $connectionToBeChanged->id);
                // Seting currentConnectionIdFromThisElement to null, so it won't be added again to (effectively removed from) $variableAfterCondBody->connectionIdsFromThisElement
                $currentConnectionIdFromThisElement = null;
                
                // FIXME: right now null means 'normal' (which should overrule). We should change the default to 'dataflow' or something
                if ($connectionToBeChanged->type === null) {
                    // TODO: should we keep a prio number for each type of connection and check if the prio is higher here?
                    $connectionTypeToConditionalSplitVariable = null; 
                }
                
            }
            if ($currentConnectionIdFromThisElement !== null) {
                array_push($updatedConnectionIdsFromThisElement, $currentConnectionIdFromThisElement);
            }
            if ($newlyAddedConnectionIdFromThisElement !== null) {
                array_push($updatedConnectionIdsFromThisElement, $newlyAddedConnectionIdFromThisElement);
            }
        }
        $variableAfterCondBody->connectionIdsFromThisElement = $updatedConnectionIdsFromThisElement;

        if ($connectionIdToConditionalSplitVariable !== null) {
            // Setting the connectionType of the connection to the connectionToConditionalSplitVariable
            $connectionToConditionalSplitVariable = getConnectionById($connectionIdToConditionalSplitVariable);
            $connectionToConditionalSplitVariable->type = $connectionTypeToConditionalSplitVariable;
        }
        
    }
    
    
}
