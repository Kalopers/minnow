#include "reassembler.hh"

#include <algorithm>
#include <ranges>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( data.empty() ) { // error judgement
    if ( is_last_substring ) {
      output_.writer().close();
    }
    return;
  }

  auto end_index = first_index + data.size(); // Caculate the index of the last bute of the substring
  auto last_available_index
    = next_index_
      + output_.writer()
          .available_capacity(); // Caculate the index of the last byte that can be written to the output stream

  if ( end_index < next_index_ || first_index >= last_available_index ) { // overflow windows judgement
    return;
  }

  if ( end_index > last_available_index ) { // resize data
    data.resize( last_available_index - first_index );
    end_index = last_available_index;
    is_last_substring = false; // Because part of it is truncated, the current substring must not be the end.
  }
  if ( first_index < next_index_ ) { // resize data
    data = data.substr( next_index_ - first_index );
    first_index = next_index_;
  }

  if ( first_index == next_index_
       && ( buffer_.empty()
            || end_index < get<1>( buffer_.front() ) + 2 ) ) { // whether the data can be push into ByteStream
    if ( buffer_.size() ) {                                    // 若重叠, 则调整data的范围
      data.resize( min( end_index, get<0>( buffer_.front() ) ) - first_index );
    }
    push_to_output( move( data ) );
  } else {
    buffer_push( first_index, end_index - 1, data );
  }
  // buffer_push( first_index, end_index - 1, data );
  flag_ |= is_last_substring;

  buffer_pop(); // try to pop the data
}

uint64_t Reassembler::bytes_pending() const
{
  return buffer_size_;
}

void Reassembler::push_to_output( string data )
{
  next_index_ += data.size();
  output_.writer().push( move( data ) );
}

void Reassembler::buffer_pop()
{
  while ( !buffer_.empty() && get<0>( buffer_.front() ) == next_index_ ) {
    buffer_size_ -= get<2>( buffer_.front() ).size();
    push_to_output( move( get<2>( buffer_.front() ) ) );
    buffer_.pop_front();
  }

  // close the output
  if ( flag_ && buffer_.empty() ) {
    output_.writer().close();
  }
}

// insert-interval
void Reassembler::buffer_push( uint64_t first_index, uint64_t last_index, std::string data )
{
  // 合并区间
  auto l = first_index, r = last_index;
  auto beg = buffer_.begin(), end = buffer_.end();
  auto lef = lower_bound( beg, end, l, []( auto& a, auto& b ) { return get<1>( a ) < b; } );
  auto rig = upper_bound( lef, end, r, []( auto& b, auto& a ) { return get<0>( a ) > b; } );
  if ( lef != end )
    l = min( l, get<0>( *lef ) );
  if ( rig != beg )
    r = max( r, get<1>( *prev( rig ) ) );

  // 当data已在buffer_中时，直接返回
  if ( lef != end && get<0>( *lef ) == l && get<1>( *lef ) == r ) {
    return;
  }

  buffer_size_ += 1 + r - l;
  if ( data.size() == r - l + 1 && lef == rig ) { // 当buffer_中没有data重叠的部分
    buffer_.emplace( rig, l, r, move( data ) );
    return;
  }
  string s( 1 + r - l, 0 );

  for ( auto&& it : views::iota( lef, rig ) ) {
    auto& [a, b, c] = *it;
    buffer_size_ -= c.size();
    ranges::copy( c, s.begin() + a - l );
  }
  ranges::copy( data, s.begin() + first_index - l );
  buffer_.emplace( buffer_.erase( lef, rig ), l, r, move( s ) );
}
