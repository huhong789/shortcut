#include "pin.H"
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <syscall.h>

// edited by hyihe
#include "happens_before.h"

//std::unordered_set<std::pair<ADDRINT, string>> locked_insts;

struct thread_data* current_thread; // Always points to thread-local data (changed by kernel on context switch)

#define INSTMNT_CALL_RET
//#define EXTRA_DEBUG

unsigned long print_limit = 0;
unsigned long int print_stop = UINT_MAX;
u_long* ppthread_log_clock;

KNOB<string> KnobPrintLimit(KNOB_MODE_WRITEONCE, "pintool", "p", "0", "syscall print start");
KNOB<string> KnobPrintStop(KNOB_MODE_WRITEONCE, "pintool", "s", "100000000", "syscall print stop");

// BEGIN GENERIC STUFF NEEDED TO REPLAY WITH PIN


// moved forward by hyihe
int fd; // File descriptor for the replay device
TLS_KEY tls_key; // Key for accessing TLS. 
int syscall_cnt = 0;

// edited by hyihe
#define MEM_REF_READ  0
#define MEM_REF_WRITE 1
#define MEM_REF_READ2 2

typedef std::map<var_key_t, var_t *, var_key_comp> var_map_t;
// current intervals of the threads
std::vector<interval_t *> thd_ints;
std::vector<int> thd_entr_type;
int num_threads = 0;
// the map containing all variables accessed in the program
var_map_t variables;

static inline bool inrange() {
    //return ((syscall_cnt >= print_limit) 
        //&& (syscall_cnt <= print_stop)); 
    if (*ppthread_log_clock > print_limit && *ppthread_log_clock <print_stop) 
    	return true;
    return false;
}

int get_record_pid()
{
    //calling kernel for this replay thread's record log
    int record_log_id;

    record_log_id = get_log_id (fd);
    if (record_log_id == -1) {
        int pid = PIN_GetPid();
        fprintf(stderr, "Could not get the record pid from kernel, pid is %d\n", pid);
        return pid;
    }
    return record_log_id;
}

//call this function with PIN_LOCKCLIENT held
ADDRINT find_static_address(ADDRINT ip)
{
	IMG img = IMG_FindByAddress(ip);
	if (!IMG_Valid(img)) {
		return ip;
	}
	ADDRINT offset = IMG_LoadOffset(img);
	return ip - offset;
}

bool detect_race(THREADID tid, VOID *ref_addr, ADDRINT size, ADDRINT ip, int ref_type) {
    if(!inrange()) {
        return false;
    }
    if ((unsigned long)ref_addr == 0x9d844a08 || (unsigned long)ref_addr == 0x9d844a04) {
	    fprintf (stderr, "tid %d addr %p ip %x\n", tid, ref_addr, ip);
    }
    // Ignore instructions from shared libraries/syscalls
    /*if((unsigned long)ip > 0x80000000 || num_threads == 1) {
        return false;
    }*/
    PIN_LockClient ();
    IMG img = IMG_FindByAddress(ip);
    if (IMG_Valid (img)) {
	    if (IMG_Name(img).find ("libpthread") != string::npos) {
		    PIN_UnlockClient ();
		    return false;
	    }
    }
    PIN_UnlockClient ();
    var_map_t::iterator lkup = variables.find(std::make_pair((void *)ref_addr, (int)size));
    if(lkup == variables.end()) {
        // No sharing ,conflict upon first access
        var_t *insert = new var_t;
        insert->resize_intvls(num_threads);
        insert->update_intvls(ref_type, thd_ints, tid);
        variables.insert(std::make_pair(std::make_pair((void *)ref_addr, (int)size), insert));
        return false;
    } else {
    	// Update varible access history and check for violations
        int race;
        bool ret = false;
        lkup->second->resize_intvls(num_threads);
        race = lkup->second->check_for_race(ref_type, thd_ints, tid);
        if(race) {
            fprintf(stderr, "race at %#x, addr %p, size %d\n", ip, ref_addr, size);
	    PIN_LockClient();
	    if (IMG_Valid(IMG_FindByAddress(ip))) {
		printf("%s -- img %s\n", RTN_FindNameByAddress(ip).c_str(), IMG_Name(IMG_FindByAddress(ip)).c_str());
	    }
	    PIN_UnlockClient();

            // do not abort, always return false
            //ret = true;
        }
        lkup->second->update_intvls(ref_type, thd_ints, tid);
        return ret;
    }
}
// end edited by hyihe

