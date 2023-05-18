README_FILE=README.md
UID = $(shell cat ${README_FILE} | grep '\#\# UID'| grep -oe '\([0-9.]*\)')
SUBMISSION_FILES = proc_count.c ${README_FILE}

ifneq ($(KERNELRELEASE),)
obj-m := proc_count.o
else
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD modules

modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

install:
	$(MAKE) -C $(KDIR) M=$$PWD install

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	rm -f ${UID}-lab0-submission.tar
endif

tar:
	@tar -cf ${UID}-lab0-submission.tar ${SUBMISSION_FILES}
