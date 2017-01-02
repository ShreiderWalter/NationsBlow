#include "Map.h"
#include "Vector2d.h"
#include "PoissonDiskSampling.h"
#include "noise/noise.h"
#include <ctime>
#include <queue>
#include <SFML/System.hpp>
#include "Hexagon.h"

const vector<vector<Biome::Type> > Map::elevation_moisture_matrix = Map::MakeBiomeMatrix();

vector<vector<Biome::Type> > Map::MakeBiomeMatrix() 
{
    vector<vector<Biome::Type> > matrix;
    vector<Biome::Type> biome_vector;

    biome_vector.push_back(Biome::Type::SubtropicalDesert);
    biome_vector.push_back(Biome::Type::TemperateDesert);
    biome_vector.push_back(Biome::Type::TemperateDesert);
    biome_vector.push_back(Biome::Type::Mountain);
    matrix.push_back(biome_vector);

    biome_vector.clear();
    biome_vector.push_back(Biome::Type::Grassland);
    biome_vector.push_back(Biome::Type::Grassland);
    biome_vector.push_back(Biome::Type::TemperateDesert);
    biome_vector.push_back(Biome::Type::Mountain);
    matrix.push_back(biome_vector);

    biome_vector.clear();
    biome_vector.push_back(Biome::Type::TropicalSeasonalForest);
    biome_vector.push_back(Biome::Type::Grassland);
    biome_vector.push_back(Biome::Type::Shrubland);
    biome_vector.push_back(Biome::Type::Tundra);
    matrix.push_back(biome_vector);

    biome_vector.clear();
    biome_vector.push_back(Biome::Type::TropicalSeasonalForest);
    biome_vector.push_back(Biome::Type::TemperateDeciduousForest);
    biome_vector.push_back(Biome::Type::Shrubland);
    biome_vector.push_back(Biome::Type::Snow);
    matrix.push_back(biome_vector);

    biome_vector.clear();
    biome_vector.push_back(Biome::Type::TropicalRainForest);
    biome_vector.push_back(Biome::Type::TemperateDeciduousForest);
    biome_vector.push_back(Biome::Type::Taiga);
    biome_vector.push_back(Biome::Type::Snow);
    matrix.push_back(biome_vector);

    biome_vector.clear();
    biome_vector.push_back(Biome::Type::TropicalRainForest);
    biome_vector.push_back(Biome::Type::TemperateRainForest);
    biome_vector.push_back(Biome::Type::Taiga);
    biome_vector.push_back(Biome::Type::Snow);
    matrix.push_back(biome_vector);

    return matrix;
}

Map::Map() {}

Map::~Map() {}

Map::Map(int width, int height, double point_spread, string seed) : m_centers_quadtree(BoundingBox(Vector2d(width / 2, height / 2), Vector2d(width / 2, height / 2)), 1)
{
    map_width = width;
    map_height = height;
    m_point_spread = point_spread;

    double l_aprox_point_count = (2 * map_width * map_height) / (3.1416 * point_spread * point_spread);
    int l_max_tree_depth = floor((log(l_aprox_point_count) / log(4)) + 0.5);
    CenterPointerQT::SetMaxDepth(l_max_tree_depth);

    m_seed = seed != "" ? seed : CreateSeed(20);
    srand(HashString(m_seed));

    z_coord = rand();
    //cout << "Seed: " << m_seed << "(" << HashString(m_seed) << ")" << endl;
}

