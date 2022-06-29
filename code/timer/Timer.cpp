#include "Timer.h"

void HeapTimer::sift_up(std::size_t i) {
    assert(i >= 0 && i < heap_.size());
    std::size_t j = (i - 1) / 2;
    while (j >= 0) {
        if (heap_[j] < heap_[i]) {
            break;
        }
        swap_node(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

void HeapTimer::swap_node(std::size_t i, std::size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

bool HeapTimer::sift_down(std::size_t index, std::size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    std::size_t i = index;
    std::size_t j = i * 2 + 1;
    while (j < n) {
        if (j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if (heap_[i] < heap_[j]) break;
        swap_node(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    std::size_t i;
    if (ref_.count(id) == 0) {
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        sift_up(i);
    } else {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if (!sift_down(i, heap_.size())) {
            sift_up(i);
        }
    }
}

void HeapTimer::do_work(int id) {
    if (heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    std::size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del(i);
}

void HeapTimer::del(std::size_t index) {
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    std::size_t i = index;
    std::size_t n = heap_.size() - 1;
    assert(i <= n);
    if (i < n) {
        swap_node(i, n);
        if (!sift_down(i, n)) {
            sift_up(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::adjust(int id, int timeout) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;
    sift_down(ref_[id], heap_.size());
}

void HeapTimer::tick() {
    if (heap_.empty()) {
        return;
    }
    while (!heap_.empty()) {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }
        node.cb();
        pop();
    }
}

void HeapTimer::pop() {
    assert(!heap_.empty());
    del(0);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

int HeapTimer::get_next_tick() {
    tick();
    int res = -1;
    if (!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if (res < 0) { res = 0; }
    }
    return res;
}