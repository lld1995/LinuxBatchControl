#pragma once
#include <mutex>
#include <condition_variable>
#include <list>
#include <assert.h>
using namespace std;

template <typename T>
class BlockingQueue
{
private:
	
	BlockingQueue(const BlockingQueue& rhs);
	BlockingQueue& operator =(const BlockingQueue& rhs);
	mutable mutex _mutex;
	condition_variable _condvar;
	list<T> _queue;

public:
	BlockingQueue()
		: _mutex()
		  , _condvar()
		  , _queue()
	{
	}

	void Put(const T& task)
	{
		{
			lock_guard<mutex> lock(_mutex);
			_queue.push_back(task);
		}
		_condvar.notify_all();
	}

	T Take()
	{
		unique_lock<mutex> lock(_mutex);
		_condvar.wait(lock, [this]
			{
			   return !_queue.empty();
			});
		assert(!_queue.empty());
		T front(_queue.front());
		_queue.pop_front();
		return front;
	}

	size_t Size() const
	{
		lock_guard<mutex> lock(_mutex);
		return _queue.size();
	}
};

