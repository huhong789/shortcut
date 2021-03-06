#!/usr/bin/python

import os
from subprocess import Popen, PIPE
import sys
import argparse
import datetime
from multiprocessing import Pool

def compMain (outputdir, pid):
    return os.system("gcc -masm=intel -c -fpic -Wall -Werror "+outputdir+"/exslice." + pid + ".c -o "+outputdir+"/exslice." + pid + ".o")

def compSlice (outputdir, strno, pid):
    return os.system("gcc -masm=intel -c -fpic -Wall -Werror "+outputdir+"/exslice" + strno + "." + pid + ".c -o "+outputdir+"/exslice" + strno + "." + pid + ".o")
import glob
import string

instr_jumps = True

# Modify these config paraemters for new checkpoint
parser = argparse.ArgumentParser()
parser.add_argument("rec_group_id", help = "record group id")
parser.add_argument("checkpoint_clock", help = "clock of the checkpoint")
parser.add_argument("-taint_syscall", help = "only taint this syscall at index XXX")
parser.add_argument("-taint_byterange", help = "taint a specific byte range: RECORD_PID,SYSCALL_INDEX,START,END")
parser.add_argument("-taint_byterange_file", help = "give a file specifying all ranges and all syscalls to be tainted.")
parser.add_argument("-outputdir", help = "the output dir of all output files.")
parser.add_argument("-compile_only", help = "needs an input file name. Skip the slice generation phase and directly compiles assemble to .so")
parser.add_argument("-compile_all", help = "Skip the slice generation phase and directly compiles all assemble files to .so", action="store_true")
parser.add_argument("-no_ckpt", help = "Skip the ckpt phase", action="store_true")
parser.add_argument("-region_start_clock", help = "Only slice a region bounded by two sys_jumpstart_runtime.")
parser.add_argument("-no_pthread_lib", help = "don't use the eglibc to record user-level operations", action="store_true")
parser.add_argument("-attach_offset", help = "attach pintool at a specific location instead of attaching it from the beginning")
args = parser.parse_args()

rec_dir = args.rec_group_id
ckpt_at = args.checkpoint_clock
taint_filter = False
input_asm_file = list()
outputdir = "/replay_logdb/rec_" + str(rec_dir)
if args.outputdir:
    outputdir = args.outputdir
if args.compile_only is not None:
    input_asm_file.append (args.compile_only)
if args.compile_all:
    input_asm_file = glob.glob(outputdir + '/exslice[123456789]*.*.c')
if args.taint_syscall:
    taint_syscall = args.taint_syscall
    taint_filter = True
else:
    taint_syscall = ""

if args.taint_byterange:
    taint_byterange = args.taint_byterange
    taint_filter = True
else:
    taint_byterange = ""
if args.taint_byterange_file:
    taint_byterange_file = args.taint_byterange_file
    taint_filter = True
else:
    taint_byrterange_file = ""

usage = "Usage: ./gen_ckpt.py rec_group_id checkpoint_clock [-o outputdir] [-taint_syscall SYSCALL_INDEX] [-taint_byterange RECORD_PID,SYSCALL_INDEX,START,END] [-taint_byterange_file filename] [-comiple_only input_asm_filename]" 

ts_start = datetime.datetime.now()

if len(input_asm_file) is 0:
# Run the pin tool to generate slice info and the recheck log
        for oldfile in glob.glob(outputdir + '/exslice*'):
            os.unlink (oldfile);

        outfd = open(outputdir+"/pinout", "w")
        checkfilename = outputdir+"/checks"
        commands = ["./runpintool"]
        if (args.no_pthread_lib):
            commands.append("-no_pthread_lib")
        if (args.attach_offset):
            commands.append ("--attach_offset="+args.attach_offset)
        if (taint_filter > 0):
        	if (taint_syscall):
	        	commands.extend(["/replay_logdb/rec_" + str(rec_dir), "../dift/obj-ia32/linkage_offset.so", "-i", "-s", 
                            str(taint_syscall), "-recheck_group", str(rec_dir), "-ckpt_clock", str(ckpt_at), "-chk", checkfilename, "-group_dir", outputdir])
        	elif (taint_byterange):
	        	commands.extend(["/replay_logdb/rec_" + str(rec_dir), "../dift/obj-ia32/linkage_offset.so", "-i", "-b", 
		        	taint_byterange, "-recheck_group", str(rec_dir), "-ckpt_clock", str(ckpt_at), "-chk", checkfilename, "-group_dir", outputdir])
        	elif (taint_byterange_file):
	        	commands.extend(["/replay_logdb/rec_" + str(rec_dir), "../dift/obj-ia32/linkage_offset.so", "-i", "-rf", 
		        	taint_byterange_file, "-recheck_group", str(rec_dir), "-ckpt_clock", str(ckpt_at), "-chk", checkfilename, "-group_dir", outputdir])
        else:
                commands.extend(["/replay_logdb/rec_" + str(rec_dir), "../dift/obj-ia32/linkage_offset.so", 
                       "-recheck_group", str(rec_dir), "-ckpt_clock", str(ckpt_at), "-chk", checkfilename, "-group_dir", outputdir])
        if (args.region_start_clock is not None):
            commands.append ("-fl") #function level slicing
            commands.append (args.region_start_clock)
        p = Popen (commands, stdout=outfd)
        print (commands)
        p.wait()
        outfd.close()

        input_asm_file = glob.glob(outputdir + '/exslice[123456789]*.*.c')
ts_pin = datetime.datetime.now()

record_pids = {}
for asm_file in input_asm_file:
    if args.compile_only:
        print "compiling " + asm_file
    record_pid = asm_file[asm_file.rfind (".", 0, -3)+1:-2]
    begin = asm_file.find("exslice") + 7
    end = asm_file.find(".")
    filecnt = int(asm_file[begin:end])
    print asm_file, record_pid, filecnt
    if (not record_pid in record_pids) or filecnt > record_pids[record_pid]:
        record_pids[record_pid] = filecnt;

linkstrs = {}
pool = Pool(processes=7)
for (record_pid,fcnt) in record_pids.items():

    linkstrs[record_pid] = "gcc -shared "+outputdir+"/exslice." + record_pid + ".o -o "+outputdir+"/exslice." + record_pid + ".so recheck_support.o"
    pool.apply_async (compMain, (outputdir, record_pid))
    for i in range(fcnt):
        strno = str(i + 1)
        pool.apply_async (compSlice, (outputdir, strno, record_pid))
        linkstrs[record_pid] += " " + outputdir + "/exslice" + strno + "." + record_pid + ".o"

pool.close()
pool.join()

for record_pid in record_pids.keys():
    os.system(linkstrs[record_pid])

ts_compile = datetime.datetime.now()

# Generate a checkpoint
if args.compile_only is None and args.compile_all is False and args.no_ckpt is False:
    p = Popen(["./resume", "/replay_logdb/rec_" + str(rec_dir), "--pthread", "../eglibc-2.15/prefix/lib/", "--ckpt_at=" + str(ckpt_at)])
    p.wait()

# Print timing info
ts_end = datetime.datetime.now()
print "Time to run pin tool: ", ts_pin - ts_start
print "Time to compile: ", ts_compile - ts_pin
print "Time to ckpt: ", ts_end - ts_compile
print "Total time: ", ts_end - ts_start

