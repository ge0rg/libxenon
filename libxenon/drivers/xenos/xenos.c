#include <xenon_smc/xenon_smc.h>
#include <xenon_smc/xenon_gpio.h>
#include <pci/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define require(x, label) if (!(x)) { printf("%s:%d [%s]\n", __FILE__, __LINE__, #x); goto label; }

static void xenos_write32(int reg, uint32_t val)
{
	write32n(0x200ec800000 + reg, val);
//	printf("set reg %4x %08x, read %08x\n", reg, val, read32n(0x200ec800000 + reg));
}

static uint32_t xenos_read32(int reg)
{
	return read32n(0x200ec800000 + reg);
}

static void xenos_ana_write(int addr, uint32_t reg)
{
	xenos_write32(0x7950, addr);
	xenos_read32(0x7950);
	xenos_write32(0x7954, reg);
	while (xenos_read32(0x7950) == addr) printf(".");
}

uint32_t ana_640x480p [] = {
0x0000004f, 0x00000003, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 
0x00060001, 0x00000000, 0x00000000, 0xffffffff, 0xac0000d0, 0x00000009, 0x1c0ffc00, 0x00000000, 
0x24900000, 0x00087400, 0xf8461778, 0x48280320, 0x002c1061, 0x4601120d, 0x00000000, 0x02064f2e, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x81612c88, 0x00000000, 0x10b3c8c3, 
0x00000000, 0x00080c0b, 0x00000000, 0x07e7d5dc, 0x00000000, 0x07bfb3d4, 0x00000000, 0x0014180e, 
0x00000000, 0x0000fe00, 0x0000fffd, 0xfd000000, 0x81612c88, 0x00000000, 0x10b3c8c3, 0x00000000, 
0x00080c0b, 0x00000000, 0x07e7d5dc, 0x00000000, 0x07bfb3d4, 0x00000000, 0x0014180e, 0x00000000, 
0x0000fe00, 0x0000fffd, 0xfd000000, 0x40000000, 0x00000000, 0x00000000, 0x00000000, 0x40000000, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xdeadbeef, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0f897040, 0x00b85b53, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 

0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 

0x000562de, 0x001162de, 0x002cfe1f, 0x00205218, 0x00115a17, 0x00000060, 0x00000060, 0x00000000, 
0x0000000f, 0x0001ee60, 0x00000002, 0x00000003, 0x000042aa, 0xffffffff, 0xffffffff, 0xffffffff, 

0x000086d7, 0x00000000, 0x00000000, 0x8436f666, 0x0000001b, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 

0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000001, 0x00000000, 
};

uint32_t ana_1024x768 [] = {
0x0000004f, 0x00000003, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000,  // 00
0x00060001, 0x00000000, 0x00000000, 0xffffffff, 0x8c0000d0, 0x00000009, 0x1c0ffc00, 0x00000000,  // 08
0x24900000, 0x00087400, 0xf8c89778, 0x94400540, 0x004b9089, 0x46018326, 0x00000000, 0x020ffe5d,  // 10
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 18
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000000, 0x00000000,  // 20
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,  // 28
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc9500000, 0x00000000, 0x1e300000,  // 30
0x00000000, 0x002c0000, 0x00000000, 0x07580000, 0x00000000, 0x06d00000, 0x00000000, 0x00580000,  // 38
0x00000000, 0x00000000, 0x0000fb00, 0x00000000, 0xc9500000, 0x00000000, 0x1e300000, 0x00000000,  // 40
0x002c0000, 0x00000000, 0x07580000, 0x00000000, 0x06d00000, 0x00000000, 0x00580000, 0x00000000,  // 48
0x00000000, 0x0000fb00, 0x00000000, 0x40000000, 0x00000000, 0x00000000, 0x00000000, 0x40000000,  // 50
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 58
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 60
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xdeadbeef, 0x00000000, 0x00000000,  // 68
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1a9894ee, 0x001db0a9, 0xffffffff,  // 70
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 78
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 80
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 88
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 90
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 98
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b8
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c8
0x000551d4, 0x000551d4, 0x002cfe1f, 0x00205218, 0x00115a17, 0x00000060, 0x000000f0, 0x00000000,  // d0
0x00000007, 0x0001ee60, 0x00000002, 0x00000004, 0x000042aa, 0xffffffff, 0xffffffff, 0xffffffff,  // d8
0x00000000, 0x00000000, 0x00000000, 0x8436f666, 0x0000001b, 0xffffffff, 0xffffffff, 0xffffffff,  // e0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // e8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // f0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000001, 0x00000000,  // f8
};

uint32_t ana_640_480_composite[] = {
0x0000005f, 0x00000003, 0x00000000, 0x00000007, 0x00000000, 0xffffffff, 0xe880a916, 0x07001482,  // 00
0x00060001, 0x00000029, 0x00000000, 0xffffffff, 0xd8000002, 0x00000001, 0x1c0ffc40, 0x06cf00d8,  // 08
0x24900000, 0x1789e000, 0x90674700, 0x74a8030c, 0x00299039, 0x2651b20d, 0x88773443, 0x0300ebff,  // 10
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 18
0x02540f38, 0x0e600002, 0x02540409, 0x00000002, 0x02540000, 0x03310002, 0x83713c67, 0x081268b2,  // 20
0x106408f9, 0x0ed370c8, 0x00381c0e, 0x00341809, 0x0783b7d7, 0x0753a9d6, 0x00841dfd, 0x07bfc7db,  // 28
0x079fdbf4, 0x07e80005, 0xfdfdfdfe, 0xff010c0a, 0x09060402, 0x80c09c47, 0x069234b1, 0x13b4d123,  // 30
0x1103ccd3, 0x00040606, 0x0024180e, 0x07f3e9eb, 0x078bb3d2, 0x07d7c7d7, 0x073f9bce, 0x000c120d,  // 38
0x003c2010, 0x000000fe, 0xfefd00fe, 0xfdfcfcfd, 0x80c09c45, 0x067228ad, 0x1344b11d, 0x1083b4cf,  // 40
0x00040606, 0x0024180e, 0x07f3e9eb, 0x078bb5d3, 0x07d7c9d8, 0x07479dcf, 0x000c120d, 0x003c200f,  // 48
0x000000ff, 0xfefd00fe, 0xfdfcfcfd, 0x40001f18, 0x0e030d0c, 0x79566e05, 0x56793e3e, 0x4000056e,  // 50
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 58
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 60
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xdeadbeef, 0x00000000, 0x00000000,  // 68
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x78870305, 0x00314733, 0xffffffff,  // 70
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 78
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 80
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 88
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 90
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 98
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b8
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c8
0x00055a9d, 0x001f4a9f, 0x002cfe1f, 0x00205218, 0x00115a17, 0x00000000, 0x00000060, 0x00000000,  // d0
0x0000000f, 0x0001ee60, 0x00000002, 0x00000004, 0x000042aa, 0xffffffff, 0xffffffff, 0xffffffff,  // d8
0x00000000, 0x00000000, 0x00000000, 0x8436f666, 0x0000001b, 0xffffffff, 0xffffffff, 0xffffffff,  // e0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // e8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // f0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000001, 0x00000000,  // f8
};

