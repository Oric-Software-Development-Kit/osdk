#ifndef SHIFTER_COLOR_H
#define SHIFTER_COLOR_H

#include "getpixel.h"

class ShifterColor
{
public:
	constexpr ShifterColor() : m_color(0) {}

	constexpr ShifterColor(int r,int g,int b)
		: m_color( (unsigned short)( (((((r>>5)+((r>>1)&8))&15)<<8) | ((((g>>5)+((g>>1)&8))&15)<<4) | (((b>>5)+((b>>1)&8))&15)) ) )
	{}

	constexpr ShifterColor(RgbColor rgb)
		: m_color( (unsigned short)( (ValueToSte((unsigned char)((rgb.m_red*15)/255))<<8) | (ValueToSte((unsigned char)((rgb.m_green*15)/255))<<4) | (ValueToSte((unsigned char)((rgb.m_blue*15)/255))<<0) ) )
	{}

	constexpr unsigned short GetValue()  const { return m_color; }

	constexpr unsigned short GetBigEndianValue() const { return (unsigned short)(((m_color&255)<<8) | ((m_color>>8)&255)); }

	constexpr RgbColor GetRgb() const
	{
		return RgbColor(
			(unsigned char)((255*ValueFromSte((m_color>>8)&15))/15),
			(unsigned char)((255*ValueFromSte((m_color>>4)&15))/15),
			(unsigned char)((255*ValueFromSte((m_color>>0)&15))/15)
		);
	}

	static constexpr unsigned char ValueToSte(unsigned char value)   { return ((value&15)>>1) | ((value&1)<<3); }
	static constexpr unsigned char ValueFromSte(unsigned char value) { return ((value&7)<<1) | ((value&8)>>3); }

	constexpr int ComputeDifference(const ShifterColor& otherColor) const
	{
		int dr=ValueFromSte((m_color>>8)&15)-ValueFromSte((otherColor.m_color>>8)&15);
		int dg=ValueFromSte((m_color>>4)&15)-ValueFromSte((otherColor.m_color>>4)&15);
		int db=ValueFromSte((m_color>>0)&15)-ValueFromSte((otherColor.m_color>>0)&15);
		if (dr<0) dr=-dr;
		if (dg<0) dg=-dg;
		if (db<0) db=-db;
		return (dr*dr)+(dg*dg)+(db*db);
	}

	bool operator< ( const ShifterColor& rhs ) const { return m_color < rhs.m_color; }
	bool operator== ( const ShifterColor& rhs ) const { return m_color == rhs.m_color; }

private:
	unsigned short	m_color;
};


#endif
