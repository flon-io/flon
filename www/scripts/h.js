/*
 * Copyright (c) 2015-2015, John Mettraux, jmettraux@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

var H = (function() {

  var self = this;

  var toElement = function(start, elt_or_selector) {

    if ( ! elt_or_selector) { elt_or_selector = start; start = document; }
    if ( ! start) { start = document; }

    if ((typeof elt_or_selector) === 'string')
      return start.querySelector(elt_or_selector);
    return elt_or_selector;
  };

  var toElements = function(start, elt_or_selector) {

    if ( ! elt_or_selector) { elt_or_selector = start; start = document; }
    if ( ! start) { start = document; }

    var r =
      (typeof elt_or_selector) === 'string' ?
      start.querySelectorAll(elt_or_selector) :
      [ elt_or_selector ];

    if ( ! r.forEach) {
      r.forEach = function(func) { [].forEach.call(this, func); };
    }

    return r;
  };

  this.elt = function(start, elt_or_selector) {

    return toElement(start, elt_or_selector);
  };

  this.elts = function(start, selector) {

    return toElements(start, selector);
  }

  this.forEach = function(start, selector, func) {

    if ((typeof selector) === 'function') { func = selector; selector = null; }

    var r = toElements(start, selector);
    r.forEach(func);

    return r;
  };

  this.dim = function(start, elt_or_selector) {

    var e = toElement(start, elt_or_selector);

    if ( ! e) return null;

    return {
      top: e.offsetTop,
      bottom: e.offsetTop + e.offsetHeight,
      left: e.offsetLeft,
      right: e.offsetLeft + e.offsetWidth,
      height: e.offsetHeight,
      width: e.offsetWidth,
    }
  }

  var onOrOff = function(dir, start, elt_or_selector, eventName, eventHandler) {

    if ( ! eventHandler) {
      eventHandler = eventName;
      eventName = elt_or_selector;
      elt_or_selector = start;
      start = document;
    }

    var es = toElements(start, elt_or_selector);
    for (var i = 0; ; i++) {
      var e = es[i]; if ( ! e) break;
      if (dir === 'on') e.addEventListener(eventName, eventHandler);
      else /* off */ e.removeEventListener(eventName, eventHandler);
    }
  };

  this.on = function(start, elt_or_selector, eventName, eventHandler) {
    onOrOff('on', start, elt_or_selector, eventName, eventHandler);
  };

  this.off = function(start, elt_or_selector, eventName, eventHandler) {
    onOrOff('off', start, elt_or_selector, eventName, eventHandler);
  };

  var indexNext = function(sel) {

    var d = sel.indexOf('.'); var s = sel.indexOf('#');
    if (d < 0) return s; if (s < 0) return d;
    return d < s ? d : s;
  };

  this.create = function(sel, atts, text) {

    sel = '%' + sel;
    if ((typeof atts) === 'string') { text = atts; atts = {}; }

    var e = document.createElement('div');

    for (var i = 0; i < sel.length; ) {
      var t = sel.substring(i, i + 1);
      var j = indexNext(sel.substring(i + 1));
      var s = j > -1 ? sel.substring(i + 1, i + 1 + j) : sel.substring(i + 1);
      //console.log([ t, j, s ]);
      if (t === '%') e = document.createElement(s);
      else if (t === '#') e.id = s;
      else if (t === '.') e.classList.add(s);
      if (j < 0) break;
      i = i + 1 + j;
    }

    for (var k in atts) e.setAttribute(k, atts[k]);

    if (text) e.appendChild(document.createTextNode(text));

    return e;
  };

  this.toNode = function(html) {

    var e = document.createElement('div');
    e.innerHTML = html; // :-(

    return e.children[0];
  };

  var defaultOn = function(type, method, uri) {

    return function(res) {
      if (type === 'load') console.log([ method + ' ' + uri, res ]);
      else console.log([ meth + ' ' + uri + ' connection problem', res ]);
    }
  };

  this.request = function(method, uri, data, callbacks) {

    if ( ! callbacks) { callbacks = data; data = null; }
    if ((typeof callbacks) === 'function') callbacks = { onok: callbacks };

    var r = new XMLHttpRequest();
    r.open(method, uri, true);

    if (data) {
      if (data.constructor.toString().match(/FormData/)) {
        //r.setRequestHeader('Content-Type', 'application/form-data');
      }
      else {
        r.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
        data = (typeof data) === 'string' ? data : JSON.stringify(data);
      }
    }

    r.onload = function() {
      var o = { status: r.status, request: r };
      o.data = null; try { o.data = JSON.parse(r.responseText); } catch (ex) {};
      if (callbacks.onok && r.status === 200)
        callbacks.onok(o);
      else
        (callbacks.onload || defaultOn('load', method, uri))(o);
    };
    r.onerror = callbacks.onerror || defaultOn('error', method, uri);

    r.send(data);
  };

  this.upload = function(uri, inputFileElt_s, data, callbacks) {

    //console.log('uploading to ' + uri + ' ...');

    if ( ! callbacks) { callbacks = data; data = {}; }

    var fd = new FormData();

    for (var k in data) fd.append(k, data[k]);

    var isMulti = Array.isArray(inputFileElt_s);
    var elts = isMulti ? inputFileElt_s : [ inputFileElt_s ];

    var fcount = 0;

    elts.forEach(function(elt) {

      var files = elt.files;

      for (var i = 0, l = files.length; i < l; i++) {

        fcount = fcount + 1;

        var f = files[i];

        var l = elt.getAttribute('data-sg-lang');
        var k = 'file-';
        if (l || isMulti) k = k + elt.name + '-';
        if (l) k = k + l + '-';
        k = k + i;

        fd.append(k, f, f.name);
      }
    });

    if (fcount < 1) return 0;

    var onok = callbacks.onok;
    callbacks.onok = function(res) {
      elts.forEach(function(elt) { elt.value = ''; });
      onok(res);
    };

    H.request('POST', uri, fd, callbacks);

    return fcount;
  };

  this.matches = function(elt, sel) {

    return elt.msMatchesSelector ?
      elt.msMatchesSelector(sel) : elt.matches(sel);
  };

  this.closest = function(start, elt_or_selector, sel) {

    if ( ! sel) {
      sel = elt_or_selector; elt_or_selector = start; start = null;
    }
    var elt = toElement(start, elt_or_selector);

    if (H.matches(elt, sel)) return elt;

    return elt.parentElement ? H.closest(elt.parentElement, sel) : null;
  };

  // adapted from http://upshots.org/javascript/jquery-copy-style-copycss
  //
  this.style = function(start, elt_or_selector) {

    var elt = toElement(start, elt_or_selector);

    var r = {};
    var style = null;

    if (window.getComputedStyle) {

      style = window.getComputedStyle(elt, null);

      for (var i = 0, l = style.length; i < l; i++) {
        var p = style[i];
        var n = p.replace(
          /-([a-za])/g, function(a, b) { return b.toUpperCase(); })
        r[n] = style.getPropertyValue(p);
      }

      return r;
    }

    if (style = elt.currentStyle) {

      for (var p in style) r[p] = style[p];
      return r;
    }

    if (style = elt.style) {

      for (var p in style) {
        var s = style[p];
        if ((typeof s) !== 'function') r[p] = s;
      }

      //return r;
    }

    return r;
  };

  this.hasClass = function(start, elt_or_selector, className) {

    if ( ! className) {
      className = elt_or_selector; elt_or_selector = start; start = null;
    }

    var elt = toElement(start, elt_or_selector);
    if (className[0] === '.') className = className.substring(1);

    try {
      if (elt.classList) return elt.classList.contains(className);
      return (new RegExp('\\b' + className + '\\b')).test(elt.className);
    }
    catch (ex) {
      return false;
    }
  };

  this.removeClass = function(start, elt_or_selector, className) {

    if ( ! className) {
      className = elt_or_selector; elt_or_selector = start; start = null;
    }

    if (className[0] === '.') className = className.substring(1);

    toElements(start, elt_or_selector).forEach(function(e) {
      e.classList.remove(className);
    });
  }

  this.addClass = function(start, elt_or_selector, className) {

    if ( ! className) {
      className = elt_or_selector; elt_or_selector = start; start = null;
    }

    if (className[0] === '.') className = className.substring(1);

    toElements(start, elt_or_selector).forEach(function(e) {
      e.classList.add(className);
    });
  }

  var tog = function(start, elt_or_selector, bool, klass) {

    if (
      (typeof elt_or_selector) === 'boolean' || elt_or_selector === 'toggle'
    ) {
      bool = elt_or_selector; elt_or_selector = start; start = null;
    }

    if (bool === 'toggle') {
      bool = ! self.hasClass(start, elt_or_selector, klass);
    }

    toElements(start, elt_or_selector).forEach(function(e) {
      self[bool ? 'addClass' : 'removeClass'](e, klass);
      return bool;
    });
  };

  this.toggleClass = function(start, elt_or_selector, className) {

    if ( ! className) {
      className = elt_or_selector; elt_or_selector = start; start = null;
    }

    return tog(start, elt_or_selector, 'toggle', className);
  };
  this.toggle = this.toggleClass;

  this.show = function(start, elt_or_selector, bool) {

    tog(start, elt_or_selector, bool, '.show');
  };

  this.hide = function(start, elt_or_selector, bool) {

    tog(start, elt_or_selector, bool, '.hidden');
  };

  this.enable = function(start, elt_or_selector, bool) {

    if (bool === undefined) {
      bool = elt_or_selector; elt_or_selector = start; start = null;
    }

    toElements(start, elt_or_selector).forEach(function(e) {
      if (bool === false || bool === null)
        e.setAttribute('disabled', 'disabled');
      else
        e.removeAttribute('disabled');
    });
  }

  this.disable = function(start, elt_or_selector) {

    self.enable(start, elt_or_selector, false);
  };

  this.cdisable = function(start, elt_or_selector) {

    H.addClass(start, elt_or_selector, '.disabled');
  };

  this.cenable = function(start, elt_or_selector) {

    H.removeClass(start, elt_or_selector, '.disabled');
  };

  this.toCamelCase = function(s) {

    return s.replace(
      /(-[a-z])/g, function(x) { return x.substring(1).toUpperCase(); });
  };

  this.prepend = function(start, elt_or_selector, child) {

    if ( ! child) {
      child = elt_or_selector; elt_or_selector = start; start = null;
    }

    var elt = toElement(start, elt_or_selector);

    elt.parentNode.insertBefore(child, elt);
  };

  this.trigger = function(start, elt_or_selector, eventName) {

    if ( ! eventName) {
      eventName = elt_or_selector; elt_or_selector = start; start = null;
    }

    var elt = toElement(start, elt_or_selector);

    var ev = document.createEvent('HTMLEvents');
    ev.initEvent(eventName, true, false);

    elt.dispatchEvent(ev);
  };

  this.clean = function(start, elt_or_selector, className) {

    var elt = toElement(start, elt_or_selector);
    if (className) H.forEach(elt, className, function(e) { e.remove(); });
    else while (elt.firstChild) elt.removeChild(elt.firstChild);

    return elt;
  };

  this.onDocumentReady = function(f) {

    if (document.readyState != 'loading') f();
    else document.addEventListener('DOMContentLoaded', f);
  };

  //
  // over.

  return this;

}).apply({});