uint32_t ana_yuv_480p[] = {
0x0000005f, 0x00000003, 0x00000000, 0x00000007, 0x00000000, 0xffffffff, 0xe880a916, 0x07001482,  // 00
0x00060001, 0x00000029, 0x00000000, 0xffffffff, 0xd8000002, 0x00000001, 0x1c0ffc40, 0x06cf00d8,  // 08
0x24900000, 0x1789e000, 0x90674700, 0x74a8030c, 0x00299039, 0x2651b20d, 0x88773443, 0x0300ebff,  // 10
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 18
0x02540f38, 0x0e600002, 0x02540409, 0x00000002, 0x02540000, 0x03310002, 0x83713c67, 0x081268b2,  // 20
0x106408f9, 0x0ed370c8, 0x00381c0e, 0x00341809, 0x0783b7d7, 0x0753a9d6, 0x00841dfd, 0x07bfc7db,  // 28
0x079fdbf4, 0x07e80005, 0xfdfdfdfe, 0xff010c0a, 0x09060402, 0x80c09c47, 0x069234b1, 0x13b4d123,  // 30
0x1103ccd3, 0x00040606, 0x0024180e, 0x07f3e9eb, 0x078bb3d2, 0x07d7c7d7, 0x073f9bce, 0x000c120d,  // 38
0x003c2010, 0x000000fe, 0xfefd00fe, 0xfdfcfcfd, 0x80c09c45, 0x067228ad, 0x1344b11d, 0x1083b4cf,  // 40
0x00040606, 0x0024180e, 0x07f3e9eb, 0x078bb5d3, 0x07d7c9d8, 0x07479dcf, 0x000c120d, 0x003c200f,  // 48
0x000000ff, 0xfefd00fe, 0xfdfcfcfd, 0x40001f18, 0x0e030d0c, 0x79566e05, 0x56793e3e, 0x4000056e,  // 50
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 58
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 60
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xdeadbeef, 0x00000000, 0x00000000,  // 68
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x78870305, 0x00314733, 0xffffffff,  // 70
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 78
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 80
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 88
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 90
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 98
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b8
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c8
0x00055a9d, 0x001f4a9f, 0x002cfe1f, 0x00205218, 0x00115a17, 0x00000060, 0x00000060, 0x00000000,  // d0
0x0000000f, 0x0001ee60, 0x00000002, 0x00000004, 0x000042aa, 0xffffffff, 0xffffffff, 0xffffffff,  // d8
0x00000000, 0x00000000, 0x00000000, 0x8436f666, 0x0000001b, 0xffffffff, 0xffffffff, 0xffffffff,  // e0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // e8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // f0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000001, 0x00000000,  // f8
};


uint32_t ana_pal[] = {
0x0000005f, 0x00000003, 0x00000000, 0x00000007, 0x00000000, 0xffffffff, 0x4bc44112, 0x08c001c6,  // 00
0x00000008, 0x00000029, 0x00000000, 0xffffffff, 0xd800002a, 0x00000001, 0x1c0ffc40, 0x06cf00d8,  // 08
0x24900000, 0x1539e000, 0x90574700, 0x7f280310, 0x0029e03a, 0x2c5d2271, 0x66573423, 0x050093d4,  // 10
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 18
0x02540f38, 0x0e600002, 0x02540409, 0x00000002, 0x02540000, 0x03310002, 0x8281085e, 0x07b264b5,  // 20
0x11a4550b, 0x0fb39cd0, 0x00301c10, 0x00442410, 0x07a3c1d9, 0x0753a1cf, 0x0047fbec, 0x0783add1,  // 28
0x07c3f1ff, 0x0014140e, 0xfcfcfbfb, 0xfbfc0906, 0x0401fffd, 0x80c09c47, 0x069234b1, 0x13b4d123,  // 30
0x1103ccd3, 0x00040606, 0x0024180e, 0x07f3e9eb, 0x078bb3d2, 0x07d7c7d7, 0x073f9bce, 0x000c120d,  // 38
0x003c2010, 0x000000fe, 0xfefd00fe, 0xfdfcfcfd, 0x80c09c45, 0x067228ad, 0x1344b11d, 0x1083b4cf,  // 40
0x00040606, 0x0024180e, 0x07f3e9eb, 0x078bb5d3, 0x07d7c9d8, 0x07479dcf, 0x000c120d, 0x003c200f,  // 48
0x000000ff, 0xfefd00fe, 0xfdfcfcfd, 0x40001f18, 0x0e030d0c, 0x79566e05, 0x56793e3e, 0x4000056e,  // 50
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 58
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 60
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xdeadbeef, 0x00000000, 0x00000000,  // 68
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x711bb61b, 0x00ecd9c7, 0xffffffff,  // 70
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 78
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 80
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 88
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 90
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // 98
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // a8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // b8
0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // c8
0x00056a14, 0x001a3154, 0x002cfe1f, 0x00205218, 0x00115a17, 0x00000060, 0x00000060, 0x00000000,  // d0
0x0000000f, 0x0001ee60, 0x00000002, 0x00000004, 0x000042aa, 0xffffffff, 0xffffffff, 0xffffffff,  // d8
0x946b0000, 0x00000000, 0x00000000, 0x8436f666, 0x0000001b, 0xffffffff, 0xffffffff, 0xffffffff,  // e0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // e8
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,  // f0
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000001, 0x00000000,  // f8
};

#define MODE_VGA_640x480  0
#define MODE_VGA_1024x768 1
#define MODE_PAL60        2
#define MODE_480p         3
#define MODE_PAL50        4

