#include "bPlusTree.h"


// Find the leaf node where key should be inserted
LeafNode* BPlusTree::findLeaf(int key) {
    if (!root) return nullptr;
    
    Node* curr = root;
    while (!curr->isLeaf()) {
        InternalNode* internal = static_cast<InternalNode*>(curr);
        int i = 0;
        while (i < curr->keys.size() && key >= curr->keys[i]) {
            i++;
        }
        curr = internal->children[i];
    }
    return static_cast<LeafNode*>(curr);
}

// Split a leaf node
void BPlusTree::splitLeaf(LeafNode* leaf) {
    int mid = (order + 1) / 2;
    
    LeafNode* new_leaf = new LeafNode();
    
    // Move half the keys and data pointers to new leaf
    new_leaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    new_leaf->data_ptrs.assign(leaf->data_ptrs.begin() + mid, leaf->data_ptrs.end());
    
    leaf->keys.resize(mid);
    leaf->data_ptrs.resize(mid);
    
    // Update linked list pointers
    new_leaf->next = leaf->next;
    new_leaf->prev = leaf;
    if (leaf->next) {
        leaf->next->prev = new_leaf;
    }
    leaf->next = new_leaf;
    
    // Insert into parent
    int key_to_push = new_leaf->keys[0];
    insertIntoParent(leaf, key_to_push, new_leaf);
}

// Split an internal node
void BPlusTree::splitInternal(InternalNode* internal) {
    int mid = order / 2;
    
    InternalNode* new_internal = new InternalNode();
    
    // Move keys (excluding middle key which goes up)
    new_internal->keys.assign(internal->keys.begin() + mid + 1, internal->keys.end());
    new_internal->children.assign(internal->children.begin() + mid + 1, internal->children.end());
    
    int key_to_push = internal->keys[mid];
    
    internal->keys.resize(mid);
    internal->children.resize(mid + 1);
    
    // Update parent pointers
    for (Node* child : new_internal->children) {
        child->parent = new_internal;
    }
    
    insertIntoParent(internal, key_to_push, new_internal);
}

// Insert key into parent after split
void BPlusTree::insertIntoParent(Node* left, int key, Node* right) {
    if (left == root) {
        // Create new root
        InternalNode* new_root = new InternalNode();
        new_root->keys.push_back(key);
        new_root->children.push_back(left);
        new_root->children.push_back(right);
        left->parent = new_root;
        right->parent = new_root;
        root = new_root;
        return;
    }
    
    InternalNode* parent = static_cast<InternalNode*>(left->parent);
    right->parent = parent;
    
    // Find position to insert
    int pos = 0;
    while (pos < parent->keys.size() && parent->keys[pos] < key) {
        pos++;
    }
    
    parent->keys.insert(parent->keys.begin() + pos, key);
    parent->children.insert(parent->children.begin() + pos + 1, right);
    
    // Check if parent needs to be split
    if (parent->keys.size() >= order) {
        splitInternal(parent);
    }
}

// Get the index of a child in its parent
int BPlusTree::getChildIndex(InternalNode* parent, Node* child) {
    for (int i = 0; i < parent->children.size(); i++) {
        if (parent->children[i] == child) {
            return i;
        }
    }
    return -1;
}

// Borrow from left sibling (leaf)
bool BPlusTree::borrowFromLeftLeaf(LeafNode* leaf, LeafNode* left_sibling, InternalNode* parent, int parent_key_idx) {
    if (left_sibling->keys.size() <= (order - 1) / 2) {
        return false;
    }
    
    // Move last key and data from left sibling to beginning of leaf
    leaf->keys.insert(leaf->keys.begin(), left_sibling->keys.back());
    leaf->data_ptrs.insert(leaf->data_ptrs.begin(), left_sibling->data_ptrs.back());
    
    left_sibling->keys.pop_back();
    left_sibling->data_ptrs.pop_back();
    
    // Update parent key
    parent->keys[parent_key_idx] = leaf->keys[0];
    
    return true;
}

