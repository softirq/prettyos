#include "type.h"
#include "const.h"
/*#include "global.h"*/
#include "stdlib.h"
#include "math.h"

int power(int index)
{
    int sum = 1;
    if(index == 0)
        return 1;

    if(index > 0)
    {
        while(index-- >0)
            sum *= 2;
    }
    /*else
    {
        while(index-- >0)
            sum /= 2;
    }*/

    return sum;
}
