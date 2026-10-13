#include <iostream>
#include <climits>
using namespace std;

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
    cout << "digit is this :  " << digit << endl;
}

int main()
{
    int num = 13432;

    val(num);
}