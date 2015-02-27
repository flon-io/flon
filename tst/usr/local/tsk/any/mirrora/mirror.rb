
#
# a mirroring device
#
# Sat Feb 21 06:29:57 JST 2015
#


require 'json'

task = JSON.parse(STDIN.read)

task['tstate'] = 'completed'
  # very important, else flon will think the task is still "created"
  # and hand it back to this tasker in a loop...

STDOUT.puts(JSON.dump(task))

