<?php

$fib = fibonacci_iterative(10);

function fibonacci_iterative($n)
{
    $a = 0;
    $b = 1;
    for ($i = 0; $i < $n; $i++) {
        $c = $a;
        $a = $b;
        $b += $c;
    }
    return $a;
}