void Map::Generate() 
{
    sf::Clock timer;

    GeneratePolygons();

    //cout << "Land distribution: ";
    timer.restart();
    GenerateLand();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    // ELEVATION
    //cout << "Coast assignment: ";
    timer.restart();
    AssignOceanCoastLand();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "Corner altitude: ";
    timer.restart();
    AssignCornerElevation();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "Altitude redistribution: ";
    timer.restart();
    RedistributeElevations();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "Center altitude: ";
    timer.restart();
    AssignPolygonElevations();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    // MOISTURE
    //cout << "Downslopes: ";
    timer.restart();
    CalculateDownslopes();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "River generation: ";
    timer.restart();
    GenerateRivers();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "Corner m_moisture: ";
    timer.restart();
    AssignCornerMoisture();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "Moisture redistribution: ";
    timer.restart();
    RedistributeMoisture();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "Center m_moisture: ";
    timer.restart();
    AssignPolygonMoisture();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    // BIOMES
    //cout << "Biome assignment: ";
    timer.restart();
    AssignBiomes();
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;

    //cout << "Populate Quadtree: ";
    timer.restart();
    for (int i = 0; i < m_centers.size(); i++) 
    {
        pair<Vector2d, Vector2d> boundingBox(m_centers[i]->GetBoundingBox());
        m_centers_quadtree.Insert2(m_centers[i], BoundingBox(boundingBox.first, boundingBox.second));
    }
    //cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << " ms." << endl;
}

void Map::GeneratePolygons() 
{
    sf::Clock timer;
    GeneratePoints();
    timer.restart();
    //Triangulate(points);
    HexagonalGrid();

    FinishInfo();

}

void Map::HexagonalGrid()
{
    int corner_index = 0, center_index = 0, edge_index = 0;
    m_corners.clear();
    m_centers.clear();
    m_edges.clear();
    pos_cen_map.clear();


    for (size_t i = 0; i < points.size(); i += 7)
    {
        Vector2d pos_center_0(points[i].GetX(), points[i].GetY());

        Center * c1 = GetCenter(pos_center_0);
        if (c1 == nullptr)
        {
            c1 = new Center(center_index++, pos_center_0);
            m_centers.push_back(c1);
            AddCenter(c1);
        }

        Corner * corner1 = new Corner(corner_index++, Vector2d());
        Corner * corner2 = new Corner(corner_index++, Vector2d());
        Corner * corner3 = new Corner(corner_index++, Vector2d());
        Corner * corner4 = new Corner(corner_index++, Vector2d());
        Corner * corner5 = new Corner(corner_index++, Vector2d());
        Corner * corner6 = new Corner(corner_index++, Vector2d());
        m_corners.push_back(corner1);
        m_corners.push_back(corner2);
        m_corners.push_back(corner3);
        m_corners.push_back(corner4);
        m_corners.push_back(corner5);
        m_corners.push_back(corner6);
        corner1->m_centers.push_back(c1);
        corner2->m_centers.push_back(c1);
        corner3->m_centers.push_back(c1);
        corner4->m_centers.push_back(c1);
        corner5->m_centers.push_back(c1);
        corner6->m_centers.push_back(c1);
        c1->m_corners.push_back(corner1);
        c1->m_corners.push_back(corner2);
        c1->m_corners.push_back(corner3);
        c1->m_corners.push_back(corner4);
        c1->m_corners.push_back(corner5);
        c1->m_corners.push_back(corner6);
        corner1->m_position = Vector2d(points[i + 1].GetX(), points[i + 1].GetY());
        corner2->m_position = Vector2d(points[i + 2].GetX(), points[i + 2].GetY());
        corner3->m_position = Vector2d(points[i + 3].GetX(), points[i + 3].GetY());
        corner4->m_position = Vector2d(points[i + 4].GetX(), points[i + 4].GetY());
        corner5->m_position = Vector2d(points[i + 5].GetX(), points[i + 5].GetY());
        corner6->m_position = Vector2d(points[i + 6].GetX(), points[i + 6].GetY());

        Edge * e12 = corner1->GetEdgeWith(corner2);
        if (e12 == nullptr)
        {
            e12 = new Edge(edge_index++, nullptr, nullptr, corner1, corner2);
            e12->v0 = corner1;
            m_edges.push_back(e12);
            corner1->m_edges.push_back(e12);
            corner2->m_edges.push_back(e12);
        }
        else
        {
            e12->v1 = corner1;
        }
        corner1->m_edges.push_back(e12);

        Edge * e23 = corner2->GetEdgeWith(corner3);
        if (e23 == nullptr)
        {
            e23 = new Edge(edge_index++, nullptr, nullptr, corner2, corner3);
            e12->v0 = corner2;
            m_edges.push_back(e23);
            corner2->m_edges.push_back(e23);
            corner3->m_edges.push_back(e23);
        }
        else
        {
            e23->v1 = corner2;
        }
        corner2->m_edges.push_back(e23);

        Edge * e34 = corner3->GetEdgeWith(corner4);
        if (e34 == nullptr)
        {
            e34 = new Edge(edge_index++, nullptr, nullptr, corner3, corner4);
            e34->v0 = corner3;
            m_edges.push_back(e34);
            corner3->m_edges.push_back(e34);
            corner4->m_edges.push_back(e34);
        }
        else
        {
            e34->v1 = corner3;
        }
        corner3->m_edges.push_back(e34);

        Edge * e45 = corner4->GetEdgeWith(corner5);
        if (e45 == nullptr)
        {
            e45 = new Edge(edge_index++, nullptr, nullptr, corner4, corner5);
            e45->v0 = corner4;
            m_edges.push_back(e45);
            corner4->m_edges.push_back(e45);
            corner5->m_edges.push_back(e45);
        }
        else
        {
            e45->v1 = corner4;
        }
        corner4->m_edges.push_back(e45);

        Edge * e56 = corner5->GetEdgeWith(corner6);
        if (e56 == nullptr)
        {
            e56 = new Edge(edge_index++, nullptr, nullptr, corner5, corner6);
            e56->v0 = corner5;
            m_edges.push_back(e56);
            corner5->m_edges.push_back(e56);
            corner6->m_edges.push_back(e56);
        }
        else
        {
            e56->v1 = corner5;
        }
        corner5->m_edges.push_back(e56);

        Edge * e61 = corner6->GetEdgeWith(corner1);
        if (e61 == nullptr)
        {
            e61 = new Edge(edge_index++, nullptr, nullptr, corner6, corner1);
            e61->v0 = corner6;
            m_edges.push_back(e61);
            corner6->m_edges.push_back(e61);
            corner1->m_edges.push_back(e61);
        }
        else
        {
            e61->v1 = corner6;
        }
        corner6->m_edges.push_back(e61);
    }
}

