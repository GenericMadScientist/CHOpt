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
#include <utility>
#include <vector>

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

// This data structure is for the directed acyclic graph at the core of the
// optimiser. Vertices represent a state of how far through the song you are,
// which can include things like the next note and your current combo. Edges
// represent activations, with the weight being the number of points gained from
// using a specific activation. A Star Power path is then a path through this
// graph, and an optimal path is a path with maximum total weight.
template <typename VertexProperty, typename EdgeProperty> class PathGraph {
public:
    using VertexId = std::size_t;

    struct Edge {
        VertexId dest_vertex_id;
        int weight;
        EdgeProperty property;
    };

private:
    std::vector<std::vector<Edge>> m_adjacency_list;
    std::vector<VertexProperty> m_vertex_properties;
    boost::unordered_flat_map<VertexProperty, VertexId>
        m_reverse_vertex_property_lookup;
    boost::unordered_flat_map<VertexId, int> m_optimal_subpath_values;

public:
    explicit PathGraph(VertexProperty root_vertex)
    {
        m_adjacency_list.emplace_back();
        m_vertex_properties.push_back(root_vertex);
        m_reverse_vertex_property_lookup.emplace(std::move(root_vertex), 0);
    }

    std::pair<VertexId, bool> insert_vertex(VertexProperty vertex)
    {
        const auto [iter, inserted] = m_reverse_vertex_property_lookup.emplace(
            vertex, m_adjacency_list.size());
        if (inserted) {
            m_adjacency_list.emplace_back();
            m_vertex_properties.emplace_back(std::move(vertex));
        }

        return {iter->second, inserted};
    }

    void add_edge(VertexId source_id, VertexId destination_id, int weight,
                  EdgeProperty edge_property)
    {
        m_adjacency_list.at(source_id).emplace_back(destination_id, weight,
                                                    edge_property);
    }

    [[nodiscard]] const std::vector<Edge>& out_edges(VertexId vertex_id) const
    {
        return m_adjacency_list.at(vertex_id);
    }

    [[nodiscard]] VertexId root_vertex_id() const { return 0; }

    [[nodiscard]] const VertexProperty&
    vertex_property(VertexId vertex_id) const
    {
        return m_vertex_properties.at(vertex_id);
    }

    void prune_suboptimal_out_edges(VertexId vertex_id)
    {
        auto& out_edge_set = m_adjacency_list.at(vertex_id);
        const auto subpath_weight = [&](const auto& edge) {
            return edge.weight
                + m_optimal_subpath_values.at(edge.dest_vertex_id);
        };
        const auto max_value_edge
            = std::ranges::max_element(out_edge_set, {}, subpath_weight);
        auto optimal_subpath_value = 0;
        if (max_value_edge != std::ranges::end(out_edge_set)) {
            optimal_subpath_value = subpath_weight(*max_value_edge);
        }
        m_optimal_subpath_values.emplace(vertex_id, optimal_subpath_value);

        const auto has_optimal_weight = [&](const auto& edge) {
            return subpath_weight(edge) == optimal_subpath_value;
        };
        const auto suboptimal_range
            = std::ranges::partition(out_edge_set, has_optimal_weight);
        const auto number_of_optimal_acts
            = std::distance(std::ranges::begin(out_edge_set),
                            std::ranges::begin(suboptimal_range));
        out_edge_set.resize(number_of_optimal_acts);
        out_edge_set.shrink_to_fit();
    }
};

// This is an auxiliary data structure used to deduplicate edges before
// insertion into PathGraph. This keeps a vector of the best activations
// associated with an edge. By using this auxiliary data structure rather than
// putting the deduplication and best activation loginc in PathGraph, we can use
// vectors instead of hash tables in the lists of out edges for each vertex. We
// also allow an empty vector of activations to denote an edge to a dummy
// vertex, which are used for representing points where we must have maximum SP.
// Allowing these are a powerful optimisation.
template <typename Vertex, typename Activation> class OutEdgeAggregate {
public:
    struct Edge {
        Vertex dest_vertex;
        int weight;
        std::vector<Activation> activations;
    };

private:
    std::vector<Edge> m_out_edges;
    boost::unordered_flat_map<Vertex, std::size_t> m_vertex_indexes;

public:
    using iterator = std::vector<Edge>::iterator;

    void add_activation(Vertex dest_vertex,
                        std::optional<Activation> activation, int weight)
    {
        const auto [iter, inserted]
            = m_vertex_indexes.emplace(dest_vertex, m_out_edges.size());
        if (inserted) {
            m_out_edges.push_back({dest_vertex, weight, {}});
            if (activation.has_value()) {
                m_out_edges.back().activations.push_back(
                    std::move(*activation));
            }
            return;
        }

        auto& edge = m_out_edges.at(iter->second);
        if (edge.weight > weight) {
            return;
        }
        if (edge.weight < weight) {
            edge.activations.clear();
            edge.weight = weight;
        }
        if (activation.has_value()) {
            edge.activations.push_back(std::move(*activation));
        }
    }

    [[nodiscard]] iterator begin() { return m_out_edges.begin(); }
    [[nodiscard]] iterator end() { return m_out_edges.end(); }
};

template <typename VertexProperty, typename EdgeProperty, typename F>
inline PathGraph<VertexProperty, EdgeProperty>
generate_optimal_graph(VertexProperty root_vertex, F out_edges)
{
    std::stack<std::tuple<std::size_t, bool>> unprocessed_vertices {
        {{0, false}}};
    boost::unordered_flat_set<std::size_t> vertices_with_out_edges;

    PathGraph<VertexProperty, EdgeProperty> graph {std::move(root_vertex)};

    while (!unprocessed_vertices.empty()) {
        const auto [vertex_id, ready_to_prune] = unprocessed_vertices.top();
        unprocessed_vertices.pop();
        if (ready_to_prune) {
            graph.prune_suboptimal_out_edges(vertex_id);
            continue;
        }

        if (vertices_with_out_edges.contains(vertex_id)) {
            continue;
        }
        vertices_with_out_edges.emplace(vertex_id);
        unprocessed_vertices.emplace(vertex_id, true);
        auto edges = out_edges(graph, vertex_id);

        for (auto& edge : edges) {
            const auto [dest_vertex_id, inserted]
                = graph.insert_vertex(edge.dest_vertex);
            unprocessed_vertices.emplace(dest_vertex_id, false);
            graph.add_edge(vertex_id, dest_vertex_id, edge.weight,
                           std::move(edge.activations));
        }
    }

    return graph;
}

#endif
