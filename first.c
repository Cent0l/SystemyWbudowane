#include <stdio.h>
#include <stdlib.h>

int my_add(int a, int b);


int main()
{
    int liczba1, liczba2;
    printf("podaj liczbe\n");
    scanf("%d",&liczba1);
    printf("podaj liczbe\n");
    scanf("%d",&liczba2);
    printf("twoja suma to %d\n",my_add(liczba1,liczba2));
    return 0;
}


int my_add(int a, int b)
{
    return a+b;
}
