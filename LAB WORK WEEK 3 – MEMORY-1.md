ASHESI UNIVERSITY
FACULTY OF ENGINEERING & COMPUTER SCIENCE
DEPARTMENT OF COMPUTER SCIENCE &
 INFORMATION SYSTEMS
CS433 OPERATING SYSTEMS
INSTRUCTOR: CHARLES ADJETEY

LAB WORK WEEK 3 – MEMORY

TASK 1 - WINDOWS

Run the Process Explorer utility (Available at sysinternals). Click on the System Information button (Ctrl+I)
to see details of memory usage activity in your system.

Provide information about the usage of physical memory, kernel memory and paging in your system. Now
start  a  new program, e.g. Microsoft Excel. Describe  how this  usage pattern changes  when  you start  this
program, a minute after you have started this program, and when you terminate this program. Provide an
explanation for this usage pattern.

Link for sysinternals https://docs.microsoft.com/en-us/sysinternals

TASK 2 - LINUX

free  is  a  command that displays used and available memory in your system. Run the  command free  –o
several times, running other programs in between, and store the results in a file. Draw a graph as follows:
X-axis: MB-used; for the Y-axis, use (i) Memory Used per unit time; (ii) (Memory Used – Memory Buffered
– Memory Cached) per unit time; and (iii) Swap Used per unit time. Explain the behavior of this graph with
respect to memory utilization in the presence of running various applications.

Read more about free: free Command in Linux with examples - GeeksforGeeks

TASK 3 – PROGRAMMING

Write a program in C or C++ that simulate the following:

Paged Memory Allocation:
•  Accept a Job
•  Divide the Job into pages based on a specify size
•
Indicate internal fragmentation if any
•  Load jobs into Page Frames randomly
•  Perform Address resolution

Demand Paged Memory Allocation:

•  Accept more than one Job
•  Divide the Jobs into pages based on a specify size
•  Load jobs into Memory Frames randomly

•  Perform Address resolution

Virtual  Memory:  Explore  virtual  memory  concepts  by  writing  programs  that  simulate  page  replacement
algorithms FIFO, and LRU (Least Recently Used).