// Borrow from right sibling (leaf)
bool BPlusTree::borrowFromRightLeaf(LeafNode* leaf, LeafNode* right_sibling, InternalNode* parent, int parent_key_idx) {
    if (right_sibling->keys.size() <= (order - 1) / 2) {
        return false;
    }
    
    // Move first key and data from right sibling to end of leaf
    leaf->keys.push_back(right_sibling->keys[0]);
    leaf->data_ptrs.push_back(right_sibling->data_ptrs[0]);
    
    right_sibling->keys.erase(right_sibling->keys.begin());
    right_sibling->data_ptrs.erase(right_sibling->data_ptrs.begin());
    
    // Update parent key
    parent->keys[parent_key_idx] = right_sibling->keys[0];
    
    return true;
}

// Borrow from left sibling (internal)
bool BPlusTree::borrowFromLeftInternal(InternalNode* node, InternalNode* left_sibling, InternalNode* parent, int parent_key_idx) {
    if (left_sibling->keys.size() <= (order - 1) / 2) {
        return false;
    }
    
    // Move parent key down to node
    node->keys.insert(node->keys.begin(), parent->keys[parent_key_idx]);
    
    // Move last child from left sibling to node
    node->children.insert(node->children.begin(), left_sibling->children.back());
    left_sibling->children.back()->parent = node;
    left_sibling->children.pop_back();
    
    // Move last key from left sibling to parent
    parent->keys[parent_key_idx] = left_sibling->keys.back();
    left_sibling->keys.pop_back();
    
    return true;
}

// Borrow from right sibling (internal)
bool BPlusTree::borrowFromRightInternal(InternalNode* node, InternalNode* right_sibling, InternalNode* parent, int parent_key_idx) {
    if (right_sibling->keys.size() <= (order - 1) / 2) {
        return false;
    }
    
    // Move parent key down to node
    node->keys.push_back(parent->keys[parent_key_idx]);
    
    // Move first child from right sibling to node
    node->children.push_back(right_sibling->children[0]);
    right_sibling->children[0]->parent = node;
    right_sibling->children.erase(right_sibling->children.begin());
    
    // Move first key from right sibling to parent
    parent->keys[parent_key_idx] = right_sibling->keys[0];
    right_sibling->keys.erase(right_sibling->keys.begin());
    
    return true;
}

// Merge with left sibling (leaf)
void BPlusTree::mergeWithLeftLeaf(LeafNode* leaf, LeafNode* left_sibling, InternalNode* parent, int parent_key_idx) {
    // Move all keys and data from leaf to left sibling
    left_sibling->keys.insert(left_sibling->keys.end(), leaf->keys.begin(), leaf->keys.end());
    left_sibling->data_ptrs.insert(left_sibling->data_ptrs.end(), leaf->data_ptrs.begin(), leaf->data_ptrs.end());
    
    // Update linked list
    left_sibling->next = leaf->next;
    if (leaf->next) {
        leaf->next->prev = left_sibling;
    }
    
    // Remove key from parent
    parent->keys.erase(parent->keys.begin() + parent_key_idx);
    parent->children.erase(parent->children.begin() + parent_key_idx + 1);
    
    // Delete the empty leaf
    leaf->next = nullptr;
    leaf->prev = nullptr;
    delete leaf;
    
    // Handle parent underflow
    handleUnderflow(parent);
}

// Merge with right sibling (leaf)
void BPlusTree::mergeWithRightLeaf(LeafNode* leaf, LeafNode* right_sibling, InternalNode* parent, int parent_key_idx) {
    // Move all keys and data from right sibling to leaf
    leaf->keys.insert(leaf->keys.end(), right_sibling->keys.begin(), right_sibling->keys.end());
    leaf->data_ptrs.insert(leaf->data_ptrs.end(), right_sibling->data_ptrs.begin(), right_sibling->data_ptrs.end());
    
    // Update linked list
    leaf->next = right_sibling->next;
    if (right_sibling->next) {
        right_sibling->next->prev = leaf;
    }
    
    // Remove key from parent
    parent->keys.erase(parent->keys.begin() + parent_key_idx);
    parent->children.erase(parent->children.begin() + parent_key_idx + 1);
    
    // Delete the empty right sibling
    right_sibling->next = nullptr;
    right_sibling->prev = nullptr;
    delete right_sibling;
    
    // Handle parent underflow
    handleUnderflow(parent);
}

