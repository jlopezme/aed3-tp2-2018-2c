#include "graph.h"
#include <algorithm>
#include <limits>
#include <queue>

#define NOT_VISITED 0


Node::Node(const int &id)
: id(id) {}

Edge::Edge(const int id, const Node &first, const Node &second, const float weight)
: id(id), nodes(make_pair(first, second)), weight(weight) {}

Graph::Graph() {
    _edges = vector<Edge>();
    _adjacencyList = vector<vector<Node>>();
}

Graph::Graph(const int &nodesNumber)
{
    _edges = vector<Edge>();
    _adjacencyList = vector<vector<Node>>(nodesNumber, vector<Node>());
}

void Graph::build(const int &nodesNumber) {
    _adjacencyList.resize(nodesNumber);
}

void Graph::addNode()
{
    _adjacencyList.push_back(vector<Node>());
}

void Graph::addEdge(const Node &first, const Node &second)
{
    addEdge(first, second, 0);
}

void Graph::addEdge(const Node &first, const Node &second, const float weight)
{
    _edges.push_back(Edge(_edges.size(), first, second, weight));
    _adjacencyList[first.id].push_back(second.id);
    _adjacencyList[second.id].push_back(first.id);
}

vector<Edge> Graph::getEdges()
{
    return _edges;
}

vector<vector<Node>> Graph::getAdjacencyList()
{
    return _adjacencyList;
}

bool Graph::operator==(Graph &other) {
    this->sortAdjacencyLists();
    other.sortAdjacencyLists();
    std::sort(this->_edges.begin(),this->_edges.end());
    std::sort(other.getEdges().begin(),other.getEdges().end());
    return  (this->_edges == other.getEdges() ) && (this->_adjacencyList == other.getAdjacencyList());
}

void Graph::sortAdjacencyLists() {
    for (auto &index : this->_adjacencyList) {
        std::sort(index.begin(), index.end() );
    }
}

vector<vector<float>> Graph::getAdjacencyMatrix() const
{
    auto nodesNumber = _adjacencyList.size();
    // initialize matrix in -1 which means no edge exists
    auto adjacencyMatrix = vector<vector<float>>(nodesNumber, vector<float>(nodesNumber, -1));
    for (auto const& edge : _edges)
    {
        auto first = edge.nodes.first.id;
        auto second = edge.nodes.second.id;
        adjacencyMatrix[first][second] = edge.weight;
        adjacencyMatrix[second][first] = edge.weight;
    }
    return adjacencyMatrix;
}

float Graph::getTotalWeigth() {
    float totalWeight = 0;
    for (auto const& edge : this->getEdges())
    {
        totalWeight += edge.weight;
    }
    return totalWeight;
}

void Graph::addEdge(Edge otherGraphEdge) {
    auto &first = otherGraphEdge.nodes.first;
    auto &second = otherGraphEdge.nodes.second;
    _edges.push_back(Edge(_edges.size(), first, second, otherGraphEdge.weight));
    _adjacencyList[first.id].push_back(second.id);
    _adjacencyList[second.id].push_back(first.id);
}

void Graph::removeEdge(Edge edge)
{
    // remove from edges list;
    _edges.erase(remove_if(
        _edges.begin(), _edges.end(),
        [edge](const Edge& x) { 
        return x.id == edge.id;
    }), _edges.end());

    // remove from adjacency list
    auto firstNode = edge.nodes.first;
    auto secondNode = edge.nodes.second;

    _adjacencyList[firstNode.id].erase(remove_if(
        _adjacencyList[firstNode.id].begin(), _adjacencyList[firstNode.id].end(),
        [secondNode](const Node& x) { 
        return x.id == secondNode.id;
    }), _adjacencyList[firstNode.id].end());

    _adjacencyList[secondNode.id].erase(remove_if(
        _adjacencyList[secondNode.id].begin(), _adjacencyList[secondNode.id].end(),
        [firstNode](const Node& x) { 
        return x.id == firstNode.id;
    }), _adjacencyList[secondNode.id].end());
}

void Graph::setMSTStrategy(MSTStrategy *strategy) {
    this->mstStrategy = strategy;
}

Graph Graph::getMST() {
    return mstStrategy->getMST(this);
}

DisjoinSetCompressed *KruskalCompressedMST::getContainer() {
    return new DisjoinSetCompressed();
}

DisjoinSetDefault * KruskalDefaultMST::getContatiner() {
    return new DisjoinSetDefault();
}

