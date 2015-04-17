#include<stdio.h>
#include<stdlib.h>
#include<procfs.h>
#include<time.h>
#include<fcntl.h>
#include<sys/times.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

int system_output_pstatus(char* commmandstatus, int argument);
int system_output_prmap(char* temp_pid, char* commandmapping);
int previous_nanosec=0;

system_output_pstatus (char* commandstatus, int argument)
{
  pstatus_t info_t;
  int fp;
  fp = open(commandstatus,O_RDONLY);
  read (fp, &info_t, sizeof(info_t));
  
  // utilization time conversation
  unsigned long conv_to_nanosec = ((info_t.pr_utime.tv_sec + info_t.pr_utime.tv_sec) * 1000000000);
  //printf("%lu\n",conv_to_nanosec_utime );
  //unsigned long conv_to_nanosec_stime = (info_t.pr_stime.tv_sec * 1000000000);
  //printf("%lu\n",conv_to_nanosec_stime );
  unsigned long add_nanosec = (conv_to_nanosec + info_t.pr_utime.tv_nsec + info_t.pr_stime.tv_nsec);
  //printf("%lu\n",add_nanosec_utime );
  //unsigned long add_nanosec_stime = (conv_to_nanosec_stime + info_t.pr_stime.tv_nsec);
  //printf("%lu\n",add_nanosec_stime);
  //unsigned long whole_nanosec = (add_nanosec_stime + add_nanosec_utime);
  //printf("%lu\n",whole_nanosec );
  unsigned long calculate_nanosec = (add_nanosec - previous_nanosec);
  //printf("%lu\n",calculate_nanosec );
  double final_utitime = (calculate_nanosec) / (argument*pow(10,9));

  printf("\n\n\n\n\n\n>----- New Status information of the process is as follows -----< \n");
  printf("\n\t Start address of the process stack		: %x", 		info_t.pr_stkbase);
  printf("\n\t Length of the stack segment is			: %d",		info_t.pr_stksize);
  printf("\n\t Start address of the heap segment		: %x", 		info_t.pr_brkbase);
  printf("\n\t Length of the heap segment is			: %d",		info_t.pr_brksize);
  printf("\n\t Process user cpu time is			: %d.%d",	info_t.pr_utime.tv_sec,info_t.pr_utime.tv_nsec);
  printf("\n\t Process system cpu time is			: %d.%d",	info_t.pr_stime.tv_sec,info_t.pr_stime.tv_nsec);
  printf("\n\t Process utilization percentage			: %f ",		final_utitime*100);
  previous_nanosec = add_nanosec;
}

system_output_prmap (char* temp_pid, char* commandmapping)
{
  prmap_t info_t1;
  int fp;
  
  fp = open(commandmapping,O_RDONLY);
  	
 		printf("\n\n>--- New Mapping information of the process---< \n");
 		printf(" \n\tVIRTUAL.ADDR --- MAP NAME --- REAL NAME --- SIZE(kb) \n\n");
	  	while(read(fp, &info_t1, sizeof(info_t1)) > 0)
	  	{
	  		if ((int)info_t1.pr_mapname[0] != 0)
	  		{
		  	  // printf("\n map name of the process 		: %s \n", 		info_t1.pr_mapname);
		  	  char command_path[40] = "/proc/";
			  strcat(command_path,temp_pid);
			  strcat(command_path,"/path/");
			  strcat(command_path,info_t1.pr_mapname);
			  char real_name_file[100] = "";
			  if(readlink(command_path, real_name_file, 100) != -1); 
			  {
			  	printf("\t %x \t %s \t %s \t %d kb \n ", info_t1.pr_vaddr, info_t1.pr_mapname, real_name_file, info_t1.pr_size/1024);
			  }
			}
	  	}
	  	printf("\n \t \t ------ End of information ------\n");
}




int main(int argc, char* argv[])
{

	if (argc!=3)
	{
		printf(" \n Correct usage: $ ./memprof <program name> <time in seconds>");/* code */
	}
	else
	{
	
	    pid_t cpid= fork();	
		if(cpid < 0)
		{
		 	exit(0);
		 	printf("invalid fork");
		}
	    else if(cpid==0 )  // Child Process
	   		{
				// usleep(5000000);
				// printf("\n  **In Child Process **");
				execl(argv[1],argv[1],0);
				printf(" \n An error ocurred running the program\n",stderr);
				abort();
	   		
	   		}
	   		else if (cpid>0) // Parent Process
	   		{
		   		int i;
				int status;
				pid_t result = 0;
				char commandstatus[20] = "/proc/";
				char commandmapping[20] = "/proc/";
				char temp_pid[20];
				sprintf(temp_pid,"%d",cpid);
		   		strcat(commandstatus,temp_pid);
		   		strcat(commandstatus,"/status");
				strcat(commandmapping,temp_pid);
		   		strcat(commandmapping,"/map");
				while(!result)
				{
					result = waitpid(cpid,&status, WNOHANG);
				
					if(result == 0)   // if child process is still running
					{
						// Print all the status information and mapping information
						system_output_pstatus(commandstatus,atoi(argv[2]) );
						system_output_prmap(temp_pid,commandmapping);
						int micro_seconds = (atoi(argv[2])*1000000);
						// SLeep for argv[2] seconds
						usleep(micro_seconds);
					}
					else if(result == -1 )  // waitpid error
						{
							// exit(0);
							// fork failure
						}
						else
						{
							//Child exited.
						}
						
				}	
	   		}
   	}
}
