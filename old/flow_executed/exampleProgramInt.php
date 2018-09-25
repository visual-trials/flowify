<?php

require_once("Flow.php");

// old program:

$oldValue1 = 2342;
$oldValue2 = $oldValue1 + 98;
$oldValue3 = $oldValue2 - 3874;

// new program:

$newValue1 = new FlowInt(2342);
$newValue2 = $newValue1->add(98);
$newValue3 = $newValue2->subtract(3874);

// output:

$output = array();
$output['oldProgram'] = $oldValue3;
$output['newProgram'] = $newValue3->getHistory();
$output['tree'] = FlowElement::getRootElement()->getContainerTree();

echo json_encode($output);
