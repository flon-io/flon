
#
# testing `out: stderr`
#
# Wed Mar  4 13:25:22 JST 2015
#


require 'json'

task = JSON.parse(STDIN.read)
task['hello'] = 'outerr'

STDERR.puts("[1;30m");

puts(JSON.dump(task))

STDERR.puts 'outerr over.'

STDERR.puts("[0;0m");

