/// @file
/// @brief
/// @copyright Copyright (c) InfoTeCS. All Rights Reserved.

#include <iostream>
#include <stdexcept>
#include <tuple>

#include <boost/cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/errinfo_api_function.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

#include "queue/queue.h"
#include "printer.h"


namespace mt {


template< std::size_t N >
void singleItemProducer( unsigned id, Printer< N >& p, Queue< int >& q, int value )
{
     int n = 10;
     while( true )
     {
          q.push( value );
          p.print( id, boost::format( "single value %1% placed" ) % value );

          if( n-- == 0 )
          {
               boost::this_thread::sleep_for( boost::chrono::milliseconds( 600 ) );
               n = 10;
          }
     }
}


template< std::size_t N >
void rangeItemsProducer( unsigned id, Printer< N >& p, Queue< int >& q )
{
     const int chunkLen = 10;
     int n = 0;

     std::array< int, chunkLen > chunk;

     while( n < std::numeric_limits< int >::max() )
     {
          for( int i = 0; i < chunk.size(); ++i )
               chunk[ i ] = ++n;

          q.push( std::begin( chunk ), std::end( chunk ) );
          p.print( id, boost::format( "range of %1% items placed" ) % chunkLen );
     }
}


template< std::size_t N >
void consumer( unsigned id, Printer< N >& p, Queue< int >& q )
{
     while( const auto val = q.pop() )
     {
          p.print( id, boost::format( "processing number %1%" ) % *val );
          boost::this_thread::sleep_for( boost::chrono::milliseconds( 100 ) );
     }
}


} // namespace mt


int main()
{
     try
     {
          constexpr auto maxQueueLen = 5;

          constexpr auto nRangeProducers = 1;
          constexpr auto nSingleProducers = 3;
          constexpr auto nConsumers = 5;

          constexpr auto nThreads = nConsumers + nRangeProducers + nSingleProducers;

          std::cout << "Test started with parameters:\n"
               << "range values producers: " << nRangeProducers << '\n'
               << "single value producers: " << nSingleProducers << '\n'
               << "consumers: " << nConsumers << '\n'
               << "total threads: " << nThreads << '\n'
               << "max queue length: " << maxQueueLen << '\n';

          mt::Printer< nThreads > printer( std::cout );
          mt::Queue< int > queue( maxQueueLen );

          boost::thread_group tg;

          int threadId = 0;

          for( auto i = 0; i < nRangeProducers; ++i )
               tg.create_thread( boost::bind( mt::rangeItemsProducer< nThreads >, ++threadId, boost::ref( printer ), boost::ref( queue ) ) );

          constexpr std::array< int, 3 > values { 555, 777, 999 };

          for( auto i = 0; i < nSingleProducers; ++i )
               tg.create_thread( boost::bind( mt::singleItemProducer< nThreads >, ++threadId, boost::ref( printer ), boost::ref( queue ), values[ i % values.size() ] ) );

          for( auto i = 0; i < nConsumers; ++i )
               tg.create_thread( boost::bind( mt::consumer< nThreads >, ++threadId, boost::ref( printer ), boost::ref( queue ) ) );

          tg.join_all();
     }
     catch( const std::exception& e )
     {
          std::cerr << "exception: " << boost::diagnostic_information( e ) << '\n';
          return 1;
     }

     return 0;
}
