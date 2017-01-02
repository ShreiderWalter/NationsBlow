
#include "Map.h"
#include "Structures.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "noise/noise.h"
#include <iostream>
#include <fstream>
#include "PoissonDiskSampling.h"

using namespace noise;
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;

const int POINT_SIZE = 2;
const int LINE_SIZE = 1;

struct InfoShown {
    enum Name {
        Elevation,
        Moisture,
        Biomes
    };
};

InfoShown::Name VideoMode;

sf::Color DELAUNAY_COLOR = sf::Color::Black;
sf::Color VORONOI_COLOR = sf::Color(sf::Uint8(52), sf::Uint8(58), sf::Uint8(94), sf::Uint8(127));
sf::Color WATER_COLOR = sf::Color(sf::Uint8(52), sf::Uint8(58), sf::Uint8(94));
sf::Color LAND_COLOR = sf::Color(sf::Uint8(178), sf::Uint8(166), sf::Uint8(148));
sf::Color LAKE_COLOR = sf::Color(sf::Uint8(95), sf::Uint8(134), sf::Uint8(169));
sf::Color RIVER_COLOR = sf::Color(sf::Uint8(40), sf::Uint8(88), sf::Uint8(132));

const sf::Color ELEVATION_COLOR[] = {
    sf::Color(sf::Uint8(104), sf::Uint8(134), sf::Uint8(89)),
    sf::Color(sf::Uint8(119), sf::Uint8(153), sf::Uint8(102)),
    sf::Color(sf::Uint8(136), sf::Uint8(166), sf::Uint8(121)),
    sf::Color(sf::Uint8(153), sf::Uint8(179), sf::Uint8(148)),
    sf::Color(sf::Uint8(170), sf::Uint8(191), sf::Uint8(159)),
    sf::Color(sf::Uint8(187), sf::Uint8(204), sf::Uint8(179)),
    sf::Color(sf::Uint8(204), sf::Uint8(217), sf::Uint8(198)),
    sf::Color(sf::Uint8(221), sf::Uint8(230), sf::Uint8(217)),
    sf::Color(sf::Uint8(238), sf::Uint8(242), sf::Uint8(236)),
    sf::Color(sf::Uint8(251), sf::Uint8(252), sf::Uint8(251)) };

const sf::Color MOISTURE_COLOR[] = {
    sf::Color(sf::Uint8(238), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(218), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(197), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(176), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(155), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(135), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(115), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(94), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(73), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(52), sf::Uint8(238), sf::Uint8(32)),
    sf::Color(sf::Uint8(32), sf::Uint8(238), sf::Uint8(32)), };

const sf::Color BIOME_COLOR[] = {
    sf::Color(sf::Uint8(248), sf::Uint8(248), sf::Uint8(248)),
    sf::Color(sf::Uint8(221), sf::Uint8(221), sf::Uint8(187)),
    sf::Color(sf::Uint8(153), sf::Uint8(153), sf::Uint8(153)),
    sf::Color(sf::Uint8(204), sf::Uint8(212), sf::Uint8(187)),
    sf::Color(sf::Uint8(196), sf::Uint8(204), sf::Uint8(187)),
    sf::Color(sf::Uint8(228), sf::Uint8(232), sf::Uint8(202)),
    sf::Color(sf::Uint8(164), sf::Uint8(196), sf::Uint8(168)),
    sf::Color(sf::Uint8(180), sf::Uint8(201), sf::Uint8(169)),
    sf::Color(sf::Uint8(196), sf::Uint8(212), sf::Uint8(170)),
    sf::Color(sf::Uint8(156), sf::Uint8(187), sf::Uint8(169)),
    sf::Color(sf::Uint8(169), sf::Uint8(204), sf::Uint8(164)),
    sf::Color(sf::Uint8(233), sf::Uint8(221), sf::Uint8(199)),
    sf::Color(sf::Uint8(52), sf::Uint8(58), sf::Uint8(94)),
    sf::Color(sf::Uint8(95), sf::Uint8(134), sf::Uint8(169)),
    sf::Color(sf::Uint8(178), sf::Uint8(166), sf::Uint8(148)) };

void drawLine(Vector2d a, Vector2d b, double width, sf::Color c, sf::RenderWindow *window);
void drawEdge(Edge *e, sf::RenderWindow *window);
void drawCorner(Corner *c, sf::RenderWindow *window);
void drawCenter(Center *c, sf::RenderWindow *window);

struct city 
{
    string name;
    Center * cell;
};

