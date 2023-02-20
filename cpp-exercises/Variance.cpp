#include<iostream>
using namespace std;

int main(){
    double number[30];
    int length;
    double mean,temp;
    double sum = 0;
    double var = 0;
    cin >> length;
    for(int i = 0;i < length;i++){
        cin>>temp;
        sum += temp;
        number[i] = temp;
    }
    mean = (sum/length);
    for(int i = 0;i < length;i++){
        var += ((number[i] - mean)*(number[i] - mean));
    }
    var = var/length;
    cout<<var<<endl;
}