void Map::GenerateLand() 
{
    noiseMap = new noise::module::Perlin();

    for each(Corner * c in m_corners) 
    {
        if (!c->IsInsideBoundingBox(map_width, map_height)) 
        {
            c->m_border = true;
            c->m_ocean = true;
            c->m_water = true;
        }
    }

    for each (Corner * c in m_corners) 
    {
        c->m_water = !IsIsland(c->m_position);
    }
}

void Map::AssignOceanCoastLand() 
{
    queue<Center *> centers_queue;
    for each (Center * c in m_centers) 
    {
        int adjacent_water = 0;
        for each (Corner * q in c->m_corners) 
        {
            if (q->m_border) 
            {
                c->m_border = true;
                c->m_ocean = true;
                q->m_water = true;
                centers_queue.push(c);
            }
            if (q->m_water) 
            {
                adjacent_water++;
            }
        }
        c->m_water = (c->m_ocean || adjacent_water >= c->m_corners.size() * 0.5);
    }

    while (!centers_queue.empty()) 
    {
        Center * c = centers_queue.front();
        centers_queue.pop();
        for each (Center * r in c->m_centers) 
        {
            if (r->m_water && !r->m_ocean) 
            {
                r->m_ocean = true;
                centers_queue.push(r);
            }
        }
    }

    for each (Center * p in m_centers) 
    {
        int num_ocean = 0;
        int num_land = 0;
        for each (Center * q in p->m_centers) 
        {
            num_ocean += q->m_ocean;
            num_land += !q->m_water;
        }
        p->m_coast = num_land > 0 && num_ocean > 0;
    }

    for each (Corner * c in m_corners) 
    {
        int adj_ocean = 0;
        int adj_land = 0;
        for each (Center * p in c->m_centers) 
        {
            adj_ocean += (int)p->m_ocean;
            adj_land += (int)!p->m_water;
        }
        c->m_ocean = adj_ocean == c->m_centers.size();
        c->m_coast = adj_land > 0 && adj_ocean > 0;
        c->m_water = c->m_border || (adj_land != c->m_centers.size() && !c->m_coast);
    }
}

