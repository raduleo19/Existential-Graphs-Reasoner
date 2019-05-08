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

        ++id;
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
            std::vector<int>(1 + where.begin(), where.end());

        new_graph.subgraphs[where[0]] =
            new_graph.subgraphs[where[0]].double_cut(new_where);
    }

    return new_graph;
}


std::vector<std::vector<int>> AEGraph::possible_erasures(int level) const {
    std::vector<std::vector<int>> total_result;
    std::vector<int> level_stack, last_forked;
    std::vector<std::pair <AEGraph, std::vector<int> > > dfs_stack;
    
    last_forked.push_back(-1);
    dfs_stack.push_back({*this, {}});
    level_stack.push_back(0);
    
    while (!dfs_stack.empty()) {
        std::pair<AEGraph, std::vector<int> > current = dfs_stack.back();
        int current_level = level_stack.back(),
            last_fork = last_forked.back();
        last_forked.pop_back();
        dfs_stack.pop_back();
        level_stack.pop_back();

        int id = 0;
        std::vector<int> maneuver = current.second;
        
        if (current.first.size() >= 2)
            last_fork = 0;
        else
            ++last_fork;
/*
        std::cerr << "STACK: ";
        for (auto i : current.second)
            std::cerr << i;
        std::cerr << " - " << current_level;
        std::cerr << std::endl << std::endl;
*/
        //std::cerr << current.first.size() << " vs " << current.first.subgraphs.size() << std::endl;

        for (auto i : current.first.subgraphs) {
            maneuver.push_back(id);
            dfs_stack.push_back({i, maneuver});
            level_stack.push_back(current_level + 1);
            last_forked.push_back(last_fork);
            if ((current_level % 2 == 0) && last_fork < 1) {
                total_result.push_back(maneuver);
            }
            maneuver.pop_back();
            ++id;
        }

        for (auto i : current.first.atoms) {
            //std::cerr << "last forked: " << last_fork << std::endl;
            maneuver.push_back(id);
            if ((current_level % 2 == 0) && last_fork < 1) {
                total_result.push_back(maneuver);
            }
            maneuver.pop_back();
            ++id;
        }
    }
/*
    if ((level & 1) && num_subgraphs() >= 2 || level == -1) {
        for (int i = 0; i < this->size(); ++i) {
            std::vector<int> current_result = {i};

            total_result.push_back(current_result);
        }
    }

    int id = 0;

    for (auto son : subgraphs) {
        std::vector<std::vector<int>> sons_result =
            son.possible_erasures(1 + level);

        for (auto son_result : sons_result) {
            son_result.insert(son_result.begin(), id);

            total_result.push_back(son_result);
        }

        ++id;
    }
*/
    return total_result;
}

AEGraph AEGraph::erase(std::vector<int> where) const {
    AEGraph new_graph = *this;

    if (where.size() == 1) {
        if (where[0] < new_graph.num_subgraphs()) {
            new_graph.subgraphs.erase(new_graph.subgraphs.begin() + where[0]);
        } else {
            new_graph.atoms.erase(new_graph.atoms.begin()
                                  + where[0] - new_graph.num_subgraphs());
        }
    } else if (where.size() > 1) {
        std::vector<int> new_where =
            std::vector<int>(1 + where.begin(), where.end());
        new_graph.subgraphs[where[0]] =
            new_graph.subgraphs[where[0]].erase(new_where);
    }

    return new_graph;
}


std::vector<std::vector<int>> AEGraph::possible_deiterations() const {
    std::vector<std::vector<int>> total_result;

    std::vector<std::string> previous_sons;

    int id = 0;

    for (auto son : subgraphs) {
        for (auto cut_sons : previous_sons) {
            std::vector<std::vector<int>> sons_result =
                                          son.get_paths_to(cut_sons);
            for (auto i : sons_result)
                i.insert(i.begin(), id),
                total_result.push_back(i);
        }

        previous_sons.push_back(son.repr());

        std::vector<std::vector<int>> sons_results =
                                      son.possible_deiterations();
        for (auto i : sons_results)
            i.insert(i.begin(), id),
            total_result.push_back(i);

        ++id;
    }

    return total_result;
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

