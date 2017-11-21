#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <mutex>
#include <vector>
#include <thread>
#include <queue>

using namespace std;

#define MAX_WEIGHT 1001
#define NUM_VERTICES 1001
#define MAX_EDGES 1001
#define ZERO 0

int **adjM;
int **mstM;

int **createAdjMat()
{
    int **adjM = new int *[NUM_VERTICES];
    for (int i = 0; i < NUM_VERTICES; i++)
    {
        adjM[i] = new int[NUM_VERTICES];
    }
    for (int i = 0; i < NUM_VERTICES; i++)
    {
        for (int j = 0; j < NUM_VERTICES; j++)
        {
            adjM[i][j] = 0;
        }
    }
    return adjM;
}

void printAdjM(int **adjM)
{
    cout << endl
         << "Graph in Adjacency Matrix [" << endl;
    for (int i = 0; i < NUM_VERTICES; i++)
    {
        cout << "\t{";
        for (int j = 0; j < NUM_VERTICES; j++)
        {
            cout << " " << adjM[i][j] << " ";
        }
        cout << "}" << endl;
    }
    cout << "]" << endl;
}

class Node
{
  private:
    int id;
    vector<pair<Node *, int>> edges;
    double totWeight;
    long numEdges;
    bool isAlive;

  public:
    mutex lock;
    Node(int node_id)
    {
        id = node_id;
        totWeight = 0;
        numEdges = 0;
        isAlive = true;
    }
    int getID() { return id; }
    double getTotWeight() { return totWeight; }
    long getNumEdges() { return numEdges; }
    bool getStatus() { return isAlive; }
    void setStatus(bool y) { isAlive = y; }
    vector<pair<Node *, int>> getEdgeList() { return edges; }
    void changeElem(int i, Node *n) { edges[i].first = n; }

    void Union(Node *to, int weight)
    {
        pair<Node *, int> edge = make_pair(to, weight);
        int i = 0;
        while (i < edges.size())
        {
            if (edge.second < edges[i].second)
                break;
            i++;
        }
        edges.insert(edges.begin() + i, edge);
    }

    pair<Node *, int> *getMinEdge()
    {
        if (edges.size() == 0)
            return NULL;
        return &edges[0];
    }

    void merge(Node *other, double eW)
    {
        totWeight += other->getTotWeight() + eW;
        numEdges += other->getNumEdges() + 1;
        vector<pair<Node *, int>> newSet, other_edges = other->getEdgeList();
        int i = 0, j = 0;
        while (i + j < edges.size() + other_edges.size())
        {
            while (i < edges.size())
            {
                pair<Node *, int> e = edges[i];
                if ((e.first) != other)
                    break;
                else
                {
                    mstM[this->id][other->getID()] = eW;
                }
                i++;
            }
            while (j < other_edges.size())
            {
                pair<Node *, int> e = other_edges[j];
                if ((e.first) != this)
                    break;
                else
                {
                    mstM[other->getID()][this->id] = eW;
                }
                j++;
            }
            if (j < other_edges.size() && (i >= edges.size() || edges[i].second > other_edges[j].second))
            {
                Node *x = other_edges[j].first;
                vector<pair<Node *, int>> ef = x->getEdgeList();
                for (int k = 0; k < ef.size(); k++)
                {
                    if (ef[k].first == other)
                    {
                        x->changeElem(k, this);
                        break;
                    }
                }
                newSet.push_back(other_edges[j]);
                j++;
            }
            else if (i < edges.size())
            {
                Node *x = edges[i].first;
                vector<pair<Node *, int>> ef = x->getEdgeList();
                for (int k = 0; k < ef.size(); k++)
                {
                    if (ef[k].first == this)
                    {
                        x->changeElem(k, this);
                        break;
                    }
                }
                newSet.push_back(edges[i]);
                i++;
            }
        }
        other->clearEdgeList();
        edges.clear();
        edges = newSet;
    }

    void clearEdgeList()
    {
        edges.clear();
    }
};

void print_graph(vector<Node *> collect)
{
    for (int i = 0; i < collect.size(); i++)
    {
        cout << "For Node " << collect[i]->getID() << endl;
        vector<pair<Node *, int>> x = collect[i]->getEdgeList();
        for (int j = 0; j < x.size(); j++)
        {
            cout << "\t" << collect[i]->getID() << " " << x[j].first->getID() << " " << x[j].second << endl;
        }
    }
}
int weight = 0;

mutex mm;
void mst(queue<Node *> &nl, int i)
{
    Node *n = NULL;
    while (!(nl.empty()))
    {
        n = nl.front();

        if (n == NULL || !n->lock.try_lock())
            continue;
        nl.pop();

        if (!n->getStatus())
        {
            n->lock.unlock();
            continue;
        }

        pair<Node *, int> *e = n->getMinEdge();
        if (e == NULL)
            break;

        Node *other = (*e).first;

        if (!other->lock.try_lock())
        {
            n->lock.unlock();
            nl.push(n);
            continue;
        }

        if (!other->getStatus())
        {
            other->lock.unlock();
            n->lock.unlock();
            nl.push(n);
            continue;
        }

        other->setStatus(false);
        n->merge(other, (*e).second);
        weight += (*e).second;

        other->lock.unlock();
        n->lock.unlock();
        nl.push(n);
    }
}

int main()
{
    queue<Node *> myqueue;
    int num_vertices, num_edges;
    vector<Node *> collect;
    cin >> num_vertices >> num_edges;
    mstM = createAdjMat();
    for (int i = 0; i < num_vertices; i++)
    {
        Node *u = new Node(i);
        collect.push_back(u);
    }
    cout << "Enter edges in the format:- from to weight\n";
    for (int j = 0; j < num_edges; j++)
    {
        int x, y, z;
        cin >> x >> y >> z;
        collect[x - 1]->Union(collect[y - 1], z);
        collect[y - 1]->Union(collect[x - 1], z);
        adjM[x - 1][y - 1] = z;
    }
    printAdjM(adjM);
    for (int i = 0; i < collect.size(); i++)
    {
        myqueue.push(collect[i]);
    }

    int N = 5;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds;
    start = std::chrono::system_clock::now();

    thread *TV = new thread[N];
    for (int i = 0; i < N; i++)
    {
        TV[i] = thread(mst, ref(myqueue), i);
    }
    for (int i = 0; i < N; i++)
        TV[i].join();

    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "Time taken: " << elapsed_seconds.count() << " ";

    printAdjM(mstM);
    cout << "Total weight: " << weight;
    return 0;
}