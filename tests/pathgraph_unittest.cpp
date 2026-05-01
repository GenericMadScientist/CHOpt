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

#include <ostream>
#include <tuple>

#include <boost/test/unit_test.hpp>

#include "pathgraph.hpp"

using TestGraph = PathGraph<int, int>;
using TestAggregate = OutEdgeAggregate<int, int>;

bool operator==(const TestGraph::Edge& lhs, const TestGraph::Edge& rhs)
{
    return std::tie(lhs.dest_vertex_id, lhs.weight, lhs.property)
        == std::tie(rhs.dest_vertex_id, rhs.weight, rhs.property);
}

std::ostream& operator<<(std::ostream& stream, const TestGraph::Edge& edge)
{
    stream << "{Destination " << edge.dest_vertex_id << ", Weight "
           << edge.weight << ", Property " << edge.property << '}';
    return stream;
}

BOOST_AUTO_TEST_SUITE(insert_vertex)

BOOST_AUTO_TEST_CASE(returns_id_and_true_for_new_vertex)
{
    TestGraph graph {100};

    const auto [id, inserted] = graph.insert_vertex(200);

    BOOST_CHECK_EQUAL(id, 1U);
    BOOST_CHECK(inserted);
}

BOOST_AUTO_TEST_CASE(returns_id_and_false_for_duplicate_vertex)
{
    TestGraph graph {100};

    graph.insert_vertex(200);
    const auto [id, inserted] = graph.insert_vertex(200);

    BOOST_CHECK_EQUAL(id, 1U);
    BOOST_CHECK(!inserted);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(adds_edge_to_out_edges)
{
    TestGraph graph {100};

    graph.insert_vertex(200);
    graph.add_edge(0, 1, -1, 5);
    const auto& out_edges = graph.out_edges(0);

    const std::vector<TestGraph::Edge> expected_edges {
        {.dest_vertex_id = 1, .weight = -1, .property = 5}};

    BOOST_CHECK_EQUAL_COLLECTIONS(out_edges.cbegin(), out_edges.cend(),
                                  expected_edges.cbegin(),
                                  expected_edges.cend());
}

BOOST_AUTO_TEST_CASE(root_vertex_id_returns_zero)
{
    TestGraph graph {100};

    BOOST_CHECK_EQUAL(graph.root_vertex_id(), 0U);
}

BOOST_AUTO_TEST_SUITE(vertex_property)

BOOST_AUTO_TEST_CASE(returns_property_of_root_vertex)
{
    TestGraph graph {100};

    BOOST_CHECK_EQUAL(graph.vertex_property(0), 100);
}

BOOST_AUTO_TEST_CASE(returns_property_of_added_vertex)
{
    TestGraph graph {100};

    graph.insert_vertex(200);

    BOOST_CHECK_EQUAL(graph.vertex_property(1), 200);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(prune_suboptimal_out_edges)

BOOST_AUTO_TEST_CASE(fails_if_destinations_havent_been_pruned)
{
    TestGraph graph {100};

    graph.insert_vertex(200);
    graph.add_edge(0, 1, 50, 0);

    BOOST_CHECK_THROW([&] { graph.prune_suboptimal_out_edges(0); }(),
                      std::out_of_range);
}

BOOST_AUTO_TEST_CASE(prunes_suboptimal_edges_if_destinations_have_been_pruned)
{
    TestGraph graph {100};

    graph.insert_vertex(200);
    graph.insert_vertex(300);
    graph.add_edge(0, 1, 50, 0);
    graph.add_edge(0, 2, 100, 0);

    for (int i = 2; i >= 0; --i) {
        graph.prune_suboptimal_out_edges(i);
    }

    const auto& out_edges = graph.out_edges(0);
    const std::vector<TestGraph::Edge> expected_out_edges {
        {.dest_vertex_id = 2, .weight = 100, .property = 0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(out_edges.cbegin(), out_edges.cend(),
                                  expected_out_edges.cbegin(),
                                  expected_out_edges.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(out_edge_aggregate)

BOOST_AUTO_TEST_CASE(add_activation_adds_an_edge)
{
    TestAggregate aggregate;

    aggregate.add_activation(0, {}, 0);

    BOOST_CHECK_EQUAL(std::distance(aggregate.begin(), aggregate.end()), 1);
}

BOOST_AUTO_TEST_CASE(add_activation_doesnt_add_multiple_edges_to_same_vertex)
{
    TestAggregate aggregate;

    aggregate.add_activation(0, {}, 0);
    aggregate.add_activation(0, {}, 0);

    BOOST_CHECK_EQUAL(std::distance(aggregate.begin(), aggregate.end()), 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(generate_optimal_graph_tests)

BOOST_AUTO_TEST_CASE(returns_graph_with_no_edges_if_out_edges_empty)
{
    const auto no_edges = [](const auto& graph, const auto& vertex) {
        (void)graph;
        (void)vertex;
        return std::vector<TestAggregate::Edge> {};
    };

    const auto graph
        = generate_optimal_graph<int, std::vector<int>, decltype(no_edges)>(
            100, no_edges);

    BOOST_CHECK(graph.out_edges(0).empty());
}

BOOST_AUTO_TEST_SUITE_END()