Graph KruskalDefaultMST::getMST(Graph* graph) {
    Graph mst(graph->getAdjacencyList().size());
    DisjoinSet *disjoinSet = this->getContatiner();
    disjoinSet->create(graph);
    vector<Edge> edges = graph->getEdges();
    sort(edges.begin(),edges.end(), cmpWeigth);
    for(Edge edge : edges){
        if(disjoinSet->find(edge.nodes.first) !=  disjoinSet->find(edge.nodes.second)){
            mst.addEdge(edge);
            disjoinSet->join(disjoinSet->find(edge.nodes.first), disjoinSet->find(edge.nodes.second));
        }
    }
    return mst;
}

Graph KruskalCompressedMST::getMST(Graph* graph) {
    Graph mst(graph->getAdjacencyList().size());
    DisjoinSet *disjoinSet = this->getContainer();
    disjoinSet->create(graph);
    vector<Edge> edges = graph->getEdges();
    sort(edges.begin(),edges.end(), cmpWeigth);
    for(Edge edge : edges){
        if(disjoinSet->find(edge.nodes.first) !=  disjoinSet->find(edge.nodes.second)){
            mst.addEdge(edge);
            disjoinSet->join(disjoinSet->find(edge.nodes.first), disjoinSet->find(edge.nodes.second));
        }
    }
    return mst;
}

Graph PrimMST::getMST(Graph *graph)
{
    auto adjacencyList = graph->getAdjacencyList();
    auto nodesNumber = adjacencyList.size();
    auto adjacencyMatrix = graph->getAdjacencyMatrix();
    auto visited = vector<bool>(nodesNumber, false);
    auto minWeight = vector<float>(nodesNumber, numeric_limits<float>::infinity());
    auto parents = vector<int>(nodesNumber, -1);
    // select first node and update adjacent weights
    visited[0] = true;
    minWeight[0] = 0;
    for (auto const& node : adjacencyList[0])
    {
        minWeight[node.id] = adjacencyMatrix[0][node.id];
        parents[node.id] = 0;
    }

    auto mst = Graph(nodesNumber);
    // add n - 1 edges to complete MST
    for (size_t edgeId = 0; edgeId < nodesNumber - 1; ++edgeId)
    {
        int closestNode;
        auto bestWeight = numeric_limits<float>::infinity();
        // select closest node
        for (size_t nodeId = 1; nodeId < nodesNumber; ++nodeId)
        {
            if (!visited[nodeId] && minWeight[nodeId] < bestWeight)
            {
                closestNode = nodeId;
                bestWeight = minWeight[nodeId];
            }
        }
        // add edge and update weights
        mst.addEdge(Node(parents[closestNode]), Node(closestNode), bestWeight);
        visited[closestNode] = true;
        for (auto const& node : adjacencyList[closestNode])
        {
            if (!visited[node.id]
                && adjacencyMatrix[closestNode][node.id] < minWeight[node.id])
            {
                minWeight[node.id] = adjacencyMatrix[closestNode][node.id];
                parents[node.id] = closestNode;
            }
        }
    }

    return mst;
}

vector<int> Graph::componenteConexaDeVertices() {
    vector<int> vertexByComponent (this->getAdjacencyList().size(), NOT_VISITED);
    int runFrom;
    int numberOfComponent = 1;
    do{
        runFrom = this->firstVertexNotVisited(vertexByComponent);
        this->visitVertexFrom(runFrom, vertexByComponent, numberOfComponent);
        numberOfComponent++;
    }while(this->areVerticesToVisit(vertexByComponent));

    return vertexByComponent;
}

int Graph::firstVertexNotVisited(vector<int> &vertexByComponent) {
    int firstVertex = -1;
    for(int index = 0;index < vertexByComponent.size(); index++){
        if(vertexByComponent.at(index) == NOT_VISITED){
            firstVertex = index;
            break;
        }
    }
    return firstVertex;
}

bool Graph::areVerticesToVisit(vector<int> &vertexByComponent) {
    for(int index = 0; index < vertexByComponent.size(); index++){
        if(vertexByComponent.at(index) == NOT_VISITED){
            return true;
        }
    }
    return false;
}

void Graph::visitVertexFrom(int origin, vector<int> &vertexByComponents, int numberOfComponent) {
    std::queue<int> vertexId;
    int actual;

    vertexId.push(origin);
    while( !vertexId.empty() ){
        actual = vertexId.front();
        vertexId.pop();

        vertexByComponents.at(actual) = numberOfComponent;
        for(Node adyacent : this->_adjacencyList.at(actual)){
            if(vertexByComponents.at(adyacent.id) == NOT_VISITED){
                vertexId.push( adyacent.id );
            }
        }
    }
}

