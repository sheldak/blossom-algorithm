#include <iostream>
#include <algorithm>

using namespace std;

// structures
struct edge{
    int vertex;
    edge *next;
};

enum location{
    inner,
    outer
};

struct treeNode{
    int vertex;
    int treeStart;
    location loc;
    treeNode* parent;
    bool visited;
};

struct cycleNode{
    int vertex;
    cycleNode *next;
};

struct blossom{
    int base;
    cycleNode *baseNode;
    blossom *prev;
};

// graph operations
edge **make_graph(int SIZE){
    edge **graph = new edge*[SIZE];

    for(int i=0; i<SIZE; i++){
        graph[i] = new edge;
        graph[i]->next = nullptr;
    }

    return graph;
}

void add_undirected_edge(edge **graph, int first, int second) {
    edge *new_edge = new edge;
    new_edge->vertex = second;
    new_edge->next = nullptr;

    edge *curr = graph[first];
    while (curr->next != nullptr)
        curr = curr->next;

    curr->next = new_edge;

    edge *new_edge2 = new edge;
    new_edge2->vertex = first;
    new_edge2->next = nullptr;

    curr = graph[second];
    while(curr->next != nullptr)
        curr = curr->next;

    curr->next = new_edge2;
}

// algorithm
int findBase(treeNode *a, treeNode *b) { // base of the blossom
    // checking visited vertices from a to the root
    treeNode *currA = a;
    while (currA != nullptr) {
        currA -> visited = true;
        currA = currA -> parent;
    }

    // finding first marked vertex on path from b to the root
    treeNode *currB = b;
    while(currB != nullptr && !currB -> visited)
        currB = currB -> parent;

    int base = -1;
    if (currB != nullptr)
        base = currB -> vertex;

    // unchecking visited vertices
    currA = a;
    while (currA != nullptr) {
        currA -> visited = false;
        currA = currA -> parent;
    }

    return base;
}


cycleNode *makeCycle(treeNode *a, treeNode *b, int base) { // making cycle from blossom
    if (a -> vertex == base) { // a is an ancestor of b
        auto *startCycle = new cycleNode;
        startCycle -> vertex = b -> vertex;
        cycleNode *currCycleNode = startCycle;
        treeNode *currTreeNode = b;

        while (currTreeNode -> vertex != base) {
            auto *newCycleNode = new cycleNode;
            newCycleNode -> vertex = currTreeNode -> parent -> vertex;
            currCycleNode -> next = newCycleNode;

            currCycleNode = newCycleNode;
            currTreeNode = currTreeNode -> parent;
        }

        currCycleNode -> next = startCycle;

        return currCycleNode;
    }
    else if (b -> vertex == base){ // b is an ancestor of a
        auto *startCycle = new cycleNode;
        startCycle -> vertex = a -> vertex;
        cycleNode *currCycleNode = startCycle;
        treeNode *currTreeNode = a;

        while (currTreeNode -> vertex != base) {
            auto *newCycleNode = new cycleNode;
            newCycleNode -> vertex = currTreeNode -> parent -> vertex;
            currCycleNode -> next = newCycleNode;

            currCycleNode = newCycleNode;
            currTreeNode = currTreeNode -> parent;
        }
        currCycleNode -> next = startCycle;

        return currCycleNode;
    }
    else {  // neither is a an ancestor of b nor b an ancestor of a
        auto *startCycle = new cycleNode;
        startCycle -> vertex = a -> vertex;
        cycleNode *currCycleNode = startCycle;
        treeNode *currTreeNode = a;

        while(currTreeNode -> vertex != base) {
            auto *newCycleNode = new cycleNode;
            newCycleNode -> vertex = currTreeNode -> parent -> vertex;
            currCycleNode -> next = newCycleNode;

            currCycleNode = newCycleNode;
            currTreeNode = currTreeNode -> parent;
        }

        cycleNode *baseCycleNode = currCycleNode;

        auto *endCycle = new cycleNode;
        endCycle -> vertex = b -> vertex;
        currCycleNode = endCycle;
        currTreeNode = b;


        while(currTreeNode -> parent -> vertex != base) {
            auto *newCycleNode = new cycleNode;
            newCycleNode -> vertex = currTreeNode -> parent -> vertex;
            newCycleNode -> next = currCycleNode;

            currCycleNode = newCycleNode;
            currTreeNode = currTreeNode -> parent;
        }
        baseCycleNode -> next = currCycleNode;

        endCycle -> next = startCycle;

        return baseCycleNode;
    }
}

void contractBlossom(blossom *lastBlossom, cycleNode *cycleBase, int *trueVertex) {
    int base = cycleBase -> vertex;

    cycleNode *currNode = cycleBase->next;

    while(currNode -> vertex != cycleBase -> vertex){
        // if blossom contains other contracted blossoms
        blossom *currBlossom = lastBlossom;
        while (currBlossom != nullptr) {
            if (currBlossom -> base == currNode -> vertex) {
                cycleNode *currBlossomNode = currBlossom -> baseNode -> next;
                while (currBlossomNode != currBlossom -> baseNode) {
                    trueVertex[currBlossomNode -> vertex] = base;
                    currBlossomNode = currBlossomNode -> next;
                }
            }
            currBlossom = currBlossom -> prev;
        }

        // contracting all vertices in current blossom
        trueVertex[currNode -> vertex] = base;
        currNode = currNode -> next;
    }
}

