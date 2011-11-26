CROSS=xenon-
CC=$(CROSS)gcc
CXX=$(CROSS)g++
OBJCOPY=$(CROSS)objcopy
LD=$(CROSS)ld
AS=$(CROSS)as
STRIP=$(CROSS)strip

ifeq ($(strip $(DEVKITXENON)),)
$(error "please set DEVKITXENON")
endif

# ipv4 or ipv6
LWIP_MODE = ipv4

INCLUDES = -I ../../drivers -I ../../include \
	-I ../../drivers/nocfe \
	-I ../../drivers/lwip/include/ -I ../../drivers/lwip/xenon/include/ -I ../../drivers/lwip/include/$(LWIP_MODE)/


CFLAGS = -g -mcpu=cell -mtune=cell -Os \
	-maltivec \
	-ffunction-sections -fdata-sections \
	-Wall -Wno-format -Wno-attributes -I. -DBYTE_ORDER=BIG_ENDIAN \
	-m32 -fno-pic -mpowerpc64 \
	-D_CFE_=1 -DENDIAN_BIG=1 \
	$(INCLUDES)

# we need to fix the toolchain to work with multilib here.
LIBDIR32 = $(DEVKITXENON)/xenon/lib/32/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -fpermissive -Wno-sign-compare -Werror=int-to-pointer-cast \
	-Werror=pointer-to-int-cast -Wno-unknown-pragmas  -Wno-unused-value -Wno-unused-variable 

AFLAGS = -Iinclude -m32 $(INCLUDES)

LIBOBJS = crti.o crtn.o \
	../../startup/xenon/startup_from_xell.o \
	../../startup/xenon/crt1.o \
	../../drivers/console/console.o \
	../../drivers/console/telnet_console.o \
	../../drivers/diskio/diskio.o \
	../../drivers/diskio/disk_rb.o \
	../../drivers/diskio/ata.o \
	../../drivers/fat/fat.o \
	../../drivers/fat/fat_rb.o \
	../../drivers/fat/file_rb.o \
	../../drivers/fat/dir_rb.o \
	../../drivers/iso9660/iso9660.o \
	../../drivers/input/input.o \
	../../drivers/newlib/newlib.o \
	../../drivers/nocfe/lib_malloc.o \
	../../drivers/nocfe/lib_queue.o \
	../../drivers/pci/io.o \
	../../drivers/ppc/atomic.o \
	../../drivers/ppc/cache.o \
	../../drivers/ppc/except.o \
	../../drivers/ppc/c_except.o \
	../../drivers/ppc/vm.o \
	../../drivers/time/time.o \
	../../drivers/usb/ohci.o \
	../../drivers/usb/usbctrl.o \
	../../drivers/usb/usbd.o \
	../../drivers/usb/usbdebug.o \
	../../drivers/usb/usbdevs.o \
	../../drivers/usb/usbhid.o \
	../../drivers/usb/usbhub.o \
	../../drivers/usb/usbmain.o \
	../../drivers/usb/usbmass.o \
	../../drivers/usb/tinyehci/usb_os.o \
	../../drivers/usb/tinyehci/tinyehci.o \
	../../drivers/utils/unicode_rb.o \
	../../drivers/utils/debug.o \
	../../drivers/utils/gmon.o \
	../../drivers/xenon_smc/xenon_gpio.o \
	../../drivers/xenon_smc/xenon_smc.o \
	../../drivers/xenon_soc/cpusleep.o \
	../../drivers/xenon_soc/xenon_power.o \
	../../drivers/xenon_soc/xenon_secotp.o \
	../../drivers/xenon_sound/sound.o \
	../../drivers/xenon_uart/xenon_uart.o \
	../../drivers/xenon_nand/xenon_sfcx.o \
	../../drivers/xenon_nand/xenon_config.o \
	../../drivers/xenos/ucode.o \
	../../drivers/xenos/edram.o \
	../../drivers/xenos/xe.o \
	../../drivers/xenos/xenos.o \
	../../drivers/xenos/xenos_edid.o \
	../../drivers/lwip/core/init.o \
	../../drivers/lwip/core/tcp_in.o \
	../../drivers/lwip/core/ipv4/inet_chksum.o \
	../../drivers/lwip/core/ipv4/inet.o \
	../../drivers/lwip/core/mem.o \
	../../drivers/lwip/core/memp.o \
	../../drivers/lwip/core/netif.o \
	../../drivers/lwip/core/pbuf.o \
	../../drivers/lwip/core/stats.o \
	../../drivers/lwip/core/sys.o \
	../../drivers/lwip/core/tcp.o \
	../../drivers/lwip/core/ipv4/ip_addr.o \
	../../drivers/lwip/core/ipv4/icmp.o \
	../../drivers/lwip/core/ipv4/ip.o \
	../../drivers/lwip/core/ipv4/ip_frag.o \
	../../drivers/lwip/core/tcp_out.o \
	../../drivers/lwip/core/udp.o \
	../../drivers/lwip/netif/etharp.o \
	../../drivers/lwip/netif/loopif.o \
	../../drivers/lwip/core/dhcp.o \
	../../drivers/lwip/core/raw.o \
	../../drivers/lwip/core/def.o \
	../../drivers/lwip/core/timers.o \
	../../drivers/lwip/xenon/src/lib.o \
	../../drivers/lwip/xenon/netif/enet.o \
	../../drivers/network/network.o\
	../../drivers/elf/elf.o \
	../../drivers/elf/elf_run.o


# Build rules
all: $(TARGETS)

clean:
	rm -rf $(OBJS) $(LIBOBJS) libxenon.a

.c.o:
	@echo [$(notdir $<)]
	@$(CC) -c $(CFLAGS) $*.c -o $*.o

.cpp.o:
	@echo [$(notdir $<)]
	@$(CXX) -c $(CXXFLAGS) $*.cpp -o $*.o

%.o: %.S
	@echo [$(notdir $<)]
	@$(CC) $(AFLAGS) -c $*.S -o $*.o

%.o: %.s
	@echo [$(notdir $<)]
	@$(CC) $(AFLAGS) -c $*.s -o $*.o

libxenon.a: $(LIBOBJS)
	ar rc $@ $(LIBOBJS)

install: libxenon.a
	mkdir -p $(DEVKITXENON)/usr/lib/
	cp libxenon.a $(DEVKITXENON)/usr/lib/
	mkdir -p $(DEVKITXENON)/usr/include/
	cp -r ../../drivers/* $(DEVKITXENON)/usr/include/
	cp ../../include/* $(DEVKITXENON)/usr/include/
	mkdir -p $(DEVKITXENON)/usr/include/lwip/
	cp ../../drivers/lwip/include/lwip/* $(DEVKITXENON)/usr/include/lwip/
	cp ../../drivers/lwip/include/$(LWIP_MODE)/lwip/* $(DEVKITXENON)/usr/include/lwip/
	mkdir -p $(DEVKITXENON)/usr/include/netif/
	cp ../../drivers/lwip/include/netif/* $(DEVKITXENON)/usr/include/netif/
	mkdir -p $(DEVKITXENON)/usr/include/arch/
	cp ../../drivers/lwip/xenon/include/arch/* $(DEVKITXENON)/usr/include/arch/
	mkdir -p $(DEVKITXENON)/usr/include/sys/
	cp ../../drivers/newlib/dirent.h $(DEVKITXENON)/usr/include/sys/
	find $(DEVKITXENON)/usr/include/ -type f \! -name "*.h" -delete
