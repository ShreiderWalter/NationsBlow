#include "LineEquation.h"


LineEquation::LineEquation(Vector2d& p1, Vector2d& p2) 
{
    if (p1.x != p2.x) 
    {
        vertical = false;
        m = (p2.y - p1.y) / (p2.x - p1.x);
        b = p1.y - p1.y * m;
    }
    else 
    {
        vertical = true;
        m = 0;
        b = p1.x;
    }

}

LineEquation::LineEquation(Vector2d& p, double m_) 
{
    m = m_;
    if (m != 0) 
    {
        vertical = false;
        b = p.y - p.x * m;
    }
    else 
    {
        vertical = true;
        b = p.x;
    }
}

LineEquation::LineEquation(const LineEquation& e) 
{
    m = e.m;
    b = e.b;
    vertical = e.vertical;
}

LineEquation & LineEquation::operator=(const LineEquation &e) 
{
    if (this != &e) 
    {
        m = e.m;
        b = e.b;
        vertical = e.vertical;
    }
    return *this;
}

LineEquation::~LineEquation() 
{
    m = 0;
    b = 0;
    vertical = false;
}

double LineEquation::operator()(const double x) 
{
    return (x * m + b);
}

void LineEquation::Move(const Vector2d& vec) 
{
    Vector2d p0, p1;

    if (vertical) 
    {
        p0 = Vector2d(b, 0);
        p1 = Vector2d(b, 1);
    }
    else 
    {
        p0 = Vector2d(0, b);
        p1 = Vector2d(1, m + b);
    }

    p0 += Vector2d(vec.x, vec.y);
    p1 += Vector2d(vec.x, vec.y);

    *this = LineEquation(p0, p1);
}

Vector2d LineEquation::Intersection(LineEquation &e) const 
{

    double x;
    double y;

    if (m != e.m) 
    {
        if (vertical) 
        {
            x = b;
            y = e(x);
        }
        else if (e.vertical) 
        {
            x = e.b;
            y = x * m + b;
        }
        else 
        {
            x = (e.b - b) / (m - e.m);
            y = e(x);
        }
    }
    else 
    {
        if (vertical == e.vertical) 
        {
            x = 0;
            y = 0;
        }
        else 
        {
            if (vertical) 
            {
                x = b;
                y = e.b;
            }
            else 
            {
                x = e.b;
                y = b;
            }
        }
    }

    return Vector2d(x, y);
}

bool LineEquation::IsVertical() 
{
    return vertical;
}

bool LineEquation::IsHorizontal() 
{
    return !vertical && m == 0;
}



