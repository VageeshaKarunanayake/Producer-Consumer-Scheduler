# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <pthread.h>
#include <unistd.h>
#include <stdbool.h> 

//Create a structure of Ready Queue
struct R_Q {
	int prID;
	int prBT;
	time_t prAT;
};

//Declare the ready queue as global variable using the created structure
struct R_Q *R_QUEUE;

//Declaration of a variable to find the maximum size of ready queue 
int RQ_MAX;

//Declaration of a variable that will help to find whether all tasks are read from the task_file
bool T_RQ_OVER = false;

//Declaration of a variable to find the end of ready queue
int RQ_END = -1;

//File pointers,mutex and conditional variables declaration
FILE *fptr;
FILE *fptr1;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//CPU identification(C1,C2,C3),No of tasks added to the ready queue(NTA) and other required variable declaration
int C1 = 1,C2 = 2,C3 = 3,NTA = 0;
int num_tasks = 0,total_waiting_time = 0,total_turnaround_time = 0;

//Function prototypes
void* task();
void* cpu(void *args);
void arrayDel();

int main(int argc, char *argv[])
{
	// Pthread variable declaration and other required varaible declaration
	pthread_t task_T,cpu_T1,cpu_T2,cpu_T3;
	float ATT,AWT;
	char F_N[40];

	//Getting the main function parameter and assigning the parameter to 'RQ_MAX'. Atoi function is used to convert the string value to integer value  
	RQ_MAX = atoi(argv[2]);	

	//Dymanically assigning the the R_QUEUE array(Ready Queue) size 
    	R_QUEUE = malloc(RQ_MAX * sizeof *R_QUEUE);

	//Convert the string input (main function parameter) and sends formatted output to a string pointed to, by 'F_N'
	sprintf(F_N, "%s", argv[1]);

	//Opening the given file and assign the open file to 'fptr' file pointer
	fptr = fopen(F_N, "r");
	
	//Check whether the file was opened or not
	if (fptr == NULL)
	{
		printf("File not Found.\n");
		return -1;
	}
	
	//Opening the given file and assign the open file to 'fptr1' file pointer
	fptr1 = fopen("simulation_log", "a");

	//Check whether the file was opened or not
	if (fptr1 == NULL)
	{
		printf("File not Found.\n");
		return -1;
	}
	
	//Thread creation for Task, CPU:1, CPU:2 and CPU:3
	int rt0 = pthread_create(&task_T,NULL,task,NULL);
	int rt1 = pthread_create(&cpu_T1,NULL,cpu,(void *)&C1);
	int rt2 = pthread_create(&cpu_T2,NULL,cpu,(void *)&C2);
	int rt3 = pthread_create(&cpu_T3,NULL,cpu,(void *)&C3);
	
	//Check whether the all the threads were created or not
	if(rt0==0)
		printf("Task thread created successfully.\n");
	else{
		printf("Task thread not created.\n");
		return -1;
	}

	if(rt1==0)
		printf("CPU-1 thread created successfully.\n");
	else{
		printf("CPU-1 thread not created.\n");
		return -1;
	}

	if(rt2==0)
		printf("CPU-2 thread created successfully.\n");
	else{
		printf("CPU-2 thread not created.\n");
		return -1;
	}

	if(rt3==0)
		printf("CPU-3 thread created successfully.\n");
	else{
		printf("CPU-3 thread not created.\n");
		return -1;
	}

	//Commanding the main thread to wait until all the threads are executed using pthread_join function
	pthread_join(task_T,NULL);	
	pthread_join(cpu_T1,NULL);
	pthread_join(cpu_T2,NULL);
	pthread_join(cpu_T3,NULL);	

	//Calculating the average waiting time and average turnaround time
	AWT = (float)total_waiting_time/num_tasks;
	ATT = (float)total_turnaround_time/num_tasks;

	//Writing the number of task,average waiting time and average turnaround time to the simulation_log
	fprintf(fptr1, "Number of tasks: %d\n",num_tasks);
	fprintf(fptr1, "Average waiting time: %.2f\n",AWT);
	fprintf(fptr1, "Average turnaround time: %.2f\n",ATT);

	// close the file pointers and free the dynamically allocated memory of 'R_QUEUE' using free function
	fclose(fptr);
	fclose(fptr1);
	free(R_QUEUE);

	return 0;
}

