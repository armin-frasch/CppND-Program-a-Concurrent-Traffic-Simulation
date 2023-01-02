#include "TrafficLight.h"
#include <chrono>
#include <iostream>
#include <random>

using namespace std::chrono_literals;

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  // _condition.wait() to wait for and receive new messages and pull them from
  // the queue using move semantics. The received object should then be returned
  // by the receive function.
  std::unique_lock<std::mutex> uniqueLock(_mtx);
  _cond.wait(uniqueLock, [this]() { return !_queue.empty(); });

  T queueElement = std::move(_queue.back());
  _queue.pop_back();

  return queueElement;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // FP.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex> as well as _condition.notify_one() to add a new
  // message to the queue and afterwards send a notification.
  std::lock_guard<std::mutex> lockguard(_mtx);
  _queue.push_back(std::move(msg));
  _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::kRED; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.
  while (true) {
    if (_queue.receive() == TrafficLightPhase::kGREEN) {
        return;
    }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be started
  // in a thread when the public method „simulate“ is called. To do this, use
  // the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics. The cycle duration should be a random value between 4 and 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.
  std::random_device rd;
  std::mt19937 gen(rd());

  while (true) {
    // based on
    // https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
    // and https://en.cppreference.com/w/cpp/numeric/random
    std::uniform_int_distribution<> distr(4, 6);
    std::this_thread::sleep_for(static_cast<std::chrono::seconds>(distr(gen)));

    TrafficLightPhase newPhase;
    if (_currentPhase == TrafficLightPhase::kRED) {
      _currentPhase = TrafficLightPhase::kGREEN;
      newPhase = TrafficLightPhase::kGREEN;
    } else {
      _currentPhase = TrafficLightPhase::kRED;
      newPhase = TrafficLightPhase::kRED;
    }
    _queue.send(std::move(newPhase));
  }
}
