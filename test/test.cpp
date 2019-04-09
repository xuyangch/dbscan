#include "SingleLinkageTree.hpp"
#include "CondensedTree.hpp"
#include <iostream>
#include <chrono> 
#include <fstream>

using namespace std;
using namespace std::chrono; 

int main() {
  try
  {
    using T = float;
    using U = int;

    ios::sync_with_stdio(false);

    ifstream file("/Users/hyacinth/workspace/dbscan/data/mst.txt");
    vector<tuple<U, U, T>> mst{};    
    U id1;
    U id2;
    T distance;
    while (file >> id1 >> id2 >> distance) {
      mst.push_back(make_tuple(id1, id2, distance));
    }

    auto start = high_resolution_clock::now();
    hdbscan::SingleLinkageTree<T, U> slt{mst};
    //slt.print();
    hdbscan::CondensedTree<T, U> ct{slt, 6};
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);     
    ct.print();
    cout << "Time taken by function: "
         << duration.count() << " milliseconds" << endl; 
  }
  catch ( const exception & e )
  {
    cout << e.what() << endl;
    return -1;
  }
  return 0;
}