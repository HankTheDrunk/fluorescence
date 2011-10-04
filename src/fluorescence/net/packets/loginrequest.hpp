#ifndef FLUO_NET_PACKETS_LOGINREQUEST_HPP
#define FLUO_NET_PACKETS_LOGINREQUEST_HPP

#include <net/packet.hpp>

#include <misc/string.hpp>

namespace fluo {
namespace net {
namespace packets {

class LoginRequest : public Packet {
public:
    LoginRequest(const UnicodeString& name, const UnicodeString& password);

    virtual bool write(int8_t* buf, unsigned int len, unsigned int& index) const;

private:
    UnicodeString name_;
    UnicodeString password_;
};

}
}
}

#endif