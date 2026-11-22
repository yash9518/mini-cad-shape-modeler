#pragma once
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "Shapes.h"

// =====================================================================
// SCENE: the whole drawing = a list of shapes + which one is selected.
//
// std::unique_ptr<Shape> is a "smart pointer":
//   - it OWNS the shape and deletes it automatically when removed
//     -> no 'new'/'delete' by hand, no memory leaks
//   - it can't be copied, only moved (std::move) -> exactly one owner
// =====================================================================
class Scene {
private:
    std::vector<std::unique_ptr<Shape>> shapes;
    int selectedIndex = -1;   // -1 means nothing is selected

    // Character used to draw shape number i (1-9, then A-Z)
    static char symbolFor(int i) {
        if (i < 9) return static_cast<char>('1' + i);
        if (i < 9 + 26) return static_cast<char>('A' + (i - 9));
        return '?';
    }

public:
    // ---------------- adding / counting ----------------
    void add(std::unique_ptr<Shape> shape) {
        shapes.push_back(std::move(shape));   // ownership moves into the vector
        selectedIndex = static_cast<int>(shapes.size()) - 1;   // new shape becomes selected
    }

    int count() const { return static_cast<int>(shapes.size()); }

    void clear() {
        shapes.clear();          // unique_ptr deletes every shape for us
        selectedIndex = -1;
    }

    double totalArea() const {
        double total = 0;
        for (const auto& s : shapes) total += s->area();
        return total;
    }

    // ---------------- selection ----------------
    bool hasSelection() const { return selectedIndex != -1; }
    int selectedId() const { return selectedIndex + 1; }   // IDs shown to the user start at 1

    Shape& selected() {
        if (!hasSelection()) {
            throw std::runtime_error("No shape selected. Select one first.");
        }
        return *shapes[selectedIndex];
    }

    bool selectById(int id) {
        if (id < 1 || id > count()) return false;
        selectedIndex = id - 1;
        return true;
    }

    // "Click" selection: search from the LAST shape (drawn on top) to the
    // first, and pick the top-most shape under the point.
    bool selectAt(const Point& p) {
        for (int i = count() - 1; i >= 0; --i) {
            if (shapes[i]->contains(p)) {
                selectedIndex = i;
                return true;
            }
        }
        selectedIndex = -1;   // clicked on empty space -> deselect
        return false;
    }

    void deleteSelected() {
        selected();   // throws if nothing is selected
        shapes.erase(shapes.begin() + selectedIndex);   // unique_ptr frees memory
        selectedIndex = -1;
    }

    // ---------------- output ----------------
    void list(std::ostream& out) const {
        if (shapes.empty()) {
            out << "  (scene is empty)\n";
            return;
        }
        for (int i = 0; i < count(); i++) {
            out << (i == selectedIndex ? " * " : "   ") << std::setw(2) << (i + 1) << ". ";
            shapes[i]->print(out);
            out << "\n        area=" << shapes[i]->area()
                << "  perimeter=" << shapes[i]->perimeter() << "\n";
        }
        out << "  " << count() << " shape(s), total area = " << totalArea()
            << "   (* = selected)\n";
    }

    // Draws the scene as ASCII art by asking each shape "do you contain
    // this point?" for the center of every character cell.
    void draw(std::ostream& out, int cols = 60, int rows = 22) const {
        if (shapes.empty()) {
            out << "  (scene is empty)\n";
            return;
        }

        // 1. Find the area of the world that contains every shape
        Box world = shapes[0]->boundingBox();
        for (const auto& s : shapes) world.include(s->boundingBox());
        double midX = (world.minX + world.maxX) / 2;
        double midY = (world.minY + world.maxY) / 2;

        // 2. Choose a zoom so it all fits. A terminal character is about
        //    twice as tall as it is wide, so rows cover 2x the distance.
        double unitsPerCol = std::max((world.maxX - world.minX) / (cols - 4),
                                      (world.maxY - world.minY) / ((rows - 2) * 2.0));
        if (unitsPerCol <= 0) unitsPerCol = 1;
        double unitsPerRow = unitsPerCol * 2;

        // 3. Paint every cell
        out << "  +" << std::string(cols, '-') << "+\n";
        for (int r = 0; r < rows; r++) {
            double y = midY + (rows / 2.0 - r - 0.5) * unitsPerRow;   // top row = highest y
            out << "  |";
            for (int c = 0; c < cols; c++) {
                double x = midX + (c - cols / 2.0 + 0.5) * unitsPerCol;
                char ch = ' ';
                for (int i = count() - 1; i >= 0; --i) {   // top-most shape wins
                    if (shapes[i]->contains(Point(x, y))) {
                        ch = (i == selectedIndex) ? '#' : symbolFor(i);
                        break;
                    }
                }
                out << ch;
            }
            out << "|\n";
        }
        out << "  +" << std::string(cols, '-') << "+\n";
        out << "  Symbols = shape ID,  # = selected shape\n";
    }

    // ---------------- file save / load ----------------
    // File format: one shape per line, e.g.
    //   CIRCLE 0 0 5
    //   RECTANGLE 3 2 4 2 45
    //   POLYGON 3 0 0 4 0 0 3
    void save(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file) throw std::runtime_error("Could not open '" + filename + "' for writing");

        file << "# MiniCAD scene file\n";
        file << std::setprecision(15);   // keep full accuracy in the file
        for (const auto& s : shapes) {
            s->save(file);
            file << "\n";
        }
        if (!file) throw std::runtime_error("Error while writing '" + filename + "'");
    }

    void load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) throw std::runtime_error("Could not open '" + filename + "'");

        // Read into a TEMPORARY list first. If any line is broken we throw
        // before touching the current scene, so the user loses nothing.
        std::vector<std::unique_ptr<Shape>> loaded;
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            ++lineNumber;
            size_t first = line.find_first_not_of(" \t\r");
            if (first == std::string::npos || line[first] == '#') continue;   // blank / comment
            try {
                loaded.push_back(parseShape(line));
            } catch (const std::exception& e) {
                throw std::runtime_error(filename + ", line " + std::to_string(lineNumber) + ": " + e.what());
            }
        }

        shapes = std::move(loaded);   // success -> replace the scene
        selectedIndex = -1;
    }
};
