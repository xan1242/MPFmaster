using ManagedBass;
using System;
using System.Globalization;
using System.Linq;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.TreeView;

namespace WinFormsApp1
{

    public struct PATHFINDHEADER
    {
        /* 0x0000 */ int id;
        /* 0x0004 */ byte majorRev;
        /* 0x0005 */ byte minorRev;
        /* 0x0006 */ byte release;
        /* 0x0007 */ byte prerelease;
        /* 0x0008 */ short saveIncrement;
        /* 0x000a */ short generateID;
        /* 0x000c */ byte projectID;
        /* 0x000d */ byte numtracks;
        /* 0x000e */ byte numsections;
        /* 0x000f */ byte numevents;
        /* 0x0010 */ byte numrouters;
        /* 0x0011 */ byte numnamedvars;
        /* 0x0012 */ short numnodes;
        /* 0x0014 */ uint nodeoffsets;
        /* 0x0018 */ uint nodedata;
        /* 0x001c */ uint eventoffsets;
        /* 0x0020 */ uint eventdata;
        /* 0x0024 */ uint namedvars;
        /* 0x0028 */ uint noderouters;
        /* 0x002c */ uint trackoffsets;
        /* 0x0030 */ uint trackinfos;
        /* 0x0034 */ uint sampleoffsets;
        /* 0x0038 */ uint mapfilelen;
        /* 0x003c */ //uint v40reserve[];
    }; /* size: 0x0048 */

    public enum PATHACTIONTYPE
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

    public enum PATHVALUETYPE
    {
        PATH_VALUE_BADTYPE = 0,
        PATH_VALUE_SPECIAL = 1,
        PATH_VALUE_VARIABLE = 2,
        PATH_VALUE_INTEGER = 3,
        PATH_VALUE_MAXTYPES = 4,
    };

    public enum PATHFADEWHAT
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

    public enum PATHSPECIALVALUETYPE
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

    public enum PATHOPERATOR
    {
        PATH_OPERATOR_INVALID = 0,
        PATH_OPERATOR_ADD = 1,
        PATH_OPERATOR_SUB = 2,
        PATH_OPERATOR_MULT = 3,
        PATH_OPERATOR_DIV = 4,
        PATH_OPERATOR_MOD = 5,
        PATH_OPERATOR_MAX = 6,
    };

    public enum PATHCOMPARE
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

    public struct PATHNODEEVENT
    {
        public uint eventID;
    }

    public struct PATHFINDNODE
    {

        public int index; /* bit position: 16 */
        public uint trackID; /* bit position: 11 */
        public uint sectionID; /* bit position: 5 */
        public int repeat; /* bit position: 0 */
        public int routerID; /* bit position: 20 */
        public uint numbranches; /* bit position: 15 */
        public uint controller; /* bit position: 12 */
        public uint beats; /* bit position: 8 */
        public uint bars; /* bit position: 0 */
        public uint partID; /* bit position: 16 */
        public uint synchevery; /* bit position: 12 */
        public uint synchoffset; /* bit position: 8 */
        public uint notes; /* bit position: 4 */
        public uint synch; /* bit position: 2 */
        public uint channelbranching; /* bit position: 1 */
    };

    public struct PATHNAMEDVAR
    {
        public string name;
        public int value;
    };

     struct PATHFINDBRANCH
    {
        public int controlmin;
        public int controlmax;
        public ushort dstnode;
    }
    public struct PATHACTCONDITION
    {
       public int value;
       public int comparevalue;
    };

    public struct PATHACTFADE
    {
        public uint tovol;
        public int id;
        public uint flip;
        public uint ms;
    };

    public struct PATHACTWAITTIME
    {
        public int millisecs;
        public int lowest;
    };

    public struct PATHACTBRANCHTO
    {
        public int node;
        public int ofsection;
        public int immediate;
    };

    public struct PATHACTSETVALUE
    {
        public int towhat;
        public uint setwhat;
    };

    public struct PATHACTEVENT
    {
        public uint eventID;
    }

