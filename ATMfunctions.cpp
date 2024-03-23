using namespace std;
#include "bank.cpp"
#include <algorithm>

vector<Account>::iterator search_account(int acc_id){
    vector<Account>::iterator it = bank.accounts.begin();
    for(; it != bank.accounts.end(); it++){
        if(it->account_num == acc_id){
            return it;
        }
    }
    return bank.accounts.end();
}

bool compare_account_num(const Account& a, const Account& b) { // I've made a change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return a.account_num < b.account_num;
}


// Opening a new account
void create_account(int account_number,int password ,int initial_amount, int ATMid){
    pthread_mutex_lock(&bank.write_bank);
    //cout << ATMid << " created an account" << endl;
    vector<Account>::iterator it = search_account(account_number);
    if(it == bank.accounts.end()){
        // Account does not exist in bank
        Account new_acc(initial_amount, account_number, password);
        bank.accounts.push_back(new_acc);
        // I've made a change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        sort(bank.accounts.begin(),bank.accounts.end(), compare_account_num);
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << ATMid << ": New account id is " << new_acc.account_num << " with password " << new_acc.password << " and initial balance " << new_acc.balance << endl;
        pthread_mutex_unlock(&write_log);
    }
    else{
        // Account already exists
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << "Error " << ATMid << ": Your transaction failed - account with the same id exists" << endl;
        pthread_mutex_unlock(&write_log);
    }

    pthread_mutex_unlock(&bank.write_bank);
}


// Deposit money into account
void deposit_to_account(int account_number,int password ,int amount, int ATMid){
    pthread_mutex_lock(&bank.read_bank);
    bank.bank_readers++;
    if(bank.bank_readers == 1){
        pthread_mutex_lock(&bank.write_bank);
    }
    pthread_mutex_unlock(&bank.read_bank);

    vector<Account>::iterator it = search_account(account_number);
    // check if the account actually exists
    if(it == bank.accounts.end()){
        //account does not exist
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << "Error " << ATMid << ": Your transaction failed - account id " << account_number << " does not exist" << endl;
        pthread_mutex_unlock(&write_log);
    }
    else{
        // account exists in bank
        pthread_mutex_lock(&it->write_account);
        if(it->password != password){
            // Password is incorrect
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << "Error " << ATMid << ": Your transaction failed - password for account id " << account_number << " is incorrect" << endl;
            pthread_mutex_unlock(&write_log);
        }
        else{
            // Password is correct
            it->balance += amount;
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << ATMid << ": Account " << it->account_num << " new balance is " << it->balance << " after amount " << amount << " $ was deposited" << endl;
            pthread_mutex_unlock(&write_log);
        }
        pthread_mutex_unlock(&it->write_account);
    }

    pthread_mutex_lock(&bank.read_bank);
    bank.bank_readers--;
    if(bank.bank_readers == 0){
        pthread_mutex_unlock(&bank.write_bank);
    }
    pthread_mutex_unlock(&bank.read_bank);
}


// Withdrawing money from account
void withdraw_from_account(int account_number,int password ,int amount, int ATMid){
    pthread_mutex_lock(&bank.read_bank);
    bank.bank_readers++;
    if(bank.bank_readers == 1){
        pthread_mutex_lock(&bank.write_bank);
    }
    pthread_mutex_unlock(&bank.read_bank);

    vector<Account>::iterator it = search_account(account_number);
    // check if the account actually exists
    if(it == bank.accounts.end()){
        //account does not exist
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << "Error " << ATMid << ": Your transaction failed - account id " << account_number << " does not exist" << endl;
        pthread_mutex_unlock(&write_log);
    }
    else{
        // account exists in bank
        pthread_mutex_lock(&it->write_account);
        if(it->password != password){
            // Password is incorrect
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << "Error " << ATMid << ": Your transaction failed - password for account id " << account_number << " is incorrect" << endl;
            pthread_mutex_unlock(&write_log);
        }
        else if(amount > it->balance){
            // Not enough money to withdraw
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << "Error " << ATMid << ": Your transaction failed - account id " << account_number << " balance is lower than" << amount << endl;
            pthread_mutex_unlock(&write_log);
        }
        else{
            // withdraw successful
            it->balance -= amount;
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << ATMid << ": Account " << it->account_num << " new balance is " << it->balance << " after " << amount << " $ was withdrew" << endl;
            pthread_mutex_unlock(&write_log);
        }
        pthread_mutex_unlock(&it->write_account);
    }

    pthread_mutex_lock(&bank.read_bank);
    bank.bank_readers--;
    if(bank.bank_readers == 0){
        pthread_mutex_unlock(&bank.write_bank);
    }
    pthread_mutex_unlock(&bank.read_bank);
}


