<?php

$myVar = 34;

$b = 76;

$extraVar = (19 + 51) * (47 + 31);

$c = $myVar + $b;

$d = dechex($c);

$e = 128;

$sq = square($c);

$f = average2($b, $e);

function square ($toBeSquared) {
    $square = $toBeSquared * $toBeSquared;

    return  $square;
}

function average2 ($first, $second) {
    $average = ($first + $second) / 2;

    return $average;
}