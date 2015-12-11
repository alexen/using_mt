/// @file
/// @brief
/// @copyright Copyright (c) InfoTeCS. All Rights Reserved.

#pragma once

#include <cstdint>
#include <bitset>
#include <ostream>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>


namespace mt {


template< std::size_t N >
class Printer {
public:
     explicit Printer( std::ostream& os ) : os_( os ) {}

     void print( unsigned id, std::string&& s ) const;
     void print( unsigned id, const boost::format& fmt ) const;

private:
     mutable boost::mutex mutex_;
     std::ostream& os_;
};



template< std::size_t N >
void Printer< N >::print( unsigned id, std::string&& s ) const
{
     boost::unique_lock< boost::mutex > lock( mutex_ );
     os_ << "[" << std::bitset< N >( 1 << id ) << "]: " << s << std::endl;
}

template< std::size_t N >
void Printer< N >::print( unsigned id, const boost::format& fmt ) const
{
     print( id, fmt.str() );
}


} // namespace mt