struct thread_data {
    u_long app_syscall; // Per thread address for specifying pin vs. non-pin system calls
    u_long app_syscall_chk;
    int record_pid;
    int syscall_cnt;
    int sysnum;
    u_long ignore_flag;
};

void inst_syscall_end(THREADID thread_id, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    //struct thread_data* tdata = (struct thread_data *) PIN_GetThreadData(tls_key, PIN_ThreadId());
    struct thread_data* tdata = current_thread;
    if (tdata) {
	if (tdata->app_syscall != 999) tdata->app_syscall = 0;
    } else {
	fprintf (stderr, "inst_syscall_end: NULL tdata\n");
    }	
}

static void sys_mmap_start(struct thread_data* tdata, u_long addr, int len, int prot, int fd)
{
    tdata->app_syscall_chk = len + prot; // Pin sometimes makes mmaps during mmap
}

void syscall_start(struct thread_data* tdata, int sysnum, ADDRINT syscallarg0, ADDRINT syscallarg1,
		   ADDRINT syscallarg2, ADDRINT syscallarg3, ADDRINT syscallarg4, ADDRINT syscallarg5)
{
    switch (sysnum) {
        case SYS_mmap:
        case SYS_mmap2:
            sys_mmap_start(tdata, (u_long)syscallarg0, (int)syscallarg1, (int)syscallarg2, (int)syscallarg4);
            break;
    }
}

void PIN_FAST_ANALYSIS_CALL set_address_one(ADDRINT syscall_num, ADDRINT syscallarg0, ADDRINT syscallarg1, ADDRINT syscallarg2,
					    ADDRINT syscallarg3, ADDRINT syscallarg4, ADDRINT syscallarg5)
{   
    //struct thread_data* tdata = (struct thread_data *) PIN_GetThreadData(tls_key, PIN_ThreadId());
    struct thread_data* tdata = current_thread;
    int sysnum = (int) syscall_num;
    
    if (sysnum == 45 || sysnum == 91 || sysnum == 120 || sysnum == 125 || sysnum == 174 || sysnum == 175 || sysnum == 190 || sysnum == 192) {
	check_clock_before_syscall (fd, (int) syscall_num);
	fprintf (stderr, "check_clock_before_syscall: sysnum %d\n", sysnum);
    }
    tdata->app_syscall = syscall_num;

    syscall_start(tdata, syscall_num, syscallarg0, syscallarg1, syscallarg2, 
		  syscallarg3, syscallarg4, syscallarg5);
}

void PIN_FAST_ANALYSIS_CALL syscall_after (ADDRINT ip)
{
    //struct thread_data* tdata = (struct thread_data *) PIN_GetThreadData(tls_key, PIN_ThreadId());
    struct thread_data* tdata = current_thread;
    if (tdata->app_syscall == 999) {
	if (check_clock_after_syscall (fd) == 0) {
	} else {
	    fprintf (stderr, "Check clock failed\n");
	}
	tdata->app_syscall = 0;
    }
}

// END GENERIC STUFF NEEDED TO REPLAY WITH PIN

void PIN_FAST_ANALYSIS_CALL instrument_call(ADDRINT address, ADDRINT target, ADDRINT next_address)
{
//    if(inrange()) {
//        fprintf (stderr, "Thread %5d Call 0x%08x target 0x%08x next 0x%08x\n", PIN_ThreadId(), address, target, next_address);
//    }
}

void PIN_FAST_ANALYSIS_CALL instrument_ret(ADDRINT address, ADDRINT target)
{
//    if(inrange()) {
//        fprintf (stderr, "Thread %5d Ret  0x%08x target 0x%08x\n", PIN_ThreadId(), address, target);
//    }
}

bool type_is_enter (ADDRINT type) {
    if((type & 0x1) && (type != 15) && (type != 17)) {
        return true;
    } else {
        return false;
    }
}

void PIN_FAST_ANALYSIS_CALL log_replay_enter (ADDRINT type, ADDRINT check)
{
    long curr_clock = *ppthread_log_clock;
    if(type_is_enter(type)) {
        // Indicates the end of an interval
        thd_entr_type[PIN_ThreadId()] = type;
        fprintf (stderr, "Thread %5d reaches sync point (%d) at clock %ld, type %u check %x\n", PIN_ThreadId(), type, curr_clock, type, check);
        update_interval_speculate(thd_ints, PIN_ThreadId(), curr_clock);
    } else {
        thd_entr_type[PIN_ThreadId()] = type;
        fprintf (stderr, "Thread %5d resumes (%d) at clock %ld\n", PIN_ThreadId(), type, curr_clock);
    }
}

