<?php

function flowifyProgram($statements) {
    
    $astNodeIdentifier = getAstNodeIdentifier(null);

    $rootFlowElement = createFlowElement('root', 'root', null, $astNodeIdentifier);
    $rootFlowElement->sendsChangesToOutside = false;
    $rootFlowElement->receivesChangesFromOutside = false;

    flowifyStatements($statements, $rootFlowElement);
    
    return $rootFlowElement;
}
    
function flowifyFunction ($functionStatement, $flowCallArguments, $functionCallFlowElement) { 

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
            $parameterFlowElement = createVariable($functionCallFlowElement, $parameterName, $astNodeIdentifier);

            // Connecting the callArgument (outside the function) to the parameter (inside the function)
            addFlowConnection($flowCallArgument, $parameterFlowElement);

            // Setting the parameter as a local var within the function body
            // $functionCallFlowElement->varsInScope[$parameterName] = $parameterFlowElement;   // FIXME: DEPRECATED!
        }
        else {
            die("The number of call arguments for function $functionName is different from the number of arguments inside the function!\n");
        }

        $argumentNumber++;
    }

    $statements = $functionStatement['stmts'];

    // TODO: We are assuming that the statements of a function will always have a null-endingType or 'return'-endingType (not 'continue' or 'break')
    //       If the parser does not gaurd against this, we should.
    
     flowifyStatements($statements, $functionCallFlowElement);
     $openEndingsFunctionCall = $functionCallFlowElement->openEndings;
    
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
            //$functionCallFlowElement->varsInScope[$variableName] = $conditionalJoinVariableFlowElement;   // FIXME: DEPRECATED!
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

    $openEndings = $bodyFlowElement->openEndings;

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
            
            // FIXME: is this correct? Is the flowElement always the element that represents the expression?
            // FIXME: this generates errors, since the flowElement can be an assignedVar (and have no changedVars)
            // FIXME: addChangedVariablesToExitingParent($flowElement);

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
            
            $ifAstNodeIdentifier = getAstNodeIdentifier($statement);
            $ifFlowElement = createAndAddFlowElementToParent('ifMain', 'if', null, $ifAstNodeIdentifier, $bodyFlowElement);
            $ifFlowElement->canContainSplitters = true;
            
            flowifyIfStatement($statement, $ifFlowElement);
            
            addChangedVariablesToExitingParent($ifFlowElement);
            
            // FIXME: should we not combine this with the already existing openEndings in bodyFlowElement?
            $bodyFlowElement->onlyHasOpenEndings = $ifFlowElement->onlyHasOpenEndings;
            
            $openEndings = combineOpenEndings($ifFlowElement->openEndings, $openEndings);
            
            if ($bodyFlowElement->onlyHasOpenEndings) {
                // If the if-statement only had openEndings we stop looping statements here, since they have become unreachable.
                break;
            }
        }
        else if($statementType === 'Stmt_For') {
            
            $forAstNodeIdentifier = getAstNodeIdentifier($statement);
            // FIXME: change this from a ifMain for a forMain
            $forFlowElement = createAndAddFlowElementToParent('ifMain', 'for', null, $forAstNodeIdentifier, $bodyFlowElement);

            $forOpenEndings = flowifyForStatement($statement, $forFlowElement);
            
            addChangedVariablesToExitingParent($forFlowElement);
            
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

function flowifyIfStatement($ifStatement, $ifFlowElement) {

    
    $ifAstNodeIdentifier = $ifFlowElement->astNodeIdentifier;
    
    // == COND ==
    
    $conditionExpression = $ifStatement['cond'];
    
    $condAstNodeIdentifier = $ifAstNodeIdentifier . "_IfCond";
    $condFlowElement = createAndAddFlowElementToParent('ifCond', 'cond', null, $condAstNodeIdentifier, $ifFlowElement);
    
    $flowElement = flowifyExpression($conditionExpression, $condFlowElement);
    
    addChangedVariablesToExitingParent($condFlowElement);
    
    $condFlowElement->canSplit = true;
    
    // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
    //       whether the then-statements or the else(if)-statements are executed. How to should
    //       we visualize this?
    
    // == THEN ==
    
    $thenStatements = $ifStatement['stmts'];
    
    $thenAstNodeIdentifier = getAstNodeIdentifier($thenStatements);
    $thenBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'then', null, $thenAstNodeIdentifier, $ifFlowElement);
    $thenBodyFlowElement->previousId = $condFlowElement->id;
    
    flowifyStatements($thenStatements, $thenBodyFlowElement);
    
    $thenBodyFlowElement->addPassthroughIfVariableNotChanged = true;
    
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
        $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement);
        $elseBodyFlowElement->previousId = $condFlowElement->id;
        
        flowifyStatements($elseStatements, $elseBodyFlowElement);
    }
    else {
        // Add an elseBody if it doesn't exist yet
        // TODO: what if an implicit else is never needed?
        $elseAstNodeIdentifier = $ifAstNodeIdentifier . "_ImplicitElse";
        // FIXME: this should be of type: 'ifElseImplicit'
        $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement);
        $elseBodyFlowElement->previousId = $condFlowElement->id;
    }
    $elseBodyFlowElement->addPassthroughIfVariableNotChanged = true;
    
    // == ELSEIF ==
    
    // TODO: $elseIfStatements = $ifStatement['elseif']
    
    
    // == ENDIF ==
    
    $endAstNodeIdentifier = $ifAstNodeIdentifier . "_IfEnd";
    // FIXME: change this to ifEnd
    $endFlowElement = createAndAddFlowElementToParent('ifCond', 'end', null, $endAstNodeIdentifier, $ifFlowElement);
    $endFlowElement->canJoin = true;
    
    // == Dealing with openEndings ==
    {
        
        if (!$thenBodyFlowElement->onlyHasOpenEndings) {
            // FIXME: should the exiting parent be the EndIf or the If? Right now its the If (by default)
            addChangedVariablesToExitingParent($thenBodyFlowElement);
            $endFlowElement->previousIds[] = $thenBodyFlowElement->id;
        }
        $ifFlowElement->openEndings = combineOpenEndings($thenBodyFlowElement->openEndings, $ifFlowElement->openEndings);
        
        
        if (!$elseBodyFlowElement->onlyHasOpenEndings) {
            // FIXME: should the exiting parent be the EndIf or the If? Right now its the If (by default)
            addChangedVariablesToExitingParent($elseBodyFlowElement);
            $endFlowElement->previousIds[] = $elseBodyFlowElement->id;
        }
        $ifFlowElement->openEndings = combineOpenEndings($elseBodyFlowElement->openEndings, $ifFlowElement->openEndings);
        
        if ($thenBodyFlowElement->onlyHasOpenEndings && $elseBodyFlowElement->onlyHasOpenEndings) {
            // Both the thenBody and the elseBody have only openEndings. This means the ifBody and its parent also only have openEndings 
            $ifFlowElement->onlyHasOpenEndings = true;
        }
    }
    
    return $ifFlowElement->openEndings;
}


