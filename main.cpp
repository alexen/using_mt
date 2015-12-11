/// @file
/// @brief
/// @copyright Copyright (c) InfoTeCS. All Rights Reserved.

#include <iostream>
#include <numeric>
#include <stdexcept>
#include <tuple>

#include <boost/cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/errinfo_api_function.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include "queue/queue.h"
#include "printer.h"


namespace mt {


struct Person {
     enum class Type {
          Regular,
          Admin,
          CEO
     };

     boost::uuids::uuid id;
     Type type;

     friend std::ostream& operator<<( std::ostream& os, const mt::Person& p )
     {
          static const char* const typeAsStr[] = { "REG", "ADM", "CEO" };
          os << "person " << p.id << " is " << typeAsStr[ static_cast< int >( p.type ) ];
          return os;
     }
};


bool compareByIds( const Person& p1, const Person& p2 )
{
     return p1.id == p2.id;
}


bool compareByType( const Person& p1, const Person& p2 )
{
     return p1.type == p2.type;
}


Person generateRandomePerson()
{
     Person p;

     p.id = boost::uuids::random_generator()();
     p.type = Person::Type( std::accumulate( p.id.begin(), p.id.end(), 0 ) % 3 );

     return p;
}


Person generateSamePerson()
{
     static auto p = generateRandomePerson();
     return p;
}


template< std::size_t N >
void singleItemProducer( unsigned id, mt::Printer< N >& p, mt::Queue< Person >& q )
{
     while( true )
     {
          const auto person = generateRandomePerson();
          p.print( id, boost::format( "produced %1%" ) % person );
          q.push( std::move( person ) );
          boost::this_thread::sleep_for( boost::chrono::milliseconds( 700 ) );
     }
}


template< std::size_t N, std::size_t nElems >
void rangeItemsProducer( unsigned id, mt::Printer< N >& p, mt::Queue< Person >& q )
{
     std::array< Person, nElems > persons;

     while( true )
     {
          std::generate( persons.begin(), persons.end(), mt::generateSamePerson );
          p.print( id, boost::format( "produced %1% persons" ) % persons.size() );
          for( const auto& each: persons )
               q.pushUnique( each, mt::compareByIds );
          p.print( id, boost::format( "produced queue size %1%" ) % q.size() );
          boost::this_thread::sleep_for( boost::chrono::milliseconds( 700 ) );
     }
}


template< std::size_t N >
void consumer( unsigned id, mt::Printer< N >& p, mt::Queue< Person >& q )
{
     while( const auto person = q.pop() )
     {
          p.print( id, boost::format( "processed %1%" ) % *person );
          boost::this_thread::sleep_for( boost::chrono::milliseconds( 1200 ) );
     }
}


} // namespace mt


template< typename T >
std::ostream& operator<<( std::ostream& os, const boost::shared_ptr< T >& ptr )
{
     ptr ? os << *ptr : os << "--";
     return os;
}


int main( /*int argc, char** argv*/ )
{
     try
     {
//          const bool useUnique = argc > 1 ? !strcmp( argv[ 1 ], "-u" ) : false;

          constexpr auto maxQueueLen = 3;

          constexpr auto nRangeProducers = 1;
          constexpr auto nSingleProducers = 0;
          constexpr auto nConsumers = 0;

          constexpr auto nThreads = nConsumers + nRangeProducers + nSingleProducers;

          std::cout << "Test started with parameters:\n"
               << "range values producers: " << nRangeProducers << '\n'
               << "single value producers: " << nSingleProducers << '\n'
               << "consumers: " << nConsumers << '\n'
               << "total threads: " << nThreads << '\n'
               << "max queue length: " << maxQueueLen << '\n';

          mt::Printer< nThreads > printer( std::cout );
          mt::Queue< mt::Person > queue( maxQueueLen );

          int threadId = 0;

          boost::thread_group tg;

          for( auto i = 0; i < nSingleProducers; ++i )
               tg.create_thread(
                    boost::bind(
                         mt::singleItemProducer< nThreads >,
                         ++threadId,
                         boost::ref( printer ),
                         boost::ref( queue )
                    )
               );

          for( auto i = 0; i < nRangeProducers; ++i )
               tg.create_thread(
                    boost::bind(
                         mt::rangeItemsProducer< nThreads, 5 >,
                         ++threadId,
                         boost::ref( printer ),
                         boost::ref( queue )
                    )
               );

          for( auto i = 0; i < nConsumers; ++i )
               tg.create_thread(
                    boost::bind(
                         mt::consumer< nThreads >,
                         ++threadId,
                         boost::ref( printer ),
                         boost::ref( queue )
                    )
               );

          tg.join_all();
     }
     catch( const std::exception& e )
     {
          std::cerr << "exception: " << boost::diagnostic_information( e ) << '\n';
          return 1;
     }

     return 0;
}
