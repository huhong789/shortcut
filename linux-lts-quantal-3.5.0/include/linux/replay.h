#ifndef __REPLAY_H__
#define __REPLAY_H__

#define MAX_LOGDIR_STRLEN 80

/* These are the device numbers for the attach mechanism in replay_ckpt_wakeup */
#define ATTACH_PIN 1
#define ATTACH_GDB 2

/* These are return values from set_pin_address */
#define PIN_NORMAL         0
#define PIN_ATTACH_RUNNING 1
#define PIN_ATTACH_BLOCKED 2
#define PIN_ATTACH_REDO    4

#define RECHECK

#include <linux/signal.h>
#include <linux/mm_types.h>

//defined in replay.c
struct ckpt_tsk; 

/* Starts replay with a (possibly) multithreaded fork */
int fork_replay (char __user * logdir, const char __user *const __user *args,
		const char __user *const __user *env, char* linker, int save_mmap, int fd,
		int pipe_fd);

/* Restore ckpt from disk - replaces AS of current process (like exec) */
/* Linker may be NULL - otherwise points to special libc linker */
long replay_ckpt_wakeup (int attach_device, char* logdir, char* linker, int fd, int follow_splits, int save_mmap, loff_t attach_index, int attach_pid, 
			 int ckpt_at, int ckpt_memory_only, int ckpt_mem_slice_pid, int record_timing, u_long nfake_calls, u_long* fake_call_points);
long replay_full_ckpt_wakeup (int attach_device, char* logdir, char* filename, char *uniqueid, char* linker, int fd,      
			      int follow_splits, int save_mmap, loff_t syscall_index, int attach_pid, u_long nfake_calls, u_long *fake_call_points, 
			      int go_live, char* execute_slice_name, char* recheck_filename);
long replay_full_ckpt_proc_wakeup (char* logdir, char* filename, char *uniqueid,int fd, int is_thread, int go_live, char* execute_slice_name, char* recheck_filename);
void fw_slice_recover (pid_t daemon_pid, pid_t slice_pid, long retval);

/* Returns linker for exec to use */
char* get_linker (void);

/* These should be used only by a PIN tool */
struct used_address {
    u_long start;
    u_long end;
};

int set_pin_address (u_long pin_address, u_long pin_chk, u_long thread_data, u_long __user* curthread_ptr, int* attach_ndx);
long get_log_id (void);
unsigned long get_clock_value (void);
long check_clock_before_syscall (int syscall);
long check_clock_after_syscall (int syscall);
long check_for_redo (void);
long redo_mmap (u_long __user * prc, u_long __user * plen);
long redo_munmap (void);
long get_used_addresses (struct used_address __user * plist, int listsize);
void print_memory_areas (void);

/* Handles replay-specific work to record a signal */
int get_record_ignore_flag (void);
long check_signal_delivery (int signr, siginfo_t* info, struct k_sigaction* ka, int ignore_flag);
long record_signal_delivery (int signr, siginfo_t* info, struct k_sigaction* ka);
void replay_signal_delivery (int* signr, siginfo_t* info);
int replay_has_pending_signal (void);
int get_record_pending_signal (siginfo_t* info);

/* used in order to correctly exit in a very particular pin bug*/
int should_call_recplay_exit_start(void);

/* used b/c pin calls set_tid_address on attach, and I'm just going to see if ignoring that works */
int is_pin_attaching(void); 

/* Called when a record/replay thread exits */
void recplay_exit_start(void);
void recplay_exit_middle(void);
void recplay_exit_finish(void);

/* Called during a vfork */
void record_vfork_handler (struct task_struct* tsk);
void replay_vfork_handler (struct task_struct* tsk);

/* Common helper functions */
struct pt_regs* get_pt_regs(struct task_struct* tsk);
char* get_path_helper (struct vm_area_struct* vma, char* path);

/* For synchronization points in kernel outside of replay.c */
#define TID_WAKE_CALL 500
struct syscall_result;

long new_syscall_enter_external (long sysnum);
long new_syscall_exit_external (long sysnum, long retval, void* retparams);
long get_next_syscall_enter_external (int syscall, char** ppretparams, struct syscall_result** ppsr);
void get_next_syscall_exit_external (struct syscall_result* psr);

/* For handling randomness within the kernel */
void record_randomness(u_long);
u_long replay_randomness(void);

/* ... and for other exec values */
void record_execval(int uid, int euid, int gid, int egid, int secureexec);
void replay_execval(int* uid, int* euid, int* gid, int* egid, int* secureexec);

