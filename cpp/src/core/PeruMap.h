#ifndef PERUMAP_H
#define PERUMAP_H

#include "Graph.h"
#include <QString>
#include <QPointF>
#include <QPolygonF>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>

struct Route {
    std::vector<QPointF> path;
    bool straight = false;
};

class PeruMap {
public:
    static PeruMap& instance();
    
    bool load(const QString& filepath);
    
    Graph build_graph() const;
    
    std::unordered_map<int, QPointF> centroids() const;
    std::unordered_map<int, std::vector<QPolygonF>> rings() const;
    std::unordered_map<int, std::pair<std::string, QPointF>> capitals() const;
    std::map<std::pair<int, int>, Route> routes() const;
    
    double geo_distance(int a, int b) const;
    std::map<int, std::vector<std::string>> frequency_table(const Graph& graph, const std::unordered_map<int, int>& color) const;

private:
    PeruMap() = default;
    ~PeruMap() = default;
    
    struct Depto {
        std::string name;
        QPointF centroid;
        std::vector<QPolygonF> rings;
    };
    
    std::unordered_map<int, Depto> _deptos;
    std::vector<std::pair<int, int>> _aristas;
    std::unordered_map<int, std::pair<std::string, QPointF>> _capitales;
    std::map<std::pair<int, int>, Route> _rutas;
    
    bool loaded = false;
};

#endif // PERUMAP_H