    public struct PATHACTCALC
    {
        public uint value;
        public uint op;
        public int by;
    }
    public struct PATHACT
    {
        public PATHACTCONDITION only;
        public PATHACTWAITTIME waittime;
        //public PATHACTWAITBEAT waitbeat;
        public PATHACTBRANCHTO branch;
        public PATHACTFADE fade;
       // public PATHACTSFXFADE sfxfade;
        //public PATHACTDRYFADE dryfade;
       // public PATHACTPITCHFADE pitchfade;
       // public PATHACTSTRETCHFADE stretchfade;
        public PATHACTSETVALUE setval;
        public PATHACTEVENT evt;
       // public PATHACTCALLBACK callback;
        public PATHACTCALC calc;
       // public PATHACTPAUSE pause;
       // public PATHLOADBANK loadbank;
    }
    public struct PATHACTION
    {
        public int track;
        public int sectionID;
        public uint type;
        public uint done;
        public uint leftvaluetype;
        public uint rightvaluetype;
        public uint assess;
        public uint comparison;
        public uint indent;
        public PATHACT act;
    }

    struct PATHEVENT
    {
    /* 0x0000 */ public uint queued;
    /* 0x0004 */ public uint expiry;
    /* 0x0008 */ public uint lastact;
    /* 0x000c */ public uint eventID; /* bit position: 8 */
   // struct /* bitfield */
   // {
        /* 0x000c */ public uint numactions; /* bit position: 0 */
        /* 0x0010 */ public uint currentaction; /* bit position: 24 */
        /* 0x0010 */ public uint voices; /* bit position: 20 */
        /* 0x0010 */ public int priority; /* bit position: 16 */
        /* 0x0010 */ public uint bumplower; /* bit position: 15 */
        /* 0x0010 */ public uint beingFiltered; /* bit position: 14 */
        /* 0x0010 */ public int project; /* bit position: 11 */
    //}; /* bitfield */
    /* 0x0010 */ public int unused; /* bit position: 0 */
    }; /* size: 0x0014 */

    struct prsNODE
    {
        public PATHFINDNODE node;
        public List<PATHFINDBRANCH> branches;
        public int index;
    }

    struct prsPATHEVENT
    {
        public PATHEVENT evt;
        public List<PATHACTION> actions;
    }

    enum PATHASSESS
    {
        PATHASSESS_NONE,
        PATHASSESS_IF,
        PATHASSESS_ELIF,
        PATHASSESS_ELSE,
        PATHASSESS_ENDIF,
        PATHASSESS_MAX
    };


    public partial class Form1 : Form
    {
        int music;
        byte[] pushbuffer;
        int counter;

        int intensity_controller = 0;
        int cur_node = 0;
        int cur_sample = 0;
        int cur_eventid = 0;
        int next_branchto = 0;
        int next_sample = 0;

        int next_intensity = 0;
        int initial_node = 0;

        bool bReadyToQueue = false;
        string queued_file;

        string sample_folder;
        string sample_ext;

        bool bPausedPlayback = false;

        //PATHNAMEDVAR[] namedvars;
        //List<PATHNAMEDVAR> namedvars = new List<PATHNAMEDVAR>();
        List<prsNODE> nodes = new List<prsNODE>();
        //List<prsPATHEVENT> prsEvents = new List<prsPATHEVENT>();

        string[] ActionStrings =
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

        string[] paAssessments =
        {
            "None",
            "If",
            "Elif",
            "Else",
            "Endif"
        };

        PATHASSESS GetPATHASSESS(string name)
        {
            for (int i = 0; i < (int)PATHASSESS.PATHASSESS_MAX; i++)
            {
                if (name.Contains(paAssessments[i]))
                {
                    return (PATHASSESS)i;
                }
            }

            return PATHASSESS.PATHASSESS_NONE;
        }

