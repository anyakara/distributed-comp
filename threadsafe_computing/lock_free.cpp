// #include <iostream>
// #include <memory>

#include <thread>
#include <x86intrin.h>
#include <atomic>
#include <cstdio>

#define INIT_PUSH 1000000
#define MAX_THREAD_NUM 100
#define MAX_VOLUME 10000000


// THIS IS THE IMPLEMENTATION WITH LOCK FREE DESIGN
// USING COMPARE AND EXCHANGE WEAK


class DBStack {
private:
	struct Node {
		int d;
		Node *p;
        //Node(int v) : d(v), p(nullptr) {}
	};
    
    std::atomic<Node*> head;
	// Node *head;

public:
	DBStack() :head(NULL) {}

	void push(int d) {
		Node *pv = new Node;
        pv->d = d;
        Node *c_head;
        do {
            c_head = head.load(std::memory_order_relaxed);
            pv->p = c_head;
        } while (!head.compare_exchange_weak(c_head, pv, std::memory_order_release, std::memory_order_relaxed));
	}


	int pop() {
        Node *c_head;
        do {
            c_head = head.load(std::memory_order_relaxed);
            if (c_head == nullptr) { return 0; }
        } while (!head.compare_exchange_weak(c_head, c_head->p, std::memory_order_acquire, std::memory_order_relaxed));
        int tmp = c_head->d;
        delete c_head;
        return tmp;
	}


	bool isEmpty() {
		bool empty = (this->head == NULL);
        return empty;
	}


	void display();
};


void testStack(DBStack* toTest, const int volume, int threadNum) {
    for (int i = 0; i < volume; i++) {
        int randNum = rand() % volume;
        int pushOrPop = i%2;
        if (pushOrPop) {
            toTest->push(randNum);
        } else {
            toTest->pop();
        }
    }
}



int main(int argc, char** argv) {        
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
	
	uint64_t tick = __rdtsc()/100000; 
	
	for (int i = 0; i < maxThreads; i++) {
		thr[i] = std::thread(testStack, &toTest, MAX_VOLUME/maxThreads, i);
	}
	
	for (int i = 0; i < maxThreads; i++) {
		thr[i].join();
	}
	
	uint64_t tick2 = __rdtsc()/100000;
	printf("%d, %llu, \n", maxThreads, (long long unsigned int)tick2-tick);
	
	return 0;
}

