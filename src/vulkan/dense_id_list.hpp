#pragma once

#include <vector>
#include <queue>

template <typename T>
class ref_count;


template <typename T>
class ref {
private:
    ref_count<T>* ref_count_server;

public:
    ref(ref_count<T>* input) {
        this->ref_count_server = input;
        this->ref_count_server->add_ref();
    }
    ~ref() {
        this->ref_count_server->rem_ref();
    }
    inline T& get() {
        return this->ref_count_server->data;
    }
    int num_references() const {
        return this->ref_count_server->num_references();
    }
};

template <typename T>
class ref_count {
friend class ref;
private:
    T data;
    int references = 0;

public:
    inline ref<T> get_ref() {
        return ref<T>(this);
    }
    void add_ref() {
        this->references += 1;
    }
    void rem_ref() {
        this->references -= 1;
    }
    int num_references() const {
        return this->references;
    }
};

template <typename T>
class dense_id_list {
private:
    std::queue<uint32_t> empty_ids;

public:
    std::vector<ref_count<T>> data;

    uint32_t add(T element) {
        if (empty_ids.empty()) {
            // @TODO: better vector resizing
            ref_count<T> new_entry;
            new_entry.get() = element;
            data.push_back(new_entry);
            return (data.size - 1);
        } else {
            auto index = empty_ids.front();
            ref_count<T> new_entry;
            new_entry.get() = element;
            data[index] = new_entry;
            empty_ids.pop();
            return index;
        }
    }

    bool rem(uint32_t index) {
        if (data.size > index) {
            if (data[index].num_references() <= 0) {
                this->empty_ids.push(index);
                return true;
            }
        }
        return false;
    }
};