void Map::AssignCornerElevation() 
{
    queue<Corner *> corner_queue;
    for each (Corner * q in m_corners) 
    {
        if (q->m_border) 
        {
            q->m_elevation = 0.0;
            corner_queue.push(q);
        }
        else 
        {
            q->m_elevation = 99999;
        }
    }

    while (!corner_queue.empty()) 
    {
        Corner * q = corner_queue.front();
        corner_queue.pop();

        for each (Corner * s in q->m_corners) 
        {
            double new_elevation = q->m_elevation + 0.01;
            if (!q->m_water && !s->m_water) 
            {
                new_elevation += 1;
            }
            if (new_elevation < s->m_elevation) 
            {
                s->m_elevation = new_elevation;
                corner_queue.push(s);
            }
        }
    }

    for each (Corner * q in m_corners) 
    {
        if (q->m_water) 
        {
            q->m_elevation = 0.0;
        }
    }
}

void Map::RedistributeElevations() 
{
    vector<Corner *> locations = GetLandCorners();
    double SCALE_FACTOR = 1.05;

    sort(locations.begin(), locations.end(), &Corner::SortByElevation);

    for (int i = 0; i < locations.size(); i++) 
    {
        double y = (double)i / (locations.size() - 1);
        double x = sqrt(SCALE_FACTOR) - sqrt(SCALE_FACTOR * (1 - y));
        x = min(x, 1.0);
        locations[i]->m_elevation = x;
    }
}

void Map::AssignPolygonElevations() 
{
    for each (Center * p in m_centers) 
    {
        double elevation_sum = 0.0;
        for each (Corner * q in p->m_corners) 
        {
            elevation_sum += q->m_elevation;
        }
        p->m_elevation = elevation_sum / p->m_corners.size();
    }
}

void Map::CalculateDownslopes() 
{
    for each (Corner * c in m_corners) 
    {
        Corner * d = c;
        for each (Corner * q in c->m_corners) 
        {
            if (q->m_elevation < d->m_elevation) 
            {
                d = q;
            }
        }
        c->m_downslope = d;
    }
}

void Map::GenerateRivers() 
{
    //int num_rios = (map_height + map_width) / 4;
    int num_rios = m_centers.size() / 3;
    for (int i = 0; i < num_rios; i++) 
    {
        Corner *q = m_corners[rand() % m_corners.size()];
        if (q->m_ocean || q->m_elevation < 0.3 || q->m_elevation > 0.9) continue;

        while (!q->m_coast) 
        {
            if (q == q->m_downslope) 
            {
                break;
            }
            Edge * e = q->GetEdgeWith(q->m_downslope);
            e->m_river_volume += 1;
            q->m_river_volume += 1;
            q->m_downslope->m_river_volume += 1;
            q = q->m_downslope;
        }
    }
}

void Map::AssignCornerMoisture() 
{
    queue<Corner *> corner_queue;
    for each (Corner * c in m_corners) 
    {
        if ((c->m_water || c->m_river_volume > 0) && !c->m_ocean) 
        {
            c->m_moisture = c->m_river_volume > 0 ? min(3.0, (0.2 * c->m_river_volume)) : 1.0;
            corner_queue.push(c);
        }
        else 
        {
            c->m_moisture = 0.0;
        }
    }

    while (!corner_queue.empty()) 
    {
        Corner * c = corner_queue.front();
        corner_queue.pop();

        for each (Corner * r in c->m_corners) 
        {
            double new_moisture = c->m_moisture * 0.9;
            if (new_moisture > r->m_moisture) 
            {
                r->m_moisture = new_moisture;
                corner_queue.push(r);
            }
        }
    }

    for each (Corner * r in m_corners) 
    {
        if (r->m_ocean) 
        {
            r->m_moisture = 1.0;
            corner_queue.push(r);
        }
    }
    while (!corner_queue.empty()) 
    {
        Corner * c = corner_queue.front();
        corner_queue.pop();

        for each (Corner * r in c->m_corners) 
        {
            double new_moisture = c->m_moisture * 0.3;
            if (new_moisture > r->m_moisture) 
            {
                r->m_moisture = new_moisture;
                corner_queue.push(r);
            }
        }
    }

}

