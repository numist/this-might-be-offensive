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

		// the file input itself is hidden, so the title field shows what was
		// picked: as the suggested title, or as the placeholder when the name is
		// camera junk (that's also what the server falls back to)
		const placeholder = title.placeholder;

		// no title field until there's a file to title. Hidden from here rather
		// than in the markup so the form still works without JS; checked on
		// load because the browser may restore a picked file on back/forward.
		function showTitleIfPicked() {
			title.hidden = !(file.files && file.files.length);
		}
		showTitleIfPicked();
		window.addEventListener("pageshow", showTitleIfPicked);

		file.addEventListener("change", function() {
			const picked = file.files && file.files[0];
			showTitleIfPicked();
			title.placeholder = picked ? picked.name : placeholder;
			if(!picked || (title.value !== "" && title.value !== suggested)) return;
			suggested = suggestedTitle(picked.name);
			title.value = suggested;
			if(suggested === "") title.focus();
		});
	});
})();
