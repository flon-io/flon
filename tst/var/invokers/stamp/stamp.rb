
#
# a flon stamping service
#
# Sat Oct  4 07:38:44 JST 2014
#

#require 'pp'
require 'json'

task = JSON.parse(STDIN.read)

task["stamp"] = Time.now.to_s

puts(JSON.dump(task))

