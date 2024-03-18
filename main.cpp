using namespace std;
#include <iostream>
#include "ATMfunctions.cpp"
#include <cstdio>
#include <cstring>

int string_to_number(char* string){
    int length = strlen(string);
    int result = 0;
    for(int i = length - 1; i >= 0; i--){
        int digit = string[i] - '0';
        result += digit * (pow(10, (length - 1 - i)));
    }
    return result;
}


void command(char* line, int thread_num){
    // parse line into command fields
    char delimiter[] = " ";
    char *command_line;

    command_line = strtok(line, delimiter);
    cout << "command0: " << command_line[0] << "command1: " << command_line[1] << "command2: " << command_line[2] << endl;
    int account = string_to_number(&command_line[1]);
    int password = string_to_number(&command_line[2]);

    cout << "Account: " << account << "password: " << password << endl;
    // executing ATM functions
    if(command_line[0] == 'O'){
        int initial_amount = string_to_number(&command_line[3]);
        create_account(account, password, initial_amount, thread_num);
    }
    else if(command_line[0] == 'D'){
        int amount = string_to_number(&command_line[3]);
        deposit_to_account(account, password, amount, thread_num);
    }
    else if(command_line[0] == 'W'){
        int amount = string_to_number(&command_line[3]);
        withdraw_from_account(account, password, amount, thread_num);
    }
    else if(command_line[0] == 'B'){
        check_balance(account, password, thread_num);
    }
    else if(command_line[0] == 'Q'){
        close_account(account, password, thread_num);
    }
    else if(command_line[0] == 'T'){
        int target_account = string_to_number(&command_line[3]);
        int amount = string_to_number(&command_line[4]);
        transfer(account, password, target_account, amount, thread_num);
    }
}


void *ATM_function(void* ATM){
    thread_args* new_thread = (thread_args*)ATM;
    int thread_num = new_thread->thread_id;
    const char* file = new_thread->filename;
    FILE* command_file = fopen(file, "r");
    if(!command_file){
        perror("Bank error: fopen failed");
        exit(-1);
    }

    char line[256] = {};
    while(fgets(line, 256, command_file) != NULL){
        // replacing the last char in line with '\0'
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        cout << "the line is: " << line << endl;
        command(line, thread_num);

        // resetting the line buffer
        memset(line, 0, sizeof(line));
    }

    if(fclose(command_file)){
        perror("Bank error: fclose failed");
        exit(-1);
    }

    pthread_exit(nullptr);
}



int main(int argc, char**argv) {
    // need to init mutex write_log
    pthread_mutex_init(&write_log, NULL); //Sould we check the return value of the init????????????????????????????????
    pthread_t* threads = new pthread_t[argc+1];
    int* threadIDs = new int[argc-1];
    //creating threads
    for(int i = 0; i < argc-1; i++){
        threadIDs[i] = i;
        thread_args ATM(argv[i+1], i);
        int res = pthread_create(&threads[i], NULL, ATM_function, (void*)&ATM);
        if(res != 0){
            //creating thread unsuccessful
            // destroy write log mutex
            pthread_mutex_destroy(&write_log);
            delete[] threads;
            delete[] threadIDs;
            exit(-1);
        }
    }
    int fee_res = pthread_create(&threads[argc-1], NULL, charge_fee, NULL);
    int status_res = pthread_create(&threads[argc], NULL, bank_status, NULL);
    if(fee_res || status_res){
        //creating thread unsuccessful
        // destroy write log mutex
        pthread_mutex_destroy(&write_log);
        delete[] threads;
        delete[] threadIDs;
        exit(-1);
    }

    // loop waiting for all threads to end
    for(int i = 0; i < argc+1; i++){
        pthread_join(threads[i], nullptr);
    }

    // destroy write log mutex
    pthread_mutex_destroy(&write_log);
    delete[] threads;
    delete[] threadIDs;
    return 0;
}
