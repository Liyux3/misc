#include<iostream>
using namespace std;

int main(){
    int SIZE;
    cin>>SIZE;
    int *set = new int[SIZE];
    int *number = new int[SIZE];
    for (int i=0;i<SIZE;i++){
        cin>>set[i];
    }
    for (int i=0;i<SIZE;i++){
        for (int j=0;j<i;j++){
            if (set[i] == set[j]){number[i]++;}
        }
    }

    int max_count = 0;
    int count,index;
    for (int i=0;i<SIZE;i++){
        count = number[i];
        if (count >= max_count){
            index = i;
            max_count = count;
        }
    }

    cout<<"The mode of the set is "<<set[index]<<endl;

    delete[] set;
    delete[] number;
}
