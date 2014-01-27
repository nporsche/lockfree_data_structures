#pragma once
#include <atomic> 

using namespace std;
template<class T>
class lockfree_queue
{
public:
	template<class T>
	struct _node
	{	
		_node(const T& t_arg)
			:t(t_arg),
			next(nullptr)
		{}

		T t;
		atomic<_node*> next;
	};

	typedef _node<T> node;

	lockfree_queue()
	{
		first = last = new node(T());	//add dummy node initially
	}

	~lockfree_queue()
	{
		node* p = first;
		while(p)
		{
			node* tmp = p;
			p = p->next;
			delete tmp;
		}
		first = nullptr;
		last = nullptr;
	}

	bool de_queue(T& t)
	{
		if(is_empty())
			return false;

		node* pfirst_snapshot = first;
		node* pfirst_snapshot_next = nullptr;
		do
		{
			if(pfirst_snapshot == last)	//empty 
				return false;

			pfirst_snapshot_next = pfirst_snapshot->next;
		}
		while(!first.compare_exchange_weak(pfirst_snapshot, pfirst_snapshot_next));

		t = pfirst_snapshot_next->t;

		delete pfirst_snapshot;

		return true;
	}

	void en_queue(const T& t)	//add new node into tail of list
	{
		node* pnew = new node(t);
		node* plast_next_snapshot = nullptr;
		
		while(!((node*)last)->next.compare_exchange_weak(plast_next_snapshot, pnew))
		{
			plast_next_snapshot = nullptr;
		}

		last = pnew;
	}

	bool is_empty()
	{
		return ((node*)first) == ((node*)last);
	}

private:
	atomic<node*> first;	//for consumer only, first position element has been consumed
	atomic<node*> last;		//for producer and consumer
};


//testing code
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <vector>         // std::vector
#include <string>
lockfree_queue<int> q;
atomic<bool> ready = false;
atomic<bool> quit = false;
void append(int val)
{
	while(!ready)
	{
		std::this_thread::yield();
	}
	// append an element to the list
	q.en_queue(val);
}

void consume(int id)
{
	while(!ready)
	{
		std::this_thread::yield();
	}
	int a;
	while(!quit)
	{
		if(q.de_queue(a))
		{
			printf("thread %d de_queue an element %d\n", id, a);
			//cout << "thread " << id << " de_queue an element " << a << endl; //is not threadsafe
		}
	}
}

int main()
{
	std::vector<std::thread> producer_threads;
	for (int i = 0; i < 10; ++i) 
		producer_threads.push_back(thread(append, i));

	std::vector<std::thread> consumer_threads;
	for (int i = 0; i < 10; ++i) 
		consumer_threads.push_back(thread(consume, i));

	ready = true;
	
	//quit = true;

	for (auto& th : consumer_threads) 
	{
		th.join();
	}
	return 0;
}
