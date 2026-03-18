// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// Global constant for efficiency
#define RECORD_SIZE (sizeof(struct clientData))
static const struct clientData BLANK_CLIENT = {0, "", "", 0.0};

// prototypes
unsigned int enterChoice(void);
void listAllAccounts(FILE *readPtr);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void clearInputBuffer(void);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; if it doesn't exist, create it and initialize all records to blank
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        FILE *newPtr;
        struct clientData blankClient = {0, "", "", 0.0};

        if ((newPtr = fopen("credit.dat", "wb+")) == NULL)
        {
            printf("%s: File could not be opened or created.\n", argv[0]);
            exit(-1);
        }

        // initialize file with 100 blank records
        for (int i = 0; i < 100; ++i)
        {
            fwrite(&blankClient, RECORD_SIZE, 1, newPtr);
        }

        fclose(newPtr);

        // reopen for reading/writing
        if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
        {
            printf("%s: File could not be reopened.\n", argv[0]);
            exit(-1);
        }
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 6)
    {
        switch (choice)
        {
        // list all accounts
        case 1:
            listAllAccounts(cfPtr);
            break;
        // create text file from record file
        case 2:
            textFile(cfPtr);
            break;
        // update record
        case 3:
            updateRecord(cfPtr);
            break;
        // create record
        case 4:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 5:
            deleteRecord(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
} // end main

// clear input buffer helper function
void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (fread(&client, RECORD_SIZE, 1, readPtr) == 1)
        {
            // write single record to text file
            if (client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
    }         // end else
} // end function textFile

// list all accounts to stdout
void listAllAccounts(FILE *readPtr)
{
    struct clientData client = {0, "", "", 0.0};

    rewind(readPtr); // sets pointer to beginning of file
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, RECORD_SIZE, 1, readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                   client.balance);
        }
    }
}

// update balance in record - OPTIMIZED
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update (1 - 100): ");
    scanf("%u", &account);
    clearInputBuffer();

    // move file pointer to correct record (SINGLE SEEK)
    fseek(fPtr, (account - 1) * RECORD_SIZE, SEEK_SET);
    
    // read first to check if exists, then update in place
    if (fread(&client, RECORD_SIZE, 1, fPtr) != 1 || client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
        return;
    }

    // display current record
    printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

    // request transaction amount from user
    printf("%s", "Enter charge (+) or payment (-): ");
    scanf("%lf", &transaction);
    clearInputBuffer();
    
    client.balance += transaction; // update record balance

    printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

    // move back and write (SINGLE OPERATION from current position)
    fseek(fPtr, -(long)RECORD_SIZE, SEEK_CUR);
    fwrite(&client, RECORD_SIZE, 1, fPtr);
} // end function updateRecord

// delete an existing record - OPTIMIZED
void deleteRecord(FILE *fPtr)
{
    unsigned int accountNum; // account number
    struct clientData client;

    // obtain number of account to delete
    printf("%s", "Enter account number to delete (1 - 100): ");
    scanf("%u", &accountNum);
    clearInputBuffer();

    // move file pointer to correct record and read to verify
    fseek(fPtr, (accountNum - 1) * RECORD_SIZE, SEEK_SET);
    if (fread(&client, RECORD_SIZE, 1, fPtr) != 1 || client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
        return;
    }

    // seek back and write blank record
    fseek(fPtr, -(long)RECORD_SIZE, SEEK_CUR);
    fwrite(&BLANK_CLIENT, RECORD_SIZE, 1, fPtr);
    printf("Account %d deleted successfully.\n", accountNum);
} // end function deleteRecord

// create and insert record - OPTIMIZED
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    // obtain number of account to create
    printf("%s", "Enter new account number (1 - 100): ");
    scanf("%u", &accountNum);
    clearInputBuffer();

    // move file pointer to correct record and check if exists
    fseek(fPtr, (accountNum - 1) * RECORD_SIZE, SEEK_SET);
    if (fread(&client, RECORD_SIZE, 1, fPtr) == 1 && client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
        return;
    }

    // create new record
    printf("%s", "Enter lastname, firstname, balance\n? ");
    scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);
    clearInputBuffer();

    client.acctNum = accountNum;
    
    // write new record (already positioned correctly)
    fwrite(&client, RECORD_SIZE, 1, fPtr);
    printf("Account #%d created successfully.\n", accountNum);
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    
    // display available options
    printf("%s", "\nEnter your choice\n"
                 " 1 - list all accounts\n"
                 " 2 - store a formatted text file of accounts called\n"
                 "     \"accounts.txt\" for printing\n"
                 " 3 - update an account\n"
                 " 4 - add a new account\n"
                 " 5 - delete an account\n"
                 " 6 - end program\n? ");

    scanf("%u", &menuChoice);
    clearInputBuffer();
    return menuChoice;
} // end function enterChoice