/*
Author : Marwan Salem ,ID : 4677 ,GROUP 3
Date: 15/10/2019
problems : if cd is used , you'll have to enter exit multiple times,
sometimes it doesnt block following commands ,, if a command isnt completed
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>
#include <time.h>
#define arglen 64
#define MAX_PATH 256
#define BUFFERSZ 1024


char curr_work_dir[MAX_PATH];//current wokring directory as a string, to be displayed for the shell
char logfile_path[MAX_PATH]="./log_4677.txt";
char input_buffer[BUFFERSZ];  // will contain the string read from the input stream
int exit_fl;

void write_to_log(int dummy){ //dummy value since the prototype for signalhandler  function pointer passed to the signal function has an int as a parameter
    FILE* logs=fopen(logfile_path,"a+");//opening file in append mode +
    if(logs!=NULL){
        time_t now;
        time(&now);// get the current time
        char* now_str=ctime(&now); //convert the current time to a string format
        now_str[24]=0;//remove the new line that is appended at the end of the time string..
        fprintf(logs,"[%s]Child process was terminated,Command:%s\n",now_str,input_buffer); //write a line to log file

        fclose(logs);
    }

}
int my_read(char args[][arglen]){ //returns number of arguments counted by the function,, including ampersands if any


    char c='M'; // holds characters iterated on from the buffer
    int idx=0;// index to iterate on the input buffer
    int token_it=0;
    int arg_ctr=0; // count number tokens in the string delimited by empty spaces
    fgets(input_buffer,1024,stdin);

    int last_space=0; // flag : 1 if the last character i checked is a space, 0 otherwise

    while((c=input_buffer[idx++])==' '); // To skip leading spaces
   // printf("HELP");
    idx--; //i skipped one char when I  exited the loop
    while((c=input_buffer[idx++])!='\n'){
        if(c==' '){
            last_space=1; //raise the flag for the next iteration
        }else{ // a non-space character
            if(last_space){ //if lastt character is a string then this is a new 'word',and I need to end the previous word

                args[arg_ctr++][token_it]=0;
                // put null character at end of string
                token_it=0; //reset the token iterator to 0 for the new token
            }

            args[arg_ctr][token_it++]=c; //copying a valid character to the argument
            last_space=0;   //lowering flag to tell the program in the next iteration that the last character was not a space
        }

    }
    args[arg_ctr++][token_it]=0; // NULL character at the end

    return arg_ctr;



}
int main()
{

    // to do print current working directory before inputting a command :done
    // spawn a child to perform some other process.:done

    //signal(SIGKILL,write_to_log);

    signal(SIGCHLD,write_to_log);  // set write_to_log as the signal handler function when is SIGCHLD signal is received; meaning that  child terminated

    pid_t ch_id;
    int wstatus;
    int dont_wait=0;
    char argss[10][arglen]; //I will allow up to 10 arguments
    char *args[10];
    while(1){

    if(getcwd(curr_work_dir,MAX_PATH)) // get the current working directory string, if it is not NULL then the function was successful, otherwise there's a problem
        printf("%s>>",curr_work_dir);
    int token_count= my_read(argss); // the function that reads the command and parses it , and saves the arguments in the argss array
    int ct=0;
    args[ct]=NULL;//keep first element as NULL ,to avoid unpredictable behaviour due to garbage
    for(ct=0;ct<token_count;ct++){
        args[ct]=argss[ct];    // copy each argument string from array of character, to an array of pointers to pass it to execvp function
    }
    args[token_count]=NULL; //NULL pointer at the end of arg list or array required by execvp


   if(!strcmp("exit",args[0])){//exiting when receiving exit
        printf("\nEXITING NOW!\n");
        exit_fl=1;//raise flag , so other instances of this program will exit as soon as exit is received

        //killpg(getpgrp(),SIGCHLD);  //added as a solution to close all processes in the same processgroup but it's bad as it prevents the process from waiting


        exit(0);

    }else{
        exit_fl=0;
    }

    if(argss[token_count-1][0]=='&'){ // DONT WAIT FOR ME
        dont_wait=1;
        args[token_count-1]=NULL; //removing the ampersand from the argument list,because execvp doesnt recognize it
    }else{
        dont_wait=0; //lower the flag
    }




    ch_id=fork(); //spawn a child

    if(ch_id >= 0){ //Fork successful
    //printf("yes\n");
        if(ch_id==0){  //I'm a Child

            int do_execvp=1; // a flag raised : when the command can be carried out by execvp function


            if(!strcmp("cd",args[0])){
                do_execvp=0;
                chdir(args[1]);
            }

            if(do_execvp){
                if(execvp(args[0],args)<0){ //if it returns -1 then this signifies a problem
                    printf("Unknown Command!\n");
                }

            }


        }else{ //I'm a parent

            if(!dont_wait){ //if there is no ampersand in the command then wait

            wait(&wstatus);

            }

            dont_wait=0; //lower the flag so it can wait in following commands unless specified otherwise

        }
    }

    }//end of while 1


    return 0;
}


