// ***************************************************************** -*- C++ -*-
/* Spec:
 * https://learn.microsoft.com/fr-fr/windows/win32/directshow/avi-riff-file-reference
 * https://cdn.hackaday.io/files/274271173436768/avi.pdf
 * http://abcavi.kibi.ru/docs/odml1.pdf
 */
// *****************************************************************************

// included header files
#include "riffvideo.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "helper_functions.hpp"

namespace Exiv2::Internal {

const std::map<std::string, std::string> infoTags = {{"AGES", "Xmp.video.Rated"},
                                                     {"CMNT", "Xmp.video.Comment"},
                                                     {"CODE", "Xmp.video.EncodedBy"},
                                                     {"COMM", "Xmp.video.Comment"},
                                                     {"DIRC", "Xmp.video.Director"},
                                                     {"DISP", "Xmp.audio.SchemeTitle"},
                                                     {"DTIM", "Xmp.video.DateTimeOriginal"},
                                                     {"GENR", "Xmp.video.Genre"},
                                                     {"IARL", "Xmp.video.ArchivalLocation"},
                                                     {"IART", "Xmp.video.Artist"},
                                                     {"IAS1", "Xmp.video.Edit1"},
                                                     {"IAS2", "Xmp.video.Edit2"},
                                                     {"IAS3", "Xmp.video.Edit3"},
                                                     {"IAS4", "Xmp.video.Edit4"},
                                                     {"IAS5", "Xmp.video.Edit5"},
                                                     {"IAS6", "Xmp.video.Edit6"},
                                                     {"IAS7", "Xmp.video.Edit7"},
                                                     {"IAS8", "Xmp.video.Edit8"},
                                                     {"IAS9", "Xmp.video.Edit9"},
                                                     {"IBSU", "Xmp.video.BaseURL"},
                                                     {"ICAS", "Xmp.audio.DefaultStream"},
                                                     {"ICDS", "Xmp.video.CostumeDesigner"},
                                                     {"ICMS", "Xmp.video.Commissioned"},
                                                     {"ICMT", "Xmp.video.Comment"},
                                                     {"ICNM", "Xmp.video.Cinematographer"},
                                                     {"ICNT", "Xmp.video.Country"},
                                                     {"ICOP", "Xmp.video.Copyright"},
                                                     {"ICRD", "Xmp.video.DateTimeDigitized"},
                                                     {"ICRP", "Xmp.video.Cropped"},
                                                     {"IDIM", "Xmp.video.Dimensions"},
                                                     {"IDPI", "Xmp.video.DotsPerInch"},
                                                     {"IDST", "Xmp.video.DistributedBy"},
                                                     {"IEDT", "Xmp.video.EditedBy"},
                                                     {"IENC", "Xmp.video.EncodedBy"},
                                                     {"IENG", "Xmp.video.Engineer"},
                                                     {"IGNR", "Xmp.video.Genre"},
                                                     {"IKEY", "Xmp.video.PerformerKeywords"},
                                                     {"ILGT", "Xmp.video.Lightness"},
                                                     {"ILGU", "Xmp.video.LogoURL"},
                                                     {"ILIU", "Xmp.video.LogoIconURL"},
                                                     {"ILNG", "Xmp.video.Language"},
                                                     {"IMBI", "Xmp.video.InfoBannerImage"},
                                                     {"IMBU", "Xmp.video.InfoBannerURL"},
                                                     {"IMED", "Xmp.video.Medium"},
                                                     {"IMIT", "Xmp.video.InfoText"},
                                                     {"IMIU", "Xmp.video.InfoURL"},
                                                     {"IMUS", "Xmp.video.MusicBy"},
                                                     {"INAM", "Xmp.video.Title"},
                                                     {"IPDS", "Xmp.video.ProductionDesigner"},
                                                     {"IPLT", "Xmp.video.NumOfColors"},
                                                     {"IPRD", "Xmp.video.Product"},
                                                     {"IPRO", "Xmp.video.ProducedBy"},
                                                     {"IRIP", "Xmp.video.RippedBy"},
                                                     {"IRTD", "Xmp.video.Rating"},
                                                     {"ISBJ", "Xmp.video.Subject"},
                                                     {"ISFT", "Xmp.video.Software"},
                                                     {"ISGN", "Xmp.video.SecondaryGenre"},
                                                     {"ISHP", "Xmp.video.Sharpness"},
                                                     {"ISRC", "Xmp.video.Source"},
                                                     {"ISRF", "Xmp.video.SourceForm"},
                                                     {"ISTD", "Xmp.video.ProductionStudio"},
                                                     {"ISTR", "Xmp.video.Starring"},
                                                     {"ITCH", "Xmp.video.Technician"},
                                                     {"IWMU", "Xmp.video.WatermarkURL"},
                                                     {"IWRI", "Xmp.video.WrittenBy"},
                                                     {"LANG", "Xmp.video.Language"},
                                                     {"LOCA", "Xmp.video.LocationInfo"},
                                                     {"PRT1", "Xmp.video.Part"},
                                                     {"PRT2", "Xmp.video.NumOfParts"},
                                                     {"RATE", "Xmp.video.Rate"},
                                                     {"STAR", "Xmp.video.Starring"},
                                                     {"STAT", "Xmp.video.Statistics"},
                                                     {"TAPE", "Xmp.video.TapeName"},
                                                     {"TCDO", "Xmp.video.EndTimecode"},
                                                     {"TCOD", "Xmp.video.StartTimecode"},
                                                     {"TITL", "Xmp.video.Title"},
                                                     {"TLEN", "Xmp.video.Length"},
                                                     {"TORG", "Xmp.video.Organization"},
                                                     {"TRCK", "Xmp.video.TrackNumber"},
                                                     {"TURL", "Xmp.video.URL"},
                                                     {"TVER", "Xmp.video.SoftwareVersion"},
                                                     {"VMAJ", "Xmp.video.VegasVersionMajor"},
                                                     {"VMIN", "Xmp.video.VegasVersionMinor"},
                                                     {"YEAR", "Xmp.video.Year"}};

const std::map<uint16_t, std::string> audioEncodingValues = {
    {0x1, "Microsoft PCM"},
    {0x2, "Microsoft ADPCM"},
    {0x3, "Microsoft IEEE float"},
    {0x4, "Compaq VSELP"},
    {0x5, "IBM CVSD"},
    {0x6, "Microsoft a-Law"},
    {0x7, "Microsoft u-Law"},
    {0x8, "Microsoft DTS"},
    {0x9, "DRM"},
    {0xa, "WMA 9 Speech"},
    {0xb, "Microsoft Windows Media RT Voice"},
    {0x10, "OKI-ADPCM"},
    {0x11, "Intel IMA/DVI-ADPCM"},
    {0x12, "Videologic Mediaspace ADPCM"},
    {0x13, "Sierra ADPCM"},
    {0x14, "Antex G.723 ADPCM"},
    {0x15, "DSP Solutions DIGISTD"},
    {0x16, "DSP Solutions DIGIFIX"},
    {0x17, "Dialoic OKI ADPCM"},
    {0x18, "Media Vision ADPCM"},
    {0x19, "HP CU"},
    {0x1a, "HP Dynamic Voice"},
    {0x20, "Yamaha ADPCM"},
    {0x21, "SONARC Speech Compression"},
    {0x22, "DSP Group True Speech"},
    {0x23, "Echo Speech Corp."},
    {0x24, "Virtual Music Audiofile AF36"},
    {0x25, "Audio Processing Tech."},
    {0x26, "Virtual Music Audiofile AF10"},
    {0x27, "Aculab Prosody 1612"},
    {0x28, "Merging Tech. LRC"},
    {0x30, "Dolby AC2"},
    {0x31, "Microsoft GSM610"},
    {0x32, "MSN Audio"},
    {0x33, "Antex ADPCME"},
    {0x34, "Control Resources VQLPC"},
    {0x35, "DSP Solutions DIGIREAL"},
    {0x36, "DSP Solutions DIGIADPCM"},
    {0x37, "Control Resources CR10"},
    {0x38, "Natural MicroSystems VBX ADPCM"},
    {0x39, "Crystal Semiconductor IMA ADPCM"},
    {0x3a, "Echo Speech ECHOSC3"},
    {0x3b, "Rockwell ADPCM"},
    {0x3c, "Rockwell DIGITALK"},
    {0x3d, "Xebec Multimedia"},
    {0x40, "Antex G.721 ADPCM"},
    {0x41, "Antex G.728 CELP"},
    {0x42, "Microsoft MSG723"},
    {0x43, "IBM AVC ADPCM"},
    {0x45, "ITU-T G.726"},
    {0x50, "Microsoft MPEG"},
    {0x51, "RT23 or PAC"},
    {0x52, "InSoft RT24"},
    {0x53, "InSoft PAC"},
    {0x55, "MP3"},
    {0x59, "Cirrus"},
    {0x60, "Cirrus Logic"},
    {0x61, "ESS Tech. PCM"},
    {0x62, "Voxware Inc."},
    {0x63, "Canopus ATRAC"},
    {0x64, "APICOM G.726 ADPCM"},
    {0x65, "APICOM G.722 ADPCM"},
    {0x66, "Microsoft DSAT"},
    {0x67, "Micorsoft DSAT DISPLAY"},
    {0x69, "Voxware Byte Aligned"},
    {0x70, "Voxware AC8"},
    {0x71, "Voxware AC10"},
    {0x72, "Voxware AC16"},
    {0x73, "Voxware AC20"},
    {0x74, "Voxware MetaVoice"},
    {0x75, "Voxware MetaSound"},
    {0x76, "Voxware RT29HW"},
    {0x77, "Voxware VR12"},
    {0x78, "Voxware VR18"},
    {0x79, "Voxware TQ40"},
    {0x7a, "Voxware SC3"},
    {0x7b, "Voxware SC3"},
    {0x80, "Soundsoft"},
    {0x81, "Voxware TQ60"},
    {0x82, "Microsoft MSRT24"},
    {0x83, "AT&T G.729A"},
    {0x84, "Motion Pixels MVI MV12"},
    {0x85, "DataFusion G.726"},
    {0x86, "DataFusion GSM610"},
    {0x88, "Iterated Systems Audio"},
    {0x89, "Onlive"},
    {0x8a, "Multitude, Inc. FT SX20"},
    {0x8b, "Infocom ITS A/S G.721 ADPCM"},
    {0x8c, "Convedia G729"},
    {0x8d, "Not specified congruency, Inc."},
    {0x91, "Siemens SBC24"},
    {0x92, "Sonic Foundry Dolby AC3 APDIF"},
    {0x93, "MediaSonic G.723"},
    {0x94, "Aculab Prosody 8kbps"},
    {0x97, "ZyXEL ADPCM"},
    {0x98, "Philips LPCBB"},
    {0x99, "Studer Professional Audio Packed"},
    {0xa0, "Malden PhonyTalk"},
    {0xa1, "Racal Recorder GSM"},
    {0xa2, "Racal Recorder G720.a"},
    {0xa3, "Racal G723.1"},
    {0xa4, "Racal Tetra ACELP"},
    {0xb0, "NEC AAC NEC Corporation"},
    {0xff, "AAC"},
    {0x100, "Rhetorex ADPCM"},
    {0x101, "IBM u-Law"},
    {0x102, "IBM a-Law"},
    {0x103, "IBM ADPCM"},
    {0x111, "Vivo G.723"},
    {0x112, "Vivo Siren"},
    {0x120, "Philips Speech Processing CELP"},
    {0x121, "Philips Speech Processing GRUNDIG"},
    {0x123, "Digital G.723"},
    {0x125, "Sanyo LD ADPCM"},
    {0x130, "Sipro Lab ACEPLNET"},
    {0x131, "Sipro Lab ACELP4800"},
    {0x132, "Sipro Lab ACELP8V3"},
    {0x133, "Sipro Lab G.729"},
    {0x134, "Sipro Lab G.729A"},
    {0x135, "Sipro Lab Kelvin"},
    {0x136, "VoiceAge AMR"},
    {0x140, "Dictaphone G.726 ADPCM"},
    {0x150, "Qualcomm PureVoice"},
    {0x151, "Qualcomm HalfRate"},
    {0x155, "Ring Zero Systems TUBGSM"},
    {0x160, "Microsoft Audio1"},
    {0x161, "Windows Media Audio V2 V7 V8 V9 / DivX audio (WMA) / Alex AC3 Audio"},
    {0x162, "Windows Media Audio Professional V9"},
    {0x163, "Windows Media Audio Lossless V9"},
    {0x164, "WMA Pro over S/PDIF"},
    {0x170, "UNISYS NAP ADPCM"},
    {0x171, "UNISYS NAP ULAW"},
    {0x172, "UNISYS NAP ALAW"},
    {0x173, "UNISYS NAP 16K"},
    {0x174, "MM SYCOM ACM SYC008 SyCom Technologies"},
    {0x175, "MM SYCOM ACM SYC701 G726L SyCom Technologies"},
    {0x176, "MM SYCOM ACM SYC701 CELP54 SyCom Technologies"},
    {0x177, "MM SYCOM ACM SYC701 CELP68 SyCom Technologies"},
    {0x178, "Knowledge Adventure ADPCM"},
    {0x180, "Fraunhofer IIS MPEG2AAC"},
    {0x190, "Digital Theater Systems DTS DS"},
    {0x200, "Creative Labs ADPCM"},
    {0x202, "Creative Labs FASTSPEECH8"},
    {0x203, "Creative Labs FASTSPEECH10"},
    {0x210, "UHER ADPCM"},
    {0x215, "Ulead DV ACM"},
    {0x216, "Ulead DV ACM"},
    {0x220, "Quarterdeck Corp."},
    {0x230, "I-Link VC"},
    {0x240, "Aureal Semiconductor Raw Sport"},
    {0x241, "ESST AC3"},
    {0x250, "Interactive Products HSX"},
    {0x251, "Interactive Products RPELP"},
    {0x260, "Consistent CS2"},
    {0x270, "Sony SCX"},
    {0x271, "Sony SCY"},
    {0x272, "Sony ATRAC3"},
    {0x273, "Sony SPC"},
    {0x280, "TELUM Telum Inc."},
    {0x281, "TELUMIA Telum Inc."},
    {0x285, "Norcom Voice Systems ADPCM"},
    {0x300, "Fujitsu FM TOWNS SND"},
    {0x301, "Fujitsu (not specified)"},
    {0x302, "Fujitsu (not specified)"},
    {0x303, "Fujitsu (not specified)"},
    {0x304, "Fujitsu (not specified)"},
    {0x305, "Fujitsu (not specified)"},
    {0x306, "Fujitsu (not specified)"},
    {0x307, "Fujitsu (not specified)"},
    {0x308, "Fujitsu (not specified)"},
    {0x350, "Micronas Semiconductors, Inc. Development"},
    {0x351, "Micronas Semiconductors, Inc. CELP833"},
    {0x400, "Brooktree Digital"},
    {0x401, "Intel Music Coder (IMC)"},
    {0x402, "Ligos Indeo Audio"},
    {0x450, "QDesign Music"},
    {0x500, "On2 VP7 On2 Technologies"},
    {0x501, "On2 VP6 On2 Technologies"},
    {0x680, "AT&T VME VMPCM"},
    {0x681, "AT&T TCP"},
    {0x700, "YMPEG Alpha (dummy for MPEG-2 compressor)"},
    {0x8ae, "ClearJump LiteWave (lossless)"},
    {0x1000, "Olivetti GSM"},
    {0x1001, "Olivetti ADPCM"},
    {0x1002, "Olivetti CELP"},
    {0x1003, "Olivetti SBC"},
    {0x1004, "Olivetti OPR"},
    {0x1100, "Lernout & Hauspie"},
    {0x1101, "Lernout & Hauspie CELP codec"},
    {0x1102, "Lernout & Hauspie SBC codec"},
    {0x1103, "Lernout & Hauspie SBC codec"},
    {0x1104, "Lernout & Hauspie SBC codec"},
    {0x1400, "Norris Comm. Inc."},
    {0x1401, "ISIAudio"},
    {0x1500, "AT&T Soundspace Music Compression"},
    {0x181c, "VoxWare RT24 speech codec"},
    {0x181e, "Lucent elemedia AX24000P Music codec"},
    {0x1971, "Sonic Foundry LOSSLESS"},
    {0x1979, "Innings Telecom Inc. ADPCM"},
    {0x1c07, "Lucent SX8300P speech codec"},
    {0x1c0c, "Lucent SX5363S G.723 compliant codec"},
    {0x1f03, "CUseeMe DigiTalk (ex-Rocwell)"},
    {0x1fc4, "NCT Soft ALF2CD ACM"},
    {0x2000, "FAST Multimedia DVM"},
    {0x2001, "Dolby DTS (Digital Theater System)"},
    {0x2002, "RealAudio 1 / 2 14.4"},
    {0x2003, "RealAudio 1 / 2 28.8"},
    {0x2004, "RealAudio G2 / 8 Cook (low bitrate)"},
    {0x2005, "RealAudio 3 / 4 / 5 Music (DNET)"},
    {0x2006, "RealAudio 10 AAC (RAAC)"},
    {0x2007, "RealAudio 10 AAC+ (RACP)"},
    {0x2500, "Reserved range to 0x2600 Microsoft"},
    {0x3313, "makeAVIS (ffvfw fake AVI sound from AviSynth scripts)"},
    {0x4143, "Divio MPEG-4 AAC audio"},
    {0x4201, "Nokia adaptive multirate"},
    {0x4243, "Divio G726 Divio, Inc."},
    {0x434c, "LEAD Speech"},
    {0x564c, "LEAD Vorbis"},
    {0x5756, "WavPack Audio"},
    {0x674f, "Ogg Vorbis (mode 1)"},
    {0x6750, "Ogg Vorbis (mode 2)"},
    {0x6751, "Ogg Vorbis (mode 3)"},
    {0x676f, "Ogg Vorbis (mode 1+)"},
    {0x6770, "Ogg Vorbis (mode 2+)"},
    {0x6771, "Ogg Vorbis (mode 3+)"},
    {0x7000, "3COM NBX 3Com Corporation"},
    {0x706d, "FAAD AAC"},
    {0x7a21, "GSM-AMR (CBR, no SID)"},
    {0x7a22, "GSM-AMR (VBR, including SID)"},
    {0xa100, "Comverse Infosys Ltd. G723 1"},
    {0xa101, "Comverse Infosys Ltd. AVQSBC"},
    {0xa102, "Comverse Infosys Ltd. OLDSBC"},
    {0xa103, "Symbol Technologies G729A"},
    {0xa104, "VoiceAge AMR WB VoiceAge Corporation"},
    {0xa105, "Ingenient Technologies Inc. G726"},
    {0xa106, "ISO/MPEG-4 advanced audio Coding"},
    {0xa107, "Encore Software Ltd G726"},
    {0xa109, "Speex ACM Codec xiph.org"},
    {0xdfac, "DebugMode SonicFoundry Vegas FrameServer ACM Codec"},
    {0xe708, "Unknown -"},
    {0xf1ac, "Free Lossless Audio Codec FLAC"},
    {0xfffe, "Extensible"},
    {0xffff, "Development"}};

}  // namespace Exiv2::Internal
// *****************************************************************************
// class member definitions

