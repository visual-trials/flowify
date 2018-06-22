<?php

/**
 *  Copyright © 2017 Jeffrey Hullekes. All rights reserved.
 */

class FlowMap extends FlowElement
{

    public $mapToKeyValues;

    function __construct($inputValue = null, $addToCurrentContainer = true, $x = null, $y = null, $isPositionOf = null) {

        // 2.1.1 add the FlowMap-element to the function-container it is new-ed in
        $this->type = 'Map';
        $this->x = $x;
        $this->y = $y;
        $this->isPositionOf = $isPositionOf;

        $this->id = self::createNewFlowElementNumber();
        if ($addToCurrentContainer) {
            self::getCurrentContainer()->add($this);
        }

        if ($this->mapToKeyValues === null) {
            $this->mapToKeyValues = array();
        }

        // 2.1.2 set mapToKeyValues by converting the input value
        if ($inputValue === null) {
            // $this->mapToKeyValues = null; // FIXME: what to do here?
        }
        else {
            if (is_array($inputValue)) {
                if ($this->isAssoc($inputValue)) {
                    foreach ($inputValue as $key => $value) {
                        $flowKey = $this->getFlowValueFromValue($key);
                        $flowValue = $this->getFlowValueFromValue($value);

                        $flowKeyValue = new FlowKeyValue($flowKey, $flowValue);

                        $this->mapToKeyValues[$key] = $flowKeyValue;
                    }
                }
                else {
                    // @Incomplete: it's a php-list, what to do?
                }
            }
            else {
                $className = get_class($inputValue);
                if ($className == 'FlowMap') {
                    // @Check: we are now making a deep copy. Is that ok?
                    // @Cleanup: accessing $inputValue->mapToKeyValues directly from here
                    foreach ($inputValue->mapToKeyValues as $bareKey => $flowKeyValue) {
                        // @Incomplete: call FlowKeyValue() with a FlowKeyValue, so that it will create a new FlowKeyValue, instead of doing it here
                        $newFlowKey = $this->createNewFlowValueFromFlowValue($flowKeyValue->key());
                        $newFlowValue = $this->createNewFlowValueFromFlowValue($flowKeyValue->value());
                        $newFloatKeyValue = new FlowKeyValue($newFlowKey, $newFlowValue);
                        $this->mapToKeyValues[$bareKey] = $newFloatKeyValue;
                    }
                }
                else {
                    // @Incomplete: it's not a php array, and not a FlowMap, what to do?
                }
            }
        }
    }

