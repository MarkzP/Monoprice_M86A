
#include "vianet.h"

#define FRAME_US  300

#define _VUNIT(z) ((z - 1) / 6)
#define _VZONE(z) ((z - 1) % 6)

#define _HB(u) (byte)((u >> 8) & 0xff)
#define _LB(u) (byte)(u & 0xff)

#define _HDR_CMD  0x01
#define _HDR_U1   0x74
#define _HDR_U2   0x75
#define _HDR_U3   0x76
#define _HDR_U4   0x77

const unsigned short vianet_frame[] =
{
  0x6500, 0x5502, 0x0504, 0x3506, 0xa508, 0x950a, 0xc50c, 0xf50e,
  0xd510, 0xe512, 0xb514, 0x8516, 0x1518, 0x251a, 0x751c, 0x451e,
  0x3520, 0x0522, 0x5524, 0x6526, 0xf528, 0xc52a, 0x952c, 0xa52e,
  0x8530, 0xb532, 0xe534, 0xd536, 0x4538, 0x753a, 0x253c, 0x153e,
  0xc540, 0xf542, 0xa544, 0x9546, 0x0548, 0x354a, 0x654c, 0x554e,
  0x7550, 0x4552, 0x1554, 0x2556, 0xb558, 0x855a, 0xd55c, 0xe55e,
  0x9560, 0xa562, 0xf564, 0xc566, 0x5568, 0x656a, 0x356c, 0x056e,
  0x2570, 0x1572, 0x4574, 0x7576, 0xe578, 0xd57a, 0x857c, 0xb57e,
  0x1580, 0x2582, 0x7584, 0x4586, 0xd588, 0xe58a, 0xb58c, 0x858e,
  0xa590, 0x9592, 0xc594, 0xf596, 0x6598, 0x559a, 0x059c, 0x359e,
  0x45a0, 0x75a2, 0x25a4, 0x15a6, 0x85a8, 0xb5aa, 0xe5ac, 0xd5ae,
  0xf5b0, 0xc5b2, 0x95b4, 0xa5b6, 0x35b8, 0x05ba, 0x55bc, 0x65be,
  0xb5c0, 0x85c2, 0xd5c4, 0xe5c6, 0x75c8, 0x45ca, 0x15cc, 0x25ce,
  0x05d0, 0x35d2, 0x65d4, 0x55d6, 0xc5d8, 0xf5da, 0xa5dc, 0x95de,
  0xe5e0, 0xd5e2, 0x85e4, 0xb5e6, 0x25e8, 0x15ea, 0x45ec, 0x75ee,
  0x55f0, 0x65f2, 0x35f4, 0x05f6, 0x95f8, 0xa5fa, 0xf5fc, 0xc5fe
};

const unsigned short vianet_payload[] =
{
  0x9100, 0xa102, 0xf104, 0xc106, 0x5108, 0x610a, 0x310c, 0x010e,
  0x2110, 0x1112, 0x4114, 0x7116, 0xe118, 0xd11a, 0x811c, 0xb11e,
  0xc120, 0xf122, 0xa124, 0x9126, 0x0128, 0x312a, 0x612c, 0x512e,
  0x7130, 0x4132, 0x1134, 0x2136, 0xb138, 0x813a, 0xd13c, 0xe13e,
  0x3140, 0x0142, 0x5144, 0x6146, 0xf148, 0xc14a, 0x914c, 0xa14e,
  0x8150, 0xb152, 0xe154, 0xd156, 0x4158, 0x715a, 0x215c, 0x115e,
  0x6160, 0x5162, 0x0164, 0x3166, 0xa168, 0x916a, 0xc16c, 0xf16e,
  0xd170, 0xe172, 0xb174, 0x8176, 0x1178, 0x217a, 0x717c, 0x417e,
  0xe180, 0xd182, 0x8184, 0xb186, 0x2188, 0x118a, 0x418c, 0x718e,
  0x5190, 0x6192, 0x3194, 0x0196, 0x9198, 0xa19a, 0xf19c, 0xc19e,
  0xb1a0, 0x81a2, 0xd1a4, 0xe1a6, 0x71a8, 0x41aa, 0x11ac, 0x21ae,
  0x01b0, 0x31b2, 0x61b4, 0x51b6, 0xc1b8, 0xf1ba, 0xa1bc, 0x91be,
  0x41c0, 0x71c2, 0x21c4, 0x11c6, 0x81c8, 0xb1ca, 0xe1cc, 0xd1ce,
  0xf1d0, 0xc1d2, 0x91d4, 0xa1d6, 0x31d8, 0x01da, 0x51dc, 0x61de,
  0x11e0, 0x21e2, 0x71e2, 0x41e6, 0xd1e8, 0xe1ea, 0xb1ec, 0x81ee,
  0xa1f0, 0x91f2, 0xc1f4, 0xf1f6, 0x61f8, 0x51fa, 0x01fc, 0x31fe
};