void Map::RedistributeMoisture() 
{
    vector<Corner *> locations = GetLandCorners();

    sort(locations.begin(), locations.end(), &Corner::SortByMoisture);

    for (int i = 0; i < locations.size(); i++) 
    {
        locations[i]->m_moisture = (double)i / (locations.size() - 1);
    }
}

void Map::AssignPolygonMoisture() 
{
    for each (Center * p in m_centers) 
    {
        double new_moisture = 0.0;
        for each (Corner * q in p->m_corners) 
        {
            if (q->m_moisture > 1.0)
            {
                q->m_moisture = 1.0;
            }
            new_moisture += q->m_moisture;
        }
        p->m_moisture = new_moisture / p->m_corners.size();
    }
}

void Map::AssignBiomes() 
{

    for each (Center * c in m_centers) 
    {
        if (c->m_ocean) 
        {
            c->m_biome = Biome::Type::Ocean;
        }
        else if (c->m_water) 
        {
            c->m_biome = Biome::Type::Lake;
        }
        else if (c->m_coast && c->m_moisture < 0.6) 
        {
            c->m_biome = Biome::Type::Beach;
        }
        else 
        {
            int elevation_index = 0;
            if (c->m_elevation > 0.85) 
            {
                elevation_index = 3;
            }
            else if (c->m_elevation > 0.6)
            {
                elevation_index = 2;
            }
            else if (elevation_index > 0.3) 
            {
                elevation_index = 1;
            }
            else 
            {
                elevation_index = 0;
            }

            int moisture_index = min((int)floor(c->m_moisture * 6), 5);
            c->m_biome = elevation_moisture_matrix[moisture_index][elevation_index];
        }
    }
}

void Map::FinishInfo() 
{
    Center::PVIter center_iter, centers_end = m_centers.end();
    for (center_iter = m_centers.begin(); center_iter != centers_end; center_iter++) 
    {
        (*center_iter)->SortCorners();
    }

    for each (Center * c  in m_centers)
    {
        for each (Edge * e in c->m_edges)
        {
            Center *aux_center = e->GetOpositeCenter(c);
            if (aux_center != nullptr)
            {
                c->m_centers.push_back(aux_center);
            }
        }
    }
    for each (Corner * c  in m_corners) 
    {
        for each (Edge * e in c->m_edges) 
        {
            Corner * aux_corner = e->GetOpositeCorner(c);
            if (aux_corner != nullptr)
            {
                c->m_corners.push_back(aux_corner);
            }
        }
    }
}

vector<Corner *> Map::GetLandCorners() 
{
    vector<Corner *> land_corners;
    for each (Corner * c in m_corners)
    {
        if (!c->m_water)
        {
            land_corners.push_back(c);
        }
    }
    return land_corners;
}

vector<Corner *> Map::GetLakeCorners() 
{
    vector<Corner *> lake_corners;
    for each (Corner * c in m_corners)
    {
        if (c->m_water && !c->m_ocean)
        {
            lake_corners.push_back(c);
        }
    }
    return lake_corners;
}

bool Map::IsIsland(Vector2d position) {

    double water_threshold = 0.00075;

    if (position.x < map_width * water_threshold || position.y < map_height * water_threshold
        || position.x > map_width * (1 - water_threshold) || position.y > map_height * (1 - water_threshold))
        return false;

    Vector2d center_pos = Vector2d(map_width / 2.0, map_height / 2.0);

    position -= center_pos;
    double x_coord = (position.x / map_width) * 4;
    double y_coord = (position.y / map_height) * 4;
    double noise_val = 0.;
    try
    {
        if (x_coord > 0 && y_coord > 0 && z_coord > 0)
        {
            noise_val = noiseMap->GetValue(x_coord, y_coord, z_coord);
        }
    }
    catch (...)
    {
    }

    position /= min(map_width, map_height);
    double radius = position.Length();

    double factor = radius - 0.5;
    /*	if(radius > 0.3)
    factor = -1 / log(radius - 0.3) / 10;
    else
    factor = radius - 0.3;
    */
    return noise_val >= 0.3*radius + factor;
}

