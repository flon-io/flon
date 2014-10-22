
t = Time.now.strftime("%Y%m%d.%H%M.babiberu");
id = "launch.rb-u0-#{t}"

f = File.open("tst/var/spool/dis/exe_#{id}.json", 'w')
f.write(%[
execute:
  [ sequence, {}, [
    [ invoke, { _0: stamp, color: blue }, [] ]
    [ invoke, { _0: stamp, color: green }, [] ]
  ] ]
exid: #{id}
payload: {
  hello: world
}
])
f.close

