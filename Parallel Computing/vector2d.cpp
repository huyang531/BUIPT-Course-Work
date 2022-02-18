#include <math.h>

#include "vector2d.h"

void Vector2d::operator=(const Vector2d& v) {
    x = v.x;
    y = v.y;
}

Vector2d Vector2d::operator+(const Vector2d& v) const {
    return Vector2d(x + v.x, y + v.y);
}

Vector2d Vector2d::operator-(const Vector2d& v) const {
    return Vector2d(x - v.x, y - v.y);
}

Vector2d Vector2d::operator*(const Vector2d& v) const {
    return Vector2d(x * v.x, y * v.y);
}

Vector2d Vector2d::operator/(const Vector2d& v) const {
    return Vector2d(x / v.x, y / v.y);
}

Vector2d Vector2d::operator+(double d) const {
    return Vector2d(x + d, y + d);
}

Vector2d Vector2d::operator-(double d) const {
    return Vector2d(x - d, y - d);
}

Vector2d Vector2d::operator*(double d) const {
    return Vector2d(x * d, y * d);
}

Vector2d Vector2d::operator/(double d) const {
    return Vector2d(x / d, y / d);
}

double Vector2d::dot(const Vector2d& v) const {
    return x * v.x + y * v.y;
}

double Vector2d::length() const {
    return sqrt(dot(*this));
}

double Vector2d::dis_between(const Vector2d& v) const {
    return sqrt(pow(x - v.x, 2) + pow(y - v.y, 2));
}

std::ostream& operator<<(std::ostream& out, const Vector2d& v) {
    return out << v.x << ',' << v.y;
}
