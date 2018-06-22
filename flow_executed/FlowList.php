<?php

/**
 *  Copyright © 2017 Jeffrey Hullekes. All rights reserved.
 */

class FlowList extends FlowElement
{

    public $listToFlowValues;

    function __construct($inputValue = null, $addToCurrentContainer = true, $x = null, $y = null, $isPositionOf = null) {

        // 4.1.1 add the FlowList-element to the function-container it is new-ed in
        $this->type = 'List';
        $this->x = $x;
        $this->y = $y;
        $this->isPositionOf = $isPositionOf;

        $this->id = self::createNewFlowElementNumber();
        if ($addToCurrentContainer) {
            self::getCurrentContainer()->add($this);
        }

        if ($this->listToFlowValues === null) {
            $this->listToFlowValues = array();
        }

        // 4.1.2 set the bare value by converting the input value
        if ($inputValue === null) {
            // $this->listToFlowValues = null; // FIXME: what to do here?
        }
        else {
            if (is_array($inputValue)) {
                if (!$this->isAssoc($inputValue)) {
                    foreach ($inputValue as $value) {
                        $flowValue = $this->getFlowValueFromValue($value);

                        array_push($this->listToFlowValues, $flowValue);
                    }
                }
                else {
                    // @Incomplete: it's a php-associative array, what to do?
                }
            }
            else {
                $className = get_class($inputValue);
                if ($className == 'FlowList') {
                    // @Check: we are now making a deep copy. Is that ok?
                    // @Cleanup: accessing $inputValue->listToFlowValues directly from here
                    foreach ($inputValue->listToFlowValues as $flowValue) {
                        $newFlowValue = $this->createNewFlowValueFromFlowValue($flowValue);
                        array_push($this->listToFlowValues, $newFlowValue);
                    }
                }
                else {
                    // @Incomplete: it's not a php non-associative array, and not a FlowList, what to do?
                }
            }
        }
    }

    function push($inputValue, $createInputAndOutputContainers = true, $x = null, $y = null, $isPositionOf = null, $xRet = null, $yRet = null, $isPositionOfRet = null) {
        $flowValueToPush = $this->getFlowValueFromValue($inputValue);

        if ($flowValueToPush === null) return null;  // FIXME: no $flowValueToPush so what to do??

        // 4.2.1 add the primitive-function to the function-container it is called in ('current function')
        $primitiveFunctionContainer = new FlowContainer('PrimitiveFunctionContainer', 'push', false, $x, $y, $isPositionOf);

        $currentContainer = self::getCurrentContainer();
        $currentContainer->add($primitiveFunctionContainer);

        if ($createInputAndOutputContainers) {

            $inputContainer = new FlowContainer('InputContainer');
            $outputContainer = new FlowContainer('OutputContainer');

            $primitiveFunctionContainer->add($inputContainer);
            $primitiveFunctionContainer->add($outputContainer);

            // @Cleanup: use ->createNewFlowValueFromFlowValue() instead of new FlowList() here!

            // 2.2.2 add this (the original FlowList-element) as an (input-)element to that primitive function (pointing to the original FlowList-element as "EQUALS")
            $firstInputToPrimitiveFunction = new FlowList($this, false);
            $inputContainer->add($firstInputToPrimitiveFunction);

            // 2.2.3  add a connection from this (original FlowList) element to the input value just added
            self::addConnection($this, $firstInputToPrimitiveFunction);

            // 2.2.4  add the inputValue as an (input-)element to that primitive function
            $secondInputToPrimitiveFunction = new FlowInt($flowValueToPush, false);
            $inputContainer->add($secondInputToPrimitiveFunction);

            // 2.2.5  add a connection from the input value (the original) to the keyName just added
            self::addConnection($flowValueToPush, $secondInputToPrimitiveFunction);

            // 2.2.6  add the return-value as an (output-)element to that primitive function
            $returnValue = new FlowList($firstInputToPrimitiveFunction, false);
            $outputContainer->add($returnValue);

            // Do the actual 'push' on the return-value
            $newFlowValue = $this->createNewFlowValueFromFlowValue($flowValueToPush);
            array_push($returnValue->listToFlowValues, $newFlowValue);

            // 2.2.7  add the external return-value as an element to the function-container that receives it ('current function')
            $externalReturnValue = new FlowList($returnValue, false, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // 2.2.8  add a connection from the return-value in the primitive funtion to the external return-value in the function-container that receives it ('current function')
            self::addConnection($returnValue, $externalReturnValue);

        }
        else {
            self::addConnection($this, $primitiveFunctionContainer);
            self::addConnection($flowValueToPush, $primitiveFunctionContainer);

            // then creating a new FlowList with the new array
            $externalReturnValue = new FlowList($this, false, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // Do the actual 'push' on the external return-value
            $newFlowValue = $this->createNewFlowValueFromFlowValue($flowValueToPush);
            array_push($externalReturnValue->listToFlowValues, $newFlowValue);

            // 4.2.z  add a connection from the primitive funtion to the external return-value in the function-container that receives it ('current function')
            self::addConnection($primitiveFunctionContainer, $externalReturnValue);
        }

        return $externalReturnValue;
    }

    function getContainerTree() {

        $listInfo = array();
        $listInfo['id'] = $this->id;
        $listInfo['x'] = $this->x;
        $listInfo['y'] = $this->y;
        $listInfo['isPositionOf'] = $this->isPositionOf;
        $listInfo['type'] = $this->type;
        $listInfo['children'] = array();

        foreach ($this->listToFlowValues as $flowValue) {
            $valueInfo = $flowValue->getContainerTree();
            array_push($listInfo['children'], $valueInfo);
        }

        return $listInfo;
    }
}