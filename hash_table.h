#include <functional>
#include <stdexcept>

template <class KeyType, class ValueType>
struct Node {
    KeyType key_;
    ValueType value_;
    Node* next_;

    Node(KeyType key, ValueType value) {
        key_ = key;
        value_ = value;
        next_ = nullptr;
    }
};

template <class KeyType, class ValueType, class Func = std::hash<KeyType>>
class HashTable {
public:
    HashTable() {
        table_ = new Node<KeyType, ValueType>*[capacity_]();
    }

    explicit HashTable(Func hasher) {
        hasher_ = hasher;
        table_ = new Node<KeyType, ValueType>*[capacity_]();
    }

    HashTable(size_t capacity, double load_factor, Func hasher = Func{}) {
        capacity_ = capacity;
        if (load_factor > 0.0 && load_factor <= 1.0) {
            load_factor_ = load_factor;
        } else {
            load_factor_ = 0.5;
        }
        hasher_ = hasher;
        table_ = new Node<KeyType, ValueType>*[capacity_]();
    }

    ~HashTable() {
        for (int i = 0; i < capacity_; ++i) {
            Node<KeyType, ValueType>* cur_node = table_[i];
            while (cur_node != nullptr) {
                auto prev = cur_node;
                cur_node = cur_node->next_;
                delete prev;
            }
        }
        delete[] table_;
    }

    void insert(KeyType key, ValueType value) {
        size_t hash = hasher_(key) % capacity_;
        if (table_[hash] == nullptr) {
            table_[hash] = new Node<KeyType, ValueType>(key, value);
            ++size_;
        } else {
            Node<KeyType, ValueType>* cur_node = table_[hash];
            Node<KeyType, ValueType>* prev = nullptr;
            while (cur_node != nullptr && cur_node->key_ != key) {
                prev = cur_node;
                cur_node = cur_node->next_;
            }
            if (cur_node == nullptr) {
                prev->next_ = new Node<KeyType, ValueType>(key, value);
                ++size_;
            } else {
                cur_node->value_ = value;
            }
        }
        if (static_cast<double>(size_) / capacity_ > load_factor_) {
            rehash(capacity_ * 2);
        }
    }

    ValueType* find(KeyType key) {
        auto node = findNode(key);
        if (node == nullptr) {
            return nullptr;
        } else {
            return &node->value_;
        }
    }

    void erase(KeyType key) {
        size_t hash = hasher_(key) % capacity_;
        Node<KeyType, ValueType>* cur_node = table_[hash];
        Node<KeyType, ValueType>* prev = nullptr;
        while (cur_node != nullptr && cur_node->key_ != key) {
            prev = cur_node;
            cur_node = cur_node->next_;
        }
        if (cur_node == nullptr) {
            return;
        } else {
            if (prev == nullptr) {
                table_[hash] = cur_node->next_;
            } else {
                prev->next_ = cur_node->next_;
            }
            --size_;
            delete cur_node;
        }
    }

    Node<KeyType, ValueType>& operator[](uint64_t index) {
        if (index >= capacity_) {
            throw std::out_of_range("Hashcode is out of range");
        }
        if (table_[index] == nullptr) {
            throw std::runtime_error("Record with given hashcode is empty");
        }
        return *table_[index];
    }

    Node<KeyType, ValueType> at(uint64_t index) {
        return this->operator[](index);
    }

    size_t size() const {
        return size_;
    }
    size_t capacity() const {
        return capacity_;
    }

private:
    Node<KeyType, ValueType>* findNode(KeyType key) {
        size_t hash = hasher_(key) % capacity_;
        Node<KeyType, ValueType>* cur_node = table_[hash];
        while (cur_node != nullptr && cur_node->key_ != key) {
            cur_node = cur_node->next_;
        }
        if (cur_node == nullptr) {
            return nullptr;
        } else {
            return cur_node;
        }
    }

    void insertNode(Node<KeyType, ValueType>* node) {
        node->next_ = nullptr;
        size_t hash = hasher_(node->key_) % capacity_;
        if (table_[hash] == nullptr) {
            table_[hash] = node;
            return;
        }
        Node<KeyType, ValueType>* cur_node = table_[hash];
        Node<KeyType, ValueType>* prev = nullptr;
        while (cur_node != nullptr) {
            prev = cur_node;
            cur_node = cur_node->next_;
        }
        prev->next_ = node;
    }

    void rehash(size_t new_capacity) {
        auto prev_table = table_;
        size_t prev_capacity = capacity_;
        table_ = new Node<KeyType, ValueType>*[new_capacity]();
        capacity_ = new_capacity;
        for (int i = 0; i < prev_capacity; ++i) {
            Node<KeyType, ValueType>* cur_node = prev_table[i];
            while (cur_node != nullptr) {
                auto cur_next = cur_node->next_;
                this->insertNode(cur_node);
                cur_node = cur_next;
            }
        }
        delete[] prev_table;
    }

    Node<KeyType, ValueType>** table_;
    Func hasher_ = Func{};
    double load_factor_ = 0.5;
    size_t size_ = 0;
    size_t capacity_ = 100;
};
