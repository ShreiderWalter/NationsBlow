#pragma once

#include "dDelaunay.h"
#include "Structures.h"
#include "Quadtree.h"
#include <vector>
#include <map>

typedef QuadTree<Center *> CenterPointerQT;

class Vector2d;

namespace noise {
    namespace module {
        class Perlin;
    }
}

class Map 
{
public:
    Map();
    Map(int width, int height, double point_spread, string seed);
    ~Map();

    void Generate();
    void GeneratePolygons();
    void GenerateLand();
    bool LoadFile(string file_name);
    bool WriteFile(string file_name);

    vector<Edge *> GetEdges();
    vector<Corner *> GetCorners();
    vector<Center *> GetCenters();

    Center * GetCenterAt(Vector2d& p_pos);

private:
    int map_width;
    int map_height;
    double m_point_spread;
    double z_coord;
    noise::module::Perlin * noiseMap;
    string m_seed;
    CenterPointerQT m_centers_quadtree;

    vector<del::vertex> points;

    map<double, map<double, Center *> > pos_cen_map;
    vector<Edge *> m_edges;
    vector<Corner *> m_corners;
    vector<Center *> m_centers;

    static const vector<vector<Biome::Type> > elevation_moisture_matrix;
    static vector<vector<Biome::Type> > MakeBiomeMatrix();

    void HexagonalGrid();
    bool IsIsland(Vector2d position);
    void AssignOceanCoastLand();
    void AssignCornerElevation();
    void RedistributeElevations();
    void AssignPolygonElevations();
    void CalculateDownslopes();
    void GenerateRivers();
    void AssignCornerMoisture();
    void RedistributeMoisture();
    void AssignPolygonMoisture();
    void AssignBiomes();

    void GeneratePoints();
    void Triangulate(vector<del::vertex> puntos);
    void FinishInfo();
    void AddCenter(Center * c);
    Center * GetCenter(Vector2d& position);
    void OrderPoints(vector<Corner *> &corners);

    vector<Corner *> GetLandCorners();
    vector<Corner *> GetLakeCorners();
    void LloydRelaxation();
    static unsigned int HashString(string seed);
    string CreateSeed(int length);
};

