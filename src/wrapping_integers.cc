#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  /*  Inputs:
   *    n: Absolute sequence number
   *    zero_point: Initial Sequence Number
   *  Output:
   *    seqno: The (relative) sequence number 
  */
  return zero_point + n; // 可以不考虑溢出的问题，因为会自动截断，进而从0重新开始，效果相当于 (zero_point + n) % pow(2, 32)
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  static constexpr uint64_t TWO31 = 1UL << 31;
  static constexpr uint64_t TWO32 = 1UL << 32;

  auto const ckpt32 = wrap( checkpoint, zero_point );
  uint64_t dis = raw_value_ - ckpt32.raw_value_;
  
   if (dis <= TWO31) { // 右边的更近
     return checkpoint + dis;
   } // dis > TWO31
   dis = TWO32 - dis;  // 左边的更近 // dis' = TWO32 - dis = TWO31 - (dis - TWO31) < TWO31
   if (checkpoint < dis) { // ckpt不够左移，左边界处理
     return checkpoint + TWO32 - dis;  
   }
   return checkpoint - dis;
}