        PATHACTIONTYPE GetPATHACTIONTYPE(string name)
        {
            for (int i = 0; i < (int)PATHACTIONTYPE.PATHACTION_MAX; i++)
            {
                if (name.Contains(ActionStrings[i]))
                {
                    return (PATHACTIONTYPE)i;
                }

                if (GetPATHASSESS(name) != PATHASSESS.PATHASSESS_NONE)
                    return PATHACTIONTYPE.PATHACTION_CONDITION;
            }

            return PATHACTIONTYPE.PATHACTION_NONE;
        }

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

        public string[] cmpPathNodeStrings =
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

        cmpPathNodeParser cmpGetPathNodeParserType(string line)
        {
            for (int i = 0; i < (int)cmpPathNodeParser.cmpPNP_MAX; i++)
            {
                if (line.Contains(cmpPathNodeStrings[i]))
                {
                    return (cmpPathNodeParser)i;
                }
            }

            return cmpPathNodeParser.cmpPNP_NULL;
        }

        void ParseMapFile(string fname)
        {
            StreamReader reader = File.OpenText(fname);
            string line;

            while ((line = reader.ReadLine()) != null)
            {
                if (!line.StartsWith('#'))
                {
                    //if (line.Contains("Var"))
                    //{
                    //    PATHNAMEDVAR var = new PATHNAMEDVAR();
                    //    reader.ReadLine();
                    //    line = reader.ReadLine();
                    //    //line.Replace("\t", "");
                    //    line.Trim();
                    //    string[] items = line.Split(' ');
                    //    var.name = items[0];
                    //    var.value = int.Parse(items[2]);
                    //    namedvars.Add(var);
                    //}
                    if (line.Contains("Node"))
                    {
                        prsNODE node = new prsNODE();

                        string[] items = line.Split(' ');
                        node.index = int.Parse(items[1]);

                        cmpPathNodeParser nodetype;
                        reader.ReadLine();
                        do
                        {
                            line = reader.ReadLine();
                            if (line.Contains("}"))
                                break;

                            nodetype = cmpGetPathNodeParserType(line);
                            items = line.Split(' ');

                            switch (nodetype)
                            {
                                case cmpPathNodeParser.cmpPNP_Wave:
                                    node.node.index = int.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Track:
                                    node.node.trackID = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Section:
                                    node.node.sectionID = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Part:
                                    node.node.partID = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Router:
                                    node.node.routerID = int.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Controller:
                                    node.node.controller = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Beats:
                                    node.node.beats = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Bars:
                                    node.node.bars = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_SynchEvery:
                                    node.node.synchevery = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_SynchOffset:
                                    node.node.synchoffset = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Notes:
                                    node.node.notes = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Synch:
                                    node.node.synch = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_ChannelBranching:
                                    node.node.channelbranching = uint.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Repeat:
                                    node.node.repeat = int.Parse(items[1]);
                                    break;
                                case cmpPathNodeParser.cmpPNP_Branches:
                                    // branches
                                    node.branches = new();
                                    reader.ReadLine();
                                    node.node.numbranches = 0;
                                    do
                                    {
                                        line = reader.ReadLine();

                                        if (line.Contains("Control"))
                                        {
                                            PATHFINDBRANCH newbranch = new PATHFINDBRANCH();
                                            line.Trim();
                                            items = line.Split(' ');
                                            string[] items2 = items[1].Split(',');
                                            newbranch.controlmin = int.Parse(items2[0]);
                                            newbranch.controlmax = int.Parse(items2[1]);
                                            newbranch.dstnode = ushort.Parse(items[3]);

                                            node.branches.Add(newbranch);
                                            node.node.numbranches = (uint)node.branches.Count();
                                        }
                                    } while (!line.Contains("}"));
                                    break;

                                default:
                                    break;
                            }
                        } while (!line.Contains("}"));
                        nodes.Add(node);
                    }

                    //if (line.Contains("Event"))
                    //{
                    //    prsPATHEVENT evt = new prsPATHEVENT();
                    //    evt.actions = new List<PATHACTION>();
                    //
                    //    line.Trim();
                    //    string[] items = line.Split(' ');
                    //    items[1] = items[1].Trim();
                    //    items[1] = items[1].Remove(0, 2);
                    //    evt.evt.eventID = uint.Parse(items[1], NumberStyles.HexNumber);
                    //
                    //    // actions
                    //    reader.ReadLine();
                    //    line = reader.ReadLine();
                    //    if (line.Contains("Actions"))
                    //    {
                    //        reader.ReadLine();
                    //        do
                    //        {
                    //
                    //            PATHACTION action = new PATHACTION();
                    //            PATHACTIONTYPE type;
                    //
                    //            line = reader.ReadLine();
                    //            if (line.Contains('}'))
                    //                break;
                    //
                    //            items = line.Split('\t');
                    //            string[] items2 = items[0].Split(',');
                    //            items2[0] = items2[0].Remove(0, 1);
                    //            items2[1] = items2[1].Trim();
                    //            items2[1] = items2[1].Replace(")", "");
                    //
                    //            action.track = int.Parse(items2[0]);
                    //            action.sectionID = int.Parse(items2[1]);
                    //
                    //            type = GetPATHACTIONTYPE(items[1]);
                    //            action.type = (uint)type;
                    //
                    //            switch (type)
                    //            {
                    //                case PATHACTIONTYPE.PATHACTION_BRANCHTO:
                    //                    //PATHACTBRANCHTO branch = new();
                    //                    items2 = items[1].Split(' ');
                    //
                    //                    action.act.branch.node = int.Parse(items2[1]);
                    //
                    //                    string[] items3 = items2[2].Split(',');
                    //                    items3[0] = items3[0].Remove(0, 1);
                    //                    items3[1] = items3[1].Trim();
                    //                    items3[1] = items3[1].Replace(")", "");
                    //
                    //                    action.act.branch.ofsection = int.Parse(items3[0]);
                    //                    action.act.branch.immediate = int.Parse(items3[1]);
                    //
                    //                    break;
                    //
                    //                default:
                    //                    break;
                    //            }
                    //
                    //            evt.actions.Add(action);
                    //        } while (!line.Contains("}"));
                    //
                    //    }
                    //
                    //    prsEvents.Add(evt);
                    //}
                }
            }
        }

