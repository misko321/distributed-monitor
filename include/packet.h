#ifndef DM_INCLUDE_PACKET_H_
#define DM_INCLUDE_PACKET_H_

// #define SIZEOF_PACKET sizeof(Packet)

class Packet {
public:
  enum Type { DM_REQUEST = 1000, DM_REPLY, DM_CONDVAR_WAIT, DM_CONDVAR_NOTIFY,
    DM_RECV_CONFIRM, DM_CONDVAR_RECV_CONFIRM };
  Packet();
  Packet(long clock, Packet::Type type, unsigned int resourceId);
  ~Packet();

  long getClock();
  Type getType();
  unsigned int getResourceId();
private:
  long clock;
  Type type;
  unsigned int resourceId;
};

#endif // DM_INCLUDE_PACKET_H_
