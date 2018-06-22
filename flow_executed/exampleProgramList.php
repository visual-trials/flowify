<?php

require_once("Flow.php");

// new program:

$newValue1 = new FlowInt(2342);
$newValue2 = $newValue1->add(98);
$newValue3 = $newValue2->subtract(3874);
$newValue4 = new FlowInt(-876);

$newListValue1 = new FlowMap();
$newListValue2 = $newListValue1->push($newValue2);
$newListValue3 = $newListValue2->push($newValue3);
$newListValue4 = $newListValue3->push($newValue4);

// output:

$output = array();
$output['oldProgram'] = $oldValue3;
$output['newProgram'] = $newListValue4->getHistory();
$output['tree'] = FlowElement::getRootElement()->getContainerTree();

echo json_encode($output);

