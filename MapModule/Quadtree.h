#pragma once

#include "Vector2d.h"
#include <vector>

#include <SFML/System.hpp>

struct BoundingBox;

template<class T>
class QuadTree 
{
public:

    QuadTree() 
    {
        m_northWest = nullptr;
        m_northEast = nullptr;
        m_southWest = nullptr;
        m_southEast = nullptr;
        m_divided = false;
        m_branch_depth = 0;
    }

    ~QuadTree() 
    {
        if (m_divided) 
        {
            delete m_northWest;
            delete m_northEast;
            delete m_southEast;
            delete m_southWest;
        }
    }

    QuadTree(const BoundingBox& p_boundary, int p_depth) 
    {
        m_boundary = p_boundary;
        m_divided = false;
        m_branch_depth = p_depth;
        m_elements_branch = 0;
    }

    bool Insert(const T p_element, Vector2d p_pos) 
    {
        if (!m_boundary.Contains(p_pos)) 
        {
            return false;
        }

        if (!m_divided) 
        {
            if (m_elements.size() < C_NODE_CAPACITY)
            {
                m_elements.push_back(make_pair(p_element, p_pos));
                return true;
            }
            Subdivide();
        }

        if (m_northWest->Insert(p_element, p_pos) || m_northEast->Insert(p_element, p_pos)
            || m_southEast->Insert(p_element, p_pos) || m_southWest->Insert(p_element, p_pos)) 
        {
            return true;
        }
        return false;
    }

    bool Insert(const T p_element, const BoundingBox& p_range) 
    {
        if (!m_boundary.Intersects(p_range))
        {
            return false;
        }
        if (!m_divided)
        {
            if (m_elements.size() < 4) 
            {
                m_elements.push_back(make_pair(p_element, p_range));
                return true;
            }
            Subdivide();
        }

        if (m_northWest->m_boundary.Intersects(p_range))
        {
            m_northWest->Insert(p_element, p_range);
        }
        if (m_northEast->m_boundary.Intersects(p_range)) 
        {
            m_northEast->Insert(p_element, p_range);
        }
        if (m_southEast->m_boundary.Intersects(p_range)) 
        {
            m_southEast->Insert(p_element, p_range);
        }
        if (m_southWest->m_boundary.Intersects(p_range)) 
        {
            m_southWest->Insert(p_element, p_range);
        }
        return true;
    }

    bool Insert2(const T p_element, const BoundingBox& p_range) 
    {
        if (!m_boundary.Intersects(p_range))
        {
            return false;
        }

        m_elements_branch++;

        if (m_branch_depth == C_MAX_TREE_DEPTH) 
        {
            m_elements.push_back(p_element);
            m_elements_regions.push_back(p_range);
            return true;
        }

        if (!m_divided) 
        {
            Subdivide2();
        }

        if (m_northWest->m_boundary.Intersects(p_range)) 
        {
            m_northWest->Insert2(p_element, p_range);
        }
        if (m_northEast->m_boundary.Intersects(p_range)) 
        {
            m_northEast->Insert2(p_element, p_range);
        }
        if (m_southEast->m_boundary.Intersects(p_range)) 
        {
            m_southEast->Insert2(p_element, p_range);
        }
        if (m_southWest->m_boundary.Intersects(p_range)) 
        {
            m_southWest->Insert2(p_element, p_range);
        }

        return true;
    }

    vector<T> QueryRange(Vector2d p_pos) 
    {
        auto l_current_leaf = this;

        while (l_current_leaf->m_divided) 
        {
            if (l_current_leaf->m_northWest->m_boundary.Contains(p_pos)) 
            {
                l_current_leaf = l_current_leaf->m_northWest;
            }
            else if (l_current_leaf->m_northEast->m_boundary.Contains(p_pos)) 
            {
                l_current_leaf = l_current_leaf->m_northEast;
            }
            else if (l_current_leaf->m_southEast->m_boundary.Contains(p_pos)) 
            {
                l_current_leaf = l_current_leaf->m_southEast;
            }
            else if (l_current_leaf->m_southWest->m_boundary.Contains(p_pos)) 
            {
                l_current_leaf = l_current_leaf->m_southWest;
            }
            else 
            {
                return std::vector<T>();
            }
        }

        std::vector<T> r_elements;
        for (size_t i = 0; i < l_current_leaf->m_elements.size(); i++) 
        {
            if (l_current_leaf->m_elements_regions[i].Contains(p_pos)) 
            {
                r_elements.push_back(l_current_leaf->m_elements[i]);
            }
        }
        return r_elements;
    }

    static void SetMaxDepth(int i) 
    {
        if (i > 0)
        {
            C_MAX_TREE_DEPTH = i;
        }
    }

