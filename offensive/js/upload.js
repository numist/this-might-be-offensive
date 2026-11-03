// Upload form: suggest a title from the picked file's name, unless that name is
// one a camera or phone made up (IMG_8451.jpeg says nothing about the picture).
// The server sanitizes the title and sets the extension, so this only strips it
// for display.
(function() {
	"use strict";

	const CAMERA_NAME = /^(img|dsc[fn]?|dcim|pxl|mvimg|photo|image|screenshot|screen shot|signal|whatsapp image|received|\d[\d_ -]*$)([ _-]|\d|$)/i;

	// the extension the server gives a typed title (upload_type() uses the
	// format's name, and HEIC/AVIF are converted to JPEG). A guess from the
	// file's name; the server goes by the file's contents.
	const STORED_EXTENSION = {
		jpg: "jpg", jpeg: "jpg", jpe: "jpg", jfif: "jpg", jif: "jpg",
		heic: "jpg", heif: "jpg", hif: "jpg", avif: "jpg",
		png: "png", gif: "gif", webp: "webp", mp3: "mp3",
	};

	function storedExtension(name) {
		const match = /\.([^.]+)$/.exec(name);
		const ext = match && STORED_EXTENSION[match[1].toLowerCase()];
		return ext ? "." + ext : "";
	}

	function withoutExtension(name) {
		return name.replace(/\.[^.]*$/, "").trim();
	}

	function suggestedTitle(name) {
		const base = withoutExtension(name);
		return CAMERA_NAME.test(base) ? "" : base;
	}

	document.addEventListener("DOMContentLoaded", function() {
		const file = document.getElementById("image");
		const title = document.getElementById("filename");
		if(!file || !title) return;

		// what we last put in the box, so we never clobber something the user typed
		let suggested = "";

		// the file input's own file-name text is hidden (styles/upload.css), so
		// the title field shows what was picked: as the suggested title, or as
		// the placeholder when the name is camera junk (that's also what the
		// server falls back to). The extension shows beside it, so nobody types it.
		const placeholder = title.placeholder;
		const extension = document.getElementById("extension");
		const defaultExtension = extension ? extension.textContent : "";

		file.addEventListener("change", function() {
			const picked = file.files && file.files[0];
			title.placeholder = picked ? withoutExtension(picked.name) : placeholder;
			if(extension) extension.textContent = picked ? storedExtension(picked.name) : defaultExtension;
			if(!picked || (title.value !== "" && title.value !== suggested)) return;
			suggested = suggestedTitle(picked.name);
			title.value = suggested;
			if(suggested === "") title.focus();
		});
	});
})();
