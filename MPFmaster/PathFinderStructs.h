#pragma once
#include <stdlib.h>

enum PATHOPERATOR
{
    PATH_OPERATOR_INVALID = 0,
    PATH_OPERATOR_ADD = 1,
    PATH_OPERATOR_SUB = 2,
    PATH_OPERATOR_MULT = 3,
    PATH_OPERATOR_DIV = 4,
    PATH_OPERATOR_MOD = 5,
    PATH_OPERATOR_MAX = 6,
};

enum PATHCOMPARE
{
    PATH_COMPARE_INVALID = 0,
    PATH_COMPARE_EQUALS = 1,
    PATH_COMPARE_NOT_EQUAL = 2,
    PATH_COMPARE_GREATER_THAN = 3,
    PATH_COMPARE_LESS_THAN = 4,
    PATH_COMPARE_GREATER_OR_EQUAL = 5,
    PATH_COMPARE_LESS_OR_EQUAL = 6,
    PATH_COMPARE_MAX = 7,
};

enum PATHSPECIALVALUETYPE
{
    PATH_SPECIALVALUE_BAD = 0,
    PATH_CONTROLLER = 1,
    PATH_CURRENTNODE = 2,
    PATH_CURRENTPART = 3,
    PATH_CURRENTSECTION = 4,
    PATH_EVENTEXPIRY = 5,
    PATH_EVENTPRIORITY = 6,
    PATH_FXBUS = 7,
    PATH_FXDRYLEVEL = 8,
    PATH_FXSENDLEVEL = 9,
    PATH_MAINVOICE = 10,
    PATH_NEXTNODE = 11,
    PATH_NOBRANCHING = 12,
    PATH_NODEDURATION = 13,
    PATH_PAUSE = 14,
    PATH_PITCHMULT = 15,
    PATH_PLAYINGNODE = 16,
    PATH_PLAYSTATUS = 17,
    PATH_RANDOMSHORT = 18,
    PATH_TIMENOW = 19,
    PATH_TIMETONEXTBEAT = 20,
    PATH_TIMETONEXTBAR = 21,
    PATH_TIMETONEXTNODE = 22,
    PATH_VOLUME = 23,
    PATH_TIMESTRETCH = 24,
    PATH_BARDURATION = 25,
    PATH_BEATDURATION = 26,
    PATH_CURRENTCHANNELSET = 27,
    PATH_PLAYINGCHANNELSET = 28,
    PATH_SPECIALVALUE_MAX = 29,
};

enum PATHFADEWHAT
{
    PATH_FADE_INVALID = 0,
    PATH_FADE_VOLUME = 1,
    PATH_FADE_SFXSENDLEVEL = 2,
    PATH_FADE_DRYLEVEL = 3,
    PATH_FADE_PITCHMULT = 4,
    PATH_FADE_STRETCHMULT = 5,
    PATH_FADE_CHANNELVOL = 6,
    PATH_FADE_PANANGLE = 7,
    PATH_FADE_PANDISTANCE = 8,
    PATH_FADE_PANSIZE = 9,
    PATH_FADE_PANTWIST = 10,
    PATH_FADE_MAXTYPES = 11,
};

enum PATHFADETYPE
{
    PATH_FADE_LINEAR = 1,
    PATH_FADE_EQPOWER = 2,
    PATH_FADE_EXPONENTIAL = 3,
    PATH_FADE_COSINE = 4,
};
enum PATHVALUETYPE
{
    PATH_VALUE_BADTYPE = 0,
    PATH_VALUE_SPECIAL = 1,
    PATH_VALUE_VARIABLE = 2,
    PATH_VALUE_INTEGER = 3,
    PATH_VALUE_MAXTYPES = 4,
};

