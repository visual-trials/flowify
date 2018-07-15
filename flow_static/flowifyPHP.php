<?php

require "coupleVisualInfoToAst.php";

$flowElementId = 0;
$flowConnectionId = 0;
$flowConnections = [];
$code = null;

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
    
    $rootFlowElementArray = arrayfyFlowElements($rootFlowElement);
    
    $flowConnectionsArray = arrayfyFlowConnections($flowConnections);
    
    $usedVisualInfos = [];
    // TODO: maybe we should give extendFlowElementsWithVisualInfo the flowElements as objects?
    extendFlowElementsWithVisualInfo($rootFlowElementArray, $visualInfos, $usedVisualInfos);

    // TODO: If we use at least part of the nr of visualInfos we had, we store it (think of something better!)
    if (count($visualInfos) > 0 && count($usedVisualInfos) / count($visualInfos) > 0.5) {
        // FIXME: check if this goes right!
        storeVisualInfos($usedVisualInfos, $fileToFlowifyWithoutExtention);
    }

    $flowifiedPhp = [];
    $flowifiedPhp['code'] = explode("\n", $code);;
    $flowifiedPhp['rootFlowElement'] = $rootFlowElementArray;
    $flowifiedPhp['flowConnections'] = $flowConnectionsArray;
    $flowifiedPhp['statements'] = $statements;
    $flowifiedPhp['visualInfos'] = $visualInfos;
    $flowifiedPhp['usedVisualInfos'] = $usedVisualInfos;

    return $flowifiedPhp;
}

function flowifyProgram($statements) {
    
    $astNodeIdentifier = getAstNodeIdentifier(null);

    $rootFlowElement = createFlowElement('root', 'root', null, $astNodeIdentifier);

    // TODO: should we do anything with the return value of the main function?
    $noReturnFlowElement = flowifyStatements($statements, $rootFlowElement);
    
    return $rootFlowElement;
}
    
