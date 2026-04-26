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
#include <optional>
#include <stack>
#include <unordered_map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

template <typename Activation> struct ActivationOptions {
    std::vector<Activation> activations;
    int weight;
};

template <typename Vertex, typename Activation> struct Edge {
    Vertex source;
    Vertex destination;
    ActivationOptions<Activation> options;
};

template <typename T> struct Property {
    T properties;
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
    using Graph
        = boost::adjacency_list<boost::hash_setS, boost::vecS, boost::directedS,
                                Property<Vertex>,
                                Property<ActivationOptions<Activation>>>;
    using VertexId = boost::graph_traits<Graph>::vertex_descriptor;
    using EdgeId = boost::graph_traits<Graph>::edge_descriptor;

    Graph m_graph;
    std::unordered_map<Vertex, VertexId> m_vertex_lookup;
    std::stack<VertexId> m_unprocessed_vertices;
    VertexId m_root_vertex_id;

    VertexId add_or_get_vertex(const Vertex& vertex)
    {
        auto [iter, inserted] = m_vertex_lookup.emplace(vertex, VertexId {});
        if (!inserted) {
            return iter->second;
        }

        const auto vertex_id = boost::add_vertex(m_graph);
        iter->second = vertex_id;
        m_unprocessed_vertices.push(vertex_id);
        m_graph[vertex_id].properties = vertex;
        return vertex_id;
    }

    void add_continuation_from_vertex(
        std::unordered_map<VertexId, std::tuple<std::optional<EdgeId>, int>>&
            optimal_continuations,
        VertexId vertex) const
    {
        const auto [begin, end] = boost::out_edges(vertex, m_graph);
        if (begin == end) {
            optimal_continuations.emplace(vertex, std::tuple {std::nullopt, 0});
            return;
        }

        const auto subpath_weight = [&](const auto& edge) {
            return m_graph[edge].properties.weight
                + std::get<1>(
                       optimal_continuations.at(boost::target(edge, m_graph)));
        };

        auto best_edge = *begin;
        auto best_weight = subpath_weight(best_edge);
        for (auto it = std::next(begin); it != end; ++it) {
            const auto weight = subpath_weight(*it);
            if (weight > best_weight) {
                best_edge = *it;
                best_weight = weight;
            }
        }

        optimal_continuations.emplace(vertex,
                                      std::tuple {best_edge, best_weight});
    }

public:
    PathGraph(Vertex root_vertex)
    {
        const auto root_vertex_id = boost::add_vertex(m_graph);
        m_graph[root_vertex_id].properties = root_vertex;
        m_unprocessed_vertices.push(root_vertex_id);
        m_vertex_lookup.emplace(root_vertex, root_vertex_id);
        m_root_vertex_id = root_vertex_id;
    }

    void add_activation(Vertex source, Vertex destination, int weight,
                        std::optional<Activation> activation)
    {
        const auto source_id = add_or_get_vertex(source);
        const auto destination_id = add_or_get_vertex(destination);

        const auto [edge_id, inserted]
            = boost::add_edge(source_id, destination_id, m_graph);
        auto& properties = m_graph[edge_id].properties;
        if (inserted) {
            std::vector<Activation> activations;
            if (activation.has_value()) {
                activations.push_back(std::move(*activation));
            }
            properties = ActivationOptions<Activation> {std::move(activations),
                                                        weight};
            return;
        }

        if (properties.weight > weight) {
            return;
        }
        if (properties.weight < weight) {
            properties.activations.clear();
            properties.weight = weight;
        }
        if (activation.has_value()) {
            properties.activations.push_back(std::move(*activation));
        }
    }

    [[nodiscard]] std::vector<Edge<Vertex, Activation>> optimal_path() const
    {
        std::vector<VertexId> reverse_topological_sort;
        boost::topological_sort(m_graph,
                                std::back_inserter(reverse_topological_sort));

        std::unordered_map<VertexId, std::tuple<std::optional<EdgeId>, int>>
            optimal_continuations;
        for (const auto& vertex : reverse_topological_sort) {
            add_continuation_from_vertex(optimal_continuations, vertex);
        }

        std::vector<Edge<Vertex, Activation>> path;
        const auto& [initial_edge, _]
            = optimal_continuations.at(m_root_vertex_id);
        const auto next_edge = [&](const auto& edge) {
            const auto vertex = boost::target(edge, m_graph);
            return std::get<0>(optimal_continuations.at(vertex));
        };
        for (auto edge = initial_edge; edge.has_value();
             edge = next_edge(*edge)) {
            const auto src_vertex_id = boost::source(*edge, m_graph);
            const auto dest_vertex_id = boost::target(*edge, m_graph);
            path.emplace_back(m_graph[src_vertex_id].properties,
                              m_graph[dest_vertex_id].properties,
                              m_graph[*edge].properties);
        }

        return path;
    }

    std::optional<Vertex> next_unprocessed_vertex()
    {
        if (m_unprocessed_vertices.empty()) {
            return std::nullopt;
        }
        const auto vertex_id = m_unprocessed_vertices.top();
        m_unprocessed_vertices.pop();
        return m_graph[vertex_id].properties;
    }
};

#endif
