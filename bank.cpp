
using namespace std;
#include "classes.h"
#include <iostream>
#include <unistd.h>
#include <random>
#include <ctime>
#include <cstdlib>
#include <cmath>


void* bank_status(void* null){
    while(1){
        sleep(0.5);

        pthread_mutex_lock(&bank.read_bank);
        bank.bank_readers++;
        if(bank.bank_readers == 1){
            pthread_mutex_lock(&bank.write_bank);
        }
        pthread_mutex_unlock(&bank.read_bank);

        printf("\033[2J"); // erases the data on the screen
        printf("\033[1;1H"); // move cursor to the upper left corner of the screen
        cout << "Current Bank Status" << endl;
        vector<Account>::iterator it = bank.accounts.begin();
        for(; it != bank.accounts.end(); it++){
            pthread_mutex_lock(&it->read_account);
            it->account_readers++;
            if(it->account_readers == 1){
                pthread_mutex_lock(&it->write_account);
            }
            pthread_mutex_unlock(&it->read_account);

            cout << "Account " << it->account_num << ": Balance - " << it->balance << " $, Account Password - " << it->password << endl;

            pthread_mutex_lock(&it->read_account);
            it->account_readers--;
            if(it->account_readers == 0){
                pthread_mutex_unlock(&it->write_account);
            }
            pthread_mutex_unlock(&it->read_account);
        }

        pthread_mutex_lock(&bank.read_bank);
        bank.bank_readers--;
        if(bank.bank_readers == 0){
            pthread_mutex_unlock(&bank.write_bank);
        }
        pthread_mutex_unlock(&bank.read_bank);
    }

    pthread_exit(nullptr);
}


void* charge_fee(void *null){
    srand(time(NULL));
    while(1){
        int total_fee = 0;
        sleep(3);
        double random_percent = (rand() % 5) + 1; // a random number between 1 and 5
        random_percent = random_percent / 100;

        pthread_mutex_lock(&bank.read_bank);
        bank.bank_readers++;
        if(bank.bank_readers == 1){
            pthread_mutex_lock(&bank.write_bank);
        }
        pthread_mutex_unlock(&bank.read_bank);

        vector<Account>::iterator it = bank.accounts.begin();
        for(; it != bank.accounts.end(); it++){
            pthread_mutex_lock(&it->write_account);

            int fee = round(it->balance * random_percent);
            it->balance -= fee;

            pthread_mutex_unlock(&it->write_account);
            total_fee += fee;
            pthread_mutex_lock(&write_log);

            cout << "Bank: commissions of " << random_percent << " % were charged, the bank gained " << fee << " $ from account " << it->account_num << endl;

            pthread_mutex_unlock(&write_log);
        }

        bank.bank_balance += total_fee;

        pthread_mutex_lock(&bank.read_bank);
        bank.bank_readers--;
        if(bank.bank_readers == 0){
            pthread_mutex_unlock(&bank.write_bank);
        }
        pthread_mutex_unlock(&bank.read_bank);
    }

    pthread_exit(nullptr);
}
