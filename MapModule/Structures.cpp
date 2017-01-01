
#include "Structures.h"
#include "LineEquation.h" 
#include <iostream>

using namespace std;

Edge::Edge(unsigned int i, Center *e1, Center *e2, Corner *o1, Corner *o2) : index(i), d0(e1), d1(e2), v0(o1), v1(o2), m_river_volume(0.0) 
{
    Vector2d pv0 = v0 == nullptr ? d0->m_position + (d1->m_position - d0->m_position) / 2 : v0->m_position;
    Vector2d pv1 = v1 == nullptr ? d0->m_position + (d1->m_position - d0->m_position) / 2 : v1->m_position;
    m_voronoi_midpoint = (pv0 + pv1) / 2;
};

bool Center::RemoveEdge(Edge *e) 
{
    vector<Edge *>::iterator Edge_iter;
    for (Edge_iter = m_edges.begin(); Edge_iter != m_edges.end(); Edge_iter++) 
    {
        if (*Edge_iter == e) 
        {
            m_edges.erase(Edge_iter);
            return true;
        }
    }
    return false;
}

Edge * Center::GetEdgeWith(Center *ce) 
{
    vector<Edge *>::iterator Edge_iter;
    for (Edge_iter = m_edges.begin(); Edge_iter != m_edges.end(); Edge_iter++) 
    {
        if ((*Edge_iter)->d0 == ce || (*Edge_iter)->d1 == ce) 
        {
            return *Edge_iter;
        }
    }
    return nullptr;
}

bool Center::RemoveCorner(Corner *c) 
{
    vector<Corner *>::iterator Corner_iter, Corners_end = m_corners.end();
    for (Corner_iter = m_corners.begin(); Corner_iter != Corners_end; Corner_iter++) 
    {
        if (*Corner_iter == c) 
        {
            m_corners.erase(Corner_iter);
            return true;
        }
    }
    return false;
}

void Center::MakeBorder() 
{
    m_border = true;
    m_water = true;
    m_ocean = true;

    Corner::PVIter Corner_iter, Corners_end = m_corners.end();
    for (Corner_iter = m_corners.begin(); Corner_iter != Corners_end; Corner_iter++) 
    {
        (*Corner_iter)->m_border = true;
        (*Corner_iter)->m_water = true;
        (*Corner_iter)->m_ocean = true;
    }
}

bool Center::IsInsideBoundingBox(int width, int height) 
{
    if (m_position.x < 0 || m_position.x >= width || m_position.y < 0 || m_position.y >= height)
        return false;

    return true;
}

bool Center::Contains(Vector2d p_pos) 
{

    if (m_corners.size() < 3)
    {
        return false;
    }

    Vector2d l_first_sec(m_corners[0]->m_position, m_corners[1]->m_position);
    Vector2d l_first_pos(m_corners[0]->m_position, p_pos);
    bool sign = l_first_sec.CrossProduct(l_first_pos) > 0;
    Corner::PVIter iter;
    for (iter = m_corners.begin() + 1; iter != m_corners.end() - 1; iter++) 
    {
        Vector2d l_a_b((*iter)->m_position, (*(iter + 1))->m_position);
        Vector2d l_a_p((*iter)->m_position, p_pos);
        if (sign != (l_a_b.CrossProduct(l_a_p) > 0)) 
        {
            return false;
        }
    }

    return true;
}

pair<Vector2d, Vector2d> Center::GetBoundingBox() 
{
    double l_min_x = m_corners[0]->m_position.x, l_max_x = m_corners[0]->m_position.x;
    double l_min_y = m_corners[0]->m_position.y, l_max_y = m_corners[0]->m_position.y;

    Corner::PVIter iter;
    for (iter = m_corners.begin() + 1; iter != m_corners.end(); iter++) 
    {
        if ((*iter)->m_position.x < l_min_x) 
        {
            l_min_x = (*iter)->m_position.x;
        }
        else if ((*iter)->m_position.x > l_max_x) 
        {
            l_max_x = (*iter)->m_position.x;
        }
        if ((*iter)->m_position.y < l_min_y) 
        {
            l_min_y = (*iter)->m_position.y;
        }
        else if ((*iter)->m_position.y > l_max_y) 
        {
            l_max_y = (*iter)->m_position.y;
        }
    }

    Vector2d l_min_pos(l_min_x, l_min_y);
    Vector2d l_max_pos(l_max_x, l_max_y);
    Vector2d l_half_diagonal(Vector2d(l_min_pos, l_max_pos) / 2);

    return make_pair(l_min_pos + l_half_diagonal, l_half_diagonal);
}