namespace Exiv2 {

enum streamTypeInfo { Audio = 1, MIDI, Text, Video };

RiffVideo::RiffVideo(BasicIo::UniquePtr io) : Image(ImageType::riff, mdNone, std::move(io)) {
}  // RiffVideo::RiffVideo

std::string RiffVideo::mimeType() const {
  return "video/riff";
}

void RiffVideo::writeMetadata() {
}  // RiffVideo::writeMetadata

void RiffVideo::readMetadata() {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());

  // Ensure that this is the correct image type
  if (!isRiffType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "RIFF");
  }

  IoCloser closer(*io_);
  clearMetadata();

  xmpData_["Xmp.video.FileSize"] = io_->size();
  xmpData_["Xmp.video.MimeType"] = mimeType();

  xmpData_["Xmp.video.Container"] = readStringTag(io_);

  io_->seekOrThrow(io_->tell() + DWORD, BasicIo::beg, ErrorCode::kerFailedToReadImageData);  // skeep id

  xmpData_["Xmp.video.FileType"] = readStringTag(io_);

  decodeBlocks();
}  // RiffVideo::readMetadata

bool RiffVideo::equal(const std::string& str1, const std::string& str2) {
  if (str1.size() != str2.size())
    return false;
  for (size_t i = 0; i < str1.size(); i++)
    if (toupper(str1[i]) != str2[i])
      return false;
  return true;
}

