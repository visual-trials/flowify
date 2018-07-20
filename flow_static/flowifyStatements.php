<?php

function flowifyProgram($statements) {
    
    $astNodeIdentifier = getAstNodeIdentifier(null);

    $rootFlowElement = createFlowElement('root', 'root', null, $astNodeIdentifier);

    // TODO: should we do anything with the return value of the main function?
    $resultingElements = flowifyStatements($statements, $rootFlowElement);
    
    // FIXME: we should do something with the resultingElements!
    
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

        }
        else {
            die("The number of call arguments for function $functionName is different from the number of arguments inside the function!\n");
        }

        $argumentNumber++;
    }

    $statements = $functionStatement['stmts'];

    // TODO: We are assuming that the statements of a function will always have a 'none'-resultType or 'return'-resultType (not 'continue' or 'break')
    //       If the parser does not gaurd against this, we should.
    
    $resultingElements = flowifyStatements($statements, $functionCallFlowElement);
    
    // If there are multiple results, we join them
    if (count($resultingElements) > 1) {
        $differentVariables = [];
        // TODO: how do we really know that all return variables are actually different? Of they are the same,
        //       should we add a passthrough by the ones who are the same?
        foreach ($resultingElements as $resultingElement) {
            array_push($differentVariables, $resultingElement->returnVar);
        }
        
        // Note: we use '*' to make sure there is no collision with existing variableNames (since they are not allowed in variable-identifiers)
        $conditionalJoinVariableFlowElement = joinVariables('*RETURN*', $differentVariables, $functionCallFlowElement);
        $returnFlowElement = $conditionalJoinVariableFlowElement;
        
        // TODO: should we add the conditionalJoinVariable/returnFlowElement to our scope?
        //$functionCallFlowElement->varsInScope[$variableName] = $conditionalJoinVariableFlowElement; 
    }
    else {
        $resultingElement = reset($resultingElements);
        $returnFlowElement = $resultingElement->returnVar;
    }
    
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

    $resultingElements = [];

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
            //       if an expression is a statement, where does the output
            //       of that expesssion/statement go?

        }
        else if ($statementType === 'Stmt_Return') {

            $expression = $statement['expr'];

            $returnFlowElement = flowifyExpression($expression, $bodyFlowElement);

            // Note: we are assuming that when we reach a 'return' statement,
            //       all statements that follow will be unreachable. That's why
            //       we stop looping through all the left-over statements and
            //       simply return the returnFlowElement
            
            $bodyFlowElement->endsWith = 'return';
            $bodyFlowElement->returnVar = $returnFlowElement;
            // TODO: we should use 'id' as identifier here! but that isnt possible right now because its numeric
            //       and php won't treat it as proper keys!
            $resultingElements['id:' . $bodyFlowElement->id] = $bodyFlowElement;
            break;
        }
        else if($statementType === 'Stmt_If') {
            
            $ifResultingElements = flowifyIfStatement($statement, $bodyFlowElement);
            
            // TODO: we have to check whether the flowifyIfStatement resulted
            //       in endings other than 'none'. If all of them are not 'none'
            //       we should stop looping statement and return the result of
            //       flowifyIfStatement. If more than one of them is 'none' we
            //       should join the vars in them (BUT this should already have been
            //       done in flowifyIfStatement).
            
            foreach ($ifResultingElements as $ifResultingElement) {
                if ($ifResultingElement->endsWith !== 'none') {
                    $resultingElements['id:' . $ifResultingElement->id] = $ifResultingElement;
                }
            }
        }
        else if($statementType === 'Stmt_For') {
            
            $forResultingElements = flowifyForStatement($statement, $bodyFlowElement);
            
            /*
            foreach ($forResultingElements as $forResultingElement) {
                if ($forResultingElement->endsWith !== 'none') {
                    $resultingElements['id:' . $forResultingElement->id] = $forResultingElement;
                }
            }
            */
        }
        else {
            echo "statementType '".$statementType."' found in function body, but not supported!\n";
            echo print_r($statement, true);
            continue;
        }

    }

    if (count($resultingElements) == 0) {
        // If no 'return', 'break' or 'continue' was encountered, the endsWith will be 'none' 
        $bodyFlowElement->endsWith = 'none';
        $resultingElements['id:' . $bodyFlowElement->id] = $bodyFlowElement;
    }
    
    return $resultingElements;

}

