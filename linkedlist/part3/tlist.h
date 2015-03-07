/*
 * tlist.h
 */

#ifndef __TLIST_H__
#define __TList_H__

using namespace std;

//using deque instead of list
#include <deque>

//included so that 'reverse' function can be called on the deque
#include <algorithm>

#include <iostream>

template <typename T>
class TList;
template <typename T>
ostream& operator<<(ostream& os, const TList<T>& rhs);

template <typename T>
class TList
{
    public:
        
        
        int size() const { return d.size(); }

        void addFront(const T& t) { d.push_front(t); }

        T popFront();
        
        void reverse(); 
    
        TList& operator+=(const TList& rhs);

        TList operator+(const TList& rhs);

        T& operator[](int i);

        const T& operator[](int i) const;

        int isEmpty() const { return d.empty(); } 
        
        friend ostream& operator<< <T>(ostream& os, const TList<T>& rhs);
        
    private:

        deque<T> d;

};


template <typename T>
T TList<T>::popFront()
{
    T t = d.front();
    d.pop_front();
    return t;
}

//call stdlib's reverse function
template <typename T>
void TList<T>::reverse()
{
    ::reverse(d.begin(), d.end());
}


template <typename T>
TList<T>& TList<T>::operator+=(const TList<T>& rhs)
{
    typename deque<T>::const_iterator i;
    for (i = rhs.d.begin(); i != rhs.d.end(); ++i){
        d.push_back(*i);
     }

    return *this;
}

template <typename T>
TList<T> TList<T>::operator+ (const TList<T>& rhs)
{
    TList<T> temp(*this);
    temp += rhs;
    return temp;
}

//runs in constant time
template <typename T>
T& TList<T>::operator[] (int i)
{
    return d[i];
}

//also runs in constant time
template <typename T>
const T& TList<T>::operator[] (int i) const
{
    return ((TList&)*this)[i];
}

template <typename T>
ostream& operator<<(ostream& os, const TList<T>& rhs)
{
    os << "{ ";
    typename deque<T>::const_iterator i;
    for (i = rhs.d.begin(); i != rhs.d.end(); ++i){
        os << *i << " ";
    }

    os << "}";
    return os;
}

#endif