int main() 
{
    /*

            double depth = floor((log(3000) / log(4)) + 0.5);

            cout << depth << endl;


            system("pause");
            */

    vector<string> names;
    ifstream names_file;
    names_file.open("../Resources/BrittanyPlaces2.txt", ios::in);

    //if (!names_file.is_open())
    //    return 0;
    sf::Clock timer;
    //while (!names_file.eof()) 
    //{
    //    string line;
    //    getline(names_file, line);
    //    names.push_back(line);
    //}
    //MarkovNames mng(names, 3, 5);

    VideoMode = InfoShown::Biomes;

    sf::RenderWindow * app = new sf::RenderWindow(sf::VideoMode(WIDTH, WIDTH, 32), "Map Generator");
    app->setFramerateLimit(60);

    Map mapa(WIDTH, WIDTH, 10, "");

    timer.restart();
    mapa.Generate();
    cout << timer.getElapsedTime().asMicroseconds() / 1000.0 << endl;

    vector<Edge *> edges = mapa.GetEdges();
    vector<Corner *> corners = mapa.GetCorners();
    vector<Center *> centers = mapa.GetCenters();

    vector<sf::ConvexShape> polygons;
    Center::PVIter center_iter, centers_end = centers.end();
    for (center_iter = centers.begin(); center_iter != centers_end; center_iter++) 
    {
        sf::ConvexShape polygon;
        polygon.setPointCount((*center_iter)->m_corners.size());
        for (int i = 0; i < (*center_iter)->m_corners.size(); i++) 
        {
            Vector2d aux = (*center_iter)->m_corners[i]->m_position;
            polygon.setPoint(i, sf::Vector2f(aux.x, aux.y));
        }
        polygon.setFillColor(BIOME_COLOR[(int)(*center_iter)->m_biome]);
        polygon.setPosition(0, 0);
        polygons.push_back(polygon);
    }

    vector<city> ciudades;
    for (int i = 0; i < 5; i++) 
    {
        city ciudad;
        do
        {
            ciudad.cell = centers[rand() % centers.size()];
        } while (ciudad.cell->m_water);

        //ciudad.name = mng.GetName();

        ciudades.push_back(ciudad);
    }

    double avg_fps = 0;
    double pases_count = 0;
    Center * selected_center = NULL;

    bool running = true;
    while (running) 
    {
        sf::Event event;
        while (app->pollEvent(event)) 
        {
            if (event.type == sf::Event::Closed) 
            {
                running = false;
            }
            else if (event.type == sf::Event::KeyPressed) 
            {
                sf::Image screen;
                switch (event.key.code) {
                case sf::Keyboard::Escape:
                    running = false;
                    break;
                case sf::Keyboard::M:
                    VideoMode = InfoShown::Moisture;
                    break;
                case sf::Keyboard::B:
                    VideoMode = InfoShown::Biomes;
                    break;
                case sf::Keyboard::E:
                    VideoMode = InfoShown::Elevation;
                    break;
                case sf::Keyboard::F1:
                    //screen = app->capture();
                    screen.saveToFile("screenshot.jpg");
                    break;
                default:
                    break;
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed) 
            {
                if (event.mouseButton.button == sf::Mouse::Button::Left) 
                {
                    timer.restart();
                    selected_center = mapa.GetCenterAt(Vector2d(event.mouseButton.x, event.mouseButton.y));
                    cout << timer.getElapsedTime().asMicroseconds() << endl;
                }
            }
        }

        app->clear(sf::Color::White);

        if (!centers.empty())
        {
            timer.restart();
            Center::PVIter cells_iter;
            for (cells_iter = centers.begin(); cells_iter != centers.end(); cells_iter++) 
            {
                drawCenter(*cells_iter, app);
            }
            cout << timer.getElapsedTime().asMicroseconds() << endl;
        }
        if (!edges.empty())
        {
            Edge::PVIter edge_iter, edges_end = edges.end();
            for (edge_iter = edges.begin(); edge_iter != edges_end; edge_iter++)
            {
                drawEdge(*edge_iter, app);
            }
        }

        if (0 && !corners.empty()) 
        {
            Corner::PVIter corner_iter, corners_end = corners.end();
            for (corner_iter = corners.begin(); corner_iter != corners_end; corner_iter++) 
            {
                drawCorner(*corner_iter, app);
            }
        }

        if (!ciudades.empty()) 
        {
            for (size_t i = 0; i < ciudades.size(); i++) 
            {
                city ciudad = ciudades[i];
                sf::CircleShape p;
                p.setFillColor(sf::Color::Red);
                p.setRadius(POINT_SIZE);
                p.setPosition(ciudad.cell->m_position.x - POINT_SIZE, ciudad.cell->m_position.y - POINT_SIZE);
                app->draw(p);

                auto cityNameStr = sf::String(ciudad.name);
                sf::Text name;

                name.setString(cityNameStr);
                name.setPosition(ciudad.cell->m_position.x - POINT_SIZE, ciudad.cell->m_position.y - POINT_SIZE - 20);
                //name.setColor(sf::Color(sf::Uint8(34), sf::Uint8(34), sf::Uint8(34)));
                name.setStyle(sf::Text::Bold);
                name.setCharacterSize(15);
                app->draw(name);
            }
        }

        if (selected_center != nullptr) 
        {
            sf::ConvexShape polygon;
            polygon.setPointCount(selected_center->m_corners.size());
            for (int i = 0; i < selected_center->m_corners.size(); i++) {

                Vector2d aux = selected_center->m_corners[i]->m_position;
                polygon.setPoint(i, sf::Vector2f(aux.x, aux.y));
            }
            polygon.setFillColor(sf::Color::Black);
            polygon.setPosition(0, 0);
            polygons.push_back(polygon);
            app->draw(polygon);
        }

        app->display();
    }

    return 0;
}


void drawLine(Vector2d a, Vector2d b, double width, sf::Color c, sf::RenderWindow *window) 
{

    Vector2d line_vec(a, b);
    sf::RectangleShape line(sf::Vector2f(line_vec.Length(), width));

    line.setFillColor(c);
    line.setRotation(line_vec.AngleAng());
    line.setPosition(a.x, a.y);

    window->draw(line);
}

void drawEdge(Edge *e, sf::RenderWindow *window) {

    Vector2d v0;
    if (e->v0 == nullptr)
    {
        v0 = (e->d0->m_position + e->d1->m_position) / 2;
    }
    else
    {
        v0 = e->v0->m_position;
    }
    Vector2d v1;
    if (e->v1 == nullptr)
    {
        v1 = (e->d0->m_position + e->d1->m_position) / 2;
    }
    else
    {
        v1 = e->v1->m_position;
    }
    if (e->m_river_volume > 0)
    {
        drawLine(v0, v1, 1 + sqrt(e->m_river_volume), RIVER_COLOR, window);
    }
    else 
    {
        drawLine(v0, v1, 1, VORONOI_COLOR, window);
    }

    //drawLine(e->d0->position, e->d1->position, 1, DELAUNAY_COLOR, window);
}

void drawCorner(Corner *c, sf::RenderWindow *window) 
{
    sf::CircleShape point;
    if (c->m_water)
    {
        point.setFillColor(WATER_COLOR);
    }
    else
    {
        point.setFillColor(LAND_COLOR);
    }

    //point.setFillColor(VORONOI_COLOR);
    point.setPosition(c->m_position.x - POINT_SIZE, c->m_position.y - POINT_SIZE);
    point.setRadius(POINT_SIZE);
    window->draw(point);
}

void drawCenter(Center *c, sf::RenderWindow *window) 
{
    sf::ConvexShape polygon;
    polygon.setPointCount(c->m_corners.size());
    for (int i = 0; i < c->m_corners.size(); i++) 
    {
        Vector2d aux = c->m_corners[i]->m_position;
        polygon.setPoint(i, sf::Vector2f(aux.x, aux.y));
    }
    switch (VideoMode) 
    {
    case InfoShown::Biomes:
        polygon.setFillColor(BIOME_COLOR[(int)c->m_biome]);
        break;
    case InfoShown::Elevation:
        if (c->m_ocean) 
        {
            polygon.setFillColor(WATER_COLOR);
        }
        else if (c->m_water) 
        {
            polygon.setFillColor(LAKE_COLOR);
        }
        else 
        {
            polygon.setFillColor(ELEVATION_COLOR[(int)floor(c->m_elevation * 10)]);
        }
        break;
    case InfoShown::Moisture:
        if (c->m_ocean) 
        {
            polygon.setFillColor(WATER_COLOR);
        }
        else if (c->m_water) 
        {
            polygon.setFillColor(LAKE_COLOR);
        }
        else 
        {
            polygon.setFillColor(MOISTURE_COLOR[(int)floor(c->m_moisture * 10)]);
        }
    default:
        break;
    }
    polygon.setPosition(0, 0);
    window->draw(polygon);

    return;
    sf::CircleShape p;
    p.setFillColor(sf::Color::Black);
    p.setRadius(POINT_SIZE);
    p.setPosition(c->m_position.x - POINT_SIZE, c->m_position.y - POINT_SIZE);
    window->draw(p);

}