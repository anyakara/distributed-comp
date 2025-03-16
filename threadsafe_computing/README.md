# Stack Data Structure - Lock-Based and Lock-Free Implementations
This repository provides two implementations of the stack data structure in C++: one using locks (mutex) and the other using a lock-free approach. Both implementations are designed to demonstrate the key differences in handling concurrent access to the stack. Main purpose of the implementation to understand how to isolate a data structures's operations to set guards for most critical data-related operations. This is an example of a stack, but the versatility of the logic allows for a queue's, tree's, heap's data structure logic alike to be modified such that most crucial data operations modifying the structure can be made atomic and locked/released as needed to prevent data races across multithreaded applications.

## Table of Contents
* Overview
* Lock-Based Stack Implementation
* Lock-Free Stack Implementation
* Template Code
* How to Run
* License

## Overview 
### Lock-Based Stack Implementation:
The lock-based stack uses ```std::mutex``` to ensure thread safety when multiple threads push or pop elements to/from the stack. The mutex prevents race conditions by blocking other threads until the lock is released.

### Lock-Free Stack Implementation:
The lock-free stack uses atomic operations (```std::atomic```) and the compare-and-swap (CAS) technique to achieve thread safety without locking. This approach ensures that threads can operate on the stack concurrently without needing to block one another, thus improving performance in highly concurrent environments.

## Lock-Based Stack Implementation
This implementation uses a ```std::mutex``` to synchronize access to the stack, ensuring that only one thread can access the stack at a time.

## Code Example (Lock-Based Stack)
```
#include <iostream>
#include <stack>
#include <mutex>
#include <thread>

template <typename T>
class LockBasedStack {
private:
    std::stack<T> stack;
    std::mutex mtx;

public:
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        stack.push(value);
    }

    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (stack.empty()) return false;
        value = stack.top();
        stack.pop();
        return true;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mtx);
        return stack.empty();
    }
};
```

In this implementation:
The push and pop operations are synchronized with a ```std::mutex``` to ensure that only one thread can access the stack at any given time.
The ```std::lock_guard``` is used to automatically acquire and release the lock, ensuring that no two threads can modify the stack concurrently.

Similar concept follows for lock-free can be found in the code segments above.

## How to Run
1. Clone the repository to your local machine:
```git clone https://github.com/yourusername/lock-free-stack.git```
```cd lock-free-stack```

2. Compile the code using ```g++``` or any compatible C++ compiler:
```g++ -std=c++11 -o stack_example stack_example.cpp```

3. Run the compiled executable:
```./stack_example```

You can modify the example code to test the stack with different data types or more complex scenarios.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
