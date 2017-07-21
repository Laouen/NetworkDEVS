#!/usr/bin/python
import sys
import gflags  # sudo pip install python-gflags

gflags.DEFINE_string('log', None,
					 'The path of the pdev.log file',
					 short_name='l')

gflags.MarkFlagAsRequired('log')

gflags.DEFINE_string('destination_folder', None,
					 'The path of the destination folder where to save results',
					 short_name='f')

gflags.MarkFlagAsRequired('log')
gflags.MarkFlagAsRequired('destination_folder')

FLAGS = gflags.FLAGS


try:
	argv = FLAGS(sys.argv)
except gflags.FlagsError as e:
	print '%s\\nUsage: %s ARGS\\n%s' % (e, sys.argv[0], FLAGS)
	sys.exit(1)

labels = {
	'DNS': [],
	'DDemux': [],
	'Demux': [],
	'Multiplexer': [],
	'IPv4': [],
	'Link': [],
	'Switch': [],
	'UDP': [],
	'SWP': []
}

last_label = ''
with open(FLAGS.log) as f:
	content = f.readlines()
	for line in content:
		is_label = False
		for label, logs in labels.iteritems():
			if label in line:
				last_label = label
				is_label = True
				logs.append(line)

		if not is_label:
			labels[last_label].append(line)

for label, logs in labels.iteritems():
	with open(FLAGS.destination_folder + "/" + label + ".log", 'w') as file:
		file.writelines(logs)