/* For replaying exec from a cache file */
const char* replay_get_exec_filename (void);


/* In replay_logdb.c */
__u64 get_replay_id (void);
void get_logdir_for_replay_id (__u64 id, char* buf);
int make_logdir_for_replay_id (__u64 id, char* buf);

/* In replay_ckpt.h */
char* copy_args (const char __user* const __user* args, const char __user* const __user* env, int* buflen);
long replay_checkpoint_to_disk (char* filename, char* execname, char* buf, int buflen, __u64 parent_rg_id);
long replay_resume_from_disk (char* filename, char** execname, char*** argsp, char*** envp, __u64* prg_id);


long replay_full_resume_hdr_from_disk (char* filename, __u64* prg_id, int* pclock, u_long* pproc_count, loff_t* ppos);
long replay_full_resume_proc_from_disk (char* filename, pid_t clock_pid, int is_thread, long* pretval, loff_t* plogpos, u_long* poutptr, u_long* pconsumed, u_long* pexpclock, u_long* pthreadclock, u_long *ignore_flag, u_long *user_log_addr, u_long *user_log_pos,u_long *child_tid,u_long *replay_hook, loff_t* ppos, char* slicelib, u_long* slice_addr, u_long* slice_size, u_long* pthread_clock_addr);

long replay_full_checkpoint_hdr_to_disk (char* filename, __u64 rg_id, int clock, u_long proc_count, struct ckpt_tsk *ct, struct task_struct *tsk, loff_t* ppos);
long replay_full_checkpoint_proc_to_disk (char* filename, struct task_struct* tsk, pid_t record_pid, int is_thread, long retval, loff_t logpos, u_long outptr, u_long consumed, u_long expclock, u_long pthread_block_clock, u_long ignore_flag, u_long user_log_addr, u_long user_log_pos,u_long replay_hook, loff_t* ppos);
long replay_full_checkpoint_proc_to_disk_light (char* filename, struct task_struct* tsk, pid_t record_pid, int is_thread, long retval, loff_t logpos, u_long outptr, u_long consumed, u_long expclock, u_long pthread_block_clock, u_long ignore_flag, u_long user_log_addr, u_long user_log_pos,u_long replay_hook, char* linker, loff_t* ppos);


/* Helper functions for checkpoint/resotre */
int checkpoint_replay_cache_files (struct task_struct* tsk, struct file* cfile, loff_t* ppos);
int restore_replay_cache_files (struct file* cfile, loff_t* ppos, int go_live);

int find_sysv_mapping_by_key (int key);
int checkpoint_ckpt_tsks_header(struct ckpt_tsk *ct, int parent_pid, int is_thread, struct file *cfile, loff_t *ppos);
int restore_ckpt_tsks_header(u_long num_procs, struct file *cfile, loff_t *ppos);
int checkpoint_sysv_mappings (struct task_struct* tsk, struct file* cfile, loff_t* ppos);
int restore_sysv_mappings (struct file* cfile, loff_t* ppos);
int add_sysv_shm(u_long addr, u_long len);

long get_ckpt_state (pid_t pid); //huh? 

int checkpoint_task_xray_monitor(struct task_struct *tsk, struct file* cfile, loff_t* ppos);
int restore_task_xray_monitor(struct task_struct *tsk, struct file* cfile, loff_t* ppos);

/* Optional stats interface */
#define REPLAY_STATS
#ifdef REPLAY_STATS
struct replay_stats {
	atomic_t started;
	atomic_t finished;
	atomic_t mismatched;
};

long get_replay_stats (struct replay_stats __user * ustats);

#endif

/* For tracking where the args are in Pin, only valid on replay */
void save_exec_args(unsigned long argv, int argc, unsigned long envp, int envc);
unsigned long get_replay_args(void);
unsigned long get_env_vars(void);
long get_attach_status(pid_t pid);
int wait_for_replay_group(pid_t pid);
pid_t get_replay_pid(pid_t parent_pid, pid_t record_pid);


long get_record_group_id(__u64 __user * prg_id);

/* Pass in the "real" resume process pid and it will give back the
	recorded replay pid that is currently running.
	Does not need to be called from a replay thread.*/
pid_t get_current_record_pid(pid_t nonrecord_pid);

/* Calls to read the filemap */
long get_num_filemap_entries(int fd, loff_t offset, int size);
long get_filemap(int fd, loff_t offset, int size, void __user * entries, int num_entries);

long reset_replay_ndx(void);

