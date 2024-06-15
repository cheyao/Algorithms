#include <cstdint>
#include <iostream>
#include <chrono>
#include <ostream>
#include <queue>
#include <string>
#include <sys/types.h>
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
	Vector2 operator-(const Vector2& rhs) const {
		return Vector2(x - rhs.x, y - rhs.y);
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
     "#     #       #     #",
     "##### ### ##### # ###",
     "#   #   # #   # #   #",
     "####### # # # # ### #",
     "#                   #",
     "#        ### ###  ###",
     "#          #   #    #",
     "##         #   #    #",
     "#          #   #    #",
    "#          #   #    #",
    "#          #   #    #",
    "#          #E  #    #",
    "#          #   #    #",
    "# #        #   #### #",
    "# #  S #########  # #",
    "# ##### # ### ### # #",
    "#     # # # #   #   #",
    "##### ### # ### #####",
    "#         #         #",
    "#####################"
};

typedef struct Node {
	Vector2 position;
	vector<const Node*> neighbors;

	Node() : position(ZERO), neighbors() {};
	Node(Vector2 pos) : position(pos), neighbors() {};

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

typedef struct PriorityQueue {
	typedef std::pair<uint64_t, Node> Element;
	std::priority_queue<Element, std::vector<Element>, decltype([] (const Element& a, const Element& b) { return a.first > b.first; })> elements;

	inline bool empty() const {
		return elements.empty();
	}

	inline void put(Node item, uint64_t priority) {
		elements.emplace(priority, item);
	}

	Node get() {
		Node best_item = elements.top().second;
		elements.pop();
		return best_item;
	}
} PriorityQueue;

namespace std {
template <> struct hash<Vector2> {
	std::size_t operator()(const Vector2& id) const noexcept {
		return std::hash<int>()(id.x ^ (id.y << 16));
	}
};
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

static uint64_t heuristic(const Node& a, const Node& b) {
	return abs(a.position.x - b.position.x) + abs(a.position.y - b.position.y);
}

static uint64_t tileCost(const Vector2 pos) {
	switch (get(maze, pos)) {
		case ' ':
			return 1;
		case '.':
			return 2;
		case 'E':
			return 0;
		default:
			cout << "Uncached" << get(maze, pos) << endl;
			return 1;
	}
}

unordered_map<Node, Node> A(vector<Node>& nodes, Node& start, Node& end) {
	PriorityQueue frontier;
	frontier.put(start, 1);
	unordered_map<Node, Node> outPath;
	outPath[start] = start;
	unordered_map<Node, uint64_t> costs;
	costs[start] = 1;

	while (!frontier.empty()) {
		auto node = frontier.get();

		if (node == end) {
			break;
		}

		for (const Node* neighbor : node.neighbors) {
			const uint64_t newCost = costs[node] + tileCost(neighbor->position);
			if (!outPath.contains(*neighbor) || newCost < costs[*neighbor]) {
				costs[*neighbor] = newCost;
				outPath[*neighbor] = node;
				frontier.put(*neighbor, newCost + heuristic(end, *neighbor));
			}
		}
	}

	return outPath;
}

const Node& findNode(const vector<Node>& nodes, const Vector2& position) {
	for (const Node& node : nodes) {
		if (node.position == position) {
			return node;
		}
	}

	return nodes[0];
}

void populate(Node& node, const vector<Node>& nodes) {
	for (const Vector2& vec : {UP, DOWN, LEFT, RIGHT}) {
		if (!node.inside(maze, vec)) {
			continue;
		}

		if (get(maze, node.position + vec) == '#') {
			continue;
		}

		node.neighbors.push_back(&findNode(nodes, node.position + vec));
	}
}

const char direction(const Node& node, const Node& direction) {
	 static unordered_map<Vector2, char> dirMap = {
		{UP, '^'}, 
		{DOWN, 'v'},
		{LEFT, '<'},
		{RIGHT, '>'}
	};
	return dirMap[direction.position - node.position];
}

const string beutify(const char& c) {
	 static unordered_map<char, string> charMap = {
		 {'#', "\u25A0"},
		 {'>', "→"},
		 {'<', "←"},
		 {'v', "↓"},
		 {'^', "↑"},
		 {'S', "\x1B[33m⛌\033[0m"},
		 {'D', "\x1B[33m★\033[0m"},
		 {' ', " "},
		 {'.', " "}
	};
	return charMap[c];
}

int main() {
	vector<Node> nodes;
	Node start;
	Node end;
	for (int y = 0; y < maze.size(); ++y) {
		for (int x = 0; x < maze[0].size(); ++x) {
			if (maze[y][x] == '#') {
				continue;
			}

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
	auto outPath = A(nodes, start, end);
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

	cout << "Flow:" << endl;
	for (auto [node, neighbor] : outPath) {
		if (node.position == start.position) {
			maze[node.position.y][node.position.x] = 'S';
		} else if (node.position == end.position) {
			maze[node.position.y][node.position.x] = 'D';
		} else {
			maze[node.position.y][node.position.x] = direction(node, neighbor);
		}
	}
	for (const auto& line : maze) {
		for (const auto& c : line) {
			if (c == '#') {
				cout << "\x1B[31m";
			} else {
				cout << "\x1B[32m";
			}

			cout << beutify(c) << "\033[0m";
		}
		cout << endl;
	}

	cout << "A took " << timeEnd-timeStart << "ns" << endl;
}

