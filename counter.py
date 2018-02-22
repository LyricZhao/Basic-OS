import os

okp = ["c", "nas", "h"]

def check(file):
	s = file.split('.')
	if len(s) == 1 or s[1] in okp:
		return True
	return False

def counter(path):
	files = os.listdir(path)

	tot_line = 0
	tot_size = 0

	for file in files:
		if not check(file):
			continue
		if not os.path.isdir(path + "/" + file):
			f = open(path + "/" + file)
			iter_f = iter(f)
			for line in iter_f:
				tot_line += 1
				tot_size += len(line)
		else:
			a, b = counter(path + "/" + file)
			tot_line += a
			tot_size += b
	return tot_line, tot_size

sub_path = raw_input()
path = "./" + sub_path
ra, rb = counter(path)
ua, ub = counter("./app")
ra += ua
rb += ub
print "total line: %d\ntotal size: %d Bytes (%.3f KB)" % (ra, rb, rb / 1024.0)