struct mode_s
{
	uint32_t *ana;
	int total_width, total_height, hsync_offset, real_active_width, active_height, vsync_offset, is_progressive, width, height, composite_sync, rgb;
} xenos_modes[] = {
	{
		.ana = ana_640x480p,
		.total_width = 800,
		.total_height = 525,
		.hsync_offset = 0x56,
		.real_active_width = 640,
		.active_height = 480,
		.vsync_offset = 0x23,
		.is_progressive = 1,
		.width = 640,
		.height = 480,
		.rgb = 1,
	}, 
	{
		.ana = ana_1024x768,
		.total_width = 1344,
		.hsync_offset = 235,
		.real_active_width = 1024,
		.total_height = 806,
		.vsync_offset = 35,
		.active_height = 768,
		.width = 1024,
		.height = 768,
		.is_progressive = 1,
		.rgb = 1,
	},
	{
		.ana = ana_640_480_composite,
		.total_width = 780,
		.hsync_offset = 80,
		.real_active_width = 640,
		.total_height = 525,
		.vsync_offset = 38,
		.active_height = 480/2,
		.is_progressive = 0,
		.width = 640,
		.height = 480/2,
		.composite_sync = 1,
	},
	{
		.ana = ana_yuv_480p,
		.total_width = 780,
		.hsync_offset = 82,
		.real_active_width = 640,
		.total_height = 525,
		.vsync_offset = 38,
		.active_height = 480,
		.width = 640,
		.height = 480,
		.is_progressive = 1,
		.composite_sync = 1,
	},
	{
		.ana = ana_pal,
		.total_width = 784,
		.hsync_offset = 91,
		.real_active_width = 640,
		.total_height = 625,
		.vsync_offset = 44,
		.active_height = 576/2,
		.is_progressive = 0,
		.width = 640,
		.height = 576/2,
		.is_progressive = 0,
		.composite_sync = 1,
	}
	};

void xenos_init_ana_new(uint32_t *mode_ana)
{
	uint32_t tmp;
	
	require(!xenon_smc_ana_read(0xfe, &tmp), ana_error);
	
	require(!xenon_smc_ana_read(0xD9, &tmp), ana_error);
	tmp &= ~(1<<18);
	require(!xenon_smc_ana_write(0xD9, tmp), ana_error);

	int addr_0[] =    {0,   0xD5, 0xD0,    0xD1,     0xD6, 0xD8, 0xD, 0xC};
//	uint32_t val[] =  {0x0, 0x60, 0x55a9d, 0x1f4a9f, 0x60, 0xb,  1,   0xd8000000};
	
	xenon_smc_ana_write(0, 0);
	int i;
	for (i = 1; i < 8; ++i)
	{
		uint32_t old;
//		xenon_smc_ana_read(addr_0[i], &old);
//		require(!xenon_smc_ana_write(addr_0[i], val[i]), ana_error);
		require(!xenon_smc_ana_write(addr_0[i], mode_ana[addr_0[i]]), ana_error);
		if (addr_0[i] == 0xd6)
			udelay(1000);
//		xenon_smc_ana_read(addr_0[i], &tmp);
	}
	
	require(!xenon_smc_ana_write(0, 0x60), ana_error);
	
	uint32_t old;
	xenos_write32(0x7938, (old = xenos_read32(0x7938)) & ~1);
	xenos_write32(0x7910, 1);
	xenos_write32(0x7900, 1);

	int addr_2[] = {2, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76};
	
	for (i = 0; i < 0x11; ++i)
		xenos_ana_write(addr_2[i], mode_ana[addr_2[i]]);

	for (i = 0x26; i <= 0x34; ++i)
		xenos_ana_write(i, mode_ana[i]);

	for (i = 0x35; i <= 0x43; ++i)
		xenos_ana_write(i, mode_ana[i]);

	for (i = 0x44; i <= 0x52; ++i)
		xenos_ana_write(i, mode_ana[i]);
	
	for (i = 0x53; i <= 0x54; ++i)
		xenos_ana_write(i, mode_ana[i]);
	
	for (i = 0x55; i <= 0x57; ++i)
		xenos_ana_write(i, mode_ana[i]);

	int addr_8[] = {3, 6, 7, 8, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};

	for (i = 0; i < 0x11; ++i)
		xenos_ana_write(addr_8[i], mode_ana[addr_8[i]]);

	xenos_write32(0x7938, old);

	xenon_smc_ana_write(0, mode_ana[0]);

	return;
	
ana_error:
	printf("error reading/writing ana\n");
}



void xenos_init_phase0(void)
{
	xenos_write32(0xe0, 0x10000000);
	xenos_write32(0xec, 0xffffffff);

	xenos_write32(0x1724, 0);
	int i;
	for (i = 0; i < 8; ++i)
	{
		int addr[] = {0x8000, 0x8c60, 0x8008, 0x8004, 0x800c, 0x8010, 0x8014, 0x880c, 0x8800, 0x8434, 0x8820, 0x8804, 0x8824, 0x8828, 0x882c, 0x8420, 0x841c, 0x841c, 0x8418, 0x8414, 0x8c78, 0x8c7c, 0x8c80};
		int j;
		for (j = 0; j < 23; ++j)
			xenos_write32(addr[j] + i * 0x1000, 0);
	}
}

void xenos_init_phase1(void)
{

	uint32_t v;
	xenos_write32(0x0244, 0x20000000); // 
	v = 0x200c0011;
	xenos_write32(0x0244, v);
	udelay(1000);
	v &=~ 0x20000000;
	xenos_write32(0x0244, v);
	udelay(1000);
	
	assert(xenos_read32(0x0244) == 0xc0011);

	xenos_write32(0x0218, 0);
	xenos_write32(0x3c04, 0xe);
	xenos_write32(0x00f4, 4);
	
	int gpu_clock = xenos_read32(0x0210);
	int mem_clock = xenos_read32(0x284);
	int edram_clock = xenos_read32(0x244);
	int fsb_clock = xenos_read32(0x248);
	assert(gpu_clock == 9);
	printf("GPU Clock at %d MHz\n", 50 * ((gpu_clock & 0xfff)+1) / (((gpu_clock >> 12) & 0x3f)+1));
	printf("MEM Clock at %d MHz\n", 50 * ((mem_clock & 0xfff)+1) / (((mem_clock >> 12) & 0x3f)+1));
	printf("EDRAM Clock at %d MHz\n", 100 * ((edram_clock & 0xfff)+1) / (((edram_clock >> 12) & 0x3f)+1));
	printf("FSB Clock at %d MHz\n", 100 * ((fsb_clock & 0xfff)+1) / (((fsb_clock >> 12) & 0x3f)+1));
	

	xenos_write32(0x0204, 0x400300); // 4400380 for jasper
	xenos_write32(0x0208, 0x180002);
	
	xenos_write32(0x01a8, 0);
	xenos_write32(0x0e6c, 0x0c0f0000);
	xenos_write32(0x3400, 0x40401);
	udelay(1000);
	xenos_write32(0x3400, 0x40400);
	xenos_write32(0x3300, 0x3a22);
	xenos_write32(0x340c, 0x1003f1f);
	xenos_write32(0x00f4, 0x1e);
	
	
	xenos_write32(0x2800, 0);
	xenos_write32(0x2804, 0x20000000);
	xenos_write32(0x2808, 0x20000000);
	xenos_write32(0x280c, 0);
	xenos_write32(0x2810, 0x20000000);
	xenos_write32(0x2814, 0);

	udelay(1000);	
	xenos_write32(0x6548, 0);
	
		// resetgui
		// initcp
		
	xenos_write32(0x04f0, (xenos_read32(0x04f0) &~ 0xf0000) | 0x40100);
}

