#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <data.h>

using namespace std;

/* ===================== B+ Tree ===================== */

template <typename T>
class BPlusTree {
private:
    struct Node {
        bool isLeaf;

        // Internal nodes
        vector<T> keys;
        vector<Node*> children;

        // Leaf nodes
        vector<pair<T, Data*>> entries;
        Node* next;

        Node(bool leaf = false)
            : isLeaf(leaf), next(nullptr) {}
    };

    Node* root;
    int t; // minimum degree

private:
    /* ---------- Core helpers ---------- */

    void splitChild(Node* parent, int index, Node* child);
    void insertNonFull(Node* node, T key, Data* value);

    void printTree(Node* node, int level) const;
    void destroy(Node* node);

public:
    explicit BPlusTree(int degree)
        : root(nullptr), t(degree) {}

    ~BPlusTree() { destroy(root); }

    void insert(T key, Data* value);
    Data* search(T key) const;
    vector<Data*> rangeQuery(T lower, T upper) const;

    void printTree() const { printTree(root, 0); }
};

/* ===================== Insert ===================== */

template <typename T>
void BPlusTree<T>::insert(T key, Data* value) {
    if (!root) {
        root = new Node(true);
        root->entries.push_back({key, value});
        return;
    }

    if (root->keys.size() == 2 * t - 1) {
        Node* newRoot = new Node(false);
        newRoot->children.push_back(root);
        splitChild(newRoot, 0, root);
        root = newRoot;
    }

    insertNonFull(root, key, value);
}

template <typename T>
void BPlusTree<T>::insertNonFull(Node* node, T key, Data* value) {
    if (node->isLeaf) {
        auto it = upper_bound(
            node->entries.begin(),
            node->entries.end(),
            key,
            [](const T& k, const pair<T, Data*>& e) {
                return k < e.first;
            });

        node->entries.insert(it, {key, value});
        return;
    }

    int i = 0;
    while (i < node->keys.size() && key >= node->keys[i]) {
        i++;
    }

    if (node->children[i]->keys.size() == 2 * t - 1) {
        splitChild(node, i, node->children[i]);
        if (key >= node->keys[i]) {
            i++;
        }
    }

    insertNonFull(node->children[i], key, value);
}

/* ===================== Split ===================== */

template <typename T>
void BPlusTree<T>::splitChild(Node* parent, int index, Node* child) {
    if (child->isLeaf) {
        Node* newLeaf = new Node(true);

        parent->children.insert(
            parent->children.begin() + index + 1, newLeaf);

        newLeaf->entries.assign(
            child->entries.begin() + t,
            child->entries.end());

        child->entries.resize(t);

        parent->keys.insert(
            parent->keys.begin() + index,
            newLeaf->entries.front().first);

        newLeaf->next = child->next;
        child->next = newLeaf;
        return;
    }

    Node* newInternal = new Node(false);

    parent->children.insert(
        parent->children.begin() + index + 1, newInternal);

    parent->keys.insert(
        parent->keys.begin() + index,
        child->keys[t - 1]);

    newInternal->keys.assign(
        child->keys.begin() + t,
        child->keys.end());

    child->keys.resize(t - 1);

    newInternal->children.assign(
        child->children.begin() + t,
        child->children.end());

    child->children.resize(t);
}

/* ===================== Search ===================== */

template <typename T>
Data* BPlusTree<T>::search(T key) const {
    Node* cur = root;
    if (!cur) return nullptr;

    while (!cur->isLeaf) {
        int i = 0;
        while (i < cur->keys.size() && key >= cur->keys[i]) {
            i++;
        }
        cur = cur->children[i];
    }

    for (const auto& [k, v] : cur->entries) {
        if (k == key) return v;
    }

    return nullptr;
}

/* ===================== Range Query ===================== */

template <typename T>
vector<Data*> BPlusTree<T>::rangeQuery(T lower, T upper) const {
    vector<Data*> result;
    if (!root) return result;

    Node* cur = root;

    while (!cur->isLeaf) {
        int i = 0;
        while (i < cur->keys.size() && lower >= cur->keys[i]) {
            i++;
        }
        cur = cur->children[i];
    }

    while (cur) {
        for (const auto& [k, v] : cur->entries) {
            if (k >= lower && k <= upper)
                result.push_back(v);
            else if (k > upper)
                return result;
        }
        cur = cur->next;
    }

    return result;
}

/* ===================== Utilities ===================== */

template <typename T>
void BPlusTree<T>::printTree(Node* node, int level) const {
    if (!node) return;

    for (int i = 0; i < level; i++) cout << "  ";

    if (node->isLeaf) {
        cout << "[Leaf] ";
        for (auto& e : node->entries) {
            cout << e.first << " ";
        }
    } else {
        cout << "[Internal] ";
        for (auto& k : node->keys) {
            cout << k << " ";
        }
    }
    cout << "\n";

    for (Node* child : node->children) {
        printTree(child, level + 1);
    }
}

template <typename T>
void BPlusTree<T>::destroy(Node* node) {
    if (!node) return;

    if (node->isLeaf) {
        for (auto& e : node->entries) {
            delete e.second;
        }
    }

    for (Node* c : node->children) {
        destroy(c);
    }

    delete node;
}

