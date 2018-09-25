<?php

function flowifyExpression ($expression, $parentFlowElement, $isToBeAssigned = false) {  

    $functionsInScope = &$parentFlowElement->functionsInScope;
    
    $expressionType = $expression['nodeType'];

    $flowElement = null;

    $astNodeIdentifier = getAstNodeIdentifier($expression);

    if ($expressionType === 'Expr_Variable') {

        $variableName = $expression['name'];
        $connectionType = null; 
        if ($isToBeAssigned) {
            
            // It is to be assigned, so we create an identity-connection to it
            $connectionType = 'identity';
            
            if (array_key_exists($variableName, $parentFlowElement->varsInScopeAvailable)) {
                // The variable already exists and it to be assigned
                $flowElement = buildPathBackwards($parentFlowElement, $variableName, $connectionType);
            }
            else {
                // The variable doesn't exist yet and it to be assigned
                // TODO: we might want to create a variable at the beginning of the function, 
                //       but only when we are not directly in that function (otherwise the assigner also creates
                //       a variable right next to it (in the same lane/body)
                $flowElement = null;
            }
        }
        else {
            // The variable is used, so we create a usage-connection to it
            $connectionType = null; // Note: null now stands for normal 'usage'. // TODO: maybe call it 'var-usage'? or 'data-flow'?
            
            if (array_key_exists($variableName, $parentFlowElement->varsInScopeAvailable)) {
                // The variable already exists and it to be used
                $flowElement = buildPathBackwards($parentFlowElement, $variableName, $connectionType);
            }
            else {
                // The variable doesn't exist yet and it to be used
                // TODO: we probably want to create a variable at the beginning of the function!
                $flowElement = null;
            }
            
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

        // TODO: what if the $flowOldVariable is null?
        addFlowConnection($flowOldVariable, $flowPrimitiveFunction);

        $variableAstNodeIdentifier = getAstNodeIdentifier($expressionVariable);
        $flowVariableAssigned = createVariable($parentFlowElement, $variableName, $variableAstNodeIdentifier);

        addFlowConnection($flowPrimitiveFunction, $flowVariableAssigned);
        
        if ($flowOldVariable !== null) {
            addFlowConnection($flowOldVariable, $flowVariableAssigned, 'identity');
        }
        
        if ($preChange) {
            $flowElement = $flowVariableAssigned;  // We take the flowVariableAssigned as output if is is a Pre inc or decr
        }
        else {
            $flowElement = $flowOldVariable;  // We take the flowOldVariable as output if is is a Post inc or decr
        }

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
        $flowVariableAssigned = createVariable($parentFlowElement, $variableName, $variableAstNodeIdentifier);

        addFlowConnection($flowPrimitiveFunction, $flowVariableAssigned);
        
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

        $flowElement = flowifyExpression($variableExpression, $parentFlowElement, $isToBeAssigned = true);
        
        $astNodeIdentifierVariable = getAstNodeIdentifier($variableExpression);

        // If the variable was not known yet, it should be declared at the beginning of the function
        if ($flowElement === null) {
            $flowElement = createVariable($parentFlowElement, $variableName, $astNodeIdentifierVariable);
        }
        else {
            $existingFlowVariable = $flowElement;
            $flowElement = createVariable($parentFlowElement, $variableName, $astNodeIdentifierVariable);
            addFlowConnection($existingFlowVariable, $flowElement, 'identity');
        }

        // TODO: we should make a connection of type 'assignment' here
        addFlowConnection($flowAssign, $flowElement);
        
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
            $functionCallFlowElement->sendsChangesToOutside = false;
            $functionCallFlowElement->receivesChangesFromOutside = false;

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
