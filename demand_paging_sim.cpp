/* demand paged memory allocation simulation
    •  accept more than one job
    •  divide the jobs into pages based on a specify size
    •  load jobs into memory frames randomly
    •  perform address resolution

    // 1. input job size and page size
    // 2. compute numpages + fragmentation
    // 3. store pages in vector (but don't load them yet)
    // 4. on demand, load pages when accessed
    // 5. handle page faults and replacement
    // 6. update tables dynamically
*/

#include <iostream> 
#include <vector>   // for dynamic array
#include <queue> // for queue implementation
#include <cstdlib> // for rand() and srand()
#include <ctime>  
#include <list>
#include <unordered_set> // for hash similar to dict in python
#include <unordered_map> // for hash map 
#include <fstream> // for file handling
#include <sstream> // for string stream to parse file - csv
#include <iomanip> // for formatting output tables
#include <algorithm> // for count_if
// #include <thread>   // for sleep in simulate
// #include <chrono>   // for sleep in simulate
using namespace std;

/*
    jobs divided into pages of equal size 
    using struct for data 
    each job has the ff:
    - an id
    - its size
    - we want to track how many pages are loaded
    - track page faults
*/
struct Job {
    int jobID;
    int jobSize; 
    int pageSize;
    vector<int> pages; // store page numbers
    unordered_set<int> loadedPages; // track which pages are currently in memory
    unordered_map<int, int> pageTable; // page number to frame number mapping
    int pageFaults; // count of page faults for this job
};

/*
    page frame struct
    each page frame has the ff:
    - an id or page frame number
    - its size
    - its availability 
    - the job id it is currently holding
    - access time for lru replacement
*/
struct PageFrame {
    int frameID;
    int frameSize;
    bool isFree; // availability
    int jobID; // job currently holding this frame
    int pageNumber; // page number currently in this frame
    int accessTime; // for lru replacement algorithm
    bool modified; // M bit - page has been modified
    bool referenced; // R bit - page has been referenced
};

// global variables for demand paging
vector<PageFrame> memoryFrames;
int currentTime = 0; // global time counter for lru
queue<int> fifoQueue; // for fifo replacement
int replacementAlgorithm = 1; // 1 = fifo, 2 = lru

// Function: divideJobIntoPages
// Purpose: Divides a job into pages based on page size for demand paging
void divideJobIntoPages(Job &job) {
    // calc num pages and displacement
    int numPages = job.jobSize / job.pageSize;
    int remainingBytes = job.jobSize % job.pageSize;

    // if there are remaining bytes, we need an additional page
    if (remainingBytes > 0) {
        numPages++;
    }

    // assign page numbers
    for (int i = 0; i < numPages; i++) {
        job.pages.push_back(i);
    }
    
    // initialize page faults counter
    job.pageFaults = 0;
}

// Function: initFrames
// Purpose: Initializes memory frames with default values
void initFrames(int numFrames, int frameSize) {
    memoryFrames.clear();
    fifoQueue = queue<int>(); // clear fifo queue
    currentTime = 0;
    
    for (int i = 0; i < numFrames; i++) {
        memoryFrames.push_back({i, frameSize, true, -1, -1, 0, false, false});
    }
}

// Function: findFreeFrame
// Purpose: Returns index of the first free frame, or -1 if none available
int findFreeFrame() {
    for (int i = 0; i < memoryFrames.size(); i++) {
        if (memoryFrames[i].isFree) {
            return i;
        }
    }
    return -1; // no free frame found
}

// Function: fifoReplacement
// Purpose: Implements FIFO page replacement algorithm
int fifoReplacement() {
    if (fifoQueue.empty()) {
        return 0; // fallback to frame 0
    }
    
    int frameToReplace = fifoQueue.front();
    fifoQueue.pop();
    return frameToReplace;
}

// Function: lruReplacement
// Purpose: Implements LRU page replacement algorithm
int lruReplacement() {
    int oldestTime = currentTime;
    int frameToReplace = 0;
    
    for (int i = 0; i < memoryFrames.size(); i++) {
        if (memoryFrames[i].accessTime < oldestTime) {
            oldestTime = memoryFrames[i].accessTime;
            frameToReplace = i;
        }
    }
    
    return frameToReplace;
}