void* task()
{
	//variable declaration for storing process ID(pID) and process burst time(pBT)
        char pID[10];
        char pBT[10];

	//Infinite loop
	while(1)
	{
		if((RQ_END+2) <= (RQ_MAX-1)) //Check whether ready queue has 2 slots to write task data
		{

			//Acquiring the lock of 'mutex' mutex variable
			pthread_mutex_lock(&mutex);

			for(int i=1;i <= 2; i++) //Loop to insert 2 tasks to ready queue
			{
		        	if(fscanf(fptr,"%s",pID) != EOF) //Get the task ID and check whether the 'task_file' is not empty
				{
		        		if(fscanf(fptr,"%s",pBT) != EOF) //Get the task burst time and check whether the 'task_file' is not empty
					{
						//increase the ready queue end by one
						RQ_END++;

						//Assign the values that were received from 'task_file' to the ready queue  
						R_QUEUE[RQ_END].prID = atoi(pID);
						R_QUEUE[RQ_END].prBT = atoi(pBT);

						//Printing the process ID and process burst time that was added to the ready queue 
						printf("Added to Ready Queue. Process ID :%d    Process Burst Time :%d\n",R_QUEUE[RQ_END].prID,R_QUEUE[RQ_END].prBT);

						//Get the current time,assign the current time to the ready queue,write the process details and the process arrival time as the current time to the simulation_log
						time_t T= time(NULL);
						R_QUEUE[RQ_END].prAT = T;
	    					struct  tm tm = *localtime(&T);
						fprintf(fptr1, "\n%s : %s\n",pID,pBT);
						fprintf(fptr1, "Arrival time: %02d:%02d:%02d\n",tm.tm_hour, tm.tm_min, tm.tm_sec);
						
						//Increase the number of tasks added to the ready queue by one
						NTA++;							 
					}else{
						//This will be executed if the process burst time is missing

						printf("Error Occurred.Task File Missing Burntime of Task No: %s\n",pID);

						//Releasing the lock of 'mutex' mutex variable
						pthread_mutex_unlock(&mutex);

						goto END;
					}
				}else{
					//This will be executed if the 'task_file' is empty

					printf("All the tasks in Task file have been served.\n");

					//Get the current time, write the time of the ready queue termination as the current time and number of tasks put into ready queue to simulation_log					
					time_t TE= time(NULL);
	    				struct  tm tmE = *localtime(&TE);
					fprintf(fptr1, "\nNumber of tasks put into Ready-Queue: %d\n",NTA);
					fprintf(fptr1, "The %02d:%02d:%02d is the time it terminates.\n",tmE.tm_hour, tmE.tm_min, tmE.tm_sec);

					//To tell the other threads that task_file is empty
					T_RQ_OVER = true;

					//Releasing the lock of 'mutex' mutex variable
					pthread_mutex_unlock(&mutex);

					//Go to a specific location (END) of the code					
					goto END;
				}
			}

			//Releasing the lock of 'mutex' mutex variable
			pthread_mutex_unlock(&mutex);

		}else if((RQ_END+1) <= RQ_MAX - 1) //Check whether ready queue has a slot to write task data
		{
			//Acquiring the lock of 'mutex' mutex variable
			pthread_mutex_lock(&mutex);

		       	if(fscanf(fptr,"%s",pID) != EOF) //Get the task ID and check whether the 'task_file' is not empty
			{
	        		if(fscanf(fptr,"%s",pBT) != EOF) //Get the task burst time and check whether the 'task_file' is not empty
				{
					//Increase the ready queue end by one
					RQ_END++;

					//Assign the values that were received from 'task_file' to the ready queue  	
					R_QUEUE[RQ_END].prID = atoi(pID);
					R_QUEUE[RQ_END].prBT = atoi(pBT);

					//Printing the process ID and process burst time that was added to the ready queue 
					printf("Added to Ready Queue. Process ID :%d    Process Burst Time :%d\n",R_QUEUE[RQ_END].prID,R_QUEUE[RQ_END].prBT);

					//Get the current time,assign the current time to the ready queue,write the process details and the process arrival time as the current time to the simulation_log
					time_t T= time(NULL);
					R_QUEUE[RQ_END].prAT = T;
	 				struct  tm tm = *localtime(&T);
					fprintf(fptr1, "%s : %s\n",pID,pBT);
					fprintf(fptr1, "Arrival time: %02d:%02d:%02d\n",tm.tm_hour, tm.tm_min, tm.tm_sec);

					//Increase the number of tasks added to the ready queue by one
					NTA++;
				}else{
					//This will be executed if the process burst time is missing

					printf("Error Occurred.Task File Missing Burntime of Task No: %s\n",pID);

					//Releasing the lock of 'mutex' mutex variable
					pthread_mutex_unlock(&mutex);

					goto END;
				}
			}else{
				//This will be executed if the task_file is empty

				printf("All the tasks in Task file have been served.\n");

				//Get the current time, write the time of the ready queue termination as current time and number of tasks put into ready queue to simulation_log
				time_t TE= time(NULL);
	    			struct  tm tmE = *localtime(&TE);
				fprintf(fptr1, "Number of tasks put into Ready-Queue: %d\n",NTA);
				fprintf(fptr1, "The %02d:%02d:%02d is the time it terminates.\n",tmE.tm_hour, tmE.tm_min, tmE.tm_sec);

				//To tell the other threads that task_file is empty
				T_RQ_OVER = true;

				//Releasing the lock of 'mutex' mutex variable
				pthread_mutex_unlock(&mutex);

				//Go to a specific location (END) of the code
				goto END;
			}
		
			//Releasing the lock of 'mutex' mutex variable
			pthread_mutex_unlock(&mutex);

		}else{
			//Acquiring the lock of 'mutex' mutex variable
			pthread_mutex_lock(&mutex);

			//This will be executed when the ready queue is full
			//pthread_cond_wait() function will stop the thread until a signal is received with 'condition' conditional variable and release the lock of 'mutex' mutex variable
			pthread_cond_wait(&condition, &mutex);

			//Releasing the lock of 'mutex' mutex variable
			pthread_mutex_unlock(&mutex);
		}
		
	}
	//Specific location (END)
	END:pthread_exit(NULL); //pthread_exit() function is used to terminate a thread
}