function flowifyForStatement($forStatement, $forFlowElement) {
    
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
    
    $forAstNodeIdentifier = $forFlowElement->astNodeIdentifier;
    
    // == INIT ==
    
    $initAstNodeIdentifier = $forAstNodeIdentifier . "_ForInit";
    // FIXME: replace ifCond with forInit
    $initFlowElement = createAndAddFlowElementToParent('ifCond', 'init', null, $initAstNodeIdentifier, $forFlowElement);
    $flowElement = flowifyExpression($initExpression, $initFlowElement);
    
    // FIXME: check openEndings of initFlowElement!

    addChangedVariablesToExitingParent($initFlowElement);
        
 
    // == STEP 1 ==
    
    $forStepAstNodeIdentifier = $forAstNodeIdentifier . "_1";
    // FIXME: change this from a ifThen for a forStep
    $forStepFlowElement = createAndAddFlowElementToParent('ifThen', '#1', null, $forStepAstNodeIdentifier, $forFlowElement);
    
    {
        
        // == COND ==
        
        // FIXME: replace ifCond with forCond
        $condBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'cond', null, $forAstNodeIdentifier . "_ForCond", $forStepFlowElement);
        $flowElement = flowifyExpression($conditionExpression, $condBodyFlowElement);
        $condBodyFlowElement->previousIds[] = $initFlowElement->id; // Note: later we add the 'back-element' to this list
        $condBodyFlowElement->canJoin = true;
        $condBodyFlowElement->canSplit = true;
        
        // FIXME: check openEndings of condBodyFlowElement!
        
        addChangedVariablesToExitingParent($condBodyFlowElement);

        // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
        //       whether the iter-statements are executed. How to should we visualize this?
        
        // == ITER ==

        $iterAstNodeIdentifier = getAstNodeIdentifier($iterStatements);
        
        // FIXME: replace ifThen with iterBody
        $iterBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'iter', null, $iterAstNodeIdentifier, $forStepFlowElement);
        flowifyStatements($iterStatements, $iterBodyFlowElement);
        $iterBodyFlowElement->previousId = $condBodyFlowElement->id;
        
        $iterOpenEndings = $iterBodyFlowElement->openEndings;

        addChangedVariablesToExitingParent($iterBodyFlowElement);
        
        
        // == UPDATE ==
        
        // FIXME: replace ifCond with forUpdate
        $updateBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'update', null, $forAstNodeIdentifier . "_ForUpdate", $forStepFlowElement);
        // FIXME: if we have multiple continues, we need to combine them first?
        {
            if (count($iterOpenEndings->continues) > 0) {
                foreach ($iterOpenEndings->continues as $elementId => $continueOpenEndingElement) {
                    $continueOpenEndingElement->exitingParentId = $forStepFlowElement->id; // FIXME: is this correct?
                    addChangedVariablesToExitingParent($continueOpenEndingElement);
                    
                    $updateBodyFlowElement->previousIds[] = $continueOpenEndingElement->id;
                    
                    logLine("Found continue-openEnding in iter: " . $continueOpenEndingElement->id);
                }
                
                
                // FIXME: only if there are non-openEndings we should add the iterBody itself as the previous
                $updateBodyFlowElement->previousIds[] = $iterBodyFlowElement->id;
                
                $updateBodyFlowElement->canJoin = true;
            }
            else {
            
                // FIXME: only if there are non-openEndings we should add the iterBody itself as the previous
                $updateBodyFlowElement->previousId = $iterBodyFlowElement->id;
            }
        }
        $flowElement = flowifyExpression($updateExpression, $updateBodyFlowElement);
        
        // FIXME: check openEndings of condBodyFlowElement!
        
        addChangedVariablesToExitingParent($updateBodyFlowElement);
        
        
    }
    
    // == BACK ==
    
    $backAstNodeIdentifier = $forAstNodeIdentifier . "_ImplicitBack";
    // FIXME: this should be of type: 'forBackImplicit'
    $backBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'back', null, $backAstNodeIdentifier, $forFlowElement);
    $backBodyFlowElement->addPassbackIfVariableNotChanged = true;
    $backBodyFlowElement->previousId = $updateBodyFlowElement->id;
    
    $condBodyFlowElement->previousIds[] = $backBodyFlowElement->id; // Note: earlier the init-element was put in this list
    
    // All variables that were changed in the update/iter-bodies have to be connected back to the condBody, through the back lane
    // FIXME: we should only do this if the variable has been changed somewhere in the iter/update body AND wasnt an openEnd!
    $varsChangedInIterOrUpdateBodies = array_merge($iterBodyFlowElement->varsInScopeChanged, $updateBodyFlowElement->varsInScopeChanged);
    foreach ($varsChangedInIterOrUpdateBodies as $variableName => $isChanged) {
        if (array_key_exists($variableName, $condBodyFlowElement->varsInScopeAvailable)) {
            // We only join with changed variables (inside the iter or update bodies) if they are (split and) joined 
            // inside the condBody. This excludes variables created locally inside the iter/update bodies.

            // We first need to find the join-element in the condBody for this variable
            $joinVariableToConnectTo = findJoinVariableInsideLane($condBodyFlowElement, $variableName);
            
            if ($joinVariableToConnectTo !== null) {
                // We take the connectionType from the output connection(s) of the join-variable
                $connectionType = $joinVariableToConnectTo->outputConnectionType;
                // We build a path from the iter/update variable
                $variableElement = buildPathBackwards($backBodyFlowElement, $variableName, $connectionType);
                // We connect it to the join element inside the condBody
                addFlowConnection($variableElement, $joinVariableToConnectTo, $connectionType);
            }
            else {
                // TODO:  we are now *not* passing back variables that have been *visibly* declared
                //        inside the iterBody, while they are *invisibly* declared (via varsInScopeAvailable)
                //        inside the function. 
                //        If we were to do this, they would *not* be completely connected back
                //        to the condBody, since no join element is present the that it can connect with.
                // logLine("ERROR: We could not find the join variable for variable " . $variableName, $isError = true);
            }
        }
    }
    
    // == DONE ==
    
    $doneAstNodeIdentifier = $forAstNodeIdentifier . "_ImplicitDone";
    // FIXME: this should be of type: 'forDoneImplicit'
    $doneBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'done', null, $doneAstNodeIdentifier, $forFlowElement);
    $doneBodyFlowElement->addPassthroughIfVariableNotChanged = true;

    // FIXME: we should somehow 'consume' the openEndings we *joined* (for example the continue-openEndings in the update)
    //        and keep on going with the left-over openEndings and maybe assemble more when we go on.
    //        For now we just look at the openEndings inside the iter!

    // FIXME: if we have multiple breaks, we need to combine them first?
    {
        if (count($iterOpenEndings->breaks) > 0) {
            foreach ($iterOpenEndings->breaks as $elementId => $breakOpenEndingElement) {
                $breakOpenEndingElement->exitingParentId = $forStepFlowElement->id; // FIXME: is this correct?
                addChangedVariablesToExitingParent($breakOpenEndingElement);
                
                $doneBodyFlowElement->previousIds[] = $breakOpenEndingElement->id;
                
                logLine("Found break-openEnding in iter: " . $breakOpenEndingElement->id);
            }
            
            
            // FIXME: only if there are non-openEndings we should add the condBody itself as the previous (or should we always do this?)
            $doneBodyFlowElement->previousIds[] = $condBodyFlowElement->id;
            
            $doneBodyFlowElement->canJoin = true;
        }
        else {
        
            // FIXME: only if there are non-openEndings we should add the condBody itself as the previous (or should we always do this?)
            $doneBodyFlowElement->previousId = $condBodyFlowElement->id;
        }
    }
    
    // FIXME: is this correct?
    addChangedVariablesToExitingParent($doneBodyFlowElement);

    // OLD: $doneBodyFlowElement->previousId = $condBodyFlowElement->id;
    
    addChangedVariablesToExitingParent($forStepFlowElement);
    
}

