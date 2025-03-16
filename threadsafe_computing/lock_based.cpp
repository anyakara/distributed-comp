// #include <iostream>
#include <thread>
#include <x86intrin.h>

#include <mutex>

// these preprocessor directives
// define upper bounds
#define INIT_PUSH 1000000
#define MAX_THREAD_NUM 100
#define MAX_VOLUME 10000000

// THIS IS THE IMPLEMENTATION WITH LOCK BASED DESIGN 

std::mutex global_lock;

// create a Database stack
class DBStack {
public:
    // constructor with head being null
	DBStack() :head(NULL) {}
	
    // push: create a new node 
	void push(int d) {
		Node *pv = new Node;
		
		pv->d = d; // pvar d is assigned d, pvar p is assigned the head
		pv->p = head;
		head = pv; // reassign pvar to head
	}

	int pop() {
        if (head == NULL) return 0; // if empty return 0
		
		int temp = head->d; // otherwise, hold temporary value 
		Node *pv = head; // assign pointer variable to head (as a temp pointer)
		head = head->p; // set head to head's next pointer (next value)
		
        // delete the pointer and set to temp
		delete pv; 
		return temp;
	}

	bool isEmpty() {
		bool empty = (this->head == NULL);
        return empty; // if the head is null then the data structure is NULL
	}

	void display();

private:
	struct Node {
		int d;
		Node *p;
	};
	
	Node *head;
};


void testStack (DBStack* toTest, const int volume, int threadNum)
{
	global_lock.lock(); // critical section begins
		for (int i = 0; i < volume; i++) {
			int randNum = rand() % volume;
			int pushOrPop = i%2;
			if (pushOrPop) {
				toTest->push(randNum);
			} else {
				toTest->pop();
			}
		}
	global_lock.unlock(); // critical section
}



int main (int argc, char** argv)
{        
    srand(time(NULL));
	DBStack toTest;
	int maxThreads = 0;
	
	if (argc > 1) { maxThreads = atoi(argv[1]); }
	else {
		printf("no arguments :( \n");
		return 0;
		 // maxThreads = MAX_THREAD_NUM;
	}
	
	std::thread thr[maxThreads];
	
	for (int i = 0; i < INIT_PUSH; i++) {
        int randVal = rand() % INIT_PUSH;
		toTest.push(randVal);
	}
	
	uint64_t tick = __rdtsc()/100000; // CPU timestamp / total time elapsed (provides how much time has ticked)
	
	for (int i = 0; i < maxThreads; i++) {
		thr[i] = std::thread(testStack, &toTest, MAX_VOLUME/maxThreads, i);
	}
	
	for (int i = 0; i < maxThreads; i++) {
		thr[i].join();
	}
	
	uint64_t tick2 = __rdtsc()/100000;
	printf("%d, %lu, \n", maxThreads, tick2 - tick);
	
	return 0;
}

