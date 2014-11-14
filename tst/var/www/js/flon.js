
/* flon.js is MIT licensed */

function byid(id)
{
  return document.getElementById(id);
}

function byclass(cl)
{
  var a = [];
  var elts = document.getElementsByClassName(cl);
  for (var i = 0; ; i++) { var e = elts[i]; if ( ! e) break; a.push(e); }

  return a;
}

function tstamp()
{
  var d = new Date();

  var f = function(i) { return (i < 10 ? '0' : '') + i; }

  return '' +
    d.getFullYear() + d.getMonth() + f(d.getDate()) + '.' +
    f(d.getHours()) + f(d.getMinutes()) + f(d.getSeconds());
}

