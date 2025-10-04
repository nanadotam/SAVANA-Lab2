/* PAGED MEMORY ALLOCATION SIMULATION
    •  Accept a Job
    •  Divide the Job into pages based on a specify size

    Indicate internal fragmentation if any
    •  Load jobs into Page Frames randomly
    •  Perform Address resolution
*/

#include <iostream> 
#include <vector>   // For dynamic array
#include <queue> // For queue implementation
#include <cstdlib> // For rand() and srand()
#include <ctime>  
#include <list>
#include <unordered_set> // for hash similiar to dict in python 
using namespace std;


/*
    Jobs divided into pages of equal size 
    Using struct for data 
    Each Job has the ff:
    - an ID
    - its size
    - we want to track how
*/
struct Job {
    int jobID;
    int jobSize; 
    int pageSize;
    int internalFragmentation; // in bytes
    vector<int> pages; // Store page numbers
    // page -> frame
    unordered_map<int, int> pageTable; // Page number to Frame number mapping
};

/*
    PAGE FRAME Struct
    Each Page Frame has the ff:
    - an ID or page frame number
    - its size
    - its availability 
    - the job ID it is currently holding

*/
struct PageFrame {
    int frameID;
    int frameSize;
    bool isFree; // Availability
    int jobID; // Job currently holding this frame
    int pageNumber; // Page number currently in this frame
};



// Function to divide job into pages
void divideJobIntoPages(Job &job) {
    int numPages = job.jobSize / job.pageSize;
    int remainingBytes = job.jobSize % job.pageSize;

    // If there are remaining bytes, we need an additional page
    if (remainingBytes > 0) {
        numPages++;
        job.internalFragmentation = job.pageSize - remainingBytes;
    } else {
        job.internalFragmentation = 0;
    }

    // Assign page numbers
    for (int i = 0; i < numPages; i++) {
        job.pages.push_back(i);
    }
}