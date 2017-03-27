#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include "common_data.h"

#define nap usleep(((rand() %800) +200)*1000)
#define maxtime 10



//char buff[85];

pthread_mutex_t i_mutex = PTHREAD_MUTEX_INITIALIZER;


unsigned long current_time(void){
	unsigned long   int         us; // Microseconds
    time_t          s;  // Seconds
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	s  = spec.tv_sec;
    us = /*round*/(spec.tv_nsec / 1.0e3); // Convert nanoseconds to microseconds
    return us;
}

unsigned long getsecs(void){

	unsigned long            us; // Microseconds
    time_t          s;  // Seconds
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	s  = spec.tv_sec;
	return s;

}

//BUS DAEMON
void* bus_daemon_func(void *params){
	/*INITIALIZE FILE DESCRIPTOR*/
	int fd1 = 0, fd2 = 0, fd3 = 0, fd4 = 0;
	
	int dest_id;

	struct message msg;
	
	long t = current_time();
	srand((unsigned) t);
	
	int res, wr;	
	unsigned long start_time = getsecs();

	fd1 = open("/dev/squeue1", O_RDWR);
	if (fd1 < 0 ){
		printf("Daemon Can not open device file Q1.\n");
		return;
	}

	fd2 = open("/dev/squeue2", O_RDWR);
	if (fd2 < 0 ){
		printf("\nDaemon Can not open device file Q2.\n");
		return;
	}

	fd3 = open("/dev/squeue3", O_RDWR);
	if (fd3 < 0 ){
		printf("\nDaemon Can not open device file Q3.\n");
		return;	
	}

	fd4 = open("/dev/squeue4", O_RDWR);
	if (fd4 < 0 ){
		printf("\nDaemon Can not open device file Q4.\n");
		return;
	}	


	while(getsecs()- start_time <= maxtime){
		
		dest_id = -1;
		wr= -1; res = -1;
		
		/*Read from squeue1*/
		while(res == -1 && (getsecs()- start_time <= maxtime)){
			res = read(fd1, &msg, sizeof(msg));
		if(res<0)
			nap;
	   else
			printf("\ndaemon read succesfully:");
		
		}
		
		/* 		Write to op bus			*/
		while(wr==-1 && getsecs()- start_time <= maxtime){
			printf("\nBus Daemon writing data to");
			dest_id = msg.receiver_id-1;
			printf(" des id:  %d", dest_id);
			
			/*	SELECT CHECK DESTINATION ID	 & WRITE DATA	*/
			switch(dest_id){
				case(0):
				
				while(wr==-1 && (getsecs()- start_time <= maxtime)){
					wr = write(fd2, &msg, sizeof(msg));
					if(wr==-1){
							//printf("\nDaemon Write unsuccesful R1:\n");	
							nap;
					}
					else{
						printf("\ndaemon write succesfully R1:");
						//printf("struct: \nMess Id: %d\nSender ID %d\nRecr ID %d\nchar length: %d\nTSC: %llu \nMESS: %s\n", msg.mess_id, msg.sender_id, 	msg.receiver_id, msg.char_length, msg.tsc, msg.data);
					}
				}
				break;
				case(1):
				while(wr==-1 && (getsecs()- start_time <= maxtime)){
					wr = write(fd3, &msg, sizeof(msg));
					if(wr==-1){
						nap;
						//printf("\ndaemon write unsuccesfully R1:\n");			
					}
					else{
					printf("\ndaemon write succesfully R1:\n");
					//printf("struct: \nMess Id: %d\nSender ID %d\nRecr ID %d\nchar length: %d\nTSC: %llu \nMESS: %s\n", msg.mess_id, msg.sender_id, 	msg.receiver_id, msg.char_length, msg.tsc, msg.data);
					}
				}
				break;
				
				case(2):
				while(wr==-1 && (getsecs()- start_time <= maxtime)){
					wr = write(fd4, &msg, sizeof(msg));
					if(wr==-1){
						nap;
						//printf("\ndaemon write unsuccesfully R1:\n");			
					}
					else{
					printf("\ndaemon write succesfully R1:\n");
					//printf("struct: \nMess Id: %d\nSender ID %d\nRecr ID %d\nchar length: %d\nTSC: %llu \nMESS: %s\n", msg.mess_id, msg.sender_id, 	msg.receiver_id, msg.char_length, msg.tsc, msg.data);
					}
					
				}
				break;
				
				default:
					printf("\n\n\nincorrect dest id %d:",dest_id);
				
			}

		}
	}
	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);
}

