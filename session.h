#ifndef Session_h
#define Session_h

class Session {
  public:
  Session();

  bool changed;

  double temperature;
  double tempeTarget;

  // Rotary push button causes on/off state change
  bool on;

  long int encoder;

  // Rotary encoder clockwise
  void cw();

  // Rotary encoder counter-clockwise
  void ccw();

};

#endif
