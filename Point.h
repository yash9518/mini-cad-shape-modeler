#pragma once   // include this file only once, even if #included many times
#include <cmath>
#include <iostream>

const double PI = 3.14159265358979323846;

// Small tolerance so a point lying exactly on an edge counts as "inside"
// (floating point maths is never perfectly exact)
const double EPSILON = 1e-9;

// A 2D point / position in the drawing
class Point {
public:
    double x;
    double y;

    // Constructor with a member initializer list
    Point(double xValue, double yValue) : x(xValue), y(yValue) {}

    // 'const' at the end = this function promises NOT to change the point
    double distanceTo(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Returns a NEW point: this point rotated around 'pivot'
    // counter-clockwise by 'degrees' (standard 2D rotation formula)
    Point rotatedAround(const Point& pivot, double degrees) const {
        double rad = degrees * PI / 180.0;
        double dx = x - pivot.x;
        double dy = y - pivot.y;
        return Point(pivot.x + dx * std::cos(rad) - dy * std::sin(rad),
                     pivot.y + dx * std::sin(rad) + dy * std::cos(rad));
    }
};

// Operator overloading: lets us write   std::cout << point;
inline std::ostream& operator<<(std::ostream& out, const Point& p) {
    // turn tiny rounding errors like -0.0000000001 into a clean 0
    double x = std::fabs(p.x) < EPSILON ? 0.0 : p.x;
    double y = std::fabs(p.y) < EPSILON ? 0.0 : p.y;
    out << "(" << x << ", " << y << ")";
    return out;
}
