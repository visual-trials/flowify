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
            $functionCallFlowElement->varsInScope[$parameterName] = $parameterFlowElement;   // FIXME: DEPRECATED!
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
    
    
    $ifOpenEndings = $ifFlowElement->openEndings;
    $ifAstNodeIdentifier = $ifFlowElement->astNodeIdentifier;
    
    {
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
        $thenOpenEndings = $thenBodyFlowElement->openEndings;
        
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
            $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement);
            $elseBodyFlowElement->previousId = $condFlowElement->id;
            
            flowifyStatements($elseStatements, $elseBodyFlowElement);
            $elseOpenEndings = $elseBodyFlowElement->openEndings;
            
            $ifOpenEndings = combineOpenEndings($elseOpenEndings, $ifOpenEndings);
            
            if ($elseBodyFlowElement->onlyHasOpenEndings) {
                // FIXME: what to do here?
            }
        }
        else {
            
            // Add an elseBody if it doesn't exist yet
            // TODO: what if an implicit else is never needed?
            $elseAstNodeIdentifier = $ifAstNodeIdentifier . "_ImplicitElse";
            // FIXME: this should be of type: 'ifElseImplicit'
            $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement);
            $elseBodyFlowElement->previousId = $condFlowElement->id;
        }
        
        if ($thenBodyFlowElement->onlyHasOpenEndings && $elseBodyFlowElement->onlyHasOpenEndings) {
            // Both the thenBody and the elseBody have only openEndings. This means the ifBody and its parent also only have openEndings 
            $ifFlowElement->onlyHasOpenEndings = true;
            // FIXME: should we break here?
        }
        
        
        // FIXME: if either the thenBody- or the elseBody onlyHasOpenEndings, we should NOT JOIN here!
        
        // FIXME: should the exiting parent be the EndIf or the If?
        addChangedVariablesToExitingParent($thenBodyFlowElement);
        addChangedVariablesToExitingParent($elseBodyFlowElement);
        
        
        // == ELSEIF ==
        
        // TODO: $elseIfStatements = $ifStatement['elseif']

        
        // == ENDIF ==
        
        $endAstNodeIdentifier = $ifAstNodeIdentifier . "_IfEnd";
        // FIXME: change this to ifEnd
        $endFlowElement = createAndAddFlowElementToParent('ifCond', 'end', null, $endAstNodeIdentifier, $ifFlowElement);
        $endFlowElement->previousIds[] = $thenBodyFlowElement->id;
        $endFlowElement->previousIds[] = $elseBodyFlowElement->id;
        $endFlowElement->canJoin = true;
    }
    
    return $ifOpenEndings;
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
        
        addChangedVariablesToExitingParent($condBodyFlowElement);

        $condBodyFlowElement->previousIds[] = $initFlowElement->id; // Note: later we add the 'back-element' to this list
        $condBodyFlowElement->canJoin = true;
        $condBodyFlowElement->canSplit = true;
        
        // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
        //       whether the iter-statements are executed. How to should we visualize this?
        
        // == ITER ==

        $iterAstNodeIdentifier = getAstNodeIdentifier($iterStatements);
        
        // FIXME: replace ifThen with iterBody
        $iterBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'iter', null, $iterAstNodeIdentifier, $forStepFlowElement);
        flowifyStatements($iterStatements, $iterBodyFlowElement);
        $iterOpenEndings = $iterBodyFlowElement->openEndings;
        // FIXME: do something with $iterOpenEndings!

        addChangedVariablesToExitingParent($iterBodyFlowElement);
        
        $iterBodyFlowElement->previousId = $condBodyFlowElement->id;
        
        
        // FIXME: If iterOpenEndings contains a 'continue', we need to join the varsInScope of the continue-body (for example a then-body)
        //        with the varsInScope of the iterBodyFlowElement. We need to do this before the update.
        //        We also need to add a passthrough for that and split it.

        // FIXME: if we have multiple continues, we need to combine them first?
        
        // for the ONE 'continue' in iterOpenEndings do the following
        {
            if (count($iterOpenEndings->continues) > 0) {
                // FIXME: implement this!
            }
        }
        
        // == UPDATE ==
        
        // FIXME: replace ifCond with forUpdate
        $updateBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'update', null, $forAstNodeIdentifier . "_ForUpdate", $forStepFlowElement);
        $flowElement = flowifyExpression($updateExpression, $updateBodyFlowElement);
        
        addChangedVariablesToExitingParent($updateBodyFlowElement);
        
        $updateBodyFlowElement->previousId = $iterBodyFlowElement->id;
        
    }
    
    // == BACK ==
    
    $backAstNodeIdentifier = $forAstNodeIdentifier . "_ImplicitBack";
    // FIXME: this should be of type: 'forBackImplicit'
    $backBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'back', null, $backAstNodeIdentifier, $forFlowElement);
    
    $backBodyFlowElement->previousId = $updateBodyFlowElement->id;
    
    $condBodyFlowElement->previousIds[] = $backBodyFlowElement->id; // Note: earlier the init-element was put in this list
    
    
    // == DONE ==
    
    $doneAstNodeIdentifier = $forAstNodeIdentifier . "_ImplicitDone";
    // FIXME: this should be of type: 'forDoneImplicit'
    $doneBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'done', null, $doneAstNodeIdentifier, $forFlowElement);
    
    addChangedVariablesToExitingParent($forStepFlowElement);

    $doneBodyFlowElement->previousId = $condBodyFlowElement->id;
    
    // TODO: implement continue statement (inside flowifyStatements)
    // TODO: implement break statement (inside flowifyStatements)
    
}


