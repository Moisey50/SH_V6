#pragma once
#include "ColorChannelNodesList.h"

#define DELIMITER_KEY_VALUE    ('=')
#define DELIMITER_SUB_PROGRAMS ('&')
#define MSG_KEY                ("FullPrgr")

struct Program
{
  ColorChannelNodesList  m_mdVideo;
  ColorChannelNodesList  m_mdSnapshot;

public:
  Program()
    : m_mdVideo("video")
    , m_mdSnapshot("snapshot")
  {
    //RESET_INSTANCE
  }
  ~Program()
  {
    Empty();
  }

  bool IsValid(char* pErr)
  {
    bool res = false;
    char errVideo[24] = { 0 };
    char errSnapshot[24] = { 0 };

    bool isValidVideo = m_mdVideo.IsValid(errVideo);
    bool isValidSnaps = m_mdSnapshot.IsValid(errSnapshot);

    res = isValidVideo && isValidSnaps;

    if (!res && pErr)
    {
      if (!pErr[0])
      {
        if (errVideo[0])
          sprintf(pErr, "Video Subprogram %s", errVideo);

        if (errSnapshot[0])
        {
          char* subProgrName_Snapshot = "Snapshot Subprogram";
          sprintf(pErr, "%s%s%s %s"
            , !pErr[0] ? "" : pErr
            , !pErr[0] ? "" : " and "
            , subProgrName_Snapshot
            , errSnapshot);
        }
      }
    }

    return res;
  }

  Program& Empty()
  {
    m_mdVideo.Empty();
    m_mdSnapshot.Empty();

    return *this;
  }

  Program& Deserialize(const string& src)
  {
    int nOffset = 0;
    int dlmtrIndx = (int)src.find_first_of(DELIMITER_KEY_VALUE, nOffset);

    string key;
    string value;

    if (dlmtrIndx >= 0)
    {
      key = src.substr(0, dlmtrIndx);
      if (key.compare(MSG_KEY) == 0)
      {
        value = src.substr(dlmtrIndx + 1);
        if (!value.empty())
        {
          dlmtrIndx = (int)value.find_first_of(DELIMITER_SUB_PROGRAMS, nOffset);
          if (dlmtrIndx >= 0)
          {
            Empty();
            m_mdVideo.Deserialize(value.substr(0, dlmtrIndx));
            m_mdSnapshot.Deserialize(value.substr(dlmtrIndx + 1));
          }
        }
      }
    }
    return *this;
  }
};

