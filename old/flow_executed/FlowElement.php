<?php

/**
 *  Copyright © 2017 Jeffrey Hullekes. All rights reserved.
 */

abstract class FlowElement
{
    private static $nrOfFlowElements = null;
    private static $currentContainer = null;
    private static $rootElement = null;
    private static $connections = array();

    protected $type;
    protected $id;
    protected $bare;
    protected $x;
    protected $y;
    protected $isPositionOf;

    protected function createNewFlowElementNumber($isRootElement = false) {

        // If a new non-root-element is being created and there is no root-element yet, we will create the root-container here (for convenience)
        if (!$isRootElement && self::$rootElement === null) {
            self::$rootElement = new FlowContainer('RootContainer', null,true);
            self::setCurrentContainer(self::$rootElement);
        }

        if (self::$nrOfFlowElements === null) {
            self::$nrOfFlowElements = 0;
        }
        else {
            self::$nrOfFlowElements++;
        }

        return self::$nrOfFlowElements;
    }

    protected function getBareValueFromValue($inputValue)
    {

        if (is_object($inputValue)) {
            $className = get_class($inputValue);

            if ($className == 'FlowNull') {
                return $inputValue->bare();
            } else if ($className == 'FlowInt') {
                return $inputValue->bare();
            } else if ($className == 'FlowString') {
                return $inputValue->bare();
            } else if ($className == 'FlowFloat') {
                return $inputValue->bare();
            } else if ($className == 'FlowBool') {
                return $inputValue->bare();
            } else if ($className == 'FlowMap') {
                return $inputValue->bare();  // @Incomplete: what is the bareValue of a FlowMap?
            } else if ($className == 'FlowList') {
                return $inputValue->bare(); // @Incomplete: what is the bareValue of a FlowList?
            } else {
                return $inputValue;  // @Incomplete: should we really return the $inputValue itself if we don't recognize the className?
            }
        }
        else {
            return $inputValue;
        }
    }

    protected function getFlowValueFromValue($inputValue) {

        // is_object() - Finds whether a variable is an object
        // is_bool() - Finds out whether a variable is a boolean
        // is_int() - Find whether the type of a variable is integer
        // is_float() - Finds whether the type of a variable is float
        // is_numeric() - Finds whether a variable is a number or a numeric string
        // is_string() - Find whether the type of a variable is string
        // is_array() - Finds whether a variable is an array

        if (is_object($inputValue)) {
            $className = get_class($inputValue);

            if ($className == 'FlowNull') {
                return $inputValue;
            }
            else if ($className == 'FlowInt') {
                return $inputValue;
            }
            else if ($className == 'FlowString') {
                return $inputValue;
            }
            else if ($className == 'FlowFloat') {
                return $inputValue;
            }
            else if ($className == 'FlowBool') {
                return $inputValue;
            }
            else if ($className == 'FlowMap') {
                return $inputValue;
            }
            else if ($className == 'FlowList') {
                return $inputValue;
            }
            else {
                // FIXME: what should we do with non-Flow-objects??
                return null;
            }
        }
        else {
            if ($inputValue === null) {
                return new FlowNull(); // FIXME: what type to return when value is null?
            }
            else if (is_bool($inputValue)) {
                return new FlowBool($inputValue);
            }
            else if (is_float($inputValue)) {
                return new FlowFloat($inputValue);
            }
            else if (is_int($inputValue)) {
                return new FlowInt($inputValue);
            }
            else if (is_string($inputValue)) {
                return new FlowString($inputValue);
            }
            else if (is_array($inputValue)) {
                if ($this->isAssoc($inputValue)) {
                    return new FlowMap($inputValue);
                }
                else {
                    return new FlowList($inputValue);
                }
            }
            else {
                // FIXME: unknown php-type
                return null;
            }
        }
    }

    // @Incomplete: this should be a function implemented/overridden by each FlowXXX-class. Then the would be no reason to create the if-statements and you would call it clone()
    protected function createNewFlowValueFromFlowValue($flowValue, $x = null, $y = null, $isPositionOf = null) {

        if (is_object($flowValue)) {
            $className = get_class($flowValue);

            if ($className == 'FlowNull') {
                return new FlowNull($flowValue, false, $x, $y, $isPositionOf);
            }
            else if ($className == 'FlowInt') {
                return new FlowInt($flowValue, false, $x, $y, $isPositionOf);
            }
            else if ($className == 'FlowString') {
                return new FlowString($flowValue, false, $x, $y, $isPositionOf);
            }
            else if ($className == 'FlowFloat') {
                return new FlowFloat($flowValue, false, $x, $y, $isPositionOf);
            }
            else if ($className == 'FlowBool') {
                return new FlowBool($flowValue, false, $x, $y, $isPositionOf);
            }
            /*
             @Incomplete: make sure that when FlowKeyValue is new-ed, it clones itself (this is currently implemented in FlowMap!)
            else if ($className == 'FlowKeyValue') {
                return new FlowKeyValue($flowValue, false);
            }
            */
            else if ($className == 'FlowMap') {
                return new FlowMap($flowValue, false, $x, $y, $isPositionOf);
            }
            else if ($className == 'FlowList') {
                return new FlowList($flowValue, false, $x, $y, $isPositionOf);
            }
            else {
                // FIXME: what should we do with non-Flow-objects??
                return null;
            }
        }
        else {
            // FIXME: we should not get these here!
        }

    }

    protected function isAssoc(array $arr)
    {
        if (array() === $arr) return false;
        return array_keys($arr) !== range(0, count($arr) - 1);
    }

    /**
     * @returns FlowContainer
     */
    protected function getCurrentContainer() {
        return self::$currentContainer;
    }

    protected function setCurrentContainer($currentContainer) {
        self::$currentContainer = $currentContainer;
    }

    public function bare() {
        return $this->bare;
    }

    public function id() {
        return $this->id;
    }

    public function type() {
        return $this->type;
    }

    protected function addConnection($fromElement, $toElement) {
        // @Incomplete: check if neither $fromElement nor $toElement us null!
        array_push(self::$connections, array( 'from' => $fromElement->id, 'to' => $toElement->id ));
    }

    static function getConnections() {
        return self::$connections;
    }

    static function getRootElement() {
        if (self::$rootElement == null) {
            // If the rootElement is being retrieved but it doesn't exist yey, we will create the root-container here (so that the return value is not null)
            // this will add the root-container to the $containerStack
            // @Duplcation: this is duplicated code!!
            self::$rootElement = new FlowContainer('RootContainer', null,true);
            self::setCurrentContainer(self::$rootElement);
        }
        return self::$rootElement;
    }
}