/// @file
/// @brief
/// @copyright Copyright (c) InfoTeCS. All Rights Reserved.

#pragma once

#include <deque>
#include <initializer_list>
#include <boost/optional.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>


namespace mt {


/// Очередь
template< typename T >
class Queue {
public:
     Queue()
          : Queue( 0 ) {}


     explicit Queue( std::size_t maxLen )
          : maxQueueLen_( maxLen ) {}


     std::size_t maxLength() const
     {
          return maxQueueLen_;
     }


     template< typename Iter >
     void push( Iter begin, Iter end )
     {
          pushRange( begin, end );
     }


     void push( std::initializer_list< T > ilist )
     {
          pushRange( std::begin( ilist ), std::end( ilist ) );
     }


     void push( T value )
     {
          pushSingle( std::move( value ) );
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


     template< typename Processor>
     void forEach( Processor&& process ) const
     {
          boost::unique_lock< boost::mutex > lock( mutex_ );
          std::for_each( std::begin( queue_ ), std::end( queue_ ), process );
     }

private:
     static
     bool
     isQueueReadyToPushElements( std::size_t maxQueueLen, std::size_t currentQueueLen, std::size_t nElements )
     {
          if( maxQueueLen )
          {
               return ( maxQueueLen - currentQueueLen ) <= nElements;
          }

          return currentQueueLen == 0;
     }


     void pushSingle( T value )
     {
          boost::unique_lock< boost::mutex > lock( mutex_ );
          pushCond_.wait( lock,
               [ this ]()
               {
                    return isQueueReadyToPushElements( maxQueueLen_, queue_.size(), 1 );
               } );

          queue_.push_back( std::move( value ) );
     }


     template< typename Iter >
     void pushRange( Iter begin, Iter end )
     {
          boost::unique_lock< boost::mutex > lock( mutex_ );

          const auto nElements = std::distance( begin, end );

          pushCond_.wait( lock,
               [ this, nElements ]()
               {
                    return isQueueReadyToPushElements( maxQueueLen_, queue_.size(), nElements );
               } );

          queue_.insert( queue_.end(), begin, end );
     }

     const std::size_t maxQueueLen_;

     mutable boost::mutex mutex_;
     mutable boost::condition_variable pushCond_;
     mutable boost::condition_variable popCond_;
     std::deque< T > queue_;
};


} // namespace mt