// Function: loadPage
// Purpose: Loads a specific page into memory using demand paging with page replacement
bool loadPage(Job &job, int pageNumber, vector<Job> &allJobs) {
    // check if page is already loaded
    if (job.loadedPages.find(pageNumber) != job.loadedPages.end()) {
        // page hit - update access time for lru and set referenced bit
        for (auto &frame : memoryFrames) {
            if (!frame.isFree && frame.jobID == job.jobID && frame.pageNumber == pageNumber) {
                frame.accessTime = currentTime;
                frame.referenced = true;
                break;
            }
        }
        return true;
    }
    
    // page fault occurred
    job.pageFaults++;
    currentTime++;
    
    // try to find a free frame first
    int frameIndex = findFreeFrame();
    
    if (frameIndex == -1) {
        // no free frames, need to replace a page
        if (replacementAlgorithm == 1) {
            frameIndex = fifoReplacement();
        } else {
            frameIndex = lruReplacement();
        }
        
        // remove the old page from its job's loaded pages
        if (!memoryFrames[frameIndex].isFree) {
            int oldJobID = memoryFrames[frameIndex].jobID;
            int oldPageNumber = memoryFrames[frameIndex].pageNumber;
            
            // find and update the old job
            for (auto &j : allJobs) {
                if (j.jobID == oldJobID) {
                    j.loadedPages.erase(oldPageNumber);
                    j.pageTable.erase(oldPageNumber);
                    break;
                }
            }
        }
    }
    
    // load the new page
    memoryFrames[frameIndex].isFree = false;
    memoryFrames[frameIndex].jobID = job.jobID;
    memoryFrames[frameIndex].pageNumber = pageNumber;
    memoryFrames[frameIndex].accessTime = currentTime;
    memoryFrames[frameIndex].referenced = true;
    memoryFrames[frameIndex].modified = false;
    
    // update job's page table and loaded pages
    job.pageTable[pageNumber] = memoryFrames[frameIndex].frameID;
    job.loadedPages.insert(pageNumber);
    
    // add to fifo queue if using fifo
    if (replacementAlgorithm == 1) {
        fifoQueue.push(frameIndex);
    }
    
    return true;
}

// job list array
// list of jobs

// need to simulate the jt, pmt, mmt
/* 
    job table:
    - job id
    - job size
    - no. of pages    
    - pages loaded
    - page faults

    page map table:
    - page number
    - frame number
    - loaded status

    memory map table:
    - frame number
    - availability or status (free or occupied)
    - job id (if occupied)
    - page number (if occupied)
    - access time
*/
// Function: displayTables
// Purpose: Displays job table, page map table, and memory map table
void displayTables(const vector<Job> &jobs) {
    cout << "\n--- job table ---\n";
    cout << left << setw(8) << "job id" << setw(12) << "job size" << setw(14) << "no. of pages" 
         << setw(16) << "pages loaded" << setw(16) << "page faults" << "\n";
    for (const auto &job : jobs) {
        cout << left << setw(8) << job.jobID << setw(12) << job.jobSize
             << setw(14) << job.pages.size() << setw(16) << job.loadedPages.size() 
             << setw(16) << job.pageFaults << "\n";
    }

    cout << "\n--- page map table ---\n";
    cout << left << setw(8) << "job id" << setw(14) << "page number" << setw(14) << "frame number" 
         << setw(14) << "status" << setw(14) << "modified" << setw(14) << "referenced" << "\n";
    for (const auto &job : jobs) {
        for (const auto &page : job.pages) {
            cout << left << setw(8) << job.jobID << setw(14) << page;
            if (job.pageTable.find(page) != job.pageTable.end()) {
                int frameNum = job.pageTable.at(page);
                // find the frame to get M and R bits
                bool modified = false, referenced = false;
                for (const auto &frame : memoryFrames) {
                    if (frame.frameID == frameNum && !frame.isFree) {
                        modified = frame.modified;
                        referenced = frame.referenced;
                        break;
                    }
                }
                cout << setw(14) << frameNum << setw(14) << "loaded" 
                     << setw(14) << (modified ? "M" : "-") << setw(14) << (referenced ? "R" : "-") << "\n";
            } else {
                cout << setw(14) << "not loaded" << setw(14) << "not loaded" 
                     << setw(14) << "-" << setw(14) << "-" << "\n";
            }
        }
    }

    cout << "\n--- memory map table ---\n";
    cout << left << setw(14) << "frame number" << setw(14) << "status" << setw(14) << "job id" 
         << setw(14) << "page number" << setw(14) << "access time" << "\n";
    for (const auto &frame : memoryFrames) {
        cout << left << setw(14) << frame.frameID;
        if (frame.isFree) {
            cout << setw(14) << "free" << setw(14) << "-" << setw(14) << "-" << setw(14) << "-" << "\n";
        } else {
            cout << setw(14) << "occupied" << setw(14) << frame.jobID << setw(14) << frame.pageNumber 
                 << setw(14) << frame.accessTime << "\n";
        }
    }
    cout << endl;
}

