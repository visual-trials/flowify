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

### Saving / loading manually entered data

- Using AST-identifier to store node-specific data
- Recovering based on diff from previous source
- Editor for determining collapsing/expanding groups, flex ratios

### Layout functions

- If: splits and join
- For: (init), join, split, loop, (update), end
- Straight lane: segments, sub-lanes based on data-flow analysis
- Recursive expressions/function calls (horizontal, vertical)
- Overall function (overview), lagers of code/architecture
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
- Syntax highlighting
- Good use/division of screen space (and resolution)

### Debugging / utils

- Memory management
- Render pipeline
- Introspection into visual elements (flow and text)

### Future

- Visual debugger
- Record and (re)play/simulation
