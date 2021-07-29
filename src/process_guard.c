/*
To protect a task from being terminated with SIGTERM
Author: Jason Lu
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int fds[2];
int task_completed = 0;
char line[3000] = "";

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
        
}

void *task_watcher(void*arg){
    pthread_t tid=*(pthread_t*)arg;
       //loop to see its parent process
    while(getppid()>2){
        sleep(1);
        //printf("Parent PID:%d\n",getppid());
        if(task_completed){
            //puts("Task safely exited.");
            return 0;
        }
    }
       //parent process is killed
    
    puts("parent process is killed...self become host.");
    pthread_kill(&tid,0);
    
    close(fds[1]);
    close(fds[0]);
    fork_n_run();
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
    while(!task_completed){
        pthread_create(&tid,NULL,task_thread,task);
        pthread_join(tid, NULL);
    }
    pthread_kill(tid_w, NULL);
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

    if(result == 0){
        puts("forked.");
        int rst=guarded_task(line);
        printf("Task exited with code: %d\n",rst);
        if(rst==-1){
            puts("restarting self as host.");
        }
    }
    if(result < 0)
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
            puts("restarting...");
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

    if (argc < 2) {
        printf("Usage: %s <task_command>",argv[0]);
        exit(0);
    }
    for(int i=1;i<argc;i++){
       strcat(line,argv[i]);
       strcat(line," ");
    }
    
    fork_n_run();

}

