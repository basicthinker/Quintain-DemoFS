KSRC = /usr/src/linux-source-3.2.0/
ccflags-y += -DCONFIG_CGROUP_MEM_RES_CTLR

obj-m += quintain.o
quintain-objs = file.o inode.o

default :
	$(MAKE) -C $(KSRC) M=`pwd`

clean :
	$(MAKE) -C $(KSRC) M=`pwd` clean
