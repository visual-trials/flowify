<?php

function extendFlowElementsWithVisualInfo (&$flowElement, &$visualInfos, &$usedVisualInfos) {
    
    $visualInfo = getVisualInfo($flowElement['astNodeIdentifier'], $visualInfos, $usedVisualInfos);
    extendFlowElementWithVisualInfo($flowElement, $visualInfo);
    
    if (array_key_exists('children', $flowElement)) {
        foreach ($flowElement['children'] as &$childFlowElement) {
            extendFlowElementsWithVisualInfo($childFlowElement, $visualInfos, $usedVisualInfos);
        }
    }
}

function extendFlowElementWithVisualInfo (&$flowElement, $visualInfo) {

    // TODO: maybe set only certain visual attributes given certain FlowElement types? Or simply check if the attributes exists and/or is null?

    if ($visualInfo === null) {
        return;
    }

    if (array_key_exists('x', $visualInfo) && $visualInfo['x'] !== null) {
        $flowElement['x'] = $visualInfo['x'];
    }
    if (array_key_exists('y', $visualInfo) && $visualInfo['y'] !== null) {
        $flowElement['y'] = $visualInfo['y'];
    }
    if (array_key_exists('isPositionOf', $visualInfo) && $visualInfo['isPositionOf'] !== null) {
        $flowElement['isPositionOf'] = $visualInfo['isPositionOf'];
    }
    if (array_key_exists('width', $visualInfo) && $visualInfo['width'] !== null) {
        $flowElement['width'] = $visualInfo['width'];
    }
    if (array_key_exists('height', $visualInfo) && $visualInfo['height'] !== null) {
        $flowElement['height'] = $visualInfo['height'];
    }
    if (array_key_exists('relativeScale', $visualInfo) && $visualInfo['relativeScale'] !== null) {
        $flowElement['relativeScale'] = $visualInfo['relativeScale'];
    }

}

function getVisualInfo ($astNodeIdentifier, &$visualInfos, &$usedVisualInfos) {

    $currentVisualInfo = [];

    if (array_key_exists($astNodeIdentifier, $visualInfos)) {
        $currentVisualInfo = $visualInfos[$astNodeIdentifier];
        // FIXME: $usedVisualInfos should not be a global that changes it state this way!
        $usedVisualInfos[$astNodeIdentifier] = $currentVisualInfo;
    }

    return $currentVisualInfo;
}

function updateVisualInfos ($changedVisualInfos, $fileToFlowifyWithoutExtention) {

    $visualInfosJSON = file_get_contents($fileToFlowifyWithoutExtention . '.viz');
    $visualInfos = json_decode($visualInfosJSON, true);  // FIXME: what if not valid json?

    // FIXME: we should verify we are still using the same viz file that the POST is referring to!
    foreach ($changedVisualInfos as $astNodeIdentifier => $visualInfo) {
        $visualInfos[$astNodeIdentifier] = $visualInfo;
    }

    // FIXME: check if this goes right!
    storeVisualInfos($visualInfos, $fileToFlowifyWithoutExtention);
}

function storeVisualInfos ($visualInfos, $fileToFlowifyWithoutExtention) {
    // FIXME: check if this goes right!
    file_put_contents($fileToFlowifyWithoutExtention . '.viz', json_encode($visualInfos, JSON_PRETTY_PRINT));
}

function storeBackupFile ($code, $fileToFlowifyWithoutExtention) {
    // FIXME: check if this goes right!
    file_put_contents($fileToFlowifyWithoutExtention . '.bck', $code);
}

