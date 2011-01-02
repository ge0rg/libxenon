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

CFLAGS = -mcpu=cell -mtune=cell -O2 \
	-Wall -I. -DBYTE_ORDER=BIG_ENDIAN \
	-I ../../drivers -I ../../include -Wall  \
	-m32 -fno-pic -mpowerpc64 \
	-I ../../drivers/nocfe -D_CFE_=1 -DENDIAN_BIG=1

# we need to fix the toolchain to work with multilib here.
LIBDIR32 = $(DEVKITXENON)/xenon/lib/32/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -fpermissive -Wno-sign-compare -Werror=int-to-pointer-cast \
	-Werror=pointer-to-int-cast -Wno-unknown-pragmas  -Wno-unused-value -Wno-unused-variable 

AFLAGS = -Iinclude -m32 
LDFLAGS = -L$(DEVKITXENON)/usr/lib -lxenon -lstdc++ -lm -m32

LIBOBJS = crti.o crtn.o \
	../../startup/xenon/startup_from_xell.o \
	../../startup/xenon/crt1.o \
	../../drivers/console/console.o \
	../../drivers/diskio/diskio.o \
	../../drivers/diskio/disk_rb.o \
	../../drivers/diskio/dvd.o \
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
	../../drivers/utils/unicode_rb.o \
	../../drivers/xenon_smc/xenon_gpio.o \
	../../drivers/xenon_smc/xenon_smc.o \
	../../drivers/xenon_soc/cpusleep.o \
	../../drivers/xenon_soc/xenon_power.o \
	../../drivers/xenon_sound/sound.o \
	../../drivers/xenon_uart/xenon_uart.o \
	../../drivers/xenos/ucode.o \
	../../drivers/xenos/xe.o \
	../../drivers/xenos/xenos.o

OBJS_MAIN = main.o ffs_content.o xee.o engine.o exception.o

TARGETS=main.elf32

# Build rules
all: $(TARGETS)

clean:
	rm -rf $(OBJS) $(LIBOBJS) $(TARGETS) libxenon.a main.elf

ffs_content.c: genffs.py files/ps.psu files/vs.vsu
	python genffs.py > ffs_content.c

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

main.elf: main.lds $(OBJS_MAIN) crt0.o crt1.o crti.o crtn.o
	$(CC) -n -T $< -o $@ $(OBJS_MAIN) $(LDFLAGS)  -Wl,-Map -Wl,main.map  -L $(LIBDIR32)

main.elf32: main.elf
	$(OBJCOPY) -O elf32-powerpc --adjust-vma 0x80000000 $< $@
	$(STRIP) -s $@
	echo "finished"

libxenon.a: $(LIBOBJS)
	ar rc $@ $(LIBOBJS)

run:
	cp main.elf32 /tftpboot/xenon
#	/home/dev360/run

install: libxenon.a
	mkdir -p $(DEVKITXENON)/usr/lib/
	cp libxenon.a $(DEVKITXENON)/usr/lib/
	mkdir -p $(DEVKITXENON)/usr/include/
	cp -r ../../drivers/* $(DEVKITXENON)/usr/include/
	cp ../../include/* $(DEVKITXENON)/usr/include/ 
	find $(DEVKITXENON)/usr/include/ -type f \! -name "*.h" -delete
