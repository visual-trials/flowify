<?php

// Helper functions

function addFlowConnection ($fromFlowElement, $toFlowElement, $connectionType = null) {
    global $flowConnections, $flowConnectionId;

    $flowConnection = new FlowConnection;
    $flowConnection->id = $flowConnectionId;
    $flowConnection->from = $fromFlowElement->id;
    $flowConnection->to = $toFlowElement->id;
    $flowConnection->type = $connectionType;
    
    // This is used when we want to add inout-connection to it later, but don't know what connectionType 
    // we should use (for example: a join-variable in a for-loop that is later connected via the back-body)
    // TODO: only if the current connectionType if *stronger* than the outputConnectionType we should overwrite it
    $fromFlowElement->outputConnectionType = $connectionType; 
    
    $flowConnections[$flowConnectionId] = $flowConnection;

    $flowConnectionId++;
    
    return $flowConnection->id;
}

function getParentElement($flowElement) {
    
    global $flowElements;
    
    $parentFlowElementId = $flowElement->parentId;
    if ($parentFlowElementId !== null) {
        return $flowElements[$parentFlowElementId];
    }
    else {
        return null;
    }
}

function getExitingParentElement($flowElement) {
    
    global $flowElements;
    
    $exitingParentFlowElementId = $flowElement->exitingParentId;
    if ($exitingParentFlowElementId !== null) {
        return $flowElements[$exitingParentFlowElementId];
    }
    else {
        return null;
    }
}

function createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = true, $hasScope = true) {

    global $flowElements, $flowElementId;
    
    // TODO: set canContainSplitters with a parameter?

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
        
        $flowElement->hasOpenEndings = false;
        $flowElement->onlyHasOpenEndings = false;
        $flowElement->openEndings = new OpenEndings; // TODO: maybe not create this always?
        
        $flowElement->parentId = null;
        
        $flowElement->canJoin = false;
        $flowElement->canSplit = false;
        $flowElement->previousId = null; // used if cannot join
        $flowElement->previousIds = []; // used if can join
        
        $flowElement->addPassthroughIfVariableNotChanged = false;
        $flowElement->addPassbackIfVariableNotChanged = false;
    
        $flowElement->canHaveChildren = $canHaveChildren;
        if ($canHaveChildren) {
            $flowElement->children = [];
        }
        
        if ($hasScope) {
            $flowElement->varsInScopeChanged = [];
            $flowElement->varsInScopeAvailable = [];
            $flowElement->functionsInScope = [];
            
            $flowElement->sendsChangesToOutside = true;
            $flowElement->receivesChangesFromOutside = true;
        }
        
        $flowElements[$flowElementId] = $flowElement;
        
        $flowElementId++;
    // }

    return $flowElement;
}

function addFlowElementToParent ($flowElement, $parentFlowElement) {
    // Setting the previousElement of the child to the lastChild of the parent, but leaving it null if there are no children yet
    if (count($parentFlowElement->children) > 0) {
        $flowElement->previousId = end($parentFlowElement->children)->id;
        reset($parentFlowElement->children);
    }
    // Adding it to the 'children' array
    array_push($parentFlowElement->children, $flowElement);
    // Setting the lastChild of the parent to the new child
    $parentFlowElement->lastChildId = $flowElement->id;
    // Setting the parent of the child to the parent
    $flowElement->parentId = $parentFlowElement->id;
    // By default the exitingParent is the parent
    $flowElement->exitingParentId = $parentFlowElement->id;
}

function createAndAddFlowElementToParent ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $parentFlowElement, $useVarScopeFromParent = true) {
    
    $flowElement = createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = true, $hasScope = true);
    
    
    if ($useVarScopeFromParent) { // FIXME: deprecated!
        // Copying the varsInScopeAvailable from the parent to the child (if child is not childless)
        $flowElement->varsInScopeAvailable = $parentFlowElement->varsInScopeAvailable; // copy!
        
        // OLD: $flowElement->varsInScope = $parentFlowElement->varsInScope; // FIXME: deprecated!
        $flowElement->functionsInScope = &$parentFlowElement->functionsInScope;
    }
    addFlowElementToParent($flowElement, $parentFlowElement);

    return $flowElement;
}

