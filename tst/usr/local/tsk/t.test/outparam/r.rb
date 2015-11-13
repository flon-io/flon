
#
# testing `out: param`
#
# Wed Mar  4 06:53:09 JST 2015
#


require 'json'

task = JSON.parse(STDIN.read)
task['hello'] = 'outparam'
task['tasker_out'] = ARGV[0]

#puts
#puts "pwd: " + `pwd`
#puts "ARGV[0]: #{ARGV[0].inspect}"

f = File.open(ARGV[0], 'wb')

f.puts(JSON.dump(task))

f.close

