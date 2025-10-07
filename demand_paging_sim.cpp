/* DEMAND PAGED MEMORY ALLOCATION SIMULATION
    •  Accept more than one Job
    •  Divide the Jobs into pages based on a specify size
    •  Load jobs into Memory Frames randomly
    •  Perform Address resolution

    // 1. Input job size and page size
    // 2. Compute numPages + fragmentation
    // 3. Store pages in vector
    // 4. On demand, load pages when accessed
    // 5. Handle page faults and replacement
    // 6. Update tables dynamically
*/

#include <iostream> 
#include <vector>   // For dynamic array
#include <queue>    // For FIFO queue
#include <cstdlib> // For rand() and srand()
#include <ctime>  
#include <unordered_map> // for hash map
#include <unordered_set> // for hash set
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
    - we want to track how many pages are loaded
    - track page faults
*/

struct Job {
    int jobID;
    int jobSize; 
    int pageSize;
    int internalFragmentation; // in bytes
    vector<int> pages; // Store page numbers
    unordered_set<int> loadedPages; // Track which pages are currently in memory
    unordered_map<int, int> pageTable; // Page number to Frame number mapping
    int pageFaults; // Count of page faults for this job
};

/*
    PAGE FRAME Struct
    Each Page Frame has the ff:
    - an ID or page frame number
    - its size
    - its availability 
    - the job ID it is currently holding
    - access time for LRU replacement
    - modified and referenced bits
*/
struct PageFrame {
    int frameID;
    int frameSize;
    bool isFree; // Availability
    int jobID; // Job currently holding this frame
    int pageNumber; // Page number currently in this frame
    int accessTime; // For LRU replacement algorithm
};

// Global memory frames
// Physical memory simulation
vector<PageFrame> memoryFrames;

// Global variables for demand paging
int currentTime = 0; // Global time counter for LRU
queue<int> fifoQueue; // For FIFO replacement

// Function to divide job into pages
void divideJobIntoPages(Job &job) {
    // calc num pages and displacement
    int numPages = job.jobSize / job.pageSize;
    int remainingBytes = job.jobSize % job.pageSize;

    // internal fragmentation - wasted space inside the last allocated page of a job
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
    
    // Initialize page faults counter
    job.pageFaults = 0;
}

// init mem frames
/*
This function allows the user to specify how 
many page frames exist in memory and the size 
for each
 */
void initFrames(int numFrames, int frameSize) {
    memoryFrames.clear();
    fifoQueue = queue<int>(); // Clear FIFO queue
    currentTime = 0;
    
    for (int i = 0; i < numFrames; i++) {
        memoryFrames.push_back({i, frameSize, true, -1, -1, 0});
    }
}

// Function to find a free frame
int findFreeFrame() {
    for (int i = 0; i < memoryFrames.size(); i++) {
        if (memoryFrames[i].isFree) {
            return i;
        }
    }
    return -1; // No free frame found
}

// Function: fifoReplacement
// Purpose: Implements FIFO page replacement algorithm
int fifoReplacement() {
    if (fifoQueue.empty()) {
        return 0; // Fallback to frame 0
    }
    
    int frameToReplace = fifoQueue.front();
    fifoQueue.pop();
    return frameToReplace;
}


// Function: loadPage
// Purpose: Loads a specific page into memory using demand paging with FIFO replacement
bool loadPage(Job &job, int pageNumber, vector<Job> &allJobs) {
    // Check if page is already loaded
    if (job.loadedPages.find(pageNumber) != job.loadedPages.end()) {
        // Page hit - update access time
        for (auto &frame : memoryFrames) {
            if (!frame.isFree && frame.jobID == job.jobID && frame.pageNumber == pageNumber) {
                frame.accessTime = currentTime;
                break;
            }
        }
        return true;
    }
    
    // Page fault occurred
    job.pageFaults++;
    currentTime++;
    
    // Try to find a free frame first
    int frameIndex = findFreeFrame();
    
    if (frameIndex == -1) {
        // No free frames, use FIFO replacement
        frameIndex = fifoReplacement();
        
        // Remove the old page from its job's loaded pages
        if (!memoryFrames[frameIndex].isFree) {
            int oldJobID = memoryFrames[frameIndex].jobID;
            int oldPageNumber = memoryFrames[frameIndex].pageNumber;
            
            // Find and update the old job
            for (auto &j : allJobs) {
                if (j.jobID == oldJobID) {
                    j.loadedPages.erase(oldPageNumber);
                    j.pageTable.erase(oldPageNumber);
                    break;
                }
            }
        }
    }
    
    // Load the new page
    memoryFrames[frameIndex].isFree = false;
    memoryFrames[frameIndex].jobID = job.jobID;
    memoryFrames[frameIndex].pageNumber = pageNumber;
    memoryFrames[frameIndex].accessTime = currentTime;
    
    // Update job's page table and loaded pages
    job.pageTable[pageNumber] = memoryFrames[frameIndex].frameID;
    job.loadedPages.insert(pageNumber);
    
    // Add to FIFO queue
    fifoQueue.push(frameIndex);
    
    return true;
}

