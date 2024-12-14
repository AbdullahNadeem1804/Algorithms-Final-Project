#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <unordered_map>
#include <limits>

using namespace std;

class Graph {
private:
    int V; // Number of vertices
    vector<vector<pair<int, int>>> adjList; // Adjacency list: {destination, weight}
    unordered_map<int, int> influenceMap; // Map to store influence score for each node

public:
    // Constructor
    Graph(int vertices) : V(vertices), adjList(vertices) {}

    // Function to add an edge to the graph
    void addEdge(int u, int v, int weight) {
        adjList[u].push_back({v, weight});
        adjList[v].push_back({u, weight});
    }

    // Function to load influence scores from influence.txt
    void loadInfluenceScores(const string& influenceFile) {
        ifstream file(influenceFile);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << influenceFile << " for reading.\n";
            return;
        }
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            int node, influence;
            ss >> node >> influence;
            influenceMap[node] = influence;
        }
        file.close();
    }

    // Function to get influence score for a given node
    int getInfluenceScore(int node) const {
        auto it = influenceMap.find(node);
        if (it != influenceMap.end()) {
            return it->second;
        }
        return -1; // Return -1 if node doesn't exist in the map
    }

    // Function to find the longest increasing path based on influence scores using DP
    vector<int> findLongestInfluencePath() {
        vector<int> dp(V, 1); // DP array to store the length of the longest path ending at each node
        vector<int> parent(V, -1); // Array to store the parent node in the longest path

        // Sort nodes based on influence scores in ascending order
        vector<int> nodes(V);
        for (int i = 0; i < V; i++) {
            nodes[i] = i;
        }
        sort(nodes.begin(), nodes.end(), [&](int a, int b) {
            return influenceMap[a] < influenceMap[b];
        });

        // Iterate through each node in increasing order of influence score
        for (int node : nodes) {
            for (auto& [neighbor, weight] : adjList[node]) {
                if (influenceMap[node] < influenceMap[neighbor] && dp[node] + 1 > dp[neighbor]) {
                    dp[neighbor] = dp[node] + 1;
                    parent[neighbor] = node;
                }
            }
        }

        // Find the node with the maximum DP value
        int maxLengthNode = max_element(dp.begin(), dp.end()) - dp.begin();

        // Reconstruct the path
        vector<int> path;
        for (int node = maxLengthNode; node != -1; node = parent[node]) {
            path.push_back(node);
        }
        reverse(path.begin(), path.end());

        return path;
    }

    // Static method to create graph from input file
    static Graph createFromInputFile(const string& inputFile) {
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
            ss >> u >> v >> w;
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

        return graph;
    }
};

int main() {
    try {
        // File paths for graph and influence data
        string graphFile = "./social-network-proj-graph/social-network-proj-graph.txt";
        string influenceFile = "./social-network-proj-graph/social-network-proj-Influences.txt";
        string outputFile = "longest_chain.txt"; // Output file path

        // Create the graph from the input file
        Graph graph = Graph::createFromInputFile(graphFile);

        // Load influence scores from influence.txt
        graph.loadInfluenceScores(influenceFile);

        // Find the longest increasing path based on influence
        vector<int> longestPath = graph.findLongestInfluencePath();

        // Output the results to a file
        ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            cerr << "Error: Could not open output file " << outputFile << endl;
            return 1;
        }

        if (!longestPath.empty()) {
            // Write the longest chain length and sequence
            outFile << "Longest Chain Length: " << longestPath.size() << endl;
            outFile << "\nUser Sequence: ";
            for (int node : longestPath) {
                outFile << node << " ";
            }
            outFile << endl;

            // Include the influence score for each node in the sequence
            outFile << "\nInfluence Scores for Each Node in the Sequence:" << endl;
            for (int node : longestPath) {
                outFile << "Node " << node << ": " << graph.getInfluenceScore(node) << endl;
            }
        } else {
            outFile << "No path found." << endl;
        }

        outFile.close();
        cout << "Results written to " << outputFile << endl;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