void PIN_FAST_ANALYSIS_CALL record_read (ADDRINT ip, VOID* addr, ADDRINT size)
{
    if(detect_race(PIN_ThreadId(), addr, size, ip, MEM_REF_READ))
        exit(1);
}

void PIN_FAST_ANALYSIS_CALL record_read2 (ADDRINT ip, VOID* addr, ADDRINT size)
{
//    if (inrange()) {
//        fprintf (stderr, "Thread %5d read2 address %p (inst %p)\n", PIN_ThreadId(), addr, ip);
//    }
}

void PIN_FAST_ANALYSIS_CALL record_write (ADDRINT ip, VOID* addr, ADDRINT size)
{
    if(detect_race(PIN_ThreadId(), addr, size, ip, MEM_REF_WRITE))
        exit(1);
}

void PIN_FAST_ANALYSIS_CALL record_locked (ADDRINT ip)
{

    if (inrange()) {
	PIN_LockClient();	
//	std::pair<ADDRINT, string> p = make_pair(ip, RTN_FindNameByAddress(ip));
//	if (locked_insts.find(p) != locked_insts.end()) { 
	    fprintf (stderr, "Thread %5d (record pid %d) locked inst %08x\n", PIN_ThreadId(), get_record_pid(), ip);
	    if (IMG_Valid(IMG_FindByAddress(ip))) {
		fprintf(stderr, "%s -- img %s static %#x\n", RTN_FindNameByAddress(ip).c_str(), 
			IMG_Name(IMG_FindByAddress(ip)).c_str(), find_static_address(ip));
	    }
//	    locked_insts.insert(p);


//	}
	PIN_UnlockClient();
    }
}

void PIN_FAST_ANALYSIS_CALL log_replay_exit ()
{
    int type = thd_entr_type[PIN_ThreadId()];
    if(!type_is_enter(type)) {
        long curr_clock = get_clock_value(fd) - 1;
        //fprintf (stderr, "Thread %5d resumes (%d) at clock %ld\n", PIN_ThreadId(), type, curr_clock);
        thd_ints[PIN_ThreadId()] = new_interval(curr_clock);
    }
}

// Update the exit time of the current interval upon context switch
void PIN_FAST_ANALYSIS_CALL log_replay_block(ADDRINT block_until) {
//    long curr_clock = get_clock_value(fd);
//    fprintf(stderr, "Context Switch! Thread %d reaches %d, current clock is %ld\n",
//        PIN_ThreadId(), block_until, curr_clock);
    // do not overwrite if context switch happened after an _EXIT
    if(type_is_enter(thd_entr_type[PIN_ThreadId()]))
        update_interval_overwrite(thd_ints, PIN_ThreadId(), block_until);
}

void PIN_FAST_ANALYSIS_CALL print_function_name (ADDRINT ip, char* name, ADDRINT value1, ADDRINT value2)
{
	if (inrange ()) {
		fprintf (stderr, "[debug] %d %s\n", PIN_ThreadId(), name);
		if (strstr (name, "pthread") || strstr (name, "sem")) {
			fprintf (stderr, "  params %x %x\n", value1, value2);
		} else if (strstr (name, "getenv")) {
			fprintf (stderr, "    getenv: %s\n", (char*) value1);
		}
		PIN_LockClient();
		if (IMG_Valid(IMG_FindByAddress(ip))) {
			fprintf(stderr, " -- img %s\n", IMG_Name(IMG_FindByAddress(ip)).c_str());
		}
		PIN_UnlockClient();

	}
}

void track_function(RTN rtn, void* v) 
{
    /*if (IMG_Name(IMG_FindByAddress(RTN_Address(rtn))).find("libpthread") == string::npos) {
	    return;
    }*/
    RTN_Open(rtn);
    const char* name = RTN_Name(rtn).c_str();
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) print_function_name, IARG_FAST_ANALYSIS_CALL,
		    IARG_ADDRINT, RTN_Address (rtn),
		    IARG_PTR, strdup (name), 
		    IARG_FUNCARG_ENTRYPOINT_VALUE, 0, 
		    IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
		    IARG_END);
    if (!strcmp (name, "pthread_log_replay")) {
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) log_replay_enter, 
			IARG_FAST_ANALYSIS_CALL, 
		       IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR) log_replay_exit, IARG_FAST_ANALYSIS_CALL, IARG_END);
    } else if (!strcmp (name, "pthread_log_block")) {
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) log_replay_block, IARG_FAST_ANALYSIS_CALL, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }
    RTN_Close(rtn);
}

