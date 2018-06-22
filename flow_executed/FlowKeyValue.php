<?php

/**
 *  Copyright © 2017 Jeffrey Hullekes. All rights reserved.
 */

class FlowKeyValue extends FlowElement
{
    private $flowKey;
    private $flowValue;

    function __construct($flowKey, $flowValue) {
        $this->type = 'KeyValue';
        $this->id = self::createNewFlowElementNumber();

        $this->flowKey = $flowKey;
        $this->flowValue = $flowValue;
    }

    function key() {
        return $this->flowKey;
    }

    function value() {
        return $this->flowValue;
    }

    function getContainerTree() {

        $keyValueInfo = array();
        $keyValueInfo['id'] = $this->id;
        $keyValueInfo['type'] = $this->type;

        $keyValueInfo['children'] = array();

        $keyInfo = $this->flowKey->getContainerTree();
        array_push($keyValueInfo['children'], $keyInfo);

        $valueInfo = $this->flowValue->getContainerTree();
        array_push($keyValueInfo['children'], $valueInfo);

        return $keyValueInfo;
    }

}
