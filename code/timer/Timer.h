#pragma once

#include <queue>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <cassert>
#include <chrono>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }

    void adjust(int id, int new_expires);
    void add(int id, int timeOut, const TimeoutCallBack& cb);
    void do_work(int id);
    void clear();
    void tick();
    void pop();
    int get_next_tick();

private:
    void del(std::size_t i);
    void sift_up(std::size_t i);
    bool sift_down(std::size_t index, std::size_t n);
    void swap_node(std::size_t i, std::size_t j);

    std::vector<TimerNode> heap_;
    std::unordered_map<int, std::size_t> ref_;
};