
/* DEMAND PAGING MEMORY ALLOCATION SIMULATION
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
#include <vector>//for dynamic array
#include <queue>//for queue implementation
#include <cstdlib>//for rand() and srand()
#include <ctime>
#include <fstream>//for file handling
#include <sstream>//for string stream to parse file - csv
#include <iomanip>
#include <algorithm>//for count_if
#include <thread>//for sleep in simulate
#include <chrono>//for sleep in simulate
#include <random>
#include <unordered_map>
#include <limits>
using namespace std;

struct Job {
    int jobID = -1;
    int jobSize = 0;
    int pageSize = 0;
    int internalFragmentation = 0;
    vector<int> pages;
    unordered_map<int,int> pageTable; // page -> frame
    int arrivalTime = 0; // event-driven
    int duration = 0;    // how many time units the job runs before completion
    int startTime = -1;  // when actually allocated
};

struct PageFrame {
    int frameID = -1;
    int frameSize = 0;
    bool isFree = true;
    int jobID = -1;
    int pageNumber = -1;
};

enum EventType { ARRIVAL=0, COMPLETE=1 };

struct Event {
    int time;
    EventType type;
    int jobID;
    // For priority queue ordering (min-heap by time, then type)
    bool operator<(const Event &other) const {
        if (time != other.time) return time > other.time; // reversed for priority_queue
        return type > other.type;
    }
};

vector<PageFrame> memoryFrames;
vector<Job> jobs;
std::mt19937 rng{static_cast<unsigned>(time(nullptr))};

// --- Helpers ---
void initFrames(int numFrames, int frameSize) {
    memoryFrames.clear();
    memoryFrames.reserve(numFrames);
    for (int i = 0; i < numFrames; ++i)
        memoryFrames.push_back(PageFrame{i, frameSize, true, -1, -1});
}

//division of jobs into pages is based on frame size
void divideJobIntoPages(Job &job) {
    job.pages.clear();
    job.pageTable.clear();
    if (job.pageSize <= 0) return;
    int numPages = job.jobSize / job.pageSize;
    int remaining = job.jobSize % job.pageSize;
    if (remaining > 0) { ++numPages; job.internalFragmentation = job.pageSize - remaining; }
    else job.internalFragmentation = 0;
    for (int i=0;i<numPages;++i) job.pages.push_back(i);
}

// Efficient random assignment using shuffle of free frames
bool assignPageFrames(Job &job) {
    vector<int> freeIndices;
    for (size_t i = 0; i < memoryFrames.size(); ++i)
        if (memoryFrames[i].isFree) freeIndices.push_back((int)i);

    if (job.pages.size() > freeIndices.size()) return false; // not enough

    shuffle(freeIndices.begin(), freeIndices.end(), rng);
    for (size_t i = 0; i < job.pages.size(); ++i) {
        int frameIndex = freeIndices[i];
        memoryFrames[frameIndex].isFree = false;
        memoryFrames[frameIndex].jobID = job.jobID;
        memoryFrames[frameIndex].pageNumber = job.pages[i];
        job.pageTable[job.pages[i]] = memoryFrames[frameIndex].frameID;
    }
    return true;
}

void freeJobFrames(int jobID) {
    for (auto &f : memoryFrames) {
        if (!f.isFree && f.jobID == jobID) {
            f.isFree = true;
            f.jobID = -1;
            f.pageNumber = -1;
        }
    }
}

int usedFramesCount() {
    return (int)count_if(memoryFrames.begin(), memoryFrames.end(), [](const PageFrame &f){ return !f.isFree; });
}

void displayMemoryMap() {
    cout << "\nMemory Map:\n";
    cout << left << setw(10) << "FrameID" << setw(10) << "Status" << setw(10) << "JobID" << setw(10) << "Page\n";
    for (auto &f : memoryFrames) {
        cout << left << setw(10) << f.frameID;
        if (f.isFree) cout << setw(10) << "Free" << setw(10) << "-" << setw(10) << "-" << "\n";
        else cout << setw(10) << "Occupied" << setw(10) << f.jobID << setw(10) << f.pageNumber << "\n";
    }
}

// Address resolution — verified displacement algorithm
void resolveAddress(const Job &job, int logicalAddress) {
    if (job.pageSize <= 0) {
        cout << "Invalid page size.\n"; return;
    }
    if (logicalAddress < 0 || logicalAddress >= job.jobSize) {
        cout << "Logical address " << logicalAddress << " out of bounds for Job " << job.jobID << "\n";
        return;
    }

    int pageNumber = logicalAddress / job.pageSize;
    int offset = logicalAddress % job.pageSize;

    // check offset vs frame size for safety
    auto it = job.pageTable.find(pageNumber);
    if (it == job.pageTable.end()) {
        cout << "Page " << pageNumber << " not loaded in memory for Job " << job.jobID << "\n";
        return;
    }
    int frameNumber = it->second;
    if (frameNumber < 0 || frameNumber >= (int)memoryFrames.size()) {
        cout << "Invalid frame number mapped: " << frameNumber << "\n"; return;
    }

    int frameSize = memoryFrames[frameNumber].frameSize;
    if (offset >= frameSize) {
        cout << "Offset " << offset << " exceeds frame size " << frameSize << " — mapping invalid.\n"; return;
    }

    long long physical = (long long)frameNumber * (long long)frameSize + (long long)offset;
    cout << "Logical " << logicalAddress << " -> Page " << pageNumber << " Offset " << offset
         << " -> Physical " << physical << " (Frame " << frameNumber << " | frameSize " << frameSize << ")\n";
}

// CSV import — format: jobID,jobSize,arrival,duration
vector<Job> importJobsFromFile(const string &filename, int pageSize) {
    vector<Job> out;
    ifstream ifs(filename);
    if (!ifs.is_open()) {
        cerr << "Failed to open " << filename << "\n"; return out;
    }
    string line;
    bool first = true;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        // skip header if not numeric
        if (first) {
            string trimmed = line;
            trimmed.erase(remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());
            if (!trimmed.empty() && !isdigit(static_cast<unsigned char>(trimmed[0]))) { first = false; continue; }
            first = false;
        }

        stringstream ss(line);
        string tok;
        Job j;
        if (!getline(ss, tok, ',')) continue;
        try { j.jobID = stoi(tok); } catch(...) { continue; }
        if (!getline(ss, tok, ',')) continue;
        try { j.jobSize = stoi(tok); } catch(...) { continue; }
        // arrival
        if (!getline(ss, tok, ',')) { j.arrivalTime = 0; }
        else {
            try { j.arrivalTime = stoi(tok); } catch(...) { j.arrivalTime = 0; }
        }
        // duration
        if (!getline(ss, tok, ',')) { j.duration = max(1, j.jobSize / 500); } // default heuristic
        else {
            try { j.duration = stoi(tok); } catch(...) { j.duration = max(1, j.jobSize / 500); }
        }
        j.pageSize = pageSize;
        divideJobIntoPages(j);
        out.push_back(j);
    }
    return out;
}
//framees are allocated jobs if enough free frames are open. else, jobs wait in queue.
// Event-driven simulator
void runSimulator(vector<Job> &jobs, int maxTimeUnits = 100, int tickMs = 200) {
    priority_queue<Event> pq;
    unordered_map<int, Job*> jobMap;
    for (auto &j : jobs) {
        pq.push(Event{j.arrivalTime, ARRIVAL, j.jobID});
        jobMap[j.jobID] = &j;
    }

    queue<int> waitingQueue; // jobIDs waiting for frames

    int currentTime = 0;
    while (!pq.empty() || !waitingQueue.empty()) {
        // process all events scheduled at or before currentTime
        while (!pq.empty() && pq.top().time <= currentTime) {
            Event ev = pq.top(); pq.pop();
            if (ev.type == ARRIVAL) {
                cout << "[t=" << currentTime << "] ARRIVAL: Job " << ev.jobID << "\n";
                Job *j = jobMap[ev.jobID];
                if (!assignPageFrames(*j)) {
                    cout << "  -> Not enough frames now. Putting Job " << j->jobID << " in waiting queue.\n";
                    waitingQueue.push(j->jobID);
                } else {
                    j->startTime = currentTime;
                    int completionTime = currentTime + j->duration;
                    cout << "  -> Allocated Job " << j->jobID << " (will complete at t=" << completionTime << ")\n";
                    pq.push(Event{completionTime, COMPLETE, j->jobID});
                }
            } else if (ev.type == COMPLETE) {
                cout << "[t=" << currentTime << "] COMPLETE: Job " << ev.jobID << "\n";
                // free frames
                freeJobFrames(ev.jobID);
                // clear pageTable for that job (reflect unload)
                Job *j = jobMap[ev.jobID];
                if (j) j->pageTable.clear();

                // attempt to allocate waiting jobs in FIFO order
                int waitingCount = (int)waitingQueue.size();
                for (int i = 0; i < waitingCount; ++i) {
                    int jid = waitingQueue.front(); waitingQueue.pop();
                    Job *wj = jobMap[jid];
                    if (!assignPageFrames(*wj)) {
                        // still can't allocate, re-enqueue
                        waitingQueue.push(jid);
                    } else {
                        wj->startTime = currentTime;
                        int ct = currentTime + wj->duration;
                        cout << "  -> Allocated waiting Job " << wj->jobID << " (complete at t=" << ct << ")\n";
                        pq.push(Event{ct, COMPLETE, wj->jobID});
                    }
                }
            }
        }

        // show snapshot
        cout << "  Memory Usage: " << usedFramesCount() << "/" << memoryFrames.size()
             << " frames used. Waiting queue: " << waitingQueue.size() << "\n";

        // advance time by 1 unit
        ++currentTime;
        if (currentTime > maxTimeUnits) {
            cout << "Reached maxTimeUnits limit, stopping simulation.\n";
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(tickMs));
    }

    cout << "Simulation finished at t=" << currentTime << ". Final memory map:\n";
    displayMemoryMap();
}

// ---- Main ----
int main() {
    cout << "Paged Memory Allocation — Event Driven Simulator\n";

    int numFrames = 8;
    int frameSize = 512;
    string fname = "jobs.csv";

    cout << "Using defaults: frames=" << numFrames << ", frameSize=" << frameSize
         << ", CSV='" << fname << "'.\n";

    initFrames(numFrames, frameSize);

    jobs = importJobsFromFile(fname, frameSize);
    if (jobs.empty()) {
        cout << "No jobs loaded from CSV '" << fname << "'. Create a CSV with lines like:\n";
        cout << "jobID,jobSize,arrival,duration\n";
        cout << "Example:\n1,1000,0,5\n2,2048,1,8\n";
        return 1;
    }

    cout << "Loaded " << jobs.size() << " jobs. Preview:\n";
    cout << left << setw(8) << "JobID" << setw(10) << "Size" << setw(10) << "Pages"
         << setw(10) << "Arrival" << setw(10) << "Duration" << setw(12) << "Frag\n";
    for (auto &j : jobs) {
        cout << left << setw(8) << j.jobID << setw(10) << j.jobSize << setw(10) << j.pages.size()
             << setw(10) << j.arrivalTime << setw(10) << j.duration << setw(12) << j.internalFragmentation << "\n";
    }

    // run simulation
    runSimulator(jobs, 200, 200);

    // interactive address resolve demo
    cout << "\nResolve logical address demo. Enter JobID and logical address (or 0 0 to exit):\n";
    while (true) {
        int jid, addr;
        cout << "JobID addr> ";
        if (!(cin >> jid >> addr)) break;
        if (jid == 0 && addr == 0) break;
        auto it = find_if(jobs.begin(), jobs.end(), [jid](const Job &j){ return j.jobID == jid; });
        if (it == jobs.end()) { cout << "Job not found.\n"; continue; }
        resolveAddress(*it, addr);
    }

    cout << "Done.\n";
    return 0;
}
