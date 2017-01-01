#pragma once

#include "Vector2d.h"
#include <vector>
#include <list>
#include <cstdint>

struct Biome {

    enum class Type : std::uint8_t 
    {
        Snow,
        Tundra,
        Mountain,
        Taiga,
        Shrubland,
        TemperateDesert,
        TemperateRainForest,
        TemperateDeciduousForest,
        Grassland,
        TropicalRainForest,
        TropicalSeasonalForest,
        SubtropicalDesert,
        Ocean,
        Lake,
        Beach,

        Size,
        None
    };
};

struct Edge;
struct Corner;

struct Center 
{
    Center() : m_index(0), m_position(0, 0), m_water(false), m_ocean(false), m_coast(false),
        m_border(false), m_biome(Biome::Type::None), m_elevation(0.0), m_moisture(0.0) {}

    Center(unsigned int i, Vector2d p) : m_index(i), m_position(p), m_ocean(false), m_water(false),
        m_coast(false), m_border(false), m_biome(Biome::Type::None), m_elevation(0.0), m_moisture(0.0) {}

    unsigned int		m_index;
    Vector2d			m_position;

    bool m_water;
    bool m_ocean;
    bool m_coast;
    bool m_border;
    Biome::Type m_biome;
    double m_elevation;
    double m_moisture;

    std::vector<Edge *>		m_edges;
    std::vector<Corner *>	m_corners;
    std::vector<Center *>	m_centers;

    bool RemoveEdge(Edge *e);
    bool RemoveCorner(Corner *c);
    Edge * GetEdgeWith(Center *ce);
    void MakeBorder();
    bool IsInsideBoundingBox(int width, int height);
    bool Contains(Vector2d p_pos);
    std::pair<Vector2d, Vector2d> GetBoundingBox();
    void SortCorners();
    bool GoesBefore(const Vector2d& p_a, const Vector2d& p_b);

    typedef std::vector<Center *>::iterator PVIter;
    typedef std::list<Center *>::iterator PLIter;
};

struct Edge 
{
    Edge() : index(0), d0(nullptr), d1(nullptr), v0(nullptr), v1(nullptr), m_river_volume(0.0) {};
    Edge(unsigned int i, Center *e1, Center *e2, Corner *o1, Corner *o2);

    unsigned int		index;

    Center				* d0, *d1;
    Corner 				* v0, *v1;

    Vector2d m_voronoi_midpoint;
    double m_river_volume;

    bool Legalize();
    bool Flip();
    void SwitchCorner(Corner *old_Corner, Corner *new_Corner);
    Corner * GetOpositeCorner(Corner *c);
    Center * GetOpositeCenter(Center *c);

    typedef std::vector<Edge *>::iterator PVIter;
    typedef std::list<Edge *>::iterator PLIter;
};

struct Corner 
{
    Corner() : m_index(0), m_position(0, 0), m_ocean(false), m_water(false), m_coast(false), m_border(false),
        m_elevation(0.0), m_moisture(0.0), m_river_volume(0.0), m_downslope(nullptr) {}

    Corner(unsigned int i, Vector2d p) : m_index(i), m_position(p), m_ocean(false), m_water(false), m_coast(false), m_border(false),
        m_elevation(0.0), m_moisture(0.0), m_river_volume(0.0), m_downslope(nullptr) {}

    unsigned int		m_index;
    Vector2d			m_position;

    bool m_ocean;
    bool m_water;
    bool m_coast;
    bool m_border;
    double m_elevation;
    double m_moisture;
    double m_river_volume;
    Corner *m_downslope;

    std::vector<Center *>	m_centers;
    std::vector<Edge *>		m_edges;
    std::vector<Corner *>	m_corners;

    bool IsPointInCircumcircle(const Vector2d& p);
    Vector2d CalculateCircumCenter();
    Center * GetOpositeCenter(Center *c0, Center *c1);
    void SwitchAdjacent(Corner *old_Corner, Corner * new_Corner);
    bool TouchesCenter(Center *c);
    Edge * GetEdgeConnecting(Center *c0, Center *c1);
    Center * GetOpositeCenter(Edge *e);
    bool IsInsideBoundingBox(int width, int height);
    Edge * GetEdgeWith(Corner *c);

    typedef std::vector<Corner *>::iterator PVIter;
    typedef std::list<Corner *>::iterator PLIter;

    static bool SortByElevation(Corner * c1, Corner * c2);
    static bool SortByMoisture(Corner * c1, Corner * c2);
};
