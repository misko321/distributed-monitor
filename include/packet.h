#ifndef DM_INCLUDE_PACKET_H_
#define DM_INCLUDE_PACKET_H_

// #define SIZEOF_PACKET sizeof(Packet)

class Packet {
public:
  enum Type { DM_REQUEST, DM_REPLY };
  Packet();
  Packet(long clock, Type type);
  ~Packet();

  long getClock();
  Type getType();
private:
  long clock;
  Type type;
};

#endif // DM_INCLUDE_PACKET_H_
