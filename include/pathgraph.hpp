/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2026 Raymond Wright
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CHOPT_PATH_GRAPH_HPP
#define CHOPT_PATH_GRAPH_HPP

#include <algorithm>
#include <limits>
#include <optional>
#include <set>
#include <stack>
#include <vector>

template <typename Vertex, typename Activation> struct Edge {
    Vertex source;
    Vertex destination;
    int weight;
    std::vector<Activation> activations;
};

// This data structure is for the directed acyclic graph at the core of the
// optimiser. Vertices represent a state of how far through the song you are,
// which can include things like the next note and your current combo. Edges
// represent activations, with the weight being the number of points gained from
// using a specific activation. A Star Power path is then a path through this
// graph, and an optimal path is a path with maximum total weight.
//
// It is useful for an edge to have a vector of activations associated with it,
// rather than just one. This serves two purposes. The first is that multiple
// equally good (for points) activations can have the same start and end
// vertices. It is then up to the optimiser to pick between the activations. The
// second is that some vertices can be dummy vertices used for optimisation,
// e.g. vertices that represent having a full bar at a certain point. An empty
// vector of activations denotes that the edge's destination is one of these
// dummy vertices.
//
// The graph also contains a queue of unprocessed vertices. The idea is that the
// graph is generated incrementally, and we want to know which vertices already
// have all their out edges calculated. To do this, we keep a queue of vertices
// that are the destination of edges that have not yet had all their out edges
// calculated. Then the optimiser can pop a vertex off the queue, add its out
// edges, and loop until the queue is empty.
template <typename Vertex, typename Activation> class PathGraph {
private:
    std::stack<Vertex> m_unprocessed_vertices;
    std::set<Vertex> m_vertices;
    std::vector<Edge<Vertex, Activation>> m_edges;
    Vertex m_root_vertex;

    [[nodiscard]] std::vector<Edge<Vertex, Activation>>
    optimal_subpath_from_vertex(const Vertex& vertex) const
    {
        auto best_weight = std::numeric_limits<int>::min();
        std::vector<Edge<Vertex, Activation>> best_path;

        for (const auto& edge : m_edges) {
            if (edge.source != vertex) {
                continue;
            }
            auto subpath = optimal_subpath_from_vertex(edge.destination);
            auto subpath_weight = edge.weight;
            for (const auto& subpath_edge : subpath) {
                subpath_weight += subpath_edge.weight;
            }

            if (subpath_weight > best_weight) {
                best_weight = subpath_weight;
                best_path = std::move(subpath);
                best_path.insert(best_path.begin(), edge);
            }
        }

        return best_path;
    }

public:
    PathGraph(Vertex root_vertex)
        : m_unprocessed_vertices {{root_vertex}}
        , m_root_vertex {std::move(root_vertex)}
    {
    }

    void add_activation(Vertex source, Vertex destination, int weight,
                        std::optional<Activation> activation)
    {
        const auto result = m_vertices.insert(destination);
        if (result.second) {
            m_unprocessed_vertices.push(destination);
        }

        const auto edge_iter = std::ranges::find_if(
            m_edges, [&](const auto& edge) {
                return edge.source == source && edge.destination == destination;
            });
        if (edge_iter != std::ranges::end(m_edges)) {
            if (edge_iter->weight > weight) {
                return;
            }
            if (edge_iter->weight < weight) {
                edge_iter->activations.clear();
                edge_iter->weight = weight;
            }
            if (activation.has_value()) {
                edge_iter->activations.push_back(std::move(*activation));
            }
            return;
        }

        std::vector<Activation> activations;
        if (activation.has_value()) {
            activations.push_back(std::move(*activation));
        }
        m_edges.emplace_back(std::move(source), std::move(destination), weight,
                             std::move(activations));
    }

    [[nodiscard]] std::vector<Edge<Vertex, Activation>> optimal_path() const
    {
        return optimal_subpath_from_vertex(m_root_vertex);
    }

    std::optional<Vertex> next_unprocessed_vertex()
    {
        if (m_unprocessed_vertices.empty()) {
            return std::nullopt;
        }
        const auto vertex = m_unprocessed_vertices.top();
        m_unprocessed_vertices.pop();
        return vertex;
    }
};

#endif
