# Multi-thread-process
The aim of this experiment is to make you familiar with some concepts of multi-thread processes and interprocess communication and have practical experience on interprocess communication programming using UNIX and POSIX facilities.

I implemented a multi process and multi-thread encryption application. 
The main process works in a multi-threaded structure, and in-task threads contain "producer consumer" relations. 
The main application will fork next child after it created. I  used POSIX thread structures in multi-thread developing.


The main process encrypt data with the same pattern described in 1st homework. The main process reads a plain text from the file named plain.txt and a 128-bit key from the file named key.txt.
A key must be 128-bit length but plain text may be longer than 128-bit, but it contains exactly fixed-size blocks, e.g. 64 bytes of plain text consists of 8 blocks.

After reading both key and plain data, the main process first writes key file and after 128-bit plain text blocks by dividing plain data to the next-child pipe. Then this process starts to read encrypted blocks from the last child process and writes them to encrypt.txt. All these three files will be in binary format.

these operations will be performed by different threads within the same task, not by different processes. In this context, these tasks do not need inter-process communication structures for data exchange because they use the same memory space. In the given assignment, the threads on the main application will work as producer consumers, and the thread responsible for each step will be the producer thread producing data for the thread responsible for the next step. Each thread will write the block data it generates to the corresponding queue, and the other thread will read from this queue and write the result to the corresponding queue after processing. When the thread is generating the data queue is full, or the consumed queue is empty, the thread will go into a wait state.

Separate from encryption threads, there will be another thread in this task that writes to the xor thread queue by reading plain data. This thread is the master thread for all encryption processes.

The second thread applies XOR operation. After reading plain block from plain data queue and applying XOR between the same length plain and key blocks, this process writes the intermediate result to the permutation queue.

The third thread applies permutation. In this step, data of length 8 bytes is divided into two blocks as the first 4 and last 4 bytes, and the byte data having the same indices as the two blocks are replaced with each other. For example, the byte at the 0th position replaces the byte at the 4th indent and the 1st byte and the 5th indent. After permutation, the result block is written to sub-box queue.

The fourth thread applies substitution. In this step, each byte of the block is replaced with a substitution value which is provided by a substitution-box. This substitution-box will be given in the context of the assignment. After substitution, the result block is written to authentication queue.
