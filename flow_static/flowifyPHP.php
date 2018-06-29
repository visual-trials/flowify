<?php declare(strict_types=1);
// #!/usr/bin/env php

require "coupleVisualInfoToAst.php";

$flowElementId = 0;
$flowConnections = [];
$code = null;  // TODO: we should make a file-based struct (or array) that contains the $code

handleRequest();

function handleRequest() {
    $getVars = $_GET;

    $fileToFlowify = 'examples/simpleMath.php';
    // FIXME: this is a security risk! We should not allow certain characters here!
    if (array_key_exists('fileToFlowify', $getVars)) {
        $fileToFlowify = $getVars['fileToFlowify'];
    }
    else {
        exit("No fileToFlowify given.");
    }

    $fileToFlowifyWithoutExtention = substr($fileToFlowify, 0, strrpos($fileToFlowify, "."));

    if (array_key_exists('visualInfos', $_POST)) {
        // FIXME: check if postVars contains the right info (not secure!)
        $changedVisualInfos = json_decode($_POST['visualInfos'], true);  // FIXME: what if not valid json?
        updateVisualInfos($changedVisualInfos, $fileToFlowifyWithoutExtention);
    }

    $flowifiedPhp = flowifyPhpAndAttachVisualInfo($fileToFlowifyWithoutExtention);

    header('Content-Type: application/json');
    echo json_encode($flowifiedPhp, JSON_PRETTY_PRINT);
}

function flowifyPhpAndAttachVisualInfo($fileToFlowifyWithoutExtention)
{
    global $flowConnections, $code;

    list($code, $visualInfos) = updateAndGetCodeAndVisualInfoForFile($fileToFlowifyWithoutExtention);
    
    $statements = getAstFromPhpCode($code);

    $rootFlowElement = flowifyProgram($statements);
    
    stripScope($rootFlowElement);
    
    $usedVisualInfos = [];
    extendFlowElementsWithVisualInfo($rootFlowElement, $visualInfos, $usedVisualInfos);

    // TODO: If we use at least part of the nr of visualInfos we had, we store it (think of something better!)
    if (count($visualInfos) > 0 && count($usedVisualInfos) / count($visualInfos) > 0.5) {
        // FIXME: check if this goes right!
        storeVisualInfos($usedVisualInfos, $fileToFlowifyWithoutExtention);
    }

    $flowifiedPhp = [];
    $flowifiedPhp['code'] = explode("\n", $code);;
    $flowifiedPhp['rootFlowElement'] = $rootFlowElement;
    $flowifiedPhp['flowConnections'] = $flowConnections;
    $flowifiedPhp['statements'] = $statements;
    $flowifiedPhp['visualInfos'] = $visualInfos;
    $flowifiedPhp['usedVisualInfos'] = $usedVisualInfos;

    // $flowifiedPhp['tokens'] = $tokens;
    // $flowifiedPhp['tokensWithPosInfo'] = $tokensWithPosInfo;

    // $flowifiedPhp['diffResult'] = $diffResult;
    // $flowifiedPhp['oldToNewPositions'] = $oldToNewPositions;

    //$flowifiedPhp['lengthCode'] = strlen($newCode);

    return $flowifiedPhp;
}

function flowifyProgram($statements) {
    
    // $varsInScope = [];
    // $functionsInScope = [];

    $astNodeIdentifier = getAstNodeIdentifier(null);

    $rootFlowElement = createFlowElement('root', 'root', null, $astNodeIdentifier);

    // TODO: should we do anything with the return value of the main function?
    $noReturnFlowElement = flowifyStatements($statements, $rootFlowElement);
    
    return $rootFlowElement;
}
    
function flowifyStatements ($statements, &$bodyFlowElement) {

    $varsInScope = &$bodyFlowElement['varsInScope'];
    $functionsInScope = &$bodyFlowElement['functionsInScope'];
    
    $returnFlowElement = null;

    // $localFunctions = [];

    // 1)    First find all defined functions, so we known the nodes of them, when they are called
    foreach ($statements as $statement) {

        $statementType = $statement['nodeType'];

        if ($statementType === 'Stmt_Function') {
            $identifierNode = $statement['name']; // TODO: we are assuming 'name' is always present

            if ($identifierNode['nodeType'] === 'Identifier') {
                // $localFunctions[$identifierNode['name']] = $statement;
                $functionsInScope[$identifierNode['name']] = $statement;
            }
            else {
                echo "Found '" . $identifierNode['nodeType'] . "' as nodeType inside 'name' of 'Stmt_Function'\n";
            }
        }
    }

    // 2) Then loop through all non-fuctions and do as if they are "executed"...
    foreach ($statements as $statement) {

        $statementType = $statement['nodeType'];

        // We already did 'Stmt_Function' above
        if ($statementType === 'Stmt_Function') {
            continue;
        }

        if ($statementType === 'Stmt_Expression' || $statementType === 'Stmt_Return') {

            $expression = $statement['expr'];

            $flowElement = flowifyExpression($expression, $bodyFlowElement);

            if ($statementType === 'Stmt_Return') {
                $returnFlowElement = $flowElement;
            }
        }
        else if($statementType === 'Stmt_If') {
            
            flowifyIfStatement($statement, $bodyFlowElement);
            
        }
        else if($statementType === 'Stmt_For') {
            
            flowifyForStatement($statement, $bodyFlowElement);
            
        }
        else {
            echo "statementType '".$statementType."' found in function body, but not supported!\n";
            echo print_r($statement, true);
            continue;
        }

    }

    return $returnFlowElement;
}

