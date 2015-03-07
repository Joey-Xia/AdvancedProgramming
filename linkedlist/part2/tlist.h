
/*
 * tlist.h
 */

#ifndef __TLIST_H__
#define __TLIST_H__

using namespace std;

#include <list>
#include <iostream>

template <typename T>
class TList;
template <typename T>
ostream& operator<<(ostream& os, const TList<T>& rhs);

template <typename T>
class TList
{
    public:
        
        
        int size() const { return l.size(); }

        void addFront(const T& t) { l.push_front(t); }

        T popFront();
        
        void reverse() { l.reverse(); };

        TList& operator+=(const TList& rhs);

        TList operator+(const TList& rhs);

        T& operator[](int i);

        const T& operator[](int i) const;

        int isEmpty() const { return l.empty(); } 
        
        friend ostream& operator<< <T>(ostream& os, const TList<T>& rhs);
        
    private:

        list<T> l;

};


//popFront() function
template <typename T>
T TList<T>::popFront()
{
    T t = l.front();
    l.pop_front();
    return t;
}

//operator +=
template <typename T>
TList<T>& TList<T>::operator+=(const TList<T>& rhs)
{
    typename list<T>::const_iterator i;
    for (i = rhs.l.begin(); i != rhs.l.end(); ++i){
        l.push_back(*i);
     }

    return *this;
}

//operator +
template <typename T>
TList<T> TList<T>::operator+ (const TList<T>& rhs)
{
    TList<T> temp(*this);
    temp += rhs;
    return temp;
}

//operator[]
template <typename T>
T& TList<T>::operator[] (int i)
{
    typename list<T>::iterator it;
    int  counter=0;
    for (it=l.begin(); counter < i; ++it){
        counter++;
     }
     return *it;
}

//operator[] const
template <typename T>
const T& TList<T>::operator[] (int i) const
{
    return ((TList&)*this)[i];
}


//put-to operator
template <typename T>
ostream& operator<<(ostream& os, const TList<T>& rhs)
{
    os << "{ ";
    typename list<T>::const_iterator i;
    for (i = rhs.l.begin(); i != rhs.l.end(); ++i){
        os << *i << " ";
    }

    os << "}";
    return os;
}

#endif
