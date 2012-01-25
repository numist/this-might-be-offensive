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

function getURLParam(param) {
  var strReturn = "";
  var href = window.location.href;
  if(href.indexOf("?") > -1) {
    var query = href.substr(href.indexOf("?")).toLowerCase().split("&");
    for(var i = 0; i < query.length; i++ ){
      if(query[i].startsWith(param.toLowerCase() + "=") > -1) {
        return unescape(query[i].split("=")[1]);
      } else if(query[i] == param.toLowerCase()) {
        return true;
      }
    }
  }
  return false;
}