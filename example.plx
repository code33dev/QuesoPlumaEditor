/* PL/I Banking System Simulation */
DECLARE MAX_ACCOUNTS FIXED BINARY(31) INIT(100);
DECLARE ACCOUNT_COUNT FIXED BINARY(31) INIT(0);
DECLARE ACCOUNTS (MAX_ACCOUNTS) STRUCT (
    ID FIXED BINARY(31),
    NAME CHAR(50),
    BALANCE DECIMAL(10,2)
);

DECLARE INPUT CHAR(10);
DECLARE CHOICE FIXED BINARY(31);

/* Procedure to Create an Account */
PROC CREATE_ACCOUNT;
    IF ACCOUNT_COUNT >= MAX_ACCOUNTS THEN DO;
        PUT LIST('Error: Maximum account limit reached.');
        RETURN;
    END;
    ACCOUNT_COUNT = ACCOUNT_COUNT + 1;
    ACCOUNTS(ACCOUNT_COUNT).ID = ACCOUNT_COUNT;
    PUT LIST('Enter Name: ');
    GET LIST(ACCOUNTS(ACCOUNT_COUNT).NAME);
    ACCOUNTS(ACCOUNT_COUNT).BALANCE = 0.00;
    PUT SKIP LIST('Account Created! ID:', ACCOUNT_COUNT);
END CREATE_ACCOUNT;

/* Procedure to Deposit Money */
PROC DEPOSIT;
    DECLARE ID FIXED BINARY(31);
    DECLARE AMOUNT DECIMAL(10,2);
    PUT LIST('Enter Account ID: ');
    GET LIST(ID);
    IF ID <= 0 OR ID > ACCOUNT_COUNT THEN DO;
        PUT LIST('Error: Invalid Account ID.');
        RETURN;
    END;
    PUT LIST('Enter Amount to Deposit: ');
    GET LIST(AMOUNT);
    IF AMOUNT <= 0 THEN DO;
        PUT LIST('Error: Invalid Deposit Amount.');
        RETURN;
    END;
    ACCOUNTS(ID).BALANCE = ACCOUNTS(ID).BALANCE + AMOUNT;
    PUT SKIP LIST('Deposit Successful! New Balance: ', ACCOUNTS(ID).BALANCE);
END DEPOSIT;

/* Procedure to Withdraw Money */
PROC WITHDRAW;
    DECLARE ID FIXED BINARY(31);
    DECLARE AMOUNT DECIMAL(10,2);
    PUT LIST('Enter Account ID: ');
    GET LIST(ID);
    IF ID <= 0 OR ID > ACCOUNT_COUNT THEN DO;
        PUT LIST('Error: Invalid Account ID.');
        RETURN;
    END;
    PUT LIST('Enter Amount to Withdraw: ');
    GET LIST(AMOUNT);
    IF AMOUNT <= 0 OR AMOUNT > ACCOUNTS(ID).BALANCE THEN DO;
        PUT LIST('Error: Insufficient Funds or Invalid Amount.');
        RETURN;
    END;
    ACCOUNTS(ID).BALANCE = ACCOUNTS(ID).BALANCE - AMOUNT;
    PUT SKIP LIST('Withdrawal Successful! New Balance: ', ACCOUNTS(ID).BALANCE);
END WITHDRAW;

/* Procedure to Transfer Money */
PROC TRANSFER;
    DECLARE FROM_ID FIXED BINARY(31);
    DECLARE TO_ID FIXED BINARY(31);
    DECLARE AMOUNT DECIMAL(10,2);
    PUT LIST('Enter Sender Account ID: ');
    GET LIST(FROM_ID);
    PUT LIST('Enter Receiver Account ID: ');
    GET LIST(TO_ID);
    IF FROM_ID <= 0 OR FROM_ID > ACCOUNT_COUNT OR TO_ID <= 0 OR TO_ID > ACCOUNT_COUNT THEN DO;
        PUT LIST('Error: Invalid Account IDs.');
        RETURN;
    END;
    PUT LIST('Enter Amount to Transfer: ');
    GET LIST(AMOUNT);
    IF AMOUNT <= 0 OR AMOUNT > ACCOUNTS(FROM_ID).BALANCE THEN DO;
        PUT LIST('Error: Insufficient Funds or Invalid Amount.');
        RETURN;
    END;
    ACCOUNTS(FROM_ID).BALANCE = ACCOUNTS(FROM_ID).BALANCE - AMOUNT;
    ACCOUNTS(TO_ID).BALANCE = ACCOUNTS(TO_ID).BALANCE + AMOUNT;
    PUT SKIP LIST('Transfer Successful! New Balance of Sender: ', ACCOUNTS(FROM_ID).BALANCE);
    PUT SKIP LIST('New Balance of Receiver: ', ACCOUNTS(TO_ID).BALANCE);
END TRANSFER;

/* Procedure to Display Account Details */
PROC DISPLAY_ACCOUNTS;
    DECLARE I FIXED BINARY(31);
    PUT SKIP LIST('Account Details:');
    DO I = 1 TO ACCOUNT_COUNT;
        PUT SKIP LIST('ID: ', ACCOUNTS(I).ID, ' Name: ', ACCOUNTS(I).NAME, ' Balance: ', ACCOUNTS(I).BALANCE);
    END;
END DISPLAY_ACCOUNTS;

/* Main Menu */
PROC MAIN;
    DO WHILE (1);
        PUT SKIP LIST('=========================');
        PUT SKIP LIST('BANKING SYSTEM MENU');
        PUT SKIP LIST('1. Create Account');
        PUT SKIP LIST('2. Deposit Money');
        PUT SKIP LIST('3. Withdraw Money');
        PUT SKIP LIST('4. Transfer Money');
        PUT SKIP LIST('5. Display Accounts');
        PUT SKIP LIST('6. Exit');
        PUT SKIP LIST('Enter Choice: ');
        GET LIST(CHOICE);
        
        SELECT (CHOICE);
            WHEN (1) CALL CREATE_ACCOUNT;
            WHEN (2) CALL DEPOSIT;
            WHEN (3) CALL WITHDRAW;
            WHEN (4) CALL TRANSFER;
            WHEN (5) CALL DISPLAY_ACCOUNTS;
            WHEN (6) DO;
                PUT SKIP LIST('Exiting System. Thank you!');
                RETURN;
            END;
            OTHERWISE PUT SKIP LIST('Invalid Choice! Please try again.');
        END;
    END;
END MAIN;
