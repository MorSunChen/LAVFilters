
#include "stdafx.h"
#include "LAVCAudio.h"

#include <MMReg.h>

#include "moreuuids.h"

typedef struct {
  const CLSID*        clsMinorType;
  const enum CodecID  nFFCodec;
} FFMPEG_SUBTYPE_MAP;

// Map Media Subtype <> FFMPEG Codec Id
FFMPEG_SUBTYPE_MAP lavc_audio_codecs[] = {
  // AAC
  { &MEDIASUBTYPE_AAC,          CODEC_ID_AAC      },
  { &MEDIASUBTYPE_LATM_AAC,     CODEC_ID_AAC_LATM },
  { &MEDIASUBTYPE_MP4A,         CODEC_ID_AAC      },
  { &MEDIASUBTYPE_mp4a,         CODEC_ID_AAC      },

  // Dolby
  { &MEDIASUBTYPE_DOLBY_AC3,    CODEC_ID_AC3      },
  { &MEDIASUBTYPE_DOLBY_DDPLUS, CODEC_ID_EAC3     },
  { &MEDIASUBTYPE_DOLBY_TRUEHD, CODEC_ID_TRUEHD   },
  { &MEDIASUBTYPE_WAVE_DOLBY_AC3, CODEC_ID_AC3    },

  // DTS
  { &MEDIASUBTYPE_DTS,          CODEC_ID_DTS      },
  { &MEDIASUBTYPE_DTS_HD,       CODEC_ID_DTS      },
  { &MEDIASUBTYPE_WAVE_DTS,     CODEC_ID_DTS      },

  // MPEG Audio
  { &MEDIASUBTYPE_MPEG1Packet,  CODEC_ID_MP1      },
  { &MEDIASUBTYPE_MPEG1Payload, CODEC_ID_MP1      },
  { &MEDIASUBTYPE_MPEG1AudioPayload, CODEC_ID_MP1 },
  { &MEDIASUBTYPE_MPEG2_AUDIO,  CODEC_ID_MP2      },
  { &MEDIASUBTYPE_MP3,          CODEC_ID_MP3      },

  // FLAC
  { &MEDIASUBTYPE_FLAC,         CODEC_ID_FLAC     },
  { &MEDIASUBTYPE_FLAC_FRAMED,  CODEC_ID_FLAC     },

  // Ogg Vorbis
  { &MEDIASUBTYPE_Vorbis2,      CODEC_ID_VORBIS   },
};

// Define Input Media Types
const AMOVIESETUP_MEDIATYPE CLAVCAudio::sudPinTypesIn[] = {
  // AAC
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_AAC          },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_LATM_AAC     },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_MP4A         },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_mp4a         },

  // Dolby
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_DOLBY_AC3    },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_DOLBY_DDPLUS },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_DOLBY_TRUEHD },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_WAVE_DOLBY_AC3 },

  // DTS
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_DTS          },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_WAVE_DTS     },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_DTS_HD       },

  // MPEG Audio
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1Packet  },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1Payload },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1AudioPayload },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG2_AUDIO  },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_MP3          },

  // FLAC
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_FLAC,        },
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_FLAC_FRAMED  },

  // Ogg Vorbis
  { &MEDIATYPE_Audio, &MEDIASUBTYPE_Vorbis2      },
};
const int CLAVCAudio::sudPinTypesInCount = countof(CLAVCAudio::sudPinTypesIn);

// Define Output Media Types
const AMOVIESETUP_MEDIATYPE CLAVCAudio::sudPinTypesOut[] = {
	{ &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM },
};
const int CLAVCAudio::sudPinTypesOutCount = countof(CLAVCAudio::sudPinTypesOut);

// Crawl the lavc_audio_codecs array for the proper codec
CodecID FindCodecId(const CMediaType *mt)
{
  for (int i=0; i<countof(lavc_audio_codecs); ++i) {
    if (mt->subtype == *lavc_audio_codecs[i].clsMinorType) {
      return lavc_audio_codecs[i].nFFCodec;
    }
  }
  return CODEC_ID_NONE;
}

// Default Channel to Speaker Map
scmap_t m_scmap_default[] = {
  //    FL  FR  FC  LFe BL  BR  FLC FRC
  {1, { 0,-1,-1,-1,-1,-1,-1,-1 }, 0},		// Mono			M1, 0
  {2, { 0, 1,-1,-1,-1,-1,-1,-1 }, 0},		// Stereo		FL, FR
  {3, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER},															// 3/0			FL, FR, FC
  {4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY},										// 3/1			FL, FR, FC, Surround
  {5, { 0, 1, 2, 3, 4,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},						// 3/2			FL, FR, FC, BL, BR
  {6, { 0, 1, 2, 3, 4, 5,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/2+LFe		FL, FR, FC, BL, BR, LFe
  {7, { 0, 1, 2, 3, 4, 5, 6,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_CENTER},	// 3/4			FL, FR, FC, BL, Bls, Brs, BR
  {8, { 0, 1, 2, 3, 6, 7, 4, 5 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/4+LFe		FL, FR, FC, BL, Bls, Brs, BR, LFe
};
