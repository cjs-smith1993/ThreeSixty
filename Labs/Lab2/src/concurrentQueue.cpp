#include "concurrentQueue.h"

int ConcurrentQueue::size() {
	mutex.lock();
	int size = queue.size();
	mutex.unlock();
	return size;
}

void ConcurrentQueue::push(int data) {
	mutex.lock();
	queue.push(data);
	mutex.unlock();
}

int ConcurrentQueue::pop() {
	mutex.lock();
	int data = queue.front();
	queue.pop();
	mutex.unlock();
	return data;
}