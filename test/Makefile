CC := gcc

CFLAGS :=  -O3
CPPFLAGS := -O3
LDFLAGS := -Wall -Werror 

#LDLIBS := libutil.a

KERN_CONFIG := replay_headers/include/linux/replay_configs.h

DEPS_DIR := .deps

TARGETS=parseklog parseulog parsemlog launcher resume parseckpt \
	getstats filemap replay_pause reset_ndx currentpid seqtt runpintool partt fulltt fullsend streamtt streamctl sync_files read_trace retaint mkpartition mkpart_iter \
        recheck_support.o process_slice parserecheck finalize_slice_ctrl_flow diverge_and_merge_point find_diverge canonicalize find_range parseulib.o parsecheckpoint sdaemon env_patch ckpt_patch xdiff \
	xdump pidinator memcmp parsepckpt parseverif fix_pckpt

# For automatic .h dependency tracking... may not work perfectly for launcher as
#    the include path changes for it
CFILES=$(wildcard *.c)
CFILES := $(filter-out fpm_main.c, $(CFILES))
DEPS=$(patsubst %.c, .deps/%.d, $(CFILES))

.PHONY: all
all: $(TARGETS)

# Build dependencies + any extra flags

process_slice: process_slice.cpp
	g++ -O3 -o process_slice process_slice.cpp -std=c++0x

diverge_and_merge_point: diverge_and_merge_point.cpp
	g++ -g -o diverge_and_merge_point diverge_and_merge_point.cpp -std=c++0x

finalize_slice_ctrl_flow: finalize_slice_ctrl_flow.cpp
	g++ -g -o finalize_slice_ctrl_flow finalize_slice_ctrl_flow.cpp -std=c++0x

launcher: libutil.a
launcher: CFLAGS += --static -I../../eglibc-2.15/prefix/include -L../../eglibc-2.15/prefix/lib 

resume: resume.cpp libutil.a
	c++ -std=c++0x resume.cpp libutil.a -o resume -pthread --static -ldl

canonicalize: CPPFLAGS += -std=c++0x

getstats: libutil.a

filemap: libutil.a 

parseckpt: libutil.a

parsecheckpoint: libutil.a parsecheckpoint.o

fix_pckpt: libutil.a fix_pckpt.o

parserecheck: parserecheck.o

wparserecheck: 

parseklog: libutil.a parseklog.o parseklib.o
	$(CC) $(CFLAGS) $(LDFLAGS) parseklib.o parseklog.o -o parseklog -lssl -lcrypto

sdaemon: libutil.a sdaemon.cpp parseklib.o libutil.a ../linux-lts-quantal-3.5.0/include/linux/replay_configs.h
	g++ $(CFLAGS) $(LDFLAGS) parseklib.o sdaemon.cpp libutil.a -o sdaemon -std=c++0x

xdiff: libutil.a xdiff.cpp parseklib.o libutil.a
	g++ $(CFLAGS) $(LDFLAGS) parseklib.o xdiff.cpp libutil.a -o xdiff -std=c++0x

xdump: libutil.a xdump.cpp parseklib.o libutil.a
	g++ $(CFLAGS) $(LDFLAGS) parseklib.o xdump.cpp libutil.a -o xdump -std=c++0x

rewriteklog: parseklib.o

libutil.a: util.o parseklib.o
	ar rv $@ $^
	ranlib $@

parseklib.o: $(KENREL_CONFIG) 

parseulib.o: $(KENREL_CONFIG)  parseulib.cpp parseulib.h
	c++ -c -std=c++0x parseulib.cpp -o parseulib.o

reset_ndx: libutil.a

currentpid: libutil.a

runpintool: libutil.a 
runpintool: LDLIBS += -lrt 

seqtt: libutil.a 
seqtt: LDLIBS += -lrt 

partt: libutil.a
partt: LDFLAGS += -pthread


thread_test: LDFLAGS += -pthread 



streamtt: libutil.a streamtt.cpp 
	c++ -std=c++0x streamtt.cpp libutil.a -pthread -lrt -o streamtt -Wall -Werror

streamctl: streamctl.cpp streamserver.h parseklib.o streamnw.cpp streamnw.h
	c++ -Wall -Werror -std=c++0x streamctl.cpp streamnw.cpp parseklib.o -o streamctl -g

retaint: retaint_test.cpp libutil.a
	c++ -Wall -Werror -std=c++0x retaint_test.cpp parseklib.o libutil.a -o retaint -g


fulltt: libutil.a

fullsend: libutil.a

read_trace: read_trace.cpp
	c++ -Wall -Werror  -std=c++0x read_trace.cpp -lrt -o read_trace -g

mkpartition: libutil.a mkpartition.cpp 
	c++ -Wall -Werror -std=c++0x mkpartition.cpp libutil.a -pthread -lrt -o mkpartition -O8 

mkpartition_utils.o: mkpartition_utils.cpp mkpartition_utils.h
	c++ -std=c++0x mkpartition_utils.cpp -o mkpartition_utils.o -c -O8

mkpart_iter: mkpartition_utils.o libutil.a generate_splits.cpp mkpartition_iter.cpp
	c++ -std=c++0x mkpartition_utils.o libutil.a generate_splits.cpp mkpartition_iter.cpp -o mkpart_iter -O8

sync_files: sync_files.cpp streamserver.h parseklib.o streamnw.cpp streamnw.h
	c++ -Wall -Werror -std=c++0x sync_files.cpp parseklib.o streamnw.cpp -o sync_files 

recheck_support.o: recheck_support.c ../dift/recheck_log.h
	gcc recheck_support.c -c -o recheck_support.o -Wall -lrt -lpthread

parsepckpt: parsepckpt.c
	gcc -o parsepckpt parsepckpt.c

parseverif: parseverif.c
	gcc -o parseverif parseverif.c

.PHONY: clean
clean:
	rm -f *.a *.o $(TARGETS)

.deps/%.d: %.c .dep_dirs_made
	@$(CC) $(CFLAGS) -M -o $@ $<

.dep_dirs_made:
	@touch .dep_dirs_made
	@mkdir .deps

-include $(DEPS)

.PHONY: nuke
nuke: clean
	rm -rf .deps .dep_dirs_made

