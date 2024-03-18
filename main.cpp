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

    //command_line holds the operation we want to do (O,D,W,B,Q,T)
    command_line = strtok(line, delimiter);
    int operation = string_to_number(&command_line);
    cout << "operation: " << operation << endl;
    //command_line holds the account number
    command_line = strtok(NULL, delimiter);
    int account = string_to_number(&command_line);
    cout << "account: " << account << endl;
    //command_line holds the password for the account
    command_line = strtok(NULL, delimiter);
    int password = string_to_number(&command_line);
    cout << "password: " << password << endl;

    // executing ATM functions
    if(operation == 'O'){
        //command_line holds the initial amount to open an account
        command_line = strtok(NULL, delimiter);
        int initial_amount = string_to_number(&command_line);
        create_account(account, password, initial_amount, thread_num);
    }
    else if(operation == 'D'){
        //command_line holds the amount to deposit
        command_line = strtok(NULL, delimiter);
        int amount = string_to_number(&command_line);
        deposit_to_account(account, password, amount, thread_num);
    }
    else if(operation == 'W'){
        //command_line holds the amount to withdraw
        command_line = strtok(NULL, delimiter);
        int amount = string_to_number(&command_line);
        withdraw_from_account(account, password, amount, thread_num);
    }
    else if(operation == 'B'){
        check_balance(account, password, thread_num);
    }
    else if(operation == 'Q'){
        close_account(account, password, thread_num);
    }
    else if(operation == 'T'){
        //command_line holds the target account number for the transfer
        command_line = strtok(NULL, delimiter);
        int target_account = string_to_number(&command_line);
        //command_line holds the amount to transfer
        command_line = strtok(NULL, delimiter);
        int amount = string_to_number(&command_line);
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