static void f1(void)
{
	uint32_t tmp;
	xenon_smc_ana_read(0, &tmp);
	tmp &= ~0x1e;
	xenon_smc_ana_write(0, tmp);
}

void xenos_ana_preinit(void)
{
	f1();
	uint32_t v;
	xenos_write32(0x6028, v = (xenos_read32(0x6028) | 0x01000000));
	while (!(xenos_read32(0x281c) & 2));
	xenon_smc_ana_write(0, 0);
	xenon_smc_ana_write(0xd7, 0xff);
	xenos_write32(0x6028, v & ~0x301);
	xenos_write32(0x7900, 0);
	xenon_smc_ana_read(0xd9, &v);
	xenon_smc_ana_write(0xd9, v | 0x40000);
	// write i2c: ...
	
}

void xenos_set_mode_f1(struct mode_s *mode)
{
	xenos_write32(0x60e8, 1);
	xenos_write32(0x6cbc, 3);
	xenos_write32(0x60ec, 0);
	xenos_write32(0x6020, 0);

	int interlace_factor = mode->is_progressive ? 1 : 2;
	
	xenos_write32(0x6000, mode->total_width - 1);
	xenos_write32(0x6010, mode->total_height - 1);
	xenos_write32(0x6004, (mode->hsync_offset << 16) | (mode->real_active_width + mode->hsync_offset));
	xenos_write32(0x6014, (mode->vsync_offset << 16) | (mode->active_height * interlace_factor + mode->vsync_offset));
	xenos_write32(0x6008, 0x100000);
	xenos_write32(0x6018, 0x60000);
	xenos_write32(0x6030, mode->is_progressive ? 0 : 1);
	xenos_write32(0x600c, 0);
	xenos_write32(0x601c, 0);

	xenos_write32(0x604c, 0);
	xenos_write32(0x6050, 0);
	xenos_write32(0x6044, 0);
	xenos_write32(0x6048, 0);

	xenos_write32(0x60e8, 0);
}



void xenos_set_mode_f2(struct mode_s *mode)
{
	int interlace_factor = mode->is_progressive ? 1 : 2;
	
	xenos_write32(0x6144, 1);
	xenos_write32(0x6120, mode->width); /* pitch */
	xenos_write32(0x6104, 2);
	xenos_write32(0x6108, 0); /* lut override */
	xenos_write32(0x6124, 0);
	xenos_write32(0x6128, 0);
	xenos_write32(0x612c, 0);
	xenos_write32(0x6130, 0);
	xenos_write32(0x6134, mode->width);
	xenos_write32(0x6138, mode->active_height * interlace_factor);
	xenos_write32(0x6110, 0x1f920000);
	xenos_write32(0x6120, mode->width);
	xenos_write32(0x6100, 1);
	xenos_write32(0x6144, 0);

			/* scaler update */
	xenos_write32(0x65cc, 1);
	xenos_write32(0x6590, 0);
	xenos_write32(0x2840, 0x1f920000);
	xenos_write32(0x2844, mode->width);
	xenos_write32(0x2848, 0x80000);
	
	xenos_write32(0x6580, 0); /* viewport */
	xenos_write32(0x6584, (mode->width << 16) | (mode->active_height * interlace_factor));
	xenos_write32(0x65e8, (mode->width >> 2) - 1);
	xenos_write32(0x6528, mode->is_progressive ? 0 : 1);
	xenos_write32(0x6550, 0xff);
	xenos_write32(0x6524, 0x300);
	xenos_write32(0x65d0, 0x100);
	xenos_write32(0x6148, 1);
	
	xenos_write32(0x6594, 0x905);
	xenos_write32(0x6584, (mode->width << 16) | (mode->height * interlace_factor));
	xenos_write32(0x65e8, (mode->width / 4) - 1);
	xenos_write32(0x6580, 0);
	xenos_write32(0x612c, 0);
	xenos_write32(0x6130, 0);
	xenos_write32(0x6434, mode->width);
	xenos_write32(0x6138, mode->height * interlace_factor);
	xenos_write32(0x6044, 0);
	xenos_write32(0x6048, 0);
	xenos_write32(0x604c, 0);
	xenos_write32(0x6050, 0);
	xenos_write32(0x65a0, 0);
	xenos_write32(0x65b4, 0x01000000);
	xenos_write32(0x65c4, 0x01000000);
	xenos_write32(0x65b0, 0);
	xenos_write32(0x65c0, 0x01000000);
	xenos_write32(0x65b8, 0x00060000);
	xenos_write32(0x65c8, 0x00040000);
	xenos_write32(0x65dc, 0);
	xenos_write32(0x65cc, 0);
	
	xenos_write32(0x64C0, 0);
	xenos_write32(0x6488, 0);
	xenos_write32(0x6484, 0);
	xenos_write32(0x649C, 7);
	xenos_write32(0x64a0, 1);
	while (!(xenos_read32(0x64a0) & 2));
}

