/*
To protect a task from being terminated with SIGTERM
Author: Jason Lu
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "util.h"

int fds[2];
int task_completed = 0;
char line[3000] = "";
int force_restart=0;

int fork_n_run();

void *task_thread(void*task){
    char*cmd=(char*)task;
    int rst=system(task);
    //printf("%d\n",rst);
    if(rst==0){
        task_completed = 1;
        puts("Task completed.");
    }else if (rst==2){
        task_completed = 1;
        puts("Task runner can't find file or directory.");
    }
    else if (rst==1){
        task_completed = 1;
        puts("Error Code 1.");
    }
    else if (rst==256){
        task_completed = 1;
        puts("Error Code -1.");
    }
    return NULL;    
}

void *task_watcher(void*arg){
    pthread_t tid=*(pthread_t*)arg;
    //loop to watch its parent process
    while(getppid()>2){ //while parent process exist
        msleep(100);
        //printf("Parent PID:%d\n",getppid());
        if(task_completed){
            //puts("Task safely exited.");
            return 0;
        }
    }
       //parent process is killed
    
    puts("Guard parent process is killed...self become host.");
    pthread_kill(tid, 0);
    
    close(fds[1]);
    close(fds[0]);
    fork_n_run();
    return NULL;
}

int guarded_task(char*task){
    pthread_t tid;
    pthread_t tid_w;
    //starting the task
    task_completed = 0;
    pthread_create(&tid,NULL,task_thread,task);
    pthread_create(&tid_w,NULL,task_watcher,&tid);

    pthread_join(tid, NULL);
    puts("Task joined.");
    //task ended
    while(!task_completed || force_restart){ //while task ended by SIGTERM, SIGKILL
        puts("Task ended unexpectly, restarting task.");
        pthread_create(&tid,NULL,task_thread,task);
        pthread_join(tid, NULL);
        puts("Task joined.");
        msleep(500);
    }
    pthread_kill(tid_w, 0);
    puts("Watcher killed.");
    if(task_completed){
            puts("Task safely exited.");
            return 0;
    }
 
    return 0;
}

int fork_n_run(){
    pipe(fds);
    int result = fork();

    if(result == 0){  //if is child
        puts("forked.");
        int rst=guarded_task(line);
        printf("Task exited with code: %d\n",rst);
        if(rst==-1){
            puts("restarting self as host.");
        }
    }
    if(result < 0) //fail to fork
    {
        puts("Fail to fork.");
        exit(1);
    }

    for(;;)
    {
        int status = 0;
        waitpid(-1, &status, 0);
        if(!WIFEXITED(status))
        {
            puts("Guard child process restarting...");
            result = fork();
            if(result == 0)
                guarded_task(line);
            if(result < 0)
            {
                puts("Crashed and cannot restart. Exiting...");
                exit(1);
            }
        }
        else exit(0);
    }
}

int main(int argc, char**argv){
    
    line[0]=0;
    pipe(fds);
    int command_start_num=1;
    if (argc < 2) {
        printf("Usage: %s <-f> <task_command>\nTips: \n1.Use full path\n2.Process reads from stdin is not recommended\n",argv[0]);
        exit(0);
    }else{
        if (strcmp(argv[1],"-f")==0){
            if(argc<3){
                printf("Usage: %s <-f> <task_command>\nTips: \n1.Use full path\n2.Process reads from stdin is not recommended\n",argv[0]);
                exit(0);
            }
            command_start_num=2;
            force_restart=1;
        }
    }
    for(int i=command_start_num;i<argc;i++){
       strcat(line,argv[i]);
       strcat(line," ");
    }
    
    fork_n_run();

}

