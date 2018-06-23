<?php

function getVisualInfosForNewPositions($visualInfos, $oldToNewPositions) {
    $newVisualInfos = [];

    foreach ($visualInfos as $astNodeIdentifier => $visualInfo) {
        $fromToParts = explode("-", $astNodeIdentifier);

        $fromPosition = $fromToParts[0];
        $toPosition = $fromToParts[1];

        if (array_key_exists($fromPosition, $oldToNewPositions) && array_key_exists($toPosition, $oldToNewPositions)) {
            $newFromPosition = $oldToNewPositions[$fromPosition];
            $newToPosition = $oldToNewPositions[$toPosition];

            $newAstNodeIdentifier = $newFromPosition . "-" . $newToPosition;
            $newVisualInfos[$newAstNodeIdentifier] = $visualInfo;
        }
        else {
            // TODO: Could not find the appropriate new position. What to do here?
        }
    }

    return $newVisualInfos;
}

function getAstNodeIdentifier ($astNode) {

    global $code;

    $startLine = 0;
    $startingColumnNumber = 0;
    $endLine = 0;
    $endColumnNumber = 0;

    if ($astNode !== null) {
        if (array_key_exists('attributes', $astNode) && array_key_exists('startLine', $astNode['attributes'] )) {
            $startLine = $astNode['attributes']['startLine'];
            $endLine = $astNode['attributes']['endLine'];
            $startLineCheck = stringPosToLineNumber($code, $astNode['attributes']['startFilePos']);
            if ($startLine !== $startLineCheck) {
                die("startLine: '$startLine'' is not equals to startLineCheck: '$startLineCheck''");
            }
            $endLineCheck = stringPosToLineNumber($code, $astNode['attributes']['endFilePos']);
            if ($endLine !== $endLineCheck) {
                die("endLine: '$endLine'' is not equals to endLineCheck: '$endLineCheck''");
            }
            $startingColumnNumber = stringPosToColumn($code, $astNode['attributes']['startFilePos']);
            $endColumnNumber = stringPosToColumn($code, $astNode['attributes']['endFilePos']);
        }
        else {
            die("Could not find attributes in node!\n" . print_r($astNode, true));
        }
    }

    $astNodeIdentifier = $startLine . ':' . $startingColumnNumber . '-' . $endLine . ':' . $endColumnNumber;

    return $astNodeIdentifier;
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