void xenos_set_mode(struct mode_s *mode)
{
	xenos_write32(0x7938, xenos_read32(0x7938) | 1);
	xenos_write32(0x06ac, 1);
	
	printf(" . ana disable\n");
	xenos_ana_preinit();
	xenos_write32(0x04a0, 0x100);
	printf(" . ana enable\n");
	xenos_init_ana_new(mode->ana);
//	xenos_init_ana();
	xenos_write32(0x7900, 1);

	printf(" . f1\n");
	xenos_set_mode_f1(mode);
	printf(" . f2\n");
	xenos_set_mode_f2(mode);

	xenos_write32(0x6028, 0x10001);
	
	if (!mode->composite_sync)
	{
		xenos_write32(0x6038, 0x04010040);
		xenos_write32(0x603c, 0);
		xenos_write32(0x6040, 0x04010040); 
	} else
	{
		xenos_write32(0x6038, 0x20000200);
		xenos_write32(0x603c, 0x20010200);
		xenos_write32(0x6040, 0x20010200);
	}
	xenos_write32(0x793c, 0);
	xenos_write32(0x7938, 0x700);

	
	xenos_write32(0x6024, 0x04010040);
	xenos_write32(0x6054, 0x00010002);
	xenos_write32(0x6058, 0xec02414a);

	xenos_write32(0x6060, 0x000000ec);
	xenos_write32(0x6064, 0x014a00ec);
	xenos_write32(0x6068, 0x00d4014a);
	xenos_write32(0x6110, 0x0f920000);


	if (!mode->rgb)
	{
		xenos_write32(0x6380, 0x00000001);
		xenos_write32(0x6384, 0x00702000);
		xenos_write32(0x6388, 0x87a26000);
		xenos_write32(0x638c, 0x87ede000);
		xenos_write32(0x6390, 0x02008000);
		xenos_write32(0x6394, 0x00418000);
		xenos_write32(0x6398, 0x0080a000);
		xenos_write32(0x639c, 0x00192000);
		xenos_write32(0x63a0, 0x00408000);
		xenos_write32(0x63a4, 0x87da4000);
		xenos_write32(0x63a8, 0x87b5e000);
		xenos_write32(0x63ac, 0x00702000);
		xenos_write32(0x63b0, 0x02008000);
		xenos_write32(0x63b4, 0x00000000);
	} else
	{
		xenos_write32(0x6380, 0x00000001);
		xenos_write32(0x6384, 0x00db4000);
		xenos_write32(0x6388, 0x00000000);
		xenos_write32(0x638c, 0x00000000);
		xenos_write32(0x6390, 0x00408000);
		xenos_write32(0x6394, 0x00000000);
		xenos_write32(0x6398, 0x00db4000);
		xenos_write32(0x639c, 0x00000000);
		xenos_write32(0x63a0, 0x00408000);
		xenos_write32(0x63a4, 0x00000000);
		xenos_write32(0x63a8, 0x00000000);
		xenos_write32(0x63ac, 0x00db4000);
		xenos_write32(0x63b0, 0x00408000);
		xenos_write32(0x63b4, 0x00000000);
	}
}

static uint32_t edram_read(int addr)
{
	uint32_t res;
	xenos_write32(0x3c44, addr);
	while (xenos_read32(0x3c4c));
	res = xenos_read32(0x3c48);
	while (xenos_read32(0x3c4c));
	xenos_write32(0x3c44, addr);
	while (xenos_read32(0x3c4c));
	res = xenos_read32(0x3c48);
	while (xenos_read32(0x3c4c));
	return res;
}

static void edram_write(int addr, unsigned int value)
{
	while (xenos_read32(0x3c4c));
	xenos_write32(0x3c40, addr);
	while (xenos_read32(0x3c4c));
	xenos_write32(0x3c48, value);
	while (xenos_read32(0x3c4c));

	xenos_write32(0x3c40, 0x1A);
	while (xenos_read32(0x3c4c));
	xenos_write32(0x3c40, value);
	while (xenos_read32(0x3c4c));
}

static void edram_rmw(int addr, int bit, int mask, int val)
{
	edram_write(addr, (edram_read(addr) & ~mask) | ((val << bit) & mask));
}

void edram_p3(int *res)
{
	int chip, phase;
	
	for (chip = 0; chip < 9; ++chip)
		res[chip] = 0;

	for (phase = 0; phase < 4; ++phase)
	{
		edram_write(0x5002, 0x15555 * phase);
		int v = edram_read(0x5005);
		for (chip = 0; chip < 9; ++chip)
		{
			int bit = (v >> chip) & 1;
			
			res[chip] |= bit << phase;
		}
	}
}

void edram_p4(int *res)
{
	int chip, phase;

	for (chip = 0; chip < 6; ++chip)
		res[chip] = 0;

	for (phase = 0; phase < 4; ++phase)
	{
		xenos_write32(0x3c5c, 0x555 * phase);
		uint32_t r = xenos_read32(0x3c68);

		for (chip = 0; chip < 6; ++chip)
		{
			int bit = (r >> chip) & 1;

			res[chip] |= bit << phase;
		}
	}
}