        int DecodeData(string FilePath)
        {
            int decoder = Bass.CreateStream(FilePath, 0, 0, BassFlags.Decode);
            int DecodeSize = (int)Bass.ChannelGetLength(decoder, PositionFlags.Bytes);
            pushbuffer = new byte[DecodeSize];
            Bass.ChannelGetData(decoder, pushbuffer, DecodeSize);
            float DecodedSampleRate = 0.0f;
            Bass.ChannelGetAttribute(decoder, ChannelAttribute.Frequency, out DecodedSampleRate);
            Bass.ChannelSetAttribute(music, ChannelAttribute.Frequency, DecodedSampleRate);
            Bass.StreamFree(decoder);

            return DecodeSize;
        }
        void PushFile(string FilePath, int hHandle)
        {
            int DecodeSize = DecodeData(FilePath);
            Bass.StreamPutData(hHandle, pushbuffer, (int)DecodeSize);
        }
        void CreateStreamPushHandle()
        {
            if (music == 0)
                music = Bass.CreateStream(44100, 2, 0, StreamProcedureType.Push);
        }



        void QueueNode(int node)
        {
            short shnode = (short)node;
            node = shnode;

            if (node < 0)
            {
                node = cur_node + node;
            }

            prsNODE nd = nodes.ElementAt(node);
            bool bFoundBranch = false;

            //if (nd.node.index <= -1)
            //{
            //    return;
            //}

            next_sample = -1;
            next_branchto = -1;

            //if (nd.node.index > 0)
                next_sample = nd.node.index;

            for (int i = 0; i < nd.node.numbranches; i++)
            {
                if ((intensity_controller >= nd.branches.ElementAt(i).controlmin) && (intensity_controller <= nd.branches.ElementAt(i).controlmax))
                {
                    next_branchto = nd.branches.ElementAt(i).dstnode;
                    bFoundBranch = true;
                }
            }
            if ((!bFoundBranch) && (nd.node.numbranches > 0))
            {
                next_branchto = nd.branches.ElementAt(0).dstnode;
            }
            cur_node = node;
        }

