#include<iostream>
using namespace std;

bool is_palindrome( string s ) {
    if (s.length() < 2)
        return true;
    else
        return (s[0] == s[s.length() - 1])&& is_palindrome(s.substr(1, s.length() - 2));
}

int main(){
    string s;
    string temp = "";
    int tem;
    getline(cin,s);
    for (int i = 0; i < s.length();i++){
        if (s[i] >= 'a' && s[i] <= 'z' || s[i] >= 'A' && s[i] <= 'Z')
            temp = temp + s[i];
    }
    s = temp;
    for (int i = 0; i < temp.length();i++){
        if (temp[i] >= 'A' && s[i] <= 'Z'){
            tem = (temp[i]-'A') % 26;
            //cout<<tem<<endl;
            tem = 'a' + tem;
            //cout<<tem<<endl;
            temp[i] = tem;
        }
    }
    s = temp;
    if (is_palindrome(s))
        cout<<"The input string is a palindrome."<<endl;
    else
        cout<<"The input string is not a palindrome."<<endl;
}