#ifndef FLOPPY_H_
#define FLOPPY_H_

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iostream>


enum CompressionMode
{
  e_CompressionNone,
  e_CompressionFilepack
};

// This class is meant to be mapped on memory area supposed to be of MFM disk format.
// The header is 256 bytes long, so this class memory usage should be similar in size and structure.
class FloppyHeader
{
public:
  FloppyHeader();
  ~FloppyHeader();

  void Clear();

  bool IsValidHeader(bool allowImpossibleFloppies = false) const;

  void SetSignature(const char signature[8]);

  void SetSideNumber(int sideNumber);
  int GetSideNumber() const;

  void SetTrackNumber(int trackNumber);
  int GetTrackNumber() const;

  void SetGeometry(int geometry);
  int GetGeometry() const;

  int FindNumberOfSectors(int& firstSectorOffset,int& sectorInterleave,std::vector<int>& sectorOrder) const;    ///< Note: This function will expect valid data to exist past the padding because it needs to scan the data...

private:
  char          m_Signature[8];   // (MFM_DISK)
  unsigned char m_Sides[4];       // :   4 bytes (2)
  unsigned char m_Tracks[4];      // :   4 bytes (42/$2A)
  unsigned char m_Geometry[4];    // :   4 bytes (1)
  unsigned char m_Padding[236];   // : 236 bytes (000000...00000 )
};


class FileEntry
{
public:
  FileEntry();
  ~FileEntry();

  int GetSector() const
  {
    if (m_CompressionMode==e_CompressionFilepack)
    {
      return m_StartSector+128;
    }
    else
    {
      return m_StartSector;
    }
  }

public:
  int             m_FloppyNumber;     // 0 for a single floppy program
  int             m_StartSide;        // 0 or 1
  int             m_StartTrack;       // 0 to 42 (80...)
  int             m_StartSector;      // 1 to 17 (or 16 or 18...)
  int             m_SectorCount;
  int             m_FinalFileSize;
  int             m_StoredFileSize;
  int             m_DiskOffset;       // Were on the disk is this file located (for external tools)
  CompressionMode m_CompressionMode;
  std::string     m_FilePath;
  std::map<std::string,std::string> m_Metadata;
};


class Floppy
{
public:

public:
  Floppy();
  ~Floppy();

  bool CreateDisk(int numberOfSides,int numberOfTracks,int numberOfSectors,int interleave);
  bool LoadDisk(const char* fileName);
  bool SaveDisk(const char* fileName) const;
  bool SaveDescription(const char* fileName) const;
  std::string OutputFloppyStats() const;

  bool WriteSector(const char *fileName);
  bool WriteLoader(const char *fileName,int loadAddress);
  bool WriteFile(const char *fileName,bool removeHeaderIfPresent,const std::map<std::string,std::string>& metadata);
  bool WriteTapeFile(const char *fileName);

  bool ReserveSectors(int sectorCount,int fillValue,const std::map<std::string,std::string>& metadata);

  bool ExtractFile(const char *fileName,int trackNumber,int sectorNumber,int sectorCount);

  bool AddDefine(std::string defineName,std::string defineValue);
  bool AddDefine(std::string defineName,int defineValue);

  void SetCompressionMode(CompressionMode compressionMode)
  {
    m_CompressionMode=compressionMode;
  }

  void AllowMissingFiles(bool allowMissingFiles)
  {
    m_AllowMissingFiles=allowMissingFiles;
  }

  bool GetAllowImpossibleFloppies() const
  {
    return m_AllowMissingFiles;
  }

  void AllowImpossibleFloppies(bool allowImpossibleFloppies)
  {
    m_AllowImpossibleFloppies=allowImpossibleFloppies;
  }

  unsigned int SetPosition(int track,int sector)
  {
    m_CurrentSector=sector;
    m_CurrentTrack =track;
    return GetDskImageOffset();
  }

  bool MoveToNextSector()
  {
    m_CurrentSector++;

    if (m_CurrentSector==m_SectorCount+1) // We reached the end of the track!
    {
      m_CurrentSector=1;
      m_CurrentTrack++;
      if (m_CurrentTrack==m_TrackCount)
      {
        // Next side is following on the floppy in the DSK format, so technically we should have nothing to do
        // All the hard work is in the loader
      }
      if (m_CurrentTrack==(m_TrackCount*2))
      {
        // We have reached the end of the floppy...
        return false;
      }
    }
    return true;
  }

  void MarkCurrentSectorUsed();

private:
  unsigned int GetDskImageOffset();

private:
  void*       m_Buffer;
  size_t      m_BufferSize;
  int         m_TrackCount;           // 42
  int         m_SectorCount;          // 17
  int         m_SideCount;            // 2
  int         m_SectorInterleave;     // 1 by default (6 is optimal with 17 sectors per track)
  int         m_OffsetFirstSector;    // 156 (Location of the first byte of data of the first sector)
  int         m_InterSectorSpacing;   // 358 (Number of bytes to skip to go to the next sector: 256+59+43)
  std::vector<int> m_SectorOffset;

  int         m_LoaderTrackPosition;
  int         m_LoaderSectorPosition;
  int         m_LoaderLoadAddress;

  int         m_CurrentTrack;
  int         m_CurrentSector;

  std::set<int>       m_SectorUsageMap;

  CompressionMode     m_CompressionMode;
  bool                m_AllFilesAreResolved;
  bool                m_AllowMissingFiles;
  bool                m_AllowImpossibleFloppies; 

  std::vector<FileEntry>                            m_FileEntries;
  std::vector<std::pair<std::string,std::string>>   m_DefineList;

  int                     m_LastFileWithMetadata;
  std::set<std::string>                             m_MetadataCategories;
};

#endif
