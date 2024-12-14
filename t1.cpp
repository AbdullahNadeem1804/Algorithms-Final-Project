#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <iomanip>
#include <set>

using namespace std;

class Graph {
private:
    int V; // Number of vertices
    vector<vector<pair<int, int>>> adjList; // Adjacency list: {destination, weight}

    // Heuristic function: Number of direct connections for a node
    int heuristic(int node) {
        return adjList[node].size();
    }

    // Function to add an edge to the graph
    void addEdge(int u, int v, int weight) {
        adjList[u].push_back({v, weight}); // Add edge u -> v
        adjList[v].push_back({u, weight}); // Add edge v -> u (undirected)
    }

    // Function to write the graph to a file
    void writeGraphToFile(const string &outputFile) {
        ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            cerr << "Error: Could not open file " << outputFile << " for writing.\n";
            return;
        }
        for (int i = 0; i < V; i++) {
            if (!adjList[i].empty()) {
                outFile << "Node " << i << " connects to:\n";
                for (const auto &edge : adjList[i]) {
                    outFile << "  Node " << edge.first << " with weight " << edge.second << "\n";
                }
                outFile << endl;
            }
        }
        outFile.close();
        cout << "Graph successfully written to " << outputFile << endl;
    }

    // Helper function to find weight between two connected nodes
    int findEdgeWeight(int from, int to) const {
        for (const auto& edge : adjList[from]) {
            if (edge.first == to) {
                return edge.second;
            }
        }
        return -1; // Not connected
    }

