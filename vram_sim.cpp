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
#include <fstream> // For file handling
#include <sstream> // For string stream to parse file - csv
#include <iomanip> // For formatting output tables
#include <algorithm> // For count_if
#include <thread>   // For sleep in simulate
#include <chrono>   // For sleep in simulate
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

    // internal fragmentation - wasted space inside the last allocated page of a job

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
// Physical memory simulation
vector<PageFrame> memoryFrames;

// init mem frames
/*
This function allows the user to specify how 
many page frames exist in memory and the size 
for each
 */
void initFrames(int numFrames, int frameSize) {
    memoryFrames.clear();
    for (int i = 0; i < numFrames; i++) {
        memoryFrames.push_back({i, frameSize, true, -1, -1});
    }
}


// Function to load job pages into page frames randomly
void assignPageFrames(Job &job){
    // check if memory has enough free frames for this job
    int freeFrames = count_if(memoryFrames.begin(), memoryFrames.end(), [](PageFrame &f){ return f.isFree; });
    if (job.pages.size() > freeFrames) {
        cerr << "Not enough free frames to load Job ID " << job.jobID << endl;
        return;
    }

    for (int page: job.pages) {
        int frameIndex;
        do {
            frameIndex = rand() % memoryFrames.size();
        } while (!memoryFrames[frameIndex].isFree);
        
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
    cout << left << setw(8) << "Job ID" << setw(12) << "Job Size" << setw(14) << "No. of Pages" << setw(24) << "Internal Fragmentation" << "\n";
    for (const auto &job : jobs) {
        cout << left << setw(8) << job.jobID << setw(12) << job.jobSize
                << setw(14) << job.pages.size() << setw(24) << job.internalFragmentation << "\n";
    }

    cout << "\n--- Page Map Table ---\n";
    cout << left << setw(8) << "Job ID" << setw(14) << "Page Number" << setw(14) << "Frame Number" << "\n";
    for (const auto &job : jobs) {
        for (const auto &page : job.pages) {
            cout << left << setw(8) << job.jobID << setw(14) << page;
            if (job.pageTable.find(page) != job.pageTable.end()) {
                cout << setw(14) << job.pageTable.at(page) << "\n";
            } else {
                cout << setw(14) << "Not Loaded" << "\n";
            }
        }
    }

    cout << "\n--- Memory Map Table ---\n";
    cout << left << setw(14) << "Frame Number" << setw(14) << "Status" << setw(14) << "Job ID" << setw(14) << "Page Number" << "\n";
    for (const auto &frame : memoryFrames) {
        cout << left << setw(14) << frame.frameID;
        if (frame.isFree) {
            cout << setw(14) << "Free" << setw(14) << "-" << setw(14) << "-" << "\n";
        } else {
            cout << setw(14) << "Occupied" << setw(14) << frame.jobID << setw(14) << frame.pageNumber << "\n";
        }
    }
    cout << endl;
}


// Import jobs from a csv file to populate into the job list
vector<Job> importJobsFromFile(string filename, int pagesize) {
    vector<Job> jobs;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return jobs;
    }

    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        Job job;

        // CSV format: jobID, jobSize
        getline(ss, token, ',');
        job.jobID = stoi(token);
        getline(ss, token, ',');
        job.jobSize = stoi(token);
        job.pageSize = pagesize;

        divideJobIntoPages(job);
        jobs.push_back(job);
    }

    file.close();
    return jobs;
}   

// Address resolution function
// Resolve logical address based on user input
void resolveAddress(Job &job, int logicalAddress) {
    int pageNumber = logicalAddress / job.pageSize;
    int offset = logicalAddress % job.pageSize;

    if (pageNumber >= job.pages.size()) {
        cout << "Logical address out of bounds for Job ID " << job.jobID << endl;
        return;
    }

    if (job.pageTable.find(pageNumber) == job.pageTable.end()) {
        cout << "Page " << pageNumber << " not loaded in memory for Job ID " << job.jobID << endl;
        return;
    }

    int frameNumber = job.pageTable[pageNumber];
    int physicalAddress = frameNumber * job.pageSize + offset;

    cout << "Logical Address: " << logicalAddress << " -> Physical Address: " << physicalAddress
        << " (Frame: " << frameNumber << ", Offset: " << offset << ")\n";
}

