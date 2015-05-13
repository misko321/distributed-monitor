#include "packet.h"

Packet::Packet() {
}

Packet::Packet(long clock, Packet::Type type, int resourceId) : clock(clock),
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

// unsigned int Packet::getSourceRank() {
//   return sourceRank;
// }
//
// unsigned int Packet::getDestinationRank() {
//   return destinationRank;
// }