// Merge with left sibling (internal)
void BPlusTree::mergeWithLeftInternal(InternalNode* node, InternalNode* left_sibling, InternalNode* parent, int parent_key_idx) {
    // Add parent key to left sibling
    left_sibling->keys.push_back(parent->keys[parent_key_idx]);
    
    // Move all keys and children from node to left sibling
    left_sibling->keys.insert(left_sibling->keys.end(), node->keys.begin(), node->keys.end());
    left_sibling->children.insert(left_sibling->children.end(), node->children.begin(), node->children.end());
    
    // Update parent pointers
    for (Node* child : node->children) {
        child->parent = left_sibling;
    }
    
    // Clear node's children to prevent double deletion
    node->children.clear();
    
    // Remove key from parent
    parent->keys.erase(parent->keys.begin() + parent_key_idx);
    parent->children.erase(parent->children.begin() + parent_key_idx + 1);
    
    delete node;
    
    // Handle parent underflow
    handleUnderflow(parent);
}

// Merge with right sibling (internal)
void BPlusTree::mergeWithRightInternal(InternalNode* node, InternalNode* right_sibling, InternalNode* parent, int parent_key_idx) {
    // Add parent key to node
    node->keys.push_back(parent->keys[parent_key_idx]);
    
    // Move all keys and children from right sibling to node
    node->keys.insert(node->keys.end(), right_sibling->keys.begin(), right_sibling->keys.end());
    node->children.insert(node->children.end(), right_sibling->children.begin(), right_sibling->children.end());
    
    // Update parent pointers
    for (Node* child : right_sibling->children) {
        child->parent = node;
    }
    
    // Clear right sibling's children to prevent double deletion
    right_sibling->children.clear();
    
    // Remove key from parent
    parent->keys.erase(parent->keys.begin() + parent_key_idx);
    parent->children.erase(parent->children.begin() + parent_key_idx + 1);
    
    delete right_sibling;
    
    // Handle parent underflow
    handleUnderflow(parent);
    }

// Handle underflow in a node
void BPlusTree::handleUnderflow(Node* node) {
    // Root has special handling
    if (node == root) {
        if (root->keys.empty() && !root->isLeaf()) {
            InternalNode* old_root = static_cast<InternalNode*>(root);
            if (!old_root->children.empty()) {
                root = old_root->children[0];
                root->parent = nullptr;
                old_root->children.clear();
                delete old_root;
            }
        }
        return;
    }
    
    int min_keys = (order - 1) / 2;
    if (node->keys.size() >= min_keys) {
        return; // No underflow
    }
    
    InternalNode* parent = static_cast<InternalNode*>(node->parent);
    int node_idx = getChildIndex(parent, node);
    
    if (node->isLeaf()) {
        LeafNode* leaf = static_cast<LeafNode*>(node);
        
        // Try to borrow from left sibling
        if (node_idx > 0) {
            LeafNode* left_sibling = static_cast<LeafNode*>(parent->children[node_idx - 1]);
            if (borrowFromLeftLeaf(leaf, left_sibling, parent, node_idx - 1)) {
                return;
            }
        }
        
        // Try to borrow from right sibling
        if (node_idx < parent->children.size() - 1) {
            LeafNode* right_sibling = static_cast<LeafNode*>(parent->children[node_idx + 1]);
            if (borrowFromRightLeaf(leaf, right_sibling, parent, node_idx)) {
                return;
            }
        }
        
        // Merge with left sibling
        if (node_idx > 0) {
            LeafNode* left_sibling = static_cast<LeafNode*>(parent->children[node_idx - 1]);
            mergeWithLeftLeaf(leaf, left_sibling, parent, node_idx - 1);
        }
        // Merge with right sibling
        else {
            LeafNode* right_sibling = static_cast<LeafNode*>(parent->children[node_idx + 1]);
            mergeWithRightLeaf(leaf, right_sibling, parent, node_idx);
        }
    } else {
        InternalNode* internal = static_cast<InternalNode*>(node);
        
        // Try to borrow from left sibling
        if (node_idx > 0) {
            InternalNode* left_sibling = static_cast<InternalNode*>(parent->children[node_idx - 1]);
            if (borrowFromLeftInternal(internal, left_sibling, parent, node_idx - 1)) {
                return;
            }
        }
        
        // Try to borrow from right sibling
        if (node_idx < parent->children.size() - 1) {
            InternalNode* right_sibling = static_cast<InternalNode*>(parent->children[node_idx + 1]);
            if (borrowFromRightInternal(internal, right_sibling, parent, node_idx)) {
                return;
            }
        }
        
        // Merge with left sibling
        if (node_idx > 0) {
            InternalNode* left_sibling = static_cast<InternalNode*>(parent->children[node_idx - 1]);
            mergeWithLeftInternal(internal, left_sibling, parent, node_idx - 1);
        }
        // Merge with right sibling
        else {
            InternalNode* right_sibling = static_cast<InternalNode*>(parent->children[node_idx + 1]);
            mergeWithRightInternal(internal, right_sibling, parent, node_idx);
        }
    }
}

