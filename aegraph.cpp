// Copyright 2019 Luca Istrate, Danut Matei
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <utility>
#include <cassert>
#include "./aegraph.h"

std::string strip(std::string s) {
    // deletes whitespace from the beginning and end of the string
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    s.erase(s.find_last_not_of(" \n\r\t")+1);
    return s;
}

std::pair<std::string, std::string> split_first(std::string s,
    char delimiter = ',') {
    // returns a pair that contains: <first_cut, rest_of_graph>

    int numOpen = 0;

    int len = s.size();
    for (int i = 0; i < len; i++) {
        char c = s[i];
        if (c == delimiter && numOpen == 0)
            return std::make_pair(
                    strip(s.substr(0, i)), strip(s.substr(i+1, len)));
        if (c == '[')
            numOpen += 1;
        if (c == ']')
            numOpen -= 1;
    }

    return {strip(s), std::string()};
}


std::vector<std::string> split_level(std::string s, char delimiter = ',') {
    // splits 's' into separate entities (atoms, subgraphs)

    std::vector<std::string> result;
    auto snd = s;
    while (true) {
        auto p = split_first(snd, delimiter);
        auto fst = p.first;
        snd = p.second;

        result.push_back(fst);

        if (snd.empty())
            return result;
    }
}


int AEGraph::num_subgraphs() const {
    return subgraphs.size();
}


int AEGraph::num_atoms() const {
    return atoms.size();
}


int AEGraph::size() const {
    return num_atoms() + num_subgraphs();
}


bool AEGraph::operator<(const AEGraph& other) const {
    return this->repr() < other.repr();
}

bool AEGraph::operator==(const AEGraph& other) const {
    return this->repr() == other.repr();
}

bool AEGraph::operator!=(const AEGraph& other) const {
    return this->repr() != other.repr();
}

AEGraph AEGraph::operator[](const int index) const {
    // offers an easier way of accessing the nested graphs
    if (index < num_subgraphs()) {
        return subgraphs[index];
    }

    if (index < num_subgraphs() + num_atoms()) {
        return AEGraph('(' + atoms[index - num_subgraphs()] + ')');
    }

    return AEGraph("()");
}

std::ostream& operator<<(std::ostream &out, const AEGraph &g) {
    out << g.repr();
    return out;
}

AEGraph::AEGraph(std::string representation) {
    // constructor that creates an AEGraph structure from a
    // serialized representation
    char left_sep = representation[0];
    char right_sep = representation[representation.size() - 1];

    assert((left_sep == '(' && right_sep == ')')
        || (left_sep == '[' && right_sep == ']'));

    // if the left separator is '(' then the AEGraph is the entire
    // sheet of assertion
    if (left_sep == '(') {
        is_SA = true;
    } else {
        is_SA = false;
    }

    // eliminate the first pair of [] or ()
    representation = representation.substr(1, representation.size() - 2);

    // split the graph into separate elements
    auto v = split_level(representation);
    // add them to the corresponding vector
    for (auto s : v) {
        if (s[0] != '[') {
            atoms.push_back(s);
        } else {
            subgraphs.push_back(AEGraph(s));
        }
    }

    // also internally sort the new graph
    this->sort();
}

std::string AEGraph::repr() const {
    // returns the serialized representation of the AEGraph

    std::string left, right;
    if (is_SA) {
        left = '(';
        right = ')';
    } else {
        left = '[';
        right = ']';
    }

    std::string result;
    for (auto subgraph : subgraphs) {
        result += subgraph.repr() + ", ";
    }

    int len = atoms.size();
    if (len != 0) {
        for (int i = 0; i < len - 1; i++) {
            result += atoms[i] + ", ";
        }
        result += atoms[len - 1];
    } else {
        if (subgraphs.size() != 0)
            return left + result.substr(0, result.size() - 2) + right;
    }

    return left + result + right;
}


void AEGraph::sort() {
    std::sort(atoms.begin(), atoms.end());

    for (auto& sg : subgraphs) {
        sg.sort();
    }

    std::sort(subgraphs.begin(), subgraphs.end());
}

bool AEGraph::contains(const std::string other) const {
    // checks if an atom is in a graph
    if (find(atoms.begin(), atoms.end(), other) != atoms.end())
        return true;

    for (const auto& sg : subgraphs)
        if (sg.contains(other))
            return true;

    return false;
}

bool AEGraph::contains(const AEGraph& other) const {
    // checks if a subgraph is in a graph
    if (find(subgraphs.begin(), subgraphs.end(), other) != subgraphs.end())
        return true;

    for (const auto& sg : subgraphs)
        if (sg.contains(other))
            return true;

    return false;
}

