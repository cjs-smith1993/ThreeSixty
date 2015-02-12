#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <queue>
#include <mutex>

class ConcurrentQueue {

private:
	std::queue<int> queue;
	std::mutex mutex;

public:
	ConcurrentQueue() {}
	int size();
	void push(int data);
	int pop();
};

#endif // CONCURRENT_QUEUE_H
