
#ifndef GET_PIXEL_H
#define GET_PIXEL_H


class RgbColor
{
public:
  constexpr RgbColor()                                                                      : m_red(0),m_green(0),m_blue(0),m_alpha(255) {}
  constexpr RgbColor(const RgbColor& rgb)                                                   : m_red(rgb.m_red),m_green(rgb.m_green),m_blue(rgb.m_blue),m_alpha(rgb.m_alpha) {}
  constexpr RgbColor(unsigned char red,unsigned char green,unsigned char blue)               : m_red(red),m_green(green),m_blue(blue),m_alpha(255) {}
  constexpr RgbColor(unsigned char red,unsigned char green,unsigned char blue,unsigned char alpha) : m_red(red),m_green(green),m_blue(blue),m_alpha(alpha) {}

  unsigned char GetRGBAverage() const { return (m_red+m_green+m_blue)/3; }

  void SetColor(unsigned char red,unsigned char green,unsigned char blue)  { m_red=red;m_green=green;m_blue=blue;m_alpha=255;}

  bool operator< ( const RgbColor& rhs ) const { return (m_red<rhs.m_red) && (m_green<rhs.m_green) && (m_blue<rhs.m_blue); }

  constexpr bool operator== ( const RgbColor& rhs ) const { return (m_red==rhs.m_red) && (m_green==rhs.m_green) &&  (m_blue==rhs.m_blue) ; }
  constexpr bool operator!= ( const RgbColor& rhs ) const { return !(*this==rhs) ; }

public:
  unsigned char m_red;
  unsigned char m_green;
  unsigned char m_blue;
  unsigned char m_alpha;
};


#endif
