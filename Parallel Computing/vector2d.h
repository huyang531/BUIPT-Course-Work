#include <fstream>
#include <iostream>
#include <sstream>

struct Vector2d {
    double x;
    double y;

    Vector2d() : x(0), y(0) {}
    Vector2d(double _x, double _y) : x(_x), y(_y) {}
    Vector2d(const Vector2d& v) : x(v.x), y(v.y) {}

    void operator=(const Vector2d& v);
    Vector2d operator+(const Vector2d& v) const;
    Vector2d operator-(const Vector2d& v) const;
    Vector2d operator*(const Vector2d& v) const;
    Vector2d operator/(const Vector2d& v) const;

    Vector2d operator+(double d) const;
    Vector2d operator-(double d) const;
    Vector2d operator*(double d) const;
    Vector2d operator/(double d) const;

    double dot(const Vector2d& v) const;
    double length() const;
    Vector2d crossProduct(const Vector2d& v);
    double dis_between(const Vector2d& v) const;
    friend std::ostream& operator<<(std::ostream& cout, const Vector2d& v);
};