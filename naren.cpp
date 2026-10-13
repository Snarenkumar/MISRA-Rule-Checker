#include <iostream>

void replacewithx(char arr[], int n)
{
    int i = 0;

    while (arr[i] != '\0')
    {
        //toupper from lower
        if (arr[i] >= 'a' && arr[i] <= 'z'  ){
            arr[i]-=32;
        }
        i++;
    }
}


void replacewithx(char arr[], int n)
{
    int i = 0;

    while (arr[i] != '\0')
    {
        //tolower from upper 
        if (arr[i] >= 'a' && arr[i] <= 'z'  ){
            arr[i]+=32;
        }
        i++;
    }
}




int main()
{
    char arr[20] = "my name is naren";

    std::cout << "it works proerply ";

    return 0;
}