// Function to load job pages into page frames randomly (OLD METHOD - NOT DEMAND PAGING)
void assignPageFrames(Job &job){
    // Check if memory has enough free frames for this job
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
    - Pages Loaded
    - Page Faults

    Page Map Table:
    - Page Number
    - Frame Number
    - Loaded Status
    - Modified/Referenced bits

    Memory Map Table:
    - frame number
    - availability or status (free or occupied)
    - job id (if occupied)
    - page number (if occupied)
    - access time
*/
void displayTables(const vector<Job> &jobs) {
    cout << "\n--- Job Table ---\n";
    cout << left << setw(8) << "Job ID" << setw(12) << "Job Size" << setw(14) << "No. of Pages" << setw(16) << "Pages Loaded" << setw(12) << "Page Faults" << setw(24) << "Internal Fragmentation" << "\n";
    for (const auto &job : jobs) {
        cout << left << setw(8) << job.jobID << setw(12) << job.jobSize
                << setw(14) << job.pages.size() << setw(16) << job.loadedPages.size() 
                << setw(12) << job.pageFaults << setw(24) << job.internalFragmentation << "\n";
    }

    cout << "\n--- Page Map Table ---\n";
    cout << left << setw(8) << "Job ID" << setw(14) << "Page Number" << setw(14) << "Frame Number" << setw(12) << "Status" << "\n";
    for (const auto &job : jobs) {
        for (const auto &page : job.pages) {
            cout << left << setw(8) << job.jobID << setw(14) << page;
            if (job.loadedPages.find(page) != job.loadedPages.end()) {
                cout << setw(14) << job.pageTable.at(page) << setw(12) << "Loaded" << "\n";
            } else {
                cout << setw(14) << "-" << setw(12) << "Not Loaded" << "\n";
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
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        stringstream ss(line);
        string token;
        Job job;

        // CSV format: jobID, jobSize
        getline(ss, token, ',');
        if (token.empty()) {
            continue; // Skip if jobID is empty
        }
        job.jobID = stoi(token);
        
        getline(ss, token, ',');
        if (token.empty()) {
            continue; // Skip if jobSize is empty
        }
        job.jobSize = stoi(token);
        job.pageSize = pagesize;

        divideJobIntoPages(job);
        jobs.push_back(job);
    }

    file.close();
    return jobs;
}   

// Address resolution function with demand paging
// Resolve logical address based on user input
void resolveAddress(Job &job, int logicalAddress, vector<Job> &allJobs) {
    int pageNumber = logicalAddress / job.pageSize;
    int offset = logicalAddress % job.pageSize;

    if (pageNumber >= job.pages.size()) {
        cout << "Logical address out of bounds for Job ID " << job.jobID << endl;
        return;
    }

    // Check if page is loaded, if not, load it using demand paging
    if (job.loadedPages.find(pageNumber) == job.loadedPages.end()) {
        cout << "Page " << pageNumber << " not in memory. Loading page on demand...\n";
        loadPage(job, pageNumber, allJobs);
    }

    int frameNumber = job.pageTable[pageNumber];
    int physicalAddress = frameNumber * job.pageSize + offset;

    cout << "Logical Address: " << logicalAddress << " -> Physical Address: " << physicalAddress
        << " (Frame: " << frameNumber << ", Offset: " << offset << ")\n";
    cout << "Page Faults for Job " << job.jobID << ": " << job.pageFaults << "\n";
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

    cout << "\nWelcome to the Demand Paged Memory Allocation Simulator!\n";

    // Preview jobs and empty memory
    previewJobs(jobs);
    showEmptyMemory();

    int choice;
    do {
        cout << "\nMAIN MENU\n";
        cout << "1. Simulate Page Allocation (Static)\n";
        cout << "2. View Tables\n";
        cout << "3. Resolve Address (Demand Paging)\n";
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
        
                cout << "Enter Logical Address to resolve (e.g., 0, 128, 512...): ";
                cin >> addr;

                resolveAddress(*it, addr, jobs);
            } else {
                cout << "Job ID not found.\n";
            }
        }
    } while (choice != 4);
    cout << "Exiting simulator. Goodbye!\n";
    return 0;
}