function flowifyStatements ($statements, $bodyFlowElement) {

    $returnFlowElement = null;

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

function flowifyExpression ($expression, $parentFlowElement) {  

    $varsInScope = &$parentFlowElement->varsInScope;
    $functionsInScope = &$parentFlowElement->functionsInScope;
    
    $expressionType = $expression['nodeType'];

    $flowElement = null;

    $astNodeIdentifier = getAstNodeIdentifier($expression);

    if ($expressionType === 'Expr_Variable') {

        $name = $expression['name'];
        if (array_key_exists($name, $varsInScope)) {
            // Note: this could be a conditionalJoinVariableFlowElement
            $flowElement = $varsInScope[$name];
        }
        else {
            $flowElement = createAndAddChildlessFlowElementToParent('variable', $name, null, $astNodeIdentifier, $parentFlowElement);
        }
    }
    else if ($expressionType === 'Scalar_LNumber') {
        $flowElement = createAndAddChildlessFlowElementToParent('constant', null, $expression['value'], $astNodeIdentifier, $parentFlowElement);
    }
    else if ($expressionType === 'Expr_PreInc'  ||
             $expressionType === 'Expr_PreDec'  ||
             $expressionType === 'Expr_PostInc' ||
             $expressionType === 'Expr_PostDec') {
        
        $preChange = false;
        if ($expressionType === 'Expr_PreInc' || $expressionType === 'Expr_PreDec') {
            $preChange = true;
        }
        $opertaionName = '--';
        if ($expressionType === 'Expr_PreInc' || $expressionType === 'Expr_PostInc') {
            $opertaionName = '++';
        }
        
        $expressionVariable = $expression['var'];
        $variableName = $expressionVariable['name'];

        // Note: This will normally find the variable declaration (or an earlier assignment). 
        //       We call this flowOldVariable since this oldVariable is now being re-assigned.
        // TODO: What if the variable was not yet declared? Would that not cause an issue?
        //       Since the old and the new variable will have the same astNodeIdentifier! 
        $flowOldVariable = flowifyExpression($expressionVariable, $parentFlowElement);

        $flowPrimitiveFunction = createAndAddChildlessFlowElementToParent('primitiveFunction', $opertaionName, null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($flowOldVariable, $flowPrimitiveFunction);

        $variableAstNodeIdentifier = getAstNodeIdentifier($expressionVariable);
        $flowVariableAssigned = createAndAddChildlessFlowElementToParent('variable', $variableName, null, $variableAstNodeIdentifier, $parentFlowElement);

        addFlowConnection($flowPrimitiveFunction, $flowVariableAssigned);
        
        // FIXME: is it correct to check for array_key_exists and is_null here?
        if (array_key_exists($variableName, $varsInScope) && !is_null($varsInScope[$variableName])) {
            addFlowConnection($varsInScope[$variableName], $flowVariableAssigned, 'identity');
        }

        $varsInScope[$variableName] = $flowVariableAssigned;
        if ($preChange) {
            $flowElement = $flowVariableAssigned;  // We take the flowVariableAssigned as output if is is a Pre inc or decr
        }
        else {
            $flowElement = $flowOldVariable;  // We take the flowOldVariable as output if is is a Post inc or decr
        }
        
    }
    else if ($expressionType === 'Expr_PostDec') {
        die ('Expr_PostDec'); // FIXME
    }
    else if ('Expr_AssignOp_' === substr($expressionType, 0, strlen('Expr_AssignOp_'))) {
        $assignOps = [
            'Expr_AssignOp_Plus'        => '+',
            'Expr_AssignOp_Minus'       => '-',
            'Expr_AssignOp_Mul'         => '*',
            'Expr_AssignOp_Div'         => '/',
            'Expr_AssignOp_Concat'      => '.',
            'Expr_AssignOp_Mod'         => '%',
            'Expr_AssignOp_BitwiseAnd'  => '&',
            'Expr_AssignOp_BitwiseOr'   => '|',
            'Expr_AssignOp_BitwiseXor'  => '^',
            'Expr_AssignOp_ShiftLeft'   => '<<',
            'Expr_AssignOp_ShiftRight'  => '>>',
            'Expr_AssignOp_Pow' => '**',
        ];
        
        $assignOpName = $assignOps[$expressionType];
        
        $expressionVariable = $expression['var'];
        $expressionAssign = $expression['expr'];
        
        $variableName = $expressionVariable['name'];

        // Note: This will normally find the variable declaration (or an earlier assignment). 
        //       We call this flowOldVariable since this oldVariable is now being re-assigned.
        // TODO: What if the variable was not yet declared? Would that not cause an issue?
        //       Since the old and the new variable will have the same astNodeIdentifier! 
        $flowOldVariable = flowifyExpression($expressionVariable, $parentFlowElement);
        $flowAssign = flowifyExpression($expressionAssign, $parentFlowElement);

        $flowPrimitiveFunction = createAndAddChildlessFlowElementToParent('primitiveFunction', $assignOpName, null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($flowOldVariable, $flowPrimitiveFunction);
        addFlowConnection($flowAssign, $flowPrimitiveFunction);

        $variableAstNodeIdentifier = getAstNodeIdentifier($expressionVariable);
        $flowVariableAssigned = createAndAddChildlessFlowElementToParent('variable', $variableName, null, $variableAstNodeIdentifier, $parentFlowElement);

        addFlowConnection($flowPrimitiveFunction, $flowVariableAssigned);
        
        // FIXME: is it correct to check for array_key_exists and is_null here?
        if (array_key_exists($variableName, $varsInScope) && !is_null($varsInScope[$variableName])) {
            addFlowConnection($varsInScope[$variableName], $flowVariableAssigned, 'identity');
        }

        $varsInScope[$variableName] = $flowVariableAssigned;
        $flowElement = $flowVariableAssigned;
        
    }
    else if ('Expr_BinaryOp_' === substr($expressionType, 0, strlen('Expr_BinaryOp_'))) {
        $binaryOps = [
            'Expr_BinaryOp_BitwiseAnd'      => '&',
            'Expr_BinaryOp_BitwiseOr'       => '|',
            'Expr_BinaryOp_BitwiseXor'      => '^',
            'Expr_BinaryOp_BooleanAnd'      => '&&',
            'Expr_BinaryOp_BooleanOr'       => '||',
            'Expr_BinaryOp_Coalesce'        => '??',
            'Expr_BinaryOp_Concat'          => '.',
            'Expr_BinaryOp_Div'             => '/',
            'Expr_BinaryOp_Equal'           => '==',
            'Expr_BinaryOp_GreaterOrEqual'  => '>=',
            'Expr_BinaryOp_Greater'         => '>',
            'Expr_BinaryOp_Identical'       => '===',
            'Expr_BinaryOp_LogicalAnd'      => 'and',
            'Expr_BinaryOp_LogicalOr'       => 'or',
            'Expr_BinaryOp_LogicalXor'      => 'xor',
            'Expr_BinaryOp_Minus'           => '-',
            'Expr_BinaryOp_Mod'             => '%',
            'Expr_BinaryOp_Mul'             => '*',
            'Expr_BinaryOp_NotEqual'        => '!=',
            'Expr_BinaryOp_NotIdentical'    => '!==',
            'Expr_BinaryOp_Plus'            => '+',
            'Expr_BinaryOp_Pow'             => '**',
            'Expr_BinaryOp_ShiftLeft'       => '<<',
            'Expr_BinaryOp_ShiftRight'      => '>>',
            'Expr_BinaryOp_SmallerOrEqual'  => '<=',
            'Expr_BinaryOp_Smaller'         => '<',
            'Expr_BinaryOp_Spaceship'       => '<=>',
        ];
        
        $binaryOpName = $binaryOps[$expressionType];
        
        $leftExpression = $expression['left'];
        $rightExpression = $expression['right'];

        $leftFlow = flowifyExpression($leftExpression, $parentFlowElement);
        $rightFlow = flowifyExpression($rightExpression, $parentFlowElement);

        $flowElement = createAndAddChildlessFlowElementToParent('primitiveFunction', $binaryOpName, null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($leftFlow, $flowElement);
        addFlowConnection($rightFlow, $flowElement);
    }
    else if ($expressionType === 'Expr_Assign') {

        $variableExpression = $expression['var'];
        $variableName = $variableExpression['name'];

        // Note: the expression (that is assigned) might be constant, which can be used to directly fill the variable (so no connection is needed towards to variable)
        //       but you COULD make the constant its own FlowElements and make the assigment iself a 'function' that combines the empty variable
        //       with the constant, resulting in a variable (header) with a value (body)

        $assignExpression = $expression['expr'];

        $flowAssign = flowifyExpression($assignExpression, $parentFlowElement);

        $flowElement = createAndAddChildlessFlowElementToParent('variable', $variableName, null, $astNodeIdentifier, $parentFlowElement);

        addFlowConnection($flowAssign, $flowElement);
        
        // FIXME: is it correct to check for array_key_exists and is_null here?
        if (array_key_exists($variableName, $varsInScope) && !is_null($varsInScope[$variableName])) {
            addFlowConnection($varsInScope[$variableName], $flowElement, 'identity');
        }

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
            // This is a call to a known function (that we have the body-AST of)
            $functionStatement = $functionsInScope[$functionName];

            // Add the function-call element as the container of the function
            $functionCallFlowElement = createAndAddFlowElementToParent('function', $functionName, null, $astNodeIdentifier, $parentFlowElement, $useVarScopeFromParent = false);

            // Flowify the body of the function (including the return value) and use that return value as our own output
            $flowElement = flowifyFunction($functionStatement, $flowCallArguments, $functionCallFlowElement);
        }
        else if (array_key_exists($functionName, $knownPrimitiveFunctions)) {
            // Primitive function call (or at least a function we don't have the body from)

            $primitiveFunctionCallFlowElement = createAndAddChildlessFlowElementToParent('primitiveFunction', $functionName, null, $astNodeIdentifier, $parentFlowElement);
            foreach ($flowCallArguments as $flowCallArgument) {
                addFlowConnection($flowCallArgument, $primitiveFunctionCallFlowElement);
            }
            $flowElement = $primitiveFunctionCallFlowElement;
        }
        else {
            // Unknown function call (or at least a function we don't have the body from and is not a known primitive function)

            // TODO: we might want to allow non existing function for the time being (and mark them as non-existing)
            // We use the 'primitiveFunction' for now.
            
            $primitiveFunctionCallFlowElement = createAndAddChildlessFlowElementToParent('primitiveFunction', $functionName, null, $astNodeIdentifier, $parentFlowElement);
            foreach ($flowCallArguments as $flowCallArgument) {
                addFlowConnection($flowCallArgument, $primitiveFunctionCallFlowElement);
            }
            $flowElement = $primitiveFunctionCallFlowElement;
            
            // die("The function $functionName could not be found!\n");
        }

    }
    else {
        echo "expressionType '" . $expression['nodeType'] ."', but not supported!\n";
    }

    // FIXME: if no elements found/created, do this?            $flowElement->name = '<unknown>';

    return $flowElement;
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

    $returnFlowElement = flowifyStatements($statements, $functionCallFlowElement);

    return $returnFlowElement;

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
        $forStepFlowElement1 = createAndAddFlowElementToParent('ifThen', '#1', null, $forStepAstNodeIdentifier1, $forFlowElement);
        
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
        
        /*
        // STEP 2
        
        $forStepAstNodeIdentifier2 = $forAstNodeIdentifier . "_2";
        // FIXME: change this from a ifThen for a forStep
        $forStepFlowElement2 = createAndAddFlowElementToParent('ifThen', '#2', null, $forStepAstNodeIdentifier2, $forFlowElement);
        
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
    $noReturnFlowElement = flowifyStatements($iterStatements, $iterBodyFlowElement);
    
    // TODO: we don't have a return statement in iter-bodies, so we call it $noReturnFlowElement here
    
    // == UPDATE ==
    
    // FIXME: replace ifCond with forUpdate
    $updateBodyFlowElement = createAndAddFlowElementToParent('ifCond', 'update', null, $forAstNodeIdentifier . "_ForUpdate", $forStepFlowElement);
    $flowElement = flowifyExpression($updateExpression, $updateBodyFlowElement);
    
    // TODO: we are not doing anything with the flowElement coming from the updateBody. Is this ok?
    
    
    
    // Checking if the loop vars were changed in the forStep (that is: cond/iter/update)
    
    // We loop through all the varsInScope we had before the condBody
    foreach ($varsInScopeBeforeCondBody as $variableName => $varInScopeElement) {
        
        $varReplacedInForStep = false;
        
        // We check if we have the same variable in our forStep scope
        if (array_key_exists($variableName, $forStepFlowElement->varsInScope)) {
            // The var exists both in beforeCondBody and in the forStep's scope
            if ($forStepFlowElement->varsInScope[$variableName]->id !== $varsInScopeBeforeCondBody[$variableName]->id) {
                // The vars differ, so it must have been replaced (or extended) inside the forStep. 
                $varReplacedInForStep = true;
            }
        }
        
        // If the variable is USED inside the iter or update body then a conditionalSplitVariable should be created.
        // The true-output of the conditionalSplitVariable should get the connection (that is: it's from should now point to it)
        // that used to be connect the beforeCondBody and the iterBody or updateBody.
        // The false-output of the conditionalSplitVariable should connect to the passThroughVariable in the done body,
        // but ONLY if the variable was REPLACED. Otherwise it should not be connected at all.
        // That way, if the variable is used after the for, it can connect to this false-output of the conditionalSplitVariable
        // (when it was changed by the for-loop).
        // 
        // If the variable was REPLACED in the forStep conditionalJoinVariableFlowElement should be created and added to the forFlowElement
        // and also connect the inputs of the  conditionalJoinVariable with the beforeCondBody- and forStep- variable.
        // The output of the conditionalJoinVariable should  be connected to the input of a corresponding (and new) conditionalSplitVariable, 
        // which in turn should be added to the condBody. In additional, the false-output fo conditionalSplitVariable 
        // should be added to the varsInScope of the forElement.
        
        if ($varReplacedInForStep) {
            
            // FIXME: also we we haven't replaced anything we should create split conditionals for variables we ONLY USE!
            
            $variableBeforeCondBody = $varsInScopeBeforeCondBody[$variableName];
            $variableAfterCondBody = $varsInScopeAfterCondBody[$variableName];
            $variableAfterIterAndUpdateBody = $forStepFlowElement->varsInScope[$variableName];
            
            $conditionalJoinVariableFlowElement = null;
            $conditionalSplitVariableFlowElement = null;

            // TODO: could this be moved outside the foreach of the $varsInScopeParent? Or are we adding elements to the thenBody and elseBody in this loop?
            // TODO: we should probably use a hashmap of all flowElements inside the thenBody and elseBody.
            $elementIdsInCondBody = getElementsIdsIn($condBodyFlowElement);
            $elementIdsInIterBody = getElementsIdsIn($iterBodyFlowElement);
            $elementIdsInUpdateBody = getElementsIdsIn($updateBodyFlowElement);
            
            $updatedConnectionIdsFromThisElement = [];

            // IMPORTANT NOTE: right now we are assuming that the condBody doesn't reassign
            //                 the variable! If it does, then the code below will not give the proper result!
            
            foreach ($variableBeforeCondBody->connectionIdsFromThisElement as $connectionIdFromVariable) {
                // By default we want to keep the connections, so we take over the id in the loop
                $currentConnectionIdFromThisElement = $connectionIdFromVariable;
                $newlyAddedConnectionIdFromThisElement = null;
                
                $connectionToBeChanged = getConnectionById($connectionIdFromVariable);
                $flowElementIdInCondOrIterOrUpdateBody = $connectionToBeChanged->to;

                // Here we check whether the flowElement to which the variableAfterCondBody is connected to, is in the iterBody or updateBody.
                $variableConnectedWithCondBody = false;
                $variableConnectedWithIterBody = false;
                $variableConnectedWithUpdateBody = false;
                if (in_array($flowElementIdInCondOrIterOrUpdateBody, $elementIdsInCondBody)) {
                    $variableConnectedWithCondBody = true;
                }
                if (in_array($flowElementIdInCondOrIterOrUpdateBody, $elementIdsInIterBody)) {
                    $variableConnectedWithIterBody = true;
                }
                if (in_array($flowElementIdInCondOrIterOrUpdateBody, $elementIdsInUpdateBody)) {
                    $variableConnectedWithUpdateBody = true;
                }
                
                if ($variableConnectedWithCondBody ||
                    $variableConnectedWithIterBody ||
                    $variableConnectedWithUpdateBody) {
                       
                    if ($conditionalJoinVariableFlowElement === null) {
                        // Note: this also assumes conditionalSplitVariableFlowElement is null!
                        
                        
                        // FIXME: double check: we are using the forStepIdentifier inside the for element (with a varname) is this ok?
                        // FIXME: should we not use the ast of the condition body itself + variable name?
                        $conditionalJoinVariableAstNodeIdentifier = $forStepAstNodeIdentifier . "_Cond_" . $variableName;
                        // TODO: should we put this conditionalJoinVariable into the condBody or the stepBody or the forBody?
                        $conditionalJoinVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalJoinVariable', $variableName, null, $conditionalJoinVariableAstNodeIdentifier, $condBodyFlowElement);
                        $connectionIdToConditionalJoinVariable = addFlowConnection($variableBeforeCondBody, $conditionalJoinVariableFlowElement, 'conditional');
                        // This connection will effectively be added to $variableAfterCondBody->connectionIdsFromThisElement
                        $newlyAddedConnectionIdFromThisElement = $connectionIdToConditionalJoinVariable;
                        
                        $passBackVariableAstNodeIdentifier = $forStepAstNodeIdentifier . "_PassBack_" . $variableName;

                        $passBackVariableFlowElement = createAndAddChildlessFlowElementToParent('passBackVariable', $variableName, null, $passBackVariableAstNodeIdentifier, $backBodyFlowElement);

                        addFlowConnection($variableAfterIterAndUpdateBody, $passBackVariableFlowElement, 'conditional');
                        addFlowConnection($passBackVariableFlowElement, $conditionalJoinVariableFlowElement, 'conditional');
                        
                        
                        // Adding the conditionalSplitVariableFlowElement and adding a connection to connect from the conditionalJoinVariableFlowElement to it
                        $connectionTypeToConditionalSplitVariable = $connectionToBeChanged->type;
                        
                        // FIXME: is this AST Identifier correct?
                        $conditionalSplitVariableAstNodeIdentifier = $forStepAstNodeIdentifier . "_" . $variableName . "_SPLIT";
                        // FIXME: change type to 'conditionalSplitVariable'?
                        // FIXME: should this be put into the forBody, forStepBody or the condBody?
                        $conditionalSplitVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalSplitVariable', $variableName, null, $conditionalSplitVariableAstNodeIdentifier, $condBodyFlowElement);
                       
                        // Adding a connection from the conditionalJoinVariableFlowElement to the conditionalSplitVariableFlowElement
                        $connectionIdToConditionalSplitVariable = addFlowConnection($conditionalJoinVariableFlowElement, $conditionalSplitVariableFlowElement, $connectionToBeChanged->type); // Note: we use the original type
                        
                        
                        
                        // Adding the variable to the doneBody as a passthrough variable
                        
                        // TODO: should we really use the variable name in the identifier?
                        $passThroughVariableAstNodeIdentifier = $forStepAstNodeIdentifier . "_Pass_" . $variableName;

                        $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $doneBodyFlowElement);

                        // Connecting the conditionalSplitVariableFlowElement to the passthrough variable (inside the doneBody)
                        // FIXME: choose a SIDE!
                        addFlowConnection($conditionalSplitVariableFlowElement, $passThroughVariableFlowElement);
                        
                        // Making sure the variable will be picked up (as output) after the for-loop
                        $forFlowElement->varsInScope[$variableName] = $passThroughVariableFlowElement;
                    }
                    
                    if ($variableConnectedWithCondBody) {
                        // The variable is used in the condBody, which means it should connect from the conditionalJoinVariableFlowElement
                        $connectionToBeChanged->from = $conditionalJoinVariableFlowElement->id; // TODO: should we do it this way?
                    }

                    if ($variableConnectedWithIterBody || $variableConnectedWithUpdateBody) {
                        // We set the from in the connection to the flowElementIdInThenOrElseBody
                        // FIXME: we should add to which SIDE the connection is connected: true-side or false-side (depending on THEN or ELSE)
                        $connectionToBeChanged->from = $conditionalSplitVariableFlowElement->id; // TODO: should we do it this way?
                        
                        // Seting currentConnectionIdFromThisElement to null, so it won't be added again to (effectively removed from) $variableAfterCondBody->connectionIdsFromThisElement
                        $currentConnectionIdFromThisElement = null;
                        
                        /* FIXME: do this too!
                        // FIXME: right now null means 'normal' (which should overrule). We should change the default to 'dataflow' or something
                        if ($connectionToBeChanged->type === null) {
                            // TODO: should we keep a prio number for each type of connection and check if the prio is higher here?
                            $connectionTypeToConditionalSplitVariable = null; 
                        }
                        */
                        
                    }
   
                }
                if ($currentConnectionIdFromThisElement !== null) {
                    array_push($updatedConnectionIdsFromThisElement, $currentConnectionIdFromThisElement);
                }
                if ($newlyAddedConnectionIdFromThisElement !== null) {
                    array_push($updatedConnectionIdsFromThisElement, $newlyAddedConnectionIdFromThisElement);
                }
            }
            $variableBeforeCondBody->connectionIdsFromThisElement = $updatedConnectionIdsFromThisElement;
            
        }
        
    }
    
}


