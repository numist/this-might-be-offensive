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
  String.prototype.endsWith = function (str){
    if(this.length < str.length) {
      // if this.length - str.length == -1, this could cause a false positive
      return false;
    }

    return this.lastIndexOf(str) == this.length - str.length;
  }
}

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