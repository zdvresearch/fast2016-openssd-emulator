
INCLUDES = -I../openSSD/ftl -I./include -I ./include/target_spw/
CC = clang
CFLAGS = -g -std=c99 -O0

SRC_DIRS = ../openSSD/ftl

VPATH = ../openSSD/ftl/

SRCS = $(foreach sdir, $(SRC_DIRS), $(wildcard $(sdir)/*.c)) mock.c

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) main.c -o simulator_disksim_traces

clean:
	rm simulator simulator_disksim_traces latency_experiment simulator_disksim_traces_tags
