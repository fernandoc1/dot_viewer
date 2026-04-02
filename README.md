# DOT Graph Viewer

A Qt6 application for visualizing large DOT graph files, specifically designed for analyzing program execution flow graphs.

## Features

- **Memory-efficient parsing** - Uses memory-mapped file I/O for handling large DOT files (tested with 15,000+ nodes)
- **Tree-based navigation** - Expand/collapse nodes to explore the graph incrementally
- **Graph visualization** - Visual tree display with nodes and edges
- **Search functionality** - Find nodes by address, instruction, or node ID (case-insensitive)
- **Configurable neighbor limit** - Control how many successors/predecessors to display per node
- **Node details panel** - View full label, execution count, and connection statistics
- **Zoom and pan controls** - Navigate large graphs with mouse wheel and drag

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Usage

### Run the application
```bash
./build/dot_viewer
```

### Load a DOT file directly
```bash
./build/dot_viewer runtime_cfg.dot
```

### Interface

1. **Load File** - Click "Load DOT File" or use File → Open (Ctrl+O)

2. **Tree View Tab** - Text-based hierarchical view
   - Double-click any node to expand/collapse its neighbors
   - **Successors** (outgoing edges) shown with →
   - **Predecessors** (incoming edges) shown with ←

3. **Graph Visualization Tab** - Visual tree display
   - Nodes displayed as rounded rectangles with address, instruction, and count
   - Edges shown as arrows (dashed red for back-edges/cycles)
   - **Double-click** a node to re-root the tree on that node
   - **Zoom**: Ctrl + mouse wheel, or +/- keys
   - **Pan**: Middle mouse button drag, or hand tool
   - **Fit to view**: Press 0 (zero) key
   - **Center**: Home key

4. **Search** - Enter address, instruction, or node ID in the search box
   - Case-insensitive search
   - Click results to navigate to that node

5. **Controls**
   - **Neighbors** - Adjust how many neighbors to show in tree view (default: 20)
   - **Tree depth** - Maximum depth for graph visualization (default: 3)

## Example

The included `runtime_cfg.dot` file is a control flow graph with:
- 15,940 nodes
- 16,907 edges
- Assembly instructions with execution counts

## Project Structure

```
dot_viewer/
├── CMakeLists.txt          # CMake build configuration
├── include/
│   ├── dotparser.h         # DOT file parser
│   ├── graphmodel.h        # Tree model for Qt view
│   ├── graphview.h         # Graph visualization widget
│   └── mainwindow.h        # Main application window
├── src/
│   ├── main.cpp            # Application entry point
│   ├── dotparser.cpp       # Parser implementation
│   ├── graphmodel.cpp      # Tree model implementation
│   ├── graphview.cpp       # Graph visualization implementation
│   └── mainwindow.cpp      # UI implementation
└── runtime_cfg.dot         # Example DOT file
```

## Requirements

- Qt 6.x (Core, Gui, Widgets)
- CMake 3.16+
- C++17 compatible compiler

## License

MIT License
