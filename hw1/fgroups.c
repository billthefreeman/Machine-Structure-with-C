/*
 * fgroups.c
 * written by: ben tanen & feiyu lu
 * assignment: COMP-40 hw #1 (part b)
 *
 * fgroups.c is a program that reads in input that describes fingerprint groups.
 * Based on the specifications from the assignment, no name can be used more
 * than once and fingerprint groups will less than 2 members are ignored.
 * This program will parse through the input and store the based data.
 * The implementation of this program uses Hanson sets and tables
 * A table is used to keep associated groups of names and fingerprints
 * (in key value pairs)
 * Sets are used to keep the members of each group (values of table structure)
 * A large set is also used to make sure that duplicate names are not used
 */

#include <stdio.h>
#include <stdlib.h>
#include <atom.h>
#include <set.h>
#include <table.h>

/* define set max lengths of fingerprints and names */
#define fingerprintLength 512
#define nameLength 512

/* functions for reading / parsing input */
void readFingerprint(char str[], int length);
void readName(char str[], int length);
void discardInputLine();

/* functions for adding and print from tables / sets */
void printSet(const void *member, void *cl);
void printTable(const void *key,void **value, void *cl);
void addToTable(Table_T table, const void *key, const void *value);

int main()
{
        /*
         * declare mainNameSet (to store all the names)
         * and mainTable (to hold all fingerprint groups)
         */
        Set_T mainNameSet = Set_new(1000,NULL,NULL);
        Table_T mainTable = Table_new(1000,NULL,NULL);

        /* until end of file is read (feof() != when EOF) */
        while (feof(stdin) == 0) {
                /* declare fingerprint and name strings */
                char fingerprint[fingerprintLength];
                char name[nameLength];

                /* read in fingerprint from input */
                readFingerprint(fingerprint,fingerprintLength);

                /* if fingerprint isn't empty string */
                if (fingerprint[0] != '\0') {
                        /* declare fprint and name Atoms */
                        const char *fprintAtom;
                        const char *nameAtom;

                        /* read in name from input */
                        readName(name,nameLength);

                        /* convert fingerprint and name to Atoms */
                        fprintAtom = Atom_string(fingerprint);
                        nameAtom = Atom_string(name);

                        /* check if in mainNameSet */
                        if (Set_member(mainNameSet,nameAtom) == 0) {
                                /* add name to mainNameSet */
                                Set_put(mainNameSet,nameAtom);

                                /* add to table */
                                addToTable(mainTable,fprintAtom, nameAtom);
                        } else {
                                fprintf(stderr,"%s already used!\n",nameAtom);
                        }
                }
        }

        /* print contents of table */
        Table_map(mainTable,printTable,stdout);

        /* free mainTable and mainSet memory */
        Table_free(&mainTable);
        Set_free(&mainNameSet);
        return EXIT_SUCCESS;
}

/* read fingerprint from line of input (under specified conditions) */
void readFingerprint(char str[], int length)
{
        /* declare iterating character array */
        char iterChar[2];

        /* checks if END-OF-FILE reached */
        if (fgets(iterChar,2,stdin) == NULL) {
                str[0] = '\0';
                return;

        /* checks if line starts on non-white space character */
        } else if (iterChar[0] == ' ') {
                discardInputLine();
                str[0] = '\0';
                return;
        }

        /* iterate over each character to put into fingerprint */
        for (int i=0;i<length;i++) {
                /* if whitespace or new-line hit, end reading */
                if (iterChar[0] == ' ' || iterChar[0] == '\n') {
                        str[i] = '\0';
                        break;
                } else {
                        str[i] = iterChar[0];
                }

                fgets(iterChar,2,stdin);
        }

        /* add terminating character to string (at max position) */
        str[length-1] = '\0';

        /*
         * check if there still exists non-white space characters to read
         * that are beyond size of array (more than fingerprintLength)
         */
        if (iterChar[0] != ' ') {
                discardInputLine();
                str[0] = '\0';
        }
}

/* read name from a particular line of input up until length limit (512char) */
void readName(char str[], int length)
{
        /* declare iterating character array */
        char iterChar[2];

        /* checks if starting on non-white space character, loops until */
        fgets(iterChar,2,stdin);
        while (iterChar[0] == ' ') {
                fgets(iterChar,2,stdin);
        }

        /* iterates over full name length (nameLength) */
        for (int i=0;i<length;i++) {
                /* checks if end of line */
                if (iterChar[0] == '\n') {
                        str[i] = '\0';
                        break;
                /* otherwise stores character in str */
                } else {
                        str[i] = iterChar[0];
                }

                /* runs fgets, checks if at end of file */
                if (fgets(iterChar,2,stdin) == NULL) {
                        str[i+1] = '\0';
                        break;
                }
        }

        /* set last character of array to string-terminating char */
        str[length-1] = '\0';

        /*
         * if character is not end of line/file (signifying name is too long)
         * report error and skip characters until end of line
         */
        if (iterChar[0] != '\n' && feof(stdin) == 0) {
                fprintf(stderr,"name too long, using %s\n",str);
                while (iterChar[0] != '\n' && feof(stdin) == 0) {
                        fgets(iterChar,2,stdin);
                }
        }
}

/*
 * in the case that input is bad format
 * read until the end of the line '\n' and continue normally
 */
void discardInputLine()
{
        /* declare iterating character array */
        char iterChar[2];
        fgets(iterChar,2,stdin);

        /* output error msg */
        fprintf(stderr,"bad format, skipping line\n");

        /* read until end of line or end of file */
        while (iterChar[0] != '\n' && feof(stdin) == 0) {
                fgets(iterChar,2,stdin);
        }
}

/* function used to print each element of set in correct format */
void printSet(const void *member, void *cl)
{
        // print each member to cl (likely stdout)
        FILE *fp = cl;
        fprintf(fp,"%s\n",(char *)member);
}

/*
 * function used to print elements of table (value sets)
 * and then free those particular sets
 */
void printTable(const void *key,void **value, void *cl)
{
        // file pointer to print to (generally stdout)
        FILE *fp = cl;

        /*
         * value is the set associated with a particular fprint key
         * if size of set / fingerprint group is greater than 1, print
         */
        if (Set_length(*value) > 1) {
                Set_map(*value,printSet,fp);
                fprintf(fp,"\n");
        }

        /* after printing, free this set from memory */
        Set_free((Set_T *)value);
        (void) key;
}

/*
 * function to add a particular nameAtom to the according set
 * if there is no row yet for corresponding fingerprint, make new set
 * if fingerprint has already been added, add new name to existing set
 */
void addToTable(Table_T table, const void *key,const void *value)
{
        /*
         * get set corresponding to particular key
         * if set doesn't exist, nameSet == NULL
         */
        Set_T nameSet = Table_get(table, key);

        /*
         * if fingerprint group hasn't been added
         * nameSet will be returned as NULL
         */
        if (nameSet == NULL) {
                nameSet = Set_new(100,NULL,NULL);
        }

        /* put name in set and put set in table */
        Set_put(nameSet,value);
        Table_put(table,key,nameSet);
}
