<?php

$fib = fibonacci_recursive(10);

function fibonacci_recursive($n)
{
    $f = 0;
    if ($n == 0) {
        $f = 0;
    }
    else {
        if ($n == 1) {
            $f = 1;
        }
        else {
            $f = fibonacci_recursive($n - 1) + fibonacci_recursive($n - 2);
        }
    }
    return $f;
}