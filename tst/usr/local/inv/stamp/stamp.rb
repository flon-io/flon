
#
# a flon stamping service
#
# Sat Oct  4 07:38:44 JST 2014
#

#require 'pp'
require 'json'

task = JSON.parse(STDIN.read)

task['stamp'] = Time.now.to_s

STDOUT.puts(JSON.dump(task))

t = Time.now
STDERR.puts("#{t} .#{t.usec} xxx #{Process.pid} stamp.rb over.")

