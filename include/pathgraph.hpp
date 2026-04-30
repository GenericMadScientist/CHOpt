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
#include <vector>

#include <boost/unordered/unordered_flat_map.hpp>

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
    using OutEdgeSet
        = boost::unordered_flat_map<std::size_t, ActivationOptions<Activation>>;

    std::vector<std::tuple<Vertex, OutEdgeSet>> m_adjacency_list;
    boost::unordered_flat_map<Vertex, std::size_t> m_vertex_lookup;
    std::stack<std::size_t> m_unprocessed_vertices;

    std::size_t add_or_get_vertex(const Vertex& vertex)
    {
        const auto [iter, inserted]
            = m_vertex_lookup.emplace(vertex, m_adjacency_list.size());
        if (inserted) {
            m_adjacency_list.push_back({vertex, {}});
            m_unprocessed_vertices.push(iter->second);
        }

        return iter->second;
    }

    void add_continuation_from_vertex(
        boost::unordered_flat_map<std::size_t,
                                  std::tuple<std::optional<std::size_t>, int>>&
            optimal_continuations,
        std::size_t vertex) const
    {
        const auto& out_edges = std::get<1>(m_adjacency_list.at(vertex));
        if (out_edges.empty()) {
            optimal_continuations.emplace(vertex, std::tuple {std::nullopt, 0});
            return;
        }

        const auto subpath_weight = [&](const auto& edge) {
            const auto& [dest, edge_properties] = edge;
            return edge_properties.weight
                + std::get<1>(optimal_continuations.at(dest));
        };

        const auto begin = out_edges.cbegin();
        auto best_edge = *begin;
        auto best_weight = subpath_weight(best_edge);
        for (auto it = std::next(begin); it != out_edges.cend(); ++it) {
            const auto weight = subpath_weight(*it);
            if (weight > best_weight) {
                best_edge = *it;
                best_weight = weight;
            }
        }

        optimal_continuations.emplace(
            vertex, std::tuple {std::get<0>(best_edge), best_weight});
    }

public:
    PathGraph(Vertex root_vertex)
    {
        m_adjacency_list.push_back({root_vertex, {}});
        m_unprocessed_vertices.push(0);
        m_vertex_lookup.emplace(std::move(root_vertex), 0);
    }

    void add_activation(Vertex source, Vertex destination, int weight,
                        std::optional<Activation> activation)
    {
        const auto source_id = add_or_get_vertex(source);
        const auto destination_id = add_or_get_vertex(destination);

        auto& out_edge_set = std::get<1>(m_adjacency_list.at(source_id));
        auto [iter, inserted] = out_edge_set.emplace(
            destination_id, ActivationOptions<Activation> {{}, 0});
        if (inserted) {
            std::vector<Activation> activations;
            if (activation.has_value()) {
                activations.push_back(std::move(*activation));
            }
            iter->second = ActivationOptions<Activation> {
                std::move(activations), weight};
            return;
        }

        if (iter->second.weight > weight) {
            return;
        }
        if (iter->second.weight < weight) {
            iter->second.activations.clear();
            iter->second.weight = weight;
        }
        if (activation.has_value()) {
            iter->second.activations.push_back(std::move(*activation));
        }
    }

    void visit_vertex(
        std::size_t vertex,
        boost::unordered_flat_map<std::size_t,
                                  std::tuple<int, std::optional<std::size_t>>>&
            optimal_continuations) const
    {
        if (optimal_continuations.contains(vertex)) {
            return;
        }

        const auto& out_edges = std::get<1>(m_adjacency_list.at(vertex));
        if (out_edges.empty()) {
            optimal_continuations[vertex] = {0, {}};
            return;
        }

        for (const auto& [out_vertex, _] : out_edges) {
            visit_vertex(out_vertex, optimal_continuations);
        }

        int best_weight = std::numeric_limits<int>::min();
        std::size_t best_out_vertex = 0;

        for (const auto& [out_vertex, act_opts] : out_edges) {
            auto new_weight = act_opts.weight
                + std::get<0>(optimal_continuations.at(out_vertex));
            if (new_weight > best_weight) {
                best_weight = new_weight;
                best_out_vertex = out_vertex;
            }
        }

        optimal_continuations.insert({vertex, {best_weight, best_out_vertex}});
    }

    [[nodiscard]] std::vector<Edge<Vertex, Activation>> optimal_path() const
    {
        boost::unordered_flat_map<std::size_t,
                                  std::tuple<int, std::optional<std::size_t>>>
            optimal_continuations;
        visit_vertex(0, optimal_continuations);

        std::vector<Edge<Vertex, Activation>> path;
        std::size_t src_vertex = 0;
        while (true) {
            const auto dest_vertex
                = std::get<1>(optimal_continuations[src_vertex]);
            if (!dest_vertex.has_value()) {
                break;
            }
            path.emplace_back(
                std::get<0>(m_adjacency_list.at(src_vertex)),
                std::get<0>(m_adjacency_list.at(*dest_vertex)),
                std::get<1>(m_adjacency_list.at(src_vertex)).at(*dest_vertex));
            src_vertex = *dest_vertex;
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
        return std::get<0>(m_adjacency_list[vertex_id]);
    }

    [[nodiscard]] Vertex root_vertex() const
    {
        return std::get<0>(m_adjacency_list[0]);
    }
};

#endif
