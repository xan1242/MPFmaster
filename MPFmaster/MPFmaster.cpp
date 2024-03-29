// MPF tool
// by Xan / Tenjoin
// highly based on tool by Nicknine (mpf2txt.py / mpftotext.py)

// TODO: big endian

#if defined (_WIN32) || defined (_WIN64)
#define path_separator "\\"
#define path_separator_char '\\'
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>
#endif

#if __GNUC__
#include <sys/stat.h>
#include <dirent.h>
#define path_separator "/"
#define path_separator_char '/'
#endif

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include "crc24.h"
#include "bswap.h"
#include "PathFinderStructs.h"
using namespace std;

//#define MUS_SAMPLE_FILE_EXT ".bin"
#define ASF_FILE_EXT ".asf"
#define SNS_FILE_EXT ".sns"
#define SNR_FILE_EXT ".snr"

PATHFINDHEADER hdr;
vector<PATHNAMEDVAR> namedvars;
vector<uint32_t> nodeOffsets;
vector<uint32_t> eventOffsets;
vector<uint32_t> routerOffsets;
vector<uint32_t> trackinfoOffsets;
vector<uint32_t> sampleOffsets;
vector<uint32_t> sampleSizes;
vector<uint32_t> sampleLengths;
vector<PATHTRACKINFO> trackinfos;

char varstr[8][32];
int curvarstr = 0;

uint32_t curEventID;
uint32_t curOffset;

wchar_t MkDirPath[1024];
char SampleFilePath[1024];
char MusFilePath[1024];
char OutPath[1024];

// sample updater
vector<string> FileDirectoryListing;
vector<string> FileDirectoryListing_SNS;
bool bEALayer3Mode = false;

// compiler stuff start
enum cmpMainTokenType
{
    cmpNULL,
    cmpVAR,
    cmpROUTER,
    cmpTRACK,
    cmpNODE,
    cmpEVENT,
    cmpMajorRev,
    cmpMinorRev,
    cmpReleaseRev,
    cmpPrereleaseRev,
    cmpGenerateID,
    cmpProjectID,
    cmpNumSections,
    cmpMainTokenMAX
};

const char* cmpMainTokenTypeStr[] =
{
    "NULL",
    "Var",
    "Router",
    "Track",
    "Node",
    "Event",
    "Major",
    "Minor",
    "Release",
    "Prerelease",
    "GenerateID",
    "ProjectID",
    "NumSections",
};

enum cmpParsingStatus
{
    cmpPS_NULL,
    cmpPS_VAR,
    cmpPS_ROUTER,
    cmpPS_TRACK,
    cmpPS_NODE,
    cmpPS_EVENT,
    cmpPS_MAX
};

enum cmpParsingSubStatus
{
    cmpPSS_NULL,
    cmpPSS_EnteredBody,
    cmpPSS_MAX
};

struct ROUTERpair
{
    uint32_t node1;
    uint32_t node2;
};

struct cmpROUTERType
{
    vector<ROUTERpair> rp;
    int index = 0;
    bool operator < (const cmpROUTERType& s) const
    {
        return (index < s.index);
    }
};

enum cmpTrackInfoParser
{
    cmpTIP_NULL,
    cmpTIP_StartingSample,
    cmpTIP_SubBanks,
    cmpTIP_MusChecksum,
    cmpTIP_MaxARAM,
    cmpTIP_MaxMRAM,
    cmpTIP_MAX
};

const char* cmpTrackInfoStrings[] =
{
    "NULL",
    "StartingSample",
    "SubBanks",
    "MusChecksum",
    "MaxARAM",
    "MaxMRAM"
};

struct cmpPATHTRACKINFO
{
    PATHTRACKINFO track;
    int index;

    bool operator < (const cmpPATHTRACKINFO& s) const
    {
        return (index < s.index);
    }
};

enum cmpPathNodeParser
{
    cmpPNP_NULL,
    cmpPNP_Wave,
    cmpPNP_Track,
    cmpPNP_Section,
    cmpPNP_Part,
    cmpPNP_Router,
    cmpPNP_Controller,
    cmpPNP_Beats,
    cmpPNP_Bars,
    cmpPNP_SynchEvery,
    cmpPNP_SynchOffset,
    cmpPNP_Notes,
    cmpPNP_Synch,
    cmpPNP_ChannelBranching,
    cmpPNP_Repeat,
    cmpPNP_Branches,
    cmpPNP_Event,
    cmpPNP_MAX
};

const char* cmpPathNodeStrings[] =
{
    "NULL",
    "Wave",
    "Track",
    "Section",
    "Part",
    "Router",
    "Controller",
    "Beats",
    "Bars",
    "SynchEvery",
    "SynchOffset",
    "Notes",
    "Synch",
    "ChannelBranching",
    "Repeat",
    "Branches",
    "Event",
};

struct cmpPATHFINDNODE
{
    PATHFINDNODE node = { 0 };
    PATHNODEEVENT sendevent = { 0 };
    vector<PATHFINDBRANCH> branches;
    int index = 0;

    bool operator < (const cmpPATHFINDNODE& s) const
    {
        return (index < s.index);
    }
};

enum cmpEventParser
{
    cmpEP_NULL,
    cmpEP_Actions,
    cmpEP_MAX
};

const char* cmpEventStrings[] =
{
    "NULL",
    "Actions",
};

const char* cmpActionStrings[] =
{
    "NONE",
    "Condition",
    "Wait",
    "WaitBeat",
    "Branch",
    "Fade",
    "DryFade",
    "SFXFade",
    "Set",
    "Event",
    "Filter ON",
    "Filter OFF",
    "Filter CLEAR",
    "Callback",
    "Calc",
    "Pause",
    "LoadBank",
    "PitchFade",
    "StretchFade",
};

struct cmpPATHEVENT
{
    PATHEVENT evt = { 0 };
    vector<PATHACTION> actions;
};

vector<PATHNAMEDVAR> cmpNamedVars;
vector<cmpROUTERType> cmpRouters;
vector<cmpPATHTRACKINFO> cmpTrackInfos;
vector<cmpPATHFINDNODE> cmpNodes;
vector<cmpPATHEVENT> cmpEvents;

char cmpReadLine[512];
unsigned int cmpCurLine = 1;
cmpParsingStatus cmpCurrentPS = cmpPS_NULL;
cmpParsingSubStatus cmpCurrentPSS = cmpPSS_NULL;
cmpParsingSubStatus cmpCurrentPSSS = cmpPSS_NULL;

vector<uint16_t> cmpNodeOffsets;
vector<uint16_t> cmpEventOffsets;
vector<uint32_t> cmpRouterOffsets;
vector<uint32_t> cmpTrackOffsets;

// compiler stuff end

// music add + concat stuff 

int LastNodeIndex = 0;
int LastPartIndex = 0;
int LastWaveIndex = 0;
int LastRouterIndex = 0;
int LastSectionIndex = 0;
char catOutLine[512];
char catReadLine[512];
char catOldFilename[512];
char catNewFilename[512];

// music add + concat stuff end

// https://github.com/Medstar117/RA3MusEditor/blob/master/pathmusic.txt#L23
struct MUSHeaderInfo
{
    uint32_t hash;
    uint32_t index;
    uint32_t HeaderOffset; // *0x10
    uint32_t DataOffset; // *0x80
    uint32_t HeaderSize;
    uint32_t DataSize;
    uint32_t pad;
};

vector<MUSHeaderInfo> EAL3HeaderInfos;

enum SampleFileType
{
    sftNONE,
    sftASF,
    sftEAL3,
    sftMAX
};

const char* SampleFileTypeStr[] =
{
    ".bin",
    ASF_FILE_EXT,
    SNS_FILE_EXT,
};

SampleFileType GetSampleFileType(uint32_t magic)
{
    if (magic == EA_SCHL_MAGIC) // TODO: add better detection mechanic
        return sftASF;
    return sftEAL3; // TODO: add detection for EA Layer 3...
}


#ifdef WIN32
DWORD GetDirectoryListing(const char* FolderPath)
{
    WIN32_FIND_DATA ffd = { 0 };
    TCHAR  szDir[MAX_PATH];
    char MBFilename[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    unsigned int NameCounter = 0;

    mbstowcs(szDir, FolderPath, MAX_PATH);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    if (strlen(FolderPath) > (MAX_PATH - 3))
    {
        printf("Directory path is too long.\n");
        return -1;
    }

    hFind = FindFirstFile(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        printf("FindFirstFile error\n");
        return dwError;
    }

    do
    {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            wcstombs(MBFilename, ffd.cFileName, MAX_PATH);
            string ext(MBFilename);
            if ((ext.find(SNR_FILE_EXT) != string::npos))
            {
                if (!bEALayer3Mode)
                {
                    cout << "Going into EALayer3 mode!\n";
                    bEALayer3Mode = true;
                }
                FileDirectoryListing.push_back(MBFilename);
            }
            else if ((ext.find(SNS_FILE_EXT) != string::npos))
            {
                if (!bEALayer3Mode)
                {
                    cout << "Going into EALayer3 mode!\n";
                    bEALayer3Mode = true;
                }
                FileDirectoryListing_SNS.push_back(MBFilename);
            }
            else if (!bEALayer3Mode)
            {
                FileDirectoryListing.push_back(MBFilename);
            }
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        printf("FindFirstFile error\n");
    }
    FindClose(hFind);

    return dwError;
}
#elif __GNUC__
void GetDirectoryListing(const char* FolderPath)
{
    struct dirent* dp;
    DIR* dir = opendir(FolderPath);
    unsigned int NameCounter = 0;

    while ((dp = readdir(dir)))
    {
        // ignore the current and previous dir files...
        if (!((strcmp(dp->d_name, ".") == 0) || (strcmp(dp->d_name, "..") == 0)))
            FileDirectoryListing.push_back(dp->d_name);
    }
    closedir(dir);
}
#endif

const char* actValToString(int val, int typ)
{
    const char* retval = (const char*)varstr[curvarstr];
    memset(varstr[curvarstr], 0, 32);
    switch (typ)
    {
    case PATH_VALUE_INTEGER:
        sprintf(varstr[curvarstr], "%d", val);
        curvarstr++;
        curvarstr %= 8;
        return retval;
    case PATH_VALUE_VARIABLE:
        sprintf(varstr[curvarstr], "var[%s]", namedvars[val].name);
        curvarstr++;
        curvarstr %= 8;
        return retval;
    case PATH_VALUE_SPECIAL:
        return specialTypes[val];
    default:
        cout << "Bad value type " << typ << " in event 0x" << std::uppercase << std::hex << curEventID << " @0x" << std::uppercase << std::hex << curOffset << '\n';
        sprintf(varstr[curvarstr], "%d", val);
        curvarstr++;
        curvarstr %= 8;
        return retval;
    }
}

const char* actFadeValToString(int typ)
{
    if ((typ >= PATH_FADE_MAXTYPES) || (typ <= 0))
    {
        return fadeTypes[0];
    }

    return fadeTypes[typ];
}

char* cmpCleanUpToken(char* intok)
{
    if (strlen(intok) > 1)
    {
        char* token = intok;
        // clean up tokens
        // clean tabs and spaces from front
        while (isspace(*token))
            token++;
        // clean tabs and spaces from back
        size_t len = strlen(token) - 1;
        while (isspace(token[len]))
        {
            token[len] = '\0';
            len--;
        }
        len = strlen(token);
        strcpy(intok, token);
    }
    return intok;
}

bool bStringHasAlpha(char* str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        //if (isalpha(str[i]))
        //    return true;
        if (!isdigit(str[i]))
        {
            if (str[i] != '-')
                return true;
        }
    }

    return false;
}

void fprintDividerLine(FILE* f, char start, char repeat, char end, int count)
{
    if (start)
        fputc(start, f);
    if (repeat)
    {
        for (int k = 0; k < count; k++)
            fputc(repeat, f);
    }
    if (end)
        fputc(end, f);
    fputc('\n', f);
}

bool MPF_ValidateHeader(PATHFINDHEADER *hdr) {
    if (hdr->id != PATHFINDER_MAGIC)
    {
        cout << "Bad header magic! Expected: 0x" << std::uppercase << std::hex << PATHFINDER_MAGIC << ", Got: 0x" << std::uppercase << std::hex << hdr->id << '\n';
        return false;
    }

    if (hdr->majorRev != PATH_SUPPORTED_VERSION)
    {
        cout << "Unsupported PATH version! Expected: " << PATH_SUPPORTED_VERSION << ", Got: " << (int) hdr->majorRev << '\n';
        return false;
    }

    return true;
}

