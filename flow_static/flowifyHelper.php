<?php

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
    $flowElement->doPassBack = false; // this is used for for-loops
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
    public $endsWith; // null, 'none', 'continue', 'break', 'return'
    public $returnVar;
}

class FlowConnection { 
    public $id;
    public $from;
    public $to; 
    public $type; 
}

function stringPosToColumn($code, $pos) {
    if ($pos > strlen($code)) {
        die("Invalid position information: $pos\n");
    }
    $lineStartPos = strrpos($code, "\n", $pos - strlen($code));
    if ($lineStartPos === false) {
        $lineStartPos = -1;
    }
    return $pos - $lineStartPos;
}

function stringPosToLineNumber($code, $pos) {
    if ($pos > strlen($code)) {
        die("Invalid position information: $pos\n");
    }
    $lineStartPos = strrpos($code, "\n", $pos - strlen($code));
    if ($lineStartPos === false) {
        $lineStartPos = 0; // if no newlines can be found, we should be on the first line, so we simply set the position to 0
    }

    // Note: we do '$lineStartPos + 1' here because we want to include the newline of the current line (so we need to do pos + 1 = count)
    // Note2: we do + 1 at the end because we want to start counting with lineNumber 1
    $lineNumber = substr_count($code, "\n", 0 , $lineStartPos + 1) + 1;

    return $lineNumber;
}