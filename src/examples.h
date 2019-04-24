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
 
const char * simple_assign_program_name = "simple_assignment.php";
const char * simple_assign_program_text = 
    "<?php\n"
    "\n"
    "$answer = 42;\n"
;

const char * i_plus_plus_program_name = "i_plus_plus.php";
const char * i_plus_plus_program_text = 
    "<?php\n"
    "\n"
    "$i = 7;\n"
    "\n"
    "$b = $i++;\n"
    "\n"
    "$c = ++$b;"
;

const char * simple_math_program_name = "simple_math.php";
const char * simple_math_program_text = 
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

const char * simple_if_program_name = "simple_if_statement.php";
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

const char * simple_if_else_program_name = "simple_if_else_statement.php";
const char * simple_if_else_program_text = 
    "<?php\n"
    "\n"
    "$myVariable = 34;\n"
    "\n"
    "$best = 76;\n"
    "\n"
    "\n"
    "if ($best > 50) {\n"
    "   $best = 50;\n"
    "   $myVariable += 10;\n"
    "}\n"
    "else {\n"
    "   $best = 30;\n"
    "   $myVariable *= 12;\n"
    "}\n"
    "\n"
    "$common = $myVariable + $best;\n"
;

const char * simple_for_program_name = "simple_for_loop.php";
const char * simple_for_program_text = 
    "<?php\n"
    "\n"
    "$b = 76;\n"
    "\n"
    "for ($i = 0; $i < 10; $i = $i + 1) {\n"
    "    $b = $b - 5;\n"
    "}\n"
;

const char * simple_for_continue_break_program_name = "simple_for_continue_break.php";
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

const char * fibonacci_iterative_program_name = "fibonacci_iterative.php";
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

const char * fibonacci_recursive_early_return_program_name = "fibonacci_recursive_early_return.php";
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

const char * large_example_program_name = "larger_example.php";
const char * large_example_program_text = 
    "<?php\n"
    "\n"
    "updateAndGetCodeAndVisualInfoForFile($fileToFlowifyWithoutExtention);\n"  // TODO: removed "list($code, $visualInfos) =" here
    "\n"
    "function updateAndGetCodeAndVisualInfoForFile($fileToFlowifyWithoutExtention) {\n"
    "    \n"
    "    // TODO: the order of the coordinates is now by AST-order, not by horizontal position in the line\n"
    "\n"
    "    // FIXME: check if php file has been read correctly!\n"
    "    $oldCode = file_get_contents($fileToFlowifyWithoutExtention . '.bck');\n"
    "    if ($oldCode === false) {\n"
    "        $oldCode = '';\n"                                                            // TODO: changed double-quote to single quote here
    "    }\n"
    "    $newCode = file_get_contents($fileToFlowifyWithoutExtention . '.php');\n"
    "    if ($newCode === false) {\n"
    "        // FIXME: give a proper error (through the api) that the file does not exist!\n"
    "        exit('The file _{$fileToFlowifyWithoutExtention}.php_ does not exist!');\n"  // TODO: changed double-quote to single quote here
    "    }\n"
    "\n"
    "    $code = $oldCode;\n"
    "    $visualInfosJSON = file_get_contents($fileToFlowifyWithoutExtention . '.viz');\n"
    "    $visualInfos = array();\n"
    "    if ($visualInfosJSON !== false) {\n"
    "        $visualInfos = json_decode($visualInfosJSON, true);  // FIXME: what if not valid json?\n"
    "    }\n"
    "\n"
    "    if ($oldCode != $newCode) {\n"
    "\n"
    "        $path = doDiff($oldCode, $newCode);\n"
    "        $oldToNewPositions = getOldToNewPositions($path, $oldCode, $newCode);\n"
    "\n"
    "        $newVisualInfos = getVisualInfosForNewPositions($visualInfos, $oldToNewPositions);\n"
    "\n"
    "        if (count($newVisualInfos) == count($visualInfos)) {\n"
    "            // If all new positions could be mapped, then we want to copy the php-file over the bck-file and store the new .viz file.\n"
    "\n"
    "            // FIXME: also check if the new positions line up with the new AST-elements!\n"
    "\n"
    "            // FIXME: check if this goes right!\n"
    "            storeBackupFile ($newCode, $fileToFlowifyWithoutExtention);\n"
    "\n"
    "            // FIXME: check if this goes right!\n"
    "            storeVisualInfos($newVisualInfos, $fileToFlowifyWithoutExtention);\n"
    "\n"
    "        }\n"
    "        else {\n"
    "            // FIXME: signal to the front-end that we couldn't store the info!\n"
    "        }\n"
    "\n"
    "        // In all cases we want to SHOW the new code and visualInfos\n"
    "        $code = $newCode;\n"
    "        $visualInfos = $newVisualInfos;\n"
    "    }\n"
    "    \n"
    "    return array($code, $visualInfos);\n"
    "    \n"
    "}\n"
    "// VisualInfo\n"
    "\n"
    "function getVisualInfo ($astNodeIdentifier, &$visualInfos, &$usedVisualInfos) {\n"
    "\n"
    "    $currentVisualInfo = [];\n"
    "\n"
    "    if (array_key_exists($astNodeIdentifier, $visualInfos)) {\n"
    "        $currentVisualInfo = $visualInfos[$astNodeIdentifier];\n"
    "        // FIXME: $usedVisualInfos should not be a global that changes it state this way!\n"
    "        $usedVisualInfos[$astNodeIdentifier] = $currentVisualInfo;\n"
    "    }\n"
    "\n"
    "    return $currentVisualInfo;\n"
    "}\n"
    "\n"
    "extendFlowElementsWithVisualInfo();\n"
    "\n"
    // TODO: I moved this functions upwards, because the parser needs functions to be defined before they are used atm!
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
;
    