void PIN_FAST_ANALYSIS_CALL debug_print_inst (ADDRINT ip, char* ins_str) 
{ 
#ifdef EXTRA_DEBUG
    if (ins_str) 
	fprintf (stderr, "%s\n", ins_str);
    else 
	fprintf (stderr, "null\n");
#endif
}

void track_inst(INS ins, void* data) 
{
  // BEGIN GENERIC STUFF NEEDED TO REPLAY WITH PIN
    // The first system call is the ioctl associated with fork_replay.
    // We do not want to issue it again, so we NULL the call and return.
    if(INS_IsSyscall(ins)) {
		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) set_address_one, 
			       IARG_FAST_ANALYSIS_CALL,
			       IARG_SYSCALL_NUMBER, 
			       IARG_SYSARG_VALUE, 0, 
			       IARG_SYSARG_VALUE, 1,
			       IARG_SYSARG_VALUE, 2,
			       IARG_SYSARG_VALUE, 3,
			       IARG_SYSARG_VALUE, 4,
			       IARG_SYSARG_VALUE, 5,
			       IARG_END);
    } else {
	// Ugh - I guess we have to instrument every instruction to find which
	// ones are after a system call - would be nice to do better.
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)syscall_after, 
			IARG_FAST_ANALYSIS_CALL,
			IARG_INST_PTR, IARG_END);
    }
  // END GENERIC STUFF NEEDED TO REPLAY WITH PIN

    if (INS_IsMemoryRead(ins)) {
	INS_InsertPredicatedCall (ins, IPOINT_BEFORE, (AFUNPTR) record_read, IARG_FAST_ANALYSIS_CALL,IARG_INST_PTR,
				  IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
    }
    if (INS_IsMemoryWrite(ins)) {
	INS_InsertPredicatedCall (ins, IPOINT_BEFORE, (AFUNPTR) record_write, IARG_FAST_ANALYSIS_CALL,  IARG_INST_PTR,
				  IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
    }
    if (INS_HasMemoryRead2(ins)) {
	INS_InsertPredicatedCall (ins, IPOINT_BEFORE, (AFUNPTR) record_read2, IARG_FAST_ANALYSIS_CALL,IARG_INST_PTR,
				  IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE, IARG_END);
    }
    if (INS_LockPrefix(ins)) {
	INS_InsertPredicatedCall (ins, IPOINT_BEFORE, (AFUNPTR) record_locked, IARG_FAST_ANALYSIS_CALL, IARG_INST_PTR, IARG_END);
    }

#ifdef INSTMNT_CALL_RET
    // sometimes commented out to make testing faster
    switch (INS_Opcode(ins)) {
	case XED_ICLASS_CALL_NEAR:
	case XED_ICLASS_CALL_FAR:
	    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) instrument_call, 
			    IARG_FAST_ANALYSIS_CALL, 
			    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, 
			   IARG_ADDRINT, INS_NextAddress(ins), IARG_END);
	    
	    break;
	    
	case XED_ICLASS_RET_NEAR:
	case XED_ICLASS_RET_FAR:
	    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) instrument_ret, 
			    IARG_FAST_ANALYSIS_CALL, 
			    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_END);    
	    break;
    }
#endif
}

// BEGIN GENERIC STUFF NEEDED TO REPLAY WITH PIN
BOOL follow_child(CHILD_PROCESS child, void* data)
{
    char** argv;
    char** prev_argv = (char**)data;
    int index = 0;

    /* the format of pin command would be:
     * pin_binary -follow_execv -t pin_tool new_addr*/
    int new_argc = 5;
    argv = (char**)malloc(sizeof(char*) * new_argc);

    argv[0] = prev_argv[index++];
    argv[1] = (char *) "-follow_execv";
    while(strcmp(prev_argv[index], "-t")) index++;
    argv[2] = prev_argv[index++];
    argv[3] = prev_argv[index++];
    argv[4] = (char *) "--";

    CHILD_PROCESS_SetPinCommandLine(child, new_argc, argv);
    return TRUE;
}

void thread_start (THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    struct thread_data* ptdata;

    ptdata = (struct thread_data *) malloc (sizeof(struct thread_data));
    assert (ptdata);

    // edited by hyihe
    num_threads++;
    // need to start a new interval for a new thread
    thd_ints.push_back(new_interval(get_clock_value(fd)));
    thd_entr_type.push_back(0);

    ptdata->app_syscall = 0;
    ptdata->record_pid = get_record_pid();

    PIN_SetThreadData (tls_key, ptdata, threadid);
    int thread_ndx;
    long thread_status = set_pin_addr (fd, (u_long) &(ptdata->app_syscall), (u_long) &(ptdata->app_syscall_chk), 
				       ptdata, (void **) &current_thread, &thread_ndx);
    if (thread_status < 2) {
	current_thread = ptdata;
    }
    fprintf (stderr,"Thread gets rc %ld ndx %d from set_pin_addr\n", thread_status, thread_ndx);
}