function flowifyExpression ($expression, &$parentFlowElement) {  

    $varsInScope = &$parentFlowElement['varsInScope'];
    $functionsInScope = &$parentFlowElement['functionsInScope'];
    
    $expressionType = $expression['nodeType'];

    $flowElement = null;

    $astNodeIdentifier = getAstNodeIdentifier($expression);

    if ($expressionType === 'Expr_Variable') {

        $name = $expression['name'];
        if (array_key_exists($name, $varsInScope)) {
            // TODO: in the beginning of flowifyExpression you should check if it's a variable. 
            //       If it already exists, you should return a ref to the flow object representing that variable (AT THAT POINT, because it could be overwritten)
            
            // Note: this could be a conditionalVariableFlowElement (containing multiple flowElements)
            $flowElement = &$varsInScope[$name];
        }
        else {
            $flowElement = createAndAddFlowElementToParent('variable', $name, null, $astNodeIdentifier, $parentFlowElement);
        }
    }
    else if ($expressionType === 'Scalar_LNumber') {
        $flowElement = createAndAddFlowElementToParent('constant', null, $expression['value'], $astNodeIdentifier, $parentFlowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Plus') {
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '+', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Minus') {
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '-', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Mul') {
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '*', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Div') {  // TODO: unduplcate this from other BinaryOps!
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '/', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Greater') {  // TODO: unduplcate this from other BinaryOps!
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '>', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Smaller') {  // TODO: unduplcate this from other BinaryOps!
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '<', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_Assign') {

        // TODO: if you get an assigment of a variable, you should overwrite (or better: stack?) the variable in the vars-array with a new Flow-object!

        $variable = $expression['var'];

        if ($variable['nodeType'] !== 'Expr_Variable') {
            die("Found" . $variable['nodeType'] . " as nodeType inside 'var' of 'Expr_Assign'\n");
        }

        $variableName = $variable['name'];

        // TODO: the expression (that is assigned) might be constant, which can be used to directly fill the variable (so no connection is needed towards to variable)
        //       but you COULD make the constant its own FlowElements and make the assigment iself a 'function' that combines the empty variable
        //       with the constant, resulting in a variable (header) with a value (body)

        $assignExpression = $expression['expr'];

        $flowAssign = flowifyExpression($assignExpression, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('variable', $variableName, null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($flowAssign, $flowElement);
        
        // TODO: add a 'identity'-connection between the newly assigned variable and the variable it overwrote (or multiple if there is more than one) 

        $varsInScope[$variableName] = $flowElement;
    }
    else if ($expressionType === 'Expr_FuncCall') {

        $functionName = null;
        $nameNode = $expression['name']; // TODO: we are assuming this always exists
        if ($nameNode['nodeType'] === 'Name') {
            if (count($nameNode['parts']) === 1) {
                $functionName = $nameNode['parts'][0];
            }
            else {
                die("Found more than one part (in nodeType 'Name') inside 'name' of 'Expr_FuncCall' (not supported yet)\n");
            }
        }
        else {
            die("Found" . $nameNode['nodeType'] . " as nodeType inside 'name' of 'Expr_FuncCall'\n");
        }


        $flowCallArguments = [];
        $callArguments = $expression['args'];
        foreach ($callArguments as $callArgument) {
            if ($callArgument['nodeType'] === 'Arg') {
                if (array_key_exists('value', $callArgument)) {
                    $callArgumentExpression = $callArgument['value'];
                    $flowCallArgument = flowifyExpression($callArgumentExpression, $parentFlowElement);
                    array_push($flowCallArguments, $flowCallArgument);
                }
                else {
                    die("There is no 'value' inside an argument node of function call '$functionName'\n");
                }
            }
            else {
                die("Found nodeType '" . $callArgument['nodeType'] . "' inside 'args' of 'Expr_FuncCall'\n");
            }
        }

        $knownPrimitiveFunctions = [
            'dechex' => true,
        ];

        // We try to find the statement of the function in scope
        if (array_key_exists($functionName, $functionsInScope)) {
            // call to known function (that we have the body-AST of

            $functionStatement = $functionsInScope[$functionName];

            $functionCallFlowElement = createFlowElement('function', $functionName, null, $astNodeIdentifier);

            // Flowify the body of the functions (including the return value) and use that  return value as our own output
            // FIXME: what if function has no return value? Should we return a null-flowElement?
            $localVars = [];  // FIXME: what should we pass as localVars to the called function?
            $flowElement = flowifyFunction($functionStatement, /*$localVars, $functionsInScope,*/ $flowCallArguments, $functionCallFlowElement);

            addFlowElementToParent($functionCallFlowElement, $parentFlowElement);  // Note: do not call this before flowifyFunction, because this COPIES $flowFunctionCallElement, so changes to it will not be in the parent!
        }
        else if (array_key_exists($functionName, $knownPrimitiveFunctions)) {
            // Primitive function call (or at least a function we don't have the body from)

            $primitiveFunctionCallFlowElement = createAndAddFlowElementToParent('primitiveFunction', $functionName, null, $astNodeIdentifier, $parentFlowElement);
            foreach ($flowCallArguments as $flowCallArgument) {
                addFlowConnection($flowCallArgument, $primitiveFunctionCallFlowElement);
            }
            $flowElement = $primitiveFunctionCallFlowElement;
        }
        else {
            // FIXME: we might want to allow non existing function for the time being (and mark them as non-existing)
            die("The function $functionName could not be found!\n");
        }

    }
    else {
        echo "expressionType '" . $expression['nodeType'] ."', but not supported!\n";
    }

    // FIXME: if no elements found/created, do this?            $flowElement['name'] = '<unknown>';

    return $flowElement;
}

function flowifyFunction ($functionStatement, $flowCallArguments, &$functionCallFlowElement) { 

    $functionCallFlowElement['varsInScope'] = [];
    $functionCallFlowElement['functionsInScope'] = [];
    $varsInScope = &$functionCallFlowElement['varsInScope'];
    $functionsInScope = &$functionCallFlowElement['functionsInScope'];
    
    $functionName = $functionCallFlowElement['name'];
    
    // NO BODY $astNodeIdentifier = getAstNodeIdentifier($functionStatement); // FIXME: shouldnt this be attached to the 'stmts' inside the $functionStatement? Also see 'stmts' in the if-statement (similar problem)
    // NO BODY $functionBodyFlowElement = createFlowElement('body', null, null, $astNodeIdentifier);

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

            // FIXME: we should put all input-parameters inside an 'input' container

            // Adding the parameter to the function
            // NO BODY $parameterFlowElement = createAndAddFlowElementToParent('variable', $parameterName, null, $astNodeIdentifier, $functionBodyFlowElement);
            $parameterFlowElement = createAndAddFlowElementToParent('variable', $parameterName, null, $astNodeIdentifier, $functionCallFlowElement);

            // Connecting the callArgument (outside the function) to the parameter (inside the function)
            addFlowConnection($flowCallArgument, $parameterFlowElement);

            // Setting the parameter as a local var within the function body
            $varsInScope[$parameterName] = $parameterFlowElement;

        }
        else {
            die("The number of call arguments for function $functionName is different from the number of arguments inside the function!\n");
        }

        $argumentNumber++;
    }

    $statements = $functionStatement['stmts'];

    // WITH BODY: maybe make this work: 
    //    - the parameters should be in the function-input
    //    - the statements should be in the function-body
    //    - the return variable(s) should be in the function-output
    // $returnFlowElement = flowifyStatements($statements, $functionBodyFlowElement);

    // OR WITHOUT BODY:
    //    - Everything should be inside the function-call (no input/body/output)
    // TODO: don't we need the astNodeIdentifier of the functionStatement for some visualInfo?
    $returnFlowElement = flowifyStatements($statements, $functionCallFlowElement);

    // addFlowElementToParent($functionBodyFlowElement, $functionCallFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $functionBodyFlowElement, so changes to it will not be in the parent!

    return $returnFlowElement;

}

function flowifyForStatement($forStatement, &$parentFlowElement) {
    
    $varsInScope = &$parentFlowElement['varsInScope'];
    $functionsInScope = &$parentFlowElement['functionsInScope'];
    
    $forAstNodeIdentifier = getAstNodeIdentifier($forStatement);
    // FIXME: change this from a ifMain for a forMain
    $forFlowElement = createFlowElement('ifMain', 'for', null, $forAstNodeIdentifier);

    {
        // FIXME: implement forCondition (3 expressions)
        
        {
            // == INIT ==
            
            // FIXME: hardcoded to 1 statement/expression!
            $initExpression = $forStatement['init'][0];
            
            // Because the position in the code of $initExpression always corresponds to the forInit,
            // we create a separate astNodeIdentifier for the forInit by postFixing the identifier of the 
            // for-statement with "_ForInit". 
            $initAstNodeIdentifier = $forAstNodeIdentifier . "_ForInit";
            // FIXME: replace ifCond with forInit
            $initFlowElement = createFlowElement('ifCond', 'init', null, $initAstNodeIdentifier);
            
            // FIXME: we should do this when creating the FlowElement (getting these from the parent, or better: referring to the parent from within the child)
            $initFlowElement['varsInScope'] = &$varsInScope;
            $initFlowElement['functionsInScope'] = &$functionsInScope;
            $flowElement = flowifyExpression($initExpression, $initFlowElement);
            // TODO: the flowElement coming from the initExpression is ignored for now. Is that ok?
            
            addFlowElementToParent($initFlowElement, $forFlowElement);  // Note: do not call this before flowifyExpression, because this COPIES $condFlowElement, so changes to it will not be in the parent!
            
            
            // == COND ==
            
            // FIXME: hardcoded to 1 statement/expression!
            $conditionExpression = $forStatement['cond'][0];
            
            // Because the position in the code of $conditionExpression always corresponds to the forCondition,
            // we create a separate astNodeIdentifier for the forCondition by postFixing the identifier of the 
            // for-statement with "_ForCond". 
            $condAstNodeIdentifier = $forAstNodeIdentifier . "_ForCond";
            // FIXME: replace ifCond with forCond
            $condFlowElement = createFlowElement('ifCond', 'cond', null, $condAstNodeIdentifier);
            
            // FIXME: we should do this when creating the FlowElement (getting these from the parent, or better: referring to the parent from within the child)
            $condFlowElement['varsInScope'] = &$varsInScope;
            $condFlowElement['functionsInScope'] = &$functionsInScope;
            $flowElement = flowifyExpression($conditionExpression, $condFlowElement);
            
            // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
            //       whether the iter-statements are executed. How to should we visualize this?
            
            addFlowElementToParent($condFlowElement, $forFlowElement);  // Note: do not call this before flowifyExpression, because this COPIES $condFlowElement, so changes to it will not be in the parent!
            

            
            // == UPDATE ==
            
            // FIXME: hardcoded to 1 statement/expression!
            $updateExpression = $forStatement['loop'][0];
            
            // Because the position in the code of $updateExpression always corresponds to the forUpdate,
            // we create a separate astNodeIdentifier for the forUpdate by postFixing the identifier of the 
            // for-statement with "_ForUpdate". 
            $updateAstNodeIdentifier = $forAstNodeIdentifier . "_ForUpdate";
            // FIXME: replace ifCond with forUpdate
            $updateFlowElement = createFlowElement('ifCond', 'update', null, $updateAstNodeIdentifier);
            
            // FIXME: we should do this when creating the FlowElement (getting these from the parent, or better: referring to the parent from within the child)
            $updateFlowElement['varsInScope'] = &$varsInScope;
            $updateFlowElement['functionsInScope'] = &$functionsInScope;
            $flowElement = flowifyExpression($updateExpression, $updateFlowElement);
            
            // TODO: the flowElement coming from the updateExpression is a boolean and determines 
            //       whether the iter-statements are executed. How to should we visualize this?
            
            addFlowElementToParent($updateFlowElement, $forFlowElement);  // Note: do not call this before flowifyExpression, because this COPIES $updateFlowElement, so changes to it will not be in the parent!
        }
        
        // FIXME: implement iterBody
        
        {
            $iterStatements = $forStatement['stmts'];
            
            $iterAstNodeIdentifier = getAstNodeIdentifier($iterStatements);
            // FIXME: replace ifThen with iterBody
            $iterBodyFlowElement = createFlowElement('ifThen', 'iter', null, $iterAstNodeIdentifier);
            
            // Note: we *copy* the varsInScope here. This is because the iterBody might replace vars in it's scope,
            //       These are however conditional-replacement when it comes to the for-statement. 
            //       Instead of the iterBody letting the vars in the for-scope to be replaced, we *add* it later to our varsInScope,
            //       by using a conditionalFlowElement.
            
            $iterBodyFlowElement['varsInScope'] = $varsInScope;  // copy!
            $iterBodyFlowElement['functionsInScope'] = &$functionsInScope;
            
            // TODO: we don't have a return statement in iter-bodies, so we call it $noReturnFlowElement here (but we shouldn't get it at all)
            $noReturnFlowElement = flowifyStatements($iterStatements, $iterBodyFlowElement);
            
            addFlowElementToParent($iterBodyFlowElement, $forFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $iterBodyFlowElement, so changes to it will not be in the parent!


            // TODO: add iterBody 3 times
            // TODO: add iterBody 3 times
            // TODO: add iterBody 3 times
            // TODO: what to do with conditionalVariableAstNodeIdentifier if there are 3 of them? Since they use the variableName for the Identifier
            //       maybe _1, _2, _3? (or is it a relative position, and are they actually the same (but a copy inside their parent?)
            

            $doneBodyFlowElement = null;
            $varsInScopeParent = &$parentFlowElement['varsInScope'];
            $varsInScopeIterBody = &$iterBodyFlowElement['varsInScope'];
            
            $implicitDoneBodyWasCreated = false;
            // We loop through all the varsInScope of the parentFlowElement
            foreach ($varsInScopeParent as $variableName => $parentVarInScopeElement) {
                
                $varReplacedInIterBody = false;
                
                // We check if we have the same variable in our iterBody scope
                if (array_key_exists($variableName, $varsInScopeIterBody)) {
                    // The var exists both in iterBodyFlowElement and in the parent's scope
                    if ($varsInScopeIterBody[$variableName]['id'] !== $varsInScopeParent[$variableName]['id']) {
                        // The vars differ, so it must have been replaced (or extended) inside the iterBody. 
                        $varReplacedInIterBody = true;
                    }
                }
                
                $iterVariableFlowElement = null;
                $doneVariableFlowElement = null;
                if ($varReplacedInIterBody) {
                    // The iterBody has replaced the variable. We use the parent's variable as the (default) done variable
                    
                    // Add an doneBody if it doesn't exist yet
                    if ($doneBodyFlowElement === null) {
                        
                        $doneAstNodeIdentifier = $forAstNodeIdentifier . "_ImplicitDone";
                        // FIXME: this should be of type: 'forDoneImplicit'
                        $doneBodyFlowElement = createFlowElement('ifElse', 'done', null, $doneAstNodeIdentifier);
                        $implicitDoneBodyWasCreated = true;
                    }
                    
                    {
                        // Adding the variable to the doneBody as a passthrough variable
                        
                        // TODO: should we really use the variable name in the identifier?
                        $passThroughVariableAstNodeIdentifier = $doneAstNodeIdentifier . "_" . $variableName;

                        // FIXME: this should be of type 'passthroughVariable'
                        $passThroughVariableFlowElement = createAndAddFlowElementToParent('variable', $variableName, null, $passThroughVariableAstNodeIdentifier, $doneBodyFlowElement);

                        // Connecting the variable in the parent to the passthrough variable (inside the iterBody)
                        addFlowConnection($varsInScopeParent[$variableName], $passThroughVariableFlowElement);

                        // TODO: do we need to add this passthrough variable to the scope of the DoneBody? 
                        // $varsInScopeDoneBody[$parameterName] = $passThroughVariableFlowElement;
                    }
                    
                    $iterVariableFlowElement = $varsInScopeIterBody[$variableName];  // copy!
                    // TODO: this is without passthrough variable: $doneVariableFlowElement = $varsInScopeParent[$variableName];  // copy!
                    $doneVariableFlowElement = $passThroughVariableFlowElement;  // copy!
                }
                
                
                
                // The variable was replaced in the iterBody.
                // This means we should create a conditionalVariableFlowElement and add it to the forFlowElement
                // and also connect this conditionalVariable with the iter- and done- variable.
                // In additional, it should be added to the varsInScope of the parent, so if the variable is used 
                // by another flowElement, it can connect to this conditionalVariable
                if ($varReplacedInIterBody) {
                    $conditionalVariableAstNodeIdentifier = $forAstNodeIdentifier . "_" . $variableName;
                    // FIXME: make this type 'conditionalVariable'
                    $conditionalVariableFlowElement = createAndAddFlowElementToParent('variable', $variableName, null, $conditionalVariableAstNodeIdentifier, $forFlowElement);
                    addFlowConnection($iterVariableFlowElement, $conditionalVariableFlowElement);
                    addFlowConnection($doneVariableFlowElement, $conditionalVariableFlowElement);
                    $varsInScopeParent[$variableName] = &$conditionalVariableFlowElement;
                }
                
            }
            if ($implicitDoneBodyWasCreated) {
                addFlowElementToParent($doneBodyFlowElement, $forFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $doneBodyFlowElement, so changes to it will not be in the parent!
            }
            
            
        }
        
        
        // TODO: implement continue statement (inside flowifyStatements)
        // TODO: implement break statement (inside flowifyStatements)
    

    }    
    
    addFlowElementToParent($forFlowElement, $parentFlowElement);  // Note: do not call this before the calls to the other addFlowElementToParent, because this COPIES $forFlowElement, so changes to it will not be in the parent!
    
}

function flowifyIfStatement($ifStatement, &$parentFlowElement) {
    
    $varsInScope = &$parentFlowElement['varsInScope'];
    $functionsInScope = &$parentFlowElement['functionsInScope'];
    
    $ifAstNodeIdentifier = getAstNodeIdentifier($ifStatement);
    $ifFlowElement = createFlowElement('ifMain', 'if', null, $ifAstNodeIdentifier);
    
    {
        // == COND ==
        
        $conditionExpression = $ifStatement['cond'];
        
        // Because the position in the code of $conditionExpression always corresponds to the ifCondition,
        // we create a separate astNodeIdentifier for the ifCondition by postFixing the identifier of the 
        // if-statement with "_IfCond". 
        $condAstNodeIdentifier = $ifAstNodeIdentifier . "_IfCond";
        $condFlowElement = createFlowElement('ifCond', 'cond', null, $condAstNodeIdentifier);
        
        // FIXME: we should do this when creating the FlowElement (getting these from the parent, or better: referring to the parent from within the child)
        $condFlowElement['varsInScope'] = &$varsInScope;
        $condFlowElement['functionsInScope'] = &$functionsInScope;
        $flowElement = flowifyExpression($conditionExpression, $condFlowElement);
        
        // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
        //       whether the then-statements or the else(if)-statements are executed. How to should
        //       we visualize this?
        
        addFlowElementToParent($condFlowElement, $ifFlowElement);  // Note: do not call this before flowifyExpression, because this COPIES $condFlowElement, so changes to it will not be in the parent!
        
        
        // == THEN ==
        
        $thenStatements = $ifStatement['stmts'];
        
        $thenAstNodeIdentifier = getAstNodeIdentifier($thenStatements);
        $thenBodyFlowElement = createFlowElement('ifThen', 'then', null, $thenAstNodeIdentifier);
        
        // Note: we *copy* the varsInScope here. This is because the thenBody might replace vars in it's scope,
        //       These are however conditional-replacement when it comes to the if-statement. 
        //       Instead of the thenBody letting the vars in the if-scope to be replaced, we *add* it later to our varsInScope,
        //       by using a conditionalFlowElement.
        
        $thenBodyFlowElement['varsInScope'] = $varsInScope;  // copy!
        $thenBodyFlowElement['functionsInScope'] = &$functionsInScope;
        
        // TODO: we don't have a return statement in then-bodies, so we call it $noReturnFlowElement here (but we shouldn't get it at all)
        $noReturnFlowElement = flowifyStatements($thenStatements, $thenBodyFlowElement);
        
        addFlowElementToParent($thenBodyFlowElement, $ifFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $thenBodyFlowElement, so changes to it will not be in the parent!
        
        
        // == ELSE ==
        
        $elseStatement = $ifStatement['else'];
        
        $elseBodyFlowElement = null;
        if ($elseStatement !== null) {
            
            if ($elseStatement['nodeType'] !== 'Stmt_Else') {
                die("Expected nodeType 'Stmt_Else' in else. Not found.");
            }
            
            // There is an else-statement, getting the body of statements in it
            $elseStatements = $elseStatement['stmts'];
            
            $elseAstNodeIdentifier = getAstNodeIdentifier($elseStatements);
            $elseBodyFlowElement = createFlowElement('ifElse', 'else', null, $elseAstNodeIdentifier);
            
            // Note: we *copy* the varsInScope here. This is because the elseBody might replace vars in it's scope,
            //       These are however conditional-replacement when it comes to the if-statement. 
            //       Instead of the elseBody letting the vars in the if-scope to be replaced, we *add* it later to our varsInScope,
            //       by using a conditionalFlowElement.
            
            $elseBodyFlowElement['varsInScope'] = $varsInScope;  // copy!
            $elseBodyFlowElement['functionsInScope'] = &$functionsInScope;
            
            // TODO: we don't have a return statement in then-bodies, so we call it $noReturnFlowElement here (but we shouldn't get it at all)
            $noReturnFlowElement = flowifyStatements($elseStatements, $elseBodyFlowElement);
            
            addFlowElementToParent($elseBodyFlowElement, $ifFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $elseBodyFlowElement, so changes to it will not be in the parent!
        }

        
        // Note: we are comparing the varsInScope from the parentFlowElement with the varsInScope of the then/elseBodyFlowElement. 
        //       We don't compare with the varsInScope of the ifFlowElement, because its only a wrapper-element, and doesn't contain varsInScope
        
        $varsInScopeParent = &$parentFlowElement['varsInScope'];
        $varsInScopeThenBody = &$thenBodyFlowElement['varsInScope'];
        $varsInScopeElseBody = [];
        if ($elseBodyFlowElement !== null) {
            $varsInScopeElseBody = &$elseBodyFlowElement['varsInScope'];
        }

        $implicitElseBodyWasCreated = false;
        // We loop through all the varsInScope of the parentFlowElement
        foreach ($varsInScopeParent as $variableName => $parentVarInScopeElement) {
            
            $varReplacedInThenBody = false;
            $varReplacedInElseBody = false;
            
            // We check if we have the same variable in our thenBody scope
            if (array_key_exists($variableName, $varsInScopeThenBody)) {
                // The var exists both in thenBodyFlowElement and in the parent's scope
                if ($varsInScopeThenBody[$variableName]['id'] !== $varsInScopeParent[$variableName]['id']) {
                    // The vars differ, so it must have been replaced (or extended) inside the thenBody. 
                    $varReplacedInThenBody = true;
                }
            }
            
            // We check if we have the same variable in our elseBody scope
            if (array_key_exists($variableName, $varsInScopeElseBody)) {
                // The var exists both in elseBodyFlowElement and in the parent's scope
                if ($varsInScopeElseBody[$variableName]['id'] !== $varsInScopeParent[$variableName]['id']) {
                    // The vars differ, so it must have been replaced (or extended) inside the elseBody. 
                    $varReplacedInElseBody = true;
                }
            }
            
            $thenVariableFlowElement = null;
            $elseVariableFlowElement = null;
            if ($varReplacedInThenBody && $varReplacedInElseBody) {
                // We overwrite the parent's varInScope and adding them both using a conditionalVariableFlowElement.
                $thenVariableFlowElement = $varsInScopeThenBody[$variableName];  // copy!
                $elseVariableFlowElement = $varsInScopeElseBody[$variableName];  // copy!
            }
            else if ($varReplacedInThenBody) {
                // Only the thenBody has replaced the variable. We use the parent's variable as the (default) else variable
                
                
                // Add an elseBody if it doesn't exist yet
                if ($elseBodyFlowElement === null) {
                    
                    $elseAstNodeIdentifier = $ifAstNodeIdentifier . "_ImplicitElse";
                    // FIXME: this should be of type: 'ifElseImplicit'
                    $elseBodyFlowElement = createFlowElement('ifElse', 'else', null, $elseAstNodeIdentifier);
                    $implicitElseBodyWasCreated = true;
                }
                
                {
                    // Adding the variable to the elseBody as a passthrough variable
                    
                    // TODO: should we really use the variable name in the identifier?
                    $passThroughVariableAstNodeIdentifier = $elseAstNodeIdentifier . "_" . $variableName;

                    // FIXME: this should be of type 'passthroughVariable'
                    $passThroughVariableFlowElement = createAndAddFlowElementToParent('variable', $variableName, null, $passThroughVariableAstNodeIdentifier, $elseBodyFlowElement);

                    // Connecting the variable in the parent to the passthrough variable (inside the thenBody)
                    addFlowConnection($varsInScopeParent[$variableName], $passThroughVariableFlowElement);

                    // TODO: do we need to add this passthrough variable to the scope of the ElseBody? 
                    // $varsInScopeElseBody[$parameterName] = $passThroughVariableFlowElement;
                }
                
                $thenVariableFlowElement = $varsInScopeThenBody[$variableName];  // copy!
                // TODO: this is without passthrough variable: $elseVariableFlowElement = $varsInScopeParent[$variableName];  // copy!
                $elseVariableFlowElement = $passThroughVariableFlowElement;  // copy!
            }
            else if ($varReplacedInElseBody) {
                // Only the elseBody has replaced the variable. We use the parent's variable as the (default) then variable
                
                {
                    // Adding the variable to the thenBody as a passthrough variable
                    
                    // TODO: should we really use the variable name in the identifier?
                    $passThroughVariableAstNodeIdentifier = $thenAstNodeIdentifier . "_" . $variableName;

                    // FIXME: this should be of type 'passthroughVariable'
                    $passThroughVariableFlowElement = createAndAddFlowElementToParent('variable', $variableName, null, $passThroughVariableAstNodeIdentifier, $thenBodyFlowElement);

                    // Connecting the variable in the parent to the passthrough variable (inside the thenBody)
                    addFlowConnection($varsInScopeParent[$variableName], $passThroughVariableFlowElement);

                    // TODO: do we need to add this passthrough variable to the scope of the ThenBody? 
                    // $varsInScopeThenBody[$parameterName] = $passThroughVariableFlowElement;
                }
                
                // TODO: this is without passthrough variable: $thenVariableFlowElement = $varsInScopeParent[$variableName];  // copy!
                $thenVariableFlowElement = $passThroughVariableFlowElement;  // copy!
                $elseVariableFlowElement = $varsInScopeElseBody[$variableName];  // copy!
            }
            else {
                // The variable wasn't replaced by either the thenBody or the elseBody, so nothing to do here
            }
            
            // The variable was replaced in either the thenBody or the elseBody.
            // This means we should create a conditionalVariableFlowElement and add it to the ifFlowElement
            // and also connect this conditionalVariable with the then- and else- variable.
            // In additional, it should be added to the varsInScope of the parent, so if the variable is used 
            // by another flowElement, it can connect to this conditionalVariable
            if ($varReplacedInThenBody || $varReplacedInElseBody) {
                $conditionalVariableAstNodeIdentifier = $ifAstNodeIdentifier . "_" . $variableName;
                // FIXME: make this type 'conditionalVariable'
                $conditionalVariableFlowElement = createAndAddFlowElementToParent('variable', $variableName, null, $conditionalVariableAstNodeIdentifier, $ifFlowElement);
                addFlowConnection($thenVariableFlowElement, $conditionalVariableFlowElement);
                addFlowConnection($elseVariableFlowElement, $conditionalVariableFlowElement);
                $varsInScopeParent[$variableName] = &$conditionalVariableFlowElement;
            }
            
        }
        if ($implicitElseBodyWasCreated) {
            addFlowElementToParent($elseBodyFlowElement, $ifFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $elseBodyFlowElement, so changes to it will not be in the parent!
        }
        
            
        foreach ($varsInScopeThenBody as $variableName => $thenBodyVarInScopeElement) {
            
            // We check if we have the same variable in our parent's scope
            if (!array_key_exists($variableName, $varsInScopeParent)) {
                
                // the variable exists in the thenBody scope, but not in the parent's scope, so it must have been declared in the thenBody
            
                // TODO: depending on the language, variables DECLARED in an then- or else- statement are or are not
                //       available in the scope outside the then- or else- statement.
                //       We assume here that the current language has BLOCK-scope, so any newly declared variables
                //       within the then- or else- statement, are NOT copied back into the if- scope!
                //       The problem: PHP does not have this kind of scoping behaviour, right? (should investigate)
            }

        }
        
        foreach ($varsInScopeElseBody as $variableName => $elseBodyVarInScopeElement) {
            
            // We check if we have the same variable in our parent's scope
            if (!array_key_exists($variableName, $varsInScopeParent)) {
                
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
    
    addFlowElementToParent($ifFlowElement, $parentFlowElement);  // Note: do not call this before the calls to the other addFlowElementToParent, because this COPIES $ifFlowElement, so changes to it will not be in the parent!
    
}

// Helper functions

function addFlowConnection ($fromFlowElement, $toFlowElement) {
    global $flowConnections;

    array_push($flowConnections, [ 'from' => $fromFlowElement['id'], 'to' => $toFlowElement['id']]);
}

function addFlowElementToParent (&$flowElement, &$parentFlowElement) {
    array_push($parentFlowElement['children'], $flowElement);
}

function createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = true, $hasScope = true) {

    global $flowElementId;

    $flowElement = [];
    $flowElement['id'] = $flowElementId++;
    $flowElement['type'] = $flowElementType;
    $flowElement['name'] = $flowElementName;
    $flowElement['value'] = $flowElementValue;
    if ($canHaveChildren) {
        $flowElement['children'] = [];
    }
    $flowElement['astNodeIdentifier'] = $astNodeIdentifier;
    
    if ($hasScope) {
        $flowElement['varsInScope'] = [];
        $flowElement['functionsInScope'] = [];
    }

    return $flowElement;
}

function createAndAddFlowElementToParent ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, &$parentFlowElement, $canHaveChildren = false, $hasScope = false) {

    $flowElement = createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren, $hasScope);
    addFlowElementToParent($flowElement, $parentFlowElement);

    return $flowElement;
}

function stripScope (&$flowElement) {
    if (array_key_exists('varsInScope', $flowElement)) {
        unset($flowElement['varsInScope']);
    }
    if (array_key_exists('functionsInScope', $flowElement)) {
        unset($flowElement['functionsInScope']);
    }
    if (array_key_exists('children', $flowElement)) {
        foreach ($flowElement['children'] as &$childFlowElement) {
            stripScope($childFlowElement);
        }
    }    
}
