# Flowify types

## Flow Elements
* If
    * condition
        * condExpr [expression]
        * lane-splitter
    * then  [statements]
    * else [statements]
    * end
        * lane-joiner
* For-loop
    * init [expression(s)]
    * start
        * lane-joiner
    * condition [expression]
        * condExpr [expression]
        * lane-splitter
    * step [statements]
    * update [statements/expression(s)]
    * back [connection-only-lane-segment]
    * done [connection-only-lane-segment]
* Function/Root
    * argument(s) [named-attachement-points]
    * return variable(s)  [attachement-point(s)]
    * body [statements]
* PrimitiveFunction
    * argument(s) [named-attachement-points]
    * return variable(s)  [attachement-point(s)]
* Variable
* Constant
* Statement (lane-segment)
* Tunnel
    * type
        * break
        * continue
        * return
        * throw
        * goto
    * entrance/exit

## Flow connections

* Types
    * Assigment
    * Identity
    * Usage
* Points
    * Connection-joiner
    * Connection-splitter