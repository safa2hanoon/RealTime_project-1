#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

void signal_catcher0(int); //SIGUSR1
void signal_catcher1(int); //SIGUSR1
void signal_catcher2(int); //SIGINT
void signal_catcher3(int); //SIGQUIT

int counter1 = 0, counter2 = 0;
pid_t ppid, pid, pid_array[2];

int main()
{
    printf("My process ID is %d\n", getpid());
    ppid = getpid();
    if (sigset(SIGINT, signal_catcher2) == SIG_ERR)
    {
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }
    if (sigset(SIGQUIT, signal_catcher3) == SIG_ERR)
    {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    for (int i = 0; i < 2; i++)
    {
        pid = fork();

        if (pid == 0)
        {
            if (i == 0)
            {
                printf("I am the child  => PID = %d\n", getpid());
                if (sigset(SIGUSR1, signal_catcher0) == SIG_ERR)
                {
                    perror("Sigset can not set SIGUSR1");
                    exit(SIGUSR1);
                }
            }
            else
            {
                printf("I am the child  => PID = %d\n", getpid());
                if (sigset(SIGUSR1, signal_catcher1) == SIG_ERR)
                {
                    perror("Sigset can not set SIGUSR1");
                    exit(SIGUSR1);
                }
            }
            while (1);
        }
        else
        {
            printf("I am the parent => PID = %d, child ID = %d\n", getpid(), pid);
            pid_array[i] = pid;
        }
    }
    sleep(1);
    FILE *out;
    int n1, n2;
    int i=1;
    while (1)
    {
        printf("------------------------------------------------\n");
        if (counter1 == 10 && counter2 == 10)
        {
            printf("\nP1 and P2 are winners of the game!\n");
            kill(pid_array[0],SIGQUIT);
            kill(pid_array[1],SIGQUIT);
            exit(SIGQUIT);
        }
        else if (counter1 == 10 && counter2 != 10)
        {
            printf("\nP1 is the winner of the game!\n");
            kill(pid_array[0],SIGQUIT);
            kill(pid_array[1],SIGQUIT);
            exit(SIGQUIT);
        }
        else if (counter1 != 10 && counter2 == 10)
        {
            printf("\nP2 is the winner of the game!\n");
            kill(pid_array[0],SIGQUIT);
            kill(pid_array[1],SIGQUIT);
            exit(SIGQUIT);
        }
        else
        {
            printf("**ROUND %d**\n",i++);
            sleep(2);
            kill(pid_array[0], SIGUSR1);
            kill(pid_array[1], SIGUSR1);
            pause();
            pause();
            out = fopen("P1.txt", "r");
            if (out == NULL)
                exit(EXIT_FAILURE);
            fscanf(out, "%d", &n1);
            out = fopen("P2.txt", "r");
            if (out == NULL)
                exit(EXIT_FAILURE);
            fscanf(out, "%d", &n2);
            fclose(out);
            remove("P1.txt");
            remove("P2.txt");
            printf("\nFirst Num = %d, Second Num = %d\n", n1, n2);
            if (n1 > n2)
            {
                counter1++;
                printf("P1 wins this round\nP1 : P2\n %d : %d\n", counter1, counter2);
            }
            else if (n2 > n1)
            {
                counter2++;
                printf("P2 wins this round\nP1 : P2\n %d : %d\n", counter1, counter2);
            }
            else
            {
                counter1++;
                counter2++;
                printf("Tie\nP1 : P2\n %d : %d\n", counter1, counter2);
            }
        }
        printf("------------------------------------------------\n");
    }
    return 0;
}

void signal_catcher0(int the_sig)
{
    FILE *in;
    printf("\nSignal SIGUSR1 %d received for P1.\n", the_sig);
    srand(time(NULL) ^ getpid());
    int num = (rand() % 100) + 1;
    in = fopen("P1.txt", "w");
    if (in == NULL)
    {
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(in, "%d", num);
    fclose(in);
    printf("P1 PID: %d, Num: %d\n", getpid(), num);
    kill(ppid,SIGINT);
}

void signal_catcher1(int the_sig)
{
    FILE *in;
    printf("\nSignal1 SIGUSR1 %d received for P2.\n", the_sig);
    srand(time(NULL) ^ getpid());
    int num = (rand() % 100) + 1;
    in = fopen("P2.txt", "w");
    if (in == NULL)
    {
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(in, "%d", num);
    fclose(in);
    printf("P2 PID: %d, Num: %d\n", getpid(), num);
    sleep(1);
    kill(ppid,SIGINT);
}

void signal_catcher2(int the_sig)
{
    printf("\nSignal SIGINT %d received.\n", the_sig);
}

void signal_catcher3(int the_sig)
{
    printf("\nSignal SIGQUIT %d received.\n", the_sig);
    kill(pid_array[0],SIGQUIT);
    kill(pid_array[1],SIGQUIT);
    exit(SIGQUIT);
}