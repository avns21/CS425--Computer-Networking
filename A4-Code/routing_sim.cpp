#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <functional>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i) {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1) cout << "-";
        else cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();
    // dist[i][j] will hold current best-known cost from i to j
    vector<vector<int>> dist = graph;
    // nextHop[i][j] stores the next hop node on the path from i to j
    vector<vector<int>> nextHop(n, vector<int>(n, -1));

    // 1) Initialization: for each direct neighbor j of node i, set nextHop[i][j] = j
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (graph[i][j] != INF && graph[i][j] != 0) {
                nextHop[i][j] = j;
            }
        }
    }

    bool updated;
    int iter = 0;
    // 2) Iteratively exchange distance vectors until no changes occur
    do {
        updated = false;
        iter++;
        // Make copies of old tables to reference during this “round”
        auto oldDist    = dist;
        auto oldNextHop = nextHop;

        // 3) For each node i, consider each reachable neighbor j
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (oldDist[i][j] == INF) continue;  // j not reachable from i
                // 4) Try to improve paths to every destination k via neighbor j
                for (int k = 0; k < n; ++k) {
                    if (oldDist[j][k] == INF) continue;  // k not reachable from j
                    int viaJ = oldDist[i][j] + oldDist[j][k];
                    // 5) If going via j is cheaper, update cost and next hop
                    if (dist[i][k] > viaJ) {
                        dist[i][k]      = viaJ;
                        nextHop[i][k]   = oldNextHop[i][j];
                        updated         = true;
                    }
                }
            }
        }

        // 6) Print the routing table for each node after this iteration
        cout << "Iteration " << iter << ":\n";
        for (int i = 0; i < n; ++i) {
            printDVRTable(i, dist, nextHop);
        }
    } while (updated);

    // 7) Finally, print the stable (converged) tables
    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i) {
        printDVRTable(i, dist, nextHop);
    }
}

void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i) {
        if (i == src) continue;
        cout << i << "\t" << dist[i] << "\t";
        // Trace back from destination i to find the first hop after src
        int hop = i;
        while (prev[hop] != src && prev[hop] != -1) {
            hop = prev[hop];
        }
        if (prev[hop] == -1) cout << "-";
        else             cout << hop;
        cout << endl;
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();
    // For each node as the source, run Dijkstra’s algorithm
    for (int src = 0; src < n; ++src) {
        // dist[v]: current best-known cost from src to v
        vector<int>  dist(n, INF);
        // prev[v]: the node immediately before v on the shortest path from src
        vector<int>  prev(n, -1);
        vector<bool> visited(n, false);

        // 1) Distance to self is zero
        dist[src] = 0;

        // 2) Min‑heap of (distance, node) for selecting the next closest unvisited node
        using P = pair<int,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        pq.push({0, src});

        // 3) Main Dijkstra loop
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (visited[u]) continue;      // Skip if already processed
            visited[u] = true;

            // 4) Relax every outgoing edge (u→v)
            for (int v = 0; v < n; ++v) {
                if (graph[u][v] == INF || graph[u][v] == 0) continue;
                int cost = graph[u][v];
                if (dist[v] > dist[u] + cost) {
                    dist[v] = dist[u] + cost;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        // 5) Print the completed routing table for this source
        printLSRTable(src, dist, prev);
    }
}

vector<vector<int>> readGraphFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }
    int n; file >> n;
    vector<vector<int>> graph(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];
    file.close();
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }
    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}
