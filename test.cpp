#include <iostream>
#include <algorithm>
#include <alloca.h>
#include <vector>

using namespace std;

class vec
{
public:
    int a;
    int *b;
public:
    vec (int _vec) : a(_vec) { b = new int[10]; };
    vec () : a(10) {};
    vec(const vec& v)
    {
        a = v.a;
        b = v.b;
        cout<<"==="<<endl;
    }
    
    vec& operator=(vec& v)
    {
        a = v.a;
        cout<<"==="<<endl;
    }
    
    vec& operator=(vec&& v)
    {
        cout<<"右值"<<endl;
    }
};

int main()
{
    allocator<vec> alloc;
    vec* test = alloc.allocate(5);
    vec test1[10];
    vec test2[10];
    for (int i = 0; i < 10; i++)
        test1[i] = vec(i);
    vector<vec> v;
    cout<<"vector"<<endl;
    v.push_back(vec(10));
    cout<<endl;
    *test = std::move(test1[1]);
    cout<<test->a<<endl;
    copy(test1, test1 + 10, test2);
    cout<<std::is_trivially_copy_assignable<pair<int, int>>::value<<endl;
    for(int i = 0; i < 10; i++)
        cout << test1[i].b << " " << test2[i].b << endl;
    vec a1(10);
    vec b1 = a1;
    cout << b1.a << endl;
    return 0;  
}