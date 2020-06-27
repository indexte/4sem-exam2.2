#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <future>

using std::cout;
using std::vector;
using std::future;
using std::thread;
using std::pair;

//F - square function
int f(int param) {
    param *= param;
    return param;
}

//an auxiliary function that starts the function f for a subarray by element
//and returns the subarray accordingly
template <typename A, typename B>
vector<B> fHelper(B(*f)(A), const vector<A>& subVector) {
    vector<B> ans;
    for (auto& i : subVector) {
        ans.push_back(f(i));
    }
    return ans;
}

//function to change the array
//parameters: function f (specified in the condition), list of input values of type A and amount of threads (default - the largest)
template <typename A, typename B>
vector<B> getChanged(B(*f)(A), vector<A> input, size_t threadAmount = 0) {
    //change the default amount of threads to the highest possible
    if (threadAmount == 0)
        threadAmount = thread::hardware_concurrency();

    //vector of future values (subvectors)
    vector<future<vector<B>>> threadFutures(threadAmount - 1);

    //auxiliary variable for vector division - the size of subvectors
    size_t subSize = input.size() / threadAmount;

    //creating a subvector to run the fHelper function for each subvector in a separate thread
    for (size_t i = 0, size = threadFutures.size(); i < size; ++i) {
        vector<A> subVector = vector<A>(input.begin() + i * subSize, input.begin() + i * subSize + subSize);
        threadFutures[i] = async(&fHelper<A, B>, f, subVector);
    }

    //array of initial values
    vector<B> result(input.size());

    //the last part of the final array which is calculated in the final thread
    vector<B> resultLast = fHelper<A, B>(f, vector<A>(input.begin() + (threadAmount - 1) * subSize, input.end()));
    std::copy(resultLast.begin(), resultLast.end(), result.begin() + (threadAmount - 1) * subSize);

    //getting values from futures and copying them to the main array
    for (size_t i = 0, size = threadFutures.size(); i < size; ++i) {
        vector<B> temp = threadFutures[i].get();
        std::copy(temp.begin(), temp.end(), result.begin() + i * subSize);
    }

    return result;
}

//auxiliary function for finding the minimum element
template <typename B>
pair<size_t, B> minHelper(bool(*compFunc)(const B& a, const B& b), const vector<B>& subvector) {
    auto i = std::min_element(subvector.begin(), subvector.end(), compFunc);
    return pair<size_t, B>(i - subvector.begin(), *i);
}

//the function to get the minimum value
//parameters: comparison function, input array, amount of threads
template <typename B>
pair<size_t, B> getMin(bool(*compFunc)(const B& a, const B& b), vector<B> input, size_t threadAmount = 0) {
    //change the default amount of threads to the highest possible
    if (threadAmount == 0)
        threadAmount = thread::hardware_concurrency();

    vector<future<pair<size_t, B>>> subMinFuture(threadAmount - 1);

    //auxiliary variable for vector division - the size of subvectors
    size_t subSize = input.size() / threadAmount;

    //creating a subvector to run the minHelper function for each subvector in a separate thread
    for (size_t i = 0, size = subMinFuture.size(); i < size; ++i) {
        vector<B> subVector = vector<B>(input.begin() + i * subSize, input.begin() + i * subSize + subSize);
        subMinFuture[i] = std::async(&minHelper<B>, compFunc, subVector);
    }

    //an array of minimum values
    vector<pair<size_t, B>> minVec;

    //the last part of the final array which is calculated in the final thread
    pair<size_t, B> resultLast = minHelper<B>(compFunc, vector<B>(input.begin() + (threadAmount - 1) * subSize, input.end()));
    resultLast.first = resultLast.first + (threadAmount - 1) * subSize;

    //getting values from futures and copying them to the main array
    for (size_t i = 0, size = subMinFuture.size(); i < size; ++i) {
        pair<size_t, B> temp = subMinFuture[i].get();
        //calculation of the correct index
        temp.first = temp.first + i * subSize;
        minVec.push_back(temp);
    }

    //adding the last minimum
    minVec.push_back(resultLast);

    vector<B> temp;
    for (auto& i : minVec) {
        temp.push_back(i.second);
    }

    return minVec[minHelper<B>(compFunc, temp).first];
}

int main()
{
    //array of input values
    vector<int> input = { 5,6,3,68,9,2,4,5,6,767,7657,36 };

    vector<int> result = getChanged<int, int>(f, input, 4);

    pair<size_t, int> min = getMin<int>([](const int& a, const int& b) {
        return (a < b);
        }, result);

    cout << "min elem: " << min.second << ", with index " << min.first;
}