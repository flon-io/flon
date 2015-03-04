
# r.rb
#
# Wed Mar  4 16:33:23 JST 2015

require 'json'

# the dispatcher expects "point", "task.state" and "payload", else
# it considers the incoming JSON as "task.payload"

task = {}
task['point'] = 'task';
task['task'] = {}
task['task']['state'] = 'created'
task['task']['event'] = 'refusal'
task['task']['from'] = 'refuser/r.rb'
task['payload'] = JSON.parse(STDIN.read)

puts JSON.dump(task)