int MPFtoTXT(char* mpffilename, char* txtfilename)
{
    FILE* f = fopen(mpffilename, "rb");
    if (!f)
    {
        cout << "Can't open file " << mpffilename << " for reading: " << strerror(errno) << '\n';
        return -1;
    }

    fread(&hdr, sizeof(PATHFINDHEADER), 1, f);

    if (!MPF_ValidateHeader(&hdr))
    {
        fclose(f);
        return -1;
    }

    FILE* fout = fopen(txtfilename, "wb");
    if (!fout)
    {
        cout << "Can't open file " << txtfilename << " for writing: " << strerror(errno) << '\n';
        fclose(f);
        return -1;
    }

    // Read version info
    fprintDividerLine(fout, '#', '-', 0, 72);
    fputs("# Version info\n", fout);
    fprintDividerLine(fout, '#', '-', 0, 72);
    fprintf(fout, "Major %d\n", hdr.majorRev);
    fprintf(fout, "Minor %d\n", hdr.minorRev);
    fprintf(fout, "Release %d\n", hdr.release);
    fprintf(fout, "Prerelease %d\n", hdr.prerelease);
    fprintf(fout, "GenerateID %d\n", hdr.generateID);
    fprintf(fout, "ProjectID %d\n", hdr.projectID);
    fprintf(fout, "NumSections %d\n", hdr.numsections);

    // Read named vars
    fseek(f, hdr.namedvars, SEEK_SET);
    fprintDividerLine(fout, '#', '-', 0, 72);
    fputs("# Named vars\n", fout);
    fprintDividerLine(fout, '#', '-', 0, 72);
    for (int i = 0; i < hdr.numnamedvars; i++)
    {
        PATHNAMEDVAR var = {0};
        fread(&var, sizeof(PATHNAMEDVAR), 1, f);
        namedvars.push_back(var);
        fprintf(fout, "Var\n");
        fputs("{\n", fout);
        fprintf(fout, "\t%s = %d\n", var.name, var.value);
        fputs("}\n", fout);
    }

    // Read routers
    fprintDividerLine(fout, '#', '-', 0, 72);
    fputs("# Routers\n", fout);
    fprintDividerLine(fout, '#', '-', 0, 72);

    fseek(f, hdr.noderouters, SEEK_SET);
    for (int i = 0; i < hdr.numrouters + 1; i++)
    {
        uint32_t offset = 0;
        fread(&offset, sizeof(uint32_t), 1, f);
        offset *= 4;
        routerOffsets.push_back(offset);
    }
    for (int i = 0; i < hdr.numrouters; i++)
    {
        uint32_t offset = routerOffsets.at(i);
        uint32_t nextOffset = routerOffsets.at(i + 1);
        uint32_t router = 0;
        uint32_t node1 = 0;
        uint32_t node2 = 0;

        fseek(f, offset, SEEK_SET);
        
        fprintf(fout, "Router %d\n", i);
        fputs("{\n", fout);

        while (ftell(f) != nextOffset)
        {
            fread(&router, sizeof(uint32_t), 1, f);
            node1 = router >> 16;
            node2 = router & 0xFFFF;
            fprintf(fout, "\t%d > %d\n", node1, node2);
        }

        fputs("}\n", fout);
    }

    // Read tracks
    fseek(f, hdr.trackoffsets, SEEK_SET);
    fprintDividerLine(fout, '#', '-', 0, 72);
    fputs("# Tracks\n", fout);
    fprintDividerLine(fout, '#', '-', 0, 72);
    for (int i = 0; ftell(f) < hdr.trackinfos; i++)
    {
        uint32_t offset = 0;
        fread(&offset, sizeof(uint32_t), 1, f);
        offset *= 4;
        trackinfoOffsets.push_back(offset);
    }
    for (int i = 0; i < trackinfoOffsets.size(); i++)
    {        
        PATHTRACKINFO trackinfo = { 0 };
        fseek(f, trackinfoOffsets.at(i), SEEK_SET);
        fread(&trackinfo, sizeof(PATHTRACKINFO), 1, f);
        fprintf(fout, "Track %d\n", i);
        fputs("{\n", fout);

        fprintf(fout, "\tStartingSample %d\n", trackinfo.startingsample);
        fprintf(fout, "\tSubBanks %d\n", trackinfo.numsubbanks);
        fprintf(fout, "\tMusChecksum 0x%08X\n", trackinfo.muschecksum);
        fprintf(fout, "\tMaxARAM %d\n", trackinfo.maxaram);
        fprintf(fout, "\tMaxMRAM %d\n", trackinfo.maxmram);

        fputs("}\n", fout);
    }

    // Read nodes
    fprintDividerLine(fout, '#', '-', 0, 72);
    fputs("# Nodes\n", fout);
    fprintDividerLine(fout, '#', '-', 0, 72);


    fseek(f, hdr.nodeoffsets, SEEK_SET);
    for (int i = 0; i < hdr.numnodes; i++)
    {
        uint32_t offset = 0;
        fread(&offset, sizeof(uint16_t), 1, f);
        offset *= 4;
        nodeOffsets.push_back(offset);
    }
    for (int i = 0; i < hdr.numnodes; i++)
    {
        PATHFINDNODE node = {0};
        uint32_t offset = nodeOffsets.at(i);

        fseek(f, offset, SEEK_SET);
        fread(&node, sizeof(PATHFINDNODE), 1, f);

        fprintf(fout, "Node %d # 0x%08X\n", i, offset);
        fputs("{\n", fout);
        fprintf(fout, "\tWave %d\n", node.index);
        fprintf(fout, "\tTrack %d\n", node.trackID);
        fprintf(fout, "\tSection %d\n", node.sectionID);
        fprintf(fout, "\tPart %d\n", node.partID);
        fprintf(fout, "\tRouter %d\n", node.routerID - 1);
        fprintf(fout, "\tController %d\n", node.controller);
        fprintf(fout, "\tBeats %d\n", node.beats);
        fprintf(fout, "\tBars %d\n", node.bars);
        fprintf(fout, "\tSynchEvery %d\n", node.synchevery);
        fprintf(fout, "\tSynchOffset %d\n", node.synchoffset);
        fprintf(fout, "\tNotes %d\n", node.notes);
        fprintf(fout, "\tSynch %d\n", node.synch);
        fprintf(fout, "\tChannelBranching %d\n", node.channelbranching);

        if (node.index == -1)
            fprintf(fout, "\tRepeat %d\n", node.repeat);
        if (node.index == -3)
        {
            PATHNODEEVENT sendevent;
            fread(&sendevent, sizeof(PATHNODEEVENT), 1, f);
            fprintf(fout, "\tEvent 0x%08X\n", sendevent.eventID);
        }
        else
            fseek(f, 4, SEEK_CUR);
        if (node.numbranches)
        {
            fputs("\tBranches\n", fout);
            fputs("\t{\n", fout);
            for (int j = 0; j < node.numbranches; j++)
            {
                PATHFINDBRANCH branch;
                fread(&branch, sizeof(PATHFINDBRANCH), 1, f);
                fprintf(fout, "\t\tControl %d,%d > %d\n", branch.controlmin, branch.controlmax, branch.dstnode);
            }
            fputs("\t}\n", fout);
        }
        fputs("}\n", fout);
    }

    // Read events
    fprintDividerLine(fout, '#', '-', 0, 72);
    fputs("# Events\n", fout);
    fprintDividerLine(fout, '#', '-', 0, 72);
    fseek(f, hdr.eventoffsets, SEEK_SET);
    for (int i = 0; i < hdr.numevents; i++)
    {
        uint32_t offset = 0;
        fread(&offset, sizeof(uint16_t), 1, f);
        offset *= 4;
        eventOffsets.push_back(offset);
    }
    for (int i = 0; i < hdr.numevents; i++)
    {
        PATHEVENT evt = { 0 };
        
        int indent = 0;
        uint32_t offset = eventOffsets.at(i);
        curOffset = offset;

        fseek(f, offset, SEEK_SET);
        fread(&evt, sizeof(PATHEVENT), 1, f);
        fprintf(fout, "Event 0x%08X # 0x%08X\n", evt.eventID, offset);
        curEventID = evt.eventID;

        fputs("{\n", fout);

        if (evt.numactions)
        {
            fputs("\tActions\n", fout);
            fputs("\t{\n", fout);

            for (int j = 0; j < evt.numactions; j++)
            {
                PATHACTION action = { 0 };
                PATHACT* act = &(action.act);
                fread(&action, sizeof(PATHACTION), 1, f);

                //int8_t track = action.track & 0xFF;
                //if (track > 0)
                //    track--;
                auto sectionID = action.sectionID;

                fprintf(fout, "(0x%X, %d)", action.track, sectionID);

                if ((action.type == PATHACTION_CONDITION) && (action.assess > PATHASSESS_IF))
                    indent -= 1;

                for (int k = 0; k < indent; k++)
                    fputc('\t', fout); 


                switch (action.type)
                {
                case PATHACTION_CONDITION:
                    if ((action.assess == PATHASSESS_IF) || (action.assess == PATHASSESS_ELIF))
                    {
                        fprintf(fout, "\t\t%s %s %s %s\n",
                            paAssessments[action.assess],
                            actValToString((*act).only.value, action.leftvaluetype),
                            paSigns[action.comparison],
                            actValToString((*act).only.compareValue, action.rightvaluetype));
                    }
                    else
                        fprintf(fout, "\t\t%s\n", paAssessments[action.assess]);
                    break;
                case PATHACTION_WAITTIME:
                    fprintf(fout, "\t\tWait %s\n", actValToString((*act).waittime.millisecs, action.rightvaluetype));
                    break;
                case PATHACTION_WAITBEAT:
                    fprintf(fout, "\t\tWaitBeat %s (%d, %d, %d)\n", actValToString((*act).waitbeat.millisecs, action.rightvaluetype), (*act).waitbeat.every, (*act).waitbeat.note, (*act).waitbeat.offset);
                    break;
                case PATHACTION_BRANCHTO:
                    fprintf(fout, "\t\tBranch %s (%d, %d)\n", actValToString((*act).branch.node, action.leftvaluetype), (*act).branch.ofsection, (*act).branch.immediate);
                    //if ((*act).branch.immediate)
                    //    fputs(" Immediate", fout);
                    //fputs("\n", fout);
                    break;
                case PATHACTION_FADE:
                    fprintf(fout, "\t\tFade %s (%d, %d, %s)\n", actFadeValToString((*act).fade.id), (*act).fade.tovol, (*act).fade.flip, actValToString((*act).fade.ms, action.rightvaluetype)); // TODO: can fade use vars as input params? if so, WHAT exactly? ms or tovol? ASSUMING 'ms' since it has "right" type set and "left" is unset
                    break;
                case PATHACTION_DRYLEVELFADE:
                    fprintf(fout, "\t\tDryFade %s (%d, %d, %s)\n", actFadeValToString((*act).dryfade.id), (*act).dryfade.tovol, (*act).dryfade.flip, actValToString((*act).dryfade.ms, action.rightvaluetype));
                    break;
                case PATHACTION_SFXFADE:
                    fprintf(fout, "\t\tSFXFade %s (%d, %d, %s)\n", actFadeValToString((*act).sfxfade.id), (*act).sfxfade.tovol, (*act).sfxfade.flip, actValToString((*act).sfxfade.ms, action.rightvaluetype));
                    break;
                case PATHACTION_PITCHFADE:
                    fprintf(fout, "\t\tPitchFade %s (%d, %d, %s)\n", actFadeValToString((*act).pitchfade.id), (*act).pitchfade.tovol, (*act).pitchfade.flip, actValToString((*act).pitchfade.ms, action.rightvaluetype));
                    break;
                case PATHACTION_STRETCHFADE:
                    fprintf(fout, "\t\tStretchFade %s (%d, %d, %s)\n", actFadeValToString((*act).stretchfade.id), (*act).stretchfade.tovol, (*act).stretchfade.flip, actValToString((*act).stretchfade.ms, action.rightvaluetype));
                    break;
                case PATHACTION_SETVALUE:
                    fprintf(fout, "\t\tSet %s = %s\n", actValToString((*act).setval.setwhat, action.leftvaluetype), actValToString((*act).setval.towhat, action.rightvaluetype));
                    break;
                case PATHACTION_EVENT:
                    fprintf(fout, "\t\tEvent 0x%08X\n", (*act).event.eventid);
                    break;
                case PATHACTION_FILTER_ON:
                    fputs("\t\tFilter ON\n", fout);
                    break;
                case PATHACTION_FILTER_OFF:
                    fputs("\t\tFilter OFF\n", fout);
                    break;
                case PATHACTION_FILTER_CLEAR:
                    fputs("\t\tFilter CLEAR\n", fout);
                    break;
                case PATHACTION_CALLBACK:
                    fprintf(fout, "\t\tCallback %s (%s)\n", actValToString((*act).callback.value, action.leftvaluetype), actValToString((*act).callback.id, action.rightvaluetype));
                    break;
                case PATHACTION_CALC:
                    fprintf(fout, "\t\tCalc %s %s %s\n", actValToString((*act).calc.value, action.leftvaluetype), paCalcSigns[(*act).calc.op], actValToString((*act).calc.by, action.rightvaluetype));
                    break;
                case PATHACTION_PAUSE:
                    fprintf(fout, "\t\tPause %s (%s)\n", actValToString((*act).pause.when, action.leftvaluetype), actValToString((*act).pause.on, action.rightvaluetype));
                    break;
                case PATHACTION_LOADBANK:
                    fprintf(fout, "\t\tLoadBank %s (%s)\n", actValToString((*act).loadbank.subbanknum, action.leftvaluetype), actValToString((*act).loadbank.unload, action.rightvaluetype));
                    break;
                default:
                    fprintf(fout, "\t\tAction %d\n", action.type);
                    break;
                }

                if ((action.type == PATHACTION_CONDITION) && (action.assess < PATHASSESS_ENDIF))
                    indent += 1;

            }

            fputs("\t}\n", fout);
        }

        fputs("}\n", fout);
    }

    fclose(fout);
    fclose(f);
    return 0;
}

