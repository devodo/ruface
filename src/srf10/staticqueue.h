/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * Static Queue data structure
 */

#ifndef STATICQUEUE_H
#define STATICQUEUE_H

#ifdef ERROR
extern "C" {
#include "../lib/myprint.h"
}
#endif

#include "../lib/datatypes.h"

#define QUEUE_CAPACITY 5

template<class T>
class StaticQueue {
 public:
  StaticQueue();
  bool enqueue(const T&);
  bool dequeue();
  bool queueFront(const T&);
  bool getFront(T&) const;
  bool popFront(T &value);
  void clear();
  bool isEmpty() const { return empty; }
  bool isFull() const { return full; }
 private:
  tU8 back;
  tU8 front;
  bool full;
  bool empty;
  T queueArray[QUEUE_CAPACITY];
};

template<class T>
StaticQueue<T>::StaticQueue()
{
  back = 0;
  front = 0;
  full = false;
  empty = true;
}

template<class T>
bool StaticQueue<T>::enqueue(const T &value)
{
  if (!isFull()) {
    queueArray[back] = value;
    back = (back+1) % QUEUE_CAPACITY;
    full = (back == front);
    empty = false;
    return true;
  } else {
#ifdef ERROR
    printS("Error: enqueue - queue is full\n");
#endif
    return false;
  }
}

template<class T>
bool StaticQueue<T>::queueFront(const T &value)
{
  if (!isFull()) {
    if (front != 0) {
      front = front - 1;
    } else {
      front = QUEUE_CAPACITY - 1;
    }
    queueArray[front] = value;
    full = (back == front);
    empty = false;    
    return true;
  } else {
#ifdef ERROR
    printS("Error: queueFront - queue is full\n");
#endif
    return false;
  }
}

template<class T>
bool StaticQueue<T>::getFront(T &value) const
{
  if (!isEmpty()) {
    value = queueArray[front];
    return true;
  } else {
#ifdef ERROR
    printS("Error: getFront - queue is empty\n");
#endif
    return false;
  }
}

template<class T>
bool StaticQueue<T>::popFront(T &value)
{
  if (!isEmpty()) {
    value = queueArray[front];
    front = (front + 1) % QUEUE_CAPACITY;
    full = false;
    empty = (back == front);
    return true;
  } else {
#ifdef ERROR
    printS("Error: popFront - queue is empty\n");
#endif
    return false;
  }
}

template<class T>
bool StaticQueue<T>::dequeue()
{
  if (!isEmpty()) {
    front = (front + 1) % QUEUE_CAPACITY;
    full = false;
    empty = (back == front);
    return true;
  } else {
#ifdef ERROR
    printS("Error: dequeue - queue is empty\n");
#endif
    return false;
  }
}

template<class T>
void StaticQueue<T>::clear()
{
  back = 0;
  front = 0;
  full = false;
  empty = true;
}

#endif
