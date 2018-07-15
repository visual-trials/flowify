<?php

require "../../lib/bootstrap_PhpParser.php";

function getAstFromPhpCode($code) {
    
    $lexer = new PhpParser\Lexer\Emulative(['usedAttributes' => [
        'startLine', 'endLine', 'startFilePos', 'endFilePos', 'comments'
    ]]);
    $parser = (new PhpParser\ParserFactory)->create(
        PhpParser\ParserFactory::PREFER_PHP7,
        $lexer
    );

    /*
    $tokensWithPosInfo = [];
    $tokenValue = null;
    $tokenStartAttributes = null;
    $tokenEndAttributes = null;

    $lexer->startLexing($code);
    $tokenId = $lexer->getNextToken($tokenValue, $tokenStartAttributes, $tokenEndAttributes);
    $tokensWithPosInfo[] = $tokenValue;
    $tokensWithPosInfo[] = $tokenStartAttributes;
    $tokensWithPosInfo[] = $tokenEndAttributes;
    */

    try {
        $stmts = $parser->parse($code);
        $tokens = $lexer->getTokens();
    } catch (PhpParser\Error $error) {
        die($error->getMessage() . "\n");  // $error->getMessageWithColumnInfo($code)
    }

    $statementsJSON = json_encode($stmts, JSON_PRETTY_PRINT);
    $statements = json_decode($statementsJSON, true);

    return $statements;
}

function getAstNodeIdentifier ($astNode) {

    global $code;

    $isListOfStatements = false;
    
    $startAstNode = $astNode;
    $endAstNode = $astNode;
    // For statements, you get a list of statements, so we need the start positions of the first statement and the end position of the last statement
    if ($astNode !== null && !is_assoc($astNode)) {
        if (count($astNode) > 0) {
            $startAstNode = $astNode[0];
            $endAstNode = $astNode[count($astNode) - 1];
        }
        $isListOfStatements = true;
    }
    
    $startLine = 0;
    $startingColumnNumber = 0;
    $endLine = 0;
    $endColumnNumber = 0;

    if ($startAstNode !== null) {
        if (array_key_exists('attributes', $startAstNode) && array_key_exists('startLine', $startAstNode['attributes'] )) {
            if (array_key_exists('attributes', $endAstNode) && array_key_exists('startLine', $endAstNode['attributes'] )) {
                $startLine = $startAstNode['attributes']['startLine'];
                $endLine = $endAstNode['attributes']['endLine'];
                $startLineCheck = stringPosToLineNumber($code, $startAstNode['attributes']['startFilePos']);
                if ($startLine !== $startLineCheck) {
                    die("startLine: '$startLine'' is not equals to startLineCheck: '$startLineCheck''");
                }
                $endLineCheck = stringPosToLineNumber($code, $endAstNode['attributes']['endFilePos']);
                if ($endLine !== $endLineCheck) {
                    die("endLine: '$endLine'' is not equals to endLineCheck: '$endLineCheck''");
                }
                $startingColumnNumber = stringPosToColumn($code, $startAstNode['attributes']['startFilePos']);
                $endColumnNumber = stringPosToColumn($code, $endAstNode['attributes']['endFilePos']);
            }
            else {
                die("Could not find attributes in node!\n" . print_r($endAstNode, true));
            }
        }
        else {
            die("Could not find attributes in node!\n" . print_r($startAstNode, true));
        }
    }

    $astNodeIdentifier = $startLine . ':' . $startingColumnNumber . '-' . $endLine . ':' . $endColumnNumber;
    
    if ($isListOfStatements) {
        $astNodeIdentifier = $astNodeIdentifier . '_Stmts';
    }

    return $astNodeIdentifier;
}

