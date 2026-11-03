// Upload form: suggest a title from the picked file's name, unless that name is
// one a camera or phone made up (IMG_8451.jpeg says nothing about the picture).
// The server sanitizes the title and sets the extension, so this only strips it
// for display.
(function() {
	"use strict";

	const CAMERA_NAME = /^(img|dsc[fn]?|dcim|pxl|mvimg|photo|image|screenshot|screen shot|signal|whatsapp image|received|\d[\d_ -]*$)([ _-]|\d|$)/i;

	function suggestedTitle(name) {
		const base = name.replace(/\.[^.]*$/, "").trim();
		return CAMERA_NAME.test(base) ? "" : base;
	}

	document.addEventListener("DOMContentLoaded", function() {
		const file = document.getElementById("image");
		const title = document.getElementById("filename");
		if(!file || !title) return;

		// what we last put in the box, so we never clobber something the user typed
		let suggested = "";

		file.addEventListener("change", function() {
			const picked = file.files && file.files[0];
			if(!picked || (title.value !== "" && title.value !== suggested)) return;
			suggested = suggestedTitle(picked.name);
			title.value = suggested;
			if(suggested === "") title.focus();
		});
	});
})();
