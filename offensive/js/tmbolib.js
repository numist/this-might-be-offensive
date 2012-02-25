String.prototype.htmlescape = function () {
	return(
		this.replace(/&/g,'&amp;').
		     replace(/>/g,'&gt;').
		     replace(/</g,'&lt;').
		     replace(/"/g,'&quot;')
	);
};

if (typeof String.prototype.startsWith != 'function') {
  String.prototype.startsWith = function (str){
    return this.indexOf(str) == 0;
  };
}

if (typeof String.prototype.endsWith != 'function') {
  String.prototype.endsWith = function (str) {
    if(this.length < str.length) {
      // if this.length - str.length == -1, this could cause a false positive
      return false;
    }

    return this.lastIndexOf(str) == this.length - str.length;
  }
}

if (typeof String.prototype.parseInt != 'function') {
  String.prototype.parseInt = function () {
    return parseInt(this);
  }
}

(function($) {
	$.fn.hasAttr = function(name) {  
	  // fucking browsers.
	  return this.attr(name) !== undefined && this.attr(name) !== false;
	};
  $.fn.setCaretPosition = function(pos) {
		if(this.length == 0) return this;
		var self = this.get(0);
    if (self.setSelectionRange) {
      self.setSelectionRange(pos, pos);
    } else if (self.createTextRange) {
      var range = self.createTextRange();
      range.collapse(true);
      range.moveEnd('character', pos);
      range.moveStart('character', pos);
      range.select();
    }
		return this;
  };
	$.fn.getCaretPosition = function() {
		if(this.length == 0) return undefined;
		if(this.filter(":focus").length == 0) return undefined;
		var self = this.get(0);

	  if (self.selectionStart) { 
	    return self.selectionStart; 
	  } else if (document.selection) { 
	    self.focus(); 

	    var r = document.selection.createRange(); 
	    if (r == null) { 
	      return 0; 
	    } 

	    var re = self.createTextRange(), 
	        rc = re.duplicate(); 
	    re.moveToBookmark(r.getBookmark()); 
	    rc.setEndPoint('EndToStart', re); 

	    return rc.text.length; 
	  }  
	  return 0;
	}
})(jQuery);

function getURLParam(param) {
  var href = window.location.href;
  if(href.indexOf("?") > -1) {
    var query = href.substr(href.indexOf("?") + 1).toLowerCase().split("&");
    for(var i = 0; i < query.length; i++ ){
      if(query[i].startsWith(param.toLowerCase() + "=")) {
        return unescape(query[i].split("=")[1]);
      } else if(query[i] == param.toLowerCase()) {
        return true;
      }
    }
  }
  return false;
}

function getSocket(token, callback) {
	$(function() {
		var host = location.host;
		// Randomize the host name if we can
		if (!(/(\d{1,3}\.){3}\d{1,3}/.test(host))) {
			host = 'realtime' + Math.floor(Math.random() * 1000000) + '.' + host
		}
		var socket = io.connect(host + '?token=' + token);
		callback(socket);
	});
}

/* image rollover stuff */
function changesrc(a,im)
{
	x = eval("document."+a);
	x.src=im;
}
