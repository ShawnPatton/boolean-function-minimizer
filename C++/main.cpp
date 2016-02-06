//*********************************  PROGRAM IDENTIFICATION  ********************************
//*                                                                                         *
//*     PROGRAM FILE NAME:  main.cpp          ASSIGNMENT #:  1          Grade: _______      *
//*                                                                                         *
//*     PROGRAM AUTHOR:     _________________________________________                       *
//*                                   Shawn Patton                                          *
//*     COURSE #:  CSC 40300 11                         DUE DATE:  Oct   5, 2015            *
//*                                                                                         *
//*******************************************************************************************

//*********************************  PROGRAM DESCRIPTION  ***********************************
//*                                                                                         *
//*     PROCESS: This program reads in terms for a boolean function and minimizes them      *
//*         via Karnaugh Maps. Works for up to 6-bit boolean functions. Ambiguous blank     *
//*         output for trivially true or false functions, which you wouldn't input anyway.  *
//*                                                                                         *
//*******************************************************************************************

#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <vector>
#include <cmath>
using namespace std;

//struct declaration
struct BoolFunc{
    unsigned long long bits= 0; //reads right-left as going top-down in truth table
        // ordered from all false to all true w/ variables progressing in certain order
    unsigned short mult = 0; //bits signifying which vars are simplified out
};

//class declarations

//function prototypes
void ReadTerm();
void ReadLine();
void Simplify(BoolFunc, int, int);
void RecursiveSimplify(BoolFunc, int, int);
unsigned long long Implies(BoolFunc);
void PrintTerm(BoolFunc, unsigned long long);
void Print();


//global variables
ifstream InFile;
ofstream OutFile;
bool notdone = true; //for termination
int NumBits = -1; //number of boolean variables
unsigned long long Mask[6] = {0x00000000FFFFFFFF, 0x0000FFFF0000FFFF, 0x00FF00FF00FF00FF,
    0x0F0F0F0F0F0F0F0F, 0x3333333333333333, 0x5555555555555555}; //bitmask where vars=false
unsigned short shift[6] = {32, 16, 8, 4, 2, 1}; //number of bits to shift, used frequently
    //and this is faster than calculating powers of 2
unsigned int used[6]; //used entries in rows of terms[]
BoolFunc terms[6][20]; //is filled and processed through iteration that is isomorphic
    //to tree structure, full size of 20 is only used in 1 row of 6-bit case
