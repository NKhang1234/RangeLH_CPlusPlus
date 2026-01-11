#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "data.h"

class Node {
protected:
    bool is_leaf;
    std::vector<int> keys;
    Node* parent;
    
public:
    Node(bool leaf = false) : is_leaf(leaf), parent(nullptr) {}
    virtual ~Node() {}
    
    bool isLeaf() const { return is_leaf; }
    std::vector<int>& getKeys() { return keys; }
    const std::vector<int>& getKeys() const { return keys; }
    
    friend class BPlusTree;
};

class LeafNode : public Node {
private:
    std::vector<Data*> data_ptrs;
    LeafNode* next;
    LeafNode* prev;
    
public:
    LeafNode() : Node(true), next(nullptr), prev(nullptr) {}
    
    std::vector<Data*>& getDataPtrs() { return data_ptrs; }
    const std::vector<Data*>& getDataPtrs() const { return data_ptrs; }
    
    friend class BPlusTree;
};

class InternalNode : public Node {
private:
    std::vector<Node*> children;
    
public:
    InternalNode() : Node(false) {}
    
    ~InternalNode() {
        for (Node* child : children) {
            delete child;
        }
    }
    
    std::vector<Node*>& getChildren() { return children; }
    const std::vector<Node*>& getChildren() const { return children; }
    
    friend class BPlusTree;
};

class BPlusTree {
private:
    Node* root;
    int order; // Maximum number of keys per node
    int count;
    
    // Find the leaf node where key should be inserted
    LeafNode* findLeaf(int key);
    
    // Split a leaf node
    void splitLeaf(LeafNode* leaf);
    
    // Split an internal node
    void splitInternal(InternalNode* internal);
    
    // Insert key into parent after split
    void insertIntoParent(Node* left, int key, Node* right);

    // Get the index of a child in its parent
    int getChildIndex(InternalNode* parent, Node* child);

    // Borrow from left sibling (leaf)
    bool borrowFromLeftLeaf(LeafNode* leaf, LeafNode* left_sibling, InternalNode* parent, int parent_key_idx);

    // Borrow from right sibling (leaf)
    bool borrowFromRightLeaf(LeafNode* leaf, LeafNode* right_sibling, InternalNode* parent, int parent_key_idx);

    // Borrow from left sibling (internal)
    bool borrowFromLeftInternal(InternalNode* node, InternalNode* left_sibling, InternalNode* parent, int parent_key_idx);

    // Borrow from right sibling (internal)
    bool borrowFromRightInternal(InternalNode* node, InternalNode* right_sibling, InternalNode* parent, int parent_key_idx);

    // Merge with left sibling (leaf)
    void mergeWithLeftLeaf(LeafNode* leaf, LeafNode* left_sibling, InternalNode* parent, int parent_key_idx);

    // Merge with right sibling (leaf)
    void mergeWithRightLeaf(LeafNode* leaf, LeafNode* right_sibling, InternalNode* parent, int parent_key_idx);

    // Merge with left sibling (internal)
    void mergeWithLeftInternal(InternalNode* node, InternalNode* left_sibling, InternalNode* parent, int parent_key_idx);

    // Merge with right sibling (internal)
    void mergeWithRightInternal(InternalNode* node, InternalNode* right_sibling, InternalNode* parent, int parent_key_idx);

    // Handle underflow in a node
    void handleUnderflow(Node* node);
    
public:
    BPlusTree(int ord = 3) : root(nullptr), order(ord), count(0) {
        if (order < 3) order = 3; // Minimum order
    }

    ~BPlusTree() {
        delete root;
    }
    
    // Insert a data pointer
    void insert(Data* data);

    // Search for data by key
    Data* search(int key);

    std::vector<Data*> rangeSearch(int key_start, int key_end);

    bool remove(int key);
    
    // Print the tree structure
    void printTree();
    
    // Print all leaf data in order
    void printLeafData();

    inline int getOrder() const {
        return order;
    }

    inline bool isEmpty() const {
        return root == nullptr;
    }

    inline Node* getRoot() const {
        return root;
    }

    std::vector<int> getAllKeysInOrder();

    std::vector<Data*> getAllDataInOrder();
};

