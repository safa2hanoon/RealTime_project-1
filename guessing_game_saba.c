#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <string.h>

// ---- GLOBAL VARIABLES -------
int big_score1 = 0, big_score2 = 0, flag_counter=0 ,rounds=0, pipe1[2], pipe2[2];
char s1[20], s2[20];
pid_t ppid, pid, pid_refree, pid_array[3];
char *array[2];
static char pipe1writemessage[30] = "child1.txt-child2.txt";
static char read1message[BUFSIZ], read2message[BUFSIZ],pipe2writemessage[BUFSIZ];

// ---- PROTOTYPES -------
void signal_catcher0(int); //SIGUSR1
void signal_catcher1(int); //SIGUSR1
void signal_catcherpid(int); //SIGINT
void signal_catcherq(int); //SIGQUIT
void massage_to_tokens(char *);

// ---- MAIN FUNCTION -------

int main()
{
 ppid = getpid();
 if ( pipe(pipe1) == -1 ) {//create a pipe using pipe()
     printf("error pipe1 not created\n");
     exit(0);
 }
  
 if ( pipe(pipe2) == -1 ) {//create a pipe using pipe()
     printf("error pipe2 not created\n");
     exit(0);
  }
// ------- FORK MULTIBLE CHILDREN FROM PARENT       
    for (int i = 0; i < 3; i++)
    {
        pid = fork();

        if (pid == -1)
        {
             fprintf(stderr,"Fork Failed");
             exit(-1);
        }
// ----------CHILDREN PROCCESS-------                
        else if (pid == 0)
        {
            //--- << Player1 >>----
            if (i == 0)
            {
                printf("I am the player1  => PID = %d\n", getpid());
               
                if (sigset(SIGUSR1, signal_catcher0) == -1)
                {
                    perror("Sigset can not set SIGUSR1");
                    exit(SIGUSR1);
                }
               
            }
            //--- << Player2 >>----
            else if (i == 1)
            {
                printf("I am the player2 => PID = %d\n", getpid());
                if (sigset(SIGUSR1, signal_catcher1) == -1)
                {
                    perror("Sigset can not set SIGUSR1");
                    exit(SIGUSR1);
                }
            }
            
           //--- << Refree >>----
           else 
            {
               //-------LOCAL VARIABLES -----------
               int score1 = 0, score2 = 0, num_child1, num_child2;
               char line_child1[3], line_child2[3];
               FILE *fpt_child1, *fpt_child2;
               
               printf("I am the refree => PID = %d\n", getpid());
               //---- CREATE PIPE -1 COMMUNICATION TO READ THE MESSAGE SENT FROM PARENT "child1.txt-childe2.txt" 
	       close(pipe1[1]); // Close the unwanted pipe1 write side
	       close(pipe2[0]); // Close the unwanted pipe2 read side
	       read(pipe1[0], read2message, sizeof(read2message));
	       printf("In Child: Reading from pipe 1 – Message is %s\n", read2message);
               //---- SLIPT THE MESSAGE TO TOKENS AND PUT THEM IN LOCAL ARRAY
	       massage_to_tokens(read2message);
    	       // -- OPEN THE FILES SENT FROM PARENT AND COMPARE EACH LINE IN EACH FILE , THEN INCREMENT ON THE Score1 & Score2
		    fpt_child1 = fopen(array[0], "r");
		    fpt_child2 = fopen(array[1], "r");
		    if (fpt_child1 == NULL)  exit(EXIT_FAILURE);
		    if (fpt_child2== NULL) exit(EXIT_FAILURE);
		    while ((fscanf(fpt_child1 ,"%s",line_child1) != EOF) && (fscanf(fpt_child2 ,"%s",line_child2) != EOF)) 
		    {
		       num_child1 = atoi(line_child1);
		       num_child2 = atoi(line_child2);
		       if (num_child1 > num_child2)
		       {
			 score1++;
			 printf("PLAYER1 wins this round || P1 : %d  P2 : %d\n", score1, score2);
		       }
		       else if (num_child2 > num_child1)
		       {
			 score2++;
			 printf("PLAYER2 wins this round || P1 : %d  P2 : %d\n", score1, score2);
		       }
		       else
		       {
			printf("No WINNER || P1 : %d  P2 : %d\n", score1, score2);
                      } 	
		   } 
		  // -- R process deletes the files child1.txt and child2.txt
		  fclose(fpt_child1);
		  fclose(fpt_child2);
		  remove(array[0]);
                 remove(array[1]);
                 
                  // -- CONVERT THE SCORES TO STRING TO CONCATENATE THEM 
                 sprintf(s1, "%d", score1);
	         sprintf(s2, "%d", score2);
	         strcat(s1,"-");
	         strcat(s1,s2);
	         printf("%s\n",s1);
	      
      	      // --- SEND THE SCORES TO PARENT USING PIPE-2       
	      strcpy(pipe2writemessage , s1);
	      close(pipe1[1]); // Close the unwanted pipe1 write side
	      close(pipe2[0]); // Close the unwanted pipe2 read side
	      printf("In Child: Writing to pipe 2 – Message \n");
	      write(pipe2[1], pipe2writemessage, sizeof(pipe2writemessage));

	   }
	   while(1);
         }
        //--<< Parent>>---
        else
        {
            printf("I am the parent => PID = %d, child ID = %d\n", getpid(), pid);
        // -- MAKE THE PARENT PROCESS SENSITIVE TO SIGINT & SIGQUIT 
            if ( sigset(SIGINT, signal_catcherpid) == -1 )
	    {
		perror("Sigset can not set SIGINT");
		exit(SIGINT);
	    }

	    if ( sigset(SIGQUIT, signal_catcherq) == -1 )
	    {
		perror("Sigset can not set SIGQUIT");
		exit(SIGQUIT);
	    }
            pid_array[i] = pid;
            sleep(2);
        }
        }
          
while (1)
{ 
    printf("------------------------------------------------\n");
        //NO NEW ROUNDS NEEDED IF ONE big_score AT LEAST EQUALS 50 THEN 
        if((big_score1 >= 50) || (big_score2 >= 50))
        {
                // -- IF THE TWO SCORES ARE GRATER THAN OR EQUAL TO 50 THEN BOTH ARE WINNERS
              if((big_score1 >= 50) && (big_score2 >= 50)){
                 printf("big score1: %d big score2:%d PLAYER1 and PLAYER2 Are Winners\n", big_score1, big_score2);
                 printf("number of rounds %d", rounds);
                 kill(pid_array[0],SIGQUIT);
                 kill(pid_array[1],SIGQUIT);
                 kill(pid_array[2],SIGQUIT);
                 exit(SIGQUIT);
              }            
              else if (big_score1 > big_score2)
	       {
		  printf("big score1: %d big score2:%d PLAYER1 is winner\n", big_score1, big_score2);
		  printf("number of rounds %d", rounds);
		  kill(pid_array[0],SIGQUIT);
                 kill(pid_array[1],SIGQUIT);
                 kill(pid_array[2],SIGQUIT);
                 exit(SIGQUIT);
	       }
	       else
	       {
		 printf("big score1: %d big score2:%d PLAYER2 is winner\n", big_score1, big_score2);
		 printf("number of rounds %d", rounds);
		  kill(pid_array[0],SIGQUIT);
                 kill(pid_array[1],SIGQUIT);
                 kill(pid_array[2],SIGQUIT);
                 exit(SIGQUIT);
	       }
        }
        else
        {
        // -- START NEW ROUND 
            printf("--<< ROUND >>-- %d**\n",rounds++);
            sleep(2);
            // -- SEND SIGUSR TO PARENT FROM PLAYER1 & PLAYER2
	    kill(pid_array[0], SIGUSR1);
	    pause();   
	    kill(pid_array[1], SIGUSR1);  
	    close(pipe1[0]); // Close the unwanted pipe1 read side
	    close(pipe2[1]); // Close the unwanted pipe2 write side
	    printf("In Parent: Writing to pipe 1 – Message is %s\n", pipe1writemessage);
	    write(pipe1[1], pipe1writemessage, sizeof(pipe1writemessage));
	    
    	    // -- RECIEVE FROM REFEE PLAYERS SCORES OF THE ROUND  
	    close(pipe1[0]); // Close the unwanted pipe1 read side
	    close(pipe2[1]); // Close the unwanted pipe2 write side
	    read(pipe2[0], read1message, sizeof(read1message));
	    printf("In Parent: Reading from pipe 2 – Message \n"); 
            massage_to_tokens(read1message);
	    
    	    //ADD ROUND SCORES TO GLOBAL ROUNDS SCORES 
            big_score1 += atoi(array[0]);
	    big_score2 += atoi(array[1]);
            printf("big score1: %d big score2:%d \n", big_score1, big_score2);
            flag_counter = 0;
        }
printf("------------------------------------------------\n");    
}
return 0;
}


