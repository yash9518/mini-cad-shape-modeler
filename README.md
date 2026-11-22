# MiniCAD – 2D Shape Modeler in C++

A command-line 2D CAD-style shape modeler written in modern C++ (C++17).
Create circles, rectangles and polygons, select them by clicking a point,
move / scale / rotate them, see the drawing as ASCII art, and save or load
the whole scene from a file.

## Features

- **Shapes:** Circle, Rectangle (rotatable), Polygon (any number of vertices)
- **Geometry:** area, perimeter, Euclidean distance, bounding boxes
- **Hit-testing / selection:** click a point to select the top-most shape under it
  - Circle: distance to center
  - Rectangle: point rotated into the rectangle's local frame
  - Polygon: ray-casting algorithm
- **Transformations:** move, scale (about the shape's center), rotate (counter-clockwise)
- **Scene management:** add, list, select by click or ID, delete
- **ASCII renderer:** draws the scene in the terminal, auto-zoomed to fit
- **Save / load:** plain-text scene file. A broken file is rejected with its line
  number, and the current scene is left unchanged.
- **Input validation:** bad numbers, negative sizes and zero-area polygons are rejected cleanly

## Build & run

```bash
g++ -std=c++17 -Wall main.cpp -o minicad
./minicad          # Windows: minicad.exe
```

Quick start: choose `14` (load demo scene), then `5` (draw).

## Design

```
Point.h    Point class: distance, rotation, operator<<
Shape.h    abstract base class Shape (pure virtual interface) + Box
Shapes.h   Circle, Rectangle, Polygon (inherit from Shape) + parseShape() factory
Scene.h    Scene: owns shapes via std::vector<std::unique_ptr<Shape>>
main.cpp   menu-driven user interface
```

| Concept | Where |
|---|---|
| Encapsulation | private data members in every shape |
| Composition | shapes are built from `Point` objects |
| Inheritance & polymorphism | `Circle`, `Rectangle`, `Polygon` override `Shape`'s virtual functions |
| Smart pointers / RAII | `std::unique_ptr<Shape>` – no manual `new`/`delete` |
| Factory pattern | `parseShape()`, `Rectangle::fromCorners()` |
| Exception handling | invalid input and file errors throw, and the menu catches them |
| File I/O | `Scene::save()` / `Scene::load()` |

## Scene file format

```
# MiniCAD scene file
CIRCLE 0 0 5                    # x y radius
RECTANGLE 8.5 -3.5 11 5 0       # centerX centerY width height angle
POLYGON 3 -12 -5 -4 -5 -8 4     # n x1 y1 x2 y2 ... xn yn
```