vector<vector<float>> Graph::getAdjacencyMatrixDi() const {
    auto nodesNumber = _adjacencyList.size();
    // initialize matrix in -1 which means no edge exists
    auto adjacencyMatrix = vector<vector<float>>(nodesNumber, vector<float>(nodesNumber, -1));

    for (auto const& edge : _edges)
    {
        auto first = edge.nodes.first.id;
        auto second = edge.nodes.second.id;
        adjacencyMatrix[first][second] = edge.weight;
    }


    return adjacencyMatrix;
}


vector<vector<float>> Graph::getFloydCycle() const {

    auto sizenodes = _adjacencyList.size();
    auto adjMatrix = getAdjacencyMatrixDi();
    auto solMatrix = vector<vector<float>>(sizenodes, vector<float>(sizenodes));
    vector<int> parents(sizenodes,-1);


    //igualo la matriz solucion a la de adj que tengo
    for(int m=0; m< sizenodes; m++){
        for(int n=0; n<sizenodes; n++){
            solMatrix[m][n]= adjMatrix[m][n];
        }
    }

    for(int k=0; k< sizenodes; k++){
        for(int i=0; i< sizenodes; i++){
            for(int j=0; j< sizenodes; j++){
                float uno = solMatrix[i][k];
                float dos = solMatrix[k][j];
                if(uno * dos > solMatrix[i][j] && (uno>0 && dos>0)){
                    solMatrix[i][j] = uno*dos;
                    parents[j] = k;
                }
            }
        }
    }

    return solMatrix;



}


void Graph::addEdgeDir(const Node &from, const Node &to, const float weight)
{
    _edges.push_back(Edge(_edges.size(), from, to, weight));
    _adjacencyList[from.id].push_back(to.id);


}

bool Graph::BellmanFord() const{
    unsigned long nodesNumber = _adjacencyList.size();

    vector<float> weight(nodesNumber, std::numeric_limits<float>::infinity());
    vector<int> parents(nodesNumber, -1);

    vector<int> * ref1 = &parents;
    vector<float> * ref2 = &weight;

    bool existsNegativeCycle = solveBellmanFord(ref1, ref2);


    if(existsNegativeCycle){
        std::cout << "SI";

        //TODO: recomponer el ciclo de divisas que forman el ciclo negativo
        int empiezoEn;
        vector<int> cicloNeg;

        for(int i : parents){
            empiezoEn = i;
            if(i != -1){
                do{
                    cicloNeg.push_back(i);
                    i = parents[i];
                }while(i != -1 || i != empiezoEn);
            }

            if(i != -1 && i == empiezoEn){
                for(int j : cicloNeg){
                    cout << j << " ";
                }
            }
            cicloNeg.clear();
        }


    }else{
        std::cout << "NO";
    }


    return existsNegativeCycle;
}

bool Graph::solveBellmanFord(vector<int> * _parents, vector<float> * _peso) const{
    bool existsNegativeCycle = false;
    unsigned long nodesNumber = _adjacencyList.size();


    vector<float> weight = *(_peso);
    vector<int> parents = *(_parents);

    // INVERTIR EDGES???
    /*
    vector<Edge> invertedEdges(edges.size());
    for (unsigned long i = 0; i < edges.size(); i++){
        invertedEdges[i] = Edge(i, edges[i].nodes.first, edges[i].nodes.first, (float)pow(edges[i].weight, -1));
    }
    */
    weight[0] = 0;
    parents[0] = 0;

    for (unsigned long i = 1; i < nodesNumber - 1; i++) {
        for (Edge e : _edges) {
            if (weight[e.nodes.second.id] > weight[e.nodes.first.id] * e.weight) {
                weight[e.nodes.second.id] = weight[e.nodes.first.id] * e.weight;
                parents[e.nodes.second.id] = e.nodes.first.id;
            }
        }
    }

    //busco ciclo negativo
    for (Edge e : _edges) {
        if (weight[e.nodes.second.id] > weight[e.nodes.first.id] * e.weight) {
            weight[e.nodes.second.id] = weight[e.nodes.first.id] * e.weight;
            parents[e.nodes.second.id] = e.nodes.first.id;
            existsNegativeCycle = true;
        }
    }

    return existsNegativeCycle;
}
