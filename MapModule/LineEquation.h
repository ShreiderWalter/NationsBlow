#pragma once

#include "Vector2d.h"

struct LineEquation
{
    double m;
    double b;
    bool vertical;

    LineEquation() {};
    LineEquation(Vector2d& a, Vector2d& b);
    LineEquation(Vector2d& p, double m);
    LineEquation(const LineEquation& e);
    LineEquation & operator=(const LineEquation &e);
    ~LineEquation();

    double operator()(const double x);

    void Move(const Vector2d& vec);
    Vector2d Intersection(LineEquation& e) const;
    bool IsHorizontal();
    bool IsVertical();
};