// -------------------------- SIGNALS FUNCTIONS ----------------------------------------
void signal_catcher0(int the_sig)
{
    FILE *in;
    printf("\nSignal1 SIGUSR1 %d received for P1.\n", the_sig);
    srand(time(NULL) + getpid());
    in = fopen("child1.txt", "w");
    if (in == NULL)
    {
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }
    for(int i =0 ; i<10 ; i++)
    {
       int num = (rand() % 100) + 1;
       fprintf(in, "%d\n", num);
       printf("P1 PID: %d, Num: %d\n", getpid(), num);
    }
    fclose(in);
    sleep(1);
    kill(ppid,SIGINT);
}
//--------------------------------------------------------------------------------------

void signal_catcher1(int the_sig)
{
    FILE *in;
    printf("\nSignal1 SIGUSR1 %d received for P2.\n", the_sig);
    srand(time(NULL) + getpid());
    in = fopen("child2.txt", "w");
    if (in == NULL)
    {
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }
    for(int i =0 ; i<10 ; i++)
    {
       int num = (rand() % 100) + 1;
       fprintf(in, "%d\n", num);
       printf("P2 PID: %d, Num: %d\n", getpid(), num);
    }
    fclose(in);
    sleep(1);
    kill(ppid,SIGINT);
}
//--------------------------------------------------------------------------------------

void signal_catcherpid(int the_sig)
{
    flag_counter++;
}
//--------------------------------------------------------------------------------------

void signal_catcherq(int the_sig)
{
    exit(1);
}
//--------------------------------------------------------------------------------------

void massage_to_tokens(char *massage){
   char *token;
   int  token_num = 0;
   token = strtok(massage, "-"); 
   while (token != NULL)
   {
    array[token_num++] = token;
    token = strtok (NULL, "/");
   }
}
