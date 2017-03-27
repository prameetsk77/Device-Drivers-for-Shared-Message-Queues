ThE "common_data.h" is included in both user space and kernel space. It is the declaration of the message struct(P.S. IT DOES NOT DEFINE AN INSTANCE OF THE struct message).

Main.c -> User Space Code.
squeue.c -> Kernel Space Code.

Referred Link for RDTSC:
http://howscripts.xyz/question/12631856/difference-between-rdtscp-rdtsc-memory-and-cpuid-rdtsc
________________________________________

I had mistakenly made the code another implementation of the queue. I have included that in the "OTHER CODE" folder.



________________________________________
