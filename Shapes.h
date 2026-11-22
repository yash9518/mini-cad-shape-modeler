#pragma once
#include <cmath>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "Shape.h"

// =====================================================================
// CIRCLE
// =====================================================================
class Circle : public Shape {          // Circle IS A Shape (inheritance)
private:                               // encapsulation: data hidden inside
    Point centerPoint;                 // Circle HAS A Point (composition)
    double radius;

public:
    Circle(const Point& c, double r) : centerPoint(c), radius(r) {
        requirePositive(r, "Circle radius");
    }

    std::string name() const override { return "Circle"; }
    double area() const override      { return PI * radius * radius; }
    double perimeter() const override { return 2 * PI * radius; }
    Point center() const override     { return centerPoint; }

    Box boundingBox() const override {
        return { centerPoint.x - radius, centerPoint.y - radius,
                 centerPoint.x + radius, centerPoint.y + radius };
    }

    bool contains(const Point& p) const override {
        return centerPoint.distanceTo(p) <= radius + EPSILON;
    }

    void move(double dx, double dy) override {
        centerPoint.x += dx;
        centerPoint.y += dy;
    }

    void scale(double factor) override {
        requirePositive(factor, "Scale factor");
        radius *= factor;
    }

    void rotate(double) override {
        // a circle rotated around its own center looks exactly the same
    }

    void print(std::ostream& out) const override {
        out << "Circle     center=" << centerPoint << "  radius=" << radius;
    }

    void save(std::ostream& out) const override {
        out << "CIRCLE " << centerPoint.x << " " << centerPoint.y << " " << radius;
    }
};

// =====================================================================
// RECTANGLE
// Stored as center + width + height + angle, so it can be rotated.
// =====================================================================
class Rectangle : public Shape {
private:
    Point centerPoint;
    double width;
    double height;
    double angle;   // degrees, counter-clockwise

public:
    Rectangle(const Point& c, double w, double h, double angleDegrees = 0)
        : centerPoint(c), width(w), height(h), angle(angleDegrees) {
        requirePositive(w, "Rectangle width");
        requirePositive(h, "Rectangle height");
    }

    // "Factory" helper: build an upright rectangle from two opposite corners
    static std::unique_ptr<Rectangle> fromCorners(const Point& a, const Point& b) {
        Point mid((a.x + b.x) / 2, (a.y + b.y) / 2);
        return std::make_unique<Rectangle>(mid, std::fabs(b.x - a.x), std::fabs(b.y - a.y));
    }

    // The 4 corners in world coordinates (after rotation)
    std::vector<Point> corners() const {
        double hw = width / 2, hh = height / 2;
        std::vector<Point> result;
        result.push_back(Point(centerPoint.x - hw, centerPoint.y - hh).rotatedAround(centerPoint, angle));
        result.push_back(Point(centerPoint.x + hw, centerPoint.y - hh).rotatedAround(centerPoint, angle));
        result.push_back(Point(centerPoint.x + hw, centerPoint.y + hh).rotatedAround(centerPoint, angle));
        result.push_back(Point(centerPoint.x - hw, centerPoint.y + hh).rotatedAround(centerPoint, angle));
        return result;
    }

    std::string name() const override { return "Rectangle"; }
    double area() const override      { return width * height; }
    double perimeter() const override { return 2 * (width + height); }
    Point center() const override     { return centerPoint; }

    Box boundingBox() const override {
        std::vector<Point> c = corners();
        Box box = { c[0].x, c[0].y, c[0].x, c[0].y };
        for (const Point& p : c) box.include({ p.x, p.y, p.x, p.y });
        return box;
    }

    // Trick: rotate the point BACKWARDS into the rectangle's own frame,
    // then it is a simple "upright rectangle" check.
    bool contains(const Point& p) const override {
        Point local = p.rotatedAround(centerPoint, -angle);
        return std::fabs(local.x - centerPoint.x) <= width / 2 + EPSILON &&
               std::fabs(local.y - centerPoint.y) <= height / 2 + EPSILON;
    }

    void move(double dx, double dy) override {
        centerPoint.x += dx;
        centerPoint.y += dy;
    }

    void scale(double factor) override {
        requirePositive(factor, "Scale factor");
        width *= factor;
        height *= factor;
    }

    void rotate(double degrees) override {
        angle = std::fmod(angle + degrees, 360.0);   // keep it in (-360, 360)
        if (angle < 0) angle += 360.0;               // then in [0, 360)
    }

    void print(std::ostream& out) const override {
        out << "Rectangle  center=" << centerPoint << "  size=" << width << " x " << height
            << "  angle=" << angle << " deg";
    }

    void save(std::ostream& out) const override {
        out << "RECTANGLE " << centerPoint.x << " " << centerPoint.y << " "
            << width << " " << height << " " << angle;
    }
};