enum PATHACTIONTYPE
{
    PATHACTION_NONE = 0,
    PATHACTION_CONDITION = 1,
    PATHACTION_WAITTIME = 2,
    PATHACTION_WAITBEAT = 3,
    PATHACTION_BRANCHTO = 4,
    PATHACTION_FADE = 5,
    PATHACTION_DRYLEVELFADE = 6,
    PATHACTION_SFXFADE = 7,
    PATHACTION_SETVALUE = 8,
    PATHACTION_EVENT = 9,
    PATHACTION_FILTER_ON = 10,
    PATHACTION_FILTER_OFF = 11,
    PATHACTION_FILTER_CLEAR = 12,
    PATHACTION_CALLBACK = 13,
    PATHACTION_CALC = 14,
    PATHACTION_PAUSE = 15,
    PATHACTION_LOADBANK = 16,
    PATHACTION_PITCHFADE = 17,
    PATHACTION_STRETCHFADE = 18,
    PATHACTION_MAX = 19,
};

struct PATHFINDHEADER
{
    /* 0x0000 */ int32_t id;
    /* 0x0004 */ uint8_t majorRev;
    /* 0x0005 */ uint8_t minorRev;
    /* 0x0006 */ uint8_t release;
    /* 0x0007 */ uint8_t prerelease;
    /* 0x0008 */ uint16_t saveIncrement;
    /* 0x000a */ uint16_t generateID;
    /* 0x000c */ uint8_t projectID;
    /* 0x000d */ uint8_t numtracks;
    /* 0x000e */ uint8_t numsections;
    /* 0x000f */ uint8_t numevents;
    /* 0x0010 */ uint8_t numrouters;
    /* 0x0011 */ uint8_t numnamedvars;
    /* 0x0012 */ uint16_t numnodes;
    /* 0x0014 */ uint32_t nodeoffsets;
    /* 0x0018 */ uint32_t nodedata;
    /* 0x001c */ uint32_t eventoffsets;
    /* 0x0020 */ uint32_t eventdata;
    /* 0x0024 */ uint32_t namedvars;
    /* 0x0028 */ uint32_t noderouters;
    /* 0x002c */ uint32_t trackoffsets;
    /* 0x0030 */ uint32_t trackinfos;
    /* 0x0034 */ uint32_t sampleoffsets;
    /* 0x0038 */ uint32_t mapfilelen;
    /* 0x003c */ uint32_t v40reserve[3];
}; /* size: 0x0048 */

struct PATHNODEBEATS
{
    /* 0x0000 */ uint32_t forcesynch : 1; /* bit position: 31 */
    /* 0x0000 */ uint32_t playbeats : 1; /* bit position: 30 */
}; /* size: 0x0004 */

struct PATHNODEEVENT
{
    /* 0x0000 */ uint32_t eventID : 24; /* bit position: 8 */
}; /* size: 0x0004 */

struct PATHNODECHANNEL
{
    /* 0x0000 */ uint32_t eventID : 24; /* bit position: 8 */
    /* 0x0000 */ uint32_t channelset : 4; /* bit position: 4 */
}; /* size: 0x0004 */

union PATHNODEEXTRA
{
    union
    {
        /* 0x0000 */ PATHNODEBEATS beat;
        /* 0x0000 */ PATHNODEEVENT sendevent;
        /* 0x0000 */ PATHNODECHANNEL channelbranch;
    }; /* size: 0x0004 */
}; /* size: 0x0004 */