void* cpu(void *args)
{
	//Declare and initialize the variables that are needed
	int PD,PB,NT = 0;
	time_t PA;
	
	//Declare a pointer and assign the received function parameter(CPU ID)
	int *CID = (int *) args;

	//Infinite loop
	while(1){

		//Acquiring the lock of 'mutex' mutex variable
		pthread_mutex_lock(&mutex);
		if(RQ_END <= -1 && T_RQ_OVER == false) // check whether the ready queue is empty and task_file is not empty
		{
			//The pthread_cond_signal() function sends a signal with 'condition' conditional variable to start the 'task' thread
			pthread_cond_signal(&condition);

			//Releasing the lock of 'mutex' mutex variable
			pthread_mutex_unlock(&mutex);
		
		
		}else if(RQ_END <= -1 && T_RQ_OVER == true){ // check whether the ready queue is empty and task_file is empty
			
			printf("All the tasks have been served by CPU %d\n",*CID);

			//Write the  number of tasks serviced by the CPU to the simulation_log
			fprintf(fptr1, "\nCPU-%d terminates after servicing %d tasks\n",*CID,NT);

			//Releasing the lock of 'mutex' mutex variable
			pthread_mutex_unlock(&mutex);

			//Used to break out of the infinite loop
			break;

		}
		else{
			//Assigning the task data stored in the ready queue to local variables
			PD = R_QUEUE[0].prID;
			PB = R_QUEUE[0].prBT;
			PA = R_QUEUE[0].prAT;

			//Printing the process ID and process burst time that was added to the ready queue 
			printf("CPU %d Serving Process ID :%d    Process Burst Time :%d\n",*CID,R_QUEUE[0].prID,R_QUEUE[0].prBT);
			
			//Get the current time and write the arrival time and service time to the simulation_log
			time_t ST= time(NULL);
			struct  tm tm = *localtime(&ST);
			struct  tm tmAT = *localtime(&PA);
			fprintf(fptr1, "\nStatistics for CPU-%d:\n",*CID);
			fprintf(fptr1, "%d : %d\n",PD,PB);
			fprintf(fptr1, "Arrival time: %02d:%02d:%02d\n",tmAT.tm_hour, tmAT.tm_min, tmAT.tm_sec);
			fprintf(fptr1, "Service time: %02d:%02d:%02d\n",tm.tm_hour, tm.tm_min, tm.tm_sec);

			//Calculate the process waiting time by getting the difference between service time and arrival time.Add the waiting time to 'total_waiting_time
			total_waiting_time += (int) difftime(ST,PA);
			
			//this function is used to remove the  first task in the array and get other tasks to the front
			arrayDel();
			
			//Releasing the lock of 'mutex' mutex variable
			pthread_mutex_unlock(&mutex);

			//This will sleep the thread for a cpu burst time
			sleep(PB);

			//Acquiring the lock of 'mutex' mutex variable
			pthread_mutex_lock(&mutex);

			//Calculate the compeletion time(CT), get the current time and write the arrival time and completion time to the simulation_log
			time_t CT = ST + PB;
			struct  tm tmCT = *localtime(&CT);
			fprintf(fptr1, "\nStatistics for CPU-%d:\n",*CID);
			fprintf(fptr1, "%d : %d\n",PD,PB);
			fprintf(fptr1, "Arrival time: %02d:%02d:%02d\n",tmAT.tm_hour, tmAT.tm_min, tmAT.tm_sec);
			fprintf(fptr1, "Completion time: %02d:%02d:%02d\n",tmCT.tm_hour, tmCT.tm_min, tmCT.tm_sec);

			//Calculate the process turnaround time by getting the difference between completion time and arrival time.Add the process turnaround time to 'total_turnaround_time
			total_turnaround_time += (int) difftime(CT,PA);
			
			//Increase the total 'num_tasks' by one
			num_tasks ++;

			//Increase the total number of tasks each thread served by one	
			NT++;

			//Releasing the lock of 'mutex' mutex variable
			pthread_mutex_unlock(&mutex);
		}
		
	}
	pthread_exit(NULL); //pthread_exit() function is used to terminate a thread
}


void arrayDel()
{
	if(RQ_END == 0)
	{
		//If the ready queue end value equal to zero, ready queue end is set to zero to delete the 0th element in array
		RQ_END = -1;
	}else{
		//If the RQ_END value not equal to zero, Using a loop shift the array value one by one to the front.Then first value will be removed 
		for(int i=0; i < RQ_END;i++)
		{
			R_QUEUE[i].prID = R_QUEUE[i+1].prID;
			R_QUEUE[i].prBT = R_QUEUE[i+1].prBT;
		}
		//Decrease the array end by one
		RQ_END--;
	}
		
}