void Map::Triangulate(vector<del::vertex> points) 
{
    int corner_index = 0, center_index = 0, edge_index = 0;
    m_corners.clear();
    m_centers.clear();
    m_edges.clear();
    pos_cen_map.clear();

    del::vertexSet v(points.begin(), points.end());
    del::triangleSet tris;
    del::EdgeSet edg;
    del::Delaunay dela;

    dela.Triangulate(v, tris);

    for each (del::triangle t in tris) 
    {
        Vector2d pos_center_0(t.GetVertex(0)->GetX(), t.GetVertex(0)->GetY());
        Vector2d pos_center_1(t.GetVertex(1)->GetX(), t.GetVertex(1)->GetY());
        Vector2d pos_center_2(t.GetVertex(2)->GetX(), t.GetVertex(2)->GetY());

        Center * c1 = GetCenter(pos_center_0);
        if (c1 == nullptr) 
        {
            c1 = new Center(center_index++, pos_center_0);
            m_centers.push_back(c1);
            AddCenter(c1);
        }
        Center * c2 = GetCenter(pos_center_1);
        if (c2 == nullptr) 
        {
            c2 = new Center(center_index++, pos_center_1);
            m_centers.push_back(c2);
            AddCenter(c2);
        }
        Center * c3 = GetCenter(pos_center_2);
        if (c3 == nullptr)
        {
            c3 = new Center(center_index++, pos_center_2);
            m_centers.push_back(c3);
            AddCenter(c3);
        }

        Corner * c = new Corner(corner_index++, Vector2d());
        m_corners.push_back(c);
        c->m_centers.push_back(c1);
        c->m_centers.push_back(c2);
        c->m_centers.push_back(c3);
        c1->m_corners.push_back(c);
        c2->m_corners.push_back(c);
        c3->m_corners.push_back(c);
        c->m_position = c->CalculateCircumCenter();

        Edge * e12 = c1->GetEdgeWith(c2);
        if (e12 == nullptr) 
        {
            e12 = new Edge(edge_index++, c1, c2, nullptr, nullptr);
            e12->v0 = c;
            m_edges.push_back(e12);
            c1->m_edges.push_back(e12);
            c2->m_edges.push_back(e12);
        }
        else
        {
            e12->v1 = c;
        }
        c->m_edges.push_back(e12);

        Edge * e23 = c2->GetEdgeWith(c3);
        if (e23 == nullptr) 
        {
            e23 = new Edge(edge_index++, c2, c3, nullptr, nullptr);
            e23->v0 = c;
            m_edges.push_back(e23);
            c2->m_edges.push_back(e23);
            c3->m_edges.push_back(e23);
        }
        else
        {
            e23->v1 = c;
        }
        c->m_edges.push_back(e23);

        Edge * e31 = c3->GetEdgeWith(c1);
        if (e31 == nullptr) 
        {
            e31 = new Edge(edge_index++, c3, c1, nullptr, nullptr);
            e31->v0 = c;
            m_edges.push_back(e31);
            c3->m_edges.push_back(e31);
            c1->m_edges.push_back(e31);
        }
        else 
        {
            e31->v1 = c;
        }
        c->m_edges.push_back(e31);
    }

}

void Map::AddCenter(Center * c)
{
    map<double, map<double, Center *> >::const_iterator it = pos_cen_map.find(c->m_position.x);
    if (it != pos_cen_map.end()) {
        pos_cen_map[(int)c->m_position.x][c->m_position.y] = c;
    }
    else {
        pos_cen_map[(int)c->m_position.x] = map<double, Center*>();
        pos_cen_map[(int)c->m_position.x][c->m_position.y] = c;
    }
}

Center * Map::GetCenter(Vector2d& position) 
{
    map<double, map<double, Center *> >::const_iterator it = pos_cen_map.find(position.x);
    if (it != pos_cen_map.end()) 
    {
        map<double, Center *>::const_iterator it2 = it->second.find(position.y);
        if (it2 != it->second.end()) 
        {
            return it2->second;
        }
    }
    return nullptr;
}

