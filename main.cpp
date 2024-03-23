using namespace std;
#include <iostream>
#include "ATMfunctions.cpp"
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>



int string_to_number(string string){
    int length = string.size();
    int result = 0;
    for(int i = length - 1; i >= 0; i--){
        int digit = string[i] - '0';
        result += digit * (pow(10, (length - 1 - i)));
    }
    return result;
}


void command(string line, int thread_num){
    // parse line into command fields
    stringstream ss(line);
    char delimiter = ' ';
    vector<string> command_line;
    string token;
    while(getline(ss, token, delimiter)){
        command_line.push_back(token);
    }

    int account = string_to_number(command_line[1]);
    int password = string_to_number(command_line[2]);

    //cout << "Account: " << account << endl;
    //cout<< "Password: " << password << endl;
    // executing ATM functions
    if(command_line[0] == "O"){
        int initial_amount = string_to_number(command_line[3]);
        //cout << "Initial amount: " << initial_amount << endl;
        create_account(account, password, initial_amount, thread_num);
    }
    else if(command_line[0] == "D"){
        int amount = string_to_number(command_line[3]);
        //cout << "Amount to deposit: " << amount << endl;
        deposit_to_account(account, password, amount, thread_num);
    }
    else if(command_line[0] == "W"){
        int amount = string_to_number(command_line[3]);
        //cout << "Amount to withdraw: " << amount << endl;
        withdraw_from_account(account, password, amount, thread_num);
    }
    else if(command_line[0] == "B"){
        //cout << "Checking balance..." << endl;
        check_balance(account, password, thread_num);
    }
    else if(command_line[0] == "Q"){
        //cout << "Closing account..." << endl;
        close_account(account, password, thread_num);
    }
    else if(command_line[0] == "T"){
        int target_account = string_to_number(command_line[3]);
        int amount = string_to_number(command_line[4]);
        //cout << "Transferring " << amount << "$ from Account: " << account << " to Target account: " << target_account << endl;
        transfer(account, password, target_account, amount, thread_num);
    }
}


void *ATM_function(void* ATM){
    thread_args* new_thread = (thread_args*)ATM;
    int thread_num = new_thread->thread_id;
    ifstream ATMfile(new_thread->filename);
    if(!ATMfile.is_open()){
        // Error opening file
        cerr << "Bank error: illegal arguments" << endl;
        exit(-1);
    }

    string line;
    while(getline(ATMfile, line)){
        // line contains a full line from the ATM file
        //cout << "the line is: " << line << endl;
        usleep(100000);
        command(line, thread_num);
    }

    ATMfile.close();
    if(ATMfile.is_open()) {
        cout << "ATM" << thread_num << " failed to close bitch!" << endl;
        perror("Bank error: close failed");
        exit(-1);
    }
    /*if(ATMfile.fail() || ATMfile.bad()) {
        cout << "ATM" << thread_num << " failed to close bitch!" << endl;
        perror("Bank error: close failed");
        exit(-1);
    }*/
    pthread_exit(nullptr);
}



int main(int argc, char**argv) {
    //checking if arguments were passed
    if(argc == 1){
        cerr << "Bank error: illegal arguments" << endl;
        exit(-1);
    }

    //opening log file
    log_file.open("log.txt");
    if(!log_file.is_open()){
        //unable to open file
        perror("Bank error: open failed");
        exit(-1);
    }

    // need to init mutex write_log
    int init_log = pthread_mutex_init(&write_log, NULL);
    if(init_log){
        //problem initializing log mutex
        perror("Bank error: pthread_mutex_init failed\n");
        exit(-1);
    }

    //creating threads
    pthread_t* threads = new pthread_t[argc+1];
    /*int* threadIDs = new int[argc-1];*/

    thread_args *ATM_args = new thread_args[argc-1];
    for(int i = 0; i < argc-1; i++){
        /*threadIDs[i] = i;*/
        ATM_args[i].thread_id = i+1;
        ATM_args[i].filename = argv[i+1];

        int res = pthread_create(&threads[i], NULL, ATM_function, (void*)&ATM_args[i]);
        if(res != 0){
            //creating thread unsuccessful
            // destroy write log mutex and deleting threads
            int destroy_write_log = pthread_mutex_destroy(&write_log);
            if(destroy_write_log){
                perror("Bank error: pthread_mutex_destroy failed\n");
            }
            delete[] threads;
            /*delete[] threadIDs;*/
            delete[] ATM_args;
            log_file.close();
            if(!log_file){
                //failed to close log file
                cout << "ATM" << i+1 << " failed to create - log file unable to close biatch!" << endl;
                perror("Bank error: close failed");
            }
            exit(-1);
        }
    }

    int fee_res = pthread_create(&threads[argc-1], NULL, charge_fee, NULL);
    int status_res = pthread_create(&threads[argc], NULL, bank_status, NULL);
    if(fee_res || status_res){
        //creating thread unsuccessful
        // destroy write log mutex and deleting threads
        int destroy_write_log = pthread_mutex_destroy(&write_log);
        if(destroy_write_log){
            perror("Bank error: pthread_mutex_destroy failed\n");
        }
        delete[] threads;
        /*delete[] threadIDs;*/
        delete[] ATM_args;
        log_file.close();
        if(!log_file){
            //failed to close log file
            cout << "either charge fee thread or bank status thread failed to create - log file unable to close biatch!" << endl;
            perror("Bank error: close failed");
        }
        exit(-1);
    }

    // loop waiting for all threads to end
    for(int i = 0; i < argc-1; i++){
        int join = pthread_join(threads[i], nullptr);
        if(join){
            perror("Bank error: pthread_join failed\n");
            int destroy_write_log = pthread_mutex_destroy(&write_log);
            if(destroy_write_log){
                perror("Bank error: pthread_mutex_destroy failed\n");
            }
            delete[] threads;
            /*delete[] threadIDs;*/
            delete[] ATM_args;
            log_file.close();
            if(!log_file){
                //failed to close log file
                cout << "either charge fee thread or bank status thread failed to create - log file unable to close biatch!" << endl;
                perror("Bank error: close failed");
            }
            exit(-1);
        }
    }

    //all ATM threads are finished - need to stop bank status and comissioner
    ATMs_finished = true;
    int join_fee = pthread_join(threads[argc-1], nullptr);
    int join_status = pthread_join(threads[argc], nullptr);
    if(join_fee || join_status){
        perror("Bank error: pthread_join failed\n");
    }

    // destroy write log mutex and threads
    int destroy_write_log = pthread_mutex_destroy(&write_log);
    if(destroy_write_log){
        perror("Bank error: pthread_mutex_destroy failed\n");
    }
    delete[] threads;
    delete[] ATM_args;
    /*delete[] threadIDs;*/
    log_file.close();
    if(!log_file){
        //failed to close log file
        cout << "All good bruv but the log file was unable to close homie!" << endl;
        perror("Bank error: close failed");
        exit(-1);
    }
    return 0;
}
