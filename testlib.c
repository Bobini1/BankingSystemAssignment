#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

FILE* database;

void dbclose()
{
    if (fclose(database) == EOF)
    {
        perror("Error closing.");
        exit(1);
    }
}

size_t flush()
{
    size_t leftovers = 0;
    int c;
    while ((c = fgetc(stdin)) != '\n' && c != EOF) 
    {
        leftovers++;
    }
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
        if (current == EOF && i == 0)
        {
            fclose(database);
            printf("Fatal error reading input (EOF).\n");
            exit(-1);
        }
        if (current == '\n' || current == EOF)
        {
            shorter = 1;
            break;
        }
        
        else str[i] = current;
    }
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

int confirmation()
{
    int check;
    printf("Are you sure? This will modify the records. y/n\n");
    while(1)
    {
        check = getchar();
        if (check == EOF)
        {
            printf("Fatal error reading input (EOF).\n");
            return 0;
        }
        if (check == '\n' || flush())
        {
            printf("y/n\n");
            continue;
        }
        if (check == 'y')
        {
            return 1;
        }
        if (check == 'n') 
        {
            return 0;
        }
        printf("y/n\n");
    }
}

void displayMoney(long balance)
{
    char buffer[25];
    sprintf(buffer, "%ld", balance);
    size_t stringSize = strlen(buffer);
    if (balance < 0)
    {
        printf("-");
        if (stringSize < 4) 
        {
            printf("0");
            printf(".");
        }
        if (stringSize < 3) printf("0");
        for (size_t i = 1; i < stringSize; i++)
        {
            printf("%c", buffer[i]);
            if (i == stringSize - 3) printf(".");
        }
    }
    else {
        if (stringSize < 3) 
        {
            printf("0");
            printf(".");
        }
        if (stringSize < 2) printf("0");
        for (size_t i = 0; i < stringSize; i++)
        {
            printf("%c", buffer[i]);
            if (i == stringSize - 3) printf(".");
        }
    }
    printf("\n");
}

typedef struct Account
{
    long ID;
    char name[21];
    char surname[21];
    char address[21];
    char PESEL[12];
    long balance;
} Account;

void printAccount(Account acc)
{
    printf("ID: %ld\n", acc.ID);
    printf("Name: %s\n", acc.name);
    printf("Surname: %s\n", acc.surname);
    printf("Address: %s\n", acc.address);
    printf("PESEL: %s\n", acc.PESEL);
    displayMoney(acc.balance);
    printf("\n");
}

long getMaxID()
{
    long maxID = 0;
    FILE* database;
    database = fopen("database", "rb");
    if (database == NULL)
    {
        perror("Could not open the database.");
        exit(2);
    }

    Account acc = {0};
    while(1)
    {
        if (!fread(&acc, sizeof(Account), 1, database)) break;
        if (acc.ID > maxID)
        {
            maxID = acc.ID;
        }
    }

    if (fclose(database) == EOF)
    {
        perror("Error closing.");
        exit(1);
    }

    return maxID;
}