function joinVariables($variableName, $differentVariables, $targetElement) {
    
    $conditionalJoinVariableAstNodeIdentifier = $targetElement->astNodeIdentifier . "_JOINED_" . $variableName;
    $conditionalJoinVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalJoinVariable', $variableName, null, $conditionalJoinVariableAstNodeIdentifier, $targetElement);
    foreach ($differentVariables as $differentVariableElement) {
        $flowConntectionId = addFlowConnection($differentVariableElement, $conditionalJoinVariableFlowElement, 'conditional');
    }
    return $conditionalJoinVariableFlowElement;
}

function buildPathBackwardsFromPrevious ($laneElement, $variableName, $connectionType = null) {
    
    // We try to find a previous lane / element. If there is no previous lane, we should go to 
    // the parent lane and try again, until we find a previous to build from.
    
    global $flowElements;

    $variableElement = null;
    
    if ($laneElement->previousId !== null) {
        logLine("We go to previousId: " . $laneElement->previousId);
        $previousLaneElement = $flowElements[$laneElement->previousId];
        $variableElement = buildPathBackwards($previousLaneElement, $variableName, $connectionType);
    }
    else {
        if ($laneElement->parentId !== null) {
            logLine("We go to parentId: " . $laneElement->parentId);
            $parentLaneElement = getParentElement($laneElement);
            
            $variableElement = buildPathBackwardsFromPrevious($parentLaneElement, $variableName, $connectionType);
        }
        else {
            logLine("ERROR: We are trying to go to the parent of element: " . $laneElement->id . " but it has no parent!");
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
                    logLine("ERROR: somehow the variable has been changed inside the lane, but the are no childs in the lane");
                    return null;
                }
                 
            }
            else if ($laneElement->canJoin) {
                
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
                else {
                    // FIXME: support other variants!
                }
                
                if ($laneElement->canSplit) {
                    logLine("We can also split");
                    
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
        
        $variableElement = buildPathBackwardsFromPrevious($laneElement, $variableName, $connectionType);
    }
    
    return $variableElement;
    
}

function OLD_buildPathBackwardsToElementFromVariable($laneElement, $toElement, $fromVariable, $variableName, $connectionType = null) {
$laneElementId =     $laneElement->id;
$fromVariableId = $fromVariable->id;
$toElementId = $toElement->id;

    $flowElement = null;
    
    // FIXME: we sometimes need to upgrade existing connection from $connectionType = 'identity' to $connectionType = null. How should we do this?
    
    $originalToElementId = $toElement->id;
    
    $isAncestorOfFromVariable = isAncestorOf($laneElement, $fromVariable);
    if ($isAncestorOfFromVariable) {
        // Connect the fromVariable to the toElement
        // TODO: what connectionType should we use here?
        if ($toElement !== null) {
            addFlowConnection($fromVariable, $toElement, $connectionType);
        }
        
        // Setting the toElement to the fromVariable
        $flowElement = $fromVariable;
    }
    else {
        
        
        // TODO: we still need to check for open ends and deal with them...
        if (count($laneElement->openEndings->continues) > 0) {
            
            // FIXME: how exactly should we proceed here?
            $openEndInsideLane = reset($laneElement->openEndings->continues);

            //echo print_r([ '$laneElement->id' => $laneElement->id], true);
            //echo print_r([ '$fromVariable->id' => $fromVariable->id], true);
            
            // FIXME: the BIGGEST problem is that we need to know which open-ends to 'go back through' and which ones not. Also in which order!
            
            // FIXME: we somehow need to get the 'other-side' of this open end
            // FIXME: HACK now assuming its an then or else, finding it via the parent and its children...
            $searchingForType = $openEndInsideLane->name === 'else' ? 'then' : 'else';
            $parentOfOpenEnd = getParentElement($openEndInsideLane);
            if ($parentOfOpenEnd->name === 'if') { // FIXME: we want the DIRECT parent, containing the openEnds
                foreach ($parentOfOpenEnd->children as $childOfIfElement) {
                    if ($childOfIfElement->name === $searchingForType && !$childOfIfElement->alreadyAdded) {
                        $childOfIfElement->alreadyAdded = true; // FIXME: HACK!!!
                        //echo print_r([ '$childOfIfElement' => $childOfIfElement], true);
    //            echo print_r([ '$laneElement->openEndings->continues' => $laneElement->openEndings->continues], true);

                        // FIXME: wouldn't this result in an infinite loop? since we will constantly go back to this laneElements that has an openEnd, and the go back to its child again...
                        // FIXME: HACKS!
                        // FIXME: HACKS!
                        // FIXME: HACKS!
                        /*
                        $laneElement->openEndings->continues = [];
                        getParentElement($laneElement)->openEndings->continues = [];
                        foreach ($laneElement->children as $laneChild) {
                            $laneChild->openEndings->continues = [];
                        }
                        */
                        // FIXME: overriding laneElement so that the following code will proceed with it as if it was the lane
                        $laneElement = $childOfIfElement;
                        
                    }
                }
            }
            
        }
        
        
        $parentOfLaneElement = getParentElement($laneElement);
        
        if ($parentOfLaneElement !== null) {
   
            if ($parentOfLaneElement->canContainSplitters && $laneElement->canContainPassthroughs) {
                
                {
                    // Adding a passthrough
                    
                    $passThroughVariableAstNodeIdentifier = $laneElement->astNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
                    $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $laneElement);

                    
                    // Connecting the passthrough variable to the toElement to the
                    if ($toElement !== null) {
                        addFlowConnection($passThroughVariableFlowElement, $toElement, $connectionType);
                    }
                    
                    // Setting the toElement to the passThroughVariableFlowElement
                    $toElement = $passThroughVariableFlowElement;
                    if ($flowElement === null) {
                        $flowElement = $passThroughVariableFlowElement;
                    }

                    // We add this passthrough variable to the scope of the laneElement
                    $laneElement->varsInScope[$variableName] = $passThroughVariableFlowElement;
                }

                
                // TODO: should we do this here? Or after calling the parent? we could/should also do this in the next recursion!
                {
                    // Getting (or if not available creating) a splitter
                    
                    $conditionalSplitVariableFlowElement = null;
                    if (array_key_exists($variableName, $parentOfLaneElement->varSplitters)) {
                        $conditionalSplitVariableFlowElement = $parentOfLaneElement->varSplitters[$variableName];
                        
                        // FIXME: we are ready, right? Even if we should have connected to the fromVariable, we are now connected to
                        //        an *existing* element, which presumably is itself connected to the fromVariable (maybe through several in-between elements)
                        // We now do a simple return, but this should be done more nicely (readable).
                        
                        // TODO: what connectionType should we use here?
                        if ($toElement !== null) {
                            addFlowConnection($conditionalSplitVariableFlowElement, $toElement, $connectionType);
                        }
                        
                        // Setting the toElement to the conditionalSplitVariableFlowElement
                        $toElement = $conditionalSplitVariableFlowElement;
                        if ($flowElement === null) {
                            $flowElement = $conditionalSplitVariableFlowElement;
                        }
                        
                        return $flowElement; // FIXME: is this correct?
                    }
                    else {
                        // FIXME: use "*SPLIT*" and put it BEFORE the variable!
                        // FIXME: is this AST Identifier correct?
                        $conditionalSplitVariableAstNodeIdentifier = $parentOfLaneElement->astNodeIdentifier . "_" . $variableName . "_SPLIT";
                        // FIXME: should this be put into the ifBody or the condBody?
                        $conditionalSplitVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalSplitVariable', $variableName, null, $conditionalSplitVariableAstNodeIdentifier, $parentOfLaneElement);
                        $parentOfLaneElement->varSplitters[$variableName] = $conditionalSplitVariableFlowElement;
                        // TODO: add conditionalSplitVariableFlowElement to varsInScope of parentOfLaneElement? of laneElement? and maybe of the sibling lane? (then -> else, else -> then)
                        
                        // TODO: what connectionType should we use here?
                        if ($toElement !== null) {
                            addFlowConnection($conditionalSplitVariableFlowElement, $toElement, $connectionType);
                        }
                        
                        // Setting the toElement to the conditionalSplitVariableFlowElement
                        $toElement = $conditionalSplitVariableFlowElement;
                        if ($flowElement === null) {
                            $flowElement = $conditionalSplitVariableFlowElement;
                        }
                    }
                    
                }
                
            }
            else {
                // there is no need to split, we simply continue to the next parent
            }
            
            $flowElement2 = buildPathBackwardsToElementFromVariable($parentOfLaneElement, $toElement, $fromVariable, $variableName);
            if ($flowElement === null) {
                $flowElement = $flowElement2;
            }
            
        }
        else {
            // We somehow reached the root node and apparently it is not an ancestor of the toElement (this should not be possible
            die("toElement is somehow not a child of the root element!");
        }
    }
    
// echo print_r([ '$laneElement->id' => $laneElementId, '$newlaneElement->id' => $laneElement->id, '$fromVariable->id' => $fromVariableId, '$toElement->id' => $toElement->id, 'oldtoElement->id' => $toElementId], true);
    return $flowElement;
    
}

/*
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
                foreach (array_values($differentVariablesWithLanesToBeJoined) as $differentVariableWithLane) {
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
*/

/*
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
*/

/*
function splitVariablesBasedOnUsage($varsInScopeAfterCondBody, $thenBodyFlowElement, $elseBodyFlowElement, $secondElseBodyFlowElement, $parentFlowElement) {

    // FIXME: workaround with 'second' else body
    
    // FIXME: don't call it then and else, call it left/right or true/false
        
    $varsInScopeAfterSplitting = [];
    foreach ($varsInScopeAfterCondBody as $variableName => $variableAfterCondBody) {
        // FIXME: workaround, so will have a varsInScope after splitting
        $varsInScopeAfterSplitting[$variableName] = $variableAfterCondBody;

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
                    $parentFlowElement->varSplitters[$variableName] = $conditionalSplitVariableFlowElement;
                    
                    // FIXME: workaround so the varsInScope of the then/else is updated if the var doesn't exist yet!
                    $varsInScopeAfterSplitting[$variableName] = $conditionalSplitVariableFlowElement;
                    
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
    
    return $varsInScopeAfterSplitting;
}
*/
