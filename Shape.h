#pragma once
#include <iostream>
#include <stdexcept>
#include <string>
#include "Point.h"

// Axis-aligned bounding box: the smallest upright rectangle around a shape.
// Used to work out how much of the drawing to show on screen.
struct Box {
    double minX, minY, maxX, maxY;

    // grow this box so it also covers 'other'
    void include(const Box& other) {
        if (other.minX < minX) minX = other.minX;
        if (other.minY < minY) minY = other.minY;
        if (other.maxX > maxX) maxX = other.maxX;
        if (other.maxY > maxY) maxY = other.maxY;
    }
};

// ---------------------------------------------------------------------
// ABSTRACT BASE CLASS
// Describes WHAT every shape can do, not HOW.
//   virtual ... = 0   -> "pure virtual": every child class MUST write it
// Because of these, a plain Shape object can never be created.
// ---------------------------------------------------------------------
class Shape {
public:
    // Virtual destructor: shapes are deleted through Shape pointers
    // (inside unique_ptr<Shape>), so the child's destructor must run too.
    virtual ~Shape() = default;

    // --- information ---
    virtual std::string name() const = 0;
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual Point center() const = 0;
    virtual Box boundingBox() const = 0;

    // --- hit-testing: is point p inside (or on the edge of) this shape? ---
    virtual bool contains(const Point& p) const = 0;

    // --- transformations ---
    virtual void move(double dx, double dy) = 0;
    virtual void scale(double factor) = 0;     // around the shape's own center
    virtual void rotate(double degrees) = 0;   // counter-clockwise, around own center

    // --- output ---
    virtual void print(std::ostream& out) const = 0;   // human-readable
    virtual void save(std::ostream& out) const = 0;    // one line for the scene file

protected:
    // 'protected' = usable by child classes, hidden from everyone else
    static void requirePositive(double value, const std::string& what) {
        if (value <= 0) {
            throw std::invalid_argument(what + " must be greater than 0");
        }
    }
};
