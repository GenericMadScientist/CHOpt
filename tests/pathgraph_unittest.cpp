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
using TestEdge = Edge<int, int>;

bool operator==(const TestEdge& lhs, const TestEdge& rhs)
{
    return std::tie(lhs.source, lhs.destination, lhs.options.weight,
                    lhs.options.activations)
        == std::tie(rhs.source, rhs.destination, rhs.options.weight,
                    rhs.options.activations);
}

std::ostream& operator<<(std::ostream& stream, const TestEdge& edge)
{
    stream << "{Source " << edge.source << ", Destination " << edge.destination
           << ", Weight " << edge.options.weight << ", Activations = {";
    for (auto it = edge.options.activations.cbegin();
         it < edge.options.activations.cend(); ++it) {
        if (it != edge.options.activations.cbegin()) {
            stream << ", ";
        }
        stream << *it;
    }
    stream << "}}";
    return stream;
}

BOOST_AUTO_TEST_SUITE(next_unprocessed_vertex)

BOOST_AUTO_TEST_CASE(returns_root_vertex_right_after_ctor)
{
    TestGraph graph {1};

    const auto next_vertex = graph.next_unprocessed_vertex();

    BOOST_CHECK(next_vertex.has_value());
    if (next_vertex.has_value()) {
        BOOST_CHECK_EQUAL(*next_vertex, 1);
    }
}

BOOST_AUTO_TEST_CASE(returns_nullopt_after_root_vertex)
{
    TestGraph graph {1};

    graph.next_unprocessed_vertex();
    const auto next_vertex = graph.next_unprocessed_vertex();
    BOOST_CHECK(!next_vertex.has_value());
}

BOOST_AUTO_TEST_CASE(returns_destination_of_added_edge_once_root_cleared)
{
    TestGraph graph {1};

    graph.next_unprocessed_vertex();
    graph.add_activation(1, 2, 0, {});
    const auto next_vertex = graph.next_unprocessed_vertex();

    BOOST_CHECK(next_vertex.has_value());
    if (next_vertex.has_value()) {
        BOOST_CHECK_EQUAL(*next_vertex, 2);
    }
}

BOOST_AUTO_TEST_CASE(does_not_add_same_destination_twice)
{
    TestGraph graph {1};

    graph.add_activation(1, 2, 0, {});
    graph.add_activation(1, 3, 0, {});
    for (auto i = 0; i < 3; ++i) {
        graph.next_unprocessed_vertex();
    }
    graph.add_activation(1, 3, 0, {});

    const auto next_vertex = graph.next_unprocessed_vertex();
    BOOST_CHECK(!next_vertex.has_value());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(optimal_path)

BOOST_AUTO_TEST_CASE(returns_empty_vector_if_no_edges_added)
{
    TestGraph graph {1};

    BOOST_CHECK(graph.optimal_path().empty());
}

BOOST_AUTO_TEST_CASE(returns_only_edge_if_one_edge_added)
{
    TestGraph graph {1};
    graph.add_activation(1, 2, 50, {0});

    const auto path = graph.optimal_path();
    const std::vector<TestEdge> expected_path {
        TestEdge {.source = 1,
                  .destination = 2,
                  .options = {.activations = {0}, .weight = 50}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(path.cbegin(), path.cend(),
                                  expected_path.cbegin(), expected_path.cend());
}

BOOST_AUTO_TEST_CASE(returns_highest_value_edge_if_all_edges_are_from_root)
{
    TestGraph graph {1};
    graph.add_activation(1, 2, 50, {0});
    graph.add_activation(1, 3, 100, {0});

    const auto path = graph.optimal_path();
    const std::vector<TestEdge> expected_path {
        TestEdge {.source = 1,
                  .destination = 3,
                  .options = {.activations = {0}, .weight = 100}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(path.cbegin(), path.cend(),
                                  expected_path.cbegin(), expected_path.cend());
}

BOOST_AUTO_TEST_CASE(only_returns_paths_from_root_vertex)
{
    TestGraph graph {1};
    graph.add_activation(2, 3, 50, {0});

    BOOST_CHECK(graph.optimal_path().empty());
}

BOOST_AUTO_TEST_CASE(returns_full_path_if_only_one_path_from_root)
{
    TestGraph graph {1};
    graph.add_activation(1, 2, 50, {0});
    graph.add_activation(2, 3, 100, {0});

    const auto path = graph.optimal_path();
    const std::vector<TestEdge> expected_path {
        TestEdge {.source = 1,
                  .destination = 2,
                  .options = {.activations = {0}, .weight = 50}},
        TestEdge {.source = 2,
                  .destination = 3,
                  .options = {.activations = {0}, .weight = 100}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(path.cbegin(), path.cend(),
                                  expected_path.cbegin(), expected_path.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(add_activation)

BOOST_AUTO_TEST_CASE(merges_activations_if_same_weight)
{
    TestGraph graph {1};

    graph.add_activation(1, 2, 50, {0});
    graph.add_activation(1, 2, 50, {1});

    const auto path = graph.optimal_path();
    const auto activations = path.at(0).options.activations;
    const std::vector<int> expected_activations {{0, 1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(activations.cbegin(), activations.cend(),
                                  expected_activations.cbegin(),
                                  expected_activations.cend());
}

BOOST_AUTO_TEST_CASE(replaces_activations_if_greater_weight)
{
    TestGraph graph {1};

    graph.add_activation(1, 2, 50, {0});
    graph.add_activation(1, 2, 100, {1});

    const auto path = graph.optimal_path();
    const auto activations = path.at(0).options.activations;
    const std::vector<int> expected_activations {1};

    BOOST_CHECK_EQUAL_COLLECTIONS(activations.cbegin(), activations.cend(),
                                  expected_activations.cbegin(),
                                  expected_activations.cend());
}

BOOST_AUTO_TEST_CASE(replaces_weight_if_greater_weight)
{
    TestGraph graph {1};

    graph.add_activation(1, 2, 50, {0});
    graph.add_activation(1, 2, 100, {1});

    const auto path = graph.optimal_path();

    BOOST_CHECK_EQUAL(path.at(0).options.weight, 100);
}

BOOST_AUTO_TEST_CASE(does_not_replace_activations_if_lower_weight)
{
    TestGraph graph {1};

    graph.add_activation(1, 2, 50, {0});
    graph.add_activation(1, 2, 25, {1});

    const auto path = graph.optimal_path();
    const auto activations = path.at(0).options.activations;
    const std::vector<int> expected_activations {0};

    BOOST_CHECK_EQUAL_COLLECTIONS(activations.cbegin(), activations.cend(),
                                  expected_activations.cbegin(),
                                  expected_activations.cend());
}

BOOST_AUTO_TEST_SUITE_END()