int edram_p2(int r3, int r4, int r5, int r6, int r7, int r8, int r9, int r10)
{
	int a = edram_read(0x5002);
	edram_write(0x5002, 0xa53ca53c);
	int b = edram_read(0x5002);
	if ((b & 0x1ffff) != 0xA53C)
		return -1;
	edram_write(0x5002, 0xfee1caa9);
	b = edram_read(0x5002);
	if ((b & 0x1ffff) != 0x1caa9)
		return -2;
	edram_write(0x5002, a);
	
	xenos_write32(0x3c54, 1);
	if (r10)
	{
		xenos_write32(0x3c54, xenos_read32(0x3c54) &~ (1<<9));
		xenos_write32(0x3c54, xenos_read32(0x3c54) &~ (1<<10));
		
//		assert(xenos_read32(0x3c54) == 0x1);
	} else
	{
		xenos_write32(0x3c54, xenos_read32(0x3c54) | (1<<9));
		xenos_write32(0x3c54, xenos_read32(0x3c54) | (1<<10));
	}
	while (xenos_read32(0x3c4c));
	
	edram_write(0x4000, 0xC0);
	xenos_write32(0x3c90, 0);
	xenos_write32(0x3c00, xenos_read32(0x3c00) | 0x800000);
//	assert(xenos_read32(0x3c00) == 0x8900000);
	xenos_write32(0x0214, xenos_read32(0x0214) | 0x80000000);
//	assert(xenos_read32(0x0214) == 0x8000001a);
	xenos_write32(0x3c00, xenos_read32(0x3c00) | 0x400000);
	xenos_write32(0x3c00, xenos_read32(0x3c00) &~0x400000);
	xenos_write32(0x3c00, xenos_read32(0x3c00) &~0x800000);
	xenos_write32(0x0214, xenos_read32(0x0214) &~0x80000000);
//	assert(xenos_read32(0x214) == 0x1a);
	edram_write(0xffff, 1);
	while (xenos_read32(0x3c4c));
	edram_write(0x5001, 7);
	int s = 7;
	
	xenos_write32(0x3c58, xenos_read32(0x3c58) | 4);
	xenos_write32(0x3c58, xenos_read32(0x3c58) &~3);
	
//	assert(xenos_read32(0x3c58) == 0x00000ff4);
	
	if (r8)
	{
		assert(0);
		while (xenos_read32(0x3c4c));
		s = edram_read(0x5003);
		s |= 0x15555;
		edram_write(0x5003, s);
		edram_write(0x5003, s &~ 0x15555);
	}
	if (r9)
	{
		assert(0);
		while (xenos_read32(0x3c4c));
		int v = xenos_read32(0x3c60) | 0x555;
		xenos_write32(0x3c60, v);
		xenos_write32(0x3c60, v &~0x555);
	}
	if (r8)
	{
		assert(0);
		xenos_write32(0x3c54, xenos_read32(0x3c54) &~2);
		xenos_write32(0x3c90, 0x2aaaa);
		xenos_write32(0x3c6c, 0x30);
		xenos_write32(0x3c70, 0x30);
		xenos_write32(0x3c74, 0x30);
		xenos_write32(0x3c78, 0x30);
		xenos_write32(0x3c7c, 0x30);
		xenos_write32(0x3c80, 0x30);
		xenos_write32(0x3c84, 0x30);
		xenos_write32(0x3c88, 0x30);
		xenos_write32(0x3c8c, 0x30);
		xenos_write32(0x3c6c, xenos_read32(0x3c6c) &~ 0xFF);
		xenos_write32(0x3c70, xenos_read32(0x3c70) &~ 0xFF);
		xenos_write32(0x3c74, xenos_read32(0x3c74) &~ 0xFF);
		xenos_write32(0x3c78, xenos_read32(0x3c78) &~ 0xFF);
		xenos_write32(0x3c7c, xenos_read32(0x3c7c) &~ 0xFF);
		xenos_write32(0x3c80, xenos_read32(0x3c80) &~ 0xFF);
		xenos_write32(0x3c84, xenos_read32(0x3c84) &~ 0xFF);
		xenos_write32(0x3c88, xenos_read32(0x3c88) &~ 0xFF);
		xenos_write32(0x3c8c, xenos_read32(0x3c8c) &~ 0xFF);
	} else
	{
		xenos_write32(0x3c54, xenos_read32(0x3c54) | 2);
//		assert(xenos_read32(0x3c54) == 3);
	}
	if (r9)
	{
		assert(0);
		edram_rmw(0x5000, 1, 2, 0);
		edram_write(0x500c, 0x3faaa);
		edram_write(0x5006, 0x30);
		edram_write(0x5007, 0x30);
		edram_write(0x5008, 0x30);
		edram_write(0x5009, 0x30);
		edram_write(0x500a, 0x30);
		edram_write(0x500b, 0x30);
		edram_rmw(0x5006, 7, 0x3f80, 0);
		edram_rmw(0x5007, 7, 0x3f80, 0);
		edram_rmw(0x5008, 7, 0x3f80, 0);
		edram_rmw(0x5009, 7, 0x3f80, 0);
		edram_rmw(0x500a, 7, 0x3f80, 0);
		edram_rmw(0x500b, 7, 0x3f80, 0);

		edram_rmw(0x5006, 14, 0x1fc000, 0);
		edram_rmw(0x5007, 14, 0x1fc000, 0);
		edram_rmw(0x5008, 14, 0x1fc000, 0);
		edram_rmw(0x5009, 14, 0x1fc000, 0);
		edram_rmw(0x500a, 14, 0x1fc000, 0);
		edram_rmw(0x500b, 14, 0x1fc000, 0);
		
		edram_write(0x500c, 0x3f000);
	} else
	{
		edram_rmw(0x5000, 1, 2, 1);
	}
	if (r8)
	{
		/* later */
	}
		// bb584
	xenos_write32(0x3c54, xenos_read32(0x3c54) | 0x20);
	xenos_write32(0x3c54, xenos_read32(0x3c54) &~0x20);
	int i, j, k;
	
	int res_cur[9], temp[9], res_base[9];
	int valid[36]                 = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	int useful[64]                = {0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0};
	int phase_count[9]            = {4,4,4,4,4,4,4,4,4};
	int stable[9]                 = {1,1,1,1,1,1,1,1,1};
	int defaults[4]               = {11,7,15,13}; // 1011 0111 1111 1101
	int data6_old[16]             = {1,3,0,3,1,1,0,0,2,2,1,3,1,2,1,1};
	int data6_new[16]             = {0,2,3,2,0,0,3,3,1,1,0,2,0,1,0,0};
	int data7[16]                 = {0,2,3,2,0,0,3,3,1,1,0,2,0,1,0,0};

	int chip, phase;
	int res = 0;
#if 0
	for (i = 0; i < 50; ++i) /* outer */
	{
		edram_p3(temp);
#if 0
		temp[0] = 0x3;
		temp[1] = 0xd;
		temp[2] = 0x6;
		temp[3] = 0x1;
		temp[4] = 0x3;
		temp[5] = 0x1;
		temp[6] = 0xd;
		temp[7] = 0x2;
		temp[8] = 0x2;
#endif
		
		memcpy(res_cur, temp, sizeof(res_cur));
		if (!i)
			memcpy(res_base, temp, sizeof(res_cur));

		for (chip = 0; chip < 9; ++chip) /* inner */
		{
			for (phase = 0; phase < 4; phase++) /* phase */
			{
				if (valid[chip + phase * 9])
					if (!useful[res_cur[chip] + phase * 4])
					{
						valid[chip + phase * 9] = 0;
						phase_count[chip]--;
					}
			}
			if (res_base[chip] != res_cur[chip])
				stable[chip] = 0;
		}
	}
#if 1
	for (chip = 0; chip < 9; ++chip)
	{
		if (!stable[chip] && phase_count[chip] == 1)
		{
			for (phase = 0; phase < 4; ++phase)
			{
				if (valid[chip + phase * 9])
				{
					res_cur[chip] = defaults[phase];
				}
			}
		}
	}
#endif
		// bb734

	int edram_rev = 0x2;
	int *data6;
	data6 = (edram_rev > 0x10) ? data6_new : data6_old;
	
	for (chip = 0; chip < 9; ++chip)
	{
		printf("%d  | ", data6[res_cur[chip]]);
		res |= data6[res_cur[chip]] << (chip * 2);
	}
	printf("final cfg: %08x\n", res);
//	assert(res == 0x2fcb);
	edram_write(0x5002, res);
#endif

	int cur[9], rp[9];
	memset(rp, 0xff, sizeof rp);
	for (i = 0; i < 10; ++i)
	{
		edram_p3(cur);
		int j;
		for (j = 0; j < 9; ++j)
			rp[j] &= cur[j];
	}
	
	res = 0;
	
	for (i = 0; i < 9; ++i)
	{
		for (j = 0; j < 4; ++j)
			if (rp[i] & (1<<j))
				break;
		if (j == 4)
			printf("ed chip %d, inv\n", i);
		j--; j&=3;
		res |= j << (i * 2);
	}

	edram_write(0x5002, res);

	xenos_write32(0x3c54, xenos_read32(0x3c54) | 0x20);
	xenos_write32(0x3c54, xenos_read32(0x3c54) &~0x20);
	
	edram_rmw(0x5000, 2, 4, 1);
	edram_rmw(0x5000, 2, 4, 0);
	edram_rmw(0x5000, 3, 8, 1);
	edram_rmw(0x5000, 3, 8, 0);

	xenos_write32(0x3c58, xenos_read32(0x3c58) | 4);
	xenos_write32(0x3c58, xenos_read32(0x3c58) | 3);

	s = 4;
	edram_write(0x5001, s);
	edram_rmw(0x5000, 0, 1, 1);
	if (r8)
		xenos_write32(0x3c54, xenos_read32(0x3c54) &~2);
	else
		xenos_write32(0x3c54, xenos_read32(0x3c54) | 2);
	edram_rmw(0x5000, 1, 2, r9 ? 0 : 1);
	if (r9)
	{
		/* not yet */
	}
		/* bbf38 */
	edram_rmw(0x5000, 5, 0x20, 1);
	edram_rmw(0x5000, 5, 0x20, 0);
	for (i = 0; i < 9; ++i)
		phase_count[i] = 4;
	for (i = 0; i < 9; ++i)
		stable[i] = 1;
	for (i = 0; i < 9; ++i)
		valid[i] = valid[i + 9] = valid[i + 18] = valid[i + 27] = 1;

#if 0	
	// bbfd4
	int ht[6];
	for (i = 0; i < 50; ++i)
	{
		edram_p4(ht);
#if 0
		ht[0] = 3;
		ht[1] = 3;
		ht[2] = 6;  // 01 10 01 10 10 10
		ht[3] = 3;  // 01 11 10 10 11 10
		ht[4] = 0xc;
		ht[5] = 6;
#endif
		memcpy(res_cur, ht, sizeof(ht));
		if (!i)
			memcpy(res_base, ht, sizeof(ht));
		for (chip = 0; chip < 6; ++chip)
		{
			for (phase = 0; phase < 4; ++phase)
			{
				if (valid[chip + phase * 9] && !useful[res_cur[chip] + phase * 4])
				{
					valid[chip + phase * 9] = 0;
					phase_count[chip]--;
				}
			}
			if (res_cur[chip] != res_base[chip])
				stable[chip] = 0;
		}
	}
#if 1
	for (chip = 0; chip < 6; ++chip)
	{
		if (!stable[chip] && phase_count[chip] == 1)
		{
			for (phase = 0; phase < 4; ++phase)
				if (valid[chip + phase * 9])
					res_cur[chip] = defaults[phase];
		}
	}
#endif

	res = 0;
	for (chip = 0; chip < 6; ++chip)
		res |= data7[res_cur[chip]] << (chip*2);
	printf("final cfg: %08x\n", res);
//	assert(res == 0x7ae);
	xenos_write32(0x3c5c, res);
#endif

	memset(rp, 0xff, sizeof rp);
	for (i = 0; i < 10; ++i)
	{
		edram_p4(cur);
		int j;
		for (j = 0; j < 6; ++j)
			rp[j] &= cur[j];
	}
	
	res = 0;
	
	for (i = 0; i < 6; ++i)
	{
		for (j = 0; j < 4; ++j)
			if (rp[i] & (1<<j))
				break;
		if (j == 4)
			printf("gp chip %d, inv\n", i);
		j--; j&=3;
		res |= j << (i * 2);
	}
	xenos_write32(0x3c5c, res);

	edram_rmw(0x5000, 5, 0x20, 1);
	edram_rmw(0x5000, 5, 0x20, 0);
	xenos_write32(0x3c54, xenos_read32(0x3c54) | 4);
	xenos_write32(0x3c54, xenos_read32(0x3c54) &~4);
	if (r6 >= 0x10)
	{
		xenos_write32(0x3cb4, 0xbbbbbb);
	} else
	{
		assert(0);
		xenos_write32(0x3c54, xenos_read32(0x3c54) | 8);
		xenos_write32(0x3c54, xenos_read32(0x3c54) &~8);
	}
	
	xenos_write32(0x3c58, xenos_read32(0x3c58) | 3);
	xenos_write32(0x3c58, xenos_read32(0x3c58) | 4);
	edram_rmw(0x5001, 0, 3, 3);
	edram_rmw(0x5001, 2, 4, 1);

	edram_rmw(0x5001, 0, 3, 3);
	edram_rmw(0x5001, 2, 4, 1);
	xenos_write32(0x3c58, xenos_read32(0x3c58) &~3);
	xenos_write32(0x3c58, xenos_read32(0x3c58) | 4);

	if (r6 >= 0x10 && !r7)
	{
		xenos_write32(0x3c94, (xenos_read32(0x3c94) &~0xFF) | 0x13);
		xenos_write32(0x3c94, xenos_read32(0x3c94) &~0x80000000);
		xenos_write32(0x3c94, xenos_read32(0x3c94) | 0x80000000);
//		assert(xenos_read32(0x3c94) == 0x80000013);
	} else
	{
		edram_rmw(0x5000, 0, 1, 1);
		edram_rmw(0x5000, 8, 0x100, 1);

		xenos_write32(0x3c54, xenos_read32(0x3c54) | 1);
		xenos_write32(0x3c54, xenos_read32(0x3c54) | 0x100);
		
		int cnt = 20;
		while (cnt--)
		{
			udelay(5000);
			if (xenos_read32(0x3c94) & 0x80000000)
			{
				printf("ping test: %08lx\n", xenos_read32(0x3c94));
				break;
			}
		}
		
		if (!cnt)
		{
			edram_rmw(0x5000, 0, 1, 0);
			edram_rmw(0x5000, 8, 0x100, 0);
		
			xenos_write32(0x3c54, xenos_read32(0x3c54) &~1);
			xenos_write32(0x3c54, xenos_read32(0x3c54) &~0x100);

			printf("ping test timed out\n");
			return 1;
		}
		printf("ping test okay\n");
	}

	xenos_write32(0x3c54, xenos_read32(0x3c54) &~0x100);
	xenos_write32(0x3c54, xenos_read32(0x3c54) &~1);
//	assert(xenos_read32(0x3c54) == 2);
	
	edram_rmw(0x5000, 8, 0x100, 0);
	edram_rmw(0x5000, 0, 1, 0);
	edram_write(0xf, 1);
	edram_write(0x100f, 1);
	edram_write(0xf, 0);
	edram_write(0x100f, 0);
	edram_write(0xffff, 1);
	edram_rmw(0x5001, 2, 4, 0);

	xenos_write32(0x3c58, xenos_read32(0x3c58) &~4);
	xenos_write32(0x3c58, (xenos_read32(0x3c58) &~ 0xF0) | 0xA0);
	xenos_write32(0x3c58, xenos_read32(0x3c58) &~ 0xF00);
	
	assert(xenos_read32(0x3c58) == 0xa0);

	xenos_write32(0x3c00, (xenos_read32(0x3c00) &~ 0xFF) | (9<<3));
	xenos_write32(0x3c00, xenos_read32(0x3c00) | 0x100);
	//assert((xenos_read32(0x3c00) &~0x600) == 0x08100148);
	
	while ((xenos_read32(0x3c00) & 0x600) < 0x400);

	return 0;
}

