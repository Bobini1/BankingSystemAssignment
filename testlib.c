#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <float.h>
#include <errno.h>

FILE* database;

int flush()
{
    int leftovers = 0;
    int c;
    printf("flushing: ");
    while ((c = fgetc(stdin)) != '\n' && c != EOF) 
    {
        leftovers++;
        printf("%c", c);
    }
    printf("\n");
    return leftovers;
}

int readnadd0(char* str, int n)
{
    int current;
    int i;
    int shorter = 0;
    for (i = 0; i <= n; i++)
    {
        current = getchar();
        if (current == '\n') 
        {
            shorter = 1;
            break;
        }
        if (current == EOF)
        {
            fclose(database);
            printf("Fatal error. Closing.\n");
            exit(0);
        }
        else str[i] = current;
    }
    if (feof(stdin)) printf("feof");
    if (ferror(stdin)) printf("ferror");
    if (!shorter)
    {
        flush();
        printf("Too long.\n");
        return 0;
    }
    str[i] = '\0';
    if (i == 0)
    {
        printf("Incorrect input.\n");
        return 0;
    }
    else return 1;
}

typedef struct Account
{
    int ID;
    char name[21];
    char surname[21];
    char address[21];
    char PESEL[12];
    long balance;
} Account;

void printAccount(Account acc)
{
    printf("ID: %d\n", acc.ID);
    printf("Name: %s\n", acc.name);
    printf("Surname: %s\n", acc.surname);
    printf("Address: %s\n", acc.address);
    printf("PESEL: %s\n", acc.PESEL);
    printf("balance: %.2lf\n\n", acc.balance / 100.0);
}

int createAccount()
{
    database = fopen("database", "ab"); //s
    Account newAccount;
    int check;
    newAccount.ID = rand() % INT_MAX;
    while (1)
    {
        printf("Name?\n");
        check = readnadd0(newAccount.name, 20);
        if (!check) continue;
        printf("readnadd0: %s\n", newAccount.name);
        break;
    }
    while (1)
    {
        printf("Surname?\n");
        check = readnadd0(newAccount.surname, 20);
        if (!check) continue;
        break;
    }
    while (1) 
    {
        printf("Address?\n");
        check = readnadd0(newAccount.address, 20);
        if (!check) continue;
        break;
    }
    while (1) 
    {
        printf("PESEL?\n");
        check = readnadd0(newAccount.PESEL, 11);
        if (!check) continue;
        if (strlen(newAccount.PESEL) != 11)
        {
            printf("Too short.\n");
            continue;
        }
        int notNum = 0;
        for (int i = 0; i < strlen(newAccount.PESEL); i++)
        {
            if (!(newAccount.PESEL[i] <= '9' && newAccount.PESEL[i] >= '0')) notNum = 1;
        }
        if (notNum)
        {
            printf("Invalid.\n");
            continue;
        }
        break;
    }
    while (1) 
    {
        printf("Balance?\n");
        char balanceTemp[21];
        size_t length;
        check = readnadd0(balanceTemp, 20);
        if (!check) continue;
        length = strlen(balanceTemp);
        if (length < 3 || balanceTemp[length-3] != '.')
        {
            printf("Wrong format. Requires exactly 2 points of precision.\n");
            continue;
        }
        char* endptr;
        balanceTemp[length-3] = balanceTemp[length-2];
        balanceTemp[length-2] = balanceTemp[length-1];
        balanceTemp[length-1] = '\0';
        errno = 0;
        newAccount.balance = strtol(balanceTemp, &endptr, 10);
        if (*endptr != balanceTemp[length-1])
        {
            printf("Incorrect input. Not a number.\n");
            continue;
        }
        if (errno == ERANGE)
        {
            printf("Value too big.\n");
            continue;
        }
        break;
    }
    fwrite(&newAccount, sizeof(newAccount), 1, database);
    printf("Press any key to continue.");
    getchar();
    if (fclose(database) == EOF) exit(0);
    else return 0; //s
}

int search()
{
    printf("Search by:\n"
           "1. ID\n"
           "2. Name\n"
           "3. Surname\n"
           "4. Address\n"
           "5. PESEL\n");
    int choice, check, c;
    char search[21];
    int searchID;
    while(1) 
    {
        check = scanf("%d", &choice);
        if (flush() || check != 1 || choice > 5 || choice < 1)
        {
            printf("Incorrect input.\n");
            continue;
        }
        break;
    }
    printf("Search string: ");
    if (choice == 1)
    {
        while (1) 
        {
            check = scanf("%d", &searchID);
            if (flush() || !check)
            {
                printf("Incorrect input.\n");
                continue;
            }
            break;
        }
    }
    else
    {
        do {
        check = readnadd0(search, 20);
        } while (!check);
    }
    

    database = fopen("database", "rb");
    Account acc;
    switch(choice)
    {
        case 1:
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (acc.ID == searchID) printAccount(acc);
            }
            break;
        case 2:
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.name, search)) printAccount(acc);
            }
            break;
        case 3:
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.surname, search)) printAccount(acc);
            }
            break;
        case 4:
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.address, search)) printAccount(acc);
            }
            break;
        case 5:
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.PESEL, search)) printAccount(acc);
            }
    }
    printf("Press any key to proceed.");
    getchar();
    return 0;
}

int payment()
{

}

int withdrawal()
{
    
}

int transfer()
{

}

int list()
{
    database = fopen("database", "rb"); //s
    Account acc;
    while(1)
    {
        if (!fread(&acc, sizeof(Account), 1, database)) break;
        printAccount(acc);
    }
    printf("Press any key to proceed.");
    getchar();
    if (fclose(database) == EOF) exit(0);
    else return 0; //s
}

int prompt()
{
    printf("What would you like to do?\n"
            "1. Create an account\n"
            "2. Search for accounts\n"
            "3. Make a payment\n"
            "4. Withdraw money\n"
            "5. Transfer money\n"
            "6. List accounts\n"
            "7. Quit!\n");
    int choice, check, c;
    while(1) 
    {
        check = scanf("%d", &choice);
        if (flush() || check != 1 || choice > 7 || choice < 1) 
        {
            printf("Incorrect input.\n");
            continue;
        }
        break;
    }
    switch(choice)
    {
        case 1:
            createAccount();
            break;
        case 2:
            search();
            break;
        case 3:
            payment();
            break;
        case 4:
            withdrawal();
            break;
        case 5:
            transfer();
            break;
        case 6:
            list();
            break;
        case 7:
            printf("Goodbye!\n");
            exit(0);
    }
    return 0;
}

int main()
{
    srand(time(NULL));
    printf("Hello! Welcome to the Banking System.\n");
    while(1) prompt();
    return 0;
}