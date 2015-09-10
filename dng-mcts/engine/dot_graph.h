//Copyright 2007 baj
//baj@mail.ustc.edu.cn

#ifndef _DOT_GRAPH_H_
#define _DOT_GRAPH_H_

#include <list>
#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <cstring>
#include <cstdio>
#include <sstream>

namespace dot {

struct Label{
	std::string label;

	Label() {

	}

	Label(const std::string& str) {
		label = str;
	}

	friend std::ostream& operator <<(std::ostream &os, const Label &l)
	{
		if (l.label.length()) {
			return os << "[label=\"" << l.label << "\"]";
		}
		else {
			return os;
		}
	}
};

struct Color{
	std::string color;

	Color() {

	}

	Color(const std::string& str){
		color = str;
	}

	friend std::ostream& operator <<(std::ostream &os, const Color &c)
	{
		if (c.color.length()) {
			return os << "[color=\"" << c.color << "\"]";
		}
		else {
			return os;
		}
	}
};

struct Edge {
	Color color;
	Label label;
};

class Node {
public:
	int id;
	Label label;
	Color color;

	std::map<int, Edge> neighbours;

	Node(): id(-1) {

	}

	Node(int i, Label l) {
		id = i; label = l;
	}

	void setColor(const std::string& str) {
		color = Color(str);
	}

	void addNeighbor(const int n, const std::string& edge_color, const std::string& edge_label)
	{
		Edge& edge = neighbours[n];

		edge.color = Color(edge_color);
		edge.label = Label(edge_label);
	}

	friend std::ostream& operator <<(std::ostream & os, const Node &n)
	{
		os << n.id << " " << n.label ;
		if (n.color.color.length()) {
			os << n.color;
		}
		return os;
	}
};

class Graph{
	std::vector<Node> nodes;
	std::map<std::string, int>  id_map;

public:
	Graph()	{
	}

	void addNode(const std::string& name, const std::string& color = "")	{
		if (id_map.count(name)) {
			nodes[id_map[name]].setColor(color);
		}
		else {
			int id = nodes.size();

			Node node = Node(id, Label(name));
			node.setColor(color);

			nodes.push_back(node);
			id_map[name] = id;
		}
	}

	void addEdge(const std::string& a, const std::string& b, const std::string& color = "", const std::string& label = "")	{
		if (!id_map.count(a)) {
			addNode(a);
		}
		if (!id_map.count(b)) {
			addNode(b);
		}

		int id_a = id_map[a];
		int id_b = id_map[b];

		nodes[id_a].addNeighbor(id_b, color, label);
	}

	friend std::ostream& operator <<(std::ostream &os, const Graph &g) {
		os << "Digraph \"\" {\nratio=fill\n";
		os << "node [style=filled];" << std::endl;

		for (unsigned int i = 0; i < g.nodes.size(); ++i) {
			os << "\t";
			os << g.nodes[i];
			os << ";" << std::endl;
		}

		for (unsigned int i = 0; i < g.nodes.size(); ++i) {
			for (std::map<int, Edge>::const_iterator it = g.nodes[i].neighbours.begin(); it != g.nodes[i].neighbours.end(); ++it) {
				os << "\t" << i << " -> " << it->first << " ";
				os << it->second.color;
				os << it->second.label;
				os << ";" << std::endl;
			}
		}

		os << "}" << std::endl;

		return os;
	}
};

} //namspace dot_graph

#endif