BoolFunc original; //holds minterms derived from original specification
unsigned long long accounted; //holds minterms that are accounted for by printed terms
//*******************************************************************************************
int main()
{
    InFile.open("BoolMinINPUT.txt");
    OutFile.open("BoolMinOUTPUT.txt");
    InFile >> noskipws;

    while(notdone)
    {
        ReadLine();
        RecursiveSimplify(original, 6-NumBits, 0);
        Print();
    }

    InFile.close();
    OutFile.close();
    return 0;
}
//*******************************************************************************************
void ReadTerm()
{
    //Given - nothing
    //Task - reads in a term
    //Returns - nothing
    char tempchar;
    unsigned long long minterm = 1; //if we are reading a term, some spot in the truth table
        //is gonna be 1, we just need to find where, starting from the right, where
        //all vars = false
    for(int i=0; i<NumBits; i++) //each char that is read confines us to the left or right
        //half of the currently available bits, eventually narrowing down to one spot
    {
        InFile.get(tempchar);
        if(isupper(tempchar))
        {
            minterm <<= shift[6-NumBits+i]; //shifting left by specific power of 2, places
                //it in position where other read vars are in specified position
                //but this var = true
        }
    }
    original.bits |= minterm; //could use addition, but this avoids errors from redundant
        //listing of minterms in input
    InFile.get(tempchar); //get rid of the space after the term
    return;
}
//*******************************************************************************************
void ReadLine()
{
    //Given - nothing
    //Task - reads a line of input into terms[]
    //Returns - nothing
    for(int i = 0; i<6; i++)
    {
        used[i] = 0;//we're overwriting the contents of terms[], in place,
        //instead of clearing it between lines of input
    }

    original.bits=0; //reset

    //counts size of terms
    char tempchar;
    NumBits = -1;
    do{InFile.get(tempchar);NumBits++;}while(isalpha(tempchar));
    for(int i = 0; i <= NumBits; i++)
    {
        InFile.unget(); //goes back to start of line, tellg and seekg caused huge errors
            //this was somehow the miracle fix. I think this is what broke the other versions
    }

    do{
        ReadTerm();
        tempchar = InFile.peek();
    }while(tempchar != 'X');
    InFile.get(tempchar);
    InFile.get(tempchar);
    tempchar = InFile.peek();
    notdone = (tempchar != 'S');
    return;
}
//*******************************************************************************************
void Simplify(BoolFunc func, int var, int row)
{
    //Given - a BoolFunc, an index representing which var to simplify out, and row number
    //Task - simplifies out that var through bitwise operations
    //Returns - nothing, puts simplified BoolFunc in terms[] in specified row
    terms[row][used[row]].bits = func.bits&Mask[var]&(func.bits>>shift[var]);
    terms[row][used[row]].mult = func.mult+shift[var];
    used[row]++;
    return;
}
//*******************************************************************************************
void RecursiveSimplify(BoolFunc func, int var, int row)
{
    //Given - same inputs as for Simplify
    //Task - runs Simplify on a BoolFunc for a range of variables, and recursively calls
        //itself on all the resulting BoolFuncs
    //Returns - nothing
    for(int i = var; i<5; i++)
    {
        Simplify(func, i, row); //does the actual work of simplifying
        RecursiveSimplify(terms[row][used[row]-1], i+1, row+1); //recursively does this
            //process on the BoolFunc just created by simplifying, for fewer variables
            //, putting result in next row down. does pre-traversal order on the underlying
            //tree-like structure
    }
    Simplify(func, 5, row); //same as inside of for-loop, when i = 5, except without
        //recursion because we've hit the condition for the end of a branch
    return;
}
//*******************************************************************************************
unsigned long long Implies(BoolFunc func)
{
    //Given - a BoolFunc
    //Task - see which minterms it implies
    //Return - the bits of the minterms this BoolFunc implies, in the horizontal
        //truth table format we've been using so much

    for(int i = 0; i<NumBits;i++)
    {
        if(func.mult%2)//checking if var was simplified out
        {
            func.bits |= func.bits << shift[5-i];//expand out further
        }
        func.mult >>= 1;//next var
    }
    return func.bits ;
}
//*******************************************************************************************
void PrintTerm(BoolFunc func, unsigned long long newaccount)
{
    //Given - a BoolFunc with a single 1 in it, and the new value for accounted
    //Task - prints the term and updates accounted
    //Returns - nothing
    string lowchars = "abcdef";
    string highchars = "ABCDEF";
    OutFile << '(';
    for(int i = 0; i<NumBits; i++)//checking the vars in order of original presentation
    {
        if((func.mult >> (NumBits-1-i))%2 == 0)//checking which vars aren't simplified out
        {
            if((func.bits & Mask[6-NumBits+i]) > 0)//checking if var is false
            {
                OutFile << lowchars[i];
            }else
            {
                OutFile << highchars[i];
            }
        }
    }
    accounted = newaccount;
    OutFile << ") + ";
    return;
}
//*******************************************************************************************
void Print()
{
    //Given - nothing
    //Task - prints everything while avoiding redundancy
    //Return - nothing
    accounted = 0;
    BoolFunc temp;
    unsigned long long tempaccount; //avoiding small amount of recalculation

    OutFile << "Answer is: ";

    for(int i = NumBits-1; i>=0;i--)//goes through used rows, from most to least simplified
    {
        for(int j = 0; j<used[i];j++)//goes through all terms in row
        {
            if((accounted | Implies(terms[i][j])) > accounted)//if term implies something not
                //yet accounted for by other printed terms
            {
                //it's technically a conglomeration of terms, separate them out.
                //I got a trick for splitting a binary string into strings with single
                //ones in the same places
                temp.mult = terms[i][j].mult;
                temp.bits = 1;//single one that we'll pull to the left
                for(int k=2*shift[6-NumBits];k>0;k--)//only check as many bits as necessary
                {
                    if(terms[i][j].bits%2)//the bit we're checking comes from left later on
                    {
                        tempaccount = accounted | Implies(temp);
                        if(tempaccount > accounted)//if the term represented
                            //by this specific bit implies something not yet accounted for
                            PrintTerm(temp, tempaccount);
                    }
                    terms[i][j].bits >>= 1;//wherever the bit that's now in the 1s place
                        //originally came from
                    temp.bits <<= 1;//that's the place this lone one is at now
                }
            }
        }
    }
    //mostly the same code as in the above loop, but for unsimplified terms unaccounted for
    if(accounted < original.bits)
    {
        temp.mult = 0;
        temp.bits = 1;
        for(int k=2*shift[6-NumBits];k>0;k--)
        {
            if(original.bits%2)
            {
                tempaccount = accounted | Implies(temp);
                if(tempaccount > accounted)
                    PrintTerm(temp, tempaccount);
            }
            original.bits >>= 1;
            temp.bits <<= 1;
        }
    }
    OutFile << endl;
    return;
}
//*******************************************************************************************
