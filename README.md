# Flowify

## Goal

Flowify is meant to visualize existing code.

The goal is to do this in a way that it will give a clear overview and insight into a (unfamiliar) codebase.

https://flowify.io/

## Features to implement

<style type="text/css">

$red-common: #D1132A;
$green-common: #2CAC87;
$green-light-common: #C1C625;
$green-alt-common: #9EC232;
$gold-common: #EBA75C;
$charcoal-common: #3A3F42;
$charcoal-alt: #43484D;
$orange-common: #E4590F;
$yellow-common: #FFCD0F;
$yellow-light-common: #EACB16;

.container {
   max-width: 60em;
   margin: 0 auto;
}
.progress-segment {
  display: flex;
  margin-bottom: 4em;
}

.item {
  width: 100%;
  background-color: lightgray;
  margin-right: 2px;
  height: 8px;

  &:first-child {
    border-top-left-radius: 3px;
    border-bottom-left-radius: 3px;
  }

  &:last-child {
    border-top-right-radius: 3px;
    border-bottom-right-radius: 3px;
  }

  &.red-common {
    background: $red-common !important;
  }

  &.gold-common {
    background: $gold-common !important;
  }

  &.orange-common {
    background: $orange-common !important;
  }

  &.yellow-common {
    background: $yellow-common !important;
  }

  &.yellow-light-common {
    background: $yellow-light-common !important;
  }

  &.green-light-common {
    background: $green-light-common !important;
  }

  &.green-common {
    background: $green-alt-common !important;
  }
}
</style>

### File handling, multiple files, namespaces, context

- Upload source file or download source from url 
- Include other files, detect/use autoloader (build context)
- Support multiple languages

### Parsing more language elements

<table>
    <tr>
        <td>Control statements</td>
        <td><div class="progress-segment">
            <div class="item red-common"></div>
            <div class="item orange-common"></div>
            <div class="item gold-common"></div>
            <div class="item yellow-common"></div>
            <div class="item yellow-light-common"></div>
            <div class="item green-light-common"></div>
            <div class="item green-common"></div>
          </div>
      </td>
    </tr>
</table>

- Control statements
- Expressions
- Declarations
- Assignments
- Variables, Scalars
- Functions
- Classes, properties, methods, inheritance, traits
- Interfaces, namespaces

### Control and dataflow analysis

- Symmetric control elements (if, while, do, for, foreach)
- Asymmetric control elements (return, throw, break, continue)
- Variable/function scope
- Variable assignment
- Variable/function-output usage
- Dataflow join and splits

### Types of visual elements and connections

- Lanes: straight (with segments), splits, joins, loops, assymetric
- Boolean decider into splitter
- Containers (expressions and functions)
- Function arguments/parameters + to-return/returned
- Variable identifiers on arrows
- Icons for identification (shape = type, color = id)
- Meaningful symbols for data-structures
- Usage dataflow
- Re-assignment dataflow
- Dataflow join and splits

### Saving / loading manually entered 

- Using AST-identifier to store node-specific data
- Recovering based on diff from previous source
- Editor for determining collapsing/expanding groups, flew ratios

### Layout functions

- If: splits and join
- For: (init), join, split, loop, (update), end
- Straight lane: segments, sub-lanes based on data-flow analysis
- Recursive expressions/function calls (horizontal, vertical)
- Overall function (overview)
- Routing/separated contol flow (jump to different lane and back)

### Zooming/expanding

- Collapsing (function) elements together (as group)
- Scaling/straight zooming
- Sizing (with flex ratio)
- Text-overlay on multi-statements (manually written)
- Layers of zoom (levels of detail)
- Hover-over of identifiers

### Corresponding textual representation

- Expandable functions (corresponding with flow)
- Text follows flow (partial function) vs. flow follows text (full file-text)
- Text scroller, cursor, editor

### Debugging

- Introspection into visual elements (flow and text)

### Future

- Visual debugger and/or (re)play/simulation
