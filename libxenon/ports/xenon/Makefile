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

CFLAGS = -Wall  -O2 -I. -DBYTE_ORDER=BIG_ENDIAN \
	-I ../../drivers -I ../../include -Wall  \
	-m32 -fno-pic -mpowerpc64 \
	-I ../../drivers/nocfe -D_CFE_=1 -DENDIAN_BIG=1

# we need to fix the toolchain to work with multilib here.
LIBDIR32 = /home/.../cross/xenon/lib/32

CXXFLAGS = $(CFLAGS) 	-fno-exceptions -fno-rtti -fpermissive -Wno-sign-compare -Werror=int-to-pointer-cast \
	-Werror=pointer-to-int-cast -Wno-unknown-pragmas  -Wno-unused-value -Wno-unused-variable 

AFLAGS = -Iinclude -m32 
LDFLAGS = -lstdc++ -lm -m32 

OBJS = ../../startup/xenon/startup_from_xell.o \
	../../startup/xenon/crt1.o \
	../../drivers/newlib/newlib.o \
	../../drivers/xenon_uart/xenon_uart.o \
	../../drivers/xenos/xe.o \
	../../drivers/xenos/xenos.o \
	../../drivers/xenon_smc/xenon_smc.o \
	../../drivers/xenon_smc/xenon_gpio.o \
	../../drivers/pci/io.o \
	../../drivers/time/time.o \
	../../drivers/ppc/cache.o \
	../../drivers/ppc/vm.o \
	../../drivers/ppc/except.o \
	../../drivers/usb/ohci.o \
	../../drivers/usb/usbd.o \
	../../drivers/usb/usbdebug.o \
	../../drivers/usb/usbdevs.o \
	../../drivers/usb/usbhid.o \
	../../drivers/usb/usbctrl.o \
	../../drivers/usb/usbhub.o \
	../../drivers/usb/usbmain.o \
	../../drivers/nocfe/lib_malloc.o \
	../../drivers/nocfe/lib_queue.o \
	../../drivers/usb/usbmass.o \
	../../drivers/diskio/diskio.o \
	../../drivers/input/input.o \
	xee.o engine.o exception.o

	
OBJS_MAIN = main.o ffs_content.o test.o

TARGETS=main.elf32

# Build rules
all: $(TARGETS)

clean:
	rm -rf $(OBJS) $(TARGETS) main.elf

ffs_content.c: genffs.py files/ps.psu files/vs.vsu
	python genffs.py > ffs_content.c

.c.o:
	$(CC) -c $(CFLAGS) $*.c -o $*.o

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $*.cpp -o $*.o

%.o: %.S
	$(CC) $(AFLAGS) -c $*.S -o $*.o

%.o: %.s
	$(CC) $(AFLAGS) -c $*.s -o $*.o

main.elf: main.lds $(OBJS) $(OBJS_MAIN) crt0.o crt1.o crti.o crtn.o
	$(CC) -n -T $< -o $@ $(OBJS) $(OBJS_MAIN) $(LDFLAGS)  -Wl,-Map -Wl,main.map  -L $(LIBDIR32)

main.elf32: main.elf
	$(OBJCOPY) -O elf32-powerpc --adjust-vma 0x80000000 $< $@
	$(STRIP) -s $@
	echo "finished"

libxenon.a: $(OBJS)
	ar rc $@ $(OBJS)

run:
	cp main.elf32 /tftpboot/xenon
	/home/dev360/run

install: libxenon.a
	mkdir -p $(DEVKITXENON)/libxenon/lib/xenon/
	cp libxenon.a $(DEVKITXENON)/libxenon/lib/xenon/
	mkdir -p $(DEVKITXENON)/libxenon/include/
	cp -r ../../drivers/* $(DEVKITXENON)/libxenon/include/
	cp ../../include/* $(DEVKITXENON)/libxenon/include/ 
	find $(DEVKITXENON)/libxenon/include/ -type f \! -name "*.h" -delete
