// final revision March 2nd, 2011

#define VERSION "0.4"


#include "pin.H"
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <string>
#include <stack>
#include <algorithm>
#include <map>

#include "Function.h"
#include "global.h"
#include "xmloutput.h"

#ifdef WIN32
#define DELIMITER_CHAR '\\'
#else 
#define DELIMITER_CHAR '/'
#endif

using namespace std;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

char main_image_name[40];

Function *currFunc;
stack<Function *> callStack;


map<string, Function *> functions;


BOOL Monitor_ON = FALSE;
BOOL evaluate_libs = FALSE;

BOOL No_Stack_Flag = FALSE;   // a flag showing our interest to include or exclude stack memory accesses in analysis. The default value indicates tracing also the stack accesses. Can be modified by 'ignore_stack_access' command line switch

string monitorfilename, reportfilename,q2xmlfilename;
string q2app;
BOOL Q2_Mode = FALSE;
string MAIP::g_namespace;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
        "output_file","MAIProfile.txt", "Specify the output file name. Default: MAIProfile.txt.");

KNOB<string> KnobQ2XMLFile(KNOB_MODE_WRITEONCE, "pintool",
        "q2_xmlfile","q2profiling.xml", "Specify the Q2 XML file. Default: q2profiling.xml.");

KNOB<string> KnobMonitorList(KNOB_MODE_WRITEONCE, "pintool",
        "use_monitor_list","", "Create output report files only for certain function(s) in the application and filter out the rest (the functions are listed in a text file whose name follows)");

// the following knobs are used in the scripting in the kernel library, the tool must accept them, though they are not used

KNOB<BOOL> KnobQ2Mode(KNOB_MODE_WRITEONCE, "pintool",
        "q2mode","0", "Run MAIP as part of Q2 -> use Q2 xml file and application");

KNOB<string> KnobQ2App(KNOB_MODE_WRITEONCE, "pintool",
        "q2app","", "Q2 application name");

KNOB<string> KnobQ2Namespace(KNOB_MODE_WRITEONCE, "pintool",
        "q2ns","q2", "Q2 namespace, default: q2");

KNOB<BOOL> KnobIgnoreUncommonFNames(KNOB_MODE_WRITEONCE, "pintool",
        "filter_uncommon_functions","1", "Filter out uncommon function names which are unlikely to be defined by user (beginning with question mark, underscore(s), etc.)");

KNOB<BOOL> KnobIgnoreStackAccess(KNOB_MODE_WRITEONCE, "pintool",
        "ignore_stack_access","0", "Ignore memory accesses within application's stack region");

KNOB<BOOL> KnobUseLibraries(KNOB_MODE_WRITEONCE, "pintool",
        "use_libraries","0", "Use also libraries");

KNOB<BOOL> KnobVersion(KNOB_MODE_WRITEONCE, "pintool",
        "version","0", "print version information");


/* ===================================================================== */
class PrintFunction 
{
    private:
        ostream &_of;
    public:
        PrintFunction(ostream &of): _of(of) { }
        void operator() (std::pair<string, Function *> tuple)
        {
            (tuple).second->processCall(true);
            (tuple).second->print(_of);
        }
};

//------------------------------------------------------------------
VOID PrintReport()
{

    ofstream of;

    of.open(reportfilename.c_str());

    if(of.bad()) 
    {
        cerr<<"Can't create the report file..."<<endl;
        return;
    }

    of  << "Statistics are reported for each function in the main image file not auxiliary functions.\n"
        << "If there is more than one call for a function, we shall report for each call that carry different statistics.\n";
    of.precision(2);

    PrintFunction printer(of);
    for_each(functions.begin(), functions.end(),printer);

    of.close();
}

//------------------------------------------------------------------

