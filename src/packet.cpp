#include "packet.h"

Packet::Packet() {
}

Packet::Packet(long clock, Packet::Type type) : clock(clock), type(type) {
}

Packet::~Packet() {
}

long Packet::getClock() {
  return clock;
}

Packet::Type Packet::getType() {
  return type;
}