bool M86a_Zone::update(byte* data)
{
  bool changed = false;

  if (data == nullptr) return reset();

  changed |= zoneId != (data[0] & 0x1f);
  zoneId = (data[0] & 0x1f);

  int s1 = (data[1] & 0x07);
  int s2 = (data[2] & 0x03);
  
  int newSource = s2 == 3 ? s1
         : s2 == 2 ? 8
         : s2 == 1 ? 7
         : 0;

  changed |= source != newSource;
  source = newSource;

  changed |= mute != ((data[1] & 0x10) != 0);
  mute = (data[1] & 0x10) != 0;

  changed |= doNotDisturb != ((data[1] & 0x20) != 0);
  doNotDisturb = (data[1] & 0x20) != 0;

  changed |= whm != ((data[2] & 0x40) != 0);
  whm = (data[2] & 0x40) != 0;

  changed |= page != ((data[2] & 0x20) != 0);
  page = (data[2] & 0x20) != 0;

  changed |= loudness != ((data[2] & 0x08) != 0);
  loudness = (data[2] & 0x08) != 0;

  changed |= volume != (data[3] & 0x3f);
  volume = data[3] & 0x3f;

  changed |= treble != (data[4] & 0x1f);
  treble = data[4] & 0x1f;

  changed |= bass != (data[5] & 0x1f);
  bass = data[5] & 0x1f;
  
  return changed;
}

bool M86a_Zone::reset()
{
  bool changed = zoneId != 0;
  zoneId = 0;  
  return changed;
}

void M86a_Zone::print(Print* p)
{
  if (p == nullptr) return;
  
  p->printf("\tZone %02d: source=%d vol=%02d treble=%02d bass=%02d loud=%d mute=%d dnd=%d page=%d whm=%d\r\n", zoneId, source, volume, treble, bass, loudness, mute, doNotDisturb, page, whm); 
}

bool M86a::update(byte* data)
{
  bool changed = false;

  if (data == nullptr || data[0] != 0x05) return reset();

  changed |= unitId != data[1]; unitId = data[1];

  for (int i = 0; i < 6; i++)
  {
    changed |= zone[i].update(data + 2 + (i * 6));
  }

  changed |= senseInput[0] != (data[39] & 0x01); senseInput[0] = (data[39] & 0x01);
  changed |= senseInput[1] != (data[39] & 0x02); senseInput[1] = (data[39] & 0x02);
  changed |= senseInput[2] != (data[39] & 0x04); senseInput[2] = (data[39] & 0x04);
  changed |= senseInput[3] != (data[39] & 0x08); senseInput[3] = (data[39] & 0x08);
  changed |= senseInput[4] != (data[39] & 0x10); senseInput[4] = (data[39] & 0x10);
  changed |= senseInput[5] != (data[39] & 0x20); senseInput[5] = (data[39] & 0x20);

  changed |= audioSource[0] != (data[41] & 0x10); audioSource[0] = (data[41] & 0x10);
  changed |= audioSource[1] != (data[41] & 0x08); audioSource[1] = (data[41] & 0x08);
  changed |= audioSource[2] != (data[41] & 0x20); audioSource[2] = (data[41] & 0x20);
  changed |= audioSource[3] != (data[42] & 0x10); audioSource[3] = (data[42] & 0x10);
  changed |= audioSource[4] != (data[42] & 0x20); audioSource[4] = (data[42] & 0x20);
  changed |= audioSource[5] != (data[42] & 0x40); audioSource[5] = (data[42] & 0x40);
  changed |= audioSource[6] != (data[43] & 0x10); audioSource[6] = (data[43] & 0x10);
  changed |= audioSource[7] != (data[43] & 0x20); audioSource[7] = (data[43] & 0x20);
  
  return changed;
}

