/*
 * strlist.cpp
 */


#include <cstdio>
#include <stdlib.h>

#include "strlist.h"


//die function
static void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

//default constructor

StrList::StrList()
{
    //list = new struct List;
    initList((List*) &list);
}

//destructor

StrList::~StrList()
{
    while (!isEmptyList((List*)&list)){
        this->popFront();
    } 
}

//copy constructor

StrList::StrList(const StrList& s)
{
    //list = new struct List;
    initList((List*)&list);
   struct List currList;
   for (currList = s.list;
            !(isEmptyList(&currList)); currList.head = currList.head->next){
         this->addFront(*((MyString*) currList.head->data));
   }
   
    this->reverse();   
    
}

//assignment operator

StrList& StrList::operator=(const StrList& rhs)
{

    if (this == &rhs){
        return *this;
    }

    while (!isEmptyList((List*)&list)){
        this->popFront();
    } 
   struct List currList;
   for (currList = rhs.list;
            !(isEmptyList(&currList)); currList.head = currList.head->next){
         this->addFront(*((MyString*) currList.head->data));
   }
 
   this->reverse();
   return *this;
}


//returns size of the StrList
int StrList::size() const
{
   int counter = 0;
   struct List currList;
   for (currList = this->list;
            !(isEmptyList(&currList)); currList.head = currList.head->next){
     counter++;    
   } 
   return counter;
}


//addFront
void StrList::addFront(const MyString& str){

    //create a new MyString object on the heap
    MyString * s1 = new MyString(str);
    
    //add this new object to our list;
    //die if addFront is unable to the c-list cannot allocate memory
    if (::addFront((List*)&list, s1)==NULL){
        die("addFront failed");
    }
}


//popFront
MyString StrList::popFront()
{
    //create temporary MyString object that will be returned
    MyString p = MyString (*((MyString*) this->list.head->data));

    //delete the MyString object that had been allocated on heap
    delete (MyString*)this->list.head->data;

    //pop the node that held the MyString object
    ::popFront((List*)&list);

    //return the temporary MyString object, that has the same value
    //as what had been in the list
    return p;
}


//reverses the StrList
void StrList::reverse()
{
   reverseList((List*)&list);
}


//operator+=
StrList& StrList::operator+=(const StrList &rhs)
{
    this->reverse();

    struct List currList;
    for (currList = rhs.list;
            !(isEmptyList(&currList)); currList.head = currList.head->next){
         this->addFront(*((MyString*) currList.head->data));
   }
   
   this->reverse();
   return *this;
}


//operator+
StrList StrList::operator+(const StrList& rhs)
{
    StrList temp(*this);
    temp += rhs;
    return temp;
}



//put-to operator

ostream& operator<<(ostream& os, const StrList& s)
{
    os << "{ "; 
    struct List currList;
    for (currList = s.list;
            !(isEmptyList(&currList)); currList.head = currList.head->next){
        os<< *((MyString*)currList.head->data) << " ";
    
   }
      os << "}";
      return os;
}

//operator[]
MyString& StrList::operator[](int i)
{
    int counter = 0;
    struct Node currNode = *(this->list.head);
    while (counter<i){
        currNode = *(currNode.next);
        counter++;
    }
    return *(MyString*)currNode.data;
}

//operator[] const
const MyString& StrList::operator[](int i) const
{
    return ((StrList&)*this)[i];
}
