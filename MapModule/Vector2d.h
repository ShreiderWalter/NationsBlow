#pragma once

#include <iostream>

class Vector2d 
{
    friend std::ostream & operator << (std::ostream &os, const Vector2d &v);
public:
    Vector2d();
    Vector2d(const Vector2d &v);
    Vector2d & operator=(const Vector2d &v);
    Vector2d(double x, double y);
    Vector2d(const Vector2d &v1, const Vector2d &v2);
    Vector2d(double angle);
    ~Vector2d();

    Vector2d & operator+=(const Vector2d &v);
    Vector2d & operator+=(const double &f);
    Vector2d & operator-=(const Vector2d &v);
    Vector2d & operator-=(const double &f);
    Vector2d & operator*=(const double &f);
    Vector2d & operator/=(const double &f);
    bool operator==(const Vector2d &v) const;
    bool operator!=(const Vector2d &v) const;
    void Normalize();
    void Reflect(const Vector2d &v);
    void Reverse();
    void Truncate(double maxLength);
    void RotateAng(double angle);
    void RotateRad(double radian);
    double DotProduct(const Vector2d &v) const;
    double CrossProduct(const Vector2d &v) const;
    double Distance(const Vector2d &v) const;
    double DistanceSqrd(const Vector2d &v) const;
    bool Sign(const Vector2d &v) const;
    double AngleAng(const Vector2d &v) const;
    double AngleAng() const;
    double AngleRad(const Vector2d &v) const;
    double AngleRad() const;
    bool isZero() const;
    double Length() const;
    double LengthSqrd() const;

public:
    double x;
    double y;

private:
    void Copy(const Vector2d &v);
};


Vector2d operator+(const Vector2d &lhs, const Vector2d &rhs);
Vector2d operator-(const Vector2d &lhs, const Vector2d &rhs);
Vector2d operator*(const double &fac, const Vector2d &rhs);
Vector2d operator*(const Vector2d &lhs, const double &fac);
Vector2d operator/(const Vector2d &lhs, const double &fac);
Vector2d Normalize(const Vector2d &vec);
Vector2d Reflect(const Vector2d &v1, const Vector2d &v2);
Vector2d Reverse(const Vector2d &v);
Vector2d Truncate(const Vector2d &v, double &maxLength);
Vector2d RotateAng(const Vector2d &v, double &angle);
Vector2d RotateRad(const Vector2d &v, double &radian);
double Distance(const Vector2d &v1, const Vector2d &v2);