void check_balance(int account_number, int password, int ATMid){
    pthread_mutex_lock(&bank.read_bank);
    bank.bank_readers++;
    if(bank.bank_readers == 1){
        pthread_mutex_lock(&bank.write_bank);
    }
    pthread_mutex_unlock(&bank.read_bank);

    vector<Account>::iterator it = search_account(account_number);
    // check if the account actually exists
    if(it == bank.accounts.end()){
        //account does not exist
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << "Error " << ATMid << ": Your transaction failed - account id " << account_number << " does not exist" << endl;
        pthread_mutex_unlock(&write_log);
    }
    else{
        // account exists
        pthread_mutex_lock(&it->read_account);
        it->account_readers++;
        if(it->account_readers == 1){
            pthread_mutex_lock(&it->write_account);
        }
        pthread_mutex_unlock(&it->read_account);

        if(it->password != password){
            // Password is incorrect
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << "Error " << ATMid << ": Your transaction failed - password for account id " << account_number << " is incorrect" << endl;
            pthread_mutex_unlock(&write_log);
        }
        else{
            // password is correct
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << ATMid << ": Account " << account_number << " balance is " << it->balance << endl;
            pthread_mutex_unlock(&write_log);
        }

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


void close_account(int account_number, int password, int ATMid){
    pthread_mutex_lock(&bank.write_bank);

    vector<Account>::iterator it = search_account(account_number);
    if(it == bank.accounts.end()){
        // Account does not exist in bank
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << "Error " << ATMid << ": Your transaction failed - account id " << account_number << " does not exist" << endl;
        pthread_mutex_unlock(&write_log);
    }
    else{
        // Account exists
        if(it->password != password){
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << "Error " << ATMid << ": Your transaction failed - password for account id " << account_number << " is incorrect" << endl;
            pthread_mutex_unlock(&write_log);
        }
        else{
            // Account exists and correct password = closing account
            int balance = it->balance;
            bank.accounts.erase(it);
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << ATMid << ": Account " << account_number << " is now closed. Balance was " << balance << endl;
            pthread_mutex_unlock(&write_log);
        }
    }

    pthread_mutex_unlock(&bank.write_bank);
}


void transfer(int source_account, int source_password, int target_account, int amount, int ATMid){
    pthread_mutex_lock(&bank.read_bank);
    bank.bank_readers++;
    if(bank.bank_readers == 1){
        pthread_mutex_lock(&bank.write_bank);
    }
    pthread_mutex_unlock(&bank.read_bank);

    vector<Account>::iterator source_it = search_account(source_account);
    vector<Account>::iterator target_it = search_account(target_account);
    // check if the account actually exists
    if(source_it == bank.accounts.end()){
        //source account does not exist
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << "Error " << ATMid << ": Your transaction failed - account id " << source_account << " does not exist" << endl;
        pthread_mutex_unlock(&write_log);
    }
    else if(target_it == bank.accounts.end()){
        pthread_mutex_lock(&write_log);
        sleep(1);
        log_file << "Error " << ATMid << ": Your transaction failed - account id " << target_account << " does not exist" << endl;
        pthread_mutex_unlock(&write_log);
    }
    else{
        // both accounts exist
        pthread_mutex_lock(&source_it->write_account);
        pthread_mutex_lock(&target_it->write_account);

        if(source_it->password != source_password){
            //source account password is incorrect
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << "Error " << ATMid << ": Your transaction failed - password for account id " << source_account << " is incorrect" << endl;
            pthread_mutex_unlock(&write_log);
        }
        else if(source_it->balance < amount){
            //amount to tranfer is bigger than source balance
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << "Error " << ATMid << ": Your transaction failed - account id " << source_account << " balance is lower than" << amount << endl;
            pthread_mutex_unlock(&write_log);
        }
        else{
            // successful transaction
            source_it->balance -= amount;
            target_it->balance += amount;
            pthread_mutex_lock(&write_log);
            sleep(1);
            log_file << ATMid << ": Transfer " << amount << " from account " << source_account << " to account " << target_account << " new account balance is " << source_it->balance << " new target account balance is " << target_it->balance << endl;
            pthread_mutex_unlock(&write_log);
        }


        pthread_mutex_unlock(&target_it->write_account);
        pthread_mutex_unlock(&source_it->write_account);
    }

    pthread_mutex_lock(&bank.read_bank);
    bank.bank_readers--;
    if(bank.bank_readers == 0){
        pthread_mutex_unlock(&bank.write_bank);
    }
    pthread_mutex_unlock(&bank.read_bank);
}