// Function to preview jobs from CSV
void previewJobs(const vector<Job> &jobs) {
    cout << "\nJobs Loaded from CSV:\n";
    cout << left << setw(8) << "Job ID" << setw(12) << "Job Size" << setw(14) << "Pages" << setw(20) << "Fragmentation" << "\n";
    for (auto &job : jobs) {
        cout << left << setw(8) << job.jobID << setw(12) << job.jobSize 
                << setw(14) << job.pages.size() << setw(20) << job.internalFragmentation << "\n";
    }
}

// Function to show empty memory map
void showEmptyMemory() {
    cout << "\nInitial Memory State (All Free):\n";
    for (auto &frame : memoryFrames) {
        cout << "Frame " << frame.frameID << " | Free\n";
    }
}

// Function to simulate allocation with delay
void simulateAllocation(vector<Job> &jobs) {
    cout << "\nSimulating page allocation...\n";
    for (auto &job : jobs) {
        cout << "Allocating Job " << job.jobID << "...\n";
        assignPageFrames(job);
        this_thread::sleep_for(chrono::milliseconds(700));
    }
    cout << "Allocation complete.\n";
}

// Function to show memory stats
void showMemoryStats() {
    int totalFrames = memoryFrames.size();
    int usedFrames = count_if(memoryFrames.begin(), memoryFrames.end(), [](PageFrame &f){ return !f.isFree; });
    int freeFrames = totalFrames - usedFrames;

    cout << "\n--- Memory Stats ---\n";
    cout << "Total Frames: " << totalFrames << "\n";
    cout << "Used Frames : " << usedFrames << "\n";
    cout << "Free Frames : " << freeFrames << "\n";
    cout << "Usage       : " << (usedFrames * 100 / totalFrames) << "%\n";
}


int main() {
    srand(time(0)); // seed once

    // Initialize the memory frames
    // no. of frames, frame size
    initFrames(10, 512);

    // Import jobs
    vector<Job> jobs = importJobsFromFile("jobs.csv", 512);

    // ASCII banner
    cout << "┏┓┏┓┓┏┏┓┳┓┏┓  ┳┳┓┏┓┳┳┓┏┓┳┓┓┏\n";
    cout << "┗┓┣┫┃┃┣┫┃┃┣┫  ┃┃┃┣ ┃┃┃┃┃┣┫┗┫\n";
    cout << "┗┛┛┗┗┛┛┗┛┗┛┗  ┛ ┗┗┛┛ ┗┗┛┛┗┗┛\n";

    cout << "\nWelcome to the Paged Memory Allocation Simulator!\n";

    // Preview jobs and empty memory
    previewJobs(jobs);
    showEmptyMemory();

    int choice;
    do {
        cout << "\nMAIN MENU\n";
        cout << "1. Simulate Page Allocation\n";
        cout << "2. View Tables\n";
        cout << "3. Resolve Address\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            simulateAllocation(jobs);
        } 
        else if (choice == 2) {
            displayTables(jobs);
        } 
        else if (choice == 3) {
            cout << "\nCurrent Memory Map:\n";
            for (auto &frame : memoryFrames) {
                if (!frame.isFree) {
                    cout << "Frame " << frame.frameID << " -> Job " << frame.jobID 
                                << " Page " << frame.pageNumber << "\n";
                }
            }
            int jobID, addr;
            cout << "Enter Job ID: ";
            cin >> jobID;

            auto it = find_if(jobs.begin(), jobs.end(), [jobID](Job &j){ return j.jobID == jobID; });
            if (it != jobs.end()) {
                // Show the valid address range for this job
                cout << "Job " << jobID << " has size " << it->jobSize 
                    << " bytes (valid logical addresses: 0 - " << (it->jobSize - 1) << ")\n";
        
                int addr;
                cout << "Enter Logical Address to resolve (e.g., 0, 128, 512...): ";
                cin >> addr;

                resolveAddress(*it, addr);
            } else {
                cout << "Job ID not found.\n";
            }
        }
    } while (choice != 4);
    cout << "Exiting simulator. Goodbye!\n";
    return 0;
}




// TODO: Simulate the jobs with event-driven time units and then delay
// TODO: Verify the displacement calculation (algorithm) in address resolution
