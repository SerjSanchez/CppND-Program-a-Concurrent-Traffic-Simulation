#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include "TrafficLight.h"

using namespace std;

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    unique_lock<mutex> uniqueLock(_mutex);
    _condition.wait(uniqueLock, [this] { return !_queue.empty(); });
    T message = move(_queue.back());
    _queue.pop_back();

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    lock_guard<mutex> lockGuard(_mutex);
    _queue.clear();
    _queue.emplace_back(msg);
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        if (_messageQueue.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(thread(&TrafficLight::cycleThroughPhases, this));
 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    chrono::time_point<chrono::system_clock> update = chrono::system_clock::now();

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(4000, 6000);

    double cycle = distr(gen) / 1000;
    long timeSinceUpdate;

    while (true)
    {
        timeSinceUpdate = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - update).count();
        if (timeSinceUpdate >= cycle)
        {
            // toggle lights
            if (_currentPhase == TrafficLightPhase::green) {
                _currentPhase = TrafficLightPhase::red;
            } else {
                _currentPhase = TrafficLightPhase::green;
            }

            _messageQueue.send(std::move(_currentPhase));

            // reset
            update = chrono::system_clock::now();
            cycle = distr(gen) / 1000;
        }
        this_thread::sleep_for(chrono::milliseconds(1));
    }
}