void Map::GeneratePoints() 
{
    //PoissonDiskSampling pds(800, 600, m_point_spread, 10);
    //vector<pair<double, double> > new_points = pds.Generate();
    //cout << "Generating " << new_points.size() << " points..." << endl;
    //for each (pair<double, double> p in new_points)
    //{
    //    points.push_back(del::vertex((int)p.first, (int)p.second));
    //}
    //points.push_back(del::vertex(-map_width, -map_height));
    //points.push_back(del::vertex(2 * map_width, -map_height));
    //points.push_back(del::vertex(2 * map_width, 2 * map_height));
    //points.push_back(del::vertex(-map_width, 2 * map_height));

    //try to simply generate hexagon grid
    Layout _layout(layout_flat, Point(10, 10), Point(0, 0));
    for (size_t i = 0; i < map_width; i += _layout.size.x)
    {
        for (size_t j = 0; j < map_height; j += _layout.size.y)
        {
            auto hex = pixel_to_hex(_layout, Point(i, j));
            auto hexInt = Hex(hex.q, hex.r, hex.s);
            auto corners = polygon_corners(_layout, hexInt);
            auto center = hex_to_pixel(_layout, hexInt);
            points.emplace_back(del::vertex((float)center.x, (float)center.y));
            for (const auto& corner : corners)
            {
                points.emplace_back(del::vertex((float)corner.x, (float)corner.y));
            }
        }
    }
}

void Map::LloydRelaxation() 
{
    vector<del::vertex> new_points;
    for each (Center * p in m_centers)
    {
        if (!p->IsInsideBoundingBox(map_width, map_height)) 
        {
            new_points.push_back(del::vertex((int)p->m_position.x, (int)p->m_position.y));
            continue;
        }
        Vector2d center_centroid;
        for each (Corner * q in p->m_corners) 
        {
            if (q->IsInsideBoundingBox(map_width, map_height)) 
            {
                center_centroid += q->m_position;
            }
            else  
            {
                Vector2d corner_pos = q->m_position;
                if (corner_pos.x < 0) 
                {
                    corner_pos.x = 0;
                }
                else if (corner_pos.x >= map_width) 
                {
                    corner_pos.x = map_width;
                }
                if (corner_pos.y < 0) 
                {
                    corner_pos.y = 0;
                }
                else if (corner_pos.y >= map_height) 
                {
                    corner_pos.y = map_height;
                }
                center_centroid += corner_pos;
            }
        }
        center_centroid /= p->m_corners.size();
        new_points.push_back(del::vertex((int)center_centroid.x, (int)center_centroid.y));
    }
    Triangulate(new_points);
}

vector<Center *> Map::GetCenters()
{
    return m_centers;
}

vector<Corner *> Map::GetCorners()
{
    return m_corners;
}

vector<Edge *> Map::GetEdges()
{
    return m_edges;
}

unsigned int Map::HashString(string seed) 
{
    unsigned int hash = 0;
    for (int i = 0; i < seed.length(); i++) 
    {
        hash += ((int)seed[i]) * pow(2, i);
    }
    return hash % UINT_MAX;
}

string Map::CreateSeed(int length)
{
    srand(time(nullptr));
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    string seed;
    for (int i = 0; i < length; ++i) 
    {
        seed.push_back(alphanum[rand() % (sizeof(alphanum) - 1)]);
    }
    return seed;
}

Center * Map::GetCenterAt(Vector2d& p_pos) 
{
    Center * r_center = nullptr;
    vector<Center *> l_aux_centers = m_centers_quadtree.QueryRange(p_pos);

    if (l_aux_centers.size() > 0) {
        double l_min_dist = Vector2d(l_aux_centers[0]->m_position, p_pos).Length();
        r_center = l_aux_centers[0];
        for (int i = 1; i < l_aux_centers.size(); i++) {
            double l_new_dist = Vector2d(l_aux_centers[i]->m_position, p_pos).Length();
            if (l_new_dist < l_min_dist) {
                l_min_dist = l_new_dist;
                r_center = l_aux_centers[i];
            }
        }
    }
    return r_center;
}