    BoundingBox m_boundary;

private:

    void Subdivide() 
    {
        m_divided = true;

        auto l_new_half = m_boundary.m_half / 2;

        auto l_nw_pos = m_boundary.m_pos - l_new_half;
        auto l_northWest = BoundingBox{ l_nw_pos, l_new_half };
        m_northWest = new QuadTree<T>(l_northWest, m_branch_depth + 1);

        auto l_ne_pos(l_nw_pos.x + m_boundary.m_half.x, l_nw_pos.y);
        auto l_nothEast = BoundingBox{ l_ne_pos, l_new_half };
        m_northEast = new QuadTree<T>(l_nothEast, m_branch_depth + 1);

        auto l_se_pos = m_boundary.m_pos + l_new_half;
        auto l_southEast = BoundingBox{ l_se_pos, l_new_half };
        m_southEast = new QuadTree<T>(l_southEast, m_branch_depth + 1);

        auto l_sw_pos(l_nw_pos.x, l_nw_pos.y + m_boundary.m_half.y);
        auto l_southWest = BoundingBox{ l_sw_pos, l_new_half };
        m_southWest = new QuadTree<T>(l_southWest, m_branch_depth + 1);

        vector<pair<T, BoundingBox> >::iterator iter;
        for (iter = m_elements.begin(); iter != m_elements.end(); iter++) 
        {
            if (m_northWest->m_boundary.Intersects(iter->second)) {
                m_northWest->Insert(iter->first, iter->second);
            }
            if (m_northEast->m_boundary.Intersects(iter->second)) {
                m_northEast->Insert(iter->first, iter->second);
            }
            if (m_southEast->m_boundary.Intersects(iter->second)) {
                m_southEast->Insert(iter->first, iter->second);
            }
            if (m_southWest->m_boundary.Intersects(iter->second)) {
                m_southWest->Insert(iter->first, iter->second);
            }
        }
        m_elements.clear();
    }

    static int C_MAX_TREE_DEPTH;

    void Subdivide2() 
    {
        m_divided = true;

        auto l_new_half = m_boundary.m_half / 2;

        auto l_nw_pos = m_boundary.m_pos - l_new_half;
        auto l_northWest = BoundingBox{ l_nw_pos, l_new_half };
        m_northWest = new QuadTree<T>(l_northWest, m_branch_depth + 1);

        Vector2d l_ne_pos(l_nw_pos.x + m_boundary.m_half.x, l_nw_pos.y);
        auto l_nothEast = BoundingBox{ l_ne_pos, l_new_half };
        m_northEast = new QuadTree<T>(l_nothEast, m_branch_depth + 1);

        auto l_se_pos = m_boundary.m_pos + l_new_half;
        auto l_southEast = BoundingBox{ l_se_pos, l_new_half };
        m_southEast = new QuadTree<T>(l_southEast, m_branch_depth + 1);

        Vector2d l_sw_pos(l_nw_pos.x, l_nw_pos.y + m_boundary.m_half.y);
        auto l_southWest = BoundingBox{ l_sw_pos, l_new_half };
        m_southWest = new QuadTree<T>(l_southWest, m_branch_depth + 1);
    }

    vector<T> m_elements;
    vector<BoundingBox> m_elements_regions;

    QuadTree *m_northWest;
    QuadTree *m_northEast;
    QuadTree *m_southEast;
    QuadTree *m_southWest;

    bool m_divided;
    int m_branch_depth;
    int m_elements_branch;
};

template <class T>
int QuadTree<T>::C_MAX_TREE_DEPTH = 6;

struct BoundingBox 
{
    Vector2d m_pos;
    Vector2d m_half;

    BoundingBox() {};

    BoundingBox(const Vector2d& p_pos, const Vector2d& p_half) 
    {
        m_pos = p_pos;
        m_half = p_half;
    }

    bool Contains(const Vector2d& p_point) const 
    {
        auto l_min_point = m_pos - m_half;
        if (p_point.x >= l_min_point.x && p_point.y >= l_min_point.y) 
        {
            auto l_max_point = m_pos + m_half;
            return p_point.x <= l_max_point.x && p_point.y <= l_max_point.y;
        }
        return false;
    }

    bool Intersects(const BoundingBox& p_sec) const 
    {
        auto l_diff_x = abs(m_pos.x - p_sec.m_pos.x);
        auto l_diff_y = abs(m_pos.y - p_sec.m_pos.y);

        if (l_diff_x > (m_half.x + p_sec.m_half.x) || l_diff_y > (m_half.y + p_sec.m_half.y)) 
        {
            return false;
        }

        return true;
    }
};