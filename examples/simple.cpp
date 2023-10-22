#include "com.hpp"
#include <iostream>
#include <thread>

int main() {
  auto [sender, receiver] = mpsc::new_channel<int>();

  int value = 10;

  auto t = std::thread([&] {
    auto val = receiver.recv();
    std::cout << "Received value: " << (*val) << std::endl;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::cout << "Sending value: " << value << std::endl;
  sender.send(std::move(value));

  t.join();
}