VOID EnterFC(char *arg_name, bool flag, RTN rtn) 
{
    string name = arg_name;

    // revise the following in case you want to exclude some unwanted functions under Windows and/or Linux
    if (!evaluate_libs && !flag) return;   // not found in the main image, so skip the current function name update

#ifdef WIN32
    if(		
            name[0]=='_' ||
            name[0]=='?' ||
            !name.compare("GetPdbDll") || 
            !name.compare("DebuggerRuntime") || 
            !name.compare("atexit") || 
            !name.compare("failwithmessage") ||
            !name.compare("pre_c_init") ||
            !name.compare("pre_cpp_init") ||
            !name.compare("mainCRTStartup") ||
            !name.compare("NtCurrentTeb") ||
            !name.compare("check_managed_app") ||
            !name.compare("DebuggerKnownHandle") ||
            !name.compare("DebuggerProbe") ||
            !name.compare("failwithmessage") ||
            !name.compare("unnamedImageEntryPoint")
      ) return;
#else
    if( name[0]=='_' ||
		name[0]=='.' || 
		name[0]=='?' || 
        !name.compare("call_gmon_start") || 
        !name.compare("frame_dummy") 
      ) 
		return;
#endif


    // update the current function name
    uint32_t n = functions.size();
    currFunc = functions[name];
    if(n<functions.size()) {
        string sourcefile;

        PIN_LockClient();
        PIN_GetSourceLocation( RTN_Address(rtn), NULL, NULL, &sourcefile); 
        PIN_UnlockClient();

        currFunc = functions[name] = new Function(name);
        currFunc->Source() =  sourcefile;
    }

    currFunc->processCall();

    callStack.push(currFunc);
    //cout << "calling: " << name << endl;
}


/* ===================================================================== */

INT32 Version()
{
    cerr << "MAIP version " << VERSION << " (c) 2010,2011, CE Lab, Delft University of Technology" << endl
        << "MAIP provides some memory access related statistics for each function in an application." << endl;

    return -1;
}

INT32 Usage()
{
    Version();
    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}


/* ===================================================================== */


VOID  Return(VOID *ip)
{
    string fn_name = RTN_FindNameByAddress((ADDRINT)ip);

    if(! (callStack.empty()) && ((callStack.top())->getName()==fn_name) )
    {  
        callStack.pop();
        //cout << "returning from: " << fn_name << endl;
    } // end of if I like the name of this function which is returning control!

}

/* ===================================================================== */

VOID Instruction(INS ins, VOID *v);

VOID UpdateCurrentFunctionName(RTN rtn,VOID *v)
{

    bool flag;
    string RName=RTN_Name(rtn);

    // in case this function is in the monitor list.
    if((!Monitor_ON || functions.count(RName)>0))
    {
        if(!Monitor_ON || functions[RName]->isMonitored())
        {

            const char *s=RName.c_str();

            flag=(!((IMG_Name(SEC_Img(RTN_Sec(rtn))).find(main_image_name)) == string::npos));
            //cout << "RName: " << RName << " - " << flag << endl;

            RTN_Open(rtn);


            // Insert a call at the entry point of a routine to push the current routine to callStack
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)EnterFC, IARG_PTR, s,  IARG_BOOL, flag, IARG_PTR, rtn, IARG_END);

            // Insert a call at the exit point of a routine to pop the current routine from callStack if we have the routine on the top
            // RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)exitFc, IARG_PTR, RName.c_str(), IARG_END);

            // For each instruction of the routine
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
                // Insert a call to docount to increment the instruction counter for this rtn
                Instruction(ins,0);
            }

            RTN_Close(rtn);	
        }
    }

}
/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
    cerr << "\nFinished executing the instrumented application..." << endl;
    cerr << "\nCreating the report file..." << endl;
    PrintReport();
    if(Q2_Mode) Q2XMLReport(q2xmlfilename,q2app,functions);

    cerr << "done!" << endl;
}

// increment routine for the total instruction counter of the current function
static VOID IncreaseTotalInstCounter(UINT32 size)
{
    callStack.top()->processInstruction(size);
}

