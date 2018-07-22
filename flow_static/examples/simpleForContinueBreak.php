<?php

$myVar = 34;

$b = 76;

for ($i = 0; $i < 10; $i = $i + 1) {
    $b = $b - 5;
    if ($b < 20) {
        continue;
    }
    if ($i > 5) {
        break;
    }
}

$c = $myVar + $b;
