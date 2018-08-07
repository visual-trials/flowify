<?php

require "astFromPhpCode.php";
require "coupleVisualInfoToAst.php";
require "flowifyHelper.php";
require "flowifyStatements.php";
require "flowifyExpressions.php";

$flowElementId = 0;
$flowElements = [];
$flowConnectionId = 0;
$flowConnections = [];
$code = null;
$logLines = [];

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
    global $flowConnections, $code, $logLines;

    list($code, $visualInfos) = updateAndGetCodeAndVisualInfoForFile($fileToFlowifyWithoutExtention);
    
    $statements = getAstFromPhpCode($code);

    $rootFlowElement = flowifyProgram($statements);
    // stripAllButUsedVars($rootFlowElement);
    // $rootFlowElement = flowifyProgram($statements);
    
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
    $flowifiedPhp['logLines'] = $logLines;
    $flowifiedPhp['code'] = explode("\n", $code);;
    $flowifiedPhp['rootFlowElement'] = $rootFlowElementArray;
    $flowifiedPhp['flowConnections'] = $flowConnectionsArray;
    $flowifiedPhp['statements'] = $statements;
    $flowifiedPhp['visualInfos'] = $visualInfos;
    $flowifiedPhp['usedVisualInfos'] = $usedVisualInfos;

    return $flowifiedPhp;
}

