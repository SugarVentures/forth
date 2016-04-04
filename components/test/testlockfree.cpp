
#include <iostream>
#include <thread>

template<typename T>
class lock_free_stack
{
private:
	struct node
	{
		std::shared_ptr<T> data;
		node* next;

		node (T const& data_): data(std::make_shared<T>(data_))
		{

		}
	};
	std::atomic<node*> head;
public:
	void push(T const& data)
	{
		node* const new_node = new node(data);
		new_node->next = head.load();
		while (!head.compare_exchange_weak(new_node->next, new_node));
	}

	std::shared_ptr<T> pop()
	{
		node* old_head = head.load();
		while (old_head && !head.compare_exchange_weak(old_head, old_head->next));
		return old_head ? old_head->data : std::shared_ptr<T>();
	}
};

void thrPush(lock_free_stack<int> *stack, int i)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(3));

    lock_free_stack<int> *lp = stack; // thread-safe, even though the
                                  // shared use_count is incremented
    {	
    	lp->push(i);
    	printf("push an item\n");
    }

    
}

void thrPop(lock_free_stack<int> *stack)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2));
	lock_free_stack<int> *lp = stack;
    std::shared_ptr<int> out = lp->pop();
    printf("item: %d\n", *out);
}

int main()
{
	lock_free_stack<int> stack;
	size_t n = 100;
	std::thread t[100];
	for (int i = 0; i < n/2; i++)
	{
		t[i] = std::thread(thrPush, &stack, i);
	}

	for (int i = n/2; i < n; i++)
	{
		t[i] = std::thread(thrPop, &stack);
	}

	for (auto& th : t) th.join();
	return 0;
}