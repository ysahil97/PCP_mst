#include<stdio.h>
#include<thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <mutex>
#include <sys/time.h>
#include <vector>
#include<queue>
#include<algorithm>
#define MAX 1001
#define INF 9999
#define notIncluded 0
#define included 1


double my_clock(void) {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (1.0e-6*t.tv_usec + t.tv_sec);
}


using namespace std;
int nVertex, graph[MAX][MAX];
int keyVal[MAX], mstSet[MAX], parent[MAX];

int num_threads,num_edges;


vector< pair<int, int> > finalres;

void foo1(int id){
  if(id != num_threads-1){
    int i, minEdgeValue = INF, minVertx = id*nVertex/num_threads;
    for (i = id*nVertex/num_threads; i < (id+1)*nVertex/num_threads; i++)
    {
        if (mstSet[i] == notIncluded && keyVal[i]< minEdgeValue)
        {
            minVertx = i;
            minEdgeValue = keyVal[i];
        }
    }
    finalres.push_back(make_pair(minEdgeValue,minVertx));
  }
  else{
    int i, minEdgeValue = INF, minVertx = id*nVertex/num_threads;
    for (i = id*nVertex/num_threads; i < nVertex; i++)
    {
        if (mstSet[i] == notIncluded && keyVal[i]< minEdgeValue)
        {
            minVertx = i;
            minEdgeValue = keyVal[i];
        }
    }
    finalres.push_back(make_pair(minEdgeValue,minVertx));
  }
}

int findMinEdgeCut()
{
    thread threads[num_threads];

    for(int i = 1; i <= num_threads; i++){
        threads[i-1] = thread(foo1,i-1);
    }
    for(int i = 1; i <= num_threads; i++){
        threads[i-1].join();
    }

    sort(finalres.begin(),finalres.end());
    int x = finalres[0].second;
    finalres.clear();
    return x;
}

void init()
{
    int i;
    for (i = 0; i < nVertex; i++)
    {
        keyVal[i] = INF;
        mstSet[i] = notIncluded;
        parent[i] = -1;
    }
}


void foo2(int id,int u){
  if(id != num_threads-1){
    int v;
    for (v = id*nVertex/num_threads; v < (id+1)*nVertex/num_threads; v++)
    {
        if (graph[u][v] && mstSet[v] == notIncluded && keyVal[v] > graph[u][v])
        {
            keyVal[v] = graph[u][v];
            parent[v] = u;
        }
    }
  }
  else{
    int v;
    for (v = id*nVertex/num_threads; v < nVertex; v++)
    {
        if (graph[u][v] && mstSet[v] == notIncluded && keyVal[v] > graph[u][v])
        {
            keyVal[v] = graph[u][v];
            parent[v] = u;
        }
    }
  }
}

void prims()
{
    int v,u, edgeCount;
    init();

    keyVal[0] = 0;
    parent[0] = -1;




    for (edgeCount = 0; edgeCount < nVertex - 1; edgeCount++)
    {
        u = findMinEdgeCut();
        mstSet[u] = included;

        thread threads[num_threads];

        for(int i = 1; i <= num_threads; i++){
          threads[i-1] = thread(foo2,i-1,u);
        }
        for(int i = 1; i <= num_threads; i++){
          threads[i-1].join();
        }
    }
}

int resultPrims()
{
    int  v, cost = 0;
    for (v = 1; v < nVertex; v++)
    {
        printf("%d -> %d   Cost:%d\n", parent[v], v, graph[parent[v]][v]);
        cost = cost + graph[parent[v]][v];
    }
    return cost;
}



int main()
{
    int i, j;
    cin >> num_threads>>nVertex>>num_edges;

    for(int i = 0;i<num_edges;i++){
      int x,y;
      cin >> x >> y;
      cin>> graph[x-1][y-1];
      graph[y-1][x-1] = graph[x-1][y-1];
    }


    double start = my_clock();
    prims();
    double end = my_clock();
    printf("Cost: %d\n", resultPrims());
    cout<<"Time: "<<end-start<<endl;
    return 0;
}
