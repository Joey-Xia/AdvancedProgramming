/* 
 * strlist.h
 */

#ifndef __STRLIST_H__
#define __STRLIST_H__

/*
   Copy the mystring.h/cpp files from lab9/solutions directory.
 */

#include "mystring.h"

/*
   Note that extern "C" is required when you are linking with an
   object file or a library compiled in C from C++.

   Use the usual -I, -L, -l flags in your Makefile to link with
   libmylist.a in your lab3/solutions directory.  Do NOT copy over
   any of the old linked list files into lab10 directory.
 */

extern "C" {
#include "mylist.h"
}

class StrList {

    public:

        // TODO: The basic 4.
        /*
           Don't worry about efficiency in this assignment.  Do what's
           the easiest.  For example, in order to append elements from
           one List to another using the C linked list API, you can
           reverse the target list, add elements using addFront, and
           then reverse it again when you're done.

           In fact, you'll have to implement many member functions
           rather inefficiency due to the deficiency of the old list
           API.
         */

        //default constructor
        StrList();

        //destructor
        ~StrList();
        
        //copy constructor
        StrList(const StrList& s);

        //assignment operator
        StrList& operator=(const StrList& rhs);

        //returns size of the StrList
        int size() const;

        //addFront
        void addFront(const MyString&str);

        //popFront
        MyString popFront();

        //reverses the StrList
        void reverse();

        //operator+=
        StrList& operator+=(const StrList &s);

        //operator+
        StrList operator+(const StrList &rhs);

        //put-to operator
        friend ostream& operator<<(ostream& os, const StrList& s);

        //operator[]
        MyString& operator[](int i);

        //operator[] const
        const MyString& operator[](int i) const;


        // isEmpty() function
        /*
           I'm giving away this function to show you that you'll have
           to cast away the const-ness of the data member when
           necessary.
         */
        int isEmpty() const { return isEmptyList((List *)&list); }


        private:

        // This class contains the old C list structure as its single
        // data member.  Do NOT add any data member.

        struct List list;
};

#endif