// SEND FUNC 1
void* sender_func1(void *params){
	struct message msg;
	int res =-1;
	int fd =-1;
	
	unsigned long start_time = getsecs();

	fd = open("/dev/squeue1", O_RDWR);		
	if (fd < 0 ){
			printf("\nCan not open device file Q1.\n");
			return;
		}	

	unsigned long t = current_time();
	srand((unsigned) t);
	
	while(getsecs()- start_time <= maxtime){
		
		res =-1;
		
		//Get Global Message ID & UPDATE IT
		pthread_mutex_lock(&i_mutex);
		int* msg_id = (int*)params;
		msg.mess_id = *msg_id;
		(*msg_id) = (*msg_id)++;
		pthread_mutex_unlock(&i_mutex);
		
		msg.sender_id = 1;
		msg.receiver_id = rand() % 3 + 1;
		msg.char_length = (rand() % 80) + 1;
		msg.tsc=0;
		
		//Generate Random Message
		char buff[msg.char_length+1];
		int i=0;
		for(i=0;i < msg.char_length ; i++)
			buff[i] = (rand() % 26) + 65; // ASCII: A to Z
		buff[msg.char_length] = '\0'; // null termination
		
		msg.data = malloc ((msg.char_length+1) * sizeof(char));
		memcpy( msg.data, buff , strlen(buff) );
		
		/*WRITE DATA TO KERNEL*/
		while( res == -1 && (getsecs() - start_time <= maxtime) ){
			
			res = write(fd, &msg, sizeof(msg));
			
			if(res==-1){
				printf("\nS1: bus_out_q1 full");
				nap;
			}
			else{
				printf("\nS1: data written succesfully Q1:\n");
			}
		}
		printf("\nSNDR1:\nMess Id: %d\nSender ID %d\nRecr ID %d\nChar length: %d\nTSC: %llu \nMESS: %s\n", msg.mess_id, 
					msg.sender_id, 	msg.receiver_id, msg.char_length, msg.tsc, msg.data);

		nap;
	}
	close(fd);
}


// SEND FUNC 2
void* sender_func2(void *params){	
	struct message msg;
	int res =-1;
	int fd =-1;
	
	unsigned long start_time = getsecs();

	fd = open("/dev/squeue1", O_RDWR);		
	if (fd < 0 ){
			printf("\nCan not open device file Q1.\n");
			return;
		}	

	unsigned long t = current_time();
	srand((unsigned) t);
	
	while(getsecs()- start_time <= maxtime){
		res =-1;
	
		//Get Global Message ID & UPDATE IT
		pthread_mutex_lock(&i_mutex);
		int* msg_id = (int*)params;
		msg.mess_id = *msg_id;
		(*msg_id) = (*msg_id)++;
		pthread_mutex_unlock(&i_mutex);
		
		msg.sender_id = 2;
		msg.receiver_id = rand() % 3 + 1 ;
		msg.char_length = (rand() % 80) + 1;
		msg.tsc=0;
		
		//Generate Random Message
		char buff[msg.char_length+1];
		int i=0;
		for(i=0;i < msg.char_length ; i++)
			buff[i] = (rand() % 26) + 65; // ASCII: A to Z
		buff[msg.char_length] = '\0'; // null termination
		
		msg.data = malloc ((msg.char_length+1) * sizeof(char));
		memcpy( msg.data, buff , strlen(buff) );


		/*WRITE DATA TO KERNEL*/
		while( res == -1 && (getsecs() - start_time <= maxtime)){
			res = write(fd, &msg, sizeof(msg));
			if(res==-1){
				printf("\nS2: bus_out_q1 full");
				nap;
			}
			else{
				//printf("\nS2: data written succesfully Q1:\n");
			}
		}
	printf("\nSNDR2: \nMess Id: %d\nSender ID %d\nRecr ID %d\nChar length: %d\nTSC: %llu \nMESS: %s\n", msg.mess_id, 
			msg.sender_id, msg.receiver_id, msg.char_length, msg.tsc, msg.data);
	
	nap;
	}
	close(fd);
}