/* Used for gdb attachment */
int replay_gdb_attached(void);
void replay_unlink_gdb(struct task_struct* tsk);

/* Set to force a replay to exit on fatal signal */
long try_to_exit (u_long pid);

/* Let's the PIN tool read the clock value too */
long pthread_shm_path (void __user** mapped_address);

/* For obtaining list of open sockets */
struct monitor_data {
	int fd;
	int type;
	int data;
	char channel[256];
};
long get_open_socks (struct monitor_data __user* entries, int num_entries);

long go_live_recheck (__u64 gid, pid_t pid, char* recheck_log);
struct open_retvals {
	dev_t           dev;
	u_long          ino;
	struct timespec mtime;
};
struct startup_db_result { 
	__u64 group_id;
	unsigned long ckpt_clock;
};


void init_startup_db (void);
void add_to_startup_cache (char* arbuf, int arglen, __u64 group_id, unsigned long ckpt_clock);
//int find_startup_cache (char* argbuf, int arglen, struct startup_db_result* result);
int find_startup_cache_user_argv (const char __user *const __user *__argv, struct startup_db_result* result);
char* copy_args (const char __user* const __user* args, const char __user* const __user* env, int* buflen);
struct fw_slice_info {
	unsigned long text_addr;
	unsigned long text_size;
	unsigned long extra_addr;
	unsigned long extra_size;
	struct pt_regs regs;
	//fpu
	char fpu_is_allocated;
	unsigned int fpu_last_cpu;
	unsigned int fpu_has_fpu;
	union thread_xstate fpu_state;
        //some extra info
        struct go_live_clock* slice_clock;
	unsigned long slice_mode; //set if you're accelerating fine-grained code region 
};

struct go_live_process_map { 
    int record_pid;
    int current_pid;
    __user char* taintbuf; // Location of taintbuf in process address space
    __user u_long* taintndx; // Location of taintbuf index in process address space
    int wait; //for futex wait
    int value; //for futex wait 
};

struct replay_group;

// ****
//NOTE: there is one user-level structure corresponding to this one in recheck_log.h
// ***
struct go_live_clock {
	char skip[128];  //since we put this structure in the shared uclock region, make sure it won't mess up original data in that region (I believe original data only occupies first 8 bytes)
	atomic_t slice_clock; //ordering
	atomic_t num_threads;  //the number of threads that has started (has called start_fw_slice)
	atomic_t num_remaining_threads; //the number of threads that hasn't finished slice exeucting
	atomic_t wait_for_other_threads; // Used by user-level slice code
	int mutex; // Used by user-level slice code
	struct replay_group* replay_group; //the address of the replay_group
	void* cache_file_structure; //This address is the cache_files_opened in recheck_support.c; this is need to make this structure shared across threads
	struct go_live_process_map process_map[0]; //current pid  <-> record pid
};

struct ckpt_data {
	u_long proc_count;
	__u64  rg_id;
	int    clock;	
};

struct ckpt_proc_data {
	pid_t  record_pid;
	long   retval;
	loff_t logpos;
	u_long outptr;
	u_long consumed;
	u_long expclock;
	u_long pthreadclock;
	u_long p_ignore_flag; //this is really just a memory address w/in the vma
	u_long p_user_log_addr;
	u_long user_log_pos;
	u_long p_clear_child_tid;
	u_long p_replay_hook;
//	u_long rss_stat_counts[NR_MM_COUNTERS]; //the counters from the checkpointed task
};


struct record_thread;
struct replay_thread;

long start_fw_slice (struct go_live_clock* slice_clock, u_long slice_addr, u_long slice_size, long record_pid, char* recheck_name, u_long user_clock_addr, u_long);
void destroy_replay_group (struct replay_group *prepg);
void fw_slice_recover_swap_register (struct task_struct *main_live_tsk);
void fw_slice_recover_swap_register_single_thread (struct task_struct *main_live_tsk, struct task_struct* recover_tsk);
struct go_live_clock* get_go_live_clock (struct task_struct* tsk);
void dump_vmas_content (u_long prefix);

struct go_live_thread_info;
void wake_up_vm_dump_waiters (struct go_live_thread_info* info);
void wait_for_vm_dump (struct go_live_thread_info* info);
void put_go_live_thread (struct go_live_thread_info* info);

struct go_live_thread_info {
	struct record_thread* orig_record_thrd;
	struct replay_thread* orig_replay_thrd;
	int rollback_process_id; 
	u64 slice_gid;
	int slice_pid;  
	u_long region_start_clock; 
	u_long recover_clock; 
};

#endif