void edram_pc(void)
{
	int i;
	
	for (i = 0; i < 6; ++i)
	{
		xenos_write32(0x3c44, 0x41);
		while (xenos_read32(0x3c4c) & 0x80000000);
	}

	for (i = 0; i < 6; ++i)
	{
		xenos_write32(0x3c44, 0x1041);
		while (xenos_read32(0x3c4c) & 0x80000000);
	}

	for (i = 0; i < 6; ++i)
		xenos_read32(0x3ca4);

	for (i = 0; i < 6; ++i)
		xenos_read32(0x3ca8);
}

int edram_compare_crc(uint32_t *crc)
{
	int i;
	
	int fail = 0;
	
	for (i = 0; i < 6; ++i)
	{
		xenos_write32(0x3c44, 0x41);
		while (xenos_read32(0x3c4c) & 0x80000000);
		if (xenos_read32(0x3c48) != *crc++)
			fail |= 1<<6;
//		printf("%08x ", xenos_read32(0x3c48));
	}

	for (i = 0; i < 6; ++i)
	{
		xenos_write32(0x3c44, 0x1041);
		while (xenos_read32(0x3c4c) & 0x80000000);
		if (xenos_read32(0x3c48) != *crc++)
			fail |= 1<<(i+6);
//		printf(" %08x", xenos_read32(0x3c48));
	}
//	printf("\n");

	for (i = 0; i < 6; ++i)
	{
		uint32_t v = xenos_read32(0x3ca4);
		if (v != *crc++)
			fail |= 1<<(i+12);
	}

	for (i = 0; i < 6; ++i)
	{
		uint32_t v = xenos_read32(0x3ca8);
		if (v != *crc++)
			fail |= 1<<(i+18);
	}
	return fail;
}

