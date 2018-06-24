<?php declare(strict_types=1);
// #!/usr/bin/env php

require "coupleVisualInfoToAst.php";

$flowContainerId = 0;
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

    // TODO: START NEW FUNCTION HERE: flowifyProgram($statements)

    $varsInScope = [];
    $functionsInScope = [];

    $astNodeIdentifier = getAstNodeIdentifier(null);

    $flowElement = createFlowElement('root', 'root', null, $astNodeIdentifier);

    // TODO: should we do anything with the return value of the main function?
    $returnFlowElement = flowifyStatements($statements, $varsInScope, $functionsInScope, $flowElement);
    
    // TODO: END NEW FUNCTION HERE

    
    $usedVisualInfos = [];
    extendFlowElementsWithVisualInfo($flowElement, $visualInfos, $usedVisualInfos);

    // TODO: If we use at least part of the nr of visualInfos we had, we store it (think of something better!)
    if (count($visualInfos) > 0 && count($usedVisualInfos) / count($visualInfos) > 0.5) {
        // FIXME: check if this goes right!
        storeVisualInfos($usedVisualInfos, $fileToFlowifyWithoutExtention);
    }

    $flowifiedPhp = [];
    $flowifiedPhp['code'] = explode("\n", $code);;
    $flowifiedPhp['containers'] = $flowElement;
    $flowifiedPhp['connections'] = $flowConnections;
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