std::vector<std::vector<int>> AEGraph::get_paths_to(const std::string other)
    const {
    // returns all paths in the tree that lead to an atom like <other>
    std::vector<std::vector<int>> paths;

    int len_atoms = num_atoms();
    int len_subgraphs = num_subgraphs();

    for (int i = 0; i < len_atoms; i++) {
        if (atoms[i] == other && size() > 1) {
            paths.push_back({i + len_subgraphs});
        }
    }

    for (int i = 0; i < len_subgraphs; i++) {
        if (subgraphs[i].contains(other)) {
            auto r = subgraphs[i].get_paths_to(other);
            for (auto& v : r)
                v.insert(v.begin(), i);
            copy(r.begin(), r.end(), back_inserter(paths));
        }
    }

    return paths;
}

std::vector<std::vector<int>> AEGraph::get_paths_to(const AEGraph& other)
    const {
    // returns all paths in the tree that lead to a subgraph like <other>
    std::vector<std::vector<int>> paths;
    int len_subgraphs = num_subgraphs();

    for (int i = 0; i < len_subgraphs; i++) {
        if (subgraphs[i] == other && size() > 1) {
            paths.push_back({i});
        } else {
            auto r = subgraphs[i].get_paths_to(other);
            for (auto& v : r)
                v.insert(v.begin(), i);
            copy(r.begin(), r.end(), back_inserter(paths));
        }
    }

    return paths;
}

std::vector<std::vector<int>> AEGraph::possible_double_cuts() const {
    std::vector<std::vector<int>> total_result;
    int id = 0;
    for (auto son : subgraphs) {
        if (son.num_atoms() == 0 && son.num_subgraphs() == 1) {
            std::vector<int> current_result = {id};
            total_result.push_back(current_result);
        }
        std::vector<std::vector<int>> sons_result = son.possible_double_cuts();
        for (auto son_result : sons_result) {
            son_result.insert(son_result.begin(), id);
            total_result.push_back(son_result);
        }
        id++;
    }
    return total_result;
}

AEGraph AEGraph::double_cut(std::vector<int> where) const {
    AEGraph new_graph = *this;
    if (where.size() == 1) {
        new_graph.atoms.insert(
            new_graph.atoms.begin(),
            new_graph.subgraphs[where[0]].subgraphs[0].atoms.begin(),
            new_graph.subgraphs[where[0]].subgraphs[0].atoms.end());
        new_graph.subgraphs.insert(
            new_graph.subgraphs.begin(),
            new_graph.subgraphs[where[0]].subgraphs[0].subgraphs.begin(),
            new_graph.subgraphs[where[0]].subgraphs[0].subgraphs.end());
        new_graph.subgraphs.erase(new_graph.subgraphs.begin() + where[0]);
    } else if (where.size() > 1) {
        std::vector<int> new_where =
            std::vector<int>(where.begin() + 1, where.end());
        new_graph.subgraphs[where[0]] =
            new_graph.subgraphs[where[0]].double_cut(new_where);
    }
    return new_graph;
}


std::vector<std::vector<int>> AEGraph::possible_erasures(int level) const {
    // TO DO: Avoid the case ([[A]]) -> ([[]])
    std::vector<std::vector<int>> total_result;
    for (int i = 0; i < this->size(); ++i) {
        if (level % 2 == 1) {
            std::vector<int> current_result = {i};
            total_result.push_back(current_result);
        }
    }
    int id = 0;
    for (auto son : subgraphs) {
        std::vector<std::vector<int>> sons_result =
            son.possible_erasures(level + 1);
        for (auto son_result : sons_result) {
            son_result.insert(son_result.begin(), id);
            total_result.push_back(son_result);
        }
        id++;
    }
    return total_result;
}


AEGraph AEGraph::erase(std::vector<int> where) const {
    AEGraph new_graph = *this;
    if (where.size() == 1) {
        if (where[0] < new_graph.num_subgraphs()) {
            new_graph.subgraphs.erase(new_graph.subgraphs.begin() + where[0]);
        } else {
            new_graph.atoms.erase(new_graph.atoms.begin() + where[0] - new_graph.num_subgraphs());
        }
    } else if (where.size() > 1) {
        std::vector<int> new_where =
            std::vector<int>(where.begin() + 1, where.end());
        new_graph.subgraphs[where[0]] =
            new_graph.subgraphs[where[0]].erase(new_where);
    }
    return new_graph;
}


std::vector<std::vector<int>> AEGraph::possible_deiterations() const {
    // 20p
    return {};
}

AEGraph AEGraph::deiterate(std::vector<int> where) const {
    AEGraph new_graph = *this;
    if (where.size() == 1) {
        // TO DO : Completeaza aici deiterate pentru graful current,
        // recursivitatea ar trebui sa functioneze asa (asa am facut la
        // celelalte)
    } else if (where.size() > 1) {
        std::vector<int> new_where =
            std::vector<int>(where.begin() + 1, where.end());
        new_graph.subgraphs[where[0]] =
            new_graph.subgraphs[where[0]].erase(new_where);
    }
    return new_graph; 
}

