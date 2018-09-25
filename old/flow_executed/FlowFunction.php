<?php

/**
 *  Copyright © 2017 Jeffrey Hullekes. All rights reserved.
 */

class FlowFunction extends FlowElement
{
    private $functionNamespace = null;
    private $functionName = null;
    private $createInputAndOutputContainers = null;

    private $xRet = null;
    private $yRet = null;
    private $isPositionOfRet = null;

    function __construct($functionName, $functionNamespace = null, $createInputAndOutputContainers = true) {  // @Incomplete: isn't it better to try the current namespace first? And/or put the namespace inside/before the functionname?
        $this->functionName = $functionName;
        $this->functionNamespace = $functionNamespace;
        $this->createInputAndOutputContainers = $createInputAndOutputContainers;
    }

    function setCoords($x = null, $y = null, $isPositionOf = null, $xRet = null, $yRet = null, $isPositionOfRet = null) {  // TODO: we probably want something else instead of setCoords(). Maybe a method 'callWithCoords', so we can safely remove the last 2 arguments and interpret them as x and y
        $this->x = $x;  // FIXME: maybe we should this $this->x here, since this is the position of the function-CALL, not of the function!
        $this->y = $y;
        $this->isPositionOf = $isPositionOf;
        $this->xRet = $xRet;
        $this->yRet = $yRet;
        $this->isPositionOfRet = $isPositionOfRet;
    }

    function call() {
        $argumentList = func_get_args();

        $argumentsOutsideFunction = array();
        foreach ($argumentList as $argument) {
            $argumentFlowValue = $this->getFlowValueFromValue($argument); // if a new Flow-element has to be created (e.g. if it's a normal php-value) then it is created outside the called function (since that is still the current container)
            array_push($argumentsOutsideFunction, $argumentFlowValue);
        }

        $functionName = $this->functionName;
        $functionNamespace = $this->functionNamespace;

        $currentContainer = self::getCurrentContainer();

        // 3.2.1 add the non-primitive function to the function-container it is called in ('current function')
        $functionContainer = new FlowContainer('FunctionContainer', $functionName, false, $this->x, $this->y, $this->isPositionOf);
        $currentContainer->add($functionContainer);


        $inputContainer = null;
        $outputContainer = null;

        if ($this->createInputAndOutputContainers) {
            $inputContainer = new FlowContainer('InputContainer');
            $bodyContainer = new FlowContainer('BodyContainer');
            $outputContainer = new FlowContainer('OutputContainer');

            $functionContainer->add($inputContainer);
            $functionContainer->add($bodyContainer);
            $functionContainer->add($outputContainer);

            // 3.2.2   add the arguments as (input-)elements to that non-primitive function and add connections from the arguments outside to the arguments inside this function
            $argumentsInsideFunction = array();
            foreach ($argumentsOutsideFunction as $argumentOutsideFunction) {
                $argumentInsideFunction = $this->createNewFlowValueFromFlowValue($argumentOutsideFunction);
                $inputContainer->add($argumentInsideFunction);
                self::addConnection($argumentOutsideFunction, $argumentInsideFunction);

                array_push($argumentsInsideFunction, $argumentInsideFunction);
            }
        }
        else {
            $bodyContainer = new FlowContainer('BodyContainer');
            $functionContainer->add($bodyContainer);

            $argumentsInsideFunction = $argumentsOutsideFunction;
        }

        self::setCurrentContainer($bodyContainer);

        if ($functionNamespace === null) {
            $returnValueInBodyContainer = call_user_func_array($functionName, $argumentsInsideFunction);
        }
        else {
            // @Incomplete: we should test use of a namespace!
            $returnValueInBodyContainer = call_user_func_array(array($functionNamespace, $functionName), $argumentsInsideFunction);
        }

        if ($this->createInputAndOutputContainers) {
            // 3.2.3   add the return-value as an (output-)element to that non-primitive function
            // @Incomplete: shouldn't we simply "move" the $returnValue from the bodyContainer to the outputContainer, instead of creating a new one here?
            $returnValueInOutputContainer = $this->createNewFlowValueFromFlowValue($returnValueInBodyContainer);
            $outputContainer->add($returnValueInOutputContainer);
            self::addConnection($returnValueInBodyContainer, $returnValueInOutputContainer);
        }
        else {
            $returnValueInOutputContainer = $returnValueInBodyContainer;
        }

        self::setCurrentContainer($currentContainer);

        // 3.2.4 add the returned-value as an element to the function-container that receives i t ('current function')
        $returnValueOutsideFunction = $this->createNewFlowValueFromFlowValue($returnValueInOutputContainer, $this->xRet, $this->yRet, $this->isPositionOfRet);
        $currentContainer->add($returnValueOutsideFunction);
        self::addConnection($returnValueInOutputContainer, $returnValueOutsideFunction);

        return $returnValueOutsideFunction;
    }

}
