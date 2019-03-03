KDIR=/lib/modules/$(shell uname -r)/build

obj-m += calc.o 
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS)
	make -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

check: all
	scripts/test.sh

clean:
	make -C $(KDIR) M=$(PWD) clean
