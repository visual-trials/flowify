<?php

// VisualInfo

function extendFlowElementsWithVisualInfo (&$flowElementArray, &$visualInfos, &$usedVisualInfos) {
    
    $visualInfo = getVisualInfo($flowElementArray['astNodeIdentifier'], $visualInfos, $usedVisualInfos);
    extendFlowElementWithVisualInfo($flowElementArray, $visualInfo);
    
    if (array_key_exists('children', $flowElementArray)) {
        foreach ($flowElementArray['children'] as &$childflowElementArray) {
            extendFlowElementsWithVisualInfo($childflowElementArray, $visualInfos, $usedVisualInfos);
        }
    }
}

function extendFlowElementWithVisualInfo (&$flowElementArray, $visualInfo) {

    // TODO: maybe set only certain visual attributes given certain FlowElement types? Or simply check if the attributes exists and/or is null?

    if ($visualInfo === null) {
        return;
    }

    if (array_key_exists('x', $visualInfo) && $visualInfo['x'] !== null) {
        $flowElementArray['x'] = $visualInfo['x'];
    }
    if (array_key_exists('y', $visualInfo) && $visualInfo['y'] !== null) {
        $flowElementArray['y'] = $visualInfo['y'];
    }
    if (array_key_exists('isPositionOf', $visualInfo) && $visualInfo['isPositionOf'] !== null) {
        $flowElementArray['isPositionOf'] = $visualInfo['isPositionOf'];
    }
    if (array_key_exists('width', $visualInfo) && $visualInfo['width'] !== null) {
        $flowElementArray['width'] = $visualInfo['width'];
    }
    if (array_key_exists('height', $visualInfo) && $visualInfo['height'] !== null) {
        $flowElementArray['height'] = $visualInfo['height'];
    }
    if (array_key_exists('relativeScale', $visualInfo) && $visualInfo['relativeScale'] !== null) {
        $flowElementArray['relativeScale'] = $visualInfo['relativeScale'];
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

// AttachInfoToAst

function updateAndGetCodeAndVisualInfoForFile($fileToFlowifyWithoutExtention) {
    
    // TODO: the order of the coordinates is now by AST-order, not by horizontal position in the line

    // FIXME: check if php file has been read correctly!
    $oldCode = file_get_contents($fileToFlowifyWithoutExtention . '.bck');
    if ($oldCode === false) {
        $oldCode = "";
    }
    $newCode = file_get_contents($fileToFlowifyWithoutExtention . '.php');
    if ($newCode === false) {
        // FIXME: give a proper error (through the api) that the file does not exist!
        exit("The file '{$fileToFlowifyWithoutExtention}.php' does not exist!");
    }

    $code = $oldCode;
    $visualInfosJSON = file_get_contents($fileToFlowifyWithoutExtention . '.viz');
    $visualInfos = array();
    if ($visualInfosJSON !== false) {
        $visualInfos = json_decode($visualInfosJSON, true);  // FIXME: what if not valid json?
    }

    if ($oldCode != $newCode) {

        $path = doDiff($oldCode, $newCode);
        $oldToNewPositions = getOldToNewPositions($path, $oldCode, $newCode);

        $newVisualInfos = getVisualInfosForNewPositions($visualInfos, $oldToNewPositions);

        if (count($newVisualInfos) == count($visualInfos)) {
            // If all new positions could be mapped, then we want to copy the php-file over the bck-file and store the new .viz file.

            // FIXME: also check if the new positions line up with the new AST-elements!

            // FIXME: check if this goes right!
            storeBackupFile ($newCode, $fileToFlowifyWithoutExtention);

            // FIXME: check if this goes right!
            storeVisualInfos($newVisualInfos, $fileToFlowifyWithoutExtention);

        }
        else {
            // FIXME: signal to the front-end that we couldn't store the info!
        }

        // In all cases we want to SHOW the new code and visualInfos
        $code = $newCode;
        $visualInfos = $newVisualInfos;
    }
    
    return array($code, $visualInfos);
    
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

function getVisualInfosForNewPositions($visualInfos, $oldToNewPositions) {
    $newVisualInfos = [];

    foreach ($visualInfos as $astNodeIdentifier => $visualInfo) {
        $positionsAndPostFix = explode("_", $astNodeIdentifier);
        $positions = $positionsAndPostFix[0];
        $postFix = null;
        if (count($positionsAndPostFix) > 1) {
            $postFix = implode('_', array_shift($positionsAndPostFix));
        }        
        $fromToParts = explode("-", $positionsAndPostFix[0]);

        $fromPosition = $fromToParts[0];
        $toPosition = $fromToParts[1];

        if (array_key_exists($fromPosition, $oldToNewPositions) && array_key_exists($toPosition, $oldToNewPositions)) {
            $newFromPosition = $oldToNewPositions[$fromPosition];
            $newToPosition = $oldToNewPositions[$toPosition];

            $newAstNodeIdentifier = $newFromPosition . "-" . $newToPosition;
            if ($postFix !== null) {
                $newAstNodeIdentifier . "_" . $postFix;
            }
            $newVisualInfos[$newAstNodeIdentifier] = $visualInfo;
        }
        else {
            // TODO: Could not find the appropriate new position. What to do here?
        }
    }

    return $newVisualInfos;
}


// Diffing

function getOldToNewPositions ($path, $code, $newCode) {

    $oldToNewPositions = [];

    // FIXME: the 0:0 position is a bit of a HACK!
    $oldToNewPositions["0:0"] = "0:0";

    foreach ($path as $stepInfo) {
        if ($stepInfo['posX'] === null) {
            // this is a new character. so nothing to map from old to new
        }
        else {
            $oldLineNumber = stringPosToLineNumber($code, $stepInfo['posX']);
            $oldColumn = stringPosToColumn($code, $stepInfo['posX']);
            $newLineNumber = stringPosToLineNumber($newCode, $stepInfo['posY']);
            $newColumn = stringPosToColumn($newCode, $stepInfo['posY']);
            $oldToNewPositions[$oldLineNumber.':'.$oldColumn] = $newLineNumber.':'.$newColumn;
        }
    }

    return $oldToNewPositions;

}

function generateDisplayablePath ($path) {
    $displayablePath = "";

    foreach ($path as $stepInfo) {
        $displayablePath .= $stepInfo['charX'] === null ? ' ' : $stepInfo['charX'];
    }
    $displayablePath .= "\n";
    foreach ($path as $stepInfo) {
        $displayablePath .= $stepInfo['direction'];
    }
    $displayablePath .= "\n";
    foreach ($path as $stepInfo) {
        $displayablePath .= $stepInfo['charY'] === null ? ' ' : $stepInfo['charY'];
    }
    $displayablePath .= "\n";

    return $displayablePath;
}

function doDiff($firstString, $secondString) {

    $stringX = $firstString;
    $stringY = $secondString;

    $lengthX = strlen($stringX);
    $lengthY = strlen($stringY);

    $foundBestSteps = [];
    $foundBestSteps[$lengthX.':'.$lengthY] = [ 'edits' => 0 ];

    $bestStep = findBestNextStep(0, 0, $stringX, $stringY, $lengthX, $lengthY, $foundBestSteps);

    $path = createPathFromBestSteps(0, 0, $foundBestSteps);

    return $path;

}

function createPathFromBestSteps ($posX, $posY, &$foundBestSteps, $path = []) {

    $bestStep = $foundBestSteps[$posX.':'.$posY];

    if (array_key_exists('direction', $bestStep)) {  // the final 'step' doesn't have a direction, because we reached the end
        $path[] = $bestStep;
        $direction = $bestStep['direction'];
        if ($direction === '=') {
            $path = createPathFromBestSteps($posX + 1, $posY + 1, $foundBestSteps, $path);
        }
        else if ($direction === '+') {
            $path = createPathFromBestSteps($posX, $posY + 1, $foundBestSteps, $path);
        }
        else if ($direction === '-') {
            $path = createPathFromBestSteps($posX + 1, $posY, $foundBestSteps, $path);
        }
        else {
            die("Unknown direction: '$direction''");
        }
    }

    return $path;
}

function chooseBestStep ($bestStepSoFar, $nextStep, $stepInfo, $nrOfEditsToAdd) {

    $bestStep = $bestStepSoFar;
    if ($nextStep !== null) {
        if ($bestStepSoFar === null) {
            $bestStep = $stepInfo;
            $bestStep['edits'] = $nextStep['edits'] + $nrOfEditsToAdd;
        }
        else {
            if ($nextStep['edits'] + $nrOfEditsToAdd < $bestStepSoFar['edits']) {
                $bestStep = $stepInfo;
                $bestStep['edits'] = $nextStep['edits'] + $nrOfEditsToAdd;
            }
        }
    }
    return $bestStep;
}

function findBestNextStep($posX, $posY, &$stringX, &$stringY, $lengthX, $lengthY, &$foundBestSteps) {

    if (array_key_exists($posX.':'.$posY, $foundBestSteps)) {
        $bestStep = &$foundBestSteps[$posX.':'.$posY];
        return $bestStep;
    }

    if ($posX > $lengthX) return null;
    if ($posY > $lengthY) return null;

    // FIXME: these could be out of bound!
    $xChar = substr($stringX, $posX, 1);
    $yChar = substr($stringY, $posY, 1);

    $stepInfo = [];

    $bestStep = null;
    if ($xChar == $yChar) {  // diagonal
        $bestNextStep = findBestNextStep($posX + 1, $posY + 1, $stringX, $stringY, $lengthX, $lengthY, $foundBestSteps);
        $stepInfo['direction'] = '=';
        $stepInfo['posX'] = $posX;
        $stepInfo['posY'] = $posY;
        $stepInfo['charX'] = $xChar;
        $stepInfo['charY'] = $yChar;
        $bestStep = chooseBestStep($bestStep, $bestNextStep, $stepInfo, 0);
    }

    $bestNextStep = findBestNextStep($posX + 1, $posY, $stringX, $stringY, $lengthX, $lengthY, $foundBestSteps);  // right
    $stepInfo['direction'] = '-';
    $stepInfo['posX'] = $posX;
    $stepInfo['posY'] = null;
    $stepInfo['charX'] = $xChar;
    $stepInfo['charY'] = null;
    $bestStep = chooseBestStep($bestStep, $bestNextStep, $stepInfo, 1);

    $bestNextStep = findBestNextStep($posX, $posY + 1, $stringX, $stringY, $lengthX, $lengthY, $foundBestSteps);  // down
    $stepInfo['direction'] = '+';
    $stepInfo['posX'] = null;
    $stepInfo['posY'] = $posY;
    $stepInfo['charX'] = null;
    $stepInfo['charY'] = $yChar;
    $bestStep = chooseBestStep($bestStep, $bestNextStep, $stepInfo, 1);

    if ($bestStep !== null) {
        $foundBestSteps[$posX.':'.$posY] = $bestStep;
    }

    if (count($foundBestSteps) > 400000) {
        die("Too many foundPaths!!". count($foundBestSteps));
    }

    return $bestStep;
}

function is_assoc(array $array)
{
    if ([] === $array) return false;
    return array_keys($array) !== range(0, count($array) - 1);
}

