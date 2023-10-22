
#include <algorithm>
#include <atomic>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

namespace mpsc {

template <typename T> struct queue_node {
  std::atomic<queue_node *> next;
  std::unique_ptr<T> value;
};

template <typename T> class queue {
  std::atomic<queue_node<T> *> head;
  std::atomic<queue_node<T> *> tail;

public:
  queue<T>() {
    queue_node<T> *dummy =
        new queue_node<T>{nullptr, std::unique_ptr<T>(nullptr)};
    head.store(dummy);
    tail.store(dummy);
  }

  ~queue<T>() {
    queue_node<T> *curr = head.load();
    while (curr->next != nullptr) {
      queue_node<T> *next = curr->next.load();
      delete curr;
      curr = next;
    }

    if (curr != nullptr)
      delete curr;
  }

  void push(T &&value) {
    queue_node<T> *dummy = nullptr;

    queue_node<T> *const new_node =
        new queue_node<T>{nullptr, std::make_unique<T>(std::move(value))};

    queue_node<T> *t = tail.load();

    while (!t->next.compare_exchange_weak(dummy, new_node))
      t = tail.load();

    tail.store(new_node);
  }

  std::unique_ptr<T> pop() {
    queue_node<T> *h = head.load();

    while (h->next == nullptr) {
      h = head.load();
    }

    head.compare_exchange_strong(h, h->next);
    delete h;

    h = head.load();
    return std::move(h->value);
  }

  std::optional<std::unique_ptr<T>> try_pop() {
    queue_node<T> *h = head.load();

    if (h->next == nullptr) {
      return std::nullopt;
    }

    delete head.exchange(h->next);
    h = head.load();
    return std::optional<T>{std::move(h->value)};
  }

  bool empty() { return head.load()->next == nullptr; }
};

template <typename T> class sender {
  std::shared_ptr<queue<T>> m_queue;

public:
  sender<T>(std::shared_ptr<queue<T>> queue) : m_queue(queue) {}

  void send(T &&value) { m_queue->push(std::move(value)); }
};

template <typename T> class receiver {
  std::shared_ptr<queue<T>> m_queue;

public:
  receiver<T>(std::shared_ptr<queue<T>> queue) : m_queue(queue) {}

  receiver<T>(const receiver<T> &) = delete;
  receiver<T> &operator=(const receiver<T> &) = delete;

  std::unique_ptr<T> recv() { return m_queue->pop(); }

  std::optional<std::unique_ptr<T>> try_recv() { return m_queue->try_pop(); }
};

template <typename T> std::pair<sender<T>, receiver<T>> new_channel() {
  std::shared_ptr<queue<T>> queue = std::make_shared<mpsc::queue<T>>();
  return std::make_pair(queue, queue);
}

} // namespace mpsc
