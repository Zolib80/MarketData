#include <mutex>
#include <deque>

template <class T>
class Queue {
public:
    void push(const T& input);
    bool pop(T& output);

private:
    std::mutex m;
    std::deque<T> data;
};

template <class T>
void Queue<T>::push(const T& input) { 
    std::lock_guard<std::mutex> L(m);
    data.push_back(input);
}

template <class T>
bool Queue<T>::pop(T& output) {
    std::lock_guard<std::mutex> L(m);
    if (data.empty()) {
        return false;
    }
    output = data.front();
    data.pop_front();
    return true;
}
