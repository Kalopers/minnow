#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( !isn_.has_value() ) {
    if ( !message.SYN ) {
      return;
    }
    isn_ = message.seqno;
  }

  if ( message.RST ) {
    flag_ = true;
  }

  auto const checkpoint = writer().bytes_pushed() + 1;
  auto const abs_seqno = message.seqno.unwrap( isn_.value(), checkpoint );

  auto const first_index = message.SYN ? 0 : abs_seqno - 1;
  reassembler_.insert( first_index, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage msg {};
  if ( writer().has_error() || flag_ ){
    msg.RST = true;
    return msg;
  }
  auto const win_sz = writer().available_capacity();
  msg.window_size = win_sz < UINT16_MAX ? win_sz : UINT16_MAX;

  if ( isn_.has_value() ) {
    uint64_t const abs_seqno = writer().bytes_pushed() + 1 + writer().is_closed();
    msg.ackno = Wrap32::wrap(abs_seqno, isn_.value() );
  }
  return msg;
}
