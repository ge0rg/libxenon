#!/bin/python
import string, glob

BASEDIR = "data/"

gfilelist = ["*su"]
	#["color.psu", "color.vsu", "texture.psu", "texture.vsu", "shaders/bm_*.vsu", "shaders/bm_*.psu", "test.kx", "shaders/gen/*su"]

filelist = []
for f in gfilelist:
	filelist += glob.glob(BASEDIR + f)

files = {}
for f in filelist:
	files[f] = open(f, "rb").read()

used = set()

def filter_fn(f):
	global used
	res = ""
	for c in f:
		if c in string.letters:
			res += c
	ores = res
	cnt =0
	while res in used:
		res = ores + "_%d" % cnt
		cnt += 1
	used.add(res)
	return res

flookup = {}

for f in filelist:
	cfn = filter_fn(f)
	flookup[f] = cfn
	print "static unsigned char content_%s[] = {" % cfn
	i = 0
	for c in files[f]:
		print "0x%02x," % ord(c),
		
		if i & 15 == 15:
			print
		
		i += 1
	print "};"

print """

struct ffs_s
{
	const char *filename;
	int size;
	void *content;
} ffs_files[] = {"""

for f in files:
	print '	{"%s", %d, content_%s},' % (f[len(BASEDIR):], len(files[f]), flookup[f])

print '	{0, 0, 0},'

print "};"