function addChangedVariablesToExitingParent ($flowElement) {
    $exitingParentFlowElement = getExitingParentElement($flowElement);
    if ($flowElement->varsInScopeChanged === null) {
        logLine("empty varsInScopeChanged for flowElement: " . $flowElement->id, $isError = true);
    }
    foreach ($flowElement->varsInScopeChanged as $variableName => $isChanged) {
        // We are adding all changed variables to the exitingParent, *if* they were available in that exitingParent
        if (array_key_exists($variableName, $exitingParentFlowElement->varsInScopeAvailable)) {
            $exitingParentFlowElement->varsInScopeChanged[$variableName] = true;
        }
    }
}

function createAndAddChildlessFlowElementToParent ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $parentFlowElement) {

    $flowElement = createFlowElement ($flowElementType, $flowElementName, $flowElementValue, $astNodeIdentifier, $canHaveChildren = false, $hasScope = false);
    addFlowElementToParent($flowElement, $parentFlowElement);

    return $flowElement;
}

function setVarsInScopeAvailableRecursively($flowElement, $variableName) {
    if ($flowElement->canHaveChildren) {
        $flowElement->varsInScopeAvailable[$variableName] = true;
        if ($flowElement->children !== null) { // TODO: not needed right, since canHaveChildren is true?
            foreach ($flowElement->children as $childFlowElement) {
                if ($childFlowElement->receivesChangesFromOutside) {
                    setVarsInScopeAvailableRecursively($childFlowElement, $variableName);
                }
            }
        }
    }
}

function findJoinVariableInsideLane ($laneElement, $variableName) {
    // TODO: workaround: for now we simply loop through all children of the condBody and check
    //        if it is a join-variable with this variableName. There is probably a nicer way of doing this.
    $joinVariable = null;
    foreach ($laneElement->children as $childElement) {
        if (!$childElement->canHaveChildren && 
            $childElement->type === 'conditionalJoinVariable' &&
            $childElement->isVariable === $variableName) {
                
            $joinVariable = $childElement;
            break;
        }
    }
    return $joinVariable;
}

function findContainingFunctionOrRoot($flowElement) {
    if ($flowElement->sendsChangesToOutside && $flowElement->parentId !== null) {
        return findContainingFunctionOrRoot(getParentElement($flowElement));
    }
    else {
        return $flowElement;
    }
}

function createVariable($parentFlowElement, $variableName, $astNodeIdentifier, $variableType = 'variable', $createDirectlyInParent = true) {
    
    // We insert the variable inside the containing function
    
    // Question: is it possible in this language that a variable is declared inside an for-loop? (in the init-expression)
    //           or is this only possible in block-scoped languages?
    
    $functionOrRoot = findContainingFunctionOrRoot($parentFlowElement);
    $elementToCreateVariableIn = null;
    if ($createDirectlyInParent) {
        $elementToCreateVariableIn = $parentFlowElement;
    }
    else {
        $elementToCreateVariableIn = $functionOrRoot;
    }
    $flowElement = createAndAddChildlessFlowElementToParent($variableType, $variableName, null, $astNodeIdentifier, $elementToCreateVariableIn);
    $flowElement->isVariable = $variableName;
    
    // The varsInScopeChanged is set for the parent
    $parentFlowElement->varsInScopeChanged[$variableName] = true;
    
    // The varsInScopeAvailable is set for all elements inside the function/program.
    setVarsInScopeAvailableRecursively($functionOrRoot, $variableName);
    
    return $flowElement;
}


function getConnectionById ($connectionId) {
    global $flowConnections;
    
    return $flowConnections[$connectionId];
}

function containsSomeOpenEndings ($openEndings) {
    
    if (count($openEndings->returns) > 0 ||
        count($openEndings->continues) > 0 ||
        count($openEndings->breaks) > 0
    ) {
        return true;
    }
    else {
        return false;
    }
}

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
    
    public $astNodeIdentifier;
    
    public $canJoin;
    public $canSplit;
    public $previousId;
    public $previousIds;
    
    public $addPassthroughIfVariableNotChanged;
    public $addPassbackIfVariableNotChanged;
    
    public $canHaveChildren;
    public $children;
    public $lastChildId;
    
    public $parentId;
    public $exitingParentId;
    
    public $functionsInScope;
    
    public $varsInScopeAvailable;
    public $varsInScopeChanged;
    
    public $isVariable;
    public $outputConnectionType;
    
    public $sendsChangesToOutside;
    public $receivesChangesFromOutside;
    
    public $hasOpenEndings;
    public $onlyHasOpenEndings;
    public $openEndings;
    
    public $endsWith; // null, 'continue', 'break', 'return'
    public $returnVar;
}

