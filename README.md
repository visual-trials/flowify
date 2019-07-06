# Flowify

## Goal

Flowify visualizes existing code.

The goal is to do this in a way that it will give a clear overview and insight into a (unfamiliar) codebase.

https://flowify.io/

## Features to implement

[green]: /web/img/green_bar.png
[yellow]: /web/img/yellow_bar.png
[red]: /web/img/red_bar.png
[grey]: /web/img/grey_bar.png
[blue]: /web/img/blue_bar.png

### File handling, multiple files, namespaces, context

| Feature | Progress |
| ------- | -------:|
| Upload source file or download source from url | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Include other files, detect/use autoloader (build context) | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Support multiple languages | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Diverse and meaningful examples | ![green] ![green] ![blue] ![grey] ![grey] |

### Parsing of language elements

| Feature | Progress |
| ------- | -------:|
| Control statements | ![green] ![green] ![yellow] ![grey] ![grey] |
| Expressions      |  ![green] ![green] ![grey] ![grey] ![grey] |
| Declarations |  ![red] ![grey] ![grey] ![grey] ![grey] |
| Assignments |  ![green] ![green] ![yellow] ![grey] ![grey] |
| Variables, Scalars |  ![green] ![green] ![green] ![green] ![grey] |
| Functions |  ![green] ![green] ![green] ![grey] ![grey] |
| Classes, properties, methods, inheritance, traits |  ![grey] ![grey] ![grey] ![grey] ![grey] |
| Interfaces, namespaces |  ![grey] ![grey] ![grey] ![grey] ![grey] |

### Saving / loading manually entered data

| Feature | Progress |
| ------- | -------:|
| Using AST-identifier to store/load node-specific data | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Recovering based on diff from previous source | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Editor for determining collapsing/expanding groups, flex ratios | ![blue] ![grey] ![grey] ![grey] ![grey] |

### Control and dataflow analysis

| Feature | Progress |
| ------- | -------:|
| Symmetric control elements (if, while, do, for, foreach) | ![green] ![green] ![yellow] ![grey] ![grey] |
| Asymmetric control elements (return, throw, break, continue) | ![red] ![grey] ![grey] ![grey] ![grey] |
| Variable/function scope | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Variable assignment | ![grey] ![yellow] ![grey] ![grey] ![grey] |
| Variable/function-output usage | ![yellow] ![grey] ![grey] ![grey] ![grey] |
| Dataflow join and splits | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Types of visual elements and connections

| Feature | Progress |
| ------- | -------:|
| Lanes: straight (with segments) | ![green] ![yellow] ![grey] ![grey] ![grey] |
| Lanes: splits (+decider bool), joins, loops, assymetric | ![green] ![yellow] ![grey] ![grey] ![grey] |
| Containers (expressions and functions) | ![green] ![green] ![grey] ![grey] ![grey] |
| Function arguments/parameters + to-return/returned | ![green] ![yellow] ![grey] ![grey] ![grey] |
| Variable identifiers on arrows | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Icons for identification (shape = type, color = id) | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Meaningful symbols for data-structures | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Usage dataflow | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Re-assignment dataflow | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Dataflow join and splits | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Layout functions

| Feature | Progress |
| ------- | -------:|
| If: splits and join | ![green] ![green] ![grey] ![grey] ![grey] |
| For: (init), join, split, loop, (update), end | ![green] ![green] ![grey] ![grey] ![grey] |
| Straight lane: segments, sub-lanes based on data-flow analysis | ![green] ![yellow] ![grey] ![grey] ![grey] |
| Recursive expressions/function calls (horizontal, vertical) | ![green] ![red] ![grey] ![grey] ![grey] |
| Overall function (overview), lagers of code/architecture | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Routing/separated contol flow (jump to different lane and back) | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Zooming/expanding

| Feature | Progress |
| ------- | -------:|
| Collapsing (function) elements together (as group) | ![yellow] ![grey] ![grey] ![grey] ![grey] |
| Scaling/straight zooming | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Sizing (with flex ratio) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Text-overlay on multi-statements (manually written) | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Layers of zoom (levels of detail) | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Hover-over of identifiers | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Corresponding textual representation

| Feature | Progress |
| ------- | -------:|
| Expandable functions (corresponding with flow) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Text follows flow (partial function) | ![green] ![yellow] ![grey] ![grey] ![grey] |
| Flow follows text (full file-text) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Text scroller, cursor, editor | ![green] ![yellow] ![grey] ![grey] ![grey] |
| Syntax highlighting | ![blue] ![grey] ![grey] ![grey] ![grey] |
| Good use/division of screen space (and resolution) | ![yellow] ![blue] ![grey] ![grey] ![grey] |

### Debugging / utils

| Feature | Progress |
| ------- | -------:|
| Memory management | ![green] ![grey] ![grey] ![grey] ![grey] |
| Render pipeline | ![green] ![grey] ![grey] ![grey] ![grey] |
| Introspection into visual elements (flow and text) | ![green] ![grey] ![grey] ![grey] ![grey] |

### Future

| Feature | Progress |
| ------- | -------:|
| Visual debugger | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Record and (re)play/simulation | ![grey] ![grey] ![grey] ![grey] ![grey] |
