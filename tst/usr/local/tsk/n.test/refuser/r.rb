
# r.rb
#
# Wed Mar  4 16:33:23 JST 2015

require 'json'

task = JSON.parse(STDIN.read)
task['task']['state'] = 'created'
task['task']['event'] = 'refusal'
task['task']['from'] = 'refuser/r.rb'

puts JSON.dump(task)