class FlowConnection { 
    public $id;
    public $from;
    public $to; 
    public $type; 
}

function getFlowDataFromElement ($flowElement) {
    $flowData = "";
    
    $previousIds = null;
    if ($flowElement->canJoin) {
        $previousIds = implode(',' , $flowElement->previousIds);
    }
    else {
        $previousIds = $flowElement->previousId;
    }
    $flowData .= 'previous: ' . $previousIds . "\n";
    $flowData .= 'lastchild: ' . $flowElement->lastChildId . "\n";
    $flowData .= 'parent: ' . $flowElement->parentId . "\n";
    $flowData .= 'exitingParent: ' . $flowElement->exitingParentId . "\n";
    $canSplit = $flowElement->canSplit ? 'true' : 'false';
    $flowData .= 'canSplit: ' . $canSplit . "\n";
    $canJoin = $flowElement->canJoin ? 'true' : 'false';
    $flowData .= 'canJoin: ' . $canJoin . "\n";
    $flowData .= "\n";
    
    if ($flowElement->canHaveChildren) { // FIXME: technically we should check canHaveScope here (but right now it amounts to the same thing)
        $varsInScopeAvailable = "";
        if (!empty($flowElement->varsInScopeAvailable)) {
            $varsInScopeAvailable = implode(',' , array_keys($flowElement->varsInScopeAvailable));
        }
        $flowData .= 'varsInScopeAvailable: ' . $varsInScopeAvailable . "\n";
        
        $varsInScopeChanged = "";
        if (!empty($flowElement->varsInScopeChanged)) {
            $varsInScopeChanged = implode(',' , array_keys($flowElement->varsInScopeChanged));
        }
        $flowData .= 'varsInScopeChanged: ' . $varsInScopeChanged . "\n";
        $flowData .= "\n";
        
        $flowData .= 'endsWith: ' . $flowElement->endsWith . "\n";
        // FIXME: deprecated? $hasOpenEndings = $flowElement->hasOpenEndings ? 'true' : 'false';
        // FIXME: deprecated? $flowData .= 'hasOpenEndings: ' . $hasOpenEndings . "\n";
        $containsSomeOpenEndings = containsSomeOpenEndings($flowElement->openEndings);
        $containsSomeOpenEndings = $containsSomeOpenEndings ? 'true' : 'false';
        $flowData .= 'containsSomeOpenEndings: ' . $containsSomeOpenEndings . "\n";
        $onlyHasOpenEndings = $flowElement->onlyHasOpenEndings ? 'true' : 'false';
        $flowData .= 'onlyHasOpenEndings: ' . $onlyHasOpenEndings . "\n";
/*        
        $openEndings = $flowElement->openEndings;
        $openEndReturns = implode(',' , array_keys($openEndings->returns));
        $openEndBreaks = implode(',' , array_keys($openEndings->breaks));
        $openEndContinues = implode(',' , array_keys($openEndings->continues));
        $flowData .= 'openEndReturns: ' . $openEndReturns . "\n";
        $flowData .= 'openEndBreaks: ' . $openEndBreaks . "\n";
        $flowData .= 'openEndContinues: ' . $openEndContinues . "\n";
        */
    }
    else {
        $flowData .= 'isVariable: ' . $flowElement->isVariable . "\n";
        $flowData .= "\n";
    }
    
    return $flowData;
}

function arrayfyFlowElements ($flowElement) {
    
    $flowElementArray = [];
    
    $flowElementArray['id'] = $flowElement->id;
    $flowElementArray['type'] = $flowElement->type;
    $flowElementArray['name'] = $flowElement->name;
    $flowElementArray['value'] = $flowElement->value;
    $flowElementArray['astNodeIdentifier'] = $flowElement->astNodeIdentifier;
    $flowElementArray['flowData'] = getFlowDataFromElement($flowElement);
    
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

function enableLogging() {
    global $loggingEnabled;
    $loggingEnabled = true;
}

function disableLogging() {
    global $loggingEnabled;
    $loggingEnabled = false;
}

function logLine($logLine, $isError = false) {
    global $logLines, $loggingEnabled;

    if ($loggingEnabled || $isError) {
        $logLines[] = $logLine;
    }
}