bool M86a::reset()
{
  bool changed = unitId != 0;
  unitId = 0;
  for (int i = 0; i < 6; i++) changed |= zone[i].reset();
  return changed;
}

void M86a::print(Print* p)
{
  if (p == nullptr) return;

  if (unitId == 0)
  {
    p->print("(offline)\r\n");
  }
  else
  {
    p->printf("Sense: %d %d %d %d %d %d ", senseInput[0], senseInput[1], senseInput[2], senseInput[3], senseInput[4], senseInput[5]);
    p->printf("Audio: %d %d %d %d %d %d %d %d\r\n", audioSource[0], audioSource[1], audioSource[2], audioSource[3], audioSource[4], audioSource[5], audioSource[6], audioSource[7]);
  
    for (int i = 0; i < 6; i++)
    {
      zone[i].print(p);
    }
  }
}

void Vianet::writeBytes(byte* buf, int len)
{
  if (_port->available())
  {
    if (_debug != nullptr) _debug->printf("[!%d bytes waiting]\r\n", _port->available());
    while (_port->available()) _port->read();
  }
  
  digitalWrite(_te, HIGH);

  _port->write(buf, len);
  _port->flush();

  digitalWrite(_te, LOW);
}

bool Vianet::getNextByte(byte* c, int us)
{
  us /= 10;
  for (int i = 0; i < us; i++)
  {
    yield();
    delayMicroseconds(10);
    if (_port->available())
    {
      *c = _port->read();
      return true;
    }
  }
  return false;
}

bool Vianet::readUnitStatus()
{
  byte c = 0;
  if (!getNextByte(&c, 1000)) return false;
  
  unsigned short preamble = ((unsigned short)c << 8);
  if (!getNextByte(&c)) return false;
  preamble += c;
  bool valid = preamble == 0xfd56;  
  
  byte data[45];
  int len = 0;
  unsigned short rawSymbol = 0;

  while (len < 45)
  {
    if (!getNextByte(&c)) return false;

    if ((c & 0x01) == 0x01)
    {
      if (rawSymbol != 0)
      {
        if (_debug != nullptr) _debug->printf("[!Out of sync at offset %d]\r\n", len);
        valid = false;
        break;
      }
      
      rawSymbol = c << 8;
    }
    else
    {
      rawSymbol += c;
      unsigned short symbol = (rawSymbol & 0xfe) >> 1;
      data[len++] = symbol;
      rawSymbol = 0;
    }
  }

  int unitId = 0;
  if (len == 45) unitId = data[1];
  valid &= unitId >= 1 && unitId <=4;

  if (valid) return _m86a[unitId - 1].update(data);

  if (!valid && _debug != nullptr) _debug->printf("[!_frame=0x%02x unitId=%d preamble=0x%04x len=%d]\r\n", _frame, unitId, preamble, len);
  
  return false;
}

