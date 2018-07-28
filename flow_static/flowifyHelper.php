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
    $flowElement->parentElement = $parentFlowElement;
}

function createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = true, $hasScope = true) {

    global $flowElements, $flowElementId;

    /*
    if (array_key_exists($astNodeIdentifier, $flowElements)) {
        // The element already exists (identified by astNodeIdentifier), so we return the existing flowElement
        $flowElement = $flowElements[$astNodeIdentifier];
    }
    */
    // else {
        $flowElement = new FlowElement;
        
        $flowElement->id = $flowElementId;
        $flowElement->astNodeIdentifier = $astNodeIdentifier;
        $flowElement->type = $flowElementType;
        $flowElement->name = $flowElementName;
        $flowElement->value = $flowElementValue;
        
        // $flowElement->usedVars = [];
        
        $flowElement->connectionIdsFromThisElement = [];
        $flowElement->doPassBack = false; // this is used for for-loops
        $flowElement->onlyHasOpenEndings = false;
        
        $flowElement->parentElement = null;
        if ($canHaveChildren) {
            $flowElement->children = [];
        }
        if ($hasScope) {
            $flowElement->varsInScope = [];
            $flowElement->functionsInScope = [];
        }
        
        // $flowElements[$astNodeIdentifier] = $flowElement;
        
        $flowElementId++;
    // }

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
    
    if ($flowElement->children !== null) {
        foreach ($flowElement->children as $childFlowElement) {
            // TODO: we should not go into a function when we are trying to find connection to elements inside a flowElement.
            //       This is because a function(call) reset its varsInScope, so no variables *outside* its scope could 
            //       connect (directly) with one of its children.
            $containerIdsInChildFlowElement = getElementsIdsIn($childFlowElement);
            $containerIdsInFlowElement = array_merge($containerIdsInFlowElement, $containerIdsInChildFlowElement);
        }
    }
    return $containerIdsInFlowElement;
}

/*
function setUsedVar ($flowElement, $variableName, $createdOrUsed) {
    if (array_key_exists($variableName, $flowElement->usedVars)) {
        $existingCreatedOrUsed = $flowElement->usedVars[$variableName];
        if ($existingCreatedOrUsed === 'created') {
            // the existing usedVar is already set to 'created', we don't overwrite it
        }
        else {
            // the existing usedVar is not already set to 'created', so we overwrite it
            $flowElement->usedVars[$variableName] = $createdOrUsed;
        }
    }
    else {
        $flowElement->usedVars[$variableName] = $createdOrUsed;
    }
}

function addUsedVarsToParent ($flowElement, $parentFlowElement) {
    // $parentFlowElement->usedVars = array_merge($flowElement->usedVars, $parentFlowElement->usedVars);
    foreach ($flowElement->usedVars as $variableName => $createdOrUsedInChild) {
        $createdOrUsedInParent = null;
        if (array_key_exists($variableName, $parentFlowElement->usedVars)) {
            $createdOrUsedInParent = $parentFlowElement->usedVars[$variableName];
        }
        
        if ($createdOrUsedInChild === 'not-yet-created') {
            // if the variable was not-yet-created in the child, so it was also not-yet-created in the parent
            $parentFlowElement->usedVars[$variableName] = 'not-yet-created';
        }
        else if ($createdOrUsedInChild === 'created') {
            // if the variable was created in the child, it was not-yet-created in the parent
            $parentFlowElement->usedVars[$variableName] = 'not-yet-created';
        }
        else if ($createdOrUsedInParent === 'not-yet-created') {
            // if the variable was not-yet-created in the parent, we don't add it to the usedVars of the parent (note: it might have been 'used' in this child, but created by another child)
        }
        else if ($createdOrUsedInParent === 'created') {
            // if the variable was created in the parent, we don't add it to the usedVars of the parent
        }
        else {
            // in all other cases (it was used in the child and either not present in the parent, or already used in the parent) we add it to the parent
            $parentFlowElement->usedVars[$variableName] = 'used';
        }
    }
}

function stripAllButUsedVars($flowElement) {
    
    // Note: properties we don't strip: id, type, name, value, astNodeIdentifier, usedVars

    if (!is_null($flowElement->varsInScope)) {
        $flowElement->varsInScope = [];
    }
    if (!is_null($flowElement->functionsInScope)) {
        $flowElement->functionsInScope = [];
    }
    
    $flowElement->connectionIdsFromThisElement = [];
    $flowElement->doPassBack = false;
    $flowElement->onlyHasOpenEndings = false;
    
    if (!is_null($flowElement->children)) {
        foreach ($flowElement->children as $childFlowElement) {
            stripAllButUsedVars($childFlowElement);
        }
        $flowElement->children = [];
    }
    
}
*/

function combineOpenEndings($newOpenEndings, $openEndings) {

    foreach ($newOpenEndings->returns as $elementId => $returnOpenEndingElement) {
        $openEndings->returns[$elementId] = $returnOpenEndingElement;
    }
    foreach ($newOpenEndings->continues as $elementId => $continueOpenEndingElement) {
        $openEndings->continues[$elementId] = $continueOpenEndingElement;
    }
    foreach ($newOpenEndings->breaks as $elementId => $breakOpenEndingElement) {
        $openEndings->breaks[$elementId] = $breakOpenEndingElement;
    }
    
    return $openEndings;
}

function addElementToOpenEndings($flowElement, $openEndings, $openEndingType, $returnFlowElement = null) {

    // TODO: we should use 'id' as identifier here! but that isnt possible right now because its numeric
    //       and php won't treat it as proper keys!
    $elementId = 'id:' . $flowElement->id;
    
    if ($openEndingType === 'return') {
        $flowElement->endsWith = 'return';
        $flowElement->returnVar = $returnFlowElement;
        $flowElement->onlyHasOpenEndings = true;
        $openEndings->returns[$elementId] = $flowElement;
    }
    else if ($openEndingType === 'continue') {
        $flowElement->endsWith = 'continue';
        $flowElement->onlyHasOpenEndings = true;
        $openEndings->continues[$elementId] = $flowElement;
    }
    else if ($openEndingType === 'break') {
        $flowElement->endsWith = 'break';
        $flowElement->onlyHasOpenEndings = true;
        $openEndings->breaks[$elementId] = $flowElement;
    }
}    

class OpenEndings {
    public $returns;
    public $continues;
    public $breaks;
    
    public function __construct() {
        $this->returns = [];
        $this->continues = [];
        $this->breaks = [];
    }
}

class FlowElement { 
    public $id;
    public $type;
    public $name; 
    public $value; 
    public $connectionIdsFromThisElement;
    public $parentElement;
    public $children;
    public $astNodeIdentifier;
    // public $usedVars;
    public $varsInScope;
    public $functionsInScope;
    public $onlyHasOpenEndings;
    public $endsWith; // null, 'continue', 'break', 'return'
    public $returnVar;
}

class FlowConnection { 
    public $id;
    public $from;
    public $to; 
    public $type; 
}


function arrayfyFlowElements ($flowElement) {
    
    $flowElementArray = [];
    
    $flowElementArray['id'] = $flowElement->id;
    $flowElementArray['type'] = $flowElement->type;
    $flowElementArray['name'] = $flowElement->name;
    $flowElementArray['value'] = $flowElement->value;
    $flowElementArray['astNodeIdentifier'] = $flowElement->astNodeIdentifier;
    
    // FIXME: added for debugging only!
    $flowElementArray['usedVars'] = $flowElement->usedVars;
    
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