void RiffVideo::readList(uint64_t size) {
  DataBuf FormTypeBuf_ = io_->read(DWORD);

#ifdef EXIV2_DEBUG_MESSAGES
  EXV_DEBUG << "-> Reading list : id= " << id << "  type= " << Exiv2::toString(FormTypeBuf_.data()) << " size= " << size
            << "(" << io_->tell() << "/" << io_->size() << ")" << std::endl;
#endif

  if (equal(Exiv2::toString(FormTypeBuf_.data()), CHUNK_ID_INFO))
    readInfoListChunk(size);
  else if (equal(Exiv2::toString(FormTypeBuf_.data()), CHUNK_ID_MOVI)) {
    readMoviList(size);
  }
}

void RiffVideo::readChunk(uint64_t size, const std::string& id) {
#ifdef EXIV2_DEBUG_MESSAGES
  EXV_DEBUG << "--> Reading Chunk : [" << id << "] size= " << size << "(" << io_->tell() << "/" << io_->size() << ")"
            << std::endl;
#endif

  if (equal(id, CHUNK_ID_AVIH))
    readAviHeader();
  else if (equal(id, CHUNK_ID_STRH))
    readStreamHeader();
  else if (equal(id, CHUNK_ID_STRF))
    readStreamFormat(size);
  else if (equal(id, CHUNK_ID_FMT)) {
    streamType_ = Audio;
    readStreamFormat(size);
  } else if (equal(id, CHUNK_ID_STRD))
    readStreamData(size);
  else if (equal(id, CHUNK_ID_STRN))
    StreamName(size);
  else if (equal(id, CHUNK_ID_VPRP))
    readVPRPChunk(size);
  else if (equal(id, CHUNK_ID_IDX1))
    readIndexChunk(size);
  else if (equal(id, CHUNK_ID_DATA))
    readDataChunk(size);
  else if (equal(id, CHUNK_ID_JUNK))
    readJunk(size);
  else {
#ifdef EXIV2_DEBUG_MESSAGES
    EXV_DEBUG << "--> Ignoring Chunk : " << id << "] size= " << size << "(" << io_->tell() << "/" << io_->size() << ")"
              << std::endl;
#endif
    io_->seekOrThrow(io_->tell() + size, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
  }
}

void RiffVideo::decodeBlocks() {
  std::string id = readStringTag(io_);
  uint64_t size = readDWORDTag(io_);
  if (equal(id, CHUNK_ID_LIST)) {
    readList(size);
  } else {
    readChunk(size, id);
  }

  if (!io_->eof() && io_->tell() < io_->size()) {
    decodeBlocks();
  }

}  // RiffVideo::decodeBlock

void RiffVideo::readAviHeader() {
#ifdef EXIV2_DEBUG_MESSAGES
  EXV_DEBUG << "--> dwMicroSecPerFrame    = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwMaxBytesPerSec      = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwPaddingGranularity  = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwFlags               = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwTotalFrames         = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwInitialFrames       = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwStreams             = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwSuggestedBufferSize = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwWidth               = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwHeight              = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwReserved1           = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwReserved2           = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwReserved3           = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwReserved4           = " << readDWORDTag(io_) << std::endl;
#else

  uint32_t TimeBetweenFrames = readDWORDTag(io_);
  xmpData_["Xmp.video.MicroSecPerFrame"] = TimeBetweenFrames;
  double frame_rate = 1000000. / TimeBetweenFrames;

  xmpData_["Xmp.video.MaxDataRate"] = readDWORDTag(io_);  // MaximumDataRate

  io_->seekOrThrow(io_->tell() + DWORD * 2, BasicIo::beg,
                   ErrorCode::kerFailedToReadImageData);  // ignore PaddingGranularity and Flags

  uint32_t frame_count = readDWORDTag(io_);  // TotalNumberOfFrames
  xmpData_["Xmp.video.FrameCount"] = frame_count;

  io_->seekOrThrow(io_->tell() + DWORD, BasicIo::beg,
                   ErrorCode::kerFailedToReadImageData);  // ignore NumberOfInitialFrames

  xmpData_["Xmp.audio.ChannelType"] = getStreamType(readDWORDTag(io_));  // NumberOfStreams

  xmpData_["Xmp.video.StreamCount"] = readDWORDTag(io_);  // SuggestedBufferSize

  uint32_t width = readDWORDTag(io_);
  xmpData_["Xmp.video.Width"] = width;

  uint32_t height = readDWORDTag(io_);
  xmpData_["Xmp.video.Height"] = height;

  io_->seekOrThrow(io_->tell() + DWORD * 4, BasicIo::beg,
                   ErrorCode::kerFailedToReadImageData);  // TimeScale, DataRate, StartTime, DataLength

  fillAspectRatio(width, height);
  fillDuration(frame_rate, frame_count);
#endif
}

void RiffVideo::readStreamHeader() {
  std::string stream = readStringTag(io_);
  streamType_ = (equal(stream, "VIDS")) ? Video : Audio;

#ifdef EXIV2_DEBUG_MESSAGES
  EXV_DEBUG << "--> fccType                = " << stream << std::endl;
  EXV_DEBUG << "--> fccHandler             = " << readStringTag(io_) << std::endl;
  EXV_DEBUG << "--> dwFlags                = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> wPriority              = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> wLanguage              = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwInitialFrames        = " << readDWORDTag(io_) << std::endl;  // 20
  EXV_DEBUG << "--> dwScale                = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwRate                 = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwStart                = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwLength               = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> dwSuggestedBufferSize  = " << readDWORDTag(io_) << std::endl;  // 40
  EXV_DEBUG << "--> dwSampleSize           = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> Left                   = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> top                    = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> right                  = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> bottom                 = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> XXXXXX                = " << readDWORDTag(io_) << std::endl;  // 56

#else

  xmpData_["Xmp.video.Codec"] = readStringTag(io_);  // DataHandler

  io_->seekOrThrow(io_->tell() + DWORD * 2 + WORD * 2, BasicIo::beg,
                   ErrorCode::kerFailedToReadImageData);  // dwFlags, wPriority, wLanguage, dwInitialFrames

  uint32_t divisor = readDWORDTag(io_);  // TimeScale

  if (divisor) {
    double rate = static_cast<double>(readDWORDTag(io_) / divisor);
    xmpData_[(streamType_ == Video) ? "Xmp.video.FrameRate" : "Xmp.audio.SampleRate"] = rate;
  }
  io_->seekOrThrow(io_->tell() + DWORD, BasicIo::beg, ErrorCode::kerFailedToReadImageData);  // dwStart

  if (divisor) {
    double frame_count = static_cast<double>(readDWORDTag(io_) / divisor);  // DataLength
    xmpData_[(streamType_ == Video) ? "Xmp.video.FrameCount" : "Xmp.audio.FrameCount"] = frame_count;
  }

  io_->seekOrThrow(io_->tell() + DWORD, BasicIo::beg, ErrorCode::kerFailedToReadImageData);  // dwSuggestedBufferSize

  xmpData_[(streamType_ == Video) ? "Xmp.video.VideoQuality" : "Xmp.video.StreamQuality"] = readDWORDTag(io_);

  xmpData_[(streamType_ == Video) ? "Xmp.video.VideoSampleSize" : "Xmp.video.StreamSampleSize"] = readDWORDTag(io_);
  io_->seekOrThrow(io_->tell() + DWORD * 2, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
#endif
}

void RiffVideo::readStreamFormat(uint64_t size_) {
  // The structure of the strf chunk depends on the media type. Video streams use the BITMAPINFOHEADER structure,
  // whereas audio streams use the WAVEFORMATEX structure.

#ifdef EXIV2_DEBUG_MESSAGES
  if (streamType_ == Video) {
    EXV_DEBUG << "--> wFormatTag      = " << readWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> nChannels       = " << readWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> nSamplesPerSec  = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> nAvgBytesPerSec = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> nBlockAlign     = " << readWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> wBitsPerSample  = " << readWORDTag(io_) << std::endl;
  } else {
    EXV_DEBUG << "--> biSize            = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biWidth           = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biHeight          = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biPlanes          = " << readWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biBitCount        = " << readWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biCompression     = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biSizeImage       = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biXPelsPerMeter   = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biYPelsPerMeter   = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biClrUsed         = " << readDWORDTag(io_) << std::endl;
    EXV_DEBUG << "--> biClrImportant    = " << readDWORDTag(io_) << std::endl;
  }

#else

  if (streamType_ == Video) {
    io_->seekOrThrow(io_->tell() + DWORD * 3, BasicIo::beg,
                     ErrorCode::kerFailedToReadImageData);  // ignore biSize, biWidth, biHeight
    xmpData_["Xmp.video.Planes"] = readWORDTag(io_);
    xmpData_["Xmp.video.PixelDepth"] = readWORDTag(io_);
    xmpData_["Xmp.video.Compressor"] = readStringTag(io_);
    xmpData_["Xmp.video.ImageLength"] = readDWORDTag(io_);
    xmpData_["Xmp.video.PixelPerMeterX"] = readQWORDTag(io_);
    xmpData_["Xmp.video.PixelPerMeterY"] = readQWORDTag(io_);
    uint32_t NumOfColours = readDWORDTag(io_);
    if (NumOfColours == 0)
      xmpData_["Xmp.video.NumOfColours"] = "Unspecified";
    else
      xmpData_["Xmp.video.NumOfColours"] = NumOfColours;
    uint32_t NumIfImpColours = readDWORDTag(io_);
    if (NumIfImpColours == 0)
      xmpData_["Xmp.video.NumIfImpColours"] = "All";
    else
      xmpData_["Xmp.video.NumIfImpColours"] = NumIfImpColours;
  } else if (streamType_ == Audio) {
    uint16_t format_tag = readWORDTag(io_);
    auto it = Internal::audioEncodingValues.find(format_tag);
    if (it != Internal::audioEncodingValues.end()) {
      xmpData_["Xmp.audio.Compressor"] = it->second;
    } else {
      xmpData_["Xmp.audio.Compressor"] = format_tag;
    }

    xmpData_["Xmp.audio.ChannelType"] = getStreamType(readDWORDTag(io_));
    xmpData_["Xmp.audio.SampleRate"] = readDWORDTag(io_);                                      // nSamplesPerSec
    io_->seekOrThrow(io_->tell() + DWORD, BasicIo::beg, ErrorCode::kerFailedToReadImageData);  // nAvgBytesPerSec
    xmpData_["Xmp.audio.SampleType"] = readDWORDTag(io_);                                      // nBlockAlign
    xmpData_["Xmp.audio.BitsPerSample"] = readDWORDTag(io_);                                   // wBitsPerSample

#endif
  if (xmpData_["Xmp.video.FileType"].toString() == "AVI ")
    io_->seekOrThrow(io_->tell() + DWORD, BasicIo::beg, ErrorCode::kerFailedToReadImageData);  // cbSize
}
else {
  io_->seekOrThrow(io_->tell() + size_, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
}

}

void RiffVideo::readStreamData(uint64_t size_) {
  io_->seekOrThrow(io_->tell() + size_, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
}

void RiffVideo::StreamName(uint64_t size_) {
  // This element contains a name for the stream. That stream name should only use plain ASCII, especially not UTF-8.
  io_->seekOrThrow(io_->tell() + size_, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
}

void RiffVideo::readInfoListChunk(uint64_t size_) {
  uint64_t current_size = DWORD;
  while (current_size < size_) {
    std::string type = readStringTag(io_);
    size_t size = readDWORDTag(io_);
    std::string content = readStringTag(io_, size);
    auto it = Internal::infoTags.find(type);
    if (it != Internal::infoTags.end()) {
      xmpData_[it->second] = content;
    }
    current_size += DWORD * 2 + size;
  }
}

void RiffVideo::readMoviList(uint64_t size_) {
  io_->seekOrThrow(io_->tell() + size_ - DWORD, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
}

void RiffVideo::readVPRPChunk(uint64_t size_) {
#ifdef EXIV2_DEBUG_MESSAGES
  EXV_DEBUG << "--> VideoFormatToken         = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> VideoStandard            = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> VerticalRefreshRate      = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> HTotalInT                = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> VTotalInLines            = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> FrameAspectRatio Height  = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> FrameAspectRatio Width   = " << readWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> FrameWidthInPixels       = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> FrameHeightInLines       = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> CompressedBMHeight       = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> FieldPerFrame            = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> CompressedBMWidth        = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> ValidBMHeight            = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> ValidBMWidth             = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> ValidBMXOffset           = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> ValidBMYOffset           = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> VideoXOffsetInT          = " << readDWORDTag(io_) << std::endl;
  EXV_DEBUG << "--> VideoYValidStartLine     = " << readDWORDTag(io_) << std::endl;
#else
  io_->seekOrThrow(io_->tell() + size_, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
#endif
}

void RiffVideo::readIndexChunk(uint64_t size_) {
#ifdef EXIV2_DEBUG_MESSAGES
  uint64_t current_size = 0;
  while (current_size < size_) {
    EXV_DEBUG << "--> Identifier               = " << readStringTag(io_) << "\t(" << current_size << "/" << size_ << ")"
              << std::endl;
    EXV_DEBUG << "--> Flags                    = " << readDWORDTag(io_) << "\t(" << current_size << "/" << size_ << ")"
              << std::endl;
    EXV_DEBUG << "--> Offset                   = " << readDWORDTag(io_) << "\t(" << current_size << "/" << size_ << ")"
              << std::endl;
    EXV_DEBUG << "--> Length                   = " << readDWORDTag(io_) << "\t(" << current_size << "/" << size_ << ")"
              << std::endl;
    current_size += DWORD * 4;
  }
#else
  io_->seekOrThrow(io_->tell() + size_, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
#endif
}

void RiffVideo::readDataChunk(uint64_t size_) {
#ifdef EXIV2_DEBUG_MESSAGES
  EXV_DEBUG << "--> Data               = " << readStringTag(static_size<size_t>(size_)) << std::endl;
  if (size_ % 2 != 0) {
    EXV_DEBUG << "--> pad byte          = " << readStringTag(1) << std::endl;
  }
#else
  io_->seekOrThrow(io_->tell() + size_, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
  if (size_ % 2 != 0)
    io_->seekOrThrow(io_->tell() + 1, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
#endif
}

void RiffVideo::readJunk(uint64_t size_) {
  io_->seekOrThrow(io_->tell() + size_, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
}

std::string RiffVideo::getStreamType(uint32_t stream) {
  if (stream == 1)
    return "Mono";
  else if (stream == 2)
    return "Stereo";
  else if (stream == 5)
    return "5.1 Surround Sound";
  else if (stream == 7)
    return "7.1 Surround Sound";
  else
    return "Mono";
}

void RiffVideo::fillDuration(double frame_rate, size_t frame_count) {
  if (frame_rate == 0)
    return;

  auto duration = static_cast<uint64_t>(frame_count * 1000. / frame_rate);
  xmpData_["Xmp.video.FileDataRate"] = io_->size() / (1048576. * duration);
  xmpData_["Xmp.video.Duration"] = duration;  // Duration in number of seconds
}  // RiffVideo::fillDuration

void RiffVideo::fillAspectRatio(size_t width, size_t height) {
  if (height == 0)
    return;
  double aspectRatio = static_cast<double>(width) / height;
  aspectRatio = floor(aspectRatio * 10) / 10;
  xmpData_["Xmp.video.AspectRatio"] = aspectRatio;

  auto aR = static_cast<int>((aspectRatio * 10.0) + 0.1);

  switch (aR) {
    case 13:
      xmpData_["Xmp.video.AspectRatio"] = "4:3";
      break;
    case 17:
      xmpData_["Xmp.video.AspectRatio"] = "16:9";
      break;
    case 10:
      xmpData_["Xmp.video.AspectRatio"] = "1:1";
      break;
    case 16:
      xmpData_["Xmp.video.AspectRatio"] = "16:10";
      break;
    case 22:
      xmpData_["Xmp.video.AspectRatio"] = "2.21:1";
      break;
    case 23:
      xmpData_["Xmp.video.AspectRatio"] = "2.35:1";
      break;
    case 12:
      xmpData_["Xmp.video.AspectRatio"] = "5:4";
      break;
    default:
      xmpData_["Xmp.video.AspectRatio"] = aspectRatio;

      break;
  }
}

Image::UniquePtr newRiffInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<RiffVideo>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isRiffType(BasicIo& iIo, bool advance) {
  constexpr int len = 4;
  const unsigned char RiffVideoId[len] = {'R', 'I', 'F', 'F'};
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  bool matched = (memcmp(buf, RiffVideoId, len) == 0);
  if (!advance || !matched) {
    iIo.seek(-1 * len, BasicIo::cur);
  }
  return matched;
}
}  // namespace Exiv2
