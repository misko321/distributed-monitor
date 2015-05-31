#ifndef DM_INCLUDE_PACKET_H_
#define DM_INCLUDE_PACKET_H_

// #define SIZEOF_PACKET sizeof(Packet)

class Packet {
public:
  enum Type { DM_REQUEST, DM_REPLY, DM_CONDVAR_WAIT, DM_CONDVAR_NOTIFY, DM_RECV_CONFIRM };
  Packet();
  Packet(long clock, Packet::Type type, int resourceId);
  ~Packet();

  long getClock();
  Type getType();
  unsigned int getResourceId();
  // unsigned int getSourceRank();
  // unsigned int getDestinationRank();
private:
  long clock;
  Type type;
  unsigned int resourceId;
  // unsigned int srcRank;
  // unsigned int destRank;
};

#endif // DM_INCLUDE_PACKET_H_