void xenos_edram_init(void)
{
	int gputype = 0; // pre-jasper
	xenos_write32(0x214, gputype ? 0x1e : 0x1a);
	xenos_write32(0x3C00, (xenos_read32(0x3c00) &~ 0x003f0000) | 0x100000);
	int v = (edram_read(0x4000) &~ 4) | 0x2A;
	edram_write(0x4000, v);
	edram_write(0x4001, gputype ? 0x31: 0x2709f1);
	v = (v &~ 0x20) | 0xC;
	edram_write(0x4000, v);
	v &= ~0xC;
	edram_write(0x4000, v);
	edram_write(0xFFFF, 1);
	v |= 4;
	edram_write(0x4000, v);
	edram_write(0x4001, gputype ? 0x31 : 0x2709f1);
	v &= ~0x4C;
	edram_write(0x4000, v);
	edram_write(0xFFFF, 1);
	udelay(1000);
	int id = edram_read(0x2000);
//	printf("EDRAM CHIP ID=%08x\n", id);
	
	if (edram_p2(0, 0, 0, 0x11, 0, 0, 0, 1))
	{
		printf("edram_p2 failed\n");
		abort();
	}
}

void xenos_autoset_mode(void)
{
	int mode = MODE_PAL60;
	int avpack = xenon_smc_read_avpack();
	printf("AVPACK detected: %02x\n", avpack);
	switch (avpack&0xF)
	{
	case 0x3: // normal (composite)
		mode = MODE_PAL60;
		break;
	case 0xF: // HDTV
		mode = MODE_PAL60; // is fine for that, too.
		break;
	case 0xb: // VGA
		mode = MODE_VGA_1024x768;
		break;
	case 0xC: // no AV pack, revert to composite.
		mode = MODE_PAL60;
		break;
	default:
		printf("unsupported AVPACK!\n");
		{
			int table[] = {0xf, 0x1, 0x3, 0x7};
			xenon_smc_set_led(1, 0xff);
			mdelay(1000);
			xenon_smc_set_led(1, table[(avpack>>6) & 3]);
			mdelay(1000);
			xenon_smc_set_led(1, table[(avpack>>4) & 3]);
			mdelay(1000);
			xenon_smc_set_led(1, table[(avpack>>2) & 3]);
			mdelay(1000);
			xenon_smc_set_led(1, table[(avpack>>0) & 3]);
			mdelay(1000);
			xenon_smc_set_led(1, 0xff);
			mdelay(1000);
			xenon_smc_set_led(0, 0);
		}
		break;
	}
	xenos_set_mode(&xenos_modes[mode]);
}

void xenos_init(void)
{
	xenos_init_phase0();
	xenos_init_phase1();
	
	xenon_gpio_set(0, 0x2300);
	xenon_gpio_set_oe(0, 0x2300);

	xenos_autoset_mode();

	xenon_smc_ana_write(0xdf, 0);
	xenos_write32(0x652c, 0x00000300);
}
