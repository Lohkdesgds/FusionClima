#include "protocol.h"

using namespace LoRaC;

bool send_pack(protocol& pk)
{
    if (!LR.has_begun()) return false;
    pk.device = lora_mac_this;
    return LR.send((char*)&pk, sizeof(pk));
}

bool convert_pack(const std::vector<char> dat, protocol_extra& pk)
{
    pack pak = LR.pop();
    if (!pak || pak.ptr_len != sizeof(pk.dat)) return false;
    
    memcpy((void*)&pk.dat, pak.ptr.get(), sizeof(pk.dat));

    if (pk.dat.device == lora_mac_this) return false; // heard itself?
    pk.signal_strength = LR.currentRssi();
    pk.snr = LR.currentSnr();
    return true;
}