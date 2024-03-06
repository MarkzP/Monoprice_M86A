#ifndef __VIANET_H
#define __VIANET_H

// This is a clean room reverse engineering of the communication protocol used between an Elan HC6 controller & 1 to 4 M86a Multi-Zone Amplifiers.
// This protocol is not publicly documented anywhere; this is an effort to implement the minimal commands required to match the Monoprice API features.
// Note that I could only get this to work reliably with Teensy boards, thanks to hardware control of the Transmit Enable pin.

#include <Arduino.h>
#include <HardwareSerial.h>

class M86a_Zone
{
  public:    
    int zoneId = 0;               // 1 to 24
    int source = 0;               // 1 to 8
    bool doNotDisturb = false;
    bool mute = false;
    bool whm = false;
    bool page = false;
    bool loudness = false;
    int volume = 48;              // 48=min, 0=max
    int bass = 16;                // 4=min, 16=mid, 28=max
    int treble = 16;              // 4=min, 16=mid, 28=max

    bool update(byte* data);
    bool reset();
    void print(Print* p);
};

class M86a
{
  public:
    int unitId;
    M86a_Zone zone[6];
    bool senseInput[6];
    bool audioSource[8];

    bool update(byte* data);
    bool reset();
    void print(Print* p);
};

class Vianet
{
  public:
    void setPowerOff(int zone);
    void setSource(int zone, int src);
    void setDoNotDisturb(int zone, bool dnd);
    void setPage(bool page);
    void setLoudness(int zone, bool loudness);
    void setMute(int zone, bool mute);
    void setVolume(int zone, int vol);
    void setBass(int zone, int bss);
    void setTreble(int zone, int trbl);

    bool isZoneOnline(int zone);
    int getSource(int zone);
    bool getPage(int zone);
    bool getDoNotDisturb(int zone);
    bool getLoudness(int zone);
    bool getMute(int zone);
    int getVolume(int zone);
    int getBass(int zone);
    int getTreble(int zone);

    bool getSenseInput(int zone);
    bool isAudioSourceDetected(int src);
   
    void begin(HardwareSerial* port, uint16_t transmitEnablePin);
    bool update();
    bool refresh();
    void print(Print* p);

    void setDebug(Print* p) { _debug = p; }    
    
  private:
    const int _maxZones = 24;
    const int _maxSources = 8;

    bool getNextByte(byte* c, int us = 500);
    bool readUnitStatus();   
    bool sendNextFrame();
    void sendControlCommand(byte cmd[4]);
    void initMaster();
    

    unsigned long _lastFrameMicros;
    bool _isMaster = false;
    uint16_t _symbol = 0;
    int _frame = 0;
    HardwareSerial* _port;
    Print* _debug = nullptr;
    M86a _m86a[4];
};

#endif
