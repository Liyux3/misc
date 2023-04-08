#include<iostream>
using namespace std;

//input 3 string, the third string will place occurrence of string2 in string 1
//checkpoint 6.6
int main(){
    string s1,s2,s3;
    getline(cin,s1);
    getline(cin,s2);
    getline(cin,s3);
    int i = 0;
    while ((s1.find(s2,i)) != -1 ) {
        s1.replace(s1.find(s2, i), s2.length(), s3);
        if (s3.find(s2) != -1) {
            i = s1.find(s2, i) + s2.length();}
        else i = s1.find(s2,i);
    }
    cout<<s1<<endl;
}
