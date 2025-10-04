/* PAGED MEMORY ALLOCATION SIMULATION
    •  Accept a Job
    •  Divide the Job into pages based on a specify size

    Indicate internal fragmentation if any
    •  Load jobs into Page Frames randomly
    •  Perform Address resolution

    // 1. Input job size and page size
    // 2. Compute numPages + fragmentation
    // 3. Store pages in vector
    // 4. Randomly assign frames
    // 5. Update tables
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
    // calc num pages and displacement
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

// Global memory frames
vector<PageFrame> memoryFrames;

// init mem frames
void initFrames(int numFrames, int frameSize) {
    memoryFrames.clear();
    for (int i = 0; i < numFrames; i++) {
        memoryFrames.push_back({i, frameSize, true, -1, -1});
    }
}


// Function to load job pages into page frames randomly
void assignPageFrames(Job &job){
    srand(time(0)); // Seed for randomness
    unordered_set<int> assignedFrames; // To avoid duplicate frame assignments

    for (int page: job.pages) {
        int frameIndex;
        do {
            frameIndex = rand() % memoryFrames.size();
        } while (!memoryFrames[frameIndex].isFree || assignedFrames.find(frameIndex) != assignedFrames.end());
        // Assign frame to page
        memoryFrames[frameIndex].isFree = false;
        memoryFrames[frameIndex].jobID = job.jobID;
        memoryFrames[frameIndex].pageNumber = page;

        // Mark frame as assigned
        job.pageTable[page] = memoryFrames[frameIndex].frameID;
}
}

// Job list array
// List of jobs

// Need to simulate the JT, PMT, MMT
/* 
    Job Table:
    - Job ID
    - Job Size
    - No. of Pages    

    Page Map Table:
    - Page Number
    - Frame Number

    Memory Map Table:
    - frame number
    - availability or status (free or occupied)
    - job ID (if occupied)
    - page number (if occupied)
*/

void displayTables(const vector<Job> &jobs) {
    cout << "\n--- Job Table ---\n";
    cout << "Job ID\tJob Size\tNo. of Pages\tInternal Fragmentation\n";
    for (const auto &job : jobs) {
        cout << job.jobID << "\t" << job.jobSize << "\t\t" << job.pages.size() << "\t\t" << job.internalFragmentation << "\n";
    }

    cout << "\n--- Page Map Table ---\n";
    cout << "Job ID\tPage Number\tFrame Number\n";
    for (const auto &job : jobs) {
        for (const auto &page : job.pages) {
            if (job.pageTable.find(page) != job.pageTable.end()) {
                cout << job.jobID << "\t" << page << "\t\t" << job.pageTable.at(page) << "\n";
            } else {
                cout << job.jobID << "\t" << page << "\t\t" << "Not Loaded\n";
            }
        }
    }

    cout << "\n--- Memory Map Table ---\n";
    cout << "Frame Number\tStatus\t\tJob ID\tPage Number\n";
    for (const auto &frame : jobList[0].pageTable) {
        cout << frame.second << "\t\t";
        if (frame.second == -1) {
            cout << "Free\t\t-\t-\n";
        } else {
            cout << "Occupied\t" << frame.first << "\t" << frame.second << "\n";
        }
    }
    cout << endl;
}

// Import jobs from a csv file to populate into the job list
void importJobsFromFile() {
}