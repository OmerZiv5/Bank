#ifndef _CLASSES_H
#define _CLASSES_H

using namespace std;
#include <vector>
#include <string>
#include <pthread.h>
#include <iostream>


class Account{
public:
    int balance;
    int account_num;
    int password;
    pthread_mutex_t read_account;
    pthread_mutex_t write_account;
    pthread_mutex_t log;
    int account_readers;

    // Creating new account
    Account(int balance_, int account_num_, int password_){
        balance = balance_;
        account_num = account_num_;
        password = password_;
        account_readers = 0;
        int read_init = pthread_mutex_init(&read_account, NULL);
        int write_init = pthread_mutex_init(&write_account, NULL);
        int log_init = pthread_mutex_init(&log, NULL);
        if(read_init || write_init || log_init){
            /***** What should we do if init failed?
             * ********************************????????????????????????????????????????????????????????????????????????????*************/
        }
    }

    ~Account(){
        pthread_mutex_destroy(&read_account);
        pthread_mutex_destroy(&write_account);
        pthread_mutex_destroy(&log);
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
        pthread_mutex_init(&read_bank, NULL);
        pthread_mutex_init(&write_bank, NULL);
    }

    ~Bank(){
        pthread_mutex_destroy(&read_bank);
        pthread_mutex_destroy(&write_bank);
    }
};


class thread_args{
public:
    const char* filename;
    int thread_id;

    thread_args(const char* file, int id){
        filename = file;
        thread_id = id;
    }

    ~thread_args(){}
};

Bank bank;
pthread_mutex_t write_log; // shared mutex for all threads that ensures safe printing to log

#endif
