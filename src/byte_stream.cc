#include "byte_stream.hh"

#include <algorithm>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  if ( flag_ == 1 ) {
    return true;
  }
  return false;
}

void Writer::push( string data )
{
  uint64_t len = min( data.size(), available_capacity() );
  if ( len == 0 ) {
    return;
  } else if ( data.size() > len ) {
    data.resize( len );
  }
  buffer_data.push( move( data ) ); // move(data) 可带来性能提升 -> 避免了string的拷贝
  bytes_pushed_ += len;
  if ( buffer_data.size() == 1 ) {
    buffer_view = buffer_data.front();
  }
  return;
}

void Writer::close()
{
  flag_ = 1;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - reader().bytes_buffered();
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  return writer().is_closed() && bytes_buffered() == 0;
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

string_view Reader::peek() const
{
  return buffer_view;
}

void Reader::pop( uint64_t len )
{
  if ( len > bytes_buffered() ) {
    return;
  }
  bytes_popped_ += len;

  while ( len > 0 ) {
    if ( len >= buffer_view.size() ) {
      len -= buffer_view.size();
      buffer_data.pop();
      buffer_view = buffer_data.front(); // 最开始就保证了 buffer_data 不为空
    } else {
      buffer_view.remove_prefix( len );
      len = 0;
    }
  }
  return;
}

uint64_t Reader::bytes_buffered() const
{
  return writer().bytes_pushed() - bytes_popped();
}
