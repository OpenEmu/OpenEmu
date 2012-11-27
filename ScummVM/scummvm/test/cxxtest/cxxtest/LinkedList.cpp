#ifndef __cxxtest__LinkedList_cpp__
#define __cxxtest__LinkedList_cpp__

#include <cxxtest/LinkedList.h>

namespace CxxTest
{
    List GlobalFixture::_list = { 0, 0 };
    List RealSuiteDescription::_suites = { 0, 0 };

    void List::initialize()
    {
        _head = _tail = 0;
    }

    Link *List::head()
    {
        Link *l = _head;
        while ( l && !l->active() )
            l = l->next();
        return l;
    }

    const Link *List::head() const
    {
        Link *l = _head;
        while ( l && !l->active() )
            l = l->next();
        return l;
    }

    Link *List::tail()
    {
        Link *l = _tail;
        while ( l && !l->active() )
            l = l->prev();
        return l;
    }

    const Link *List::tail() const
    {
        Link *l = _tail;
        while ( l && !l->active() )
            l = l->prev();
        return l;
    }

    bool List::empty() const
    {
        return (_head == 0);
    }

    unsigned List::size() const
    {
        unsigned count = 0;
        for ( const Link *l = head(); l != 0; l = l->next() )
            ++ count;
        return count;
    }

    Link *List::nth( unsigned n )
    {
        Link *l = head();
        while ( n -- )
            l = l->next();
        return l;
    }

    void List::activateAll()
    {
        for ( Link *l = _head; l != 0; l = l->justNext() )
            l->setActive( true );
    }

    void List::leaveOnly( const Link &link )
    {
        for ( Link *l = head(); l != 0; l = l->next() )
            if ( l != &link )
                l->setActive( false );
    }

    Link::Link() :
        _next( 0 ),
        _prev( 0 ),
        _active( true )
    {
    }

    Link::~Link()
    {
    }

    bool Link::active() const
    {
        return _active;
    }

    void Link::setActive( bool value )
    {
        _active = value;
    }

    Link * Link::justNext()
    {
        return _next;
    }

    Link * Link::justPrev()
    {
        return _prev;
    }

    Link * Link::next()
    {
        Link *l = _next;
        while ( l && !l->_active )
            l = l->_next;
        return l;
    }

    Link * Link::prev()
    {
        Link *l = _prev;
        while ( l && !l->_active )
            l = l->_prev;
        return l;
    }

    const Link * Link::next() const
    {
        Link *l = _next;
        while ( l && !l->_active )
            l = l->_next;
        return l;
    }

    const Link * Link::prev() const
    {
        Link *l = _prev;
        while ( l && !l->_active )
            l = l->_prev;
        return l;
    }

    void Link::attach( List &l )
    {
        if ( l._tail )
            l._tail->_next = this;

        _prev = l._tail;
        _next = 0;

        if ( l._head == 0 )
            l._head = this;
        l._tail = this;
    }

    void Link::detach( List &l )
    {
        if ( _prev )
            _prev->_next = _next;
        else
            l._head = _next;

        if ( _next )
            _next->_prev = _prev;
        else
            l._tail = _prev;
    }
}

#endif // __cxxtest__LinkedList_cpp__