// Building a path backwards

function buildPathBackwardsFromPrevious ($laneElement, $variableName, $connectionType = null) {
    
    // We try to find a previous lane / element. If there is no previous lane, we should go to 
    // the parent lane and try again, until we find a previous to build from.
    
    global $flowElements;

    $variableElement = null;
    
    if ($laneElement->previousId !== null) {
        logLine("We go to previousId: " . $laneElement->previousId);
        $previousLaneElement = $flowElements[$laneElement->previousId];
        $variableElement = buildPathBackwards($previousLaneElement, $variableName, $connectionType);

        if ($variableElement !== null && $laneElement->addPassthroughIfVariableNotChanged) {
            $passThroughVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
            $passThroughVariable = createVariable($laneElement, $variableName, $passThroughVariableAstNodeIdentifier, 'passThroughVariable');

            addFlowConnection($variableElement, $passThroughVariable, $connectionType);
            
            $variableElement = $passThroughVariable;
        }
        
        if ($variableElement !== null && $laneElement->addPassbackIfVariableNotChanged) {
            $passBackVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_*PASSBACK*_" . $variableName;
            $passBackVariable = createVariable($laneElement, $variableName, $passBackVariableAstNodeIdentifier, 'passBackVariable');

            addFlowConnection($variableElement, $passBackVariable, $connectionType);
            
            $variableElement = $passBackVariable;
        }
        
    }
    else {
        if ($laneElement->parentId !== null) {
            logLine("We go to parentId: " . $laneElement->parentId);
            $parentLaneElement = getParentElement($laneElement);
            
            $variableElement = buildPathBackwardsFromPrevious($parentLaneElement, $variableName, $connectionType);
        }
        else {
            logLine("ERROR: We are trying to go to the parent of element: " . $laneElement->id . " but it has no parent!", $isError = true);
        }
    }
    
    return $variableElement;
}    

