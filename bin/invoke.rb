
t = Time.now.strftime("%Y%m%d.%H%M.kurukuku");
id = "invoke.rb-u0-#{t}-0"

f = File.open("tst/var/spool/dis/inv_#{id}.json", 'w')
f.write(%[
{
  invoke: [ invoke, { _0: stamp, color: blue }, [] ]
  exid: #{id}
  nid: \"0\"
  payload: {
    hello: world
  }
}
])
f.close

