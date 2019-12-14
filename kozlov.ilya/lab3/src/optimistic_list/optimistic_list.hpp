#include <logger/logger.h>
#include <min_max/min_value.h>
#include <min_max/max_value.h>

template<typename T>
OptimisticList<T>::OptimisticList()
{
  // TODO: handle error
  Logger::logDebug(tag, "constructing...");
  head = new Node<T>(MinValue::get<T>());
  head->next = new Node<T>(MaxValue::get<T>());
}

template<typename T>
OptimisticList<T>::~OptimisticList()
{
  Logger::logDebug(tag, "destructing...");
  Node<T>* curr = head;
  while (curr != nullptr)
  {
    Node<T>* prev = curr;
    curr = curr->next;
    delete prev;
  }
}

template<typename T>
bool OptimisticList<T>::add(T element)
{
  // TODO: try_lock
  Logger::logDebug(tag, "add(" + std::to_string(element) + ")");
  int key = std::hash<T>()(element);
  bool res = false;
  bool need_return = false;
  while (true)
  {
    Node<T>* prev = head;
    Node<T>* curr = prev->next;
    while (curr->key < key)
    {
      prev = curr;
      curr = curr->next;
    }
    prev->lock();
    curr->lock();
    if (validate(prev, curr))
    {
      need_return = true;
      if (curr->key == key)
      {
        res = false;
      }
      else
      {
        auto node = new Node<T>(element);
        node->next = curr;
        prev->next = node;
        res = true;
      }
    }
    prev->unlock();
    curr->unlock();
    if (need_return)
    {
      return res;
    }
  }
}

template<typename T>
bool OptimisticList<T>::remove(T element)
{
  // TODO: try_lock
  Logger::logDebug(tag, "remove(" + std::to_string(element) + ")");
  int key = std::hash<T>()(element);
  bool res = false;
  bool need_return = false;
  while (true)
  {
    Node<T>* prev = head;
    Node<T>* curr = prev->next;
    while (curr->key < key)
    {
      prev = curr;
      curr = curr->next;
    }
    prev->lock();
    curr->lock();
    if (validate(prev, curr))
    {
      need_return = true;
      if (curr->key == key)
      {
        prev->next = curr->next;
        delete curr;
        res = true;
      }
      else
      {
        res = false;
      }
    }
    prev->unlock();
    if (!res)
    {
      curr->unlock();
    }
    if (need_return)
    {
      return res;
    }
  }
}

template<typename T>
bool OptimisticList<T>::contains(T element) const
{
  // TODO: try_lock
  Logger::logDebug(tag, "contains(" + std::to_string(element) + ")");
  int key = std::hash<T>()(element);
  bool res = false;
  bool need_return = false;
  while (true)
  {
    Node<T>* prev = head;
    Node<T>* curr = prev->next;
    while (curr->key < key)
    {
      prev = curr;
      curr = curr->next;
    }
    prev->lock();
    curr->lock();
    if (validate(prev, curr))
    {
      need_return = true;
      res = curr->key == key;
    }
    prev->unlock();
    curr->unlock();
    if (need_return)
    {
      return res;
    }
  }
}

template<typename T>
bool OptimisticList<T>::validate(Node<T>* prev, Node<T>* curr) const
{
  //Logger::logDebug(tag, "validating...");
  Node<T>* node = head;
  while (node->key < prev->key)
  {
    node = node->next;
  }
  if (node == prev)
  {
    return node->next == curr;
  }
  return false;
}

template<typename T>
bool OptimisticList<T>::empty() const
{
  return *head == Node<T>(MinValue::get<T>()) && *(head->next) == Node<T>(MaxValue::get<T>());
}