// Function: importJobsFromFile
// Purpose: Imports jobs from CSV file and divides them into pages
vector<Job> importJobsFromFile(string filename, int pagesize) {
    vector<Job> jobs;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "error opening file: " << filename << endl;
        return jobs;
    }

    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        Job job;

        // csv format: jobid, jobsize
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

// Function: resolveAddress
// Purpose: Resolves logical address to physical address with demand paging
void resolveAddress(Job &job, int logicalAddress, vector<Job> &allJobs) {
    int pageNumber = logicalAddress / job.pageSize;
    int offset = logicalAddress % job.pageSize;

    if (pageNumber >= job.pages.size()) {
        cout << "logical address out of bounds for job id " << job.jobID << endl;
        return;
    }

    // check if page is loaded, if not load it (demand paging)
    if (job.loadedPages.find(pageNumber) == job.loadedPages.end()) {
        cout << "page fault! loading page " << pageNumber << " for job " << job.jobID << "...\n";
        loadPage(job, pageNumber, allJobs);
    } else {
        // page hit - update access time and set referenced bit
        for (auto &frame : memoryFrames) {
            if (!frame.isFree && frame.jobID == job.jobID && frame.pageNumber == pageNumber) {
                frame.accessTime = currentTime;
                frame.referenced = true;
                currentTime++;
                break;
            }
        }
    }

    int frameNumber = job.pageTable[pageNumber];
    int physicalAddress = frameNumber * job.pageSize + offset;

    cout << "logical address: " << logicalAddress << " -> physical address: " << physicalAddress
        << " (frame: " << frameNumber << ", offset: " << offset << ")\n";
}

// Function: previewJobs
// Purpose: Displays a preview of jobs loaded from CSV file
void previewJobs(const vector<Job> &jobs) {
    cout << "\njobs loaded from csv:\n";
    cout << left << setw(8) << "job id" << setw(12) << "job size" << setw(14) << "pages" 
            << setw(16) << "pages loaded" << setw(16) << "page faults" << "\n";
    for (auto &job : jobs) {
        cout << left << setw(8) << job.jobID << setw(12) << job.jobSize 
                << setw(14) << job.pages.size() << setw(16) << job.loadedPages.size() 
                << setw(16) << job.pageFaults << "\n";
    }
}

// Function: showEmptyMemory
// Purpose: Displays initial empty memory state
void showEmptyMemory() {
    cout << "\ninitial memory state (all free):\n";
    for (auto &frame : memoryFrames) {
        cout << "frame " << frame.frameID << " | free\n";
    }
}

// Function: simulateDemandPaging
// Purpose: Simulates demand paging by randomly accessing logical addresses for each job
void simulateDemandPaging(vector<Job> &jobs) {
    cout << "\nsimulating demand paging...\n";
    cout << "pages will be loaded on demand when accessed.\n";
    
    // show initial state
    cout << "\ninitial state - no pages loaded yet:\n";
    for (auto &job : jobs) {
        cout << "job " << job.jobID << ": " << job.loadedPages.size() << " pages loaded\n";
    }
    
    // simulate random access to logical addresses for each job
    cout << "\n--- simulation running ---\n";
    for (auto &job : jobs) {
        cout << "\naccessing job " << job.jobID << "...\n";
        // randomly access 3 logical addresses per job
        for (int i = 0; i < 3; i++) {
            int randomAddress = rand() % job.jobSize;
            cout << "accessing logical address: " << randomAddress << "\n";
            resolveAddress(job, randomAddress, jobs);
        }
    }
    
    cout << "\n--- simulation complete ---\n";
    cout << "final state:\n";
    for (auto &job : jobs) {
        cout << "job " << job.jobID << ": " << job.loadedPages.size() 
             << " pages loaded, " << job.pageFaults << " page faults\n";
    }
}

