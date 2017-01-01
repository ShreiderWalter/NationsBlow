#include "Vector2d.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

#ifndef EQ_THRESHOLD
#define EQ_THRESHOLD 0.00001
#endif

using namespace std;

ostream & operator<<(ostream &os, const Vector2d &v) 
{
    os << "(" << v.x << "," << v.y << ")";
    return os;
}

Vector2d::Vector2d(): x(0.0), y(0.0)
{
}

Vector2d::~Vector2d() 
{
}

Vector2d::Vector2d(const Vector2d & v) 
{
    Copy(v);
}

Vector2d & Vector2d::operator =(const Vector2d & v) {
    if (this != &v) 
    {
        Copy(v);
    }
    return *this;
}

Vector2d::Vector2d(double x, double y): x(x), y(y)
{
}

Vector2d::Vector2d(const Vector2d &v1, const Vector2d &v2): x(v2.x - v1.x), y(v2.y - v1.y)
{
}

Vector2d::Vector2d(double angle): x(cos(angle * M_PI / 180)), y(sin(angle * M_PI / 180))
{
}

Vector2d & Vector2d::operator +=(const Vector2d & v) 
{
    x += v.x;
    y += v.y;

    return *this;
}

Vector2d & Vector2d::operator +=(const double & f) 
{
    x += f;
    y += f;

    return *this;
}

Vector2d & Vector2d::operator -=(const Vector2d & v) 
{
    x -= v.x;
    y -= v.y;

    return *this;
}

Vector2d & Vector2d::operator -=(const double & f)
{
    x -= f;
    y -= f;

    return *this;
}

Vector2d & Vector2d::operator *=(const double & f) 
{
    y *= f;

    return *this;
}

Vector2d & Vector2d::operator /=(const double & f)
{
    x /= f;
    y /= f;

    return *this;
}

bool Vector2d::operator ==(const Vector2d & v) const 
{

    auto diff_x = abs(this->x - v.x);
    auto diff_y = abs(this->y - v.y);

    return diff_x < EQ_THRESHOLD && diff_y < EQ_THRESHOLD;
}

bool Vector2d::operator !=(const Vector2d & v) const 
{

    auto diff_x = abs(this->x - v.x);
    auto diff_y = abs(this->y - v.y);

    return diff_x > EQ_THRESHOLD || diff_y > EQ_THRESHOLD;
}

void Vector2d::Normalize() 
{
    auto mod = this->Length();

    if (mod > 0) 
    {
        this->x /= mod;
        this->y /= mod;
    }
}

void Vector2d::Reflect(const Vector2d & v) 
{
    *this += 2.0 * this->DotProduct(v) * Vector2d(v.x * -1, v.y * -1);
}

void Vector2d::Reverse() 
{
    x *= -1;
    y *= -1;
}

void Vector2d::Truncate(double maxLength) 
{
    if (Length() > maxLength) 
    {
        Normalize();
        *this *= maxLength;
    }
}

void Vector2d::RotateAng(double angle) 
{
    RotateRad(angle * M_PI / 180);
}

void Vector2d::RotateRad(double radians) 
{
    auto newX = x * cos(radians) - y * sin(radians);
    auto newY = x * sin(radians) + y * cos(radians);

    x = newX;
    y = newY;
}

double Vector2d::DotProduct(const Vector2d & v) const 
{
    return (x * v.x + y * v.y);
}

double Vector2d::CrossProduct(const Vector2d & v) const 
{
    return (x * v.y - v.x * y);
}

double Vector2d::Distance(const Vector2d & v) const 
{
    Vector2d dist(*this, v);
    return dist.Length();
}

double Vector2d::DistanceSqrd(const Vector2d & v) const 
{
    Vector2d dist(*this, v);
    return dist.LengthSqrd();
}

bool Vector2d::Sign(const Vector2d & v) const 
{
    return (x * v.y > v.x * y);
}

double Vector2d::AngleAng(const Vector2d & v) const 
{
    return AngleRad(v) * 180 / M_PI;
}

double Vector2d::AngleAng() const 
{
    return AngleRad() * 180 / M_PI;
}

double Vector2d::AngleRad(const Vector2d & v) const 
{
    if (isZero() || v.isZero())
    {
        return 0;
    }
    auto angle = atan2(v.y - y, v.x - x);

    return angle;
}

double Vector2d::AngleRad() const 
{
    if (isZero())
    {
        return 0;
    }
    return atan2(y, x);
}

bool Vector2d::isZero() const 
{
    return (x == 0 && y == 0);
}

double Vector2d::Length() const 
{
    return sqrt(x * x + y * y);
}

double Vector2d::LengthSqrd() const 
{
    return (x * x + y * y);
}

void Vector2d::Copy(const Vector2d & v) 
{
    x = v.x;
    y = v.y;
}

Vector2d operator +(const Vector2d & lhs, const Vector2d & rhs) 
{
    Vector2d aux(lhs);
    aux += rhs;

    return aux;
}

Vector2d operator -(const Vector2d & lhs, const Vector2d & rhs) 
{
    Vector2d aux(lhs);
    aux -= rhs;

    return aux;
}

Vector2d operator *(const double & fac, const Vector2d & rhs) 
{
    Vector2d aux(rhs);
    aux *= fac;

    return aux;
}

Vector2d operator *(const Vector2d & lhs, const double & fac) 
{
    Vector2d aux(lhs);
    aux *= fac;

    return aux;
}

Vector2d operator /(const Vector2d & lhs, const double & fac) 
{
    Vector2d aux(lhs);
    aux /= fac;

    return aux;
}

Vector2d Normalize(const Vector2d & vec) 
{
    Vector2d aux(vec);
    aux.Normalize();

    return aux;
}

Vector2d Reflect(const Vector2d & v1, const Vector2d & v2) 
{
    Vector2d aux(v1);
    aux.Reflect(v2);

    return aux;
}

Vector2d Reverse(const Vector2d &v) 
{
    Vector2d aux(v);
    aux.Reverse();

    return aux;
}

Vector2d Truncate(const Vector2d & v, double & maxLength) 
{
    Vector2d aux(v);
    aux.Truncate(maxLength);

    return aux;
}

Vector2d RotateAng(const Vector2d & v, double & angle) 
{
    Vector2d aux(v);
    aux.RotateAng(angle);

    return aux;
}

Vector2d RotateRad(const Vector2d & v, double & radians) 
{
    Vector2d aux(v);
    aux.RotateRad(radians);

    return aux;
}

double Distance(const Vector2d & v1, const Vector2d & v2) 
{
    Vector2d aux(v1, v2);

    return aux.Length();
}