# Execution Architecture Documentation

This document explains the overarching workflow of how user code is executed, traced, and visualized within the **Algorithm Visualizer Pro** application.

## 1. Triggering Execution
When the "Run" button is clicked in the GUI, the `MainWindow::onExecuteClicked()` event is triggered:
- The Python code written inside the GUI text editor (`m_codeEditor`) is captured.
- This code is dynamically saved to a temporary Python file on the OS (e.g., `algovisXXXXXX.py`).
- The application automatically searches the directory tree to resolve the absolute path to the backend Python-tracing helper script (`python_tracer.py`).

## 2. The Python Tracing Process
The C++ application uses Qt's `QProcess` to spawn a secure, isolated external background process running the system's underlying Python interpreter.
```bash
python python_tracer.py <temp_file.py>
```

All Python execution tracking happens entirely within `python_tracer.py`.
- **`sys.settrace()` Hook**: The Python script uses this built-in Python module to attach an `ExecutionTracer` class to the interpreter. This acts as a hook, executing a callback method just before every single line of code is executed, a function is called, or an iteration starts.
- **Variable Capture**: For each line of code executed in the user's code frame, the tracer captures:
  - `line`: The current script line number (`frame.f_lineno`).
  - `event`: The type of event executing (`line`, `call`, or `return`).
  - `func`: The current scope function name.
  - `variables`: A dictionary of all active local variables scoped in the frame (`frame.f_locals`).
- **Serialization**: Variable states are sanitized and serialized. Complex structures fallback to string representations `repr()` to prevent serialization crashes. Functions/modules are filtered out.
- **JSON Output**: Once the user's execution terminates successfully or throws a runtime error, the tracer bundles every captured frame into a large JSON Array and prints it entirely to `stdout`.

## 3. Parsing and Loading the Timeline
Back inside the C++ UI context (`mainwindow.cpp`):
- `QProcess` waits asynchronously for the process to finish and listens to `readyReadStandardOutput`.
- Once the Python script completes its execution, the Qt application collects the resulting console output buffer and parses the JSON payload using `QJsonDocument`.
- The parsed JSON array becomes a literal **Execution Timeline containing $N$ steps** of states.
- This chronological payload is injected into the visualization controller via `m_visualizer->loadSteps(stepsArray)`.

## 4. Playback and GUI Rendering
Rather than executing and visualizing code in real-time, the Qt application acts like a media player stepping through recorded frames.
- A `QTimer` thread (`m_playTimer`) iterates sequentially based on the selected GUI playback speed (e.g., 50ms - 2000ms delay).
- On each timer tick, `ASTVisualizer::executeStep()` pulls the next JSON snapshot from the dataset.

### Visual State Mapping (`ASTVisualizer::processStep`)
The `ASTVisualizer` unpacks the JSON variables payload and determines how it should be rendered logically:
- **Code Highlighting**: `m_codeHighlighter->highlightLine(step.line)` highlights the line overlay on the C++ editor to match the active recorded tick.
- **Automatic Heuristics**: The variable names are scanned dynamically to auto-trigger rich components.
   - If an array is observed alongside index variables (`i`, `j`, `left`, `right`) it automatically transitions to **Sorting Mode**, mapping the indices explicitly to golden array highlights.
   - If dict structures contain keys like `value`, `left` and `right`, it constructs and paints dynamic **Binary Trees**.
   - If dict structures interact with state variables like `visited`, `current`, `frontier`, it paints **Graphs**.
- **Syncing UI Items**: Graphic interfaces (`syncVariable()`, `syncArray()`, `syncStack()`, `syncGraph()`) extract values and smoothly animate arrays, text items, nodes, and variables across the `QGraphicsScene`. Old instances are updated in place to appear interactive.