    function put($key, $value, $createInputAndOutputContainers = true, $x = null, $y = null, $isPositionOf = null, $xRet = null, $yRet = null, $isPositionOfRet = null) {
        $flowValueToPut = $this->getFlowValueFromValue($value);
        $flowKeyToPut = $this->getFlowValueFromValue($key);

        if ($flowValueToPut === null) return null;  // FIXME: no $flowValueToPut so what to do??

        // 2.2.1 add the primitive-function to the function-container it is called in ('current function')
        $primitiveFunctionContainer = new FlowContainer('PrimitiveFunctionContainer', 'put', false, $x, $y, $isPositionOf);

        $currentContainer = self::getCurrentContainer();
        $currentContainer->add($primitiveFunctionContainer);

        if ($createInputAndOutputContainers) {

            $inputContainer = new FlowContainer('InputContainer');
            $outputContainer = new FlowContainer('OutputContainer');

            $primitiveFunctionContainer->add($inputContainer);
            $primitiveFunctionContainer->add($outputContainer);

            // @Cleanup: use ->createNewFlowValueFromFlowValue() instead of new FlowMap() here!

            // 2.2.2 add this (the original FlowMap-element) as an (input-)element to that primitive function (pointing to the original FlowMap-element as "EQUALS")
            $firstInputToPrimitiveFunction = new FlowMap($this, false);
            $inputContainer->add($firstInputToPrimitiveFunction);

            // 2.2.3  add a connection from this (original FlowMap) element to the input value just added
            self::addConnection($this, $firstInputToPrimitiveFunction);

            // 2.2.4  add the keyName as an (input-)element to that primitive function
            // @Incomplete: call createNewFlowValueFromFlowValue instead! No hardcoded to FlowInt!
            $secondInputToPrimitiveFunction = new FlowInt($flowKeyToPut, false);
            $inputContainer->add($secondInputToPrimitiveFunction);

            // 2.2.5  add a connection from the input value (the original) to the keyName just added
            self::addConnection($flowKeyToPut, $secondInputToPrimitiveFunction);

            // 2.2.6  add the value as an (input-)element to that primitive function
            $thirdInputToPrimitiveFunction = new FlowInt($flowValueToPut, false);
            $inputContainer->add($thirdInputToPrimitiveFunction);

            // 2.2.7  add a connection from the input value (the original) to the value just added
            self::addConnection($flowValueToPut, $thirdInputToPrimitiveFunction);

            // 2.2.8  add the return-value as an (output-)element to that primitive function
            $returnValue = new FlowMap($this, false);
            $outputContainer->add($returnValue);

            // Do the actual 'put' on the return-value
            $bareKey = $this->getBareValueFromValue($flowKeyToPut);
            $newFlowKey = $this->createNewFlowValueFromFlowValue($flowKeyToPut);
            $newFlowValue = $this->createNewFlowValueFromFlowValue($flowValueToPut);
            $flowKeyValue = new FlowKeyValue($newFlowKey, $newFlowValue);
            $returnValue->mapToKeyValues[$bareKey] = $flowKeyValue;

            // 2.2.9  add the external return-value as an element to the function-container that receives it ('current function')
            $externalReturnValue = new FlowMap($returnValue, false, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // 2.2.10  add a connection from the return-value in the primitive funtion to the external return-value in the function-container that receives it ('current function')
            self::addConnection($returnValue, $externalReturnValue);

        }
        else {
            self::addConnection($this, $primitiveFunctionContainer);
            self::addConnection($flowValueToPut, $primitiveFunctionContainer);
            self::addConnection($flowKeyToPut, $primitiveFunctionContainer);

            // then creating a new FlowMap with the new array
            $externalReturnValue = new FlowMap($this, false, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // Do the actual 'put' on the extenral return-value
            $bareKey = $this->getBareValueFromValue($flowKeyToPut);
            $newFlowKey = $this->createNewFlowValueFromFlowValue($flowKeyToPut);
            $newFlowValue = $this->createNewFlowValueFromFlowValue($flowValueToPut);
            $flowKeyValue = new FlowKeyValue($newFlowKey, $newFlowValue);
            $externalReturnValue->mapToKeyValues[$bareKey] = $flowKeyValue;

            // 1.2.z  add a connection from the primitive function to the external return-value in the function-container that receives it ('current function')
            self::addConnection($primitiveFunctionContainer, $externalReturnValue);
        }

        return $externalReturnValue;
    }

    function get($key, $createInputAndOutputContainers = true, $x = null, $y = null, $isPositionOf = null, $xRet = null, $yRet = null, $isPositionOfRet = null) {
        $flowKeyToGet = $this->getFlowValueFromValue($key);

        // 2.2.1 add the primitive-function to the function-container it is called in ('current function')
        $primitiveFunctionContainer = new FlowContainer('PrimitiveFunctionContainer', 'get', false, $x, $y, $isPositionOf);

        $currentContainer = self::getCurrentContainer();
        $currentContainer->add($primitiveFunctionContainer);

        if ($createInputAndOutputContainers) {

            $inputContainer = new FlowContainer('InputContainer');
            $outputContainer = new FlowContainer('OutputContainer');

            $primitiveFunctionContainer->add($inputContainer);
            $primitiveFunctionContainer->add($outputContainer);

            // @Cleanup: use ->createNewFlowValueFromFlowValue() instead of new FlowMap() here!

            // 2.2.2 add this (the original FlowMap-element) as an (input-)element to that primitive function (pointing to the original FlowMap-element as "EQUALS")
            $firstInputToPrimitiveFunction = new FlowMap($this, false);
            $inputContainer->add($firstInputToPrimitiveFunction);

            // 2.2.3  add a connection from this (FlowMap) element to the input value just added
            self::addConnection($this, $firstInputToPrimitiveFunction);

            // 2.2.4  add the keyName as an (input-)element to that primitive function
            // @Incomplete: call createNewFlowValueFromFlowValue instead! No hardcoded to FlowInt!
            $secondInputToPrimitiveFunction = new FlowInt($flowKeyToGet, false);
            $inputContainer->add($secondInputToPrimitiveFunction);

            // 2.2.5  add a connection from the input value (the original) to the keyName just added
            self::addConnection($flowKeyToGet, $secondInputToPrimitiveFunction);

            // Do the actual 'get' on the this
            $bareKey = $this->getBareValueFromValue($flowKeyToGet);
            $flowValue = null;
            if (array_key_exists($bareKey, $this->mapToKeyValues)) {
                $flowKeyValue = $this->mapToKeyValues[$bareKey];
                $flowValue = $flowKeyValue->value();
            }
            else {
                //FIXME: what if key does not exist! Create FlowNull??
                // $flowValue = new FlowNull(false);
            }

            // 2.2.8  add the return-value as an (output-)element to that primitive function
            $returnValue = $this->createNewFlowValueFromFlowValue($flowValue);
            $outputContainer->add($returnValue);

            // 2.2.9  add the external return-value as an element to the function-container that receives it ('current function')
            $externalReturnValue = $this->createNewFlowValueFromFlowValue($returnValue, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // 2.2.10  add a connection from the return-value in the primitive funtion to the external return-value in the function-container that receives it ('current function')
            self::addConnection($returnValue, $externalReturnValue);

        }
        else {
            self::addConnection($this, $primitiveFunctionContainer);
            self::addConnection($flowKeyToGet, $primitiveFunctionContainer);

            // Do the actual 'get' on the this
            $bareKey = $this->getBareValueFromValue($flowKeyToGet);
            $flowValue = null;
            if (array_key_exists($bareKey, $this->mapToKeyValues)) {
                $flowKeyValue = $this->mapToKeyValues[$bareKey];
                $flowValue = $flowKeyValue->value();
            }
            else {
                //FIXME: what if key does not exist! Create FlowNull??
                // $flowValue = new FlowNull(false);
            }

            // then creating a new FlowMap with the new array
            $externalReturnValue = $this->createNewFlowValueFromFlowValue($flowValue, $xRet, $yRet, $isPositionOfRet);
            $currentContainer->add($externalReturnValue);

            // 1.2.z  add a connection from the primitive function to the external return-value in the function-container that receives it ('current function')
            self::addConnection($primitiveFunctionContainer, $externalReturnValue);
        }

        return $externalReturnValue;
    }

    function getContainerTree() {

        $mapInfo = array();
        $mapInfo['id'] = $this->id;
        $mapInfo['x'] = $this->x;
        $mapInfo['y'] = $this->y;
        $mapInfo['isPositionOf'] = $this->isPositionOf;
        $mapInfo['type'] = $this->type;
        $mapInfo['children'] = array();

        foreach ($this->mapToKeyValues as $bareKey => $flowKeyValue) {
            $flowKeyValue = $this->mapToKeyValues[$bareKey];

            $keyValueInfo = $flowKeyValue->getContainerTree();
            array_push($mapInfo['children'], $keyValueInfo);
        }

        return $mapInfo;
    }

}