int createAccount()
{
    if (getMaxID() == LONG_MAX)
    {
        printf("The database is full. Can't add a new entry!\n");
    }
    Account newAccount = {0};
    int check;
    database = fopen("database", "ab");
    if (database == NULL)
    {
        perror("Could not open the database.");
        exit(2);
    }
    while (1)
    {
        printf("Name?\n");
        check = readnadd0(newAccount.name, 20);
        if (!check) continue;
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
        for (size_t i = 0; i < strlen(newAccount.PESEL); i++)
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
        char balanceTemp[22];
        size_t length;
        check = readnadd0(balanceTemp, 21);
        if (!check) continue;
        length = strlen(balanceTemp);
        if (length < 3 || balanceTemp[length-3] != '.')
        {
            printf("Wrong format. Requires exactly 2 points of precision and no trailing spaces.\n");
            continue;
        }
        char* endptr;
        balanceTemp[length-3] = balanceTemp[length-2];
        balanceTemp[length-2] = balanceTemp[length-1];
        balanceTemp[length-1] = '\0';
        errno = 0;
        newAccount.balance = strtol(balanceTemp, &endptr, 10);
        if (endptr != &balanceTemp[length-1])
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
    if (confirmation()) 
    {
        newAccount.ID = getMaxID() + 1;
        
        clearerr(database);
        fwrite(&newAccount, sizeof(Account), 1, database);
        if (ferror(database))
        {
            perror("Error writing to file.");
            fclose(database);
            exit(4);
        }
    }
    printf("Press return to proceed.");
    flush();
    if (fclose(database) == EOF) 
    {
        perror("Error closing.");
        exit(1);
    }
    else return 0;
}

int search()
{
    database = fopen("database", "rb");
    if (database == NULL)
    {
        perror("Could not open the database.");
        exit(2);
    }
    printf("Search by:\n"
           "1. ID\n"
           "2. Name\n"
           "3. Surname\n"
           "4. Address\n"
           "5. PESEL\n");
    int choice, check;
    char search[21];
    long searchID;
    while(1)
    {
        choice = getchar();
        if (choice == EOF)
        {
            printf("Fatal error reading input (EOF).\n");
            fclose(database);
            exit(-1);
        }
        if (choice == '\n' || flush() || choice > '5' || choice < '1') 
        {
            printf("Incorrect input.\n");
            continue;
        }
        break;
    }
    printf("Search string: ");
    if (choice == '1')
    {
        char IDTemp[21];
        while (1)
        {
            check = readnadd0(IDTemp, 20);
            if (!check) continue;
            errno = 0;
            char* endptr;
            searchID = strtol(IDTemp, &endptr, 10);
            if (endptr != &IDTemp[strlen(IDTemp)])
            {
                printf("Incorrect input.\n");
                continue;
            }
            if (errno == ERANGE)
            {
                printf("Value too big.\n");
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
    
    Account acc = {0};
    switch(choice)
    {
        case '1':
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (acc.ID == searchID) printAccount(acc);
            }
            break;
        case '2':
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.name, search)) printAccount(acc);
            }
            break;
        case '3':
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.surname, search)) printAccount(acc);
            }
            break;
        case '4':
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.address, search)) printAccount(acc);
            }
            break;
        case '5':
            while(1)
            {
                if (!fread(&acc, sizeof(Account), 1, database)) break;
                if (!strcmp(acc.PESEL, search)) printAccount(acc);
            }
    }

    printf("Press return to proceed.");
    flush();
    if (fclose(database) == EOF)
    {
        perror("Error closing.");
        exit(1);
    }
    else return 0;
}