// this is the main function which has all the details for the words read/written
static VOID UpdateMemAccInfo(VOID * ip, VOID * ESP, CHAR r, VOID * addr, INT32 size, BOOL isPrefetch, BOOL SameInst)
{
    callStack.top()->updateMemInfo(ip,ESP,r,addr,size,isPrefetch,SameInst);	
}

static VOID MemAccDetected_2nd_read(VOID * ip, VOID * ESP, VOID * addr, INT32 size, BOOL isPrefetch)
{
    // just in case for future improvements you want to compute something extra for the instructions having two read operations!
    UpdateMemAccInfo(ip, ESP, 'R', addr, size, isPrefetch, TRUE);

}

//------------------------------------
static VOID *WriteAdd;
static INT32 WriteSize;
//------------------------------------
static VOID Record_WriteAddSize(VOID * addr, INT32 size)
{
    WriteAdd=addr;
    WriteSize=size;
}
//---------------------------------------------------------------------------------

static VOID RecordWrite(VOID * ip, VOID *StackP, BOOL SameInst)
{
    // is this part of a read/write instruction?

    UpdateMemAccInfo(ip,StackP,'W',WriteAdd,WriteSize,FALSE,SameInst);
}
//--------------------------------------------------------------------------------

static VOID RecordOpRead(VOID * ip, VOID * addr, VOID * ESP, UINT32 size)
{
    callStack.top()->processOperand(ip,addr,ESP,size,true);
}
//--------------------------------------------------------------------------------
static VOID RecordOpWrite(VOID * ip, VOID * addr, VOID * ESP, UINT32 size)
{
    callStack.top()->processOperand(ip,addr,ESP,size,false);
}
//--------------------------------------------------------------------------------

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    BOOL do_not_inc_inst_count=FALSE; // this ensures that for a single instruction with combined read/write we get accurate statistics
    UINT32 opsize;	
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    UINT32 nOperands = INS_OperandCount(ins);

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncreaseTotalInstCounter, IARG_UINT32, nOperands,IARG_END);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        opsize=INS_MemoryOperandSize(ins,memOp);

        if (INS_MemoryOperandIsRead(ins, memOp))

            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordOpRead,
                    IARG_INST_PTR,
                    IARG_MEMORYOP_EA, memOp,
                    IARG_REG_VALUE, REG_STACK_PTR,
                    IARG_UINT32,opsize,	
                    IARG_END);
        // Note that in some architectures a single memory operand can be
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.

        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            if(INS_HasFallThrough(ins)) // false for branch and calls, but otherwise is true (the control flow is not changed)
                // IPOINT_AFTER can not be used for branches and calls!
            {
                INS_InsertPredicatedCall(ins, IPOINT_AFTER, (AFUNPTR)RecordOpWrite,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA, memOp,
                        IARG_REG_VALUE, REG_STACK_PTR,
                        IARG_UINT32,opsize,	
                        IARG_END);
            }
            if(INS_IsBranchOrCall(ins)) // IPOINT_TAKEN_BRANCH is only valid for branches and calls!
            {
                INS_InsertPredicatedCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)RecordOpWrite,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA, memOp,
                        IARG_REG_VALUE, REG_STACK_PTR,
                        IARG_UINT32,opsize,	
                        IARG_END);
            }
        }
    }
    // Inspect the instruction to get more info
    if ( INS_IsMemoryRead(ins) )
    {	
        do_not_inc_inst_count=TRUE;
        INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)UpdateMemAccInfo,
                IARG_INST_PTR,
                IARG_REG_VALUE, REG_STACK_PTR,
                IARG_UINT32, 'R',
                IARG_MEMORYREAD_EA,
                IARG_MEMORYREAD_SIZE,
                IARG_BOOL, INS_IsPrefetch(ins),
                IARG_BOOL,FALSE,
                IARG_END);
    }
    if (INS_HasMemoryRead2(ins))
    {         
        INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)MemAccDetected_2nd_read,
                IARG_INST_PTR,
                IARG_REG_VALUE, REG_STACK_PTR,
                IARG_MEMORYREAD2_EA,
                IARG_MEMORYREAD_SIZE,
                IARG_UINT32, INS_IsPrefetch(ins),
                IARG_END);
    }
    if ( INS_IsMemoryWrite(ins) )
    {
        INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)Record_WriteAddSize,
                IARG_MEMORYWRITE_EA,
                IARG_MEMORYWRITE_SIZE,
                IARG_END);

        if(INS_HasFallThrough(ins)) // if false for branch and calls, but otherwise is true (the control flow is not changed)
            // IPOINT_AFTER can not be used for branches and calls!
            INS_InsertPredicatedCall(
                    ins, IPOINT_AFTER, (AFUNPTR)RecordWrite, 
                    IARG_INST_PTR,
                    IARG_REG_VALUE, REG_STACK_PTR,
                    IARG_BOOL,do_not_inc_inst_count,
                    IARG_END);

        if(INS_IsBranchOrCall(ins)) // IPOINT_TAKEN_BRANCH is only valid for branches and calls!
            INS_InsertPredicatedCall(
                    ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)RecordWrite, 
                    IARG_INST_PTR,
                    IARG_REG_VALUE, REG_STACK_PTR,
                    IARG_BOOL,do_not_inc_inst_count,
                    IARG_END);
    } // end of IsMemoryWrite()


    if (INS_IsRet(ins))   // track return instruction to feed the current function statistics to the MemList
    {
        INS_InsertPredicatedCall(
                ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)Return,
                IARG_INST_PTR,
                IARG_END);
    }

}
/////////////////////////////////////////////////////////////////////////
const char * StripPath(const char * path)
{
    const char * file = strrchr(path,DELIMITER_CHAR);
    if (file)
        return file+1;
    else
        return path;
}
/* ===================================================================== */