// Function: showMemoryStats
// Purpose: Displays memory usage statistics
void showMemoryStats() {
    int totalFrames = memoryFrames.size();
    int usedFrames = count_if(memoryFrames.begin(), memoryFrames.end(), [](PageFrame &f){ return !f.isFree; });
    int freeFrames = totalFrames - usedFrames;

    cout << "\n--- memory stats ---\n";
    cout << "total frames: " << totalFrames << "\n";
    cout << "used frames : " << usedFrames << "\n";
    cout << "free frames : " << freeFrames << "\n";
    cout << "usage       : " << (usedFrames * 100 / totalFrames) << "%\n";
}

// Function: setReplacementAlgorithm
// Purpose: Allows user to select page replacement algorithm (FIFO or LRU)
void setReplacementAlgorithm() {
    cout << "\nselect page replacement algorithm:\n";
    cout << "1. FIFO (First In, First Out)\n";
    cout << "2. LRU (Least Recently Used)\n";
    cout << "Enter choice: ";
    cin >> replacementAlgorithm;
    
    if (replacementAlgorithm == 1) {
        cout << "fifo replacement algorithm selected.\n";
    } else if (replacementAlgorithm == 2) {
        cout << "lru replacement algorithm selected.\n";
    } else {
        cout << "invalid choice, defaulting to fifo.\n";
        replacementAlgorithm = 1;
    }
}


int main() {
    srand(time(0)); // seed once

    // initialize the memory frames
    // no. of frames, frame size
    initFrames(8, 512); // smaller memory for demand paging demo

    // import jobs
    vector<Job> jobs = importJobsFromFile("jobs.csv", 512);

    // ascii banner
    cout << "┏┓┏┓┓┏┏┓┳┓┏┓  ┳┳┓┏┓┳┳┓┏┓┳┓┓┏\n";
    cout << "┗┓┣┫┃┃┣┫┃┃┣┫  ┃┃┃┣ ┃┃┃┃┃┣┫┗┫\n";
    cout << "┗┛┛┗┗┛┛┗┛┗┛┗  ┛ ┗┗┛┛ ┗┗┛┛┗┗┛\n";

    cout << "\nWelcome to the demand paged memory simulator!\n";

    // preview jobs and empty memory
    previewJobs(jobs);
    showEmptyMemory();

    int choice;
    do {
        cout << "\nmain menu\n";
        cout << "1. Simulate demand paging\n";
        cout << "2. View tables\n";
        cout << "3. Resolve address (with demand paging)\n";
        cout << "4. View memory stats\n";
        cout << "5. Set replacement algorithm\n";
        cout << "6. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            simulateDemandPaging(jobs);
        } 
        else if (choice == 2) {
            displayTables(jobs);
        } 
        else if (choice == 3) {
            cout << "\ncurrent memory map:\n";
            for (auto &frame : memoryFrames) {
                if (!frame.isFree) {
                    cout << "frame " << frame.frameID << " -> job " << frame.jobID 
                                << " page " << frame.pageNumber << "\n";
                }
            }
            int jobID, addr;
            cout << "Enter job id: ";
            cin >> jobID;

            auto it = find_if(jobs.begin(), jobs.end(), [jobID](Job &j){ return j.jobID == jobID; });
            if (it != jobs.end()) {
                // show the valid address range for this job
                cout << "Job " << jobID << " has size " << it->jobSize 
                    << " Bytes (valid logical addresses: 0 - " << (it->jobSize - 1) << ")\n";
        
                cout << "Enter logical address to resolve (e.g., 0, 128, 512...): ";
                cin >> addr;

                resolveAddress(*it, addr, jobs);
            } else {
                cout << "Job id not found.\n";
            }
        }
        else if (choice == 4) {
            showMemoryStats();
        }
        else if (choice == 5) {
            setReplacementAlgorithm();
        }
    } while (choice != 6);
    cout << "Exiting simulator. Goodbye!\n";
    return 0;
}



// All TODO items have been completed:
// ✓ Removed internal fragmentation logic from demand paging paths
// ✓ Removed menu option 6 (Load Specific Pages)
// ✓ Fixed simulator with proper break conditions
// ✓ Added function comments
// ✓ Enhanced page map table with Modified and Referenced bits