// SEND FUNC 3
void* sender_func3(void *params){
	
	struct message msg;
	int res =-1;
	int fd =-1;
	
	unsigned long start_time = getsecs();

	fd = open("/dev/squeue1", O_RDWR);		
	if (fd < 0 ){
			printf("\nCan not open device file Q1.\n");
			return;
		}	

	unsigned long t = current_time();
	srand((unsigned) t);
	
	while(getsecs()- start_time <= maxtime){
		res =-1;
	
		//Get Global Message ID & UPDATE IT
		pthread_mutex_lock(&i_mutex);
		int* msg_id = (int*)params;
		msg.mess_id = *msg_id;
		(*msg_id) = (*msg_id)++;
		pthread_mutex_unlock(&i_mutex);
		
		msg.sender_id = 3;
		msg.receiver_id = rand() % 3 + 1 ;
		msg.char_length = (rand() % 80) + 1;
		msg.tsc=0;
		
		//Generate Random Message
		char buff[msg.char_length+1];
		int i=0;
		for(i=0;i < msg.char_length ; i++)
			buff[i] = (rand() % 26) + 65; // ASCII: A to Z
		buff[msg.char_length] = '\0'; // null termination
		
		msg.data = malloc ((msg.char_length+1) * sizeof(char));
		memcpy( msg.data, buff , strlen(buff) );
	
		/*WRITE DATA TO KERNEL*/
		while( res == -1 && (getsecs() - start_time <= maxtime)){
			res = write(fd, &msg, sizeof(msg));
			if(res==-1){
				printf("\nS2: bus_out_q1 full");
				nap;
			}
			else{
				//printf("\nS3: data written succesfully Q1:\n");
			}
		}
	printf("\nSNDR3:\nMess Id: %d\nSender ID %d\nRecr ID %d\nChar length: %d\nTSC: %llu \nMESS: %s\n", msg.mess_id, 
					msg.sender_id, 	msg.receiver_id, msg.char_length, msg.tsc, msg.data);
	nap;

	}
	close(fd);

}




// RCVR FUNC 1
void* rcvr_func1(void *params){
	int fd=-1;
	int res=-1;
	
	struct message msg;
	
	unsigned long t = current_time();
	srand((unsigned) t);
	
	fd = open("/dev/squeue2", O_RDWR);
	if (fd < 0 ){
			printf("\nCan not open device file R1.\n");
			return;
	}
		
	unsigned long start_time = getsecs();
	/*READ DATA FROM KERNEL*/
	while((getsecs()- start_time <= maxtime)){
		res =-1;
		while( res == -1 && (getsecs()- start_time <= maxtime)){
			res = read(fd, &msg, sizeof(msg));
			if(res==-1){
				printf("\nR1 bus_out_q2 Empty");
				nap;
			}
			
			else{
				printf("\n\n\tRCVR1: \n\tMess Id: %d\n\tSender ID %d\n\tRecr ID %d\n\tchar length: %d\n\tTSC: %llu \n\tMESS: %s\n", msg.mess_id, msg.sender_id, 	msg.receiver_id, msg.char_length, msg.tsc, msg.data);
			}
		}
		free(msg.data);
		nap;
	}
	close(fd);
}