struct PATHFINDNODE
{
    /* 0x0000 */ int32_t index : 16; /* bit position: 16 */
    /* 0x0000 */ uint32_t trackID : 5; /* bit position: 11 */
    /* 0x0000 */ uint32_t sectionID : 6; /* bit position: 5 */
    //struct /* bitfield */
   // {
        /* 0x0000 */ int32_t repeat : 5; /* bit position: 0 */
        /* 0x0004 */ uint32_t routerID : 12; /* bit position: 20 */
        /* 0x0004 */ uint32_t numbranches : 5; /* bit position: 15 */
        /* 0x0004 */ uint32_t controller : 3; /* bit position: 12 */
        /* 0x0004 */ uint32_t beats : 4; /* bit position: 8 */
    //}; /* bitfield */
    //struct /* bitfield */
   // {
        /* 0x0004 */ uint32_t bars : 8; /* bit position: 0 */
        /* 0x0008 */ uint32_t partID : 16; /* bit position: 16 */
        /* 0x0008 */ uint32_t synchevery : 4; /* bit position: 12 */
        /* 0x0008 */ uint32_t synchoffset : 4; /* bit position: 8 */
        /* 0x0008 */ uint32_t notes : 4; /* bit position: 4 */
        /* 0x0008 */ uint32_t synch : 2; /* bit position: 2 */
        /* 0x0008 */ uint32_t channelbranching : 1; /* bit position: 1 */
    //}; /* bitfield */
    /* 0x0008 */ uint32_t unused : 1; /* bit position: 0 */
    /* 0x000c */// union PATHNODEEXTRA extra;
}; /* size: 0x0010 */

struct PATHEVENT
{
    /* 0x0000 */ uint32_t queued;
    /* 0x0004 */ uint32_t expiry;
    /* 0x0008 */ uint32_t lastact;
    /* 0x000c */ uint32_t eventID : 24; /* bit position: 8 */
   // struct /* bitfield */
   // {
        /* 0x000c */ uint32_t numactions : 8; /* bit position: 0 */
        /* 0x0010 */ uint32_t currentaction : 8; /* bit position: 24 */
        /* 0x0010 */ uint32_t voices : 4; /* bit position: 20 */
        /* 0x0010 */ int32_t priority : 4; /* bit position: 16 */
        /* 0x0010 */ uint32_t bumplower : 1; /* bit position: 15 */
        /* 0x0010 */ uint32_t beingFiltered : 1; /* bit position: 14 */
        /* 0x0010 */ int32_t project : 3; /* bit position: 11 */
    //}; /* bitfield */
    /* 0x0010 */ int32_t unused : 11; /* bit position: 0 */
}; /* size: 0x0014 */

struct PATHNAMEDVAR
{
    /* 0x0000 */ int8_t name[16];
    /* 0x0010 */ int32_t value;
}; /* size: 0x0014 */

struct PATHTRACKINFO
{
    /* 0x0000 */ uint32_t startingsample;
    /* 0x0004 */ uint16_t numsubbanks;
    /* 0x0006 */ uint16_t purgemode;
    /* 0x0008 */ uint32_t muschecksum;
    /* 0x000c */ uint32_t maxaram;
    /* 0x0010 */ uint32_t maxmram;
}; /* size: 0x0014 */

struct PATHFINDSAMPLE
{
    /* 0x0000 */ uint32_t offset;
    /* 0x0004 */ uint32_t duration;
}; /* size: 0x0008 */

struct PATHFADESTATS
{
    /* 0x0000 */ PATHFADEWHAT fadewhat;
    /* 0x0004 */ int16_t fadebus;
    /* 0x0006 */ int16_t fadefrom;
    /* 0x0008 */ int16_t fadeto;
    /* 0x000a */ int16_t fadenum;
    /* 0x000c */ uint32_t fadestart;
    /* 0x0010 */ uint32_t fadetime;
}; /* size: 0x0014 */

struct PATHFINDBRANCH
{
    /* 0x0000 */ int8_t controlmin;
    /* 0x0001 */ int8_t controlmax;
    /* 0x0002 */ uint16_t dstnode;
}; /* size: 0x0004 */

