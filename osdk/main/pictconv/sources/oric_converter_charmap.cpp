

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "FreeImage.h"

#include "defines.h"
#include "getpixel.h"
#include "hires.h"
#include "oric_converter.h"
#include "dithering.h"

#include "common.h"

#include "image.h"

#include <cstdint>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <numeric>

#ifndef _WIN32
#define _int64 int64_t
#endif

void dither_riemersma_monochrom(ImageContainer& image,int width,int height);


void OricPictureConverter::convert_charmap(const ImageContainer& sourcePicture)
{
  ImageContainer convertedPicture(sourcePicture);

  //
  // Perform the global dithering, if required
  //
  switch (m_dither)
  {
  case DITHER_RIEMERSMA:
    dither_riemersma_monochrom(convertedPicture,m_Buffer.m_buffer_width,m_Buffer.m_buffer_height);
    m_dither=DITHER_NONE;
    break;
  }

  //
  // Perform the conversion to text mode.
  // The method is the following:
  // - The picture is cut in 6x8 blocks
  // - Each block is counted
  // - 6x8=48 bits, so we can use a 64 bit std::map to count the elements
  //
  int charWidth =m_Buffer.m_buffer_width/6;
  int charHeight=m_Buffer.m_buffer_height/8;

  printf("\r\n Size of the picture:%u x %u characters",charWidth,charHeight);

  unsigned int maxCharacters=m_charmap_dual_charset ? 192 : 96;

  //
  // Phase 1: Character extraction
  // Scan the image, build a map of unique 6x8 tiles, assign sequential indices,
  // and store the bitmap data for each unique character.
  //
  // tilemap[charY][charX] stores the global character index for each tile position.
  // charBitmaps[charIndex] stores the 8-byte bitmap for each unique character.
  //
  std::vector<std::vector<unsigned int>> tilemap(charHeight, std::vector<unsigned int>(charWidth));
  std::vector<unsigned char> charBitmaps;  // charIndex*8 bytes

  std::map<uint64_t,unsigned int> characterMap;  // mask -> charIndex

  unsigned int charIndex=0;
  for (int charY=0;charY<charHeight;++charY)
  {
    for (int charX=0;charX<charWidth;++charX)
    {
      unsigned char character[8];
      _int64 val=0;
      for (int line=0;line<8;++line)
      {
        unsigned char hiresByte=0;
        for (int bit=0;bit<6;++bit)
        {
          val<<=1;
          hiresByte<<=1;
          ORIC_COLOR color=convert_pixel_monochrom(convertedPicture,charX*6+bit,charY*8+line);
          if (color!=ORIC_COLOR_BLACK)
          {
            val|=1;
            hiresByte|=1;
          }
        }
        character[line]=hiresByte;
      }
      auto it=characterMap.find(val);
      if (it==characterMap.end())
      {
        characterMap[val]=charIndex;
        for (int line=0;line<8;++line)
          charBitmaps.push_back(character[line]);
        charIndex++;
      }
      tilemap[charY][charX]=characterMap[val];
    }
  }

  unsigned int totalChars=charIndex;
  printf("\r\n Found %u different characters",totalChars);

  if (totalChars>maxCharacters)
  {
    printf("\r\n Warning: %u characters found, only %u exported",totalChars,maxCharacters);
    if (!m_charmap_dual_charset)
      printf(" (use -f5z for dual charset)");
  }

  if (!m_charmap_dual_charset)
  {
    //
    // Single charset mode (original behavior)
    //
    unsigned int actualCharCount=std::min(totalChars,maxCharacters);

    m_Buffer.SetBufferSize(charWidth*6,charHeight);
    unsigned char *ptr_hires=m_Buffer.GetBufferData();

    m_SecondaryBuffer.SetBufferSize(6,actualCharCount*8);
    unsigned char *ptr_redef=m_SecondaryBuffer.GetBufferData();

    // Copy character bitmaps
    for (unsigned int i=0;i<actualCharCount;i++)
    {
      for (int line=0;line<8;++line)
      {
        *ptr_redef++=charBitmaps[i*8+line];
      }
    }

    // Write tilemap
    for (int charY=0;charY<charHeight;++charY)
    {
      for (int charX=0;charX<charWidth;++charX)
      {
        unsigned int idx=tilemap[charY][charX];
        if (idx<96)
          *ptr_hires++=(unsigned char)(32+idx);
        else
          *ptr_hires++=32;
      }
    }

    // Trim the secondary buffer to the actual number of characters stored
    m_SecondaryBuffer.m_buffer_height=actualCharCount*8;
    m_SecondaryBuffer.m_buffer_size=m_SecondaryBuffer.m_buffer_height*m_SecondaryBuffer.m_buffer_cols;
  }
  else
  {
    //
    // Dual charset mode (-f5z)
    // Uses both STD (attribute 8) and ALT (attribute 9) character sets.
    // All tiles on a given row must use the same charset (it's a per-row attribute).
    //
    // Characters appearing on rows assigned to different charsets get duplicated
    // (a copy in each charset). The algorithm assigns rows greedily to minimize
    // the total character count per charset, keeping each within the 96 limit.
    //
    unsigned int usedChars=std::min(totalChars,maxCharacters);

    // Phase 2: Build per-row character sets
    std::vector<std::set<unsigned int>> rowCharSets(charHeight);
    for (int charY=0;charY<charHeight;++charY)
    {
      for (int charX=0;charX<charWidth;++charX)
      {
        unsigned int idx=tilemap[charY][charX];
        if (idx<usedChars)
          rowCharSets[charY].insert(idx);
      }
    }

    // Phase 3: Greedy row assignment
    // For each row, assign to the charset where it adds fewer new characters.
    // Characters shared across charsets are duplicated.
    std::set<unsigned int> stdChars;
    std::set<unsigned int> altChars;
    std::vector<unsigned char> rowAssignment(charHeight,0);  // 0=STD, 1=ALT

    // Process rows with the most unique characters first (hardest to place)
    std::vector<int> rowOrder(charHeight);
    std::iota(rowOrder.begin(),rowOrder.end(),0);
    std::sort(rowOrder.begin(),rowOrder.end(),
      [&rowCharSets](int a, int b) {
        return rowCharSets[a].size()>rowCharSets[b].size();
      });

    for (int ri=0;ri<charHeight;++ri)
    {
      int charY=rowOrder[ri];
      const std::set<unsigned int>& rowSet=rowCharSets[charY];

      // Count how many unique chars each charset would have if this row is assigned to it
      unsigned int stdNewTotal=0;
      unsigned int altNewTotal=0;
      {
        std::set<unsigned int> stdCandidate=stdChars;
        stdCandidate.insert(rowSet.begin(),rowSet.end());
        stdNewTotal=(unsigned int)stdCandidate.size();
      }
      {
        std::set<unsigned int> altCandidate=altChars;
        altCandidate.insert(rowSet.begin(),rowSet.end());
        altNewTotal=(unsigned int)altCandidate.size();
      }

      bool canStd=(stdNewTotal<=96);
      bool canAlt=(altNewTotal<=96);

      if (canStd && canAlt)
      {
        // Assign to whichever adds fewer new characters
        if (stdNewTotal<=altNewTotal)
        {
          rowAssignment[charY]=0;
          stdChars.insert(rowSet.begin(),rowSet.end());
        }
        else
        {
          rowAssignment[charY]=1;
          altChars.insert(rowSet.begin(),rowSet.end());
        }
      }
      else
      if (canStd)
      {
        rowAssignment[charY]=0;
        stdChars.insert(rowSet.begin(),rowSet.end());
      }
      else
      if (canAlt)
      {
        rowAssignment[charY]=1;
        altChars.insert(rowSet.begin(),rowSet.end());
      }
      else
      {
        // Neither charset can fit this row - find which is closer and force it
        printf("\r\n Warning: Row %d has %u chars, STD would need %u/96 and ALT %u/96",
          charY,(unsigned int)rowSet.size(),stdNewTotal,altNewTotal);
        // Force into whichever is less full
        if (stdNewTotal<=altNewTotal)
        {
          rowAssignment[charY]=0;
          stdChars.insert(rowSet.begin(),rowSet.end());
        }
        else
        {
          rowAssignment[charY]=1;
          altChars.insert(rowSet.begin(),rowSet.end());
        }
      }
    }

    unsigned int stdCount=(unsigned int)stdChars.size();
    unsigned int altCount=(unsigned int)altChars.size();

    // Count shared characters (duplicated in both charsets)
    unsigned int sharedCount=0;
    for (auto c : stdChars)
    {
      if (altChars.count(c))
        sharedCount++;
    }

    m_charmap_std_count=stdCount;
    m_charmap_alt_count=altCount;

    printf("\r\n Dual charset: %u STD + %u ALT characters (%u shared, %u unique total)",
      stdCount,altCount,sharedCount,stdCount+altCount-sharedCount);

    if (stdCount>96 || altCount>96)
    {
      printf("\r\n Error: Charset overflow (STD=%u, ALT=%u, max 96 each)",stdCount,altCount);
    }

    // Build local index maps for each charset
    // STD: chars in stdChars get local indices 0..stdCount-1
    // ALT: chars in altChars get local indices 0..altCount-1
    std::map<unsigned int,unsigned int> stdLocalIndex;
    std::map<unsigned int,unsigned int> altLocalIndex;
    {
      unsigned int idx=0;
      for (auto c : stdChars) stdLocalIndex[c]=idx++;
    }
    {
      unsigned int idx=0;
      for (auto c : altChars) altLocalIndex[c]=idx++;
    }

    // Phase 4: Generate output buffers

    // Attribute buffer (tertiary): one byte per tile row, 8=STD, 9=ALT
    m_TertiaryBuffer.SetBufferSize(1,charHeight);
    unsigned char *ptr_attr=m_TertiaryBuffer.GetBufferData();
    for (int charY=0;charY<charHeight;++charY)
    {
      ptr_attr[charY]=rowAssignment[charY] ? 9 : 8;
    }

    // Tilemap buffer (primary): remap to 32+localIndex
    m_Buffer.SetBufferSize(charWidth*6,charHeight);
    unsigned char *ptr_hires=m_Buffer.GetBufferData();

    for (int charY=0;charY<charHeight;++charY)
    {
      for (int charX=0;charX<charWidth;++charX)
      {
        unsigned int idx=tilemap[charY][charX];
        if (idx<usedChars)
        {
          if (rowAssignment[charY]==0)
            *ptr_hires++=(unsigned char)(32+stdLocalIndex[idx]);
          else
            *ptr_hires++=(unsigned char)(32+altLocalIndex[idx]);
        }
        else
        {
          *ptr_hires++=32;
        }
      }
    }

    // Charset data (secondary): STD chars first, then ALT chars
    unsigned int totalCharsetBytes=(stdCount+altCount)*8;
    m_SecondaryBuffer.SetBufferSize(6,totalCharsetBytes);
    unsigned char *ptr_redef=m_SecondaryBuffer.GetBufferData();

    // Write STD characters in local index order
    for (auto c : stdChars)
    {
      for (int line=0;line<8;++line)
        *ptr_redef++=charBitmaps[c*8+line];
    }
    // Write ALT characters in local index order
    for (auto c : altChars)
    {
      for (int line=0;line<8;++line)
        *ptr_redef++=charBitmaps[c*8+line];
    }

    // Trim secondary buffer to actual size
    m_SecondaryBuffer.m_buffer_height=totalCharsetBytes;
    m_SecondaryBuffer.m_buffer_size=m_SecondaryBuffer.m_buffer_height*m_SecondaryBuffer.m_buffer_cols;
  }

  return;
}


