#include <iostream>
#include <vector>

const int M = 4; // Tamanho da árvore (ordem máxima)
const int L = M / 2; // Mínimo de chaves em cada nó não raiz

class Node {
public:
    bool is_leaf;
    int key_count;
    int keys[M];
    Node* children[M + 1];

    Node(bool is_leaf = false) : is_leaf(is_leaf), key_count(0) {
        for (int i = 0; i < M; i++) {
            keys[i] = 0;
            children[i] = nullptr;
        }
        children[M] = nullptr;
    }
};

class BPlusTree {
private:
    Node* root;

    Node* find_leaf(Node* node, int key) {
        int i = 0;
        while (i < node->key_count && key > node->keys[i])
            i++;

        if (node->is_leaf)
            return node;
        else
            return find_leaf(node->children[i], key);
    }

    void split(Node* parent, int index) {
        Node* child = parent->children[index];
        Node* new_child = new Node(child->is_leaf);

        for (int i = 0; i < L; i++) {
            new_child->keys[i] = child->keys[i + L];
            child->keys[i + L] = 0;
        }

        if (!child->is_leaf) {
            for (int i = 0; i < L + 1; i++) {
                new_child->children[i] = child->children[i + L];
                child->children[i + L] = nullptr;
            }
        }

        child->key_count = L;
        new_child->key_count = L;

        for (int i = parent->key_count; i > index; i--) {
            parent->children[i + 1] = parent->children[i];
        }

        parent->children[index + 1] = new_child;

        for (int i = parent->key_count - 1; i >= index; i--) {
            parent->keys[i + 1] = parent->keys[i];
        }

        parent->keys[index] = child->keys[L];
        child->keys[L] = 0;
        parent->key_count++;
    }

    void insert_non_full(Node* node, int key) {
        int i = node->key_count - 1;

        if (node->is_leaf) {
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;
            node->key_count++;
        }
        else {
            while (i >= 0 && key < node->keys[i])
                i--;

            i++;
            if (node->children[i]->key_count == M) {
                split(node, i);
                if (key > node->keys[i])
                    i++;
            }
            insert_non_full(node->children[i], key);
        }
    }

    // Função auxiliar para buscar e excluir uma chave em um nó folha
    bool delete_leaf_key(Node* node, int key) {
        int i = 0;
        while (i < node->key_count && key > node->keys[i])
            i++;

        if (i == node->key_count || node->keys[i] != key)
            return false;

        for (int j = i; j < node->key_count - 1; j++) {
            node->keys[j] = node->keys[j + 1];
        }

        node->keys[node->key_count - 1] = 0;
        node->key_count--;
        return true;
    }

    // Função auxiliar para rearranjar as chaves em um nó não folha após a exclusão
    void rearrange_non_leaf(Node* node, int index) {
        Node* left_child = node->children[index];
        Node* right_child = node->children[index + 1];

        left_child->keys[L] = node->keys[index];
        for (int i = index; i < node->key_count - 1; i++) {
            node->keys[i] = node->keys[i + 1];
            node->children[i + 1] = node->children[i + 2];
        }

        right_child->key_count--;
        for (int i = right_child->key_count; i >= 0; i--) {
            right_child->keys[i + 1] = right_child->keys[i];
        }
        right_child->keys[0] = 0;
        right_child->children[right_child->key_count + 1] = right_child->children[right_child->key_count];
        right_child->children[right_child->key_count] = nullptr;
    }

    // Função auxiliar para lidar com o caso de exclusão em um nó não folha
    void delete_internal(Node* node, int key) {
        int i = 0;
        while (i < node->key_count && key > node->keys[i])
            i++;

        if (node->is_leaf) {
            delete_leaf_key(node, key);
        }
        else {
            Node* child = node->children[i];

            if (child->key_count == L) {
                if (i > 0 && node->children[i - 1]->key_count > L) {
                    Node* left_sibling = node->children[i - 1];
                    for (int j = child->key_count; j > 0; j--) {
                        child->keys[j] = child->keys[j - 1];
                    }
                    child->keys[0] = node->keys[i - 1];
                    node->keys[i - 1] = left_sibling->keys[left_sibling->key_count - 1];
                    left_sibling->keys[left_sibling->key_count - 1] = 0;
                    left_sibling->key_count--;
                    child->key_count++;
                }
                else if (i < node->key_count && node->children[i + 1]->key_count > L) {
                    Node* right_sibling = node->children[i + 1];
                    child->keys[child->key_count] = node->keys[i];
                    node->keys[i] = right_sibling->keys[0];
                    for (int j = 0; j < right_sibling->key_count - 1; j++) {
                        right_sibling->keys[j] = right_sibling->keys[j + 1];
                    }
                    right_sibling->keys[right_sibling->key_count - 1] = 0;
                    right_sibling->key_count--;
                    child->key_count++;
                }
                else if (i > 0) {
                    rearrange_non_leaf(node, i - 1);
                }
                else {
                    rearrange_non_leaf(node, i);
                    i++;
                }
            }

            delete_internal(node->children[i], key);
        }
    }

    Node* search_node(Node* node, int key) {
        if (!node)
            return nullptr;

        int i = 0;
        while (i < node->key_count && key > node->keys[i])
            i++;

        if (i < node->key_count && node->keys[i] == key) {
            // Chave encontrada, retornar o nó atual
            return node;
        }
        else if (node->is_leaf) {
            // Nó folha e chave não encontrada
            return nullptr;
        }
        else {
            // Nó não folha, buscar no próximo nível
            return search_node(node->children[i], key);
        }
    }

public:
    BPlusTree() {
        root = nullptr;
    }

    Node* search(int key) {
        return search_node(root, key);
    }

    void insert(int key) {
        if (root == nullptr) {
            root = new Node(true);
            root->keys[0] = key;
            root->key_count++;
        }
        else {
            if (root->key_count == M) {
                Node* new_root = new Node(false);
                new_root->children[0] = root;
                split(new_root, 0);
                root = new_root;
            }
            insert_non_full(root, key);
        }
    }

    void deleteNode(int key) {
        if (!root)
            return;

        delete_internal(root, key);

        if (root->key_count == 0) {
            Node* new_root = root->is_leaf ? nullptr : root->children[0];
            delete root;
            root = new_root;
        }
    }

    void print() {
        print_node(root);
    }

    void print_node(Node* node) {
        if (node == nullptr)
            return;

        for (int i = 0; i < node->key_count; i++) {
            printf("%d ", node->keys[i]);
        }
        printf("\n");

        if (!node->is_leaf) {
            for (int i = 0; i < node->key_count + 1; i++) {
                print_node(node->children[i]);
            }
        }
    }
};

int main() {
    BPlusTree tree;

    tree.insert(3);
    tree.insert(5);
    tree.insert(20);
    tree.insert(35);
    tree.insert(88);
    tree.insert(150);
    tree.insert(217);
    tree.insert(72);

    tree.print();

    tree.deleteNode(150);
    tree.deleteNode(88);
    
    printf("\n");

    printf("\nAfter deletion:\n");
    tree.print();

    printf("\n");

    int target_number = 72;
    Node* found_node = tree.search(target_number);
    if (found_node) {
        printf("Chave %d encontrada!\n", target_number);
    } else {
        printf("Chave %d nao encontrada!\n", target_number);
    }

    int targer_number2 = 150;
    found_node = tree.search(targer_number2);
    if (found_node) {
        printf("Chave %d encontrada!\n", targer_number2);
    } else {
        printf("Chave %d nao encontrada!\n", targer_number2);
    }

    return 0;
}
