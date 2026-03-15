# Algorithm Visualization Tool

A desktop application that visualizes the execution of Python programs step-by-step.
The system traces Python code using `sys.settrace()` and renders the program state using a C++ Qt visualization engine.

The tool helps understand algorithms, recursion, and data structures by showing how variables, arrays, graphs, and call stacks change during execution.

---

## Features

* Step-by-step visualization of Python program execution
* Automatic detection of variables, arrays, and graphs
* Sorting visualization with animated bars
* Recursion call stack visualization
* Graph visualization using adjacency lists
* Console output display
* Line-by-line code highlighting
* Animated execution playback

---

## System Architecture

```
Python Code
     │
     ▼
Python Tracer (sys.settrace)
     │
     ▼
Execution Steps → JSON
     │
     ▼
C++ Qt Visualization Engine
     │
     ▼
Animated Algorithm Visualization
```

---

## How It Works

1. The user writes Python code in the editor.
2. When **Run** is clicked:

   * The code is saved to a temporary `.py` file.
3. A Python tracing script executes the program using `sys.settrace()`.
4. The tracer captures:

   * current line number
   * local variables
   * arrays
   * recursion stack
   * console output
5. Each execution step is converted into JSON.
6. The C++ Qt visualizer reads the JSON and renders:

   * variables
   * arrays
   * graphs
   * recursion stack
7. The program execution is replayed step-by-step using animations.

---

## Example Visualization

### Array Visualization

```
[5] [3] [1]
```

### Sorting Visualization

```
|       |
| |     |
| | |   |
```

### Recursion Stack

```
factorial(5)
factorial(4)
factorial(3)
```

---

## Technologies Used

* **C++**
* **Qt (QGraphicsScene / QGraphicsView)**
* **Python**
* **sys.settrace()**
* **JSON**

---

## Project Structure

```
AlgorithmVisualization
│
├── mainwindow.cpp
├── ASTVisualizer.cpp
├── PythonParserEngine.cpp
├── python_tracer.py
│
├── Widgets/
│   └── code_highlighter.cpp
│
└── README.md
```

---

## Installation

### Requirements

* Qt (Qt Creator recommended)
* Python 3.x
* C++17 compatible compiler

### Steps

1. Clone the repository

```
git clone https://github.com/VishwaPriyan-S/AlgorithmVisualization.git
```

2. Open the project in **Qt Creator**

3. Build the project

4. Ensure the Python tracer script is located in the executable directory.

5. Run the application

---

## Usage

1. Write Python code in the editor.
2. Select visualization mode:

   * Generic
   * Sorting
   * Recursion
   * Graph
3. Click **Run**.
4. Watch the execution visualized step-by-step.

---

## Future Improvements

* Support for more data structures
* Breakpoints
* Interactive debugging
* Additional algorithm visualizations
* Web version

---
