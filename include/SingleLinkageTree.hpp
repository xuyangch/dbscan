#ifndef DBSCAN_SINGLELINKAGETREE_H_
#define DBSCAN_SINGLELINKAGETREE_H_

#include <vector>
#include <unordered_map>
#include <iostream>
#include <tuple>
#include <cstdint>
#include <queue>

namespace hdbscan{
template<typename T, typename U>
  class CondensedTree;

template<typename T, typename U>
  class SingleLinkageTree {
    friend CondensedTree<T, U>;
   public:   
    SingleLinkageTree(const std::vector<std::tuple<U, U, T>> &mst): 
    // first node that has > 0 distance: node_nums_mst_
        nodes_(2 * mst.size() + 1) {

      total_nums_ = mst.size() + 1;
      node_nums_mst_ = mst.size() + 1;
      for (auto& edge : mst) {        
        auto x = std::get<0>(edge);
        auto y = std::get<1>(edge);
        combine(x, y, total_nums_);
        nodes_[total_nums_].distance = std::get<2>(edge);
        ++total_nums_;
      }
    };

    void get_leaves(const U node_id, const T distance, std::vector<std::pair<U, T>> &result) const {
      if (nodes_[node_id].left != -1) {
        get_leaves(nodes_[node_id].left, distance, result);
      }
      if (nodes_[node_id].right != -1) {
        get_leaves(nodes_[node_id].right, distance, result);
      }
      if (nodes_[node_id].left == -1 && nodes_[node_id].right == -1) {
        result.emplace_back(node_id, 1 / distance);
      }
    }
    
    void print() {
      std::cout << "Parents: " << std::endl;
      for (decltype(nodes_.size()) i = 0; i < nodes_.size(); ++i) {
        std::cout << i << "->" << nodes_[i].parent << std::endl;
      }

      std::cout << "Representatives: " << std::endl;
      for (decltype(nodes_.size()) i = 0; i < nodes_.size(); ++i) {
        std::cout << i << "->" << nodes_[i].rep << std::endl;
      }

      std::cout << std::endl << "Distance: " << std::endl;
      for (decltype(nodes_.size()) i = 0; i < nodes_.size() - node_nums_mst_; ++i) {
        std::cout << i + node_nums_mst_ << ": " << nodes_[i + node_nums_mst_].distance << std::endl;
      }
    };
    
   private:    
    struct Node {
      T distance = 0;
      // TODO: change to pointers maybe?
      U left = -1;
      U right = -1;
      U parent = -1;

      int rep = -1;
      int size = 1;
      
    };
    std::vector<Node> nodes_;
    int total_nums_ = 0;
    int node_nums_mst_ = 0;    

    U find(U x, int id) {
      while (nodes_[x].rep >= 0) {
        U rep = find(nodes_[x].rep, id);
        nodes_[x].rep = id;
        return rep;
      }
      return x;
    };

    void combine(U x, U y, int id) {
      U rx = find(x, id);
      U ry = find(y, id);
      nodes_[rx].rep = id;
      nodes_[ry].rep = id;
      nodes_[rx].parent = id;
      nodes_[ry].parent = id;
      nodes_[id].size = nodes_[rx].size + nodes_[ry].size;
      nodes_[id].left = rx;
      nodes_[id].right = ry;
    };

  };  
};

#endif