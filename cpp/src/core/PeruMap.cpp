#define _USE_MATH_DEFINES
#include "PeruMap.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath>
#include <algorithm>

PeruMap& PeruMap::instance() {
    static PeruMap inst;
    return inst;
}

bool PeruMap::load(const QString& filepath) {
    if (loaded) return true;
    
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QByteArray val = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(val);
    if (doc.isNull()) return false;
    
    QJsonObject root = doc.object();
    
    // Parse Deptos
    QJsonObject deptosObj = root["deptos"].toObject();
    for (auto it = deptosObj.begin(); it != deptosObj.end(); ++it) {
        int id = it.key().toInt();
        QJsonObject d = it.value().toObject();
        
        Depto depto;
        depto.name = d["nombre"].toString().toStdString();
        QJsonArray cent = d["centroide"].toArray();
        depto.centroid = QPointF(cent[0].toDouble(), cent[1].toDouble());
        
        QJsonArray anillos = d["anillos"].toArray();
        for (const auto& a : anillos) {
            QJsonArray anillo_pts = a.toArray();
            QPolygonF poly;
            for (const auto& p : anillo_pts) {
                QJsonArray pt = p.toArray();
                poly << QPointF(pt[0].toDouble(), pt[1].toDouble());
            }
            depto.rings.push_back(poly);
        }
        
        _deptos[id] = depto;
    }
    
    // Parse Aristas
    QJsonArray aristasArr = root["aristas"].toArray();
    for (const auto& a : aristasArr) {
        QJsonArray edge = a.toArray();
        _aristas.push_back({edge[0].toInt(), edge[1].toInt()});
    }
    
    // Parse Capitales
    if (root.contains("capitales")) {
        QJsonObject capObj = root["capitales"].toObject();
        for (auto it = capObj.begin(); it != capObj.end(); ++it) {
            int id = it.key().toInt();
            QJsonObject c = it.value().toObject();
            QJsonArray coord = c["coord"].toArray();
            _capitales[id] = {c["nombre"].toString().toStdString(), QPointF(coord[0].toDouble(), coord[1].toDouble())};
        }
    }
    
    // Parse Rutas
    if (root.contains("rutas")) {
        QJsonObject rutasObj = root["rutas"].toObject();
        for (auto it = rutasObj.begin(); it != rutasObj.end(); ++it) {
            QStringList parts = it.key().split("-");
            if (parts.size() == 2) {
                int a = parts[0].toInt();
                int b = parts[1].toInt();
                QJsonObject r = it.value().toObject();
                
                Route route;
                if (r.contains("recta")) route.straight = r["recta"].toBool();
                if (r.contains("via")) {
                    QJsonArray via = r["via"].toArray();
                    for (const auto& p : via) {
                        QJsonArray pt = p.toArray();
                        route.path.push_back(QPointF(pt[0].toDouble(), pt[1].toDouble()));
                    }
                }
                _rutas[{a, b}] = route;
            }
        }
    }
    
    loaded = true;
    return true;
}

Graph PeruMap::build_graph() const {
    std::unordered_map<int, std::string> labels;
    std::vector<int> vertices;
    for (const auto& pair : _deptos) {
        vertices.push_back(pair.first);
        labels[pair.first] = pair.second.name;
    }
    return Graph::from_edges(vertices, _aristas, labels);
}

std::unordered_map<int, QPointF> PeruMap::centroids() const {
    std::unordered_map<int, QPointF> cents;
    for (const auto& p : _deptos) cents[p.first] = p.second.centroid;
    return cents;
}

std::unordered_map<int, std::vector<QPolygonF>> PeruMap::rings() const {
    std::unordered_map<int, std::vector<QPolygonF>> r;
    for (const auto& p : _deptos) r[p.first] = p.second.rings;
    return r;
}

std::unordered_map<int, std::pair<std::string, QPointF>> PeruMap::capitals() const {
    return _capitales;
}

std::map<std::pair<int, int>, Route> PeruMap::routes() const {
    return _rutas;
}

double PeruMap::geo_distance(int a, int b) const {
    auto c = centroids();
    double lon1 = c.at(a).x();
    double lat1 = c.at(a).y();
    double lon2 = c.at(b).x();
    double lat2 = c.at(b).y();
    
    double p1 = lat1 * M_PI / 180.0;
    double p2 = lat2 * M_PI / 180.0;
    double dp = (lat2 - lat1) * M_PI / 180.0;
    double dl = (lon2 - lon1) * M_PI / 180.0;
    
    double h = std::pow(std::sin(dp / 2.0), 2) + std::cos(p1) * std::cos(p2) * std::pow(std::sin(dl / 2.0), 2);
    return 2.0 * 6371.0 * std::asin(std::sqrt(h));
}

std::map<int, std::vector<std::string>> PeruMap::frequency_table(const Graph& graph, const std::unordered_map<int, int>& color) const {
    std::map<int, std::vector<std::string>> table;
    std::vector<int> verts = graph.vertices();
    
    std::sort(verts.begin(), verts.end(), [&graph](int a, int b) {
        return graph.label(a) < graph.label(b);
    });
    
    for (int v : verts) {
        auto it = color.find(v);
        if (it != color.end() && it->second != 0) {
            table[it->second].push_back(graph.label(v));
        }
    }
    return table;
}