function flowifyIfStatement($ifStatement, $parentFlowElement) {
    
    $resultingElements = [];

    $ifAstNodeIdentifier = getAstNodeIdentifier($ifStatement);
    $ifFlowElement = createAndAddFlowElementToParent('ifMain', 'if', null, $ifAstNodeIdentifier, $parentFlowElement);
    
    {
        // == COND ==
        
        $conditionExpression = $ifStatement['cond'];
        
        $condAstNodeIdentifier = $ifAstNodeIdentifier . "_IfCond";
        $condFlowElement = createAndAddFlowElementToParent('ifCond', 'cond', null, $condAstNodeIdentifier, $ifFlowElement);
        
        $flowElement = flowifyExpression($conditionExpression, $condFlowElement);
        
        // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
        //       whether the then-statements or the else(if)-statements are executed. How to should
        //       we visualize this?
        
        $varsInScopeAfterCondBody = $condFlowElement->varsInScope; // copy!
        
        // == THEN ==
        
        $thenStatements = $ifStatement['stmts'];
//        $thenBodyHasReturn = false;
        
        $thenAstNodeIdentifier = getAstNodeIdentifier($thenStatements);
        $thenBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'then', null, $thenAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
        
        // Note: we *copy* the varsInScope here. This is because the thenBody might replace vars in it's scope,
        //       These are however conditional-replacement when it comes to the if-statement. 
        //       Instead of the thenBody letting the vars in the if-scope to be replaced, we *add* it later to our varsInScope,
        //       by using a conditionalFlowElement.
        
        $thenBodyFlowElement->varsInScope = $ifFlowElement->varsInScope;  // copy!
        $thenBodyFlowElement->functionsInScope = &$ifFlowElement->functionsInScope;
        
        $thenResultingElements = flowifyStatements($thenStatements, $thenBodyFlowElement);
        
        $resultingElements = array_merge($resultingElements, $thenResultingElements);

        
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
            $elseResultingElements = flowifyStatements($elseStatements, $elseBodyFlowElement);
            
            $resultingElements = array_merge($resultingElements, $elseResultingElements);
        }
        else {
            
            // Add an elseBody if it doesn't exist yet
            // FIXME: what if an implicit else is never needed?
            $elseAstNodeIdentifier = $ifAstNodeIdentifier . "_ImplicitElse";
            // FIXME: this should be of type: 'ifElseImplicit'
            $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
        }
        
        // FIXME: we should look at $thenResultingElements and $elseResultingElements and JOIN and SPLIT where needed/possible!
        //        that is: all 'none'-results should be joined. All others should stay in the $resultingElements
        
        
        // Adding a passthrough variable if either side has changed a variable, while the other has not
        addPassThroughsBasedOnChange($thenBodyFlowElement, $elseBodyFlowElement, $varsInScopeAfterCondBody);
        
        // Joining variables between then and else, if they are different
        joinVariablesBasedOnDifference($thenBodyFlowElement->varsInScope, $elseBodyFlowElement->varsInScope, $ifFlowElement);

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
       
    return $resultingElements;
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
        $forFlowElement->varsInScope = $forStepFlowElement1->varsInScope; // copy back!
        
        /*
        // STEP 2
        
        $forStepAstNodeIdentifier2 = $forAstNodeIdentifier . "_2";
        // FIXME: change this from a ifThen for a forStep
        $forStepFlowElement2 = createAndAddFlowElementToParent('ifThen', '#2', null, $forStepAstNodeIdentifier2, $forFlowElement, $useVarScopeFromParent = false);
        $forStepFlowElement2->varsInScope = $forFlowElement->varsInScope; // copy!
        $forStepFlowElement2->functionsInScope = &$forFlowElement->functionsInScope;
        
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
        
        */

        // TODO: implement continue statement (inside flowifyStatements)
        // TODO: implement break statement (inside flowifyStatements)
    

    }    
    
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
    
    $varsInScopeBeforeCondBody = $forStepFlowElement->varsInScope; // copy!
    
    // FIXME: replace ifCond with forCond
    $condBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'cond', null, $forAstNodeIdentifier . "_ForCond", $forStepFlowElement);
    $flowElement = flowifyExpression($conditionExpression, $condBodyFlowElement);

    $varsInScopeAfterCondBody = $forStepFlowElement->varsInScope; //copy!


    // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
    //       whether the iter-statements are executed. How to should we visualize this?
    
    // == ITER ==

    $iterAstNodeIdentifier = getAstNodeIdentifier($iterStatements);
    
    // FIXME: replace ifThen with iterBody
    $iterBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'iter', null, $iterAstNodeIdentifier, $forStepFlowElement);
    $resultingElements = flowifyStatements($iterStatements, $iterBodyFlowElement);
    // FIXME: do something with $resultingElements!

    // == UPDATE ==
    
    // FIXME: replace ifCond with forUpdate
    $updateBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'update', null, $forAstNodeIdentifier . "_ForUpdate", $forStepFlowElement);
    $flowElement = flowifyExpression($updateExpression, $updateBodyFlowElement);
    

    
    // IMPORTANT NOTE: right now we are assuming that the condBody doesn't reassign
    //                 the variable! If it does, then the code below will not give the proper result!
    
    
    // Adding a passthrough variable if the iter/update side has changed a variable: the done-side then needs a passthrough
    addPassThroughsBasedOnChange($doneBodyFlowElement, $forStepFlowElement, $varsInScopeAfterCondBody);
    
    
    // Joining variables between afterCondBody and afterForStep, if they are different
    // Note: we ARE updating varsInScope of the forStepFlowElement here (addToVarsInScope = true). We do this, so we will have 
    //       the conditionalJoinVariables in the scope of the forStepFlowElement. We can use that
    //       to connect to the conditionalSplitVariables after this!
    joinVariablesBasedOnDifference($varsInScopeAfterCondBody, $forStepFlowElement->varsInScope, $forStepFlowElement, $backBodyFlowElement, $addToVarsInScope = true, $updateExistingConnections = true);
    
    
    // FIXME: we now copy the varsInScope of the forStepFlowElement towards the varsInScope of the condBodyFlowElement
    //        but we don't want to lose the changes the condBodyFlowElement did itself to the scope. So we have to rerun it
    //        of do something smarter than that.
    // SOLUTION: we should probably create a $condBodyFlowElement_0 and $condBodyFlowElement_1 which indicates
    //           it which *iteration* these $condBodyFlowElements are.
    // OR POSSIBLY BETTER: create varsInScopeAfterCondBodyAfterStep vs varsInScopeAfterCondBodyBeforeStep
    $condBodyFlowElement->varsInScope = $forStepFlowElement->varsInScope; // copy!
    
    
    
    
    // FIXME: we should give it $condBodyFlowElement itself (not it's varsInScope)
    // FIXME: we also want to give it the updateBody! (besides the updateElement!)
    // FIXME: should we really give it the forElement as the last argument? Shouldnt it use the varsInScope after the cond?
    //        note: giving it the condBodyFlowElement as the last variable doesnt really work, since
    //              variables that are declared inside the for loop will also be present in that scope
    //              and things go wrong if you allow that to be passed as last argument (see the 'c' variable
    //              in fibonacci_iterative)
    // FIXME: by giving it the forFlowElement as the last argument, all split variables are now added to the forFlowElement
    //        this is not what we want, since they either should be added to the condBodyFlowElement or forStepFlowElement
    
    splitVariablesBasedOnUsage($condBodyFlowElement->varsInScope, $doneBodyFlowElement, $iterBodyFlowElement, $updateBodyFlowElement, $forFlowElement);

    
    // FIXME: we should take the doneBody and copy its varsInScope to the varsInScope of the for(Step)FlowElement!
    $forStepFlowElement->varsInScope = $doneBodyFlowElement->varsInScope; // copy!
        
    
}



