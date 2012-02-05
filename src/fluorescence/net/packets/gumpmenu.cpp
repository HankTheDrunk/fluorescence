/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "gumpmenu.hpp"

#include <misc/log.hpp>

namespace fluo {
namespace net {
namespace packets {

void dumpBuffer(int8_t* buf, unsigned int length) {
    for (unsigned int i = 0; i < length; ++i) {
        if (i % 8 == 0) {
            printf("\n%4u: ", i);
        }
        printf("%x(%c) ", (uint8_t)buf[i], buf[i]);
    }
    printf("\n");
}

GumpMenu::GumpMenu() : Packet(0xB0) {
}

bool GumpMenu::read(const int8_t* buf, unsigned int len, unsigned int& index) {
    bool ret = true;
    
    ret &= PacketReader::read(buf, len, index, playerSerial_);
    ret &= PacketReader::read(buf, len, index, gumpSerial_);
    ret &= PacketReader::read(buf, len, index, locX_);
    ret &= PacketReader::read(buf, len, index, locY_);
    
    uint16_t dataLength = 0;
    ret &= PacketReader::read(buf, len, index, dataLength);
    ret &= PacketReader::readUtf8Fixed(buf, len, index, gumpData_, dataLength);
    
    uint16_t textLineCount;
    ret &= PacketReader::read(buf, len, index, textLineCount);
    for (unsigned int i = 0; i < textLineCount; ++i) {
        ret &= PacketReader::read(buf, len, index, dataLength);
        boost::shared_ptr<UnicodeString> newStr(new UnicodeString());
        ret &= PacketReader::readUnicodeFixed(buf, len, index, *newStr, dataLength);
        textLines_.push_back(newStr);
    }

    return ret;
}

void GumpMenu::onReceive() {
    LOG_INFO << "Gump data: " << std::endl;
    LOG_INFO << gumpData_ << std::endl;
    LOG_INFO << "Additional lines: " << textLines_.size() << std::endl;
    for (unsigned int i = 0; i < textLines_.size(); ++i) {
        LOG_INFO << "\t" << i << ": " << *textLines_[i] << std::endl;
    }
}

}
}
}