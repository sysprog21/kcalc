OUT ?= build

EXEC := \
	test-simple \
	test-unit \
	test-bench
EXEC := $(addprefix $(OUT)/,$(EXEC))

all: $(EXEC)

.PHONY: check clean

CC ?= gcc
CFLAGS = -Wall -std=c99 -g -O2 -I.
LDFLAGS = -lm

OBJS := \
	expression.o

deps := $(OBJS:%.o=%.o.d)
OBJS := $(addprefix $(OUT)/,$(OBJS))
deps := $(addprefix $(OUT)/,$(deps))

$(OUT)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ -MMD -MF $@.d $<

$(OUT)/test-%: test-%.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): | $(OUT)

$(OUT):
	mkdir -p $@

.PRECIOUS: %.o

check: $(EXEC)
	@for test in $^ ; \
	do \
		echo "Execute $$test..." ; $$test && echo "OK!\n" ; \
	done

clean:
	$(RM) $(EXEC) $(OBJS) $(deps)
	@rm -rf $(OUT)

-include $(deps)
