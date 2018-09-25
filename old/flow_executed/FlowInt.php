<?php

/**
 *  Copyright © 2017 Jeffrey Hullekes. All rights reserved.
 */

class FlowInt extends FlowElement
{

    function __construct($inputValue = null, $addToCurrentContainer = true, $x = null, $y = null, $isPositionOf = null) {

        // 1.1.1 add the FlowInt-element to the function-container it is new-ed in
        $this->type = 'Int';
        $this->x = $x;
        $this->y = $y;
        $this->isPositionOf = $isPositionOf;

        $this->id = self::createNewFlowElementNumber();
        if ($addToCurrentContainer) {
            self::getCurrentContainer()->add($this);
        }

        // 1.1.2 set the bare value by converting the input value
        $this->bare = $this->getBareValueFromValue($inputValue);  // @Incomplete: should we cast/convert to intval here? What if it's another Flow-class or other object? or what if it's an array?

        // @Incomplete: what if the inputValue is a FlowElement? Should we create a connection between this FlowInt and that FlowElement?
    }

    function add($inputValue = null, $createInputAndOutputContainers = true, $x = null, $y = null, $isPositionOf = null, $xRet = null, $yRet = null, $isPositionOfRet = null) {
        return $this->doIntegerOperation('add', $inputValue, $createInputAndOutputContainers, $x, $y, $isPositionOf, $xRet, $yRet, $isPositionOfRet);
    }

    function substract($inputValue = null, $createInputAndOutputContainers = true, $x = null, $y = null, $isPositionOf = null, $xRet = null, $yRet = null, $isPositionOfRet = null) {
        return $this->doIntegerOperation('substract', $inputValue, $createInputAndOutputContainers, $x, $y, $isPositionOf, $xRet, $yRet, $isPositionOfRet);
    }

    function multiply($inputValue = null, $createInputAndOutputContainers = true, $x = null, $y = null, $isPositionOf = null, $xRet = null, $yRet = null, $isPositionOfRet = null) {
        return $this->doIntegerOperation('multiply', $inputValue, $createInputAndOutputContainers, $x, $y, $isPositionOf, $xRet, $yRet, $isPositionOfRet);
    }

    function doIntegerOperation($operation, $inputValue, $createInputAndOutputContainers, $x, $y, $isPositionOf, $xRet, $yRet, $isPositionOfRet) {
        $flowValueToDoOperationWith = $this->getFlowValueFromValue($inputValue);

        if ($flowValueToDoOperationWith === null) return null;  // FIXME: no $flowValueToDoOperationWith so what to do??

        $returnBareValue = null;
        if ($operation === 'add') {
            $returnBareValue = $this->bare() + $flowValueToDoOperationWith->bare();
        } else if ($operation === 'substract') {
            $returnBareValue = $this->bare() - $flowValueToDoOperationWith->bare();
        } else if ($operation === 'multiply') {
            $returnBareValue = $this->bare() * $flowValueToDoOperationWith->bare();
        } else {
            // FIXME: unsupported operation
        }

        // 1.2.1  add the primitive-function to the function-container it is called in ('current function')
        $primitiveFunctionContainer = new FlowContainer('PrimitiveFunctionContainer', $operation, false, $x, $y, $isPositionOf);

        $currentContainer = self::getCurrentContainer();
        $currentContainer->add($primitiveFunctionContainer);

        if ($createInputAndOutputContainers) {

            $inputContainer = new FlowContainer('InputContainer');
            $outputContainer = new FlowContainer('OutputContainer');

            $primitiveFunctionContainer->add($inputContainer);
            $primitiveFunctionContainer->add($outputContainer);

            // @Cleanup: use ->createNewFlowValueFromFlowValue() instead of new FlowInt() here!

            // 1.2.2  add this (original FlowInt-element) as an (input-)element to that primitive function (pointing to the original FlowInt-element as "EQUALS")
            $firstInputToPrimitiveFunction = new FlowInt($this, false);
            $inputContainer->add($firstInputToPrimitiveFunction);

            // 1.2.3  add a connection from this (original FlowInt) element to the input value just added
            self::addConnection($this, $firstInputToPrimitiveFunction);

            //  1.2.4  add the input value as an (input-)element to that primitive function
            $secondInputToPrimitiveFunction = new FlowInt($flowValueToDoOperationWith, false);
            $inputContainer->add($secondInputToPrimitiveFunction);

            // 1.2.5  add a connection from the input value (the original) to the input value just added
            self::addConnection($flowValueToDoOperationWith, $secondInputToPrimitiveFunction);

            // 1.2.6  add the return-value as an (output-)element to that primitive function
            $returnValue = new FlowInt($returnBareValue, false);
            $outputContainer->add($returnValue);

            // 1.2.7  add the external return-value as an element to the function-container that receives it ('current function')
            $externalReturnValue = new FlowInt($returnValue, false, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // 1.2.8  add a connection from the return-value in the primitive funtion to the external return-value in the function-container that receives it ('current function')
            self::addConnection($returnValue, $externalReturnValue);
        }
        else {
            // 1.2.x Do not create input- and output- containers, instead connect the inputs and output directly to the primitive container
            self::addConnection($this, $primitiveFunctionContainer);
            self::addConnection($flowValueToDoOperationWith, $primitiveFunctionContainer);

            // 1.2.y  add the external return-value as an element to the function-container that receives it ('current function')
            $externalReturnValue = new FlowInt($returnBareValue, false, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // 1.2.z  add a connection from the primitive funtion to the external return-value in the function-container that receives it ('current function')
            self::addConnection($primitiveFunctionContainer, $externalReturnValue);
        }

        return $externalReturnValue;

    }

    function getContainerTree () {
        $elementInfo = array();
        $elementInfo['id'] = $this->id;
        $elementInfo['type'] = $this->type;
        $elementInfo['x'] = $this->x;
        $elementInfo['y'] = $this->y;
        $elementInfo['isPositionOf'] = $this->isPositionOf;

        $elementInfo['bareValue'] = $this->bare;

        return $elementInfo;
    }

}