int MPF_ExtractSamples(char* mpffilename, char* mustrackfilename, char* outparam, int index)
{
    FILE* f = fopen(mpffilename, "rb");
    if (!f)
    {
        cout << "Can't open file " << mpffilename << " for reading: " << strerror(errno) << '\n';
        return -1;
    }

    fread(&hdr, sizeof(PATHFINDHEADER), 1, f);

    if (!MPF_ValidateHeader(&hdr))
    {
        fclose(f);
        return -1;
    }

    FILE* fmus = fopen(mustrackfilename, "rb");
    if (!fmus)
    {
        cout << "Can't open file " << mustrackfilename << " for reading: " << strerror(errno) << '\n';
        fclose(f);
        return -1;
    }

    FILE* fsamp = NULL;
    void* filebuf = NULL;
    const char* outfolder = ".";

    struct stat st = { 0 };

    // get the track infos to know what we're checking
    cout << "#--------\n";
    cout << "# Tracks\n";
    cout << "#--------\n";

    fseek(f, hdr.trackoffsets, SEEK_SET);
    for (int i = 0; ftell(f) < hdr.trackinfos; i++)
    {
        uint32_t offset = 0;
        fread(&offset, sizeof(uint32_t), 1, f);
        offset *= 4;
        trackinfoOffsets.push_back(offset);
    }
    for (int i = 0; i < trackinfoOffsets.size(); i++)
    {
        PATHTRACKINFO trackinfo = { 0 };
        fseek(f, trackinfoOffsets.at(i), SEEK_SET);
        fread(&trackinfo, sizeof(PATHTRACKINFO), 1, f);
        trackinfos.push_back(trackinfo);

        cout << "Track " << i << '\n';
        cout << "{\n";
        cout << "\tStartingSample " << std::dec << trackinfo.startingsample << '\n';
        cout << "\tNumSubBanks " << std::dec << trackinfo.numsubbanks << '\n';
        cout << "\tPurgeMode " << std::dec << trackinfo.purgemode << '\n';
        cout << "\tMusChecksum 0x" << std::uppercase << std::hex << trackinfo.muschecksum << '\n';
        cout << "\tMaxARAM " << std::dec << trackinfo.maxaram << '\n';
        cout << "\tMaxMRAM " << std::dec << trackinfo.maxmram << '\n';
        cout << "}\n";
    }

    // check the mus checksum to know which track we're on and if it's valid
    uint32_t muschecksum = 0;
    int track_index = -1;

    fread(&muschecksum, sizeof(uint32_t), 1, fmus);
    for (int i = 0; i < trackinfos.size(); i++)
    {
        if (trackinfos.at(i).muschecksum == muschecksum)
            track_index = i;
    }

    if (track_index < 0)
    {
        cout << "Can't find track " << mustrackfilename << " in MPF " << mpffilename << '\n';
        cout << "Checksum mismatch! Read checksum: 0x" << std::uppercase << std::hex << muschecksum << '\n';
        if (f)
            fclose(f);
        if (fmus)
            fclose(fmus);
        return -1;
    }


    // extract the sample files themselves
    // the sample offset list is always at the end of MPF (it has to be because it has no size descriptor)
    
    if (index == -1)
    {
        // check for folder existence, if it doesn't exist, make it
        if (stat(outparam, &st))
        {
            cout << "Creating folder: " << outparam << '\n';
#if defined(_WIN32)
            mbstowcs(MkDirPath, outparam, 1024);
            _wmkdir(MkDirPath);
#else
            mkdir(outparam, 0770);
#endif
        }
        outfolder = outparam;
    }

    fseek(f, hdr.sampleoffsets, SEEK_SET);
    while (!feof(f))
    {
        PATHFINDSAMPLE sample = { 0 };
        fread(&sample, sizeof(PATHFINDSAMPLE), 1, f);
        uint32_t offset = sample.offset * 0x80;
        sampleOffsets.push_back(offset);
    }
    sampleOffsets.pop_back();


    int starting_sample = trackinfos.at(track_index).startingsample;
    int last_sample = sampleOffsets.size();
    if (track_index != trackinfos.size() - 1)
        last_sample = trackinfos.at(track_index + 1).startingsample;

    stat(mustrackfilename, &st);

    // sample sizes have to be in sequential order for this to work
    for (int i = starting_sample; i < last_sample; i++)
    {
        fseek(fmus, sampleOffsets[i], SEEK_SET);

        if ((index == i + 1) || index == -1)
        {
            uint32_t size = 0;
            uint32_t magic = 0;

            // get file extension
            fread(&magic, sizeof(uint32_t), 1, fmus);
            SampleFileType ftype = GetSampleFileType(magic);
            const char* file_ext = SampleFileTypeStr[ftype];

            if ((ftype == sftEAL3) && !bEALayer3Mode)
            {
                cout << "Going into EALayer3 mode!\n";
                bEALayer3Mode = true;

                // get sample sizes from the header instead
                uint32_t hdrcount = 0;
                MUSHeaderInfo hi = { 0 };
                fseek(fmus, 4, SEEK_SET);
                fread(&hdrcount, sizeof(uint32_t), 1, fmus);
                fseek(fmus, 0x28, SEEK_SET);

                for (int j = 0; j < hdrcount; j++)
                {
                    fread(&hi, sizeof(MUSHeaderInfo), 1, fmus);
                    sampleSizes.push_back(hi.DataSize);
                }
            }

            fseek(fmus, sampleOffsets[i], SEEK_SET);

            if (bEALayer3Mode)
            {
                size = sampleSizes.at(i);
            }
            else
            {
                if (i == last_sample - 1)
                    size = st.st_size - sampleOffsets.at(i);
                else
                    size = sampleOffsets.at(i + 1) - sampleOffsets.at(i);
            }

            // open the new file
            if (index == -1)
                sprintf(SampleFilePath, "%s%s%d%s", outfolder, path_separator, i + 1, file_ext);
            else
            {
                if (outparam == NULL)
                {
                    strcpy(SampleFilePath, mpffilename);
                    char* pathpoint = strrchr(SampleFilePath, path_separator_char);
                    if (pathpoint)
                        sprintf(pathpoint, "%s%d%s", path_separator, i + 1, file_ext);
                    else
                        sprintf(SampleFilePath, "%d%s", i + 1, file_ext);
                }
                else
                    strcpy(SampleFilePath, outparam);
            }
            cout << "Extracting: " << SampleFilePath << '\n';
            fsamp = fopen(SampleFilePath, "wb");
            if (!fsamp)
            {
                cout << "Can't open file " << SampleFilePath << " for writing: " << strerror(errno) << '\n';
                if (fmus)
                    fclose(fmus);
                if (f)
                    fclose(f);
                return -1;
            }
            
            filebuf = malloc(size);
            if (filebuf)
            {
                fread(filebuf, size, 1, fmus);
                fwrite(filebuf, size, 1, fsamp);
                if (fsamp)
                    fclose(fsamp);


                free(filebuf);
            }
            else
            {
                return -1;
            }
        }
    }

    // if we're in EALayer3 mode, we need the headers extracted too
    if (bEALayer3Mode)
    {
        uint32_t hdrcount = 0;
        MUSHeaderInfo hi = { 0 };
        size_t previous_pos = 0;
        fseek(fmus, 4, SEEK_SET);
        fread(&hdrcount, sizeof(uint32_t), 1, fmus);
        fseek(fmus, 0x28, SEEK_SET);

        for (int i = 0; i < hdrcount; i++)
        {
            if ((index == i + 1) || index == -1)
            {
                fread(&hi, sizeof(MUSHeaderInfo), 1, fmus);
                previous_pos = ftell(fmus);
                uint32_t offset = hi.HeaderOffset * 0x10;
                fseek(fmus, offset, SEEK_SET);

                // open the new file
                if (index == -1)
                    sprintf(SampleFilePath, "%s%s%d%s", outfolder, path_separator, i + 1 + starting_sample, SNR_FILE_EXT);
                else
                {
                    if (outparam == NULL)
                    {
                        strcpy(SampleFilePath, mpffilename);
                        char* pathpoint = strrchr(SampleFilePath, path_separator_char);
                        if (pathpoint)
                            sprintf(pathpoint, "%s%d%s", path_separator, i + 1 + starting_sample, SNR_FILE_EXT);
                        else
                            sprintf(SampleFilePath, "%d%s", i + 1 + starting_sample, SNR_FILE_EXT);
                    }
                    else
                        strcpy(SampleFilePath, outparam);
                }
                cout << "Extracting: " << SampleFilePath << '\n';
                fsamp = fopen(SampleFilePath, "wb");
                if (!fsamp)
                {
                    cout << "Can't open file " << SampleFilePath << " for writing: " << strerror(errno) << '\n';
                    if (fmus)
                        fclose(fmus);
                    if (f)
                        fclose(f);
                    return -1;
                }

                filebuf = malloc(hi.HeaderSize);
                if (filebuf)
                {
                    fread(filebuf, hi.HeaderSize, 1, fmus);
                    fwrite(filebuf, hi.HeaderSize, 1, fsamp);
                    if (fsamp)
                        fclose(fsamp);


                    free(filebuf);
                }
                else
                {
                    return -1;
                }


                fseek(fmus, previous_pos, SEEK_SET);
            }
        }
    }

    if (fmus)
        fclose(fmus);

    if (f)
        fclose(f);
    
    return 0;
}

int GetHighestSampleNumName()
{
    char sampname[32];
    char* cursor;
    int BiggestNum = 0;
    int ReadNum = 0;

    for (int i = 0; i < FileDirectoryListing.size(); i++)
    {
        strcpy(sampname, FileDirectoryListing.at(i).c_str());
        cursor = strchr(sampname, '.');
        if (!cursor)
        {
            cout << "File " << FileDirectoryListing.at(i) << " does not have a valid file extension. Please use only use files with decimal integer names and an extension.\n";
            return -1;
        }
        *cursor = '\0';
        cmpCleanUpToken(sampname);
        if (bStringHasAlpha(sampname))
        {
            cout << "File " << FileDirectoryListing.at(i) << " contains alphabetic characters in its name. Please use only use files with decimal integer names and an extension.\n";
            return -1;
        }

        ReadNum = stoi(sampname);

        if (ReadNum > BiggestNum)
            BiggestNum = ReadNum;
    }

    return BiggestNum;
}

uint32_t read_patch(void* sf, off_t* offset) {
    uint32_t result = 0;
    uint8_t byte_count = *(uint8_t*)((size_t)sf + *offset);
    (*offset)++;

    if (byte_count == 0xFF) { /* signals 32b size (ex. custom user data) */
        (*offset) += 4 + bswap_32(*(uint32_t*)((size_t)sf + *offset));
        return 0;
    }


    if (byte_count > 4) {
        (*offset) += byte_count;
        return 0;
    }

    for (; byte_count > 0; byte_count--) { /* count of 0 is also possible, means value 0 */
        result <<= 8;
        result += *(uint8_t*)((size_t)sf + *offset);
        (*offset)++;
    }

    return result;
}

// code snippets from VGMStream: https://github.com/vgmstream/vgmstream/blob/50a11404e8b6949af6cc6572a99f8327842bf923/src/meta/ea_schl.c#L1557
uint32_t GetSampleMSLength_GSTR(void* in)
{
    void* sf = NULL;
    off_t offset = 0;
    size_t size = *(uint32_t*)((size_t)in + 4);
    bool is_header_end = false;

    // first search for GSTR
    size_t searchoff = size;
    int cursor = 0;
    while (searchoff)
    {
        if (*(uint32_t*)((size_t)in + cursor) == EA_GSTR_MAGIC)
        {
            break;
        }
        searchoff--;
        cursor++;
    }

    sf = (void*)((size_t)in + cursor + 8);

    uint32_t sample_rate = 0;
    uint32_t num_samples = 0;

    bool bFoundRate = 0;
    bool bFoundNum = 0;

    while (!is_header_end && offset < size) {

        if (bFoundRate && bFoundNum)
            break;

        uint8_t patch_type = *(uint8_t*)((size_t)sf + offset);
        offset++;

        switch (patch_type) {
        case 0x00:
            if (!is_header_end)
                read_patch(sf, &offset);
            break;
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x19:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0xFC:
        case 0x83:
        case 0xA0:
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x94:
        case 0x95:
        case 0xA2:
        case 0xA3:
        case 0x8F:
        case 0x90:
        case 0x91:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0x1A:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2a:
        case 0x8C:
        case 0x8A:
        case 0x8B:
        case 0x8D:
        case 0x8E:
        case 0x92:
        case 0x93:
        case 0x98:
        case 0x99:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
        case 0xA6:
        case 0xA7:
        case 0xA1:
            read_patch(sf, &offset);
            break;

        case 0x84:
            sample_rate = read_patch(sf, &offset);
            bFoundRate = true;
            break;

        case 0x85:
            num_samples = read_patch(sf, &offset);
            bFoundNum = true;
            break;

        case 0xFF:
        case 0xFE:
            is_header_end = 1;
            break;

        case 0xFD:
        default:
            break;
        }
    }

    if ((num_samples == 0) || sample_rate == 0)
        return 0;

    //long long unsigned int msLength = (num_samples * 1000) / sample_rate;
    double msLength = (double)num_samples / (double)sample_rate;
    msLength *= 1000.0;
    return (uint32_t)msLength;
}

int MPF_UpdateSamples(char* mpffilename, char* samplefolder)
{
    FILE* f = fopen(mpffilename, "rb");
    if (!f)
    {
        cout << "Can't open file " << mpffilename << " for reading: " << strerror(errno) << '\n';
        return -1;
    }

    // rip the mpf data out and omit the sample offsets
    fread(&hdr, sizeof(PATHFINDHEADER), 1, f);
    fseek(f, 0, SEEK_SET);
    void* mpfdata = malloc(hdr.sampleoffsets);
    fread(mpfdata, hdr.sampleoffsets, 1, f);

    // get the track infos
    fseek(f, hdr.trackoffsets, SEEK_SET);
    for (int i = 0; ftell(f) < hdr.trackinfos; i++)
    {
        uint32_t offset = 0;
        fread(&offset, sizeof(uint32_t), 1, f);
        offset *= 4;
        trackinfoOffsets.push_back(offset);
    }
    for (int i = 0; i < trackinfoOffsets.size(); i++)
    {
        PATHTRACKINFO trackinfo = { 0 };
        fseek(f, trackinfoOffsets.at(i), SEEK_SET);
        fread(&trackinfo, sizeof(PATHTRACKINFO), 1, f);
        trackinfos.push_back(trackinfo);
    }
    fclose(f);

    // parse the folder
    GetDirectoryListing(samplefolder);

    int LastSampleFile = GetHighestSampleNumName();
    if (LastSampleFile < 0)
        return -1;

    // sanity check before proceeding with repacking
    for (int t = 0; t < trackinfos.size(); t++)
    {
        if (LastSampleFile < trackinfos.at(t).startingsample)
        {
            cout << "ERROR: Can't update samples for track " << t << '\n';
            cout << "Last sample file is lower than the starting sample! Check if all files are in the folder.\n";
            return -1;
        }
    }

    for (int t = 0; t < trackinfos.size(); t++)
    {
        int starting_sample = trackinfos.at(t).startingsample;
        int last_sample = LastSampleFile;
        EAL3HeaderInfos.clear();

        if (t != trackinfos.size() - 1)
            last_sample = trackinfos.at(t + 1).startingsample;
        int sample_count = last_sample - starting_sample;
        int hc = 0;

        strcpy(MusFilePath, mpffilename);
        char* extpoint = strrchr(MusFilePath, '.');
        char numext[16];
        if (trackinfos.size() > 1)
        {
            sprintf(numext, "_%d.mus", t);
            if (extpoint)
                strcpy(extpoint, numext);
            else
                strcat(MusFilePath, numext);
        }
        else
        {
            if (extpoint)
                strcpy(extpoint, ".mus");
            else
                strcat(MusFilePath, ".mus");
        }
    
        FILE* fmus = fopen(MusFilePath, "wb");
        if (!fmus)
        {
            cout << "Can't open file " << MusFilePath << " for writing: " << strerror(errno) << '\n';
            return -1;
        }
    
        FILE* fsamp = NULL;
        char sampname[32];
        void* sampdata;
        struct stat st = { 0 };

        // write the checksum
        fwrite(&(trackinfos.at(t).muschecksum), sizeof(uint32_t), 1, fmus);
    
        // space for header infos
        if (bEALayer3Mode)
        {
            fseek(fmus, 4, SEEK_SET);
            fwrite(&sample_count, sizeof(uint32_t), 1, fmus);
            fseek(fmus, 0x28, SEEK_SET);
    
            size_t headers_size = sizeof(MUSHeaderInfo) * sample_count + 0x28;
            headers_size = (headers_size + 0x100) - (headers_size % 0x100);
            fseek(fmus, headers_size, SEEK_SET);
    
            for (int i = starting_sample; i < last_sample; i++)
            {
                MUSHeaderInfo hi = { 0 };
                hi.index = hc;
                hi.hash = 0xDEADBEEF;
    
                // align by 0x10 bytes
                size_t mus_offset = ftell(fmus);
                if (mus_offset % 0x10)
                    mus_offset = mus_offset - (mus_offset % 0x10) + 0x10;
    
                hi.HeaderOffset = mus_offset / 0x10;
                fseek(fmus, mus_offset, SEEK_SET);
    
                sprintf(sampname, "%s%s%d%s", samplefolder, path_separator, i + 1, SNR_FILE_EXT);
    
                cout << "Reading: " << sampname << '\n';
                fsamp = fopen(sampname, "rb");
                if (!fsamp)
                {
                    cout << "Can't open file " << sampname << " for reading: " << strerror(errno) << '\n';
                    if (fmus)
                        fclose(fmus);
                    return -1;
                }
    
                stat(sampname, &st);
                hi.HeaderSize = st.st_size;
    
                EAL3HeaderInfos.push_back(hi);
    
                sampdata = malloc(st.st_size);
                fread(sampdata, st.st_size, 1, fsamp);
                fwrite(sampdata, st.st_size, 1, fmus);
    
                // while here, get the sample length
                // credit: VGMStream: https://github.com/vgmstream/vgmstream/blob/master/src/meta/ea_eaac.c#L1155
                uint32_t header1 = bswap_32(*(uint32_t*)(sampdata));
                uint32_t header2 = bswap_32(*(uint32_t*)((size_t)sampdata + sizeof(uint32_t)));
    
                uint32_t sample_rate = (header1 >> 0) & 0x03FFFF;
                uint32_t num_samples = (header2 >> 0) & 0x1FFFFFFF;
                double length_ms = (double)num_samples / (double)sample_rate;
                length_ms *= 1000.0;
                sampleLengths.push_back((uint32_t)length_ms);
    
                free(sampdata);
    
                fclose(fsamp);
                hc++;
            }
        }

        if (bEALayer3Mode)
        {
            // get the aram size
            trackinfos.at(t).maxaram = ftell(fmus);
            // align by 0x10 bytes
            if (trackinfos.at(t).maxaram % 0x10)
                trackinfos.at(t).maxaram = trackinfos.at(t).maxaram - (trackinfos.at(t).maxaram % 0x10) + 0x10;
        }

        for (int i = starting_sample; i < last_sample; i++)
        {
            // align by 0x80 bytes
            size_t mus_offset = ftell(fmus);
            if (mus_offset % 0x80)
                mus_offset = (mus_offset + 0x80) - (mus_offset % 0x80);
            fseek(fmus, mus_offset, SEEK_SET);
    
            sampleOffsets.push_back(mus_offset / 0x80);
            if (bEALayer3Mode)
            {
                sprintf(sampname, "%s%s%d%s", samplefolder, path_separator, i + 1, SNS_FILE_EXT);
                EAL3HeaderInfos.at(i - starting_sample).DataOffset = mus_offset / 0x80;
            }
            else
                sprintf(sampname, "%s%s%d%s", samplefolder, path_separator, i + 1, ASF_FILE_EXT);
    
            cout << "Reading: " << sampname << '\n';
    
            fsamp = fopen(sampname, "rb");
            if (!fsamp)
            {
                cout << "Can't open file " << sampname << " for reading: " << strerror(errno) << '\n';
                if (fmus)
                    fclose(fmus);
                return -1;
            }
            stat(sampname, &st);
    
            if (bEALayer3Mode)
                EAL3HeaderInfos.at(i - starting_sample).DataSize = st.st_size;
    
            sampdata = malloc(st.st_size);
            fread(sampdata, st.st_size, 1, fsamp);
            fwrite(sampdata, st.st_size, 1, fmus);
    
            // get sample length for GSTR
            uint32_t magic = *(uint32_t*)sampdata;
            if ((magic == EA_SCHL_MAGIC) && !bEALayer3Mode)
            {
                sampleLengths.push_back(GetSampleMSLength_GSTR(sampdata));
            }
    
            free(sampdata);
    
            fclose(fsamp);
        }
    
        // go back and write the infos...
        if (bEALayer3Mode)
        {
            fseek(fmus, 0x28, SEEK_SET);
            for (int i = 0; i < hc; i++)
            {
                fwrite(&(EAL3HeaderInfos.at(i)), sizeof(MUSHeaderInfo), 1, fmus);
            }
        }
    
        fclose(fmus);
    }

    // re-generate MPF with new offsets
    cout << "Updating: " << mpffilename << '\n';
    f = fopen(mpffilename, "wb");
    if (!f)
    {
        cout << "Can't open file " << mpffilename << " for writing: " << strerror(errno) << '\n';
        return -1;
    }

    fwrite(mpfdata, hdr.sampleoffsets, 1, f);


    for (int i = 0; i < LastSampleFile; i++)
    {
        PATHFINDSAMPLE sample = { 0 };
        sample.offset = sampleOffsets.at(i);
        sample.duration = sampleLengths.at(i);
        fwrite(&(sample), sizeof(PATHFINDSAMPLE), 1, f);
    }

    // update the map file size
    uint32_t mapsize = ftell(f);
    fseek(f, 0x38, SEEK_SET);
    fwrite(&mapsize, sizeof(uint32_t), 1, f);

    if (bEALayer3Mode)
    {
        // update aram sizes
        for (int i = 0; i < trackinfoOffsets.size(); i++)
        {
            fseek(f, trackinfoOffsets.at(i), SEEK_SET);
            fwrite(&(trackinfos.at(i)), sizeof(PATHTRACKINFO), 1, f);
        }
    }

    free(mpfdata);
    fclose(f);
    return 0;
}

