String.prototype.htmlescape = function () {
	return(
		this.replace(/&/g,'&amp;').
		     replace(/>/g,'&gt;').
		     replace(/</g,'&lt;').
		     replace(/"/g,'&quot;')
	);
};
