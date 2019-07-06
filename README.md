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

### File handling, multiple files, namespaces, context

| Feature | Progress |
| ------- | -------:|
| Upload source file or download source from url | ![green] ![yellow] ![grey] ![grey] ![grey] |
| Include other files, detect/use autoloader (build context) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Support multiple languages | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Diverse and meaningful examples | |

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

### Control and dataflow analysis

| Symmetric control elements (if, while, do, for, foreach) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Asymmetric control elements (return, throw, break, continue) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Variable/function scope | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Variable assignment | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Variable/function-output usage | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Dataflow join and splits | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Types of visual elements and connections

| Lanes: straight (with segments), splits, joins, loops, assymetric | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Boolean decider into splitter | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Containers (expressions and functions) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Function arguments/parameters + to-return/returned | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Variable identifiers on arrows | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Icons for identification (shape = type, color = id) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Meaningful symbols for data-structures | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Usage dataflow | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Re-assignment dataflow | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Dataflow join and splits | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Saving / loading manually entered data

| Using AST-identifier to store node-specific data | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Recovering based on diff from previous source | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Editor for determining collapsing/expanding groups, flex ratios | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Layout functions

| If: splits and join | ![grey] ![grey] ![grey] ![grey] ![grey] |
| For: (init), join, split, loop, (update), end | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Straight lane: segments, sub-lanes based on data-flow analysis | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Recursive expressions/function calls (horizontal, vertical) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Overall function (overview), lagers of code/architecture | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Routing/separated contol flow (jump to different lane and back) | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Zooming/expanding

| Collapsing (function) elements together (as group) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Scaling/straight zooming | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Sizing (with flex ratio) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Text-overlay on multi-statements (manually written) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Layers of zoom (levels of detail) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Hover-over of identifiers | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Corresponding textual representation

| Expandable functions (corresponding with flow) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Text follows flow (partial function) vs. flow follows text (full file-text) | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Text scroller, cursor, editor | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Syntax highlighting | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Good use/division of screen space (and resolution) | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Debugging / utils

| Memory management | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Render pipeline | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Introspection into visual elements (flow and text) | ![grey] ![grey] ![grey] ![grey] ![grey] |

### Future

| Visual debugger | ![grey] ![grey] ![grey] ![grey] ![grey] |
| Record and (re)play/simulation | ![grey] ![grey] ![grey] ![grey] ![grey] |
