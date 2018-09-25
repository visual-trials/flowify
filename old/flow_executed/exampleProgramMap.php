<?php

require_once("Flow.php");

// new program:

$newValue1 = new FlowInt(2342);
$newValue2 = new FlowInt(947);
//$newValue2 = $newValue1->add(98);
//$newValue3 = $newValue2->subtract(3874);
$newValue3 = new FlowInt(-876);

$newListValue1 = new FlowList();
$newListValue1 = $newListValue1->push($newValue3);

$newMapValue1 = new FlowMap();
$newMapValue1 = $newMapValue1->put('myFirstKey', $newValue1);
$newMapValue1 = $newMapValue1->put('mySecondKey', $newValue2);
$newMapValue1 = $newMapValue1->put('myThirdKey', $newListValue1);


// output:

$output = array();
$output['newProgram'] = $newMapValue1->getHistory();
$output['tree'] = FlowElement::getRootElement()->getContainerTree();

echo json_encode($output);