// Insert a data pointer
void BPlusTree::insert(int key, Data* data) {
    count += 1;
    
    if (!root) {
        root = new LeafNode();
        LeafNode* leaf = static_cast<LeafNode*>(root);
        leaf->keys.push_back(key);
        leaf->data_ptrs.push_back(data);
        return;
    }
    
    LeafNode* leaf = findLeaf(key);
    
    // Find position to insert
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) {
        pos++;
    }
    
    leaf->keys.insert(leaf->keys.begin() + pos, key);
    leaf->data_ptrs.insert(leaf->data_ptrs.begin() + pos, data);
    
    // Check if leaf needs to be split
    if (leaf->keys.size() >= order) {
        splitLeaf(leaf);
    }
}

// Search for data by key
Data* BPlusTree::search(int key) {
    if (!root) return nullptr;
    
    LeafNode* leaf = findLeaf(key);
    
    for (int i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == key) {
            return leaf->data_ptrs[i];
        }
    }
    return nullptr;
}

std::vector<Data*> BPlusTree::rangeSearch(int key_start, int key_end) {
    std::vector<Data*> result;
    if (!root) return result;
    
    LeafNode* leaf = findLeaf(key_start);
    
    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); i++) {
            if (leaf->keys[i] >= key_start && leaf->keys[i] <= key_end) {
                result.push_back(leaf->data_ptrs[i]);
            } else if (leaf->keys[i] > key_end) {
                return result;
            }
        }
        leaf = leaf->next;
    }
    return result;
}

// Delete a key from the tree
bool BPlusTree::remove(int key) {
    if (!root) return false;
    
    LeafNode* leaf = findLeaf(key);
    
    // Find the key in the leaf
    int pos = -1;
    for (int i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == key) {
            pos = i;
            break;
        }
    }
    
    if (pos == -1) {
        return false; // Key not found
    }
    
    // Remove key and data pointer
    leaf->keys.erase(leaf->keys.begin() + pos);
    leaf->data_ptrs.erase(leaf->data_ptrs.begin() + pos);
    count -= 1;
    if (count == 0) {
        delete root;
        root = nullptr;
        return true;
    }
    
    // Handle underflow
    handleUnderflow(leaf);
    
    return true;
}

