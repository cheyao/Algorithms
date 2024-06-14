#include <iostream>
#include <chrono>
#include <ostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

typedef struct Vector2 {
	int x, y;

	Vector2() : x(0), y(0) {};
	Vector2(uint64_t _x, uint64_t _y) : x(_x), y(_y) {};
	bool operator==(const Vector2& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator<(const Vector2& rhs) const {
		if (y != rhs.y) {
			return y < rhs.y;
		}

		return x < rhs.x;
	}
	Vector2 operator+(const Vector2& rhs) const {
		return Vector2(x + rhs.x, y + rhs.y);
	}
	Vector2 operator-() const {
		return Vector2(-x, -y);
	}
	Vector2& operator+=(const Vector2& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
} Vector2;

#define ZERO  Vector2( 0, 0)
#define UP    Vector2( 0,-1)
#define DOWN  Vector2( 0, 1)
#define LEFT  Vector2(-1, 0)
#define RIGHT Vector2( 1, 0)

vector<string> maze = {
     "#####################",
     "#S    #       #     #",
     "##### ### ##### # ###",
     "#   #   # #   # #   #",
     "# ##### # # # # ### #",
     "#     # #   #   #   #",
     "# ### # ######### ###",
     "#   #   #       #   #",
     "### ##### ##### # # #",
     "# # #     #   # #E# #",
    "# # # ### # # # ### #",
    "#   #   # # # #     #",
    "# ### ### # # ##### #",
    "# # # #   # #     # #",
    "# # # # ### ### ### #",
    "# #   #   #   #   # #",
    "# ##### # ### ### # #",
    "#     # # # #   #   #",
    "##### ### # ### #####",
    "#         #         #",
    "#####################"
};

typedef struct Node {
	Vector2 position;
	vector<Node*> neighbors;

	bool inside(vector<string>& maze, Vector2 offset = ZERO) {
		return (
				position.x + offset.x >= 0 &&
				position.x + offset.x < maze[0].size() &&
				position.y + offset.y >= 0 &&
				position.y + offset.y < maze.size()
		       );
	}

	bool operator==(const Node& other) const {
		return position == other.position;
	}
	bool operator!=(const Node& other) const {
		return !(*this == other);
	}

	friend std::ostream& operator<<(ostream& os, Node& node) {
		os << "Node: (" << node.position.x << ", " << node.position.y << ")";
		return os;
	}
} Node;

namespace std {
template <> struct hash<Node> {
	std::size_t operator()(const Node& id) const noexcept {
		return std::hash<int>()(id.position.x ^ (id.position.y << 16));
	}
};
}

template<typename T> 
static auto& get(vector<T>& vec, Vector2 pos) {
	return vec[pos.y][pos.x];
}

unordered_map<Node, Node> BFD(vector<Node>& nodes, Node& start, Node& end) {
	queue<Node> frontier;
	frontier.emplace(start);
	unordered_map<Node, Node> outPath;
	outPath[start] = start;

	while (!frontier.empty()) {
		auto node = frontier.front();
		frontier.pop();

		if (node == end) {
			break;
		}

		for (Node* neighbor : node.neighbors) {
			if (!outPath.contains(*neighbor)) {
				outPath[*neighbor] = node;
				frontier.emplace(*neighbor);
			}
		}
	}

	return outPath;
}

Node& findNode(vector<Node>& nodes, const Vector2& position) {
	for (Node& node : nodes) {
		if (node.position == position) {
			return node;
		}
	}

	return nodes[0];
}

void populate(Node& node, vector<Node>& nodes) {
	for (const Vector2& vec : {UP, DOWN, LEFT, RIGHT}) {
		if (!node.inside(maze, vec)) {
			continue;
		}

		if (get(maze, node.position + vec) == '#') {
			continue;
		}

		if (node.position == Vector2(3, 15)) {
			cout << "Hi " << vec.x << ", " << vec.y << endl;
		}
		node.neighbors.push_back(&findNode(nodes, node.position + vec));
	}
}

int main() {
	vector<Node> nodes;
	Node start;
	Node end;
	for (int y = 0; y < maze.size(); ++y) {
		for (int x = 0; x < maze[0].size(); ++x) {
			Node node;
			node.position = Vector2(x, y);
			node.neighbors.clear();

			nodes.push_back(node);

			 if (maze[y][x] == 'S') {
				 start = node;
			 }
			 if (maze[y][x] == 'E') {
				 end = node;
			 }
		}
	}

	for (auto& node : nodes) {
		populate(node, nodes);
	}
	populate(start, nodes);
	populate(end, nodes);

	auto timeStart = std::chrono::high_resolution_clock::now();
	auto outPath = BFD(nodes, start, end);
	auto timeEnd = std::chrono::high_resolution_clock::now();

	// Print out the results
	cout << "Output:" << endl;
	Node cur = end;
	while (cur != start) {
		// cout << cur << endl;
		Node next = outPath[cur];
		get(maze, cur.position) = '0';
		cur = next;
	}

	for (auto& line : maze) {
		cout << line << endl;
	}
	cout << "BFD took " << timeEnd-timeStart << "ns" << endl;
}

