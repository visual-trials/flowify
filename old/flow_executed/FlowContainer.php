<?php

/**
 *  Copyright © 2017 Jeffrey Hullekes. All rights reserved.
 */

class FlowContainer extends FlowElement
{
    private $containerName = null;
    private $elements = array();

    function __construct($containerType = 'Container', $containerName = null, $isRootElement = false, $x, $y, $isPositionOf) {
        if ($isRootElement) {
            $this->type = 'RootContainer';
            $this->x = $x;
            $this->y = $y;
            $this->isPositionOf = $isPositionOf;
            $this->id = self::createNewFlowElementNumber($isRootElement);

            $this->containerName = $containerName;
        }
        else {
            $this->type = $containerType;
            $this->x = $x;
            $this->y = $y;
            $this->isPositionOf = $isPositionOf;
            $this->id = self::createNewFlowElementNumber();

            $this->containerName = $containerName;
        }
    }

    public function name() {
        return $this->containerName;
    }

    public function add($flowElement) {
        array_push($this->elements, $flowElement);
    }

    public function elements() {
        return $this->elements;
    }

    public function getContainerTree() {
        $containerTree = array();
        $containerTree['id'] = $this->id;
        $containerTree['type'] = $this->type;
        $containerTree['x'] = $this->x;
        $containerTree['y'] = $this->y;
        $containerTree['isPositionOf'] = $this->isPositionOf;
        $containerTree['name'] = $this->containerName;
        $containerTree['children'] = array();
        foreach ($this->elements as $flowElement) {
            $elementInfo = $flowElement->getContainerTree();
            array_push($containerTree['children'], $elementInfo);
        }
        return $containerTree;
    }
}

