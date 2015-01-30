
#
# a flon test invoker
#
# Thu Nov 27 16:07:39 JST 2014
#

require 'json'

task = JSON.parse(STDIN.read)

task['args1'] = task['args']

STDOUT.puts(JSON.dump(task))

