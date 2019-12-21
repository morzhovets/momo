//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// template <class... Args> reference emplace_back(Args&&... args);

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

class A
{
    int i_;
    double d_;

    A(const A&);
    A& operator=(const A&);

public:
    A(int i, double d)
        : i_(i), d_(d) {}

    A(A&& a) = default;
    //    : i_(a.i_),
    //      d_(a.d_)
    //{
    //    a.i_ = 0;
    //    a.d_ = 0;
    //}

    A& operator=(A&& a) = default;
    //{
    //    i_ = a.i_;
    //    d_ = a.d_;
    //    a.i_ = 0;
    //    a.d_ = 0;
    //    return *this;
    //}

    int geti() const {return i_;}
    double getd() const {return d_;}
};

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

void main()
{
#ifndef _LIBCPP_HAS_NO_VARIADICS
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        vector<A> c;
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        //assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        //assert(is_contiguous_container_asan_correct(c));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<A, stack_allocator<A, 4> > c;
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        //assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        //assert(is_contiguous_container_asan_correct(c));
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<A, min_allocator<A>> c;
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        //assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        //assert(is_contiguous_container_asan_correct(c));
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
#endif	// _LIBCPP_HAS_NO_VARIADICS
}
