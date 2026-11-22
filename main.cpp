// =====================================================================
//  MiniCAD - a small 2D CAD shape modeler in C++
//  Build:  g++ -std=c++17 -Wall main.cpp -o minicad
// =====================================================================
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "Scene.h"

// Thrown when input ends (Ctrl+Z on Windows / Ctrl+D on Linux)
struct InputClosed {};

// ---------------- safe input helpers ----------------
// If the user types letters instead of a number, ask again instead of crashing.
double readNumber(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        double value;
        if (std::cin >> value) return value;
        if (std::cin.eof()) throw InputClosed();
        std::cout << "  Please enter a number.\n";
        std::cin.clear();   // reset the error flag
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');   // throw away bad input
    }
}

int readInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        int value;
        if (std::cin >> value) return value;
        if (std::cin.eof()) throw InputClosed();
        std::cout << "  Please enter a whole number.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

Point readPoint(const std::string& label) {
    std::cout << label << "\n";
    double x = readNumber("    x: ");
    double y = readNumber("    y: ");
    return Point(x, y);
}

std::string readWord(const std::string& prompt) {
    std::cout << prompt;
    std::string word;
    if (!(std::cin >> word)) throw InputClosed();
    return word;
}

// ---------------- menu ----------------
void printMenu(Scene& scene) {
    std::cout << "\n================= MiniCAD =================\n";
    std::cout << " Shapes: " << scene.count() << "    Selected: ";
    if (scene.hasSelection()) std::cout << "#" << scene.selectedId() << " " << scene.selected().name();
    else std::cout << "none";
    std::cout << "\n-------------------------------------------\n"
              << "  1) Add circle            8) Move selected\n"
              << "  2) Add rectangle         9) Scale selected\n"
              << "  3) Add polygon          10) Rotate selected\n"
              << "  4) List shapes          11) Delete selected\n"
              << "  5) Draw scene           12) Save scene to file\n"
              << "  6) Select by clicking   13) Load scene from file\n"
              << "  7) Select by ID         14) Load demo scene\n"
              << "  0) Exit\n"
              << "-------------------------------------------\n";
}

void showSelected(Scene& scene) {
    std::cout << "  #" << scene.selectedId() << " ";
    scene.selected().print(std::cout);
    std::cout << "\n";
}

void loadDemo(Scene& scene) {
    scene.clear();
    scene.add(std::make_unique<Circle>(Point(0, 0), 5));
    scene.add(Rectangle::fromCorners(Point(3, -6), Point(14, -1)));
    scene.add(std::make_unique<Polygon>(std::vector<Point>{ Point(-12, -5), Point(-4, -5), Point(-8, 4) }));
    scene.add(std::make_unique<Rectangle>(Point(10, 5), 6, 3, 30));   // a rotated rectangle
}

int main() {
    std::cout << std::fixed << std::setprecision(2);   // print numbers like 3.14
    Scene scene;

    std::cout << "Welcome to MiniCAD! Tip: choose 14 to load a demo scene, then 5 to draw it.\n";

    try {
        while (true) {
            printMenu(scene);
            int choice = readInt("Choose an option: ");

            // Any problem inside an action (bad radius, no selection, missing
            // file...) throws an exception. We catch it, show the message,
            // and go back to the menu instead of crashing.
            try {
                switch (choice) {
                case 0:
                    std::cout << "Goodbye!\n";
                    return 0;

                case 1: {
                    Point c = readPoint("  Center:");
                    double r = readNumber("  Radius: ");
                    scene.add(std::make_unique<Circle>(c, r));
                    std::cout << "  Added:";
                    showSelected(scene);
                    break;
                }
                case 2: {
                    Point a = readPoint("  First corner:");
                    Point b = readPoint("  Opposite corner:");
                    scene.add(Rectangle::fromCorners(a, b));
                    std::cout << "  Added:";
                    showSelected(scene);
                    break;
                }
                case 3: {
                    int n = readInt("  Number of vertices (3 or more): ");
                    if (n < 3) throw std::invalid_argument("A polygon needs at least 3 vertices");
                    std::vector<Point> pts;
                    for (int i = 1; i <= n; i++) {
                        pts.push_back(readPoint("  Vertex " + std::to_string(i) + ":"));
                    }
                    scene.add(std::make_unique<Polygon>(pts));
                    std::cout << "  Added:";
                    showSelected(scene);
                    break;
                }
                case 4:
                    scene.list(std::cout);
                    break;

                case 5:
                    scene.draw(std::cout);
                    break;

                case 6: {
                    Point p = readPoint("  Click at:");
                    if (scene.selectAt(p)) {
                        std::cout << "  Hit! Selected:";
                        showSelected(scene);
                    } else {
                        std::cout << "  No shape there. Selection cleared.\n";
                    }
                    break;
                }
                case 7: {
                    int id = readInt("  Shape ID: ");
                    if (scene.selectById(id)) {
                        std::cout << "  Selected:";
                        showSelected(scene);
                    } else {
                        std::cout << "  No shape with ID " << id << ".\n";
                    }
                    break;
                }
                case 8: {
                    Shape& s = scene.selected();
                    double dx = readNumber("  Move by dx: ");
                    double dy = readNumber("  Move by dy: ");
                    s.move(dx, dy);
                    showSelected(scene);
                    break;
                }
                case 9: {
                    Shape& s = scene.selected();
                    s.scale(readNumber("  Scale factor (e.g. 2 = double, 0.5 = half): "));
                    showSelected(scene);
                    break;
                }
                case 10: {
                    Shape& s = scene.selected();
                    s.rotate(readNumber("  Rotate by degrees (counter-clockwise): "));
                    showSelected(scene);
                    break;
                }
                case 11: {
                    int id = scene.selectedId();
                    scene.deleteSelected();
                    std::cout << "  Deleted shape #" << id << ".\n";
                    break;
                }
                case 12: {
                    std::string file = readWord("  File name to save (e.g. scene.txt): ");
                    scene.save(file);
                    std::cout << "  Saved " << scene.count() << " shape(s) to " << file << "\n";
                    break;
                }
                case 13: {
                    std::string file = readWord("  File name to load: ");
                    scene.load(file);
                    std::cout << "  Loaded " << scene.count() << " shape(s) from " << file << "\n";
                    break;
                }
                case 14:
                    loadDemo(scene);
                    std::cout << "  Demo scene loaded (4 shapes). Choose 5 to draw it.\n";
                    break;

                default:
                    std::cout << "  Unknown option.\n";
                }
            } catch (const std::exception& e) {
                std::cout << "  Error: " << e.what() << "\n";
            }
        }
    } catch (const InputClosed&) {
        std::cout << "\nInput closed. Goodbye!\n";
    }
    return 0;
}