void trace_instrumentation(TRACE trace, void* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
	for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
#ifdef EXTRA_DEBUG
		string s = INS_Disassemble (ins);
		char* ins_str = strdup (s.c_str());
		INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) debug_print_inst,
				IARG_FAST_ANALYSIS_CALL,
				IARG_INST_PTR,
				IARG_PTR, ins_str, IARG_END);
#endif
		track_inst (ins, NULL);
	}
    }
}

int main(int argc, char** argv) 
{    
    int rc;

    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Intialize the replay device
    rc = devspec_init (&fd);
    if (rc < 0) return rc;
    ppthread_log_clock = map_shared_clock (fd);

    // Obtain a key for TLS storage
    tls_key = PIN_CreateThreadDataKey(0);
    print_limit = atol(KnobPrintLimit.Value().c_str());
    print_stop = atol(KnobPrintStop.Value().c_str());

    PIN_AddThreadStartFunction(thread_start, 0);

    //PIN_AddFollowChildProcessFunction(follow_child, argv);
    RTN_AddInstrumentFunction(track_function, 0);
    PIN_AddSyscallExitFunction(inst_syscall_end, 0);
    TRACE_AddInstrumentFunction (trace_instrumentation, 0);
    PIN_SetSyntaxIntel();
    PIN_StartProgram();
    return 0;
}
// END GENERIC STUFF NEEDED TO REPLAY WITH PIN


// edited by hyihe
// Implementations of functions and data types defined in happens_before.h
int var_t::check_for_race(int acc_type, std::vector<interval_t *> &thd_ints, uint32_t tid) {
	int ret = 0;
	interval_t prev;
	interval_t *current = thd_ints[tid];
	if (acc_type == MEM_REF_WRITE) {
		for(uint32_t i=0; i<thd_ints.size(); i++) {
			if(i == tid)
				continue;
			// Write must happen AFTER all previous writes
			// AND reads
			if(!happens_before(this->last_wr[i], current) || 
				!happens_before(this->last_rd[i], current)) {
				ret = 1;
				prev = (!happens_before(this->last_wr[i], current))?
					*this->last_wr[i] : *this->last_rd[i];
				break;
			}
		}
	} else {
		for(uint32_t i=0; i<thd_ints.size(); i++) {
			if(i == tid)
				continue;
			// Read must happen AFTER all previous writes
			if(!happens_before(this->last_wr[i], current)) {
				ret = 1;
				prev = *this->last_wr[i];
				break;
			}
		}
	}
	if(ret) {
		std::cerr << "@@@@ Race detected! Violating access is a "
		<< ((acc_type==MEM_REF_WRITE)? "write" : "read") << "!" << std::endl;
		fprintf(stderr, "@@@@ Interval interleaving %ld:%ld with %ld:%ld\n",
			prev.first, prev.second, current->first, current->second);
	}
	return ret;
}

void var_t::update_intvls(int acc_type, const std::vector<interval_t *> &thd_ints, uint32_t tid) {
	// skipping error checking here to speed up
	interval_t *current = thd_ints[tid];
	if(acc_type == MEM_REF_WRITE) {
		this->last_wr[tid] = current;
	} else {
		this->last_rd[tid] = current;
	}
}

int var_t::resize_intvls(int target_size) {
	int diff = target_size - this->last_rd.size();
	if(diff < 0)
		return -1;
	for(int i = 0; i < diff; ++i) {
		// Intervals associated with NULL pointers
		// happens before all other intervals
		this->last_rd.push_back(0);
		this->last_wr.push_back(0);
	}
	return 0;
}

interval_t *new_interval(long clock) {
	interval_t *ret = new interval_t;
	ret->first = clock;
	ret->second = clock + 1;
	return ret;
}

void update_interval_speculate(std::vector<interval_t *> &thd_ints, uint32_t tid, long clock) {
	if(thd_ints[tid]->second != clock) {
//		fprintf(stderr, "Thread %5d's interval updated (spec) from %ld to %ld\n",
//			tid, thd_ints[tid]->second, clock);
		thd_ints[tid]->second = clock;
	}
}

void update_interval_overwrite(std::vector<interval_t *> &thd_ints, uint32_t tid, long clock) {
	thd_ints[tid]->second = clock;
}
// end edited by hyihe
