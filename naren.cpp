#include <iostream>
#include <climits>

void val(int num)
{
    // Base case
    // if (num> 0){
    //     return;
    // }

    if (num == 0)
    {
        return;
    }

    val(num / 10);

    int digit = num % 10;
    
}

int main()
{
    int num = 13432;

    val(num);
}