public:
    // Constructor
    Graph(int vertices) : V(vertices), adjList(vertices) {}

    // A* algorithm to find the shortest path
    vector<pair<int, int>> aStarShortestPath(int start, int goal) {
        // Priority queue for open set, sorted by f(n) = g(n) + h(n)
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> openSet;
        
        // Distance from start node
        vector<int> gScore(V, numeric_limits<int>::max());
        gScore[start] = 0;

        // Estimated total cost from start to goal through this node
        vector<int> fScore(V, numeric_limits<int>::max());
        fScore[start] = heuristic(start);

        // Track the path
        vector<int> cameFrom(V, -1);

        // Add start node to open set
        openSet.push({fScore[start], start});

        while (!openSet.empty()) {
            int current = openSet.top().second;
            openSet.pop();

            // Found the goal
            if (current == goal) {
                // Reconstruct path with weights
                vector<int> path;
                vector<pair<int, int>> weightedPath;
                while (current != -1) {
                    path.push_back(current);
                    current = cameFrom[current];
                }
                reverse(path.begin(), path.end());

                // Add weights to the path
                for (size_t i = 0; i < path.size() - 1; ++i) {
                    int weight = findEdgeWeight(path[i], path[i+1]);
                    weightedPath.push_back({path[i], weight});
                }
                // Add the last node
                weightedPath.push_back({path.back(), 0});

                return weightedPath;
            }

            // Explore neighbors
            for (auto& [neighbor, edgeWeight] : adjList[current]) {
                // Tentative g score
                int tentativeGScore = gScore[current] + edgeWeight;

                // If this path is better than any previous one
                if (tentativeGScore < gScore[neighbor]) {
                    // Update path
                    cameFrom[neighbor] = current;
                    gScore[neighbor] = tentativeGScore;
                    
                    // f(n) = g(n) + h(n)
                    fScore[neighbor] = gScore[neighbor] + heuristic(neighbor);
                    
                    // Add to open set
                    openSet.push({fScore[neighbor], neighbor});
                }
            }
        }

        // No path found
        return {};
    }

    // Write detailed path information to file
    void writeDetailedPathToFile(const vector<pair<int, int>>& path, const string& outputFile) {
        ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            cerr << "Error: Could not open file " << outputFile << " for writing.\n";
            return;
        }

        outFile << "Detailed Shortest Path Information:\n";

        int totalWeight = 0;
        outFile << left << setw(10) << "Node" << setw(15) << "Edge Weight" << "\n";
        
        for (const auto& [node, weight] : path) {
            outFile << left << setw(10) << node << setw(15) << weight << "\n";
            
            if (weight > 0) {
                totalWeight += weight;
            }
        }

        outFile << "\nTotal Path Weight: " << totalWeight << endl;
        outFile.close();
        cout << "Path written to " << outputFile << endl;
    }

    // Write nodes information to file (only for nodes in the path)
    void writeNodesInfoToFile(const vector<pair<int, int>>& path, const string& outputFile) {
        ofstream outFile(outputFile, ios::app);
        if (!outFile.is_open()) {
            cerr << "Error: Could not open file " << outputFile << " for writing.\n";
            return;
        }

        // Create a set of nodes in the path for efficient lookup
        set<int> pathNodes;
        for (const auto& [node, _] : path) {
            pathNodes.insert(node);
        }

        outFile << "\n\nDetailed Nodes Information (for nodes in the shortest path):" << endl;
        
        for (int node : pathNodes) {
            outFile << "\nNode " << node << " connections:" << endl;
            
            if (adjList[node].empty()) {
                outFile << "  No connections" << endl;
            } else {
                for (const auto& [dest, weight] : adjList[node]) {
                    outFile << "  -> Node " << dest << " (Weight: " << weight << ")" << endl;
                }
            }
        }

        outFile.close();
    }

    // Static method to create graph from input file
    static Graph createFromInputFile(const string& inputFile, const string& graphOutputFile) {
        ifstream file(inputFile);
        if (!file.is_open()) {
            throw runtime_error("Error opening input file: " + inputFile);
        }

        int maxNode = 0;
        vector<tuple<int, int, int>> edges; // Temporarily store edges {u, v, weight}

        // Read the file line by line
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            int u, v, w;
            ss >> u >> v >> w; // Read node1, node2, weight
            edges.push_back({u, v, w});
            maxNode = max(maxNode, max(u, v)); // Update max node value
        }
        file.close();

        // Create graph with correct number of vertices
        Graph graph(maxNode + 1);

        // Populate the adjacency list
        for (const auto &[u, v, w] : edges) {
            graph.addEdge(u, v, w);
        }

        // Write graph to output file
        graph.writeGraphToFile(graphOutputFile);

        return graph;
    }

    // Get the number of vertices
    int getVertexCount() const {
        return V;
    }

    // Visualize only the shortest path
    void visualizeShortestPath(const vector<pair<int, int>>& path, const string& outputDotFile) {
        ofstream outFile(outputDotFile);
        if (!outFile.is_open()) {
            cerr << "Error: Could not open file " << outputDotFile << " for writing.\n";
            return;
        }

        outFile << "graph G {" << endl; // Graphviz syntax for undirected graph

        // Create edges for the shortest path only
        for (size_t i = 0; i < path.size() - 1; i++) {
            int u = path[i].first;
            int v = path[i + 1].first;
            int weight = path[i].second; // assuming the weight is stored for each edge in the path
            outFile << "  " << u << " -- " << v << " [label=\"" << weight
                    << "\", color=\"red\", penwidth=2.0];" << endl;
        }

        outFile << "}" << endl;
        outFile.close();

        cout << "Shortest path visualization written to " << outputDotFile << endl;
    }

};

int main() {
    try {
        // Input and output file names
        string inputFile = "./social-network-proj-graph/social-network-proj-graph.txt";
        string graphOutputFile = "graph_output.txt";
        string pathOutputFile = "a_star_shortest_path.txt";

        // Create graph from input file and write graph details
        Graph graph = Graph::createFromInputFile(inputFile, graphOutputFile);

        // Using first node (0) as start and last node as goal
        int startNode = 0;
        int goalNode = graph.getVertexCount() - 1; //Just change these for the start and end nodes you want to find the shortest path for

        cout << "Finding shortest path from node " << startNode << " to node " << goalNode << endl;

        // Find shortest path
        vector<pair<int, int>> shortestPath = graph.aStarShortestPath(startNode, goalNode);

        // Write path to output file
        if (!shortestPath.empty()) {
            graph.writeDetailedPathToFile(shortestPath, pathOutputFile);
            graph.writeNodesInfoToFile(shortestPath, pathOutputFile);
        } else {
            cout << "No path found between nodes " << startNode << " and " << goalNode << endl;
        }
        // Visualize shortest path
        if (!shortestPath.empty()) {
            string pathDotFile = "shortest_path_visualization.dot";
            graph.visualizeShortestPath(shortestPath, pathDotFile);
        }

    }

    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}