void Center::SortCorners() 
{
    Corner * item = nullptr;
    int hole = 0;
    for (int i = 1; i < m_corners.size(); i++) 
    {
        item = m_corners[i];
        hole = i;
        while (hole > 0 && GoesBefore(item->m_position, m_corners[hole - 1]->m_position)) 
        {
            m_corners[hole] = m_corners[hole - 1];
            hole--;
        }
        m_corners[hole] = item;
    }
}

bool Center::GoesBefore(const Vector2d& p_a, const Vector2d& p_b) 
{
    if ((p_a - m_position).x >= 0 && (p_b - m_position).x < 0)
    {
        return true;
    }

    if (p_a.x == 0 && p_b.x == 0)
    {
        return p_a.y < p_b.y;
    }

    Vector2d ca(m_position, p_a);
    Vector2d cb(m_position, p_b);
    return ca.CrossProduct(cb) > 0;
}

bool Edge::Legalize() 
{

    if (v0 == nullptr || v1 == nullptr)
    {
        return false;
    }
    if (v0->IsPointInCircumcircle(v1->GetOpositeCenter(this)->m_position))
    {
        return Flip();
    }
    return false;
}

bool Edge::Flip() 
{
    Center *cen0 = v0->GetOpositeCenter(d0, d1);
    Center *cen1 = v1->GetOpositeCenter(d0, d1);

    if (cen0 == nullptr || cen1 == nullptr)
    {
        return false;
    }
    Edge * e00 = cen0->GetEdgeWith(d0);	// nv0
    Edge * e01 = cen0->GetEdgeWith(d1);	// nv1
    Edge * e10 = cen1->GetEdgeWith(d0);	// nv0
    Edge * e11 = cen1->GetEdgeWith(d1);	// nv1

    e00->SwitchCorner(v0, v1);
    e11->SwitchCorner(v1, v0);

    d0->RemoveEdge(this);
    d1->RemoveEdge(this);

    d0->RemoveCorner(v0);
    d1->RemoveCorner(v1);

    cen0->m_corners.push_back(v1);
    cen1->m_corners.push_back(v0);

    cen0->m_edges.push_back(this);
    cen1->m_edges.push_back(this);

    v0->m_centers.clear();
    v0->m_edges.clear();

    v0->m_centers.push_back(cen0);
    v0->m_centers.push_back(cen1);
    v0->m_centers.push_back(d1);
    v0->m_position = v0->CalculateCircumCenter();

    v0->m_edges.push_back(this);
    v0->m_edges.push_back(e01);
    v0->m_edges.push_back(e11);

    v1->m_centers.clear();
    v1->m_edges.clear();

    v1->m_centers.push_back(cen0);
    v1->m_centers.push_back(d0);
    v1->m_centers.push_back(cen1);
    v1->m_position = v1->CalculateCircumCenter();

    v1->m_edges.push_back(this);
    v1->m_edges.push_back(e00);
    v1->m_edges.push_back(e10);

    d0 = cen0;
    d1 = cen1;

    e00->Legalize();
    e01->Legalize();
    e10->Legalize();
    e11->Legalize();

    return true;
}

void Edge::SwitchCorner(Corner *old_Corner, Corner *new_Corner) 
{
    if (v0 == old_Corner) 
    {
        v0 = new_Corner;
    }
    else if (v1 == old_Corner) 
    {
        v1 = new_Corner;
    }
}

Corner * Edge::GetOpositeCorner(Corner *c) 
{
    if (v0 == c) 
    {
        return v1;
    }
    else if (v1 == c) 
    {
        return v0;
    }
    return nullptr;
}

Center * Edge::GetOpositeCenter(Center *c) 
{
    if (d0 == c) 
    {
        return d1;
    }
    else if (d1 == c) 
    {
        return d0;
    }
    return nullptr;
}

Edge * Corner::GetEdgeWith(Corner *c) 
{
    for each (Edge * e in m_edges)
    {
        if (e->v0 == c || e->v1 == c)
        {
            return e;
        }
    }
    return nullptr;
}

bool Corner::IsPointInCircumcircle(const Vector2d& p) {

    if (this->m_centers.empty())
    {
        return false;
    }
    Center *point_circumference = this->m_centers[0];

    Vector2d Corner_Center(this->m_position, point_circumference->m_position);
    Vector2d Corner_p(this->m_position, p);

    return Corner_Center.Length() >= Corner_p.Length();
}

