
/* flon.js is MIT licensed */

function byid(id)
{
  return document.getElementById(id);
}

function toarray(htmlcol)
{
  var a = [];
  for (var i = 0; ; i++) { var e = htmlcol[i]; if ( ! e) break; a.push(e); }

  return a;
}

function byx(func, args)
{
  var from = document; var x = args[0];
  if (args.length > 1) { from = x; x = args[1]; }

  return toarray(from[func](x));
}
function byclass() { return byx('getElementsByClassName', arguments); }
function bytag() { return byx('getElementsByTagName', arguments); }

function tstamp()
{
  var d = new Date();

  var f = function(i) { return (i < 10 ? '0' : '') + i; }

  return '' +
    d.getFullYear() + d.getMonth() + f(d.getDate()) + '.' +
    f(d.getHours()) + f(d.getMinutes()) + f(d.getSeconds());
}

