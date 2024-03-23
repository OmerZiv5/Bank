#ifndef _CLASSES_H
#define _CLASSES_H

using namespace std;
#include <vector>
#include <string>
#include <pthread.h>
#include <iostream>
#include <fstream>


class Account{
public:
    int balance;
    int account_num;
    int password;
    pthread_mutex_t read_account;
    pthread_mutex_t write_account;
    //pthread_mutex_t log;
    int account_readers;

    // Creating new account
    Account(int balance_, int account_num_, int password_){
        balance = balance_;
        account_num = account_num_;
        password = password_;
        account_readers = 0;
        int read_init = pthread_mutex_init(&read_account, NULL);
        int write_init = pthread_mutex_init(&write_account, NULL);
        //int log_init = pthread_mutex_init(&log, NULL);
        if(read_init || write_init){    // || log_init
            //problem initializing at least one of the mutex locks
            perror("Bank error: pthread_mutex_init failed\n");
            exit(-1);
        }
    }

    ~Account(){
        int destroy_read_account = pthread_mutex_destroy(&read_account);
        int destroy_write_account = pthread_mutex_destroy(&write_account);
        //int destroy_log = pthread_mutex_destroy(&log);
        if(destroy_read_account || destroy_write_account){  // || destroy_log
            //problem destroying at least one of the mutex locks
            perror("Bank error: pthread_mutex_destroy failed\n");
            exit(-1);
        }
    }
};


class Bank{
public:
    int bank_balance;
    vector<Account> accounts;
    pthread_mutex_t read_bank;
    pthread_mutex_t write_bank;
    int bank_readers;

    Bank(){
        bank_balance = 0;
        bank_readers = 0;
        int read_init = pthread_mutex_init(&read_bank, NULL);
        int write_init = pthread_mutex_init(&write_bank, NULL);
        if(read_init || write_init){
            //problem initializing at least one of the mutex locks
            perror("Bank error: pthread_mutex_init failed\n");
            exit(-1);
        }
    }

    ~Bank(){
        int destroy_read_bank = pthread_mutex_destroy(&read_bank);
        int destroy_write_bank = pthread_mutex_destroy(&write_bank);
        if(destroy_read_bank || destroy_write_bank){
            //problem destroying at least one of the mutex locks
            perror("Bank error: pthread_mutex_destroy failed\n");
            exit(-1);
        }
    }
};


class thread_args{
public:
    string filename;
    int thread_id;

    thread_args(){
        filename = "";
        thread_id = -1;
    }

    ~thread_args(){}
};

Bank bank;
pthread_mutex_t write_log; // shared mutex for all threads that ensures safe printing to log
ofstream log_file;  //creating log file


#endif