bool Vianet::sendNextFrame()
{
  unsigned long now = micros();
  if ((now - _lastFrameMicros) < FRAME_US) return false;
  _lastFrameMicros = now;
  
  if (++_frame > 127) _frame = 0;
  
  if (_frame == 1)
  {
    byte enc[12] = { 0xed, 0x06, 0x91, 0x00, 0x91, 0x00, 0xa1, 0x02, 0x91, 0x00, 0x91, 0x00 };
    writeBytes(enc, 12);
  }
  else
  {
    unsigned short encodedFrame = vianet_frame[_frame];
    byte enc[2] = { _HB(encodedFrame), _LB(encodedFrame) };
    writeBytes(enc, 2);

    if (_frame == _HDR_U1 || _frame == _HDR_U2 || _frame == _HDR_U3 || _frame == _HDR_U4) return readUnitStatus();
  }

  return false;
}

void Vianet::sendControlCommand(byte cmd[4])
{
  unsigned long now = micros();
  if ((now - _lastFrameMicros) < 100) delayMicroseconds(100 - (now - _lastFrameMicros));
  _lastFrameMicros = micros();
  
  unsigned short p0 = vianet_payload[cmd[0]];
  unsigned short p1 = vianet_payload[cmd[1]];
  unsigned short p2 = vianet_payload[cmd[2]];
  unsigned short p3 = vianet_payload[cmd[3]];

  byte enc[12] = { 0xed, 0x06, 0xd3, 0xfe, _HB(p0), _LB(p0), _HB(p1), _LB(p1), _HB(p2), _LB(p2), _HB(p3), _LB(p3) };

  if (_debug != nullptr) _debug->printf("[0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x%02x]\r\n", enc[0], enc[1], enc[2], enc[3], enc[4], enc[5], enc[6], enc[7], enc[8], enc[9], enc[10], enc[11] );

  writeBytes(enc, 12);
}

void Vianet::initMaster()
{
  byte enc[2] = { 0x55, 0x02 };
  writeBytes(enc, 2);

  delayMicroseconds(1000);  
  do
  {
    writeBytes(enc, 2);
    delayMicroseconds(100);
  } while (_port->available());

  _lastFrameMicros = micros();
  _isMaster = true;
  _frame = 0;  
}

void Vianet::setPowerOff(int zone)
{
  if (zone < 0 || zone > _maxZones) return;

  byte payload[] = { 0x04, (byte)(zone == 0 ? 0x02 : zone), 0x04, (byte)(zone == 0 ? 0x00 : 0x10) };
  sendControlCommand(payload);
}

void Vianet::setSource(int zone, int src)
{
  if (src == 0) setPowerOff(zone);
  else
  {
    if (zone < 1 || zone > _maxZones) return;
    if (src < 1 || src > _maxSources) return;

    bool power = getSource(zone) != 0;

    byte payloadSrc[] = { 0x04, (byte)zone, 0x04, (byte)(0x2f + src) };
    sendControlCommand(payloadSrc);

    if (!power)
    {
      delayMicroseconds(FRAME_US);
      update();
      byte payloadPwr[] = { 0x04, (byte)zone, 0x04, 0x11 };
      sendControlCommand(payloadPwr);
    }
  }
}

void Vianet::setDoNotDisturb(int zone, bool dnd)
{
  if (zone < 0 || zone > _maxZones) return;
  
  byte payload[] = { 0x04, (byte)zone, 0x04, (byte)(dnd ? 0x0b : 0x0a) };
  sendControlCommand(payload);
}

void Vianet::setPage(bool page)
{
  byte payload[] = { 0x04, 0x02, 0x04, (byte)(page ? 0x05 : 0x04) };
  sendControlCommand(payload);
}

void Vianet::setLoudness(int zone, bool loudness)
{
  if (zone < 1 || zone > _maxZones) return;

  byte payload[] = { 0x04, (byte)zone, 0x04, (byte)(loudness ? 0x17 : 0x16) };
  sendControlCommand(payload);
}

void Vianet::setMute(int zone, bool mute)
{
  if (zone < 1 || zone > _maxZones) return;
  
  byte payload[] = { 0x04, (byte)zone, 0x04, (byte)(mute ? 0x14 : 0x13) };
  sendControlCommand(payload);
}