        void HandlePushing()
        {
            long remaining = 0;

            remaining = Bass.StreamPutData(music, (byte[])null, 0);
            double time = Bass.ChannelBytes2Seconds(music, remaining);

            label1.Text = "Time: " + time.ToString("0.00") + " (bytes: " + remaining.ToString() + ")";
            label2.Text = "Pushing status: ";
            if ((time - 0.1) <= 0)
            {
                //string fname = String.Format("E:\\Need for Speed Most Wanted\\InteractiveMusic_ps2\\MW_Music.mus_{0}.asf.wav", counter);

                label2.Text += "Pushing";
                label3.Text = "File: " + queued_file;
                bReadyToQueue = true;


                //PushFile(fname, music);
                //counter++;
            }
            else
            {
                label2.Text += "Not pushing";
            }


        }
        void HandlePathfinderPlayback()
        {
            HandlePushing();

            if (next_branchto >= 0 && bReadyToQueue)
            {
                label4.Text = String.Format("Queued: {0}", next_branchto);
                QueueNode(next_branchto);
               
                if (next_sample > 0)
                {
                    if (music != 0)
                    {
                        queued_file = String.Format("{0}\\{1}{2}", sample_folder, next_sample, sample_ext);
                        PushFile(queued_file, music);
                    }
                }
                bReadyToQueue = false;
            }
        }


        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            music = 0;

            // 10ms update period
            Bass.UpdatePeriod = 10;

            if (!Bass.Init(-1, 44100, DeviceInitFlags.Latency))
            {
                MessageBox.Show("Can't initialize device");
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (music != 0 && (next_branchto >= 0) && !bPausedPlayback)
                HandlePathfinderPlayback();

            if (music != 0)
            {
                label7.Text = "Playback: ";
                switch (Bass.ChannelIsActive(music))
                {
                    case PlaybackState.Stalled:
                        label7.Text += "Stalled";
                        break;
                    case PlaybackState.Stopped:
                        label7.Text += "Stopped";
                        break;
                    case PlaybackState.Paused:
                        label7.Text += "Paused";
                        break;
                    case PlaybackState.Playing:
                        label7.Text += "Playing";
                        break;
                    default:
                        break;
                }
                label5.Text = "Intensity: " + intensity_controller;
                label6.Text = "Next node: " + next_branchto;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (music != 0)
            {
                Bass.ChannelStop(music);
                Bass.StreamFree(music);
                music = 0;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //music = Bass.CreateStream("C:\\Temp\\MW_Music.mus_0.asf.wav", 0, 0, BassFlags.Default);
            //ParseMapFile("D:\\Temp\\mpfmastertest\\MW_Music_decomp.txt");
            if (nodes.Count > 0)
            {
                bPausedPlayback = false;
                if (music == 0)
                {
                    next_branchto = initial_node;
                    CreateStreamPushHandle();
                }
                Bass.ChannelPlay(music, false);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (music != 0)
            {
                if (Bass.ChannelIsActive(music) == PlaybackState.Playing)
                {
                    Bass.ChannelPause(music);
                    bPausedPlayback = true;
                }
            }
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {
            int.TryParse(textBox2.Text, out next_intensity);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            intensity_controller = next_intensity;
            label5.Text = "Intensity: " + intensity_controller;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            int.TryParse(textBox1.Text, out initial_node);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            next_branchto = initial_node;
            label6.Text = "Next node: " + next_branchto;
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                if (sampleFolderDialog.ShowDialog() == DialogResult.OK)
                {
                    nodes.Clear();

                    sample_folder = Path.GetDirectoryName(sampleFolderDialog.FileName);
                    sample_ext = Path.GetExtension(sampleFolderDialog.FileName);
                    ParseMapFile(openFileDialog1.FileName);
                }
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}