function buildPathBackwards($laneElement, $variableName, $connectionType = null) {
    
    // TODO: optionally, we could add $startWithChild = null here, so we won't have to start from the lastChild of the parent.
    
    global $flowElements;
    
    $variableElement = null;
    
    if (!$laneElement->canHaveChildren) {
        logLine("We are at element: " . $laneElement->id);
        
        // The labeElement is not a lane, so we check if it is the variable we are searching for
        if ($laneElement->isVariable === $variableName) {
            logLine("Found variable $variableName as element:" . $laneElement->id);
            $variableElement = $laneElement;
        }
    }
    else {
        logLine("We are at lane: " . $laneElement->id);
        
        if (array_key_exists($variableName, $laneElement->varsInScopeAvailable)) {
        
            if (array_key_exists($variableName, $laneElement->varsInScopeChanged)) {
                // The variable has been changed inside the lane, so we should be able to find it there
                
                // We start with the last child in the lane
                if ($laneElement->lastChildId !== null) {
                    $variableElement = buildPathBackwards($flowElements[$laneElement->lastChildId], $variableName, $connectionType);
                }
                else {
                    // Somehow the variable has been changed inside the lane, but the are no childs in the lane. This should never happen.
                    logLine("ERROR: somehow the variable has been changed inside the lane, but the are no childs in the lane", $isError = true);
                    return null;
                }
                 
            }
            // FIXME: WORKAROUND using openEndings!!
            else if (containsSomeOpenEndings($laneElement->openEndings) ) {  
                // We start with the last child in the lane
                if ($laneElement->lastChildId !== null) {
                    $variableElement = buildPathBackwards($flowElements[$laneElement->lastChildId], $variableName, $connectionType);
                }
                else {
                    // We've entered a lane with openEndings, but there are no childs in the lane. 
                    logLine("NOTE: we've entered a lane (" . $laneElement->id . ") with openEndings, but there are no childs in the lane");
                    // return null;
                }
            }
            else if ($laneElement->canJoin) {
                logLine("We can join");
                
                // FIXME: We should traverse into all joined lanes here, BUT only if either has changed OR if we have an assymetric join!
                // loop through: $laneElement->previousIds
                if (count($laneElement->previousIds) === 2) {
                    $leftLaneId = $laneElement->previousIds[0];
                    $rightLaneId = $laneElement->previousIds[1];
                    $leftLane = $flowElements[$leftLaneId];
                    $rightLane = $flowElements[$rightLaneId];
                    logLine("We found the left lane:" . $leftLaneId);
                    logLine("We found the right lane:" . $rightLaneId);
                    $variableElementLeft = buildPathBackwards($leftLane, $variableName, $connectionType);
                    $variableElementRight = buildPathBackwards($rightLane, $variableName, $connectionType);
                    
                    // FIXME: we should also update the varsInScopeChanged on the way back!
                    $conditionalJoinVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_JOINED_" . $variableName;
                    $conditionalJoinVariableFlowElement = createVariable($laneElement, $variableName, $conditionalJoinVariableAstNodeIdentifier, 'conditionalJoinVariable');

                    addFlowConnection($variableElementLeft, $conditionalJoinVariableFlowElement, $connectionType);
                    addFlowConnection($variableElementRight, $conditionalJoinVariableFlowElement, $connectionType);
                    
                    $variableElement = $conditionalJoinVariableFlowElement;
                }
                // TODO: this can happen if (for example) for-loops, where the back-lane has not been connected yet as previous
                else if (count($laneElement->previousIds) === 1) {
                    $leftLaneId = $laneElement->previousIds[0];
                    $leftLane = $flowElements[$leftLaneId];
                    logLine("We found the left lane:" . $leftLaneId);
                    $variableElementLeft = buildPathBackwards($leftLane, $variableName, $connectionType);
                    
                    // FIXME: we should also update the varsInScopeChanged on the way back!
                    $conditionalJoinVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_JOINED_" . $variableName;
                    $conditionalJoinVariableFlowElement = createVariable($laneElement, $variableName, $conditionalJoinVariableAstNodeIdentifier, 'conditionalJoinVariable');

                    addFlowConnection($variableElementLeft, $conditionalJoinVariableFlowElement, $connectionType);
                    
                    $variableElement = $conditionalJoinVariableFlowElement;
                }
                else {
                    // FIXME: support other variants!
                }
                
                if ($laneElement->canSplit) {
                    logLine("We can also split: " . $laneElement->id);
                    
                    $conditionalSplitVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_SPLIT_" . $variableName;
                    $conditionalSplitVariableFlowElement = createVariable($laneElement, $variableName, $conditionalSplitVariableAstNodeIdentifier, 'conditionalSplitVariable');
                    
                    addFlowConnection($variableElement, $conditionalSplitVariableFlowElement, $connectionType);
                    
                    $variableElement = $conditionalSplitVariableFlowElement;
                }
                
                // FIXME: If we don't traverse the (joined) lanes, we should probably skip towards (or past) the split of the lanes!?
                
            }
            else if ($laneElement->canSplit) {
                logLine("We can split");
                
                // FIXME: the variable could have been changed on the lane that can split, if so, we should connect to it
                
                // We are adding a splitter in this laneElement, but are proceeding to find the actual
                // variable in the (any) previous element.
                $variableElement = buildPathBackwardsFromPrevious($laneElement, $variableName, $connectionType);

                $conditionalSplitVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_SPLIT_" . $variableName;
                $conditionalSplitVariableFlowElement = createVariable($laneElement, $variableName, $conditionalSplitVariableAstNodeIdentifier, 'conditionalSplitVariable');
                
                addFlowConnection($variableElement, $conditionalSplitVariableFlowElement, $connectionType);
                
                $variableElement = $conditionalSplitVariableFlowElement;
            }
        }
        else {
            // The variableName is not available, so don't have to look into it, we should look somewhere else (see below)
        }
    }
        
    if ($variableElement === null) {
        // The variable has been *not* changed inside the lane and is not the current element, 
        // so we should try it in the previous (or parent-previous etc) element.
        
        logLine("variableElement is null, starting buildPathBackwardsFromPrevious from laneElement: " . $laneElement->id);
        $variableElement = buildPathBackwardsFromPrevious($laneElement, $variableName, $connectionType);
        logLine("variableElement after buildPathBackwardsFromPrevious: " . $variableElement->id . " in laneElement: " . $laneElement->id);
        
        /*
        we now do this inside buildPathBackwardsFromPrevious
        if ($variableElement !== null && $laneElement->addPassthroughIfVariableNotChanged) {
            $passThroughVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
            $passThroughVariable = createVariable($laneElement, $variableName, $passThroughVariableAstNodeIdentifier, 'passThroughVariable');

            addFlowConnection($variableElement, $passThroughVariable, $connectionType);
            
            $variableElement = $passThroughVariable;
        }
        */
        
 
    }
    
    return $variableElement;
}


// OLD code:

function joinVariables($variableName, $differentVariables, $targetElement) {
    
    $conditionalJoinVariableAstNodeIdentifier = $targetElement->astNodeIdentifier . "_JOINED_" . $variableName;
    $conditionalJoinVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalJoinVariable', $variableName, null, $conditionalJoinVariableAstNodeIdentifier, $targetElement);
    foreach ($differentVariables as $differentVariableElement) {
        $flowConntectionId = addFlowConnection($differentVariableElement, $conditionalJoinVariableFlowElement); // OLD: , 'conditional'
    }
    return $conditionalJoinVariableFlowElement;
}