int payment()
{
    database = fopen("database", "rb+");
    printf("Add funds to what account? (ID)\n");
    int check;
    long ID;
    char IDTemp[21];
    while (1) 
    {
        check = readnadd0(IDTemp, 20);
        if (!check) continue;
        errno = 0;
        char* endptr;
        ID = strtol(IDTemp, &endptr, 10);
        if (endptr != &IDTemp[strlen(IDTemp)])
        {
            printf("Incorrect input.\n");
            continue;
        }
        if (errno == ERANGE) 
        {
            printf("Value too big.\n");
            continue;
        }
        break;
    }
    if (database == NULL)
    {
        perror("Could not open the database.");
        exit(2);
    }
    Account acc = {0};
    while(1)
    {
        if (!fread(&acc, sizeof(Account), 1, database)) 
        {
            printf("There is no account with this ID.\n");
            break;
        }
        if (acc.ID == ID)
        {
            printf("How much to add?\n");
            unsigned long amount;
            while (1) 
            {
                char amountTemp[22];
                size_t length;
                check = readnadd0(amountTemp, 21);
                if (!check) continue;
                length = strlen(amountTemp);
                int i = 0;
                while(isspace(amountTemp[i])) i++;
                if (length < 3 || amountTemp[length-3] != '.' || amountTemp[i] == '-')
                {
                    printf("Wrong format. Requires exactly 2 points of precision "
                    "and no trailing spaces. Must be positive.\n");
                    continue;
                }
                char* endptr;
                amountTemp[length-3] = amountTemp[length-2];
                amountTemp[length-2] = amountTemp[length-1];
                amountTemp[length-1] = '\0';
                errno = 0;
                amount = strtoul(amountTemp, &endptr, 10);
                
                if (endptr != &amountTemp[length-1])
                {
                    printf("Incorrect input. Not a number.\n");
                    continue;
                }
                if (errno == ERANGE || acc.balance > (long long)(LONG_MAX - amount))
                {
                    printf("Value too big.\n");
                    continue;
                }
                acc.balance += amount;
                break;
            }
            check = fseek(database, -(long long)sizeof(Account), SEEK_CUR);
            if (check != 0)
            {
                perror("Error reading the database file.");
                fclose(database);
                exit(3);
            }
            if (confirmation()) 
            {
                clearerr(database);
                fwrite(&acc, sizeof(Account), 1, database);
                if (ferror(database))
                {
                    perror("Error writing to file.");
                    fclose(database);
                    exit(4);
                }
                printf("New balance: ");
                displayMoney(acc.balance);
            }
            break;
        }
    }
    printf("Press return to proceed.");
    flush();
    if (fclose(database) == EOF) 
    {
        perror("Error closing.");
        exit(1);
    }
    else return 0; //s
}

int withdrawal()
{
    database = fopen("database", "rb+");
    if (database == NULL)
    {
        perror("Could not open the database.");
        exit(2);
    }
    printf("Retrieve funds from what account? (ID)\n");
    int check;
    long ID;
    char IDTemp[21];
    while (1) 
    {
        check = readnadd0(IDTemp, 20); //s
        if (!check) continue;
        errno = 0;
        char* endptr;
        ID = strtol(IDTemp, &endptr, 10);
        if (endptr != &IDTemp[strlen(IDTemp)])
        {
            printf("Incorrect input.\n");
            continue;
        }
        if (errno == ERANGE) 
        {
            printf("Value too big.\n");
            continue;
        }
        break;
    }
    Account acc = {0};
    while (1)
    {
        if (!fread(&acc, sizeof(Account), 1, database)) 
        {
            printf("There is no account with this ID.\n");
            break;
        }
        if (acc.ID == ID)
        {
            printf("How much to withdraw?\n");
            unsigned long amount;
            while (1)
            {
                char amountTemp[23];
                size_t length;
                check = readnadd0(amountTemp, 22);
                if (!check) continue;
                length = strlen(amountTemp);
                int i = 0;
                while(isspace(amountTemp[i])) i++;
                if (length < 3 || amountTemp[length-3] != '.' || amountTemp[i] == '-')
                {
                    printf("Wrong format. Requires exactly 2 points of precision "
                    "and no trailing spaces. Must be positive.\n");
                    continue;
                }
                char* endptr;
                amountTemp[length-3] = amountTemp[length-2];
                amountTemp[length-2] = amountTemp[length-1];
                amountTemp[length-1] = '\0';
                errno = 0;
                amount = strtoul(amountTemp, &endptr, 10);
                
                if (endptr != &amountTemp[length-1])
                {
                    printf("Incorrect input. Not a number.\n");
                    continue;
                }
                if (errno == ERANGE)
                {
                    printf("Value too big.\n");
                    continue;
                }
                if (acc.balance < (long long)(LONG_MIN + amount))
                {
                    printf("Not enough money on the account.\n");
                    continue;
                }
                acc.balance -= amount;
                break;
            }
            check = fseek(database, -(long long)sizeof(Account), SEEK_CUR);
            if (check != 0)
            {
                perror("Error reading the database file.");
                fclose(database);
                exit(3);
            }
            if (confirmation()) 
            {
                clearerr(database);
                fwrite(&acc, sizeof(Account), 1, database);
                if (ferror(database))
                {
                    perror("Error writing to file.");
                    fclose(database);
                    exit(4);
                }
                printf("New balance: ");
                displayMoney(acc.balance);
            }
            break;
        }
    }
    printf("Press return to proceed.");
    flush();
    if (fclose(database) == EOF) 
    {
        perror("Error closing.");
        exit(1);
    }
    else return 0; //s
}

