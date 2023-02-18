#include<iostream>
using namespace std;
int find_power(int number,int power){
    int output = 1;
    for (int i=0;i<power;i++){
        output = output*number;
    }
    return output;
}
int main(){
    int array[15];
    int i;
    cin>>i;
    if (i <= 15) {
        for (int j = 0; j < i; j++) {
            array[j] = j * j;}
        if ((15 - i - 5) > 0) {
            for (int k = i; k < (i + 5); k++) {
                array[k] = 0;}
            for (int x = (i + 5); x < 15; x++) {
                array[x] = find_power(3, x);}}
        else {
            for (int k = i; k < 15; k++)
                array[k] = 0;}}
    else {
        for (int k = 0; k < 15; k++) {
            array[k] = k * k;}}
    for (int y=0;y<15;y++){
        cout<<array[y]<<" ";
    }
}
