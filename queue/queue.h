/// @file
/// @brief
/// @copyright Copyright (c) InfoTeCS. All Rights Reserved.

#pragma once

#include <set>
#include <deque>
#include <initializer_list>
#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>


namespace {
namespace debug {

template< typename T, typename Iter, typename Compare >
bool isUnique( Iter begin, Iter end, Compare&& compare )
{
     std::set< T, Compare > uniq( compare );
     std::copy( begin, end, std::inserter( uniq, uniq.end() ) );
     return uniq.size() == std::distance( begin, end );
}

} // namespace debug
} // namespace {unnamed}



namespace mt {


template< typename T >
class Queue : boost::noncopyable {
public:
     Queue() : Queue( 0 ) {}

     explicit Queue( std::size_t maxLen ) : maxQueueLen_( maxLen ) {}


     std::size_t maxLength() const
     {
          return maxQueueLen_;
     }


     std::size_t size() const
     {
          return queue_.size();
     }


     void push( T value )
     {
          boost::unique_lock< boost::mutex > lock( mutex_ );
          pushCond_.wait( lock,
               [ this ](){ return isQueueReadyToPush( maxQueueLen_, queue_.size(), 1 ); } );

          queue_.push_back( std::move( value ) );
          popCond_.notify_one();

          BOOST_ASSERT( ( maxQueueLen_ == 0 ) || ( queue_.size() <= maxQueueLen_ ) );
     }


     template< typename CompareFunc >
     void pushUnique( T value, CompareFunc&& compare )
     {
          boost::unique_lock< boost::mutex > lock( mutex_ );
          pushCond_.wait( lock,
               [ this ](){ return isQueueReadyToPush( maxQueueLen_, queue_.size(), 1 ); } );

          if( queue_.end() == std::find_if( std::begin( queue_ ), std::end( queue_ ),
               [ & ]( const T& each ){ return compare( each, value ); } ) )
          {
               queue_.push_back( std::move( value ) );
               popCond_.notify_one();
          }
          BOOST_ASSERT( debug::isUnique< T >( std::begin( queue_ ), std::end( queue_ ), compare ) );
          BOOST_ASSERT( ( maxQueueLen_ == 0 ) || ( queue_.size() <= maxQueueLen_ ) );
     }


     template< typename Iter >
     void push( Iter begin, Iter end )
     {
          boost::unique_lock< boost::mutex > lock( mutex_ );

          const auto nElements = std::distance( begin, end );

          pushCond_.wait( lock,
               [ this, nElements ](){ return isQueueReadyToPush( maxQueueLen_, queue_.size(), nElements ); } );

          queue_.insert( queue_.end(), begin, end );
          popCond_.notify_all();

          BOOST_ASSERT( ( maxQueueLen_ == 0 ) || ( queue_.size() <= maxQueueLen_ ) );
     }


     void push( std::initializer_list< T > ilist )
     {
          push( std::begin( ilist ), std::end( ilist ) );
     }


     boost::optional< T > pop()
     {
          boost::unique_lock< boost::mutex > lock( mutex_ );
          popCond_.wait( lock, [ this ](){ return !queue_.empty(); } );
          pushCond_.notify_one();

          const auto val = boost::make_optional( queue_.front() );
          queue_.pop_front();
          return val;
     }

private:
     static
     bool
     isQueueReadyToPush( std::size_t maxQueueLen, std::size_t currentQueueLen, std::size_t nElements )
     {
          if( maxQueueLen )
          {
               return ( maxQueueLen - currentQueueLen ) >= nElements;
          }

          return currentQueueLen == 0;
     }


     const std::size_t maxQueueLen_;

     mutable boost::mutex mutex_;
     mutable boost::condition_variable pushCond_;
     mutable boost::condition_variable popCond_;
     std::deque< T > queue_;
};


} // namespace mt
