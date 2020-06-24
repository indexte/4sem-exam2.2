#include <iostream>
#include <thread>
#include <list>
#include <vector>
#include <algorithm>
#include <future>

using std::cout;
using std::list;
using std::vector;

//F - squaring function
void f(int& A) {
    A *= A;
}

//find min element
auto  findMin(const list<int> input) {
    return *std::min_element(input.begin(), input.end());
}

int main()
{
    //list of input values
    list<int> input = { 5,6,3,68,9,1,2,4,5,6,767,7657,36 };
    //vector of threads
    vector<std::thread> threadVector;

    //add threads
    for (auto i = input.begin(), end = input.end(); i != end; ++i) {
        threadVector.push_back(std::thread(f, std::ref(*i)));
    }

    //wait for result
    for (auto& i : threadVector) {
        i.join();
    }

    //using the future we get the minimum value
    std::future<int> ret = std::async(&findMin, input);
    //show min value
    cout << "Min element is " << ret.get();
}