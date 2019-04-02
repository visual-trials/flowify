/*

   Copyright 2018 Jeffrey Hullekes

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 */
 
const char * simple_assign_program_text = 
    "<?php\n"
    "\n"
    "$answer = 42;\n"
;

const char * i_plus_plus_program_text = 
    "<?php\n"
    "\n"
    "$i = 7;\n"
    "\n"
    "$b = $i++;\n"
    "\n"
    "$c = ++$b;"
;

const char * simple_functions_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "$extraVar = (19 + 51) * (47 + 31);\n"
    "\n"
    "$c = $myVar + $b;\n"
    "\n"
    "$d = dechex($c);\n"
    "\n"
    "$e = 128;\n"
    "\n"
    "$sq = square($c);\n"
    "\n"
    "$f = average2($b, $e);\n"
    "\n"
    "function square ($toBeSquared) {\n"
    "    $square = $toBeSquared * $toBeSquared;\n"
    "\n"
    "    return  $square;\n"
    "}\n"
    "\n"
    "function average2 ($first, $second) {\n"
    "    $average = ($first + $second) / 2;\n"
    "\n"
    "    return $average;\n"
    "}"
;

const char * simple_if_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "if ($b > 50) {\n"
    "	$b = 50;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

const char * simple_if_else_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "\n"
    "if ($b > 50) {\n"
    "   $b = 50;\n"
    "   $myVar += 10;\n"
    "}\n"
    "else {\n"
    "    $b = 30;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

const char * simple_for = 
    "<?php\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "for ($i = 0; $i < 10; $i = $i + 1) {\n"
    "    $b = $b - 5;\n"
    "}\n"
;

const char * simple_for_continue_break_program_text = 
    "<?php\n"
    "\n"
    "$myVar = 34;\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "for ($i = 0; $i < 10; $i = $i + 1) {\n"
    "    if ($b < 20) {\n"
    "        break;\n"
    "    }\n"
    "    if ($i > 5) {\n"
    "        continue;\n"
    "    }\n"
    "    $b = $b - 5;\n"
    "}\n"
    "\n"
    "$c = $myVar + $b;\n"
;

const char * fibonacci_iterative_program_text = 
    "<?php\n"
    "\n"
    "$fib = fibonacci_iterative(10);\n"
    "\n"
    "function fibonacci_iterative($n)\n"
    "{\n"
    "    $a = 0;\n"
    "    $b = 1;\n"
    "    for ($i = 0; $i < $n; $i++) {\n"
    "        $c = $a;\n"
    "        $a = $b;\n"
    "        $b += $c;\n"
    "    }\n"
    "    return $a;\n"
    "}\n"
;

const char * fibonacci_recursive_early_return_program_text = 
    "<?php\n"
    "\n"
    "$fib = fibonacci_recursive(10);\n"
    "\n"
    "function fibonacci_recursive($n)\n"
    "{\n"
    "    if ($n == 0) {\n"
    "        return 0;\n"
    "    }\n"
    "    if ($n == 1) {\n"
    "        return 1;\n"
    "    }\n"
    "    return fibonacci_recursive($n - 1) + fibonacci_recursive($n - 2);\n"
    "}"
;

const char * large_example = 
    "<?php\n"
    "\n"
    "extendFlowElementsWithVisualInfo();\n"
    "// VisualInfo\n"
    "\n"
    "function extendFlowElementsWithVisualInfo (&$flowElementArray, &$visualInfos, &$usedVisualInfos) {\n"
    "    \n"
    "    $visualInfo = getVisualInfo($flowElementArray['astNodeIdentifier'], $visualInfos, $usedVisualInfos);\n"
    "    extendFlowElementWithVisualInfo($flowElementArray, $visualInfo);\n"
    "    \n"
    "    if (array_key_exists('children', $flowElementArray)) {\n"
    "        foreach ($flowElementArray['children'] as &$childflowElementArray) {\n"
    "            extendFlowElementsWithVisualInfo($childflowElementArray, $visualInfos, $usedVisualInfos);\n"
    "        }\n"
    "    }\n"
    "}\n"
    "\n"
    "function extendFlowElementWithVisualInfo (&$flowElementArray, $visualInfo) {\n"
    "\n"
    "    // TODO: maybe set only certain visual attributes given certain FlowElement types? Or simply check if the attributes exists and/or is null?\n"
    "\n"
    "    if ($visualInfo === null) {\n"
    "        return;\n"
    "    }\n"
    "\n"
    // TODO: I added braces here, but the parser should be able to deal with precedence!
    "    if (array_key_exists('x', $visualInfo) && ($visualInfo['x'] !== null)) {\n"
    "        $flowElementArray['x'] = $visualInfo['x'];\n"
    "    }\n"
    "    if (array_key_exists('y', $visualInfo) && ($visualInfo['y'] !== null)) {\n"
    "        $flowElementArray['y'] = $visualInfo['y'];\n"
    "    }\n"
    "    if (array_key_exists('isPositionOf', $visualInfo) && ($visualInfo['isPositionOf'] !== null)) {\n"
    "        $flowElementArray['isPositionOf'] = $visualInfo['isPositionOf'];\n"
    "    }\n"
    "    if (array_key_exists('width', $visualInfo) && ($visualInfo['width'] !== null)) {\n"
    "        $flowElementArray['width'] = $visualInfo['width'];\n"
    "    }\n"
    "    if (array_key_exists('height', $visualInfo) && ($visualInfo['height'] !== null)) {\n"
    "        $flowElementArray['height'] = $visualInfo['height'];\n"
    "    }\n"
    "    if (array_key_exists('relativeScale', $visualInfo) && ($visualInfo['relativeScale'] !== null)) {\n"
    "        $flowElementArray['relativeScale'] = $visualInfo['relativeScale'];\n"
    "    }\n"
    "}\n"
;
    