blossom *blossomContraction(blossom *lastBlossom, treeNode **vertices, int *trueVertex, int vertexA, int vertexB) {
    auto *newBlossom = new blossom;

    int base = findBase(vertices[vertexA], vertices[vertexB]);
    cycleNode *cycle = makeCycle(vertices[vertexA], vertices[vertexB], base);
    contractBlossom(lastBlossom, cycle, trueVertex);

    newBlossom -> base = base;
    newBlossom -> baseNode = cycle;

   return newBlossom;
}

void modifyMatching(int *matching, treeNode *vertexA, treeNode *vertexB) {
    // modifying matching by path from the root of a to the root of b to get 1 more matching

    // changing marching from vertex a
    treeNode *currNode = vertexA -> parent;

    while (currNode != nullptr) {
        matching[currNode -> vertex] = currNode -> parent -> vertex;
        matching[currNode -> parent -> vertex] = currNode -> vertex;
        currNode = currNode -> parent -> parent;
    }

    // changing matching from vertex b
    currNode = vertexB -> parent;

    while (currNode != nullptr) {
        matching[currNode -> vertex] = currNode -> parent -> vertex;
        matching[currNode -> parent -> vertex] = currNode -> vertex;
        currNode = currNode -> parent -> parent;
    }

    // adding matching between vertices a and b
    matching[vertexA -> vertex] = vertexB -> vertex;
    matching[vertexB -> vertex] = vertexA -> vertex;
}

void resetVertices(treeNode **vertices, const int *matching, const int *trueVertices, int SIZE) {
    // if a vertex is in the matching, vertices[i] = nullptr
    // the rest of vertices are free vertices (roots of trees)
    for (int i=0; i<SIZE; i++) {
        if (i == trueVertices[i] && matching[i] == -1) {
            auto *newTreeNode = new treeNode;
            newTreeNode -> vertex = i;
            newTreeNode -> treeStart = i;
            newTreeNode -> loc = outer;
            newTreeNode -> parent = nullptr;
            newTreeNode -> visited = false;
            vertices[i] = newTreeNode;
        }
        else
            vertices[i] = nullptr;
    }
}

void extendBlossom(edge **graph, blossom *currBlossom, int *matching, int *trueVertex) { // reversing blossom contraction
    // if there is no matching from blossom's base, there is no need to change any matching
    if (matching[currBlossom -> base] != -1) {
        bool change = true;
        edge *baseNeigh = graph[currBlossom -> base] -> next;

        // if matching is to blossom's base, there is no need to change any matching
        while(baseNeigh != nullptr) {
            if (trueVertex[baseNeigh -> vertex] == matching[currBlossom -> base])
                change = false;

            baseNeigh = baseNeigh -> next;
        }

        // if matching is to other blossom's vertex, there is a need to change matching
        cycleNode *currNode = currBlossom -> baseNode -> next;
        while (currNode -> vertex != currBlossom -> baseNode -> vertex && change) {
            edge *currNeigh = graph[matching[currBlossom -> base]] -> next;
            while (currNeigh != nullptr) {
                if (trueVertex[currNeigh -> vertex] == currNode -> vertex) {
                    matching[currNode -> vertex] = matching[currBlossom -> base];
                    matching[matching[currBlossom -> base]] = currNeigh -> vertex;
                    //matching[currBlossom -> base] = -1;

//                    matching[currNode -> vertex] = trueVertex[currNeigh -> vertex];
//                    matching[trueVertex[currNeigh -> vertex]] = currNode -> vertex;

                    // changing matching of all vertices in the blossom
                    cycleNode *matchingBase = currNode;
                    currNode = currNode -> next;
                    while(currNode != matchingBase) {
                        matching[currNode -> vertex] = currNode -> next -> vertex;
                        matching[currNode -> next -> vertex] = currNode -> vertex;

                        currNode = currNode -> next -> next;
                    }
                    change = true;
                    break;
                }
                currNeigh = currNeigh -> next;
            }
            currNode = currNode -> next;
        }
    }

    cycleNode *currNode = currBlossom -> baseNode -> next;

    while (currNode -> vertex != currBlossom -> baseNode -> vertex) {
        trueVertex[currNode -> vertex] = currNode -> vertex;
        currNode = currNode -> next;
    }
}