int transfer()
{
    int check;
    long ID1, ID2;
    char IDTemp[21];
    database = fopen("database", "rb+");
    if (database == NULL)
    {
        perror("Could not open the database.");
        exit(2);
    }
    printf("Take funds from what account? (ID)\n");
    while (1) 
    {
        check = readnadd0(IDTemp, 20); //s
        if (!check) continue;
        errno = 0;
        char* endptr;
        ID1 = strtol(IDTemp, &endptr, 10);
        if (endptr != &IDTemp[strlen(IDTemp)])
        {
            printf("Incorrect input.\n");
            continue;
        }
        if (errno == ERANGE)
        {
            printf("Value too big.\n");
            continue;
        }
        break;
    }
    printf("Transfer to what account? (ID)\n");
    while (1) 
    {
        check = readnadd0(IDTemp, 20); //s
        if (!check) continue;
        errno = 0;
        char* endptr;
        ID2 = strtol(IDTemp, &endptr, 10);
        if (endptr != &IDTemp[strlen(IDTemp)])
        {
            printf("Incorrect input.\n");
        }
        if (errno == ERANGE)
        {
            printf("Value too big.\n");
            continue;
        }
        break;
    }
    if (ID1 == ID2)
    {
        printf("Can't transfer to the same account.\n");
        printf("Press return to proceed.");
        flush();
        if (fclose(database) == EOF) 
        {
            perror("Error closing.");
            exit(1);
        }
        return 1;
    }
    printf("How much money to move?\n");
    unsigned long amount;
    while (1)
    {
        char amountTemp[22];
        size_t length;
        check = readnadd0(amountTemp, 21);
        if (!check) continue;
        length = strlen(amountTemp);
        int i = 0;
        while(isspace(amountTemp[i])) i++;
        if (length < 3 || amountTemp[length-3] != '.' || amountTemp[i] == '-')
        {
           printf("Wrong format. Requires exactly 2 points of precision "
                    "and no trailing spaces. Must be positive.\n");
            continue;
        }
        char* endptr;
        amountTemp[length-3] = amountTemp[length-2];
        amountTemp[length-2] = amountTemp[length-1];
        amountTemp[length-1] = '\0';
        errno = 0;
        amount = strtoul(amountTemp, &endptr, 10);
        
        if (endptr != &amountTemp[length-1])
        {
            printf("Incorrect input. Not a number.\n");
            continue;
        }
        if (errno == ERANGE)
        {
            printf("Value too low.\n");
            continue;
        }
        break;
    }
    Account acc1 = {0}, acc2 = {0};
    fpos_t acc1Pos, acc2Pos;
    while(1)
    {
        if (!fread(&acc1, sizeof(Account), 1, database))
        {
            printf("The account to move money from doesn't exist.\n");
            printf("Press return to proceed.");
            flush();
            if (fclose(database) == EOF) 
            {
                perror("Error closing.");
                exit(1);
            }
            else return 1; //s
        }
        
        if (acc1.ID == ID1)
        {
            if (acc1.balance < (long long)(LONG_MIN + amount))
            {
                printf("Not enough money on the first account.\n");
                printf("Press return to proceed.");
                flush();
                if (fclose(database) == EOF) 
                {
                    perror("Error closing.");
                    exit(1);
                }
                else return 1;
            }
            else 
            {
                acc1.balance -= amount;
                check = fseek(database, -(long long)sizeof(Account), SEEK_CUR);
                if (check != 0)
                {
                    perror("Error reading the database file.\n");
                    fclose(database);
                    exit(3);
                }
                check = fgetpos(database, &acc1Pos);
                if (check != 0)
                {
                    perror("Error reading the database file.\n");
                    fclose(database);
                    exit(5);
                }
            }
            break;
        }
    }
    check = fseek(database, 0, SEEK_SET);
    if (check != 0)
    {
        perror("Error reading the database file.\n");
        fclose(database);
        exit(3);
    }
    while(1)
    {
        if (!fread(&acc2, sizeof(Account), 1, database))
        {
            printf("The account to move money to doesn't exist.\n");
            printf("Press return to proceed.");
            flush();
            if (fclose(database) == EOF)
    {
        perror("Error closing.");
        exit(1);
    }
            return 1; //s
        }
        
        if (acc2.ID == ID2)
        {
            if (acc2.balance > (long long)(LONG_MAX - amount))
            {
                printf("Not enough money on the first account.\n");
                printf("Press return to proceed.");
                flush();
                if (fclose(database) == EOF)
    {
        perror("Error closing.");
        exit(1);
    }
                return 1;
            }
            else 
            {
                acc2.balance += amount;
                check = fseek(database, -(long long)sizeof(Account), SEEK_CUR);
                if (check != 0)
                {
                    perror("Error reading the database file.");
                    fclose(database);
                    exit(3);
                }
                check = fgetpos(database, &acc2Pos);
                if (check != 0)
                {
                    perror("Error reading the database file.");
                    fclose(database);
                    exit(5);
                }
            }
            break;
        }
    }
    if (confirmation())
    {
        check = fsetpos(database, &acc1Pos); //s
        if (check)
        {
            perror("Error reading from file.");
            fclose(database);
            exit(6);
        }

        clearerr(database);
        fwrite(&acc1, sizeof(Account), 1, database);
        if (ferror(database))
        {
            perror("Error writing to file.");
            fclose(database);
            exit(4);
        }

        printf("New balance of the first account: ");
        displayMoney(acc1.balance);

        check = fsetpos(database, &acc2Pos);
        if (check)
        {
            printf("Error reading from file.\n");
            fclose(database);
            exit(6);
        }

        clearerr(database);
        fwrite(&acc2, sizeof(Account), 1, database);
        if (ferror(database))
        {
            perror("Error writing to file.");
            fclose(database);
            exit(4);
        }
        printf("New balance of the second account: ");
        displayMoney(acc2.balance);
    }

    printf("Press return to proceed.");
    flush();
    if (fclose(database) == EOF) 
    {
        perror("Error closing.");
        exit(1);
    }
    else return 0; //s
}

int list()
{
    database = fopen("database", "rb");
    if (database == NULL)
    {
        perror("Could not open the database.");
        exit(2);
    }
    Account acc = {0};
    while(1)
    {
        if (!fread(&acc, sizeof(Account), 1, database)) break;
        printAccount(acc);
    }
    printf("Press return to proceed.");
    flush();
    if (fclose(database) == EOF) 
    {
        perror("Error closing.");
        exit(1);
    }
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
    int choice;
    while(1)
    {
        choice = getchar();
        if (choice == EOF)
        {
            printf("Fatal error reading input (EOF).\n");
            exit(-1);
        }
        if (choice == '\n' || flush() || choice > '7' || choice < '1') 
        {
            printf("Incorrect input.\n");
            continue;
        }
        break;
    }
    switch(choice)
    {
        case '1':
            createAccount();
            break;
        case '2':
            search();
            break;
        case '3':
            payment();
            break;
        case '4':
            withdrawal();
            break;
        case '5':
            transfer();
            break;
        case '6':
            list();
            break;
        case '7':
            printf("Goodbye!\n");
            exit(0);
    }
    return 0;
}

int main()
{
    printf("Hello! Welcome to the Banking System.\n");
    while(1) prompt();
    return 0;
}