void Vianet::setVolume(int zone, int vol)
{
  if (zone < 1 || zone > _maxZones) return;
  if (vol < 0) vol = 0;
  if (vol > 100) vol = 100;

  byte payload[] = { 0x0b, 0x00, (byte)zone, (byte)vol };
  sendControlCommand(payload);
}

void Vianet::setBass(int zone, int bass)
{
  if (zone < 1 || zone > _maxZones) return;
  if (bass < 58) bass = 58;
  if (bass > 70) bass = 70;

  byte payload[] = { 0x0b, 0x01, (byte)zone, (byte)bass };
  sendControlCommand(payload);
}

void Vianet::setTreble(int zone, int treble)
{
  if (zone < 1 || zone > _maxZones) return;
  if (treble < 58) treble = 58;
  if (treble > 70) treble = 70;

  byte payload[] = { 0x0b, 0x02, (byte)zone, (byte)treble };
  sendControlCommand(payload);
}

bool Vianet::isZoneOnline(int zone)
{
  if (zone < 1 || zone > _maxZones) return false;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].zoneId == zone;
}

int Vianet::getSource(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return 0;
  
  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].source;
}

bool Vianet::getDoNotDisturb(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return false;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].doNotDisturb;
}

bool Vianet::getPage(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return false;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].page;
}


bool Vianet::getLoudness(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return false;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].loudness;
}

bool Vianet::getMute(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return false;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].mute;
}

int Vianet::getVolume(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return 0;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].volume;
}

int Vianet::getBass(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return 0;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].bass;
}

int Vianet::getTreble(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return 0;

  return _m86a[_VUNIT(zone)].zone[_VZONE(zone)].treble;
}

bool Vianet::getSenseInput(int zone)
{
  if (zone < 1 || zone > _maxZones || !isZoneOnline(zone)) return false;

  return _m86a[_VUNIT(zone)].senseInput[_VZONE(zone)];
}

bool Vianet::isAudioSourceDetected(int src)
{
  if (src < 1 || src > _maxSources) return false;
  src--;

  return (_m86a[0].unitId != 0 && _m86a[0].audioSource[src])
      || (_m86a[1].unitId != 0 && _m86a[1].audioSource[src])
      || (_m86a[2].unitId != 0 && _m86a[2].audioSource[src])
      || (_m86a[3].unitId != 0 && _m86a[3].audioSource[src]);
}

void Vianet::begin(HardwareSerial* port, int transmitEnablePin)
{
  _te = transmitEnablePin;
  pinMode(_te, OUTPUT);
  digitalWrite(_te, LOW);
  
  _port = port;
  _port->begin(115200);

  for (int i = 0; i < 4; i++)
  {
    _m86a[i].reset();
  }
}

bool Vianet::update()
{
  unsigned long now = micros();
  
  if (!_isMaster)
  {
    if (_port->available())
    {
      byte c = Serial1.read();
      
      if ((c & 0x01) == 0x01)
      {
        _symbol = (unsigned short)c << 8;
      }
      else
      {
        _lastFrameMicros = now;
        _symbol += c;
    
        if (_symbol == 0x5502)
        {
          if (_debug != nullptr) _debug->print("[initMaster: Hook]\r\n");
          initMaster();
        }
      }
    }

    if ((now - _lastFrameMicros) > 1000000)
    {
      if (_debug != nullptr) _debug->print("[initMaster: Timeout]\r\n");
      initMaster();
    }
  }

  if (_isMaster)
  {
    return sendNextFrame();
  }

  return false;
}

bool Vianet::refresh()
{
  if (!_isMaster) return false;
  
  bool changed = false;
  int frame = _frame;
  
  while (frame == _frame)
  {
    yield();
    changed |= update();
  }

  while (frame != _frame)
  {
    yield();
    changed |= update();
  }

  return changed;
}

void Vianet::print(Print* p)
{
  for (int i = 0; i < 4; i++)
  {
    p->printf("Unit %d: ", i + 1); _m86a[i].print(p);
  }
}