Vector2d Corner::CalculateCircumCenter() {
    if (this->m_centers.size() != 3)
        return Vector2d();

    Vector2d a = m_centers[0]->m_position;
    Vector2d b = m_centers[1]->m_position;
    Vector2d c = m_centers[2]->m_position;
    /*
    double d = 2 * (a.x*(b.y - c.y) + b.x*(c.y - a.y) + c.x*(a.y - b.y));
    double new_x = ((a.x*a.x + a.y*a.y)*(b.y - c.y) + (b.x*b.x + b.y*b.y)*(c.y - a.y) + (c.x*c.x + c.y*c.y)*(a.y - b.y)) / d;
    double new_y = ((a.x*a.x + a.y*a.y)*(c.x - b.x) + (b.x*b.x + b.y*b.y)*(a.x - c.x) + (c.x*c.x + c.y*c.y)*(b.x - a.x)) / d;

    return Vector2d(new_x, new_y);*/


    Vector2d ab_midpoint((a.x + b.x) / 2, (a.y + b.y) / 2);
    LineEquation ab_equ(a, b);

    LineEquation ab_bisector;
    if (ab_equ.IsVertical()) 
    {
        ab_bisector = LineEquation(ab_midpoint, Vector2d(ab_midpoint.x + 1, ab_midpoint.y));
    }
    else if (ab_equ.IsHorizontal()) 
    {
        ab_bisector = LineEquation(ab_midpoint, Vector2d(ab_midpoint.x, ab_midpoint.y + 1));
    }
    else 
    {
        double ab_bisector_slope = ab_equ.m == 0 ? 0 : -1 / ab_equ.m;
        ab_bisector = LineEquation(ab_midpoint, ab_bisector_slope);
    }

    Vector2d bc_midpoint((b.x + c.x) / 2, (b.y + c.y) / 2);
    LineEquation bc_equ(b, c);

    LineEquation bc_bisector;
    if (bc_equ.IsVertical()) 
    {
        bc_bisector = LineEquation(bc_midpoint, Vector2d(bc_midpoint.x + 1, bc_midpoint.y));
    }
    else if (bc_equ.IsHorizontal()) 
    {
        bc_bisector = LineEquation(bc_midpoint, Vector2d(bc_midpoint.x, bc_midpoint.y + 1));
    }
    else 
    {
        double bc_bisector_slope = bc_equ.m == 0 ? 0 : -1 / bc_equ.m;
        bc_bisector = LineEquation(bc_midpoint, bc_bisector_slope);
    }

    return ab_bisector.Intersection(bc_bisector);
}

Center * Corner::GetOpositeCenter(Center *c0, Center *c1) 
{
    vector<Center *>::iterator Center_iter, Centers_end = m_centers.end();
    for (Center_iter = m_centers.begin(); Center_iter != Centers_end; Center_iter++) 
    {
        if (*Center_iter != c0 && *Center_iter != c1)
        {
            return *Center_iter;
        }
    }
    return nullptr;
}

Center * Corner::GetOpositeCenter(Edge *e) 
{
    bool found = false;
    for each (Edge * ed in m_edges) 
    {
        if (ed == e) 
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        return nullptr;
    }
    for each (Center * c in m_centers) 
    {
        if (c != e->d0 && c != e->d1)
        {
            return c;
        }
    }

    return nullptr;
}

bool Corner::TouchesCenter(Center *c) 
{
    for (int i = 0; i < m_centers.size(); i++)
    {
        if (m_centers[i] == c)
        {
            return true;
        }
    }
    return false;
}

Edge * Corner::GetEdgeConnecting(Center *c0, Center *c1) 
{
    vector<Edge *>::iterator Edge_iter, Edges_end = m_edges.end();
    for (Edge_iter = m_edges.begin(); Edge_iter != Edges_end; Edge_iter++) 
    {
        if (((*Edge_iter)->d0 == c0 && (*Edge_iter)->d1 == c1) || ((*Edge_iter)->d1 == c0 && (*Edge_iter)->d0 == c1))
        {
            return *Edge_iter;
        }
    }
    return nullptr;
}

bool Corner::IsInsideBoundingBox(int width, int height) 
{
    if (m_position.x < 0 || m_position.x >= width || m_position.y < 0 || m_position.y >= height)
    {
        return false;
    }
    return true;
}

bool Corner::SortByElevation(Corner *c1, Corner *c2) 
{
    return c1->m_elevation < c2->m_elevation;
}

bool Corner::SortByMoisture(Corner *c1, Corner *c2)
{
    return c1->m_moisture < c2->m_moisture;
}