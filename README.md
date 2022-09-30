# FIFO-Scheduling-Algorithm
An algorithm that schedules processes in a first-in-first-out (FIFO) priority method. 

Admitting new processes and labeling them as ready to run: 
The Program includes a class called ProcessManagement. When the program starts, it will read an input file that will provide details for the set of processes to run.
Each line in this input file indicates the following details for a process (in this order):
Arrival Time, Required CPU Time, [IO Event time, IO Event Duration]*

The arrival time indicates when a process will be added to the set of processes that you will be managing.
The required CPU time indicates how long each process should be on the CPU
And finally, each process may have zero or more (blocking) IO events that occur during their execution. These are defined by the time into the process execution that
they arrive (IO Event Time) and how long they take (IO Event Duration).

Handling running processes and what can happen while a process is running: 
When a process is running, the program will update the amount of time the process has spent on the processor and then check for either of the two following cases:
1. An IO request may need to be made. The program will submit an IO request to the ioModule object.
At this point, the process will be waiting for the IO event to complete and will not be run until notified that the IO is done.
2. The process may be finished running. At this point, it marks the process as being done and makes sure not to schedule it any more time on the processor.

Responding to interrupts that are raised when an IO operation is complete: 
The ioModule class will add IOInterrupt objects to this list at the appropriate time step after an IO request has been made.
(each process and IO event has a unique ID).
The program will take this information and make it so the indicated process is marked as ready to run again and clear the interrupt out of the list.
