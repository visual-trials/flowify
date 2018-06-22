<?php

require_once("Flow.php");

// new program:

$resultDoNumbers = doNumbers();

$newValue1 = $resultDoNumbers['newValue1'];
$newValue2 = $resultDoNumbers['newValue2'];
$newValue3 = $resultDoNumbers['newValue3'];

// new list program:

$resultDoAddToList = doAddToList($newValue1, $newValue2, $newValue3);

$newListValue1 = $resultDoAddToList['newListValue1'];
$newListValue2 = $resultDoAddToList['newListValue2'];
$newListValue3 = $resultDoAddToList['newListValue3'];
$newListValue4 = $resultDoAddToList['newListValue4'];

// output:

$output = array();
$output['newProgram'] = $newListValue4->getHistory();
$output['tree'] = FlowElement::getRootElement()->getContainerTree();

echo json_encode($output);


function doNumbers() {
    $newFunction1 = new FlowFunction('Doing numbers');

    $newValue1 = new FlowInt(2342);
    $newValue2 = $newValue1->add(98);
    $newValue3 = $newValue2->subtract(3874);

    return array(
        'newValue1' => $newValue1,
        'newValue2' => $newValue2,
        'newValue3' => $newValue3,
    );
}

function doAddToList ($newValue1, $newValue2, $newValue3) {
    $newFunction2 = new FlowFunction('Adding to List');

    $newValue4 = new FlowInt(-876);

    $newListValue1 = new FlowMap();
    $newListValue2 = $newListValue1->push($newValue2);
    $newListValue3 = $newListValue2->push($newValue3);
    $newListValue4 = $newListValue3->push($newValue4);

    return array(
        'newListValue1' => $newListValue1,
        'newListValue2' => $newListValue2,
        'newListValue3' => $newListValue3,
        'newListValue4' => $newListValue4,
    );
}