// RCVR FUNC 2
void* rcvr_func2(void *params){
	int fd=-1;
	int res=-1;
	
	struct message msg;
	
	unsigned long t = current_time();
	srand((unsigned) t);
	
	fd = open("/dev/squeue3", O_RDWR);
	if (fd < 0 ){
			printf("\nCan not open device file R2.\n");
			return;
	}
		
	unsigned long start_time = getsecs();
	/*READ DATA FROM KERNEL*/
	while((getsecs()- start_time <= maxtime)){
		res =-1;
		while( res == -1 && (getsecs()- start_time <= maxtime)){
			res = read(fd, &msg, sizeof(msg));
			if(res==-1){
				printf("\nR2 bus_out_q3 Empty");
				nap;
			}
			
			else{
				printf("\n\n\tRCVR2: \n\tMess Id: %d\n\tSender ID %d\n\tRecr ID %d\n\tchar length: %d\n\tTSC: %llu \n\tMESS: %s\n", msg.mess_id, msg.sender_id, 	msg.receiver_id, msg.char_length, msg.tsc, msg.data);
			}
		}
		free(msg.data);
		nap;
	}
	close(fd);
}

// RCVR FUNC 3
void* rcvr_func3(void *params){
	int fd=-1;
	int res=-1;
	
	struct message msg;
	
	unsigned long t = current_time();
	srand((unsigned) t);
	
	fd = open("/dev/squeue4", O_RDWR);
	if (fd < 0 ){
			printf("\nCan not open device file R3.\n");
			return;
	}
		
	unsigned long start_time = getsecs();
	/*READ DATA FROM KERNEL*/
	while((getsecs()- start_time <= maxtime)){
		res =-1;
		while( res == -1 && (getsecs()- start_time <= maxtime)){
			res = read(fd, &msg, sizeof(msg));
			if(res==-1){
				printf("\nR3 bus_out_q4 Empty");
				nap;
			}
			
			else{
				printf("\n\n\tRCVR3: \n\tMess Id: %d\n\tSender ID %d\n\tRecr ID %d\n\tchar length: %d\n\tTSC: %llu \n\tMESS: %s\n", msg.mess_id, msg.sender_id, msg.receiver_id, msg.char_length, msg.tsc, msg.data);
			}
		}
		free(msg.data);
		nap;
	}
	close(fd);
}



int main(int argc, char** argv)
{

	pthread_t sender1, sender2, sender3 , recvr1, recvr2, recvr3, bus_daemon;
	
	unsigned long t=current_time();
	
	srand((unsigned) t);

	int c=0;
	int msg_id2=0;
	
	
	unsigned long a=0;
	a=current_time();

	printf("\nInitializing Threads\n");
	
	/*	THREADS		*/
	pthread_create(&sender1, NULL, sender_func1, (void*) &msg_id2);
	
	pthread_create(&sender2, NULL, sender_func2, (void*)&msg_id2);
	
	pthread_create(&sender3, NULL, sender_func3, (void*)&msg_id2);
	
	pthread_create(&recvr1, NULL, rcvr_func1, NULL);
	
	pthread_create(&recvr2, NULL, rcvr_func2, NULL);
	
	pthread_create(&recvr3, NULL, rcvr_func3, NULL);
	
	pthread_create(&bus_daemon, NULL, bus_daemon_func, NULL);
	
	printf("\nWaiting for Threads...\n");
	
	pthread_join(sender1, NULL);
	pthread_join(sender2, NULL);
	pthread_join(sender3, NULL);
	printf("\nSNDRS JOINED...\n");
	
	
	pthread_join(recvr1, NULL);
	pthread_join(recvr2, NULL);
	pthread_join(recvr3, NULL);
	
	printf("\nRCVRS JOINED...\n");
	pthread_join(bus_daemon, NULL);
	printf("\n\nExiting Program ...\n");
	
	return 0;

}



