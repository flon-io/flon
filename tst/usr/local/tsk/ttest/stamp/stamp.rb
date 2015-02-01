
#
# a flon stamping service
#
# Sun Feb  1 17:48:29 JST 2015
#

require 'json'


task = JSON.parse(STDIN.read)

task['stamp'] = 'ttest stamp'

STDOUT.puts(JSON.dump(task))

#t = Time.now
#STDERR.puts("#{t} .#{t.usec} xxx #{Process.pid} stamp.rb over.")

