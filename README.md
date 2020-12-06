# Pintos

This is a course project of Operating System lecture at Sogang University.

## Project 1: Threads

All 29 tests passed.

## Project 2: User Programs

All 80 tests passed.

### System Design Rationale

In Pintos, threads and user processes are one-to-one mapping. Thus, Each thread
contains a process control block of its corresponding user process. You can find
the definition of process control block in struct process in userprog/process.h.

Each thread, even the main thread, can have a child or children. Since a child
thread can exit while its process control block is remaining on memory, a parent
thread has a list of pointers to the process control blocks of its children.

## Built With

* Ubuntu 20.04.1 LTS
* QEMU emulator version 4.2.1 (Debian 1:4.2-3ubuntu6.10)
* gcc version 9.3.0 (Ubuntu 9.3.0-10ubuntu2) 

## License

This project is licensed under the MIT License.
See [LICENSE](LICENSE) for details.
