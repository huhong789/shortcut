#define CONSERVATIVE 1
#define ACCURATE 2
#define HANDLED 4
#define MERGE_POINT 8
#define NO_SPEC 16

struct function_status {
    const char*   name;
    int     status;
} functions[] = 
{
 {"socket", HANDLED}, {"__socket", HANDLED},
 {"socketpair", HANDLED},
 {"__bind", HANDLED}, {"bind", HANDLED}, 
 {"__connect", HANDLED}, {"connect", HANDLED}, {"__connect_internal", HANDLED}, {"__libc_connect", HANDLED},
 {"dup2", HANDLED}, {"__dup2", HANDLED},
 {"__libc_fcntl", HANDLED}, {"__fcntl", HANDLED}, {"__fcntl_nocancel", HANDLED}, {"fcntl", HANDLED},
 {"listen", HANDLED}, {"__listen", HANDLED},
 {"send", HANDLED}, {"__send", HANDLED}, {"__libc_send", HANDLED},
 {"__write", HANDLED}, {"write", HANDLED}, {"__libc_write", HANDLED}, {"__write_nocancel", HANDLED}, 
 {"__select", HANDLED}, {"select", HANDLED}, {"__libc_select", HANDLED}, {"___newselect_nocancel", HANDLED}, 
 {"unlink", HANDLED}, {"__unlink", HANDLED}, 
 {"rename", HANDLED}, 
 {"uname", HANDLED}, {"__uname", HANDLED}, 
 {"setsockopt", HANDLED}, {"__setsockopt", HANDLED}, 
 {"getsockopt", HANDLED}, {"__getsockopt", HANDLED}, 
 {"pipe", HANDLED}, {"__pipe", HANDLED}, 
 {"accept", HANDLED}, {"__libc_accept", HANDLED},
 {"getpid", HANDLED}, {"__getpid", HANDLED},                                               
 {"execve", HANDLED}, {"__execve", HANDLED},                                               
 {"log_scripterror", HANDLED}, {"logit", HANDLED}, {"ap_log_rerror", HANDLED},
 {"msg_fatal", HANDLED}, {"msg_warn", HANDLED}, 
 {"__libc_writev", HANDLED}, {"writev", HANDLED}, {"__writev", HANDLED},
 {"__open", HANDLED}, {"open", HANDLED}, {"__open_nocancel", HANDLED}, {"__libc_open", HANDLED},
 {"__libc_open64", HANDLED}, {"open64", HANDLED}, {"__open64", HANDLED},
 {"mmap", HANDLED}, {"__mmap", HANDLED}, {"__mmap64", HANDLED}, {"mmap64", HANDLED},
 {"munmap", HANDLED}, {"__munmap", HANDLED},
 {"__readdir", HANDLED}, {"__readdir64_r", HANDLED}, {"readdir64_r", HANDLED},
 {"__rmdir", HANDLED}, {"rmdir", HANDLED},
 //{"memmove", HANDLED}, 
 {"main", HANDLED}, 
 {"guc_name_compare", HANDLED}, //for postgres only
 {"strcmp", HANDLED}, {"__strcasecmp", HANDLED}, {"strcasecmp", HANDLED},
 {"strncmp", HANDLED}, {"__strncasecmp", HANDLED}, {"strncasecmp", HANDLED},
 {"__strcmp_gg", HANDLED},
 {"__strncmp_g", HANDLED},
 {"_IO_fgets", HANDLED}, {"fgets_unlocked", HANDLED},
 {"__libc_read", HANDLED}, {"__read", HANDLED}, {"__read_nocancel", HANDLED}, {"read", HANDLED},
 {"recv", HANDLED}, {"__libc_recv", HANDLED},
 {"fstat64", HANDLED}, {"fstat", HANDLED}, {"__fstat", HANDLED}, {"__fxstat64", HANDLED},
 {"__stat", HANDLED}, {"stat", HANDLED}, {"stat64", HANDLED},
 {"lstat64", HANDLED}, {"__lstat", HANDLED}, {"lstat", HANDLED}, {"___lxstat64", HANDLED}, 
 {"__xstat", HANDLED}, {"___xstat64", HANDLED}, {"__xstat64", HANDLED}, 
 {"__close", HANDLED}, {"__close_nocancel", HANDLED}, {"__libc_close", HANDLED}, {"close", HANDLED},
 {"__chdir", HANDLED}, {"chdir", HANDLED},
 {"apr_palloc", HANDLED}, {"apr_pcalloc", HANDLED}, 
 {"__malloc", HANDLED}, {"__libc_malloc", HANDLED}, {"malloc", HANDLED}, 
 {"realloc", HANDLED}, {"__realloc", HANDLED}, {"__libc_realloc", HANDLED},
 {"__calloc", HANDLED}, {"__libc_calloc", HANDLED}, {"calloc", HANDLED}, 
 {"__cfree", HANDLED}, {"cfree", HANDLED},
 {"__libc_free", HANDLED}, {"free", HANDLED}, {"__free", HANDLED},
 {"llseek", HANDLED}, {"__libc_lseek", HANDLED}, {"__llseek", HANDLED},
 {"lseek64", HANDLED}, {"__lseek64", HANDLED}, {"__libc_lseek64", HANDLED}, 
 {"_dl_runtime_resolve", HANDLED}, {"__dcgettext", HANDLED}, 
 {"getspnam", HANDLED}, 
 {"getpwnam", HANDLED}, 
 {"getpwuid", HANDLED}, 
 {"__fork", HANDLED}, {"__libc_fork", HANDLED}, {"fork", HANDLED}, 
 {"time", HANDLED},
 {"sigaction", HANDLED},{"__libc_sigaction", HANDLED}, {"__sigaction", HANDLED},
 {"_IO_fflush", HANDLED}, {"fflush", HANDLED}, 
 {"kill", HANDLED}, {"__kill", HANDLED}, 
 //{"setlocale", HANDLED},
 //{"daemon", HANDLED}, 
 //{"strpbrk", HANDLED}, 
 //{"__tz_convert", HANDLED},
 {"__closedir", HANDLED}, {"closedir", HANDLED}, 
 {"dlsym", HANDLED}, {"__dlsym", HANDLED}, {"__libc_dlsym", HANDLED}, 
 {"initgroups", HANDLED},
 {"getuid", HANDLED}, {"__getuid", HANDLED},
 {"getgid", HANDLED}, {"__getgid", HANDLED},
 {"geteuid", HANDLED}, {"__geteuid", HANDLED},
 {"getegid", HANDLED}, {"__getegid", HANDLED},
 {"setgid", HANDLED}, {"__setgid", HANDLED},
 {"setuid", HANDLED}, {"__setuid", HANDLED},
 {"epoll_create", HANDLED},
 {"epoll_ctl", HANDLED},
 {"poll", HANDLED}, {"__libc_poll", HANDLED}, {"__poll", HANDLED}, 
 {"sendfile64", HANDLED},
 {"semget", HANDLED},
 {"_IO_vfprintf", HANDLED}, {"vfprintf", HANDLED}, {"_IO_vfprintf_internal", HANDLED},
 {"getservbyname", HANDLED},
 {"semop", HANDLED}, //for postgres
 {"_IO_ferror", HANDLED}, {"ferror", HANDLED},
 {"getaddrinfo", HANDLED}, 
 {"getgrnam", HANDLED}, 
 {"getgrgid", HANDLED},
 {"_IO_getc", HANDLED}, 
 {"fcrypt", HANDLED}, {"crypt", HANDLED}, {"__crypt_r", HANDLED},
 {"_IO_fclose", HANDLED}, {"_IO_new_fclose", HANDLED}, {"fclose", HANDLED}, {"__new_fclose", HANDLED}, 
 {"mutex_trylock", HANDLED},
 {"__nptl_deallocate_tsd", HANDLED},
 {"exit", HANDLED}, {"_exit", HANDLED}, 
 {"error", HANDLED}, {"__assert_fail", HANDLED}, 
 {"__errno_location", HANDLED}, 
 {"getprotobyname", HANDLED}, 
 {"getnameinfo", HANDLED}, 
 {"tolower", HANDLED}, {"toupper", HANDLED},
 {"__gettimeofday", HANDLED}, {"gettimeofday", HANDLED}, {"__gettimeofday_internal", HANDLED},
 {"__getsockname", HANDLED}, {"getsockname", HANDLED},
 {"isspace", HANDLED}, 
 {"__libc_waitpid", HANDLED}, {"__waitpid_nocancel", HANDLED}, {"__waitpid", HANDLED}, {"waitpid", HANDLED},
 {"memcmp", HANDLED}, //postgres: the static analysis has problem
 {"yy_shift", HANDLED}, {"yy_find_shift_action", HANDLED}, {"yy_reduce", HANDLED}, {"yy_accept", HANDLED}, // lighttpd 

 //libssl
 {"DH_compute_key", HANDLED}, 
 {"DH_generate_key", HANDLED},
 {"RSA_blinding_on", HANDLED}, 
 {"RAND_status", HANDLED}, 
 {"RSA_sign", HANDLED},
 {"RAND_seed", HANDLED},
 {"RAND_bytes", HANDLED},
 {"BN_hex2bn", HANDLED},
 
 //conservative functions
 {"rewind", CONSERVATIVE}, 
 //{"EVP_DigestUpdate", CONSERVATIVE}, 
 {"__snprintf", CONSERVATIVE}, {"vsnprintf", CONSERVATIVE},
 {"__uflow", CONSERVATIVE},
 //no_spec functions
 {"PEM_read_PrivateKey", NO_SPEC}, {"strnvis", NO_SPEC}, {"DSA_new", NO_SPEC}, {"DH_new", NO_SPEC},
 {"RSA_new", NO_SPEC}, {"BN_dup", NO_SPEC}, {"RSA_free", NO_SPEC}, {"DSA_free", NO_SPEC}, {"BN_bn2bin", NO_SPEC},
 {"HMAC_Init", NO_SPEC}, {"BN_sub", NO_SPEC}, 
 {"do_log", NO_SPEC}, {"pcre_compile", NO_SPEC}, 
 {"pcre_exec", NO_SPEC}, {"send_all_header_fields", NO_SPEC},
 {0, 0}
};

GHashTable* functions_hash;