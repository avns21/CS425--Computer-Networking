# Routing Simulation (DVR & LSR)

## Objective  
Implement and simulate two fundamental routing protocols—Distance Vector Routing (DVR) and Link State Routing (LSR)—on a network represented by an adjacency matrix. The goal is to compute and display routing tables (cost and next‑hop) for every node.

---

## Setup Instructions  
  
1. **Build the executable**:  
   ```bash
   make
   ```

2. **Run the simulation** on any input file:  
   ```bash
   ./routing_sim input1.txt
   ```

3. **Clean build artifacts**:  
   ```bash
   make clean
   ```

---

## Input File Format  
- First line: integer **n** = number of nodes.  
- Next **n** lines: **n** space‑separated integers each, representing the adjacency matrix.  
  - A nonzero finite value = link cost between nodes _i_ and _j_.  
  - **0** on the diagonal (node to itself).  
  - **0** off‑diagonal means no link.  
  - **9999** represents an unreachable link (∞ cost).  

### Example (`input1.txt`)  
```
4
0   10  100  30
10   0   20  40
100 20    0  10
30  40   10   0
```

---

## Implemented Features  
- **Distance Vector Routing (DVR)**  
  - Initializes each node’s distance vector and next‑hop table.  
  - Iteratively exchanges routing tables with neighbors until convergence.  
  - Prints per‑iteration tables and final stable tables.

- **Link State Routing (LSR)**  
  - Each node learns full topology.  
  - Runs Dijkstra’s algorithm per node to compute shortest paths.  
  - Builds and prints final routing tables.

---

## Code Flow  
1. **`main()`**  
   - Parse command‑line argument (input file).  
   - Read adjacency matrix via `readGraphFromFile()`.  
   - Call `simulateDVR(graph)`.  
   - Call `simulateLSR(graph)`.

2. **`simulateDVR()`**  
   - Copy the input graph into a `dist` matrix.  
   - Initialize `nextHop` for direct neighbors.  
   - Loop until no updates:  
     1. Copy old tables.  
     2. For each node _i_, for each neighbor _j_ with finite cost:  
        - For each destination _k_, see if path _i → j → k_ is cheaper.  
        - If so, update `dist[i][k]` and `nextHop[i][k]`.  
     3. Print all nodes’ tables for this iteration.  
   - After convergence, print final tables.

3. **`simulateLSR()`**  
   - For each source node _src_:  
     1. Initialize `dist[src]=0`, all others = ∞; `prev[] = -1`.  
     2. Use a min‑heap to select the unvisited node _u_ with smallest `dist[u]`.  
     3. Relax all outgoing edges _u → v_: if `dist[u] + cost(u,v) < dist[v]`, update `dist[v]` and `prev[v]`.  
     4. Repeat until all nodes visited.  
     5. Print final routing table by back‑tracking `prev[]` to determine the first hop.

---

## Sample Output  
Running `./routing_sim input1.txt` yields:

```
--- Distance Vector Routing Simulation ---
Iteration 1:
Node 0 Routing Table:
Dest    Cost    Next Hop
0       0       -
1       10      1
2       100     2
3       30      3
... (tables for Nodes 1–3)

--- DVR Final Tables ---
Node 0 Routing Table:
Dest    Cost    Next Hop
0       0       -
1       10      1
2       40      3
3       30      3

... (final tables for Nodes 1–3)

--- Link State Routing Simulation ---
Node 0 Routing Table:
Dest    Cost    Next Hop
1       10      1
2       40      3
3       30      3

Node 1 Routing Table:
Dest    Cost    Next Hop
0       10      0
2       20      2
3       40      3

... (tables for Nodes 2 & 3)
```

---

## Dependencies  
- **C++ compiler** with C++11 support (e.g. `g++`).  
- **Make** (optional, for using the provided `Makefile`).  
- Standard C++ library (no external libraries required).

---

## Testing  
1. **Compile** with `make`.  
2. **Execute** on each sample input:  
   ```bash
   ./routing_sim input1.txt
   ./routing_sim input2.txt
   ./routing_sim input3.txt
   ./routing_sim input4.txt
   ```
3. **Verify**:  
   - DVR tables converge to the expected final costs/next hops.  
   - LSR tables match Dijkstra’s output.

---
## Member Contributions
- **Mohammed Sibtain Ansari** (210616) (33.33%): Implemented DVR algorithm.
- **Avinash Shukla** (210236) (33.33%): Implemented LSR algorithm.
- **Mantapuram Shreeja** (210592) (33.33%): Handled code integration and README documentation.

---

## References
- Computer Networking: A Top-Down Approach, Jim Kurose
- Lecture slides and class discussions

---
## Academic Integrity Declaration  
We hereby declare that this submission is our own original work. We have neither given nor received unauthorized assistance, and all sources consulted have been appropriately cited. We understand that any violation of academic integrity policies may result in disciplinary action.
