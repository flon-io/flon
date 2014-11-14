
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

Date.prototype.to_s = function()
{
  return '' +
    this.getFullYear() + '-' + this.getMonth() + '-' + this.getDate() + ' ' +
    this.getHours() + ':' + this.getMinutes() + ':' + this.getSeconds();
}

