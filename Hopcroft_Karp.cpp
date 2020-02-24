#include <iostream>
#include <algorithm>

using namespace std;

struct edge{
    int vertex{};
    edge *next{};
};

struct appointment{
    int personA;
    int personB;
};

// Graph operations
edge **make_graph(int SIZE){
    edge **graph = new edge*[SIZE];

    for(int i=0; i<SIZE; i++){
        graph[i] = new edge;
        graph[i]->next = nullptr;
    }

    return graph;
}

void add_undirected_edge(edge **graph, int first, int second) {      // for undirected graphs
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

void addUndirectedEdgeBipartite(edge **bipartiteGraph, int* officials, int officialsNum, int personA, int personB) {
    bool firstOfficial = false;
    bool secondOfficial = false;

    for (int i=0; i<officialsNum && (!firstOfficial || !secondOfficial); i++) {
        if (officials[i] == personA)
            firstOfficial = true;
        else if (officials[i] == personB)
            secondOfficial = true;
    }

    if (!firstOfficial || !secondOfficial){
        add_undirected_edge(bipartiteGraph, personA, personB);
    }
}

int **make_graph_matrix(int SIZE){ // making graph represented by matrix; -1 when there is no edge
    int **graph = new int*[SIZE];

    for(int i=0; i<SIZE; i++)
        graph[i] = new int[SIZE];

    for(int i=0; i<SIZE; i++){
        for(int j=0; j<SIZE; j++)
            graph[i][j] = -1;
    }

    return graph;
}

int **transcript_to_matrix(edge **graph, int SIZE){
    int **graph_matrix = make_graph_matrix(SIZE);

    for(int i=0; i<SIZE; i++){
        edge *curr = graph[i];
        while(curr->next != nullptr){
            graph_matrix[i][curr->next->vertex] = 1;
            curr = curr->next;
        }
    }

    return graph_matrix;
}

void print(int **graph, int SIZE){
    for(int i=0; i<SIZE; i++){
        for(int j=0; j<SIZE; j++)
            if(graph[j][i] != -1)
                cout<<graph[j][i]<<" ";
            else
                cout<<"N ";
        cout<<endl;
    }
}

void print(edge **graph, int SIZE){
    int **graph_matrix = transcript_to_matrix(graph, SIZE);

    print(graph_matrix, SIZE);
}


// Main algorithm
bool same(edge **graphA, edge **graphB, int SIZE){
    for (int i=0; i<SIZE; i++) {
        edge *currA = graphA[i];
        edge *currB = graphB[i];

        while(currA != nullptr) {
            if (currB == nullptr || currA->vertex != currB->vertex)
                return false;
            currA = currA -> next;
            currB = currB -> next;
        }

        if (currB != nullptr)
            return false;
    }
    return true;
}

//int *getOthers(int citizensNum, int *officials, int officialsNum) {
//    int *others = new int[citizensNum-officialsNum];
//    sort(officials, officials + officialsNum);
//
//    int currOfficial = 0;
//    int other = 0;
//    for (int i=0; i<citizensNum; i++) {
//        if (currOfficial >= officialsNum || officials[currOfficial] > i)
//            others[other++] = i;
//        else
//            currOfficial++;
//    }
//    return others;
//}

int *findMatchingGreedy(edge **graph, int citizensNum) {
    int *matching = new int[citizensNum];

    for(int i=0; i<citizensNum; i++)
        matching[i] = -1;

    for (int i=0; i<citizensNum; i++) {
        edge *currEdge = graph[i] -> next;
        while (currEdge != nullptr && matching[i] == -1) {
            if (matching[currEdge -> vertex] == -1){
                matching[i] = currEdge -> vertex;
                matching[currEdge -> vertex] = i;
            }
            currEdge = currEdge -> next;
        }
    }

    return matching;
}

bool dfs(edge **bipartiteGraph, int currPerson, int *matching, bool* visited) {
    visited[currPerson] = true;
    edge *currMetPerson = bipartiteGraph[currPerson] -> next;

    while (currMetPerson != nullptr) {
        if (!visited[currMetPerson -> vertex] && (matching[currMetPerson->vertex] == -1 ||
                (matching[currMetPerson->vertex] != currPerson &&
                dfs(bipartiteGraph, matching[currMetPerson->vertex], matching, visited)))) {
            visited[currMetPerson->vertex] = true;
            matching[currMetPerson->vertex] = currPerson;
            matching[currPerson] = currMetPerson->vertex;
            return true;
        }
        currMetPerson = currMetPerson -> next;
    }

    return false;
}

int *findMatching(edge **bipartiteGraph, int citizensNum, int *officials, int officialsNum) {
    // preparing data
    // int *others = getOthers(citizensNum, officials, officialsNum);

    int *matching = new int[citizensNum];
    bool *visited = new bool[citizensNum];

    for(int i=0; i<citizensNum; i++){
        matching[i] = -1;
        visited[i] = false;
    }

    // main loop
    bool continueLoop = true;

    while(continueLoop) {
        continueLoop = false;
        for (int i=0; i<officialsNum; i++) {
            if (matching[officials[i]] == -1 && dfs(bipartiteGraph, officials[i], matching, visited))
                continueLoop = true;
        }

        for (int i=0; i<citizensNum; i++)
            visited[i] = false;
    }

    return matching;
}

void Hopcroft_Karp() {
    string name;
    cin >> name;

    int citizensNum;
    cin >> citizensNum;

    int officialsNum;
    cin >> officialsNum;

    int *officials = new int[officialsNum];
    for (int i=0; i<officialsNum; i++)
        cin >> officials[i];

    int appointmentsNum;
    cin >> appointmentsNum;

    edge **graph = make_graph(citizensNum);
    edge **bipartite_graph = make_graph(citizensNum);

    auto *appointments = new appointment[appointmentsNum];
    for (int i=0; i<appointmentsNum; i++) {
        cin >> appointments[i].personA >> appointments[i].personB;
        add_undirected_edge(graph, appointments[i].personA, appointments[i].personB);
        addUndirectedEdgeBipartite(bipartite_graph, officials, officialsNum, appointments[i].personA, appointments[i].personB);
    }

    int *matching;

    if (same(graph, bipartite_graph,citizensNum))
        matching = findMatching(bipartite_graph, citizensNum, officials, officialsNum);
    else
        matching = findMatchingGreedy(graph, citizensNum);

    int **matchingPairs = new int*[citizensNum/2];
    int currMatching = 0;

    for(int i=0; i<citizensNum; i++){
        if (matching[i] != -1) {
            matchingPairs[currMatching] = new int[2];
            matchingPairs[currMatching][0] = i;
            matchingPairs[currMatching][1] = matching[i];
            matching[matching[i]] = -1;
            matching[i] = -1;
            currMatching++;
        }
    }

    cout << currMatching << endl;
    for(int i=0; i<currMatching; i++)
        cout << matchingPairs[i][0] << " " << matchingPairs[i][1] << endl;
}