function joinVariables($variableName, $differentVariables, $targetElement) {
    
    $conditionalJoinVariableAstNodeIdentifier = $targetElement->astNodeIdentifier . "_JOINED_" . $variableName;
    $conditionalJoinVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalJoinVariable', $variableName, null, $conditionalJoinVariableAstNodeIdentifier, $targetElement);
    foreach ($differentVariables as $differentVariableElement) {
        $flowConntectionId = addFlowConnection($differentVariableElement, $conditionalJoinVariableFlowElement, 'conditional');
    }
    return $conditionalJoinVariableFlowElement;
}

function joinVariablesBasedOnDifference ($firstVarsInScope, $secondVarsInScope, $targetElement, $passBackBodyFlowElement = null, $addToVarsInScope = true, $updateExistingConnections = false) {
    
    // FIXME: we can most likely simplyfy the code below, since we only have two scopes (not an arbitrary amount anymore)
    //        we can probably get rid of 'doPassBack'
    //        whether we want to keep doPassBack depends on how we implement 'continue' in for loops
    
    $differentVariablesPerVariableName = [];
    foreach ($firstVarsInScope as $variableName => $variableFlowElement) {
        if (!array_key_exists($variableName, $differentVariablesPerVariableName)) {
            $differentVariablesPerVariableName[$variableName] = [];
        }
        // TODO: workaround for PHP not allowing numbers as keys
        $differentVariablesPerVariableName[$variableName]['id:' . $variableFlowElement->id] = $variableFlowElement;
    }
    foreach ($secondVarsInScope as $variableName => $variableFlowElement) {
        if (!array_key_exists($variableName, $differentVariablesPerVariableName)) {
            $differentVariablesPerVariableName[$variableName] = [];
        }
        // We assume that the secondVarsInScope should be passed-back
        $variableFlowElement->doPassBack = true;
        // TODO: workaround for PHP not allowing numbers as keys
        $differentVariablesPerVariableName[$variableName]['id:' . $variableFlowElement->id] = $variableFlowElement;
    }

    foreach ($differentVariablesPerVariableName as $variableName => $differentVariables) {
        // Only if we found more than 1 different variable should we join them
        if (count($differentVariables) > 1) {
            
            $differentVariablesToBeJoined = [];
            if ($passBackBodyFlowElement !== null) {
                // We should do pass-back the variables when their doPassBack is set to true
                foreach (array_values($differentVariables) as $differentVariable) {
                    if ($differentVariable->doPassBack) {
                        // We need to pass-back this variable. We first create a pass-back flowElement,
                        // add it to $passBackBodyFlowElement and connect it with $differentVariable
                        // we then add the pass-back flowElement to $differentVariablesToBeJoined
                        
                        // FIXME: it is conceivable that the SAME variable(name) is pass-backed for more than ONE flowElementBodiesWithDifferentScope
                        //        we should have a pass-back body for each flowElementBodiesWithDifferentScope, or we need find another
                        //        way to make sure this passBackVariableAstNodeIdentifier is unique
                        $passBackVariableAstNodeIdentifier = $targetElement->astNodeIdentifier . "_PASSBACK_" . $variableName;
                        $passBackVariableFlowElement = createAndAddChildlessFlowElementToParent('passBackVariable', $variableName, null, $passBackVariableAstNodeIdentifier, $passBackBodyFlowElement);
                        addFlowConnection($differentVariable, $passBackVariableFlowElement, 'conditional');
                        
                        // TODO: is it possible that $differentVariable already had connections *from* itself to others? If so, we need to deal with those connections!
                        
                        array_push($differentVariablesToBeJoined, $passBackVariableFlowElement);
                    }
                    else {
                        // No need to pass-back this variable, so it can be added to differentVariablesToBeJoined
                        array_push($differentVariablesToBeJoined, $differentVariable);
                    }
                }
                
            }
            else {
                // There is no passBackBodyFlowElement, so no need to check which variables have to be
                // pass-backed. We simply set differentVariablesToBeJoined to add all 
                // original ones (without pass-back in between).
                $differentVariablesToBeJoined = array_values($differentVariables);
            }
            
            $conditionalJoinVariableFlowElement = joinVariables($variableName, $differentVariablesToBeJoined, $targetElement);
            
            if ($addToVarsInScope) {
                $targetElement->varsInScope[$variableName] = $conditionalJoinVariableFlowElement;
            }
            if ($updateExistingConnections) {
                foreach ($differentVariablesToBeJoined as $differentVariable) {
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
            $passThroughVariableAstNodeIdentifier = $elseAstNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
            $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $elseBodyFlowElement);

            // Connecting the variable in the parent to the passthrough variable (inside the thenBody)
            addFlowConnection($varsInScopeBeforeChange[$variableName], $passThroughVariableFlowElement);

            // We add this passthrough variable to the scope of the elseBody
            $elseBodyFlowElement->varsInScope[$variableName] = $passThroughVariableFlowElement;
            
        }
        else if ($varReplacedInElseBody) {
            // Only the elseBody has replaced the variable. We use the parent's variable as the (default) then variable
            
            // Adding the variable to the thenBody as a passthrough variable
            $passThroughVariableAstNodeIdentifier = $thenAstNodeIdentifier . "_*PASSTHROUGH*_" . $variableName;
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
        
    foreach ($parentFlowElement->varsInScope as $variableName => $parentVarInScopeElement) {

        // We also want to create a conditional *split* element between the condBody and thenBody, and between the condBody and elseBody
        // We need to know the connections going from the condBody into the thenBody and elseBody (for this variable)
        // We do this by looping all the connections from the condBody for this variable (in its 'connectionIdsFromThisElement')
        $variableAfterCondBody = $varsInScopeAfterCondBody[$variableName];
        $conditionalSplitVariableFlowElement = null;
        $connectionIdToConditionalSplitVariable = null;
        $connectionTypeToConditionalSplitVariable = null;
        $updatedConnectionIdsFromThisElement = [];
        
        // TODO: could this be moved outside the foreach of the $parentFlowElement->varsInScope? Or are we adding elements to the thenBody and elseBody in this loop?
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
