#pragma once
#include <list>
#include <atomic> 

//if one producer and one consumer then it's thread safe
//It is not thread safe if mutiple producers or mutiple consumers
using namespace std;
template<class T>
class threadsafe_queue
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
		_node* next;
	};

	threadsafe_queue()
	{
		phead = ptail = new _node(T());	//add dummy node;
	}

	~threadsafe_queue()
	{
		_node* p = phead; 
		while(p)
		{
			_node* tmp = p;
			p = p->next;
			delete tmp;
		}
		phead = nullptr;
		ptail = nullptr;
	}

	bool de_queue(T& t)
	{
		//TODO:
	}

	void en_queue(const T& t)	//add new node into tail of list
	{
		ptail->next = new _node(t);

		ptail = ptail->next;
	}

	bool is_empty()
	{
		return phead->next == ptail;
	}

private:
	atomic<_node*> phead;
	atomic<_node*> ptail;
};

