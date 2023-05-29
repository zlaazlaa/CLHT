#include "../include/clht.h"

#include "iostream"
#define num_buckets 256;
using namespace std;

class LRUCache {
public:
    using key_type = int;
    using value_type = int;
    class Node {
    public:
        key_type key;
        value_type value;
        Node *tail;
        Node *head;
    };
    
    using hash_table_value_type = Node;
    explicit LRUCache(int capacity, clht_t* clht) {
        this->capacity = capacity;
        this->size = 0;
        this->clht = clht;
        head = new Node();
        tail = new Node();
        head->head = nullptr;
        head->tail = tail;
        tail->head = head;
        tail->tail = nullptr;
    }
    
    int get(int key) {
        hash_table_value_type* val = get_from_hash(&key);
        if (val != 0) {
            move_to_top(val);
            return val->value;
        }
        return -1;
    }

    void put(int key, int value) {
        hash_table_value_type* val = get_from_hash(&key);
        if (val != 0) {
            val->value = value;
            move_to_top(val);
        } else {
            if (size == capacity) {
                remove_from_hash(&tail->head->key);
                remove(tail->head);
            }
            Node *new_node = new Node();
            new_node->key = key;
            new_node->value = value;
            insert_to_top(new_node);
            put_to_hash(&key, new_node);
        }
    }

    void put_to_hash(key_type* key, hash_table_value_type* val) {
        clht_put(clht, (clht_addr_t)key, (clht_val_t)val) == 0;
    }

    hash_table_value_type* get_from_hash(key_type* key) {
        return (hash_table_value_type*)clht_get(clht->ht, (clht_addr_t)key);
    }

    clht_val_t remove_from_hash(key_type* key) {
        return clht_remove(clht, (clht_addr_t)key);
    }

    void init_gc(int id) {
        clht_gc_thread_init(clht, id);
    }

private:
    
    Node *head, *tail;
    int capacity;
    int size;
    clht_t *clht;

    void insert_to_top(Node *now) {
        now->head = head;
        now->tail = head->tail;
        head->tail->head = now;
        head->tail = now;
        size++;
    }

    void remove(Node *now) {
        if (now->head != nullptr) now->head->tail = now->tail;
        if (now->tail != nullptr) now->tail->head = now->head;
        delete now;
        size--;
    }

    void move_to_top(Node *now) {
        if (now->head != nullptr) now->head->tail = now->tail;
        if (now->tail != nullptr) now->tail->head = now->head;
        now->head = head;
        now->tail = head->tail;
        head->tail->head = now;
        head->tail = now;
    }
};

clht_t* clht = clht_create(256);

int main() {
    LRUCache* lRUCache = new LRUCache(2, clht);
    lRUCache->put(1, 1); // 缓存是 {1=1}
    lRUCache->put(2, 2); // 缓存是 {1=1, 2=2}
    cout << endl << lRUCache->get(1);    // 返回 1
    lRUCache->put(3, 3); // 该操作会使得关键字 2 作废，缓存是 {1=1, 3=3}
    cout << endl << lRUCache->get(2);    // 返回 -1 (未找到)
    lRUCache->put(4, 4); // 该操作会使得关键字 1 作废，缓存是 {4=4, 3=3}
    cout << endl << lRUCache->get(1);    // 返回 -1 (未找到)
    cout << endl << lRUCache->get(3);    // 返回 3
    cout << endl << lRUCache->get(4);    // 返回 4
}
/**
 * Your LRUCache object will be instantiated and called as such:
 * LRUCache* obj = new LRUCache(capacity);
 * int param_1 = obj->get(key);
 * obj->put(key,value);
 */