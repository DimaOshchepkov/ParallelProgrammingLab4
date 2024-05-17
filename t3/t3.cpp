#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <sstream>
#include <vector>

using namespace std;

queue<vector<int>> dataQueue; // Queue for storing arrays
mutex m; // Mutex for synchronizing access to the queue
condition_variable cv; // Condition variable to notify about queue status

// Function for reading arrays from a file
void readerThread(const string& inputFilename) {
    ifstream inputFile(inputFilename);
    if (!inputFile.is_open()) {
        cerr << "Error opening input file: " << inputFilename << endl;
        return;
    }

    string line;
    getline(inputFile, line);// Read the number of arrays
    int M = std::stoi(line);

    for (int i = 0; i < M; ++i) {
        vector<int> data;
        string line;
        getline(inputFile, line); // Read a line with an array
        stringstream ss(line);
        int num;
        while (ss >> num) {
            data.push_back(num);
        }

        {
            unique_lock<mutex> lock(m);
            dataQueue.push(data); // Add the array to the queue
            cv.notify_one(); // Notify the calculator thread
        }
    }

    inputFile.close();
}

// Function for calculating the sum of array elements
void calculatorThread(const string& outputFilename) {
    ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        cerr << "Error opening output file: " << outputFilename << endl;
        return;
    }

    while (true) {
        unique_lock<mutex> lock(m);
        cv.wait(lock, [] { return !dataQueue.empty(); }); // Wait for data to appear in the queue

        vector<int> data = dataQueue.front();
        dataQueue.pop();

        // Calculate the sum of array elements
        int sum = 0;
        for (int num : data) {
            sum += num;
        }

        outputFile << sum << endl; // Write the sum to the file
    }
}

int main() {
    string inputFilename = "C:\\Programming\\ParallelProgramming\\ParallelProgrammingLab4\\input.txt";
    string outputFilename = "C:\\Programming\\ParallelProgramming\\ParallelProgrammingLab4\\output.txt";

    // Create threads
    thread reader(readerThread, inputFilename);
    thread calculator(calculatorThread, outputFilename);

    // Wait for threads to complete
    reader.join();
    calculator.join();

    return 0;
}