function flowifyIfStatement($ifStatement, $parentFlowElement) {
    
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
        
        $thenAstNodeIdentifier = getAstNodeIdentifier($thenStatements);
        $thenBodyFlowElement = createAndAddFlowElementToParent('ifThen', 'then', null, $thenAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
        
        // Note: we *copy* the varsInScope here. This is because the thenBody might replace vars in it's scope,
        //       These are however conditional-replacement when it comes to the if-statement. 
        //       Instead of the thenBody letting the vars in the if-scope to be replaced, we *add* it later to our varsInScope,
        //       by using a conditionalFlowElement.
        
        $thenBodyFlowElement->varsInScope = $ifFlowElement->varsInScope;  // copy!
        $thenBodyFlowElement->functionsInScope = &$ifFlowElement->functionsInScope;
        
        // TODO: we don't have a return statement in then-bodies, so we call it $noReturnFlowElement here (but we shouldn't get it at all)
        $noReturnFlowElement = flowifyStatements($thenStatements, $thenBodyFlowElement);
        
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
            $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
            
            // Note: we *copy* the varsInScope here. This is because the elseBody might replace vars in it's scope,
            //       These are however conditional-replacement when it comes to the if-statement. 
            //       Instead of the elseBody letting the vars in the if-scope to be replaced, we *add* it later to our varsInScope,
            //       by using a conditionalFlowElement.
            
            $elseBodyFlowElement->varsInScope = $ifFlowElement->varsInScope;  // copy!
            $elseBodyFlowElement->functionsInScope = &$ifFlowElement->functionsInScope;
            
            // TODO: we don't have a return statement in then-bodies, so we call it $noReturnFlowElement here (but we shouldn't get it at all)
            $noReturnFlowElement = flowifyStatements($elseStatements, $elseBodyFlowElement);
        }
        
        // Note: we are comparing the varsInScope from the parentFlowElement with the varsInScope of the then/elseBodyFlowElement. 
        //       We don't compare with the varsInScope of the ifFlowElement, because its only a wrapper-element (although it has 
        //       the same scope as the parent)
        
        $varsInScopeParent = &$parentFlowElement->varsInScope;
        $varsInScopeThenBody = &$thenBodyFlowElement->varsInScope;
        $varsInScopeElseBody = [];
        if ($elseBodyFlowElement !== null) {
            $varsInScopeElseBody = &$elseBodyFlowElement->varsInScope;
        }

        // We loop through all the varsInScope of the parentFlowElement
        foreach ($varsInScopeParent as $variableName => $parentVarInScopeElement) {
            
            $varReplacedInThenBody = false;
            $varReplacedInElseBody = false;
            
            // We check if we have the same variable in our thenBody scope
            if (array_key_exists($variableName, $varsInScopeThenBody)) {
                // The var exists both in thenBodyFlowElement and in the parent's scope
                if ($varsInScopeThenBody[$variableName]->id !== $varsInScopeParent[$variableName]->id) {
                    // The vars differ, so it must have been replaced (or extended) inside the thenBody. 
                    $varReplacedInThenBody = true;
                }
            }
            
            // We check if we have the same variable in our elseBody scope
            if (array_key_exists($variableName, $varsInScopeElseBody)) {
                // The var exists both in elseBodyFlowElement and in the parent's scope
                if ($varsInScopeElseBody[$variableName]->id !== $varsInScopeParent[$variableName]->id) {
                    // The vars differ, so it must have been replaced (or extended) inside the elseBody. 
                    $varReplacedInElseBody = true;
                }
            }

            $thenVariableFlowElement = null;
            $elseVariableFlowElement = null;
            if ($varReplacedInThenBody && $varReplacedInElseBody) {
                // We overwrite the parent's varInScope and adding them both using a conditionalJoinVariableFlowElement.
                $thenVariableFlowElement = $varsInScopeThenBody[$variableName];  // NOT A copy ANYMORE!
                $elseVariableFlowElement = $varsInScopeElseBody[$variableName];  // NOT A copy ANYMORE!
            }
            else if ($varReplacedInThenBody) {
                // Only the thenBody has replaced the variable. We use the parent's variable as the (default) else variable
                
                
                // Add an elseBody if it doesn't exist yet
                if ($elseBodyFlowElement === null) {
                    
                    $elseAstNodeIdentifier = $ifAstNodeIdentifier . "_ImplicitElse";
                    // FIXME: this should be of type: 'ifElseImplicit'
                    $elseBodyFlowElement = createAndAddFlowElementToParent('ifElse', 'else', null, $elseAstNodeIdentifier, $ifFlowElement, $useVarScopeFromParent = false);
                }
                
                {
                    // Adding the variable to the elseBody as a passthrough variable
                    
                    // TODO: should we really use the variable name in the identifier?
                    $passThroughVariableAstNodeIdentifier = $elseAstNodeIdentifier . "_" . $variableName;
                    $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $elseBodyFlowElement);

                    // Connecting the variable in the parent to the passthrough variable (inside the thenBody)
                    addFlowConnection($varsInScopeParent[$variableName], $passThroughVariableFlowElement);

                    // TODO: do we need to add this passthrough variable to the scope of the ElseBody? 
                    // $varsInScopeElseBody[$parameterName] = $passThroughVariableFlowElement;
                }
                
                $thenVariableFlowElement = $varsInScopeThenBody[$variableName];  // NOT A copy ANYMORE!
                // TODO: this is without passthrough variable: $elseVariableFlowElement = $varsInScopeParent[$variableName];  // NOT A copy ANYMORE!
                $elseVariableFlowElement = $passThroughVariableFlowElement;  // NOT A copy ANYMORE!
            }
            else if ($varReplacedInElseBody) {
                // Only the elseBody has replaced the variable. We use the parent's variable as the (default) then variable
                
                {
                    // Adding the variable to the thenBody as a passthrough variable
                    
                    // TODO: should we really use the variable name in the identifier?
                    $passThroughVariableAstNodeIdentifier = $thenAstNodeIdentifier . "_" . $variableName;
                    $passThroughVariableFlowElement = createAndAddChildlessFlowElementToParent('passThroughVariable', $variableName, null, $passThroughVariableAstNodeIdentifier, $thenBodyFlowElement);

                    // Connecting the variable in the parent to the passthrough variable (inside the thenBody)
                    addFlowConnection($varsInScopeParent[$variableName], $passThroughVariableFlowElement);

                    // TODO: do we need to add this passthrough variable to the scope of the ThenBody? 
                    // $varsInScopeThenBody[$parameterName] = $passThroughVariableFlowElement;
                }
                
                // TODO: this is without passthrough variable: $thenVariableFlowElement = $varsInScopeParent[$variableName];  // NOT A copy ANYMORE!
                $thenVariableFlowElement = $passThroughVariableFlowElement;  // NOT A copy ANYMORE!
                $elseVariableFlowElement = $varsInScopeElseBody[$variableName];  // NOT A copy ANYMORE!
            }
            else {
                // The variable wasn't replaced by either the thenBody or the elseBody, so nothing to do here
            }
            
            // The variable was replaced in either the thenBody or the elseBody.
            // This means we should create a conditionalJoinVariableFlowElement and add it to the ifFlowElement
            // and also connect this conditionalJoinVariable with the then- and else- variable.
            // In additional, it should be added to the varsInScope of the parent, so if the variable is used 
            // by another flowElement, it can connect to this conditionalJoinVariable
            if ($varReplacedInThenBody || $varReplacedInElseBody) {
                $conditionalJoinVariableAstNodeIdentifier = $ifAstNodeIdentifier . "_" . $variableName;
                $conditionalJoinVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalJoinVariable', $variableName, null, $conditionalJoinVariableAstNodeIdentifier, $ifFlowElement);
                addFlowConnection($thenVariableFlowElement, $conditionalJoinVariableFlowElement, 'conditional');
                addFlowConnection($elseVariableFlowElement, $conditionalJoinVariableFlowElement, 'conditional');
                $varsInScopeParent[$variableName] = $conditionalJoinVariableFlowElement;
            }

            {            
                
                // We also want to create a conditional *split* element between the condBody and thenBody, and between the condBody and elseBody
                // We need to know the connections going from the condBody into the thenBody and elseBody (for this variable)
                // We do this by looping all the connections from the condBody for this variable (in its 'connectionIdsFromThisElement')
                $variableAfterCondBody = $varsInScopeAfterCondBody[$variableName];
                $conditionalSplitVariableFlowElement = null;
                $connectionIdToConditionalSplitVariable = null;
                $connectionTypeToConditionalSplitVariable = null;
                $updatedConnectionIdsFromThisElement = [];
                
                // TODO: could this be moved outside the foreach of the $varsInScopeParent? Or are we adding elements to the thenBody and elseBody in this loop?
                // TODO: we should probably use a hashmap of all flowElements inside the thenBody and elseBody.
                $elementIdsInThenBody = getElementsIdsIn($thenBodyFlowElement);
                $elementIdsInElseBody = [];
                if ($elseBodyFlowElement !== null) {
                    $elementIdsInElseBody = getElementsIdsIn($elseBodyFlowElement);
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
                        
                    if ($variableConnectedWithThenBody || $variableConnectedWithElseBody) {
                        if ($conditionalSplitVariableFlowElement === null) {
                            // Adding the conditionalSplitVariableFlowElement and adding a connection to connect from the variableAfterCondBody to it
                            $connectionTypeToConditionalSplitVariable = $connectionToBeChanged->type;
                            
                            // FIXME: is this AST Identifier correct?
                            $conditionalSplitVariableAstNodeIdentifier = $ifAstNodeIdentifier . "_" . $variableName . "_SPLIT";
                            // FIXME: should this be put into the ifBody or the condBody?
                            $conditionalSplitVariableFlowElement = createAndAddChildlessFlowElementToParent('conditionalSplitVariable', $variableName, null, $conditionalSplitVariableAstNodeIdentifier, $ifFlowElement);
                            
                            // Adding a connection from the variableAfterCondBody to the conditionalSplitVariableFlowElement
                            $connectionIdToConditionalSplitVariable = addFlowConnection($variableAfterCondBody, $conditionalSplitVariableFlowElement, $connectionToBeChanged->type); // Note: we use the original type
                            // This connection will effectively be added to $variableAfterCondBody->connectionIdsFromThisElement
                            $newlyAddedConnectionIdFromThisElement = $connectionIdToConditionalSplitVariable;
                        }

                        // We set the from in the connection to the flowElementIdInThenOrElseBody
                        // FIXME: we should add to which SIDE the connection is connected: true-side or false-side (depending on THEN or ELSE)
                        $connectionToBeChanged->from = $conditionalSplitVariableFlowElement->id; // TODO: should we do it this way?
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
    
}

// Helper functions

function addFlowConnection ($fromFlowElement, $toFlowElement, $connectionType = null) {
    global $flowConnections, $flowConnectionId;

    $flowConnection = new FlowConnection;
    $flowConnection->id = $flowConnectionId;
    $flowConnection->from = $fromFlowElement->id;
    $flowConnection->to = $toFlowElement->id;
    $flowConnection->type = $connectionType;
    
    $flowConnections[$flowConnectionId] = $flowConnection;
    array_push($fromFlowElement->connectionIdsFromThisElement, $flowConnectionId);

    $flowConnectionId++;
    
    return $flowConnection->id;
}

function addFlowElementToParent ($flowElement, $parentFlowElement) {
    array_push($parentFlowElement->children, $flowElement);
}

function createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = true, $hasScope = true) {

    global $flowElementId;

    $flowElement = new FlowElement;
    $flowElement->id = $flowElementId++;
    $flowElement->type = $flowElementType;
    $flowElement->name = $flowElementName;
    $flowElement->value = $flowElementValue;
    $flowElement->connectionIdsFromThisElement = [];
    if ($canHaveChildren) {
        $flowElement->children = [];
    }
    $flowElement->astNodeIdentifier = $astNodeIdentifier;
    
    if ($hasScope) {
        $flowElement->varsInScope = [];
        $flowElement->functionsInScope = [];
    }

    return $flowElement;
}

function createAndAddFlowElementToParent ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $parentFlowElement, $useVarScopeFromParent = true) {
    
    $flowElement = createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = true, $hasScope = true);
    if ($useVarScopeFromParent) {
        $flowElement->varsInScope = &$parentFlowElement->varsInScope;
        $flowElement->functionsInScope = &$parentFlowElement->functionsInScope;
    }
    addFlowElementToParent($flowElement, $parentFlowElement);

    return $flowElement;
}

