/////////////////////////////////////////////////////////////////////////////////////////////
//
// XC8 Stuff
//

#include "defines.h"
#include "../output/core_locations.h"


////////////////
//
// A7 detection

// Clear the CLC interrupt flag
//    PIR4bits.CLC1IF = 0;
//   Sleep();
//    while (a7i) // ((CLC1CONbits.LC1OUT))
// while (CLC1CONbits.LC1OUT) \

bool a7i = false;
bool s1, s2, s3, s4;

void
WHILE_A7()
{
    do
    {
        s1 = A11_RAW;
        s2 = A7_RAW;
        s3 = A11_RAW;
        s4 = A7_RAW;
        if (s1 && s3)
        {
            if (s2 == s4)
                a7i = s2;
        }
    } while(a7i);
}

void
WHILE_NOT_A7()
{
    do
    {
        s1 = A11_RAW;
        s2 = A7_RAW;
        s3 = A11_RAW;
        s4 = A7_RAW;
        if (s1 && s3)
        {
            if (s2 == s4)
                a7i = s2;
        }
    } while(!a7i);
}



