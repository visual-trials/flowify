<?php

require_once("Flow.php");

// Example: /* name: 'myInput', x: 124, y: 187, type: 'stock' */
// Code folding:
//<editor-fold desc=""> { myFunction = x:321, y:634 }, { $myFirstInputValue = x:288, y:338 }
// </editor-fold>

// Note the return-values and arguments should also be able to get a position (INSIDE the function). We could auto-generate this, but could also give them in the call.
// Optional: additionally give positions of return-values to position LOCAL variables (that get the return value).
// Aternatively: when positioning/showing local variables is important, always declare these empty local-variables first (so you can position them and they are shown separetely)

$createInputAndOutputContainers = true;

$myFirstInputValue = new FlowInt(134287527590, true, 50, 315, 'left-middle');
$mySecondInputValue = new FlowInt(983, true, 50, 115, 'left-middle');

$mySemiOutputValue = $mySecondInputValue->add($myFirstInputValue, $createInputAndOutputContainers, 366, 187, 'center', 653, 215, 'center');


$myThirdInputValue = new FlowInt(3, true, 663, 315, 'center');
$myMultiplyFunction = new FlowFunction('myMultiplyFunction', null, false);  // FIXME: hardcoded createInputAndOutputContainers to false here
$myMultiplyFunction->setCoords(1020, 287, 'center', 1450, 340, 'right-middle');  // FIXME: we want to make this 'setCoords' not needed anymore
$myOutputValue = $myMultiplyFunction->call($mySemiOutputValue, $myThirdInputValue);

/*
$myListValue = new FlowList();
$myNewList = $myListValue->push($myFirstInputValue, true);
*/

$myMapValue = new FlowMap(null, true, 50, 410, 'left-middle');

$myPutKey = new FlowInt(231, true, 50, 515, 'left-middle');
$myNewMap = $myMapValue->put($myPutKey, $myFirstInputValue, $createInputAndOutputContainers, 402, 505, 'center', 739, 525, 'center');

$myGetKey = new FlowInt(231, true, 620, 665, 'center');
$myValue = $myNewMap->get($myGetKey, $createInputAndOutputContainers, 1082, 647, 'center', 1450, 665, 'right-middle');

/*
$myPutFunction = new FlowFunction('myPutFunction', null,true);
$myOutputValue = $myPutFunction->call($myNewMap, $mySemiOutputValue);


function myPutFunction ($myFirstInputValue, $mySecondInputValue) {

    $myNewMap = $myFirstInputValue->put(5, $mySecondInputValue, true);

    return $myNewMap;
}
*/


// $myListValue = new FlowList();

// $myNewList = $myListValue->push($mySemiOutputValue);

// $myOutputValue = $mySemiOutputValue->substract($myFirstInputValue);

// Flow::doFor($listToLoopThrough, $loopFunction)
// Flow::doIf($ifClause, $thenFunction, $elseFunction)

// Flow::callFunction('myFunction');
// Flow::callFunction(function($args) { statements... ; return $x; });

// $myFunction = new FlowFunction('myFunction');
// $outputMap = /* pos f, width/height f */ /* pos v1 */ /* pos v2 */ /* pos ret */  $myFunction->call($myFirstInputValue, $mySecondInputValue); // @Incomplete: the positions of v1, v2 and return are relative to the function. So these should be defined INSIDE the function. For now we define them here

// $myFourthInputValue = /* pos v */ new FlowInt(9475);
// $myFourthInputValue2 = /* pos f */ /* pos v */ $myFourthInputValue->add(1);

// $outputMap = $outputMap->put('myThirdOutputKey', 1337); // pos f + pos k/v
// $outputMap = $outputMap->put('myFourthOutputKey', $myFourthInputValue2); // pos f + pos k/v


function myMultiplyFunction ($myFirstInputValue, $mySecondInputValue) {
    global $createInputAndOutputContainers;  // FIXME: this is kinda ugly

    $myMultiplyOutput = $myFirstInputValue->multiply($mySecondInputValue, $createInputAndOutputContainers);

    return $myMultiplyOutput;
}


/*
function myFunction ($myFirstInputValue, $mySecondInputValue) {
    $myOutputMap = new FlowMap(); // pos v

    // @Incomplete: support get on a FlowMap! $myInputMap->get

    $myOutputMap = $myOutputMap->put('myFirstOutputKey', $myFirstInputValue); // pos f + pos k/v
    $myOutputMap = $myOutputMap->put('mySecondtOutputKey', $mySecondInputValue);  // pos f + pos k/v

    // @Incomplete: make sure the Map itself becomes part of the history! (ie it is placed inside the function and it's child is part part of the history of the value that was "get-ted"

    return $myOutputMap;
}
*/

// output:

$output = array();
// $output['newProgram'] = $outputMap->getHistory();
//echo print_r(FlowElement::getRootElement(),true);
$output['containers'] = FlowElement::getRootElement()->getContainerTree();
$output['connections'] = FlowElement::getConnections();

echo json_encode($output);
