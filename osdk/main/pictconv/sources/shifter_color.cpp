

#include "shifter_color.h"

// ============================================================================
//
//                    Compile-time verification of ShifterColor
//
// Replaces former UnitTest++ runtime checks from colortest.cpp
//
// ============================================================================

// ValueToSte
static_assert(ShifterColor::ValueToSte( 0)== 0, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 1)== 8, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 2)== 1, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 3)== 9, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 4)== 2, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 5)==10, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 6)== 3, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 7)==11, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 8)== 4, "ValueToSte");
static_assert(ShifterColor::ValueToSte( 9)==12, "ValueToSte");
static_assert(ShifterColor::ValueToSte(10)== 5, "ValueToSte");
static_assert(ShifterColor::ValueToSte(11)==13, "ValueToSte");
static_assert(ShifterColor::ValueToSte(12)== 6, "ValueToSte");
static_assert(ShifterColor::ValueToSte(13)==14, "ValueToSte");
static_assert(ShifterColor::ValueToSte(14)== 7, "ValueToSte");
static_assert(ShifterColor::ValueToSte(15)==15, "ValueToSte");

// ValueFromSte
static_assert(ShifterColor::ValueFromSte( 0)== 0, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 8)== 1, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 1)== 2, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 9)== 3, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 2)== 4, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte(10)== 5, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 3)== 6, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte(11)== 7, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 4)== 8, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte(12)== 9, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 5)==10, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte(13)==11, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 6)==12, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte(14)==13, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte( 7)==14, "ValueFromSte");
static_assert(ShifterColor::ValueFromSte(15)==15, "ValueFromSte");

// GetBigEndianValue
static_assert(ShifterColor(RgbColor(  0,  1,  0)).GetBigEndianValue()==0x0001, "BigEndian black");
static_assert(ShifterColor(RgbColor(255,  0,  0)).GetBigEndianValue()==0x000F, "BigEndian red");
static_assert(ShifterColor(RgbColor(  0,255,  0)).GetBigEndianValue()==0xF000, "BigEndian green");
static_assert(ShifterColor(RgbColor(  0,  0,255)).GetBigEndianValue()==0x0F00, "BigEndian blue");
static_assert(ShifterColor(RgbColor(255,255,255)).GetBigEndianValue()==0xFF0F, "BigEndian white");

// RGB round-trip
static_assert(RgbColor(  0,  0,  0)==ShifterColor(RgbColor(  0,  0,  0)).GetRgb(), "RGB round-trip black");
static_assert(RgbColor(255,  0,  0)==ShifterColor(RgbColor(255,  0,  0)).GetRgb(), "RGB round-trip red");
static_assert(RgbColor(  0,255,  0)==ShifterColor(RgbColor(  0,255,  0)).GetRgb(), "RGB round-trip green");
static_assert(RgbColor(  0,  0,255)==ShifterColor(RgbColor(  0,  0,255)).GetRgb(), "RGB round-trip blue");
static_assert(RgbColor(255,255,255)==ShifterColor(RgbColor(255,255,255)).GetRgb(), "RGB round-trip white");

// ComputeDifference
static_assert(ShifterColor(RgbColor(  0,  0,  0)).ComputeDifference(ShifterColor(RgbColor(  0,  0,  0)))==  0, "Diff black-black");
static_assert(ShifterColor(RgbColor(255,  0,  0)).ComputeDifference(ShifterColor(RgbColor(255,  0,  0)))==  0, "Diff red-red");
static_assert(ShifterColor(RgbColor(255,255,255)).ComputeDifference(ShifterColor(RgbColor(255,255,255)))==  0, "Diff white-white");
static_assert(ShifterColor(RgbColor(128,  0,  0)).ComputeDifference(ShifterColor(RgbColor(  0,  0,  0)))== 49, "Diff grey-black");
static_assert(ShifterColor(RgbColor(255,255,255)).ComputeDifference(ShifterColor(RgbColor(  0,  0,  0)))==675, "Diff white-black");

// RgbColor operators
static_assert(RgbColor(  0,  0,  0)==RgbColor(  0,  0,  0), "RgbColor == black");
static_assert(RgbColor(255,  0,  0)==RgbColor(255,  0,  0), "RgbColor == red");
static_assert(RgbColor(  0,255,  0)==RgbColor(  0,255,  0), "RgbColor == green");
static_assert(RgbColor(  0,  0,255)==RgbColor(  0,  0,255), "RgbColor == blue");
static_assert(RgbColor(255,255,255)==RgbColor(255,255,255), "RgbColor == white");
static_assert(RgbColor(  0,  0,  0)!=RgbColor(  1,  0,  0), "RgbColor != red diff");
static_assert(RgbColor(255,  0,  0)!=RgbColor( 23,  0,  0), "RgbColor != red diff 2");
static_assert(RgbColor( 48,  0,  0)!=RgbColor( 48,  1,  0), "RgbColor != green diff");
