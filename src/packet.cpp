#include "packet.h"

Packet::Packet() {
}

Packet::Packet(long clock, Packet::Type type, unsigned int resourceId) : clock(clock),
                                                                type(type),
                                                                resourceId(resourceId) {
}

Packet::~Packet() {
}

long Packet::getClock() {
  return clock;
}

Packet::Type Packet::getType() {
  return type;
}

unsigned int Packet::getResourceId() {
  return resourceId;
}