int  main(int argc, char *argv[])
{
    char temp[100];


    if(PIN_Init(argc,argv))
        return Usage();

    if(KnobVersion.Value())
        return Version();

    PIN_InitSymbols();

    monitorfilename = KnobMonitorList.Value();
    reportfilename = KnobOutputFile.Value(); 
    evaluate_libs = KnobUseLibraries.Value();

    Q2_Mode = KnobQ2Mode.Value();
    q2xmlfilename = KnobQ2XMLFile.Value();
    q2app = KnobQ2App.Value();
    g_namespace = KnobQ2Namespace.Value() + ":";

    // assume 'Out_of_the_main_function_scope' as the first current routine
    currFunc = new Function("Out_of_the_main_function_scope");
    functions[currFunc->getName()] = currFunc;

    callStack.push(currFunc);

    // parse the command line arguments to get the main image name
    for (int i=1;i<argc-1;i++)
    {
        if (!strcmp(argv[i],"-use_monitor_list")) Monitor_ON = TRUE;
        if (!strcmp(argv[i],"--"))
            strcpy(temp,argv[i+1]);
    }

    strcpy(main_image_name,StripPath(temp));

    //cerr<<main_image_name<<endl;

    // ------------------ Monitorlist file processing ---------------------------------------   

    if (Monitor_ON)  // user is interested in filtering out 
    {
        ifstream monitorin;

        monitorin.open(monitorfilename.c_str());

        if (!monitorin)
        {
            cerr<<"\nCan not open the monitor list file ("<<monitorfilename.c_str()<<")... Aborting!\n";
            return 4;
        }


        string item;

        monitorin>>item;    // get the next function name in the monitor list
        while(!monitorin.eof()) {
            cout << item << " added..." << endl; 
            functions[item] = new Function(item,true);
            monitorin>>item;    // get the next function name in the monitor list
        };

        monitorin.close();
    }
    // -----------------------------------------------------------------------------------------   


    RTN_AddInstrumentFunction(UpdateCurrentFunctionName,0);
    //    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    cerr << "Starting the application to be analysed..." << endl;
    PIN_StartProgram();

    return 0;
}
