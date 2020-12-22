#include<iostream>
#include<alloca.h>
#include<memory.h>
#include <vector>

using namespace std;

class test
{
public:
    int a;
    int b;
    int *c;
    // test(int _a, int _b) : a(_a), b(_b) {c = new int(20);}
    // test(const test& t) : a(t.a), b(t.b) {cout << "copy" << endl;}
    /*
    test& operator=(const test& t)
    {
        *c = *(t.c);
        a = t.a;
        b = t.b;
        cout << "assignment" << endl;
    }
    */
};

#include <iostream>
using namespace std;

class Base
{
    public:
        virtual void test(int a)//有virtual关键字，参数列表不同 
        {
            cout<<"this is base there are different parameters with virtual"<<endl;
        }
        
        void test1() 
        {
            cout<<"this is base with the same parameters with not virtual"<<endl;
        }
        
         virtual void test2() 
        {
            cout<<"this is base with the same parameters with virtual"<<endl;
        }
};

class Ship:public Base
{
    public:
        void test()
        {
            cout<<"this is Ship there are different parameters with virtual cover"<<endl;
        }
        
        void test1() 
        {
            cout<<"this is Ship with the same parameters with not virtual cover"<<endl;
        }
        
        void test2() 
        {
            cout<<"this is Ship with the same parameters with virtual cover"<<endl;
        }
};

class Fun
{
    int a;
    int b;
    int c;
public:
    Fun() : a(0), b(0), c(0) {}
    Fun(Fun& rhs)
    {
        cout << "test" << endl;
    }
    Fun(Fun&& rhs)
    {
        cout << "right" << endl;
    }
    void operator=(Fun&& rhs)
    {
        cout << "right=" << endl;
    }
};


int main()
{
    Fun a;
    Fun b;
    swap(a, b);
    return 0;
}