int *blossomAlgorithm(edge **graph, int SIZE) {
    int *matching = new int[SIZE]; // result of the algorithm
    for(int i=0; i<SIZE; i++) // initially no matching
        matching[i] = -1;

    auto **vertices = new treeNode*[SIZE]; // forest
    int *trueVertex = new int[SIZE]; // to know which vertices are contracted
    blossom *lastBlossom = nullptr; // list of blossoms

    for (int i=0; i<SIZE; i++) // initially every vertex is not contracted
        trueVertex[i] = i;

    resetVertices(vertices, matching, trueVertex, SIZE); // initially all vertices are roots of trees

    while(true) { // until there is no blossom to contract and no augmenting path to modify matching
        bool addedVertices = false; // flag to get information if any vertices were added

        // adding vertices to the forest (every time just one)
        for (int i=0; i<SIZE && !addedVertices; i++){

            // having vertex not in forest, looking for vertex in the forest
            if (vertices[i] == nullptr && i == trueVertex[i]) {
                edge *currNeigh = graph[i]->next;
                while(currNeigh != nullptr) {

                    // adding vertex outside forest and its matching to the outer vertex in the forest
                    if (vertices[trueVertex[currNeigh->vertex]] != nullptr &&
                    vertices[trueVertex[currNeigh->vertex]] -> loc == outer) {

                        auto *innerVertex = new treeNode; // found vertex
                        auto *outerVertex = new treeNode; // that vertex's matching

                        innerVertex -> vertex = i;
                        innerVertex -> treeStart = vertices[trueVertex[currNeigh->vertex]]->treeStart;
                        innerVertex -> loc = inner;
                        innerVertex -> parent = vertices[trueVertex[currNeigh->vertex]];
                        innerVertex -> visited = false;

                        outerVertex -> vertex = matching[i];
                        outerVertex -> treeStart = vertices[trueVertex[currNeigh->vertex]]->treeStart;
                        outerVertex -> loc = outer;
                        outerVertex -> parent = innerVertex;
                        outerVertex -> visited = false;

                        vertices[i] = innerVertex;
                        vertices[matching[i]] = outerVertex;
                        addedVertices = true;
                        break;
                    }
                    currNeigh = currNeigh -> next;
                }
            }
        }

        // connecting two outer vertices in the forest
        if (!addedVertices) { // because just one action in every iteration of while loop
            for (int i=0; i<SIZE && !addedVertices; i++) {

                // non-contracted outer vertices
                if (vertices[i] != nullptr && i == trueVertex[i] && vertices[i] -> loc == outer) {
                    edge *currNeigh = graph[i]->next;
                    while(currNeigh != nullptr) {

                        // second vertex has to be non-contracted and outer as well
                        if (vertices[trueVertex[currNeigh->vertex]] != nullptr && i != trueVertex[currNeigh->vertex]
                        && vertices[trueVertex[currNeigh->vertex]] -> loc == outer) {

                            // making augmenting path and change matching to get one more
                            if (vertices[i] -> treeStart != vertices[trueVertex[currNeigh->vertex]] -> treeStart) {
                                modifyMatching(matching, vertices[i], vertices[trueVertex[currNeigh->vertex]]);
                                resetVertices(vertices, matching, trueVertex, SIZE);
                            }
                            // contracting blossom
                            else {
                                blossom *newBlossom = blossomContraction(lastBlossom, vertices, trueVertex, i, trueVertex[currNeigh->vertex]);
                                newBlossom -> prev = lastBlossom;
                                lastBlossom = newBlossom;
                            }
                            addedVertices = true;
                            break;
                        }
                        currNeigh = currNeigh -> next;
                    }
                }
            }
        }

        // no action in that iteration so there is no possibility to add more matchings
        if (!addedVertices)
            break;
    }

    // reversing blossom's contractions
    blossom *currBlossom = lastBlossom;
    while(currBlossom != nullptr){
        extendBlossom(graph, currBlossom, matching, trueVertex);
        currBlossom = currBlossom -> prev;
    }

    return matching;
}

void Edmonds() {
    // reading parameters and making graph
    string name;
    cin >> name;

    int citizensNum;
    cin >> citizensNum;

    int officialsNum;
    cin >> officialsNum;

    int *officials = new int[officialsNum];
    for (int i = 0; i < officialsNum; i++)
        cin >> officials[i];

    int appointmentsNum;
    cin >> appointmentsNum;

    edge **graph = make_graph(citizensNum);

    for (int i = 0; i < appointmentsNum; i++) {
        int personA, personB;
        cin >> personA >> personB;
        add_undirected_edge(graph, personA, personB);
    }

    // Edmond's blossom algorithm
    int *matching;

    matching = blossomAlgorithm(graph, citizensNum);

    // preparing data to write
    int **matchingPairs = new int *[citizensNum / 2];
    int currMatching = 0;

    for (int i = 0; i < citizensNum; i++) {
        if (matching[i] != -1) {
            matchingPairs[currMatching] = new int[2];
            matchingPairs[currMatching][0] = i;
            matchingPairs[currMatching][1] = matching[i];
            matching[matching[i]] = -1;
            matching[i] = -1;
            currMatching++;
        }
    }

    // returning result
    cout << currMatching << endl;
    for (int i = 0; i < currMatching; i++)
        cout << matchingPairs[i][0] << " " << matchingPairs[i][1] << endl;
}