// Print the tree structure
void BPlusTree::printTree() {
    if (!root) {
        std::cout << "Empty tree\n";
        return;
    }
    
    std::vector<Node*> level;
    level.push_back(root);
    
    while (!level.empty()) {
        std::vector<Node*> next_level;
        
        for (Node* node : level) {
            std::cout << "[";
            for (int i = 0; i < node->keys.size(); i++) {
                std::cout << node->keys[i];
                if (i < node->keys.size() - 1) std::cout << ",";
            }
            std::cout << "] ";
            
            if (!node->isLeaf()) {
                InternalNode* internal = static_cast<InternalNode*>(node);
                for (Node* child : internal->children) {
                    next_level.push_back(child);
                }
            }
        }
        std::cout << "\n";
        level = next_level;
    }
}

// Print all leaf data in order
void BPlusTree::printLeafData() {
    if (!root) {
        std::cout << "Empty tree\n";
        return;
    }
    
    // Find leftmost leaf
    Node* curr = root;
    while (!curr->isLeaf()) {
        InternalNode* internal = static_cast<InternalNode*>(curr);
        curr = internal->children[0];
    }
    
    LeafNode* leaf = static_cast<LeafNode*>(curr);
    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); i++) {
            std::cout << "Key: " << leaf->keys[i] 
                        << ", Data: " << leaf->data_ptrs[i]->getData() << "\n";
        }
        leaf = leaf->next;
    }
}

std::vector<int> BPlusTree::getAllKeysInOrder() {
    std::vector<int> keys_in_order;
    if (!root) return keys_in_order;
    
    // Find leftmost leaf
    Node* curr = root;
    while (!curr->isLeaf()) {
        InternalNode* internal = static_cast<InternalNode*>(curr);
        curr = internal->children[0];
    }
    
    LeafNode* leaf = static_cast<LeafNode*>(curr);
    while (leaf) {
        for (int key : leaf->keys) {
            keys_in_order.push_back(key);
        }
        leaf = leaf->next;
    }
    return keys_in_order;
}

std::vector<Data*> BPlusTree::getAllDataInOrder() {
    std::vector<Data*> data_in_order;
    if (!root) return data_in_order;
    
    // Find leftmost leaf
    Node* curr = root;
    while (!curr->isLeaf()) {
        InternalNode* internal = static_cast<InternalNode*>(curr);
        curr = internal->children[0];
    }
    
    LeafNode* leaf = static_cast<LeafNode*>(curr);
    while (leaf) {
        for (Data* data_ptr : leaf->data_ptrs) {
            data_in_order.push_back(data_ptr);
        }
        leaf = leaf->next;
    }
    return data_in_order;
}

// // Example usage
// int main() {
//     BPlusTree tree(3); // Order 3
    
//     // Create and insert data
//     Data* d1 = new Data(10, "Record 10");
//     Data* d2 = new Data(20, "Record 20");
//     Data* d3 = new Data(5, "Record 5");
//     Data* d4 = new Data(15, "Record 15");
//     Data* d5 = new Data(25, "Record 25");
//     Data* d6 = new Data(30, "Record 30");
//     Data* d7 = new Data(35, "Record 35");
    
//     tree.insert(d1);
//     tree.insert(d2);
//     tree.insert(d3);
//     tree.insert(d4);
//     tree.insert(d5);
//     tree.insert(d6);
//     tree.insert(d7);
    
//     std::cout << "B+ Tree Structure:\n";
//     tree.printTree();
    
//     std::cout << "\nLeaf Data (in order):\n";
//     tree.printLeafData();
    
//     std::cout << "\nSearch for key 15:\n";
//     Data* found = tree.search(15);
//     if (found) {
//         std::cout << "Found: " << found->getData() << "\n";
//     } else {
//         std::cout << "Not found\n";
//     }
    
//     std::cout << "\nSearch for key 100:\n";
//     found = tree.search(100);
//     if (found) {
//         std::cout << "Found: " << found->getData() << "\n";
//     } else {
//         std::cout << "Not found\n";
//     }
    
//     // Note: In a real application, you'd need to manage Data object deletion
//     delete d1; delete d2; delete d3; delete d4;
//     delete d5; delete d6; delete d7;
    
//     return 0;
// }