// =====================================================================
// POLYGON: any closed shape with straight edges (triangle, pentagon...)
// =====================================================================
class Polygon : public Shape {
private:
    std::vector<Point> vertices;   // corners, in order around the shape

    // Shoelace formula: walk around the corners adding x1*y2 - x2*y1
    static double shoelaceArea(const std::vector<Point>& v) {
        double sum = 0;
        size_t n = v.size();
        for (size_t i = 0; i < n; i++) {
            const Point& a = v[i];
            const Point& b = v[(i + 1) % n];   // % n -> last corner joins the first
            sum += a.x * b.y - b.x * a.y;
        }
        return std::fabs(sum) / 2;
    }

public:
    explicit Polygon(const std::vector<Point>& v) : vertices(v) {
        if (v.size() < 3) {
            throw std::invalid_argument("A polygon needs at least 3 vertices");
        }
        if (shoelaceArea(v) < EPSILON) {
            throw std::invalid_argument("Polygon has zero area (all points on one line?)");
        }
    }

    std::string name() const override { return "Polygon"; }
    double area() const override      { return shoelaceArea(vertices); }

    double perimeter() const override {
        double total = 0;
        for (size_t i = 0; i < vertices.size(); i++) {
            total += vertices[i].distanceTo(vertices[(i + 1) % vertices.size()]);
        }
        return total;
    }

    // center = average of all corners (used as the pivot for scale/rotate)
    Point center() const override {
        double sx = 0, sy = 0;
        for (const Point& p : vertices) { sx += p.x; sy += p.y; }
        return Point(sx / vertices.size(), sy / vertices.size());
    }

    Box boundingBox() const override {
        Box box = { vertices[0].x, vertices[0].y, vertices[0].x, vertices[0].y };
        for (const Point& p : vertices) box.include({ p.x, p.y, p.x, p.y });
        return box;
    }

    // Ray casting: shoot a ray from p to the right and count how many
    // edges it crosses. Odd number of crossings -> p is inside.
    bool contains(const Point& p) const override {
        bool inside = false;
        size_t n = vertices.size();
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            const Point& a = vertices[i];
            const Point& b = vertices[j];
            if ((a.y > p.y) != (b.y > p.y)) {   // this edge crosses p's row
                double crossX = a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y);
                if (p.x < crossX) inside = !inside;
            }
        }
        return inside;
    }

    void move(double dx, double dy) override {
        for (Point& p : vertices) {   // Point& -> change the real vertex, not a copy
            p.x += dx;
            p.y += dy;
        }
    }

    void scale(double factor) override {
        requirePositive(factor, "Scale factor");
        Point c = center();
        for (Point& p : vertices) {
            p.x = c.x + (p.x - c.x) * factor;
            p.y = c.y + (p.y - c.y) * factor;
        }
    }

    void rotate(double degrees) override {
        Point c = center();
        for (Point& p : vertices) p = p.rotatedAround(c, degrees);
    }

    void print(std::ostream& out) const override {
        out << "Polygon    " << vertices.size() << " vertices:";
        for (const Point& p : vertices) out << " " << p;
    }

    void save(std::ostream& out) const override {
        out << "POLYGON " << vertices.size();
        for (const Point& p : vertices) out << " " << p.x << " " << p.y;
    }
};

// =====================================================================
// FACTORY FUNCTION: turn one line of a scene file back into a shape.
// Returns unique_ptr<Shape>, so the caller doesn't need to know which
// concrete class was created (that's polymorphism again).
// =====================================================================
inline std::unique_ptr<Shape> parseShape(const std::string& line) {
    std::istringstream in(line);
    std::string type;
    in >> type;

    if (type == "CIRCLE") {
        double x, y, r;
        if (!(in >> x >> y >> r)) throw std::runtime_error("expected: CIRCLE x y radius");
        return std::make_unique<Circle>(Point(x, y), r);
    }
    if (type == "RECTANGLE") {
        double x, y, w, h, a;
        if (!(in >> x >> y >> w >> h >> a))
            throw std::runtime_error("expected: RECTANGLE x y width height angle");
        return std::make_unique<Rectangle>(Point(x, y), w, h, a);
    }
    if (type == "POLYGON") {
        int n;
        if (!(in >> n) || n < 3) throw std::runtime_error("expected: POLYGON n x1 y1 x2 y2 ... (n >= 3)");
        std::vector<Point> pts;
        for (int i = 0; i < n; i++) {
            double x, y;
            if (!(in >> x >> y)) throw std::runtime_error("POLYGON has fewer points than it says");
            pts.push_back(Point(x, y));
        }
        return std::make_unique<Polygon>(pts);
    }
    throw std::runtime_error("unknown shape type '" + type + "'");
}
