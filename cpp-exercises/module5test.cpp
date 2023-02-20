#include<iostream>
using namespace std;
int factorial(int num)
{
    if (num == 0)
        return 1;
    else
        return num * factorial(num - 1);
}
int fib(int num) {
    if (num < 2){
        if (num == 1)
            return 1;
        else
            return 0;}
    else{
        return num = fib(num
    -1) +fib(num - 2);}
}
void stars(int n)
{
    cout << '*';
    if (n > 1)
        stars(n - 1);
    else
        cout<<endl;
}

int gcd(int x, int y){
    if (y == 0)
        return x;
    else
        return gcd(y, x%y);

}

int main(){
    int num1,num2,gcdnum;
    int factorials;
    int fibnum;
    cout<<"type integer numbers: ";
    cin>>num1>>num2;
    stars(num1);
    factorials = factorial(num1);
    cout<<"here is the factorials of "<<num1<<": "<<factorials<<endl;
    fibnum = fib(num1);
    cout<<"here is the Fibonacci number of "<<num1<<": "<<fibnum<<endl;
    gcdnum = gcd(num1,num2);
    cout<<gcdnum<<endl;
    cout<<-5%7<<endl;

}