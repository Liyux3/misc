#include <iostream>
#include <string>
using namespace std;

struct courses{
    string info,lecturer,name;
};
int main() {
    string input,temp;
    courses course[30];

    int i=0;
    cin>>input;
    while (input != "exit") {
        if (input == "add") {
            cin>>input;
            course[i].name = input;
            cin>>input;
            course[i].info = input;
            cin>>input;
            course[i].lecturer = input;
            i++;}
        if (input == "show") {
            cin>>input;
            for (int j=0;j<=i;j++){
                if (course[j].name == input){
                    cout<<"Name: "<<course[j].info<<", Lecturer: "<<course[j].lecturer<<endl;
                }
            }
        }
        cin>>input;
    }
    return 0;
}