function createAndAddChildlessFlowElementToParent ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $parentFlowElement) {

    $flowElement = createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = false, $hasScope = false);
    addFlowElementToParent($flowElement, $parentFlowElement);

    return $flowElement;
}

function getConnectionById ($connectionId) {
    global $flowConnections;
    
    return $flowConnections[$connectionId];
}

function getElementsIdsIn($flowElement) {
    $containerIdsInFlowElement = [$flowElement->id];
    
    foreach ($flowElement->children as $childFlowElement) {
        // TODO: we should not go into a function when we are trying to find connection to elements inside a flowElement.
        //       This is because a function(call) reset its varsInScope, so no variables *outside* its scope could 
        //       connect (directly) with one of its children.
        $containerIdsInChildFlowElement = getElementsIdsIn($childFlowElement);
        $containerIdsInFlowElement = array_merge($containerIdsInFlowElement, $containerIdsInChildFlowElement);
    }
    return $containerIdsInFlowElement;
}

function arrayfyFlowElements ($flowElement) {
    
    $flowElementArray = [];
    
    $flowElementArray['id'] = $flowElement->id;
    $flowElementArray['type'] = $flowElement->type;
    $flowElementArray['name'] = $flowElement->name;
    $flowElementArray['value'] = $flowElement->value;
    $flowElementArray['astNodeIdentifier'] = $flowElement->astNodeIdentifier;
    
    if (!is_null($flowElement->children)) {
        $flowElementChildrenArray = [];
        foreach ($flowElement->children as $childFlowElement) {
            array_push($flowElementChildrenArray, arrayfyFlowElements($childFlowElement));
        }
        $flowElementArray['children'] = $flowElementChildrenArray;
    }
    
    return $flowElementArray;
}

function arrayfyFlowConnections($flowConnections) {
    $flowConnectionsArray = [];
    foreach ($flowConnections as $flowConnectionIdNow => $flowConnection) {
        $flowConnectionArray = [];
        $flowConnectionArray['id'] = $flowConnection->id;
        $flowConnectionArray['type'] = $flowConnection->type;
        $flowConnectionArray['from'] = $flowConnection->from;
        $flowConnectionArray['to'] = $flowConnection->to;
        
        array_push($flowConnectionsArray, $flowConnectionArray);
    }
    return $flowConnectionsArray;
}

class FlowElement { 
    public $id;
    public $type;
    public $name; 
    public $value; 
    public $connectionIdsFromThisElement;
    public $children;
    public $astNodeIdentifier;
    public $varsInScope;
    public $functionsInScope;    
}

class FlowConnection { 
    public $id;
    public $from;
    public $to; 
    public $type; 
}