struct PATHACTSTRETCHFADE
{
    /* 0x0000 */ uint32_t tovol : 14; /* bit position: 18 */
    /* 0x0000 */ int32_t id : 3; /* bit position: 15 */
    /* 0x0000 */ uint32_t flip : 1; /* bit position: 14 */
    /* 0x0000 */ uint32_t ms : 14; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTCONDITION
{
    /* 0x0000 */ int32_t value : 16; /* bit position: 16 */
    /* 0x0000 */ int32_t compareValue : 16; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTWAITTIME
{
    /* 0x0000 */ int32_t millisecs : 16; /* bit position: 16 */
    /* 0x0000 */ int32_t lowest : 16; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTBRANCHTO
{
    /* 0x0000 */ int32_t node : 16; /* bit position: 16 */
    /* 0x0000 */ int32_t ofsection : 8; /* bit position: 8 */
    /* 0x0000 */ int32_t immediate : 1; /* bit position: 7 */
}; /* size: 0x0004 */

struct PATHACTSFXFADE
{
    /* 0x0000 */ uint32_t tovol : 8; /* bit position: 24 */
    /* 0x0000 */ int32_t id : 7; /* bit position: 17 */
    /* 0x0000 */ uint32_t flip : 1; /* bit position: 16 */
    /* 0x0000 */ uint32_t ms : 16; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTPITCHFADE
{
    /* 0x0000 */ uint32_t tovol : 14; /* bit position: 18 */
    /* 0x0000 */ int32_t id : 3; /* bit position: 15 */
    /* 0x0000 */ uint32_t flip : 1; /* bit position: 14 */
    /* 0x0000 */ uint32_t ms : 14; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHLOADBANK
{
    /* 0x0000 */ uint32_t subbanknum : 8; /* bit position: 24 */
    /* 0x0000 */ uint32_t unload : 8; /* bit position: 16 */
}; /* size: 0x0004 */

struct PATHACTSETVALUE
{
    /* 0x0000 */ int32_t towhat : 16; /* bit position: 16 */
    /* 0x0000 */ uint32_t setwhat : 8; /* bit position: 8 */
}; /* size: 0x0004 */

struct PATHACTEVENT
{
    /* 0x0000 */ uint32_t eventid : 24; /* bit position: 8 */
}; /* size: 0x0004 */

struct PATHACTCALC
{
    /* 0x0000 */ uint32_t value : 8; /* bit position: 24 */
    /* 0x0000 */ uint32_t op : 8; /* bit position: 16 */
    /* 0x0000 */ int32_t by : 16; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTWAITBEAT
{
    /* 0x0000 */ int32_t millisecs : 16; /* bit position: 16 */
    /* 0x0000 */ uint32_t every : 4; /* bit position: 12 */
    /* 0x0000 */ uint32_t note : 4; /* bit position: 8 */
    /* 0x0000 */ uint32_t offset : 4; /* bit position: 4 */
}; /* size: 0x0004 */

struct PATHACTFADE
{
    /* 0x0000 */ uint32_t tovol : 8; /* bit position: 24 */
    /* 0x0000 */ int32_t id : 7; /* bit position: 17 */
    /* 0x0000 */ uint32_t flip : 1; /* bit position: 16 */
    /* 0x0000 */ uint32_t ms : 16; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTDRYFADE
{
    /* 0x0000 */ uint32_t tovol : 8; /* bit position: 24 */
    /* 0x0000 */ int32_t id : 7; /* bit position: 17 */
    /* 0x0000 */ uint32_t flip : 1; /* bit position: 16 */
    /* 0x0000 */ uint32_t ms : 16; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTCALLBACK
{
    /* 0x0000 */ int32_t value : 16; /* bit position: 16 */
    /* 0x0000 */ uint32_t id : 16; /* bit position: 0 */
}; /* size: 0x0004 */

struct PATHACTPAUSE
{
    /* 0x0000 */ uint32_t when : 16; /* bit position: 16 */
    /* 0x0000 */ uint32_t on : 1; /* bit position: 15 */
}; /* size: 0x0004 */

union PATHACT
{
    union
    {
        /* 0x0000 */ PATHACTCONDITION only;
        /* 0x0000 */ PATHACTWAITTIME waittime;
        /* 0x0000 */ PATHACTWAITBEAT waitbeat;
        /* 0x0000 */ PATHACTBRANCHTO branch;
        /* 0x0000 */ PATHACTFADE fade;
        /* 0x0000 */ PATHACTSFXFADE sfxfade;
        /* 0x0000 */ PATHACTDRYFADE dryfade;
        /* 0x0000 */ PATHACTPITCHFADE pitchfade;
        /* 0x0000 */ PATHACTSTRETCHFADE stretchfade;
        /* 0x0000 */ PATHACTSETVALUE setval;
        /* 0x0000 */ PATHACTEVENT event;
        /* 0x0000 */ PATHACTCALLBACK callback;
        /* 0x0000 */ PATHACTCALC calc;
        /* 0x0000 */ PATHACTPAUSE pause;
        /* 0x0000 */ PATHLOADBANK loadbank;
    }; /* size: 0x0004 */
}; /* size: 0x0004 */

struct PATHACTION
{
    /* 0x0000 */ int32_t track;
    /* 0x0004 */ int32_t sectionID : 8; /* bit position: 24 */
    /* 0x0004 */ uint32_t type : 7; /* bit position: 17 */
    /* 0x0004 */ uint32_t done : 1; /* bit position: 16 */
    /* 0x0004 */ uint32_t leftvaluetype : 2; /* bit position: 14 */
    /* 0x0004 */ uint32_t rightvaluetype : 2; /* bit position: 12 */
    /* 0x0004 */ uint32_t assess : 3; /* bit position: 9 */
    /* 0x0004 */ uint32_t comparison : 3; /* bit position: 6 */
    /* 0x0004 */ uint32_t indent : 3; /* bit position: 3 */
    /* 0x0004 */ uint32_t unused : 3; /* bit position: 0 */
    /* 0x0008 */ PATHACT act;
}; /* size: 0x000c */

const char* specialTypes[] =
{
    "SPECIALVALUE_BAD",
    "CONTROLLER",
    "CURRENTNODE",
    "CURRENTPART",
    "CURRENTSECTION",
    "EVENTEXPIRY",
    "EVENTPRIORITY",
    "FXBUS",
    "FXDRYLEVEL",
    "FXSENDLEVEL",
    "MAINVOICE",
    "NEXTNODE",
    "NOBRANCHING",
    "NODEDURATION",
    "PAUSE",
    "PITCHMULT",
    "PLAYINGNODE",
    "PLAYSTATUS",
    "RANDOMSHORT",
    "TIMENOW",
    "TIMETONEXTBEAT",
    "TIMETONEXTBAR",
    "TIMETONEXTNODE",
    "VOLUME",
    "TIMESTRETCH",
    "BARDURATION",
    "BEATDURATION",
    "CURRENTCHANNELSET",
    "PLAYINGCHANNELSET"
};

const char* fadeTypes[] =
{
    "INVALID",
    "VOLUME",
    "SFXSENDLEVEL",
    "DRYLEVEL",
    "PITCHMULT",
    "STRETCHMULT",
    "CHANNELVOL",
    "PANANGLE",
    "PANDISTANCE",
    "PANSIZE",
    "PANTWIST",
};

const char* paAssessments[] =
{
    "None",
    "If",
    "Elif",
    "Else",
    "Endif"
};

enum PATHASSESS
{
    PATHASSESS_NONE,
    PATHASSESS_IF,
    PATHASSESS_ELIF,
    PATHASSESS_ELSE,
    PATHASSESS_ENDIF,
    PATHASSESS_MAX
};

const char* paSigns[] =
{
    "None",
    "==",
    "!=",
    ">",
    "<",
    ">=",
    "<="
};

const char* paCalcSigns[] =
{
    "None",
    "+",
    "-",
    "*",
    "/",
    "%"
};

#define PATHFINDER_MAGIC 0x50464478
#define PATH_SUPPORTED_VERSION 5

#define EA_SCHL_MAGIC 0x6C484353
#define EA_GSTR_MAGIC 0x52545347