function flowifyStatements ($statements, $varsInScope, &$functionsInScope, &$bodyFlowElement) {

    // Note: we made $varsInScope a non-ref in flowifyStatements(), but it still is a ref in flowifyExpression().
    //       We assume that the functionBody (with it arguments as extra local vars) should not change $varsInScope,
    //       but flowifyExpression should. Is this correct?

    $returnFlowElement = null;

    $localFunctions = [];

    // 1)    First find all defined functions, so we known the nodes of them, when they are called
    foreach ($statements as $statement) {

        $statementType = $statement['nodeType'];

        if ($statementType === 'Stmt_Function') {
            $identifierNode = $statement['name']; // TODO: we are assuming 'name' is always present

            if ($identifierNode['nodeType'] === 'Identifier') {
                $localFunctions[$identifierNode['name']] = $statement;
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

            $flowElement = flowifyExpression($expression, $varsInScope, $localFunctions, $bodyFlowElement);

            if ($statementType === 'Stmt_Return') {
                $returnFlowElement = $flowElement;
            }
        }
        else if($statementType === 'Stmt_If') {
            
            flowifyIfStatement($statement, $varsInScope, $localFunctions, $bodyFlowElement);
            
        }
        else {
            echo "statementType '$statementType' found in function body, but not supported!\n";
            echo print_r($statement, true);
            continue;
        }

    }

    return $returnFlowElement;
}

function flowifyExpression ($expression, &$varsInScope, &$functionsInScope, &$parentFlowElement) {  // TODO: should &$functionsInScope really be an argument by reference?

    $expressionType = $expression['nodeType'];

    $flowElement = null;

    $astNodeIdentifier = getAstNodeIdentifier($expression);

    if ($expressionType === 'Expr_Variable') {

        $name = $expression['name'];
        if (array_key_exists($name, $varsInScope)) {
            // TODO: in the beginning of flowifyExpression you should check if it's a variable. If it already exists, you should return a ref to the flow object representing that variable (AT THAT POINT, because it could be overwritten)
            $flowElement = &$varsInScope[$name]; // FIXME: you might want to duplicate certain flow-elements, if they are used inside another function for example (as input-vars)
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

        $leftFlow = flowifyExpression($leftExpression, $varsInScope, $functionsInScope, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $varsInScope, $functionsInScope, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '+', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Mul') {
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $varsInScope, $functionsInScope, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $varsInScope, $functionsInScope, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '*', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Div') {  // TODO: unduplcate this from other BinaryOps!
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $varsInScope, $functionsInScope, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $varsInScope, $functionsInScope, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '/', null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_BinaryOp_Greater') {  // TODO: unduplcate this from other BinaryOps!
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $varsInScope, $functionsInScope, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $varsInScope, $functionsInScope, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('primitiveFunction', '>', null, $astNodeIdentifier, $parentFlowElement);

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

        $flowAssign = flowifyExpression($assignExpression, $varsInScope, $functionsInScope, $parentFlowElement);

        $flowElement = createAndAddFlowElementToParent('variable', $variableName, null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($flowAssign, $flowElement);

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
                    $flowCallArgument = flowifyExpression($callArgumentExpression, $varsInScope, $functionsInScope, $parentFlowElement);
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
            $flowElement = flowifyFunction($functionStatement, $localVars, $functionsInScope, $flowCallArguments, $functionCallFlowElement);

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

function flowifyFunction ($functionStatement, $varsInScope, &$functionsInScope, $flowCallArguments, &$functionCallFlowElement) { // TODO: should &$functionsInScope really be an argument by reference?

    // Note: we made $varsInScope a non-ref in flowifyFunction(), but it still is a ref in flowifyExpression().
    //       We assume that the functionBody (with it arguments as extra local vars) should not change $varsInScope,
    //       but flowifyExpression should. Is this correct?

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

            // FIXME: the $astNodeIdentifier should be based on the attributes of that AST-element!) That way each parameter will have it's own proper $astNodeIdentifier
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
    // $returnFlowElement = flowifyStatements($statements, $varsInScope, $functionsInScope, $functionBodyFlowElement);

    // OR WITHOUT BODY:
    //    - Everything should be inside the function-call (no input/body/output)
    // TODO: don't we need the astNodeIdentifier of the functionStatement for some visualInfo?
    $returnFlowElement = flowifyStatements($statements, $varsInScope, $functionsInScope, $functionCallFlowElement);

    // addFlowElementToParent($functionBodyFlowElement, $functionCallFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $functionBodyFlowElement, so changes to it will not be in the parent!

    return $returnFlowElement;

}

function flowifyIfStatement($ifStatement, &$varsInScope, &$functionsInScope, &$parentFlowElement) {
    
    $astNodeIdentifier = getAstNodeIdentifier($ifStatement);
    // FIXME: this should be a flowElement of the type 'if(-body)' (right?) now using placeholder 'function'.
    $ifFlowElement = createFlowElement('function', 'if', null, $astNodeIdentifier);
    
    // echo print_r($ifStatement, true);

    {
        // == COND ==
        
        $conditionExpression = $ifStatement['cond'];
        
        // FIXME: we can't really use $conditionExpression here! since it's ALSO used by the expression itself (in this case the BinOpGreater expression)
        //        to fix this, there has to be a special visual attribute INSIDE the if-visual info containing the position of the COND-part
        //        This would ALSO solve the problem if the THEN-clause positioning BTW!
        $astNodeIdentifier = getAstNodeIdentifier($conditionExpression);
        // FIXME: this should be a flowElement of the type 'condition' (right?) now using placeholder 'function'.
        $condFlowElement = createFlowElement('function', 'cond', null, $astNodeIdentifier);
        
        $flowElement = flowifyExpression($conditionExpression, $varsInScope, $localFunctions, $condFlowElement);
        
        // TODO: the flowElement coming from the conditionExpression is a boolean and determines 
        //       whether the then-statements or the else(if)-statements are executed. How to should
        //       we visualize this?
        
        addFlowElementToParent($condFlowElement, $ifFlowElement);  // Note: do not call this before flowifyExpression, because this COPIES $condFlowElement, so changes to it will not be in the parent!
        
        
        // == THEN ==
        
        $thenStatements = $ifStatement['stmts'];
        
        // FIXME: HACK: we currently don't get positions from all the statements in the the-body,
        //              so we now use the positional info from FIRST statement (UGLY)
        $astNodeIdentifier = getAstNodeIdentifier($thenStatements[0]);
        // FIXME: this should be a flowElement of the type 'then-body' (right?) now using placeholder 'function'.
        $thenBodyFlowElement = createFlowElement('function', 'then', null, $astNodeIdentifier);
        
        // FIXME: we don't have a return statement in then-bodies, so we call it $noReturnFlowElement (but we shouldn't get it at all)
        $noReturnFlowElement = flowifyStatements($thenStatements, $varsInScope, $functionsInScope, $thenBodyFlowElement);

        addFlowElementToParent($thenBodyFlowElement, $ifFlowElement);  // Note: do not call this before flowifyStatements, because this COPIES $thenBodyFlowElement, so changes to it will not be in the parent!
        
        
        // == ELSE ==
        
        // TODO: $elseStatements = $ifStatement['else']
        
        // TODO: even if the else statement does'nt exist, the else-flowElements has to be created!
        
        
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

function createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = true) {

    global $flowContainerId;

    $flowElement = [];
    $flowElement['id'] = $flowContainerId++;
    $flowElement['type'] = $flowElementType;
    $flowElement['name'] = $flowElementName;
    $flowElement['value'] = $flowElementValue;
    if ($canHaveChildren) {
        $flowElement['children'] = [];
    }
    $flowElement['astNodeIdentifier'] = $astNodeIdentifier;

    return $flowElement;
}

function createAndAddFlowElementToParent ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, &$parentFlowElement, $canHaveChildren = false) {

    $flowElement = createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren);
    addFlowElementToParent($flowElement, $parentFlowElement);

    return $flowElement;
}