// compiler stuff start

cmpMainTokenType CompilerGetMainTokenType(char* str)
{
    string ln(str);
    for (int i = 0; i < cmpMainTokenMAX; i++)
    {
        if (ln.find(cmpMainTokenTypeStr[i]) != string::npos)
            return (cmpMainTokenType)i;
    }

    return cmpNULL;
}

char* chrToUpper(char* str)
{
    for (int i = 0; i < strlen(str); i++)
        str[i] = toupper(str[i]);
    return str;
}

int cmpParseVAR(char* line)
{
    if (strchr(line, '='))
    {
        char parseLine[512];
        strcpy(parseLine, line);

        char* token = strtok(parseLine, "=");
        
        int tokcounter = 0;
        size_t toklen = 0;

        while ((token != NULL) && tokcounter < 2)
        {
            token = cmpCleanUpToken(token);

            if (!tokcounter)
            {
                toklen = strlen(token);
                if (toklen > 16)
                    toklen = 16;
                memset(cmpNamedVars.back().name, 0, 16);
                memcpy(cmpNamedVars.back().name, token, toklen);
            }
            else
            {
                if (bStringHasAlpha(token))
                {
                    cout << "ERROR: can't parse var, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                cmpNamedVars.back().value = stoi(token);
            }

            token = strtok(NULL, "=");
            tokcounter++;
        }
        return 1;
    }

    return 0;
}

int cmpParseROUTER(char* line)
{
    string ln(line);
    if (ln.find(">") != string::npos)
    {
        char parseLine[512];
        strcpy(parseLine, line);

        char* token = strtok(parseLine, ">");

        int tokcounter = 0;
        ROUTERpair rpair = { 0 };

        while ((token != NULL) && tokcounter < 2)
        {
            token = cmpCleanUpToken(token);

            if (!tokcounter)
            {
                if (bStringHasAlpha(token))
                {
                    cout << "ERROR: can't parse router, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                rpair.node1 = stoi(token);
            }
            else
            {
                if (bStringHasAlpha(token))
                {
                    cout << "ERROR: can't parse router, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                rpair.node2 = stoi(token);
            }

            token = strtok(NULL, ">");
            tokcounter++;
        }

        cmpRouters.back().rp.push_back(rpair);

        return 1;
    }
    return 0;
}

cmpTrackInfoParser cmpGetTrackInfoParserType(char* line)
{
    string ln(line);

    for (int i = 0; i < cmpTIP_MAX; i++)
    {
        if (ln.find(cmpTrackInfoStrings[i]) != string::npos)
        {
            return (cmpTrackInfoParser)i;
        }
    }

    return cmpTIP_NULL;
}

int cmpParseTRACK(char* line)
{
    cmpTrackInfoParser type = cmpGetTrackInfoParserType(line);

    if (type)
    {
        char parseLine[512];
        strcpy(parseLine, line);

        char* token = strtok(parseLine, " ");
        char* bs16tok = NULL;

        int tokcounter = 0;
        uint32_t val = 0;

        while ((token != NULL) && tokcounter < 2)
        {
            token = cmpCleanUpToken(token);

            if (tokcounter)
            {
                if (bStringHasAlpha(token) && (type != cmpTIP_MusChecksum))
                {
                    cout << "ERROR: can't parse track var, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }

                switch (type)
                {
                case cmpTIP_StartingSample:
                    cmpTrackInfos.back().track.startingsample = stoi(token);
                    break;
                case cmpTIP_SubBanks:
                    cmpTrackInfos.back().track.numsubbanks = stoi(token);
                    break;
                case cmpTIP_MusChecksum:
                    bs16tok = token;
                    if ((token[0] == '0') && (token[1] == 'x'))
                        bs16tok += 2;
                    chrToUpper(bs16tok);
                    sscanf(bs16tok, "%X", &val); // stoi is not being friendly, using sscanf here
                    cmpTrackInfos.back().track.muschecksum = val;
                    break;
                case cmpTIP_MaxARAM:
                    cmpTrackInfos.back().track.maxaram = stoi(token);
                    break;
                case cmpTIP_MaxMRAM:
                    cmpTrackInfos.back().track.maxmram = stoi(token);
                    break;
                default:
                    break;
                }
            }

            token = strtok(NULL, " ");
            tokcounter++;
        }
        return 1;
    }

    return 0;
}

cmpPathNodeParser cmpGetPathNodeParserType(char* line)
{
    string ln(line);

    for (int i = 0; i < cmpPNP_MAX; i++)
    {
        if (ln.find(cmpPathNodeStrings[i]) != string::npos)
        {
            return (cmpPathNodeParser)i;
        }
    }

    return cmpPNP_NULL;
}

int cmpParseNODE(char* line)
{
    cmpPathNodeParser type = cmpGetPathNodeParserType(line);

    if (type)
    {
        char parseLine[64];
        strcpy(parseLine, line);

        char* token = strtok(parseLine, " ");

        int tokcounter = 0;
        int32_t eventID = 0;

        while ((token != NULL) && tokcounter < 2)
        {
            token = cmpCleanUpToken(token);

            if (tokcounter)
            {
                if (bStringHasAlpha(token) && (type != cmpPNP_Event))
                {
                    cout << "ERROR: can't parse node var, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }

                switch (type)
                {
                case cmpPNP_Wave:
                    cmpNodes.back().node.index = stoi(token);
                    break;
                case cmpPNP_Track:
                    cmpNodes.back().node.trackID = stoi(token);
                    break;
                case cmpPNP_Section:
                    cmpNodes.back().node.sectionID = stoi(token);
                    break;
                case cmpPNP_Part:
                    cmpNodes.back().node.partID = stoi(token);
                    break;
                case cmpPNP_Router:
                    cmpNodes.back().node.routerID = stoi(token) + 1;
                    break;
                case cmpPNP_Controller:
                    cmpNodes.back().node.controller = stoi(token);
                    break;
                case cmpPNP_Beats:
                    cmpNodes.back().node.beats = stoi(token);
                    break;
                case cmpPNP_Bars:
                    cmpNodes.back().node.bars = stoi(token);
                    break;
                case cmpPNP_SynchEvery:
                    cmpNodes.back().node.synchevery = stoi(token);
                    break;
                case cmpPNP_SynchOffset:
                    cmpNodes.back().node.synchoffset = stoi(token);
                    break;
                case cmpPNP_Notes:
                    cmpNodes.back().node.notes = stoi(token);
                    break;
                case cmpPNP_Synch:
                    cmpNodes.back().node.synch = stoi(token);
                    break;
                case cmpPNP_ChannelBranching:
                    cmpNodes.back().node.channelbranching = stoi(token);
                    break;
                case cmpPNP_Repeat:
                    cmpNodes.back().node.repeat = stoi(token);
                    break;
                case cmpPNP_Event:
                    if ((token[0] != '0') && (token[1] != 'x'))
                    {
                        cout << "ERROR: can't parse node sendevent, unknown token (expected hexadecimal number): [" << token << "] at line " << cmpCurLine << '\n';
                        return -1;
                    }
                    chrToUpper(token + 2);
                    sscanf(token + 2, "%X", &eventID);
                    cmpNodes.back().sendevent.eventID = eventID;
                    break;
                default:
                    break;
                }
            }
            else
            {
                if (type == cmpPNP_Branches)
                {
                    cmpCurrentPSSS = cmpPSS_EnteredBody;
                    return 1;
                }
            }

            token = strtok(NULL, " ");
            tokcounter++;
        }
        return 1;
    }

    return 0;
}

int cmpParseNODEBRANCH(char* line)
{
    string ln(line);
    if (ln.find("Control") != string::npos)
    {
        char parseLine[64];
        strcpy(parseLine, line);

        char* token = strtok(parseLine, ",");
        int tokcounter = 0;
        char* cursor;

        PATHFINDBRANCH newbranch = { 0 };
        cmpNodes.back().branches.push_back(newbranch);
        cmpNodes.back().node.numbranches = cmpNodes.back().branches.size();

        while ((token != NULL) && tokcounter < 2)
        {
            if (!tokcounter)
            {
                char* tok_trim = strrchr(token, ' ');
                if (!tok_trim)
                {
                    cout << "ERROR: can't parse node branch, found unexpected token: [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }

                tok_trim += 1;
                int value = stoi(tok_trim);
                if (value < -127)
                    value = -127;
                if (value > 127)
                    value = 127;
                cmpNodes.back().branches.back().controlmin = (int8_t)value;
            }
            else
            {
                cursor = strchr(token, '>');
                if (!cursor)
                {
                    cout << "ERROR: can't parse node branch var, missing separator arrow '>': [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                *cursor = '\0';
                token = cmpCleanUpToken(token);
                if (bStringHasAlpha(token))
                {
                    cout << "ERROR: can't parse node branch var, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                int value = stoi(token);
                if (value < -127)
                    value = -127;
                if (value > 127)
                    value = 127;
                cmpNodes.back().branches.back().controlmax = (int8_t)value;
            }

            token = strtok(NULL, ",");
            tokcounter++;
        }



        strcpy(parseLine, line);
        token = strtok(parseLine, ">");
        tokcounter = 0;

        while ((token != NULL) && tokcounter < 2)
        {
            if (tokcounter)
            {
                token = cmpCleanUpToken(token);
                if (bStringHasAlpha(token))
                {
                    cout << "ERROR: can't parse node branch var, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                cmpNodes.back().branches.back().dstnode = (uint16_t)stoi(token);
            }

            token = strtok(NULL, ">");
            tokcounter++;
        }

        return 1;
    }

    return 0;
}

cmpEventParser cmpGetEventParserType(char* line)
{
    string ln(line);

    for (int i = 0; i < cmpEP_MAX; i++)
    {
        if (ln.find(cmpEventStrings[i]) != string::npos)
        {
            return (cmpEventParser)i;
        }
    }

    return cmpEP_NULL;
}

int cmpParseEVENT(char* line)
{
    cmpEventParser type = cmpGetEventParserType(line);

    if (type)
    {
        char parseLine[512];
        strcpy(parseLine, line);

        char* token = strtok(parseLine, " ");

        int tokcounter = 0;

        while ((token != NULL) && tokcounter < 2)
        {
            token = cmpCleanUpToken(token);

            if (!tokcounter)
            {
                if (type == cmpEP_Actions)
                {
                    cmpCurrentPSSS = cmpPSS_EnteredBody;
                    return 1;
                }
            }

            token = strtok(NULL, " ");
            tokcounter++;
        }
        return 1;
    }

    return 0;
}

PATHASSESS cmpGetActionAssessmentType(char* str)
{
    string ln(str);

    for (int i = 0; i < PATHASSESS_MAX; i++)
    {
        if (ln.find(paAssessments[i]) != string::npos)
        {
            return (PATHASSESS)i;
        }
    }

    return PATHASSESS_NONE;
}

PATHACTIONTYPE cmpGetActionParserType(char* line)
{
    string ln(line);

    for (int i = 0; i < PATHACTION_MAX; i++)
    {
        if (ln.find(cmpActionStrings[i]) != string::npos)
        {
            return (PATHACTIONTYPE)i;
        }

        if (cmpGetActionAssessmentType(line))
            return PATHACTION_CONDITION;
    }

    return PATHACTION_NONE;
}

PATHSPECIALVALUETYPE cmpGetActionSpecialValueType(char* str)
{
    string ln(str);

    for (int i = 0; i < PATH_SPECIALVALUE_MAX; i++)
    {
        if (ln.find(specialTypes[i]) != string::npos)
        {
            return (PATHSPECIALVALUETYPE)i;
        }
    }

    return PATH_SPECIALVALUE_BAD;
}

PATHFADEWHAT cmpGetActionFadeWhatType(char* str)
{
    string ln(str);

    for (int i = 0; i < PATH_FADE_MAXTYPES; i++)
    {
        if (ln.find(fadeTypes[i]) != string::npos)
        {
            return (PATHFADEWHAT)i;
        }
    }

    return PATH_FADE_INVALID;
}

PATHOPERATOR cmpGetActionOperatorType(char* str)
{
    string ln(str);

    for (int i = 0; i < PATH_OPERATOR_MAX; i++)
    {
        if (ln.find(paCalcSigns[i]) != string::npos)
        {
            return (PATHOPERATOR)i;
        }
    }

    return PATH_OPERATOR_INVALID;
}

PATHCOMPARE cmpGetActionComparatorType(char* str)
{
    for (int i = 0; i < PATH_COMPARE_MAX; i++)
    {
        if (strncmp(str, paSigns[i], 2) == 0)
        {
            return (PATHCOMPARE)i;
        }
    }

    return PATH_COMPARE_INVALID;
}

char* cmpGetActionComparatorPos(char* str)
{
    char* result = NULL;
    for (int i = 0; i < PATH_COMPARE_MAX; i++)
    {
        result = strstr(str, paSigns[i]);
        if (result)
            return result;
    }
    return result;
}

char* cmpGetActionOperatorPos(char* str)
{
    char* result = NULL;
    for (int i = 0; i < PATH_OPERATOR_MAX; i++)
    {
        result = strstr(str, paCalcSigns[i]);
        if (result)
            return result;
    }
    return result;
}

int cmpGetNamedVarIndex(char* var)
{
    string ln(var);
    char varname[16];
    for (int i = 0; i < cmpNamedVars.size(); i++)
    {
        memset(varname, 0, 16);
        memcpy(varname, cmpNamedVars.at(i).name, 16);
        if (ln.find(varname) != string::npos)
        {
            return i;
        }
    }
    return 0;
}

PATHVALUETYPE cmpGetActionValueType(char* str, bool* hex)
{
    string ln(str);

    if (!bStringHasAlpha(str))
        return PATH_VALUE_INTEGER;
    else if ((str[0] == '0') && (str[1] == 'x'))
    {
        *hex = true;
        return PATH_VALUE_INTEGER;
    }

    if (ln.find("var[") != string::npos)
        return PATH_VALUE_VARIABLE;

    if (cmpGetActionSpecialValueType(str))
        return PATH_VALUE_SPECIAL;

    return PATH_VALUE_BADTYPE;
}

int cmpPathActionSetValue(PATHACTION* act, int32_t* valPointer, bool bRight, char* param)
{
    bool bHex = false;
    PATHVALUETYPE vt = cmpGetActionValueType(param, &bHex);
    char locparam[512];
    strcpy(locparam, param);

    char* cursor = NULL;

    if (vt == PATH_VALUE_BADTYPE)
    {
        cout << "ERROR: bad value type provided: [" << param << "] at line " << cmpCurLine << '\n';
        return -1;
    }

    if (bRight)
        (*act).rightvaluetype = vt;
    else
        (*act).leftvaluetype = vt;

    switch (vt)
    {
    case PATH_VALUE_INTEGER:
        if (bHex)
        {
            int32_t val = 0;
            sscanf(param + 2, "%X", &val);
            *valPointer = val;
        }
        else
            *valPointer = stoi(param);
        break;
    case PATH_VALUE_VARIABLE:
        cursor = strchr(locparam, ']');
        if (!cursor)
        {
            cout << "ERROR: can't parse variable name: [" << param << "] - incorrect formatting, missing closing bracket, at line " << cmpCurLine << '\n';
            return -1;
        }
        *cursor = 0;
        cursor = strchr(locparam, '[') + 1;
        *valPointer = cmpGetNamedVarIndex(cursor);
        break;
    case PATH_VALUE_SPECIAL:
        *valPointer = cmpGetActionSpecialValueType(param);
        break;
    }

    return 0;
}

int cmpParseACTION(char* input_line)
{
    PATHACTIONTYPE type = cmpGetActionParserType(input_line);
    char* line = input_line;
    char param1[32] = {};
    char param2[32] = {};
    char param3[32] = {};
    char param4[32] = {};

    char* cursor = NULL;
    char* cursor2 = NULL;
    char* cursor3 = NULL;

    size_t param_len = 0;
    size_t param_len2 = 0;

    int32_t value = 0;


    if (type)
    {
        PATHACTION act = { 0 };
        const char* actnamestr = cmpActionStrings[type];

        act.type = type;

        // get the track and sectionID
        // get the track param
        cursor = strchr(line, '(') + 1;
        if (!cursor)
        {
            cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting bracket for track and sectionID, at line " << cmpCurLine << '\n';
            return -1;
        }
        cursor2 = strchr(line, ',');
        if (!cursor2)
        {
            cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing comma for track and sectionID, at line " << cmpCurLine << '\n';
            return -1;
        }
        strncpy(param1, cursor, cursor2 - cursor);
        cmpCleanUpToken(param1);
        // get the sectionID param
        cursor = strchr(line, ',') + 1;
        cursor2 = strchr(line, ')');
        if (!cursor2)
        {
            cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing closing bracket for track and sectionID, at line " << cmpCurLine << '\n';
            return -1;
        }
        strncpy(param2, cursor, cursor2 - cursor);
        cmpCleanUpToken(param2);
        
        std::string hexCheckStr = param1;
        if ((param1[1] == 'x') || (param1[1] == 'X'))
            hexCheckStr = param1 + 2;

        if (!all_of(hexCheckStr.begin(), hexCheckStr.end(), ::isxdigit))
        {
            cout << "ERROR: can't parse " << actnamestr << " action - track ID not in hexadecimal format, at line " << cmpCurLine << '\n';
            return -1;
        }

        // apply the values

        act.track = stoi(param1, nullptr, 16);
        act.sectionID = stoi(param2);

        if (act.track == 0)
        {
            act.track++;
            act.track |= 0x11000000; // TODO - figure this out fully
        }
        // shift the line pointer to the end of the track and sectionID token
        line = strchr(line, ')') + 1;
        // clean it up
        cmpCleanUpToken(line);

        memset(param1, 0, 32);
        memset(param2, 0, 32);

        switch (type)
        {
        case PATHACTION_CONDITION:
            // get the condition assessment param
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                // apply the assessment type immediately
                act.assess = cmpGetActionAssessmentType(line);
                if (act.assess == PATHASSESS_ENDIF) // if it's an ENDIF, stop parsing immediately, we have nothing more to parse!
                {
                    act.track = 0xFFFFFFFF;
                    break;
                }

                if (act.assess == PATHASSESS_ELSE) // if it's an ELSE, stop parsing immediately, we have nothing more to parse!
                {
                    break;
                }

                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, line, cursor - line);
            cmpCleanUpToken(param1);
            // apply the assessment type immediately
            act.assess = cmpGetActionAssessmentType(param1);
            if (act.assess == PATHASSESS_ENDIF) // if it's an ENDIF, stop parsing immediately, we have nothing more to parse!
            {
                act.track = 0xFFFFFFFF;
                break;
            }
            if (act.assess == PATHASSESS_ELSE) // if it's an ELSE, stop parsing immediately, we have nothing more to parse!
            {
                break;
            }
            // get the left value
            cursor = strchr(line, ' ');
            cursor++;
            cursor2 = cmpGetActionComparatorPos(cursor);
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing comparison action, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param2, cursor, cursor2 - cursor);
            cmpCleanUpToken(param2);
            // get the comparison op position
            cursor3 = cursor2 + 1;
            if (*cursor3 == '=')
                cursor3++;
            strncpy(param3, cursor2, cursor3 - cursor2);
            cmpCleanUpToken(param3);
            // get the right value
            strcpy(param4, cursor3);
            cmpCleanUpToken(param4);
            // apply the values
            if (cmpPathActionSetValue(&act, &value, false, param2) < 0)
            {
                cout << "ERROR: can't set condition 'value' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.only.value = value;
            if (cmpPathActionSetValue(&act, &value, true, param4) < 0)
            {
                cout << "ERROR: can't set condition 'compareValue' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.only.compareValue = value;
            // apply the comparison op
            act.comparison = cmpGetActionComparatorType(param3);


            break;
        case PATHACTION_WAITTIME:
            // get the waittime millisecs
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            strcpy(param1, cursor);
            cmpCleanUpToken(param1);
            // apply the values
            if (cmpPathActionSetValue(&act, &value, true, param1) < 0)
            {
                cout << "ERROR: can't set Wait 'millisecs' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.waittime.lowest = 0;
            act.act.waittime.millisecs = value;

            break;
        case PATHACTION_WAITBEAT:
            // get the first waitbeat param string (millisecs)
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = strchr(line, '(');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing opening bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get waitbeat 'every' param
            cursor = strchr(line, '(') + 1;
            cursor2 = strchr(line, ',');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing first comma, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param2, cursor, cursor2 - cursor);
            cmpCleanUpToken(param2);
            // get waitbeat 'note' param
            cursor = strchr(line, ',') + 1;
            cursor2 = strchr(cursor, ',');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing second comma, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param3, cursor, cursor2 - cursor);
            cmpCleanUpToken(param3);
            // get waitbeat 'offset' param
            cursor = strchr(line, ',') + 1;
            cursor = strchr(cursor, ',') + 1;
            cursor2 = strchr(cursor, ')');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing closing bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param4, cursor, cursor2 - cursor);
            cmpCleanUpToken(param4);
            // apply values
            if (cmpPathActionSetValue(&act, &value, true, param1) < 0)
            {
                cout << "ERROR: can't set WaitBeat 'millisecs' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.waitbeat.millisecs = value;
            act.act.waitbeat.every = stoi(param2);
            act.act.waitbeat.note = stoi(param3);
            act.act.waitbeat.offset = stoi(param4);

            break;
        case PATHACTION_BRANCHTO:
            // get the first branch param string
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = strchr(cursor, '(');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing opening bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get the second branch param string
            cursor = strchr(line, '(') + 1;
            cursor2 = strchr(cursor, ',');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing comma, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param2, cursor, cursor2 - cursor);
            cmpCleanUpToken(param2);
            // get the third branch param string
            cursor = strchr(line, ',') + 1;
            cursor2 = strchr(cursor, ')');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing closing bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param3, cursor, cursor2 - cursor);
            cmpCleanUpToken(param3);
            // apply the values
            if (cmpPathActionSetValue(&act, &value, false, param1) < 0)
            {
                cout << "ERROR: can't set Branch 'node' value at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.branch.node = value;

            if (cmpPathActionSetValue(&act, &value, true, param2) < 0)
            {
                cout << "ERROR: can't set Branch 'ofsection' value at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.branch.ofsection = value;

            act.act.branch.immediate = stoi(param3);
            if (!act.act.branch.immediate)
                act.rightvaluetype = 0;

            break;
        case PATHACTION_STRETCHFADE:
        case PATHACTION_PITCHFADE:
        case PATHACTION_SFXFADE:
        case PATHACTION_DRYLEVELFADE:
        case PATHACTION_FADE:
            // get the first fade param string
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = strchr(cursor, '(');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing opening bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get fade 'tovol' param
            cursor = strchr(line, '(') + 1;
            cursor2 = strchr(cursor, ',');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing first comma, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param2, cursor, cursor2 - cursor);
            cmpCleanUpToken(param2);
            // get fade 'flip' param
            cursor = strchr(line, ',') + 1;
            cursor2 = strchr(cursor, ',');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing second comma, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param3, cursor, cursor2 - cursor);
            cmpCleanUpToken(param3);
            // get fade 'ms' param
            cursor = strchr(line, ',') + 1;
            cursor = strchr(cursor, ',') + 1;
            cursor2 = strchr(cursor, ')');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing closing bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param4, cursor, cursor2 - cursor);
            cmpCleanUpToken(param4);

            // ASSUMING MS VALUE IS THE 'RIGHT' VALUE
            if (cmpPathActionSetValue(&act, &value, true, param4) < 0)
            {
                cout << "ERROR: can't set " << actnamestr << " 'ms' at line " << cmpCurLine << '\n';
                return -1;
            }
            // apply the values
            switch (type)
            {
            case PATHACTION_STRETCHFADE:
                act.act.stretchfade.id = cmpGetActionFadeWhatType(param1);
                act.act.stretchfade.tovol = stoi(param2);
                act.act.stretchfade.flip = stoi(param3);
                act.act.stretchfade.ms = value;
                break;
            case PATHACTION_PITCHFADE:
                act.act.pitchfade.id = cmpGetActionFadeWhatType(param1);
                act.act.pitchfade.tovol = stoi(param2);
                act.act.pitchfade.flip = stoi(param3);
                act.act.pitchfade.ms = value;
                break;
            case PATHACTION_SFXFADE:
                act.act.sfxfade.id = cmpGetActionFadeWhatType(param1);
                act.act.sfxfade.tovol = stoi(param2);
                act.act.sfxfade.flip = stoi(param3);
                act.act.sfxfade.ms = value;
                break;
            case PATHACTION_DRYLEVELFADE:
                act.act.dryfade.id = cmpGetActionFadeWhatType(param1);
                act.act.dryfade.tovol = stoi(param2);
                act.act.dryfade.flip = stoi(param3);
                act.act.dryfade.ms = value;
                break;
            case PATHACTION_FADE:
            default:
                act.act.fade.id = cmpGetActionFadeWhatType(param1);
                act.act.fade.tovol = stoi(param2);
                act.act.fade.flip = stoi(param3);
                act.act.fade.ms = value;
                break;
            }

            break;
        case PATHACTION_SETVALUE:
            // get the set left value
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = strchr(line, '=');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing equals sign, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get the set right value
            cursor = cursor2 + 1;
            strcpy(param2, cursor);
            cmpCleanUpToken(param2);
            // apply values
            if (cmpPathActionSetValue(&act, &value, false, param1) < 0)
            {
                cout << "ERROR: can't set " << actnamestr << " 'setwhat' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.setval.setwhat = value;
            if (cmpPathActionSetValue(&act, &value, true, param2) < 0)
            {
                cout << "ERROR: can't set " << actnamestr << " 'towhat' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.setval.towhat = value;

            break;
        case PATHACTION_EVENT:
            // get the eventID
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            strcpy(param1, cursor);
            cmpCleanUpToken(param1);
            // apply the values
            if (hdr.minorRev > 1)
            {
                if (cmpPathActionSetValue(&act, &value, false, param1) < 0)
                {
                    cout << "ERROR: can't set Event 'eventID' at line " << cmpCurLine << '\n';
                    return -1;
                }
            }
            else
            {
                if ((param1[0] != '0') && (param1[1] != 'x'))
                {
                    cout << "ERROR: can't parse " << actnamestr << " action - expected a hexadecimal number parameter, at line " << cmpCurLine << '\n';
                    return -1;
                }

                sscanf(param1 + 2, "%X", &value);
            }

            act.act.event.eventid = value;
            break;
        case PATHACTION_CALLBACK:
            // get callback value
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = strchr(cursor, '(');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing opening bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get callback id
            cursor = strchr(line, '(') + 1;
            cursor2 = strchr(cursor, ')');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing closing bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param2, cursor, cursor2 - cursor);
            cmpCleanUpToken(param2);
            // apply values
            if (cmpPathActionSetValue(&act, &value, false, param1) < 0)
            {
                cout << "ERROR: can't set Callback 'value' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.callback.value = value;
            if (cmpPathActionSetValue(&act, &value, true, param2) < 0)
            {
                cout << "ERROR: can't set Callback 'id' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.callback.id = value;

            break;
        case PATHACTION_CALC:
            // get calc value
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = cmpGetActionOperatorPos(cursor);
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing operator action, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get calc operator
            strcpy(param2, cursor2);
            param2[1] = '\0';
            // get calc by
            strcpy(param3, cursor2 + 1);
            cmpCleanUpToken(param3);
            // apply values
            if (cmpPathActionSetValue(&act, &value, false, param1) < 0)
            {
                cout << "ERROR: can't set Calc 'value' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.calc.value = value;
            act.act.calc.op = cmpGetActionOperatorType(param2);
            if (cmpPathActionSetValue(&act, &value, true, param3) < 0)
            {
                cout << "ERROR: can't set Calc 'by' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.calc.by = value;
            
            break;
        case PATHACTION_PAUSE:
            // get pause when
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = strchr(cursor, '(');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing opening bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get pause on
            cursor = strchr(line, '(') + 1;
            cursor2 = strchr(cursor, ')');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing closing bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param2, cursor, cursor2 - cursor);
            cmpCleanUpToken(param2);
            // apply values
            if (cmpPathActionSetValue(&act, &value, false, param1) < 0)
            {
                cout << "ERROR: can't set Pause 'when' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.pause.when = value;
            if (cmpPathActionSetValue(&act, &value, true, param2) < 0)
            {
                cout << "ERROR: can't set Pause 'on' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.pause.on = value;
            break;
        case PATHACTION_LOADBANK:
            // get LoadBank subbanknum
            cursor = strchr(line, ' ');
            if (!cursor)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing starting space, at line " << cmpCurLine << '\n';
                return -1;
            }
            cursor++;
            cursor2 = strchr(cursor, '(');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing opening bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param1, cursor, cursor2 - cursor);
            cmpCleanUpToken(param1);
            // get LoadBank unload
            cursor = strchr(line, '(') + 1;
            cursor2 = strchr(cursor, ')');
            if (!cursor2)
            {
                cout << "ERROR: can't parse " << actnamestr << " action - incorrect formatting, missing closing bracket, at line " << cmpCurLine << '\n';
                return -1;
            }
            strncpy(param2, cursor, cursor2 - cursor);
            cmpCleanUpToken(param2);
            // apply values
            if (cmpPathActionSetValue(&act, &value, false, param1) < 0)
            {
                cout << "ERROR: can't set LoadBank 'subbanknum' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.loadbank.subbanknum = value;
            if (cmpPathActionSetValue(&act, &value, true, param2) < 0)
            {
                cout << "ERROR: can't set LoadBank 'unload' at line " << cmpCurLine << '\n';
                return -1;
            }
            act.act.loadbank.unload = value;
            break;
        // these have no values to be set...
        case PATHACTION_FILTER_CLEAR:
        case PATHACTION_FILTER_OFF:
        case PATHACTION_FILTER_ON:
        default:
            break;
        }

        cmpEvents.back().actions.push_back(act);
        cmpEvents.back().evt.numactions = cmpEvents.back().actions.size();

        return 1;
    }

    return 0;
}

bool CompilerGetLine(FILE* f)
{
    memset(cmpReadLine, 0, 512);
    fgets(cmpReadLine, 512, f);

    if (cmpReadLine[0] != '#')
    {
        // clean up line for easier parsing
        // clean up comment indicators
        char* cleanup = strchr(cmpReadLine, '#');
        if (cleanup)
            *cleanup = 0;

        // clean up the spaces
        cmpCleanUpToken(cmpReadLine);

        return true;
    }

    memset(cmpReadLine, 0, 512);
    return false;
}

int CompilerParseLine(char* line)
{
    char token[512];
    char* cursor = strchr(line, ' ');
    if (cursor)
        strcpy(token, cursor);
    else
        strcpy(token, line);
    cmpCleanUpToken(token);

    if (cmpCurrentPS == cmpPS_NULL)
    {
        PATHNAMEDVAR var = { 0 };
        cmpROUTERType router;
        cmpPATHTRACKINFO trackinfo = { 0 };
        cmpPATHFINDNODE node = { };
        cmpPATHEVENT evt = { };
        uint32_t val = 0;
    
        switch (CompilerGetMainTokenType(line))
        {
        case cmpVAR:
            cmpNamedVars.push_back(var);
            cmpCurrentPS = cmpPS_VAR;
            break;
        case cmpROUTER:
            if (bStringHasAlpha(token))
            {
                cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                return -1;
            }
            router.index = stoi(token);
            cmpRouters.push_back(router);
            cmpCurrentPS = cmpPS_ROUTER;
            break;
        case cmpTRACK:
            if (bStringHasAlpha(token))
            {
                cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                return -1;
            }
            trackinfo.index = stoi(token);
            cmpTrackInfos.push_back(trackinfo);
            cmpCurrentPS = cmpPS_TRACK;
            break;
        case cmpNODE:
            if (bStringHasAlpha(token))
            {
                cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << token << "] at line " << cmpCurLine << '\n';
                return -1;
            }
            node.index = stoi(token);
            cmpNodes.push_back(node);
            cmpCurrentPS = cmpPS_NODE;
            break;
        case cmpEVENT:
            if ((token[0] != '0') && (token[1] != 'x'))
            {
                cout << "ERROR: unknown token in event ID (expected only hexadecimal integer): [" << token << "] at line " << cmpCurLine << '\n';
                return -1;
            }
            chrToUpper(token + 2);
            sscanf(token + 2, "%X", &val);
            evt.evt.eventID = val;
            cmpEvents.push_back(evt);
            cmpCurrentPS = cmpPS_EVENT;
            break;
        case cmpMajorRev:
            hdr.majorRev = (uint8_t)stoi(token);
            break;
        case cmpMinorRev:
            hdr.minorRev = (uint8_t)stoi(token);
            break;
        case cmpReleaseRev:
            hdr.release = (uint8_t)stoi(token);
            break;
        case cmpPrereleaseRev:
            hdr.prerelease = (uint8_t)stoi(token);
            break;
        case cmpGenerateID:
            hdr.generateID = (uint16_t)stoi(token);
            break;
        case cmpProjectID:
            hdr.projectID = (uint8_t)stoi(token);
            break;
        case cmpNumSections:
            hdr.numsections = (uint8_t)stoi(token);
            break;
        default:
            if (!isspace(*token) && (strlen(token) > 0))
            {
                cout << "ERROR: unknown token [" << token << "] at line " << cmpCurLine << '\n';
                return -1;
            }
        }
    }
    else
    {
        switch (cmpCurrentPS)
        {
        case cmpPS_VAR:
            if (cmpCurrentPSS == cmpPSS_NULL)
            {
                // looking for body entrance
                if (strchr(token, '{'))
                {
                    cmpCurrentPSS = cmpPSS_EnteredBody;
                }
            }
            if (cmpCurrentPSS == cmpPSS_EnteredBody)
            {
                // attempt to get the var
                int varparseret = cmpParseVAR(line);
                if (varparseret < 0)
                {
                    return -1;
                }
    
                // looking for body exit
                if (strchr(token, '}'))
                {
                    cmpCurrentPSS = cmpPSS_NULL;
                    cmpCurrentPS = cmpPS_NULL;
                }
            }
            break;
        case cmpPS_ROUTER:
            if (cmpCurrentPSS == cmpPSS_NULL)
            {
                // looking for body entrance
                if (strchr(token, '{'))
                {
                    cmpCurrentPSS = cmpPSS_EnteredBody;
                }
            }
            if (cmpCurrentPSS == cmpPSS_EnteredBody)
            {
                // attempt to parse
                int varparseret = cmpParseROUTER(line);
                if (varparseret < 0)
                {
                    return -1;
                }
    
                // looking for body exit
                if (strchr(token, '}'))
                {
                    cmpCurrentPSS = cmpPSS_NULL;
                    cmpCurrentPS = cmpPS_NULL;
                }
            }
            break;
        case cmpPS_TRACK:
            if (cmpCurrentPSS == cmpPSS_NULL)
            {
                // looking for body entrance
                if (strchr(token, '{'))
                {
                    cmpCurrentPSS = cmpPSS_EnteredBody;
                }
            }
            if (cmpCurrentPSS == cmpPSS_EnteredBody)
            {
                // attempt to parse
                int varparseret = cmpParseTRACK(line);
                if (varparseret < 0)
                {
                    return -1;
                }
    
                // looking for body exit
                if (strchr(token, '}'))
                {
                    cmpCurrentPSS = cmpPSS_NULL;
                    cmpCurrentPS = cmpPS_NULL;
                }
            }
            break;
        case cmpPS_NODE:
            if (cmpCurrentPSS == cmpPSS_NULL)
            {
                // looking for body entrance
                if (strchr(token, '{'))
                {
                    cmpCurrentPSS = cmpPSS_EnteredBody;
                }
            }
            if (cmpCurrentPSS == cmpPSS_EnteredBody)
            {
                // attempt to parse
                // nodes can contain sub-bodies for branches, check for those now...
                // TODO: make this better, compilers shouldn't have hardcoded rules for this
                if (cmpCurrentPSSS == cmpPSS_EnteredBody)
                {
                    int varparseret = cmpParseNODEBRANCH(line);
                    if (varparseret < 0)
                    {
                        return -1;
                    }
    
                    // looking for sub-body exit
                    if (strchr(token, '}'))
                    {
                        cmpCurrentPSSS = cmpPSS_NULL;
                    }
                }
                else
                {
                    int varparseret = cmpParseNODE(line);
                    if (varparseret < 0)
                    {
                        return -1;
                    }
    
                    // looking for body exit
                    if (strchr(token, '}'))
                    {
                        cmpCurrentPSS = cmpPSS_NULL;
                        cmpCurrentPS = cmpPS_NULL;
                    }
                }
            }
            break;
        case cmpPS_EVENT:
            if (cmpCurrentPSS == cmpPSS_NULL)
            {
                // looking for body entrance
                if (strchr(token, '{'))
                {
                    cmpCurrentPSS = cmpPSS_EnteredBody;
                }
            }
            if (cmpCurrentPSS == cmpPSS_EnteredBody)
            {
                // attempt to parse
                // events can contain sub-bodies for actions, check for those now...
                // TODO: make this better, compilers shouldn't have hardcoded rules for this
                if (cmpCurrentPSSS == cmpPSS_EnteredBody)
                {
                    int varparseret = cmpParseACTION(line);
                    if (varparseret < 0)
                    {
                        return -1;
                    }
    
                    // looking for sub-body exit
                    if (strchr(token, '}'))
                    {
                        cmpCurrentPSSS = cmpPSS_NULL;
                    }
                }
                else
                {
                    int varparseret = cmpParseEVENT(line);
                    if (varparseret < 0)
                    {
                        return -1;
                    }
    
                    // looking for body exit
                    if (strchr(token, '}'))
                    {
                        cmpCurrentPSS = cmpPSS_NULL;
                        cmpCurrentPS = cmpPS_NULL;
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    
    
    return 0;
}

int MPFCompiler(char* txtfilename, char* mpffilename)
{
    FILE* f = fopen(txtfilename, "rb");
    if (!f)
    {
        cout << "Can't open file [" << txtfilename << "] for reading: " << strerror(errno) << '\n';
        return -1;
    }

    FILE* fout = fopen(mpffilename, "wb");
    if (!fout)
    {
        cout << "Can't open file [" << mpffilename << "] for writing: " << strerror(errno) << '\n';
        fclose(f);
        return -1;
    }

    cout << "Parsing...\n";
    while (!feof(f))
    {
        if (CompilerGetLine(f))
        {
            if (CompilerParseLine(cmpReadLine))
            {
                cout << "Failed compilation at line: " << cmpCurLine << '\n';
                cmpCurrentPS = cmpPS_NULL;
                cmpCurrentPSS = cmpPSS_NULL;
                fclose(f);
                return -1;
            }
        }
        cmpCurLine++;
    }
    if (cmpCurrentPS != cmpPS_NULL)
    {
        cout << "Unexpected EOF! Read lines: " << cmpCurLine << '\n';
        fclose(f);
        return -1;
    }

    cout << "Writing output file...\n";
    // link everything and create a binary
    // prepare known header data...
    hdr.id = PATHFINDER_MAGIC;
    if (hdr.majorRev == 0)
        hdr.majorRev = PATH_SUPPORTED_VERSION;
    if (hdr.minorRev == 0)
        hdr.minorRev = 1;
    if (hdr.release == 0)
        hdr.release = 0xB0;
    if (hdr.prerelease == 0)
        hdr.prerelease = 1;
    if (hdr.numsections == 0)
        hdr.numsections = 1;
    hdr.numtracks = cmpTrackInfos.size();
    hdr.numevents = cmpEvents.size();
    hdr.numrouters = cmpRouters.size();
    hdr.numnamedvars = cmpNamedVars.size();
    hdr.numnodes = cmpNodes.size();
    hdr.nodeoffsets = sizeof(PATHFINDHEADER);

    // precalculate size of node offset list to reserve space...
    size_t nodeoffsets_size = hdr.numnodes * sizeof(uint16_t);
    size_t nodes_end = 0;
    size_t offset_calc = 0;
    // go to the spot to write the actual data first...
    hdr.nodedata = sizeof(PATHFINDHEADER) + nodeoffsets_size;
    // align by 4 bytes
    hdr.nodedata = hdr.nodedata + (hdr.nodedata % 4);
    fseek(fout, hdr.nodedata, SEEK_SET);
    // sort the vector by the indicies
    std::sort(cmpNodes.begin(), cmpNodes.end());
    // write the node data...
    for (int i = 0; i < hdr.numnodes; i++)
    {
        offset_calc = ftell(fout);
        offset_calc /= 4;
        cmpNodeOffsets.push_back(offset_calc);
        fwrite(&(cmpNodes.at(i).node), sizeof(PATHFINDNODE), 1, fout);
        fwrite(&(cmpNodes.at(i).sendevent), sizeof(PATHNODEEVENT), 1, fout);

        for (int j = 0; j < cmpNodes.at(i).node.numbranches; j++)
        {
            fwrite(&(cmpNodes.at(i).branches.at(j)), sizeof(PATHFINDBRANCH), 1, fout);
        }
    }
    nodes_end = ftell(fout);
    // go back and update the node offsets
    fseek(fout, hdr.nodeoffsets, SEEK_SET);
    for (int i = 0; i < hdr.numnodes; i++)
    {
        fwrite(&(cmpNodeOffsets.at(i)), sizeof(uint16_t), 1, fout);
    }
    // continue with events...
    fseek(fout, nodes_end, SEEK_SET);
    hdr.eventoffsets = nodes_end;
    size_t eventoffsets_size = hdr.numevents * sizeof(uint16_t);
    size_t events_end = 0;
    hdr.eventdata = nodes_end + eventoffsets_size;
    // align by 4 bytes
    hdr.eventdata = hdr.eventdata + (hdr.eventdata % 4);
    fseek(fout, hdr.eventdata, SEEK_SET);
    for (int i = 0; i < hdr.numevents; i++)
    {
        offset_calc = ftell(fout);
        offset_calc /= 4;
        cmpEventOffsets.push_back(offset_calc);
        fwrite(&(cmpEvents.at(i).evt), sizeof(PATHEVENT), 1, fout);

        for (int j = 0; j < cmpEvents.at(i).evt.numactions; j++)
        {
            fwrite(&(cmpEvents.at(i).actions.at(j)), sizeof(PATHACTION), 1, fout);
        }
    }
    events_end = ftell(fout);
    // go back and update the event offsets
    fseek(fout, hdr.eventoffsets, SEEK_SET);
    for (int i = 0; i < hdr.numevents; i++)
    {
        fwrite(&(cmpEventOffsets.at(i)), sizeof(uint16_t), 1, fout);
    }
    // continue with named vars...
    hdr.namedvars = events_end;
    // align by 4 bytes
    hdr.namedvars = hdr.namedvars + (hdr.namedvars % 4);
    fseek(fout, hdr.namedvars, SEEK_SET);
    for (int i = 0; i < hdr.numnamedvars; i++)
    {
        fwrite(&(cmpNamedVars.at(i)), sizeof(PATHNAMEDVAR), 1, fout);
    }
    // continue with routers...
    hdr.noderouters = ftell(fout);
    // sort the vector by the indicies
    std::sort(cmpRouters.begin(), cmpRouters.end());
    size_t routeroffsets_size = (hdr.numrouters + 1) * sizeof(uint32_t);
    size_t routerdata = hdr.noderouters + routeroffsets_size;
    // align by 4 bytes
    routerdata = routerdata + (routerdata % 4);
    fseek(fout, routerdata, SEEK_SET);
    for (int i = 0; i < hdr.numrouters; i++)
    {
        offset_calc = ftell(fout);
        offset_calc /= 4;
        cmpRouterOffsets.push_back(offset_calc);
        for (int j = 0; j < cmpRouters.at(i).rp.size(); j++)
        {
            uint32_t router = cmpRouters.at(i).rp.at(j).node2;
            router |= cmpRouters.at(i).rp.at(j).node1 << 16;
            fwrite(&router, sizeof(uint32_t), 1, fout);
        }
    }
    size_t routers_end = ftell(fout);
    // extra offset for router
    offset_calc = routers_end;
    offset_calc /= 4;
    cmpRouterOffsets.push_back(offset_calc);
    // go back and update the router offsets
    fseek(fout, hdr.noderouters, SEEK_SET);
    for (int i = 0; i < hdr.numrouters + 1; i++)
    {
        fwrite(&(cmpRouterOffsets.at(i)), sizeof(uint32_t), 1, fout);
    }
    // continue with tracks...
    hdr.trackoffsets = routers_end;
    // sort the vector by the indicies
    std::sort(cmpTrackInfos.begin(), cmpTrackInfos.end());
    size_t trackoffsets_size = hdr.numtracks * sizeof(uint32_t);
    hdr.trackinfos = hdr.trackoffsets + trackoffsets_size;
    // align by 4 bytes
    hdr.trackinfos = hdr.trackinfos + (hdr.trackinfos % 4);
    fseek(fout, hdr.trackinfos, SEEK_SET);
    for (int i = 0; i < hdr.numtracks; i++)
    {
        offset_calc = ftell(fout);
        offset_calc /= 4;
        cmpTrackOffsets.push_back(offset_calc);
        fwrite(&(cmpTrackInfos.at(i).track), sizeof(PATHTRACKINFO), 1, fout);
    }
    size_t tracks_end = ftell(fout);
    // go back and update the track offsets
    fseek(fout, hdr.trackoffsets, SEEK_SET);
    for (int i = 0; i < hdr.numtracks; i++)
    {
        fwrite(&(cmpTrackOffsets.at(i)), sizeof(uint32_t), 1, fout);
    }

    // NOT writing sample offsets now... because we want to separate that process from the compilation...
    // we will, however, say where the offsets will be!
    hdr.sampleoffsets = tracks_end;

    // finally, go back to the start and write the header
    fseek(fout, 0, SEEK_SET);
    fwrite(&hdr, sizeof(PATHFINDHEADER), 1, fout);

    fclose(fout);
    fclose(f);

    cout << "Compilation completed without errors.\nRemember to add the samples with the \"update samples (-su)\" command before using the file!\n";

    return 0;
}

// compiler stuff end

// music add + concat stuff 

int FindLastBits(const char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (!f)
    {
        cout << "Can't open file [" << filename << "] for reading: " << strerror(errno) << '\n';
        return -1;
    }


    char param[32];
    char* cursor;
    int readNode = 0;
    int readPart = 0;
    int readWave = 0;
    int readRouter = 0;
    int readSection = 0;


    while (!feof(f))
    {
        if (CompilerGetLine(f))
        {
            string ln(cmpReadLine);
            if (ln.find("Node") != string::npos)
            {
                cursor = strchr(cmpReadLine, ' ') + 1;
                if (!cursor)
                {
                    cout << "ERROR: can't parse node - incorrect formatting, missing space after Node token word, at line " << cmpCurLine << '\n';
                    return -1;
                }
                strcpy(param, cursor);
                cmpCleanUpToken(param);
                if (bStringHasAlpha(param))
                {
                    cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << param << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                readNode = stoi(param);
                if (readNode > LastNodeIndex)
                    LastNodeIndex = readNode;
            }
            if (ln.find("Router") != string::npos)
            {
                cursor = strchr(cmpReadLine, ' ') + 1;
                if (!cursor)
                {
                    cout << "ERROR: can't parse router - incorrect formatting, missing space after router token word, at line " << cmpCurLine << '\n';
                    return -1;
                }
                strcpy(param, cursor);
                cmpCleanUpToken(param);
                if (bStringHasAlpha(param))
                {
                    cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << param << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                readRouter = stoi(param);
                if (readRouter > LastRouterIndex)
                    LastRouterIndex = readRouter;
            }
            if (ln.find("Part") != string::npos)
            {
                cursor = strchr(cmpReadLine, ' ') + 1;
                if (!cursor)
                {
                    cout << "ERROR: can't parse part - incorrect formatting, missing space after Part token word, at line " << cmpCurLine << '\n';
                    return -1;
                }
                strcpy(param, cursor);
                cmpCleanUpToken(param);
                if (bStringHasAlpha(param))
                {
                    cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << param << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                readPart = stoi(param);
                if (readPart > LastPartIndex)
                    LastPartIndex = readPart;
            }
            if (ln.find("Section") != string::npos)
            {
                cursor = strchr(cmpReadLine, ' ') + 1;
                if (!cursor)
                {
                    cout << "ERROR: can't parse section - incorrect formatting, missing space after Section token word, at line " << cmpCurLine << '\n';
                    return -1;
                }
                strcpy(param, cursor);
                cmpCleanUpToken(param);
                if (bStringHasAlpha(param))
                {
                    cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << param << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                readSection = stoi(param);
                if (readSection > LastSectionIndex)
                    LastSectionIndex = readSection;
            }
            if (ln.find("Wave") != string::npos)
            {
                cursor = strchr(cmpReadLine, ' ') + 1;
                if (!cursor)
                {
                    cout << "ERROR: can't parse wave - incorrect formatting, missing space after Wave token word, at line " << cmpCurLine << '\n';
                    return -1;
                }
                strcpy(param, cursor);
                cmpCleanUpToken(param);
                if (bStringHasAlpha(param))
                {
                    cout << "ERROR: unknown token, found non-numeric char in token (expected only integers): [" << param << "] at line " << cmpCurLine << '\n';
                    return -1;
                }
                readWave = stoi(param);
                if (readWave > LastWaveIndex)
                    LastWaveIndex = readWave;
            }
        }
        cmpCurLine++;
    }

    fclose(f);
    return 0;
}

int AppendSlot(const char* filename, bool bProStreetMode, uint32_t TrackID = 0x11000001, uint32_t SectionID = 0)
{
    FILE* f = fopen(filename, "ab");
    if (!f)
    {
        cout << "Can't open file [" << filename << "] for updating: " << strerror(errno) << '\n';
        return -1;
    }

    int AppendNodeIndex = LastNodeIndex + 1;
    int AppendPartIndex = LastPartIndex + 1;
    int AppendWaveIndex = LastWaveIndex + 1;
    uint32_t AppendEventID = 0;
    char AppendName[64];

    // generate an eventID using crc24
    sprintf(AppendName, "PATH_EVENT_Sample%d", AppendWaveIndex);
    AppendEventID = crc_octets((unsigned char*)AppendName, strlen(AppendName));

    cout << "Appending EventID: [0x" << std::uppercase << std::hex << AppendEventID << "], Sample: [" << std::dec << AppendWaveIndex << "]\n";

    // write out info
    fprintf(f, "\n# Appended slot\n# EventID: 0x%08X\n# Sample: %d\n", AppendEventID, AppendWaveIndex);
    // the entrance node
    fprintf(f, "Node %d\n", AppendNodeIndex);
    fputs("{\n", f);
    fputs("\tWave 0\n", f);
    fputs("\tTrack 0\n", f);
    fputs("\tSection 1\n", f);
    fprintf(f, "\tPart %d\n", AppendPartIndex);
    fputs("\tRouter -1\n", f);
    fputs("\tController 0\n", f);
    fputs("\tBeats 1\n", f);
    fputs("\tBars 1\n", f);
    fputs("\tSynchEvery 0\n", f);
    fputs("\tSynchOffset 0\n", f);
    fputs("\tNotes 0\n", f);
    fputs("\tSynch 0\n", f);
    fputs("\tChannelBranching 0\n", f);
    fputs("\tBranches\n", f);
    fputs("\t{\n", f);
    fprintf(f, "\t\tControl 0,127 > %d\n", AppendNodeIndex + 2);
    fputs("\t}\n", f);
    fputs("}\n", f);
    // the ending node
    fprintf(f, "Node %d\n", AppendNodeIndex + 1);
    fputs("{\n", f);
    fputs("\tWave -1\n", f);
    fputs("\tTrack 0\n", f);
    fputs("\tSection 1\n", f);
    fprintf(f, "\tPart %d\n", AppendPartIndex + 1);
    fputs("\tRouter -1\n", f);
    fputs("\tController 0\n", f);
    fputs("\tBeats 1\n", f);
    fputs("\tBars 1\n", f);
    fputs("\tSynchEvery 0\n", f);
    fputs("\tSynchOffset 0\n", f);
    fputs("\tNotes 0\n", f);
    fputs("\tSynch 0\n", f);
    fputs("\tChannelBranching 0\n", f);
    fputs("\tRepeat 0\n", f);
    fputs("}\n", f);
    // the main node
    fprintf(f, "Node %d\n", AppendNodeIndex + 2);
    fputs("{\n", f);
    fprintf(f, "\tWave %d\n", AppendWaveIndex);
    fputs("\tTrack 0\n", f);
    fputs("\tSection 1\n", f);
    fprintf(f, "\tPart %d\n", AppendPartIndex + 1);
    fputs("\tRouter -1\n", f);
    fputs("\tController 0\n", f);
    fputs("\tBeats 4\n", f);
    fputs("\tBars 1\n", f);
    fputs("\tSynchEvery 0\n", f);
    fputs("\tSynchOffset 0\n", f);
    fputs("\tNotes 4\n", f);
    fputs("\tSynch 0\n", f);
    fputs("\tChannelBranching 0\n", f);
    fputs("\tBranches\n", f);
    fputs("\t{\n", f);
    fprintf(f, "\t\tControl 0,127 > %d\n", AppendNodeIndex + 1);
    fputs("\t}\n", f);
    fputs("}\n", f);
    // event ID
    fprintf(f, "Event 0x%08X\n", AppendEventID);
    fputs("{\n", f);
    fputs("\tActions\n", f);
    fputs("\t{\n", f);
    fprintf(f, "(0x%X, %d)\t\tBranch -1 (-1, -1)\n", TrackID, SectionID);
    if (bProStreetMode)
        fprintf(f, "(0x%X, %d)\t\tSet CONTROLLER = 120\n", TrackID, SectionID);
    fprintf(f, "(0x%X, %d)\t\tBranch %d (-1, 0)\n", TrackID, SectionID, AppendNodeIndex);
    fputs("\t}\n", f);
    fputs("}\n", f);
    fclose(f);
    return 0;
}

int ConcatMaps(const char* dstfile, const char* srcfile)
{
    int AppendNodeIndex = LastNodeIndex + 1;
    int AppendPartIndex = LastPartIndex + 1;
    int AppendWaveIndex = LastWaveIndex;
    int AppendRouterIndex = LastRouterIndex + 1;
    int AppendSectionIndex = LastSectionIndex + 1;

    int readIdx = 0;

    int param1 = 0;
    int param2 = 0;

    char* cursor;
    char* cursor2;

    FILE* f = fopen(srcfile, "rb");
    if (!f)
    {
        cout << "Can't open file [" << srcfile << "] for reading: " << strerror(errno) << '\n';
        return -1;
    }

    FILE* fdst = fopen(dstfile, "ab");
    if (!fdst)
    {
        cout << "Can't open file [" << dstfile << "] for appending: " << strerror(errno) << '\n';
        return -1;
    }

    fprintDividerLine(fdst, '#', '-', 0, 72);
    fprintf(fdst, "# File: %s\n", srcfile);
    fprintDividerLine(fdst, '#', '-', 0, 72);

    while (!feof(f))
    {
        fgets(cmpReadLine, 512, f);
        string ln(cmpReadLine);
        strcpy(catReadLine, cmpReadLine);
        
        // skip version info
        if (!((ln.find("Major ") != string::npos) || 
            (ln.find("Minor ") != string::npos) || 
            (ln.find("Release ") != string::npos) || 
            (ln.find("Prerelease ") != string::npos) || 
            (ln.find("GenerateID ") != string::npos) || 
            (ln.find("ProjectID ") != string::npos) ||
            (ln.find("NumSections ") != string::npos)))
        {
            // skip track info
            if (ln.find("Track ") != string::npos)
            {
                size_t oldpos = ftell(f);
                fgets(catReadLine, 512, f);
                string ln2(catReadLine);
                if (ln2.find("{") != string::npos)
                {
                    int gotc = 0;
                    do
                    {
                        gotc = fgetc(f);
                    } while (gotc != '}');
                    fseek(f, 1, SEEK_CUR);
                    strcpy(catOutLine, "");
                }
                else
                {
                    fseek(f, oldpos, SEEK_SET);
                    strcpy(catOutLine, cmpReadLine);
                }
            }
            else if (ln.find("Router ") != string::npos)
            {
                // check if indented
                bool bIndented = false;

                if (catReadLine[0] == '\t')
                    bIndented = true;

                cmpCleanUpToken(catReadLine);
                cursor = strchr(catReadLine, ' ') + 1;
                readIdx = stoi(cursor);
                if (readIdx > 0)
                    if (bIndented)
                        sprintf(catOutLine, "\tRouter %d\n", readIdx + AppendRouterIndex);
                    else
                        sprintf(catOutLine, "Router %d\n", readIdx + AppendRouterIndex);
                else
                    if (bIndented)
                        sprintf(catOutLine, "\tRouter %d\n", readIdx);
                    else
                        sprintf(catOutLine, "Router %d\n", readIdx);
            }
            else if (ln.find(" > ") != string::npos)
            {
                // detect whether it's a router or a control branch
                if (!(ln.find("Control ") != string::npos))
                {
                    cmpCleanUpToken(catReadLine);
                    cursor = strchr(catReadLine, '>');
                    *cursor = 0;
                    cmpCleanUpToken(catReadLine);
                    cursor++;
                    while (*cursor == ' ')
                        cursor++;
                    param1 = stoi(catReadLine);
                    param2 = stoi(cursor);

                    if (param2 > 65535)
                        param2 = 65535;

                    sprintf(catOutLine, "\t%d > %d\n", param1 + AppendNodeIndex, param2 + AppendNodeIndex);
                }
                else
                {
                    cmpCleanUpToken(catReadLine);
                    cursor = strchr(catReadLine, '>');
                    *cursor = 0;
                    cmpCleanUpToken(catReadLine);
                    cursor++;
                    while (*cursor == ' ')
                        cursor++;
                    param1 = stoi(cursor);
                    sprintf(catOutLine, "\t\t%s > %d\n", catReadLine, param1 + AppendNodeIndex);
                }
            }
            else if (ln.find("Node ") != string::npos)
            {
                cmpCleanUpToken(catReadLine);
                cursor = strchr(catReadLine, ' ') + 1;
                readIdx = stoi(cursor);
                sprintf(catOutLine, "Node %d\n", readIdx + AppendNodeIndex);
            }
            else if (ln.find("Wave ") != string::npos)
            {
                cmpCleanUpToken(catReadLine);
                cursor = strchr(catReadLine, ' ') + 1;
                readIdx = stoi(cursor);
                if (readIdx > 0)
                    sprintf(catOutLine, "\tWave %d\n", readIdx + AppendWaveIndex);
                else
                    sprintf(catOutLine, "\tWave %d\n", readIdx);
            }
            else if (ln.find("Part ") != string::npos)
            {
                cmpCleanUpToken(catReadLine);
                cursor = strchr(catReadLine, ' ') + 1;
                readIdx = stoi(cursor);
                if (readIdx >= 0)
                    sprintf(catOutLine, "\tPart %d\n", readIdx + AppendPartIndex);
                else
                    sprintf(catOutLine, "\tPart %d\n", readIdx);
            }
            else if (ln.find("Section ") != string::npos)
            {
                cmpCleanUpToken(catReadLine);
                cursor = strchr(catReadLine, ' ') + 1;
                readIdx = stoi(cursor);
                if (readIdx >= 0)
                    sprintf(catOutLine, "\tSection %d\n", readIdx + AppendSectionIndex);
                else
                    sprintf(catOutLine, "\tSection %d\n", readIdx);
            }
            else if (ln.find("Branch ") != string::npos)
            {
                cursor = strchr(catReadLine, '\t');
                cursor = strchr(cursor, ' ');
                *cursor = 0;
                cursor++;
                cursor2 = strchr(cursor, ' ');
                *cursor2 = 0;
                cursor2++;

                if (bStringHasAlpha(cursor))
                    strcpy(catOutLine, cmpReadLine);
                else
                {
                    param1 = stoi(cursor);
                    if (param1 >= 0)
                        sprintf(catOutLine, "%s %d %s", catReadLine, param1 + AppendNodeIndex, cursor2);
                    else
                        strcpy(catOutLine, cmpReadLine);
                }
            }
            else if ((ln.find("If CURRENTNODE") != string::npos) || (ln.find("Elif CURRENTNODE") != string::npos))
            {
                cursor = strrchr(catReadLine, ' ');
                *cursor = 0;
                cursor++;
                cmpCleanUpToken(cursor);
                if (bStringHasAlpha(cursor))
                    strcpy(catOutLine, cmpReadLine);
                else
                {
                    param1 = stoi(cursor);
                    if (param1 >= 0)
                        sprintf(catOutLine, "%s %d\n", catReadLine, param1 + AppendNodeIndex);
                    else
                        strcpy(catOutLine, cmpReadLine);
                }
            }
            else if ((ln.find("If CURRENTPART") != string::npos) || (ln.find("Elif CURRENTPART") != string::npos))
            {
                cursor = strrchr(catReadLine, ' ');
                *cursor = 0;
                cursor++;
                cmpCleanUpToken(cursor);
                if (bStringHasAlpha(cursor))
                    strcpy(catOutLine, cmpReadLine);
                else
                {
                    param1 = stoi(cursor);
                    if (param1 >= 0)
                        sprintf(catOutLine, "%s %d\n", catReadLine, param1 + AppendPartIndex);
                    else
                        strcpy(catOutLine, cmpReadLine);
                }
            }
            else if ((ln.find("If CURRENTSECTION") != string::npos) || (ln.find("Elif CURRENTSECTION") != string::npos))
            {
                cursor = strrchr(catReadLine, ' ');
                *cursor = 0;
                cursor++;
                cmpCleanUpToken(cursor);
                if (bStringHasAlpha(cursor))
                    strcpy(catOutLine, cmpReadLine);
                else
                {
                    param1 = stoi(cursor);
                    if (param1 >= 0)
                        sprintf(catOutLine, "%s %d\n", catReadLine, param1 + AppendSectionIndex);
                    else
                        strcpy(catOutLine, cmpReadLine);
                }
            }
            else
                strcpy(catOutLine, cmpReadLine);

            fputs(catOutLine, fdst);
            fflush(fdst);
        }

    }

    fclose(fdst);
    fclose(f);
    return 0;
}

int ShiftSampleNamesInFolder(const char* foldername, int by)
{
    GetDirectoryListing(foldername);

    unsigned int MinSampNumber = 0xFFFFFFFF;
    unsigned int MaxSampNumber = 0;

    int ReadNumber = 0;

    char* cursor;

    for (int i = 0; i < FileDirectoryListing.size(); i++)
    {
        strcpy(catReadLine, FileDirectoryListing.at(i).c_str());
        cursor = strchr(catReadLine, '.');
        *cursor = 0;
        ReadNumber = stoi(catReadLine);

        if (ReadNumber < MinSampNumber)
            MinSampNumber = ReadNumber;

        if (ReadNumber > MaxSampNumber)
            MaxSampNumber = ReadNumber;
    }

    // if we're incrementing the names, go in reverse order to avoid collisions
    if (by > 0)
    {
        for (int i = MaxSampNumber; i >= MinSampNumber; i--)
        {
            if (bEALayer3Mode)
            {
                sprintf(catOldFilename, "%s%s%d.snr", foldername, path_separator, i);
                sprintf(catNewFilename, "%s%s%d.snr", foldername, path_separator, i + by);
            }
            else
            {
                sprintf(catOldFilename, "%s%s%d.asf", foldername, path_separator, i);
                sprintf(catNewFilename, "%s%s%d.asf", foldername, path_separator, i + by);
            }
            
            cout << "Renaming: " << catOldFilename << " >> " << catNewFilename << '\n';
            rename(catOldFilename, catNewFilename);


            if (bEALayer3Mode)
            {
                sprintf(catOldFilename, "%s%s%d.sns", foldername, path_separator, i);
                sprintf(catNewFilename, "%s%s%d.sns", foldername, path_separator, i + by);
                cout << "Renaming: " << catOldFilename << " >> " << catNewFilename << '\n';
                rename(catOldFilename, catNewFilename);
            }
        }
    }
    if (by < 0)
    {
        for (int i = MinSampNumber; i <= MaxSampNumber; i++)
        {
            if (bEALayer3Mode)
            {
                sprintf(catOldFilename, "%s%s%d.snr", foldername, path_separator, i);
                sprintf(catNewFilename, "%s%s%d.snr", foldername, path_separator, i + by);
            }
            else
            {
                sprintf(catOldFilename, "%s%s%d.asf", foldername, path_separator, i);
                sprintf(catNewFilename, "%s%s%d.asf", foldername, path_separator, i + by);
            }

            rename(catOldFilename, catNewFilename);

            if (bEALayer3Mode)
            {
                sprintf(catOldFilename, "%s%s%d.sns", foldername, path_separator, i);
                sprintf(catNewFilename, "%s%s%d.sns", foldername, path_separator, i + by);
                rename(catOldFilename, catNewFilename);
            }
        }
    }
    return 0;
}

// music add + concat stuff end

int main(int argc, char* argv[])
{
    cout << "EA Pathfinder v5 MPF tool\n";

    if (argc < 2)
    {
        cout \
            << "USAGE (decompile MPF to TXT): " << argv[0] << " MPFfile [OutFile]\n" \
            << "USAGE (compile TXT to MPF): " << argv[0] << " -c sourceMapFile [MPFout]\n"\
            << "USAGE (extract by sample num): " << argv[0] << " -s MPFfile MusTrackFile SampleNumber [OutSampleFile]\n"\
            << "USAGE (extract all samples): " << argv[0] << " -sa MPFfile MusTrackFile [OutSampleFolder]\n"\
            << "USAGE (update samples): " << argv[0] << " -su MPFfile SampleFolder\n"\
            << "USAGE (shift sample names): " << argv[0] << " -ss SampleFolder shiftAmount\n"\
            << "USAGE (append a new slot): " << argv[0] << " -a sourceMapFile [TrackID] [SectionID]\n"\
            << "USAGE (append a new slot (NFS Pro Street)): " << argv[0] << " -ap sourceMapFile [TrackID] [SectionID]\n"\
            << "USAGE (concat files): " << argv[0] << " -t destinationMapFile sourceMapFile\n"\
            << "\n"\
            << "For sample updating, the MUS file will be generated with the name of the MPF and placed next to it. You MUST have all files from the lowest to highest number!\n"\
            << "If you omit the optional [out] name, it'll inherit the name of the input file.\n"\
            << "The compiler is not very well written, so please follow the decompilation syntax closely!\n"\
            << "Newly compiled files do not contain samples. Add them with the update samples command.\n";
        return -1;
    }

    if ((argv[1][0] == '-') && argv[1][1] == 'c')
    {
        if (argc < 4)
        {
            strcpy(OutPath, argv[2]);
            char* extpoint = strrchr(OutPath, '.');
            if (extpoint)
                *extpoint = '\0';
            strcat(OutPath, ".mpf");
        }
        else
            strcpy(OutPath, argv[argc - 1]);
        cout << "Compiling MPF source to: " << OutPath << '\n';
        return MPFCompiler(argv[2], OutPath);
    }

    // sample actions
    if ((argv[1][0] == '-') && argv[1][1] == 's')
    {
        // all samples
        if (argv[1][2] == 'a')
        {
            if (argc < 4)
            {
                cout << "ERROR: not enough arguments for sample extraction!\n"\
                     << "USAGE (extract all samples): " << argv[0] << " -sa MPFfile MusTrackFile [OutSampleFolder]\n";
                return -1;
            }

            if (argc < 5)
            {
                strcpy(OutPath, argv[3]);
                char* extpoint = strrchr(OutPath, '.');
                if (extpoint)
                    *extpoint = '\0';
            }
            else
                strcpy(OutPath, argv[argc - 1]);

            cout << "Extracting all samples to: " << OutPath << '\n';

            return MPF_ExtractSamples(argv[2], argv[3], OutPath, -1);
        }

        // update samples in MPF
        if (argv[1][2] == 'u')
        {
            if (argc < 4)
            {
                cout << "ERROR: not enough arguments for sample updating!\n"\
                     << "USAGE (update samples): " << argv[0] << " -su MPFfile SampleFolder\n";
                return -1;
            }

            cout << "Updating samples in: " << argv[argc - 1] << '\n';

            return MPF_UpdateSamples(argv[2], argv[argc - 1]);
        }

        // shift sample indicies
        if (argv[1][2] == 's')
        {
            if (argc < 4)
            {
                cout << "ERROR: not enough arguments for shifting!\n"\
                     << "USAGE (shift sample names): " << argv[0] << " -ss SampleFolder shiftAmount\n";
                return -1;
            }
            return ShiftSampleNamesInFolder(argv[2], stoi(argv[3]));
        }

        // sample by index
        if (argc < 5)
        {
            cout << "ERROR: not enough arguments for sample extraction!\n"\
                 << "USAGE (extract by sample num): " << argv[0] << " -s MPFfile MusTrackFile SampleNumber [OutSampleFile]\n";
            return -1;
        }

        int si = stoi(argv[4]);
        if (si <= 0)
            si = 1;

        if (argc < 6)
            return MPF_ExtractSamples(argv[2], argv[3], NULL, si);
        else
            return MPF_ExtractSamples(argv[2], argv[3], argv[argc - 1], si);
    }
    // appending mode
    if ((argv[1][0] == '-') && argv[1][1] == 'a')
    {
        uint32_t TrackID = 0x11000001;
        uint32_t SectionID = 0;

        if (argc < 3)
        {
            cout << "ERROR: not enough arguments for appending!\n"\
                 << "USAGE (append a new slot): " << argv[0] << " -a[p] sourceMapFile [TrackID] [SectionID]\n";
            return -1;
        }

        if (FindLastBits(argv[2]) < 0)
        {
            cout << "ERROR: Can't parse file during finding indicies for appending!\n";
            return -1;
        }

        if (argc > 3)
        {
            TrackID = stoul(argv[3], nullptr, 16);
        }

        if (argc > 4)
        {
            SectionID = stoul(argv[4]);
        }

        if (AppendSlot(argv[2], (argv[1][2] == 'p'), TrackID, SectionID) < 0)
        {
            cout << "ERROR: Can't append the slot!\n";
            return -1;
        }
        return 0;
    }

    // concat mode
    if ((argv[1][0] == '-') && argv[1][1] == 't')
    {
        if (argc < 4)
        {
            cout << "ERROR: not enough arguments for concat!\n"\
                 << "USAGE (concat files): " << argv[0] << " -t destinationMapFile sourceMapFile\n";
            return -1;
        }


        if (FindLastBits(argv[2]) < 0)
        {
            cout << "ERROR: Can't parse file during finding indicies for concating!\n";
            return -1;
        }

        if (ConcatMaps(argv[2], argv[3]) < 0)
        {
            cout << "ERROR: Can't concat!\n";
            return -1;
        }
        return 0;
    }

    if (argc < 3)
    {
        strcpy(OutPath, argv[1]);
        char* extpoint = strrchr(OutPath, '.');
        if (extpoint)
            *extpoint = '\0';
        strcat(OutPath, "_decomp.txt");
    }
    else
        strcpy(OutPath, argv[argc - 1]);

    cout << "Decompiling MPF to: " << OutPath << '\n';

    return MPFtoTXT(argv[1], OutPath);
}
