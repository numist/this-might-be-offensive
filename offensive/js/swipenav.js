// Swipe left/right on the pic page to follow the "older"/"newer" links.
// Mirrors the arrow keys: swiping left brings in the item to the right (older),
// swiping right brings in the one to the left (newer).
//
// The image follows the finger, slides off when the swipe commits, and the
// next page slides in via a cross-document view transition (pic.css) where
// the browser supports one. Every swipe is still a normal page load.
(function() {
	"use strict";

	const LOCK_DISTANCE = 10;     // px of travel before deciding horizontal vs vertical
	const MIN_DISTANCE = 60;      // px; a fast flick needs at least this much
	const COMMIT_FRACTION = 0.35; // of the screen width; a slow drag needs this much
	const FLICK_SPEED = 0.3;      // px/ms over the last FLICK_WINDOW ms
	const FLICK_WINDOW = 100;     // ms
	const EDGE_ZONE = 24;         // px; leave screen edges to the OS back/forward gesture
	const RESISTANCE = 0.3;       // drag factor when there's nothing in that direction
	const SLIDE_MS = 180;
	const LAST_KEY = "swipenav-last"; // direction last travelled, for preloading
	const NAV_KEY = "swipenav-nav";   // direction of the navigation in flight

	const IGNORE = "input, textarea, select, [contenteditable], .ui-dialog, .ui-widget-overlay";

	let start = null;
	let preloaded = false;
	let leaving = false;

	function scrollsHorizontally(el) {
		for(; el && el !== document.body && el !== document.documentElement; el = el.parentElement) {
			if(el.scrollWidth > el.clientWidth) {
				const overflow = getComputedStyle(el).overflowX;
				if(overflow === "auto" || overflow === "scroll") return true;
			}
		}
		const page = document.scrollingElement || document.documentElement;
		return page.scrollWidth > page.clientWidth;
	}

	function zoomed() {
		return window.visualViewport ? window.visualViewport.scale > 1.01 : false;
	}

	function reducedMotion() {
		return window.matchMedia("(prefers-reduced-motion: reduce)").matches;
	}

	// "previous" is older (swipe left), "next" is newer (swipe right)
	function target(id) {
		const link = document.getElementById(id);
		// with nothing newer/older the link is a hidden placeholder pointing at the index
		if(!link || getComputedStyle(link).visibility === "hidden") return null;
		return link;
	}

	// sessionStorage can throw (disabled storage, private modes)
	function store(key, value) {
		try {
			if(value) sessionStorage.setItem(key, value);
			else sessionStorage.removeItem(key);
		} catch(e) {}
	}

	function read(key) {
		try { return sessionStorage.getItem(key); } catch(e) { return null; }
	}

	// Fetch the neighbors' images on the first touch so desktop never pays for
	// them. Only the direction last travelled, if known, to spare mobile data.
	function preload() {
		if(preloaded) return;
		preloaded = true;
		if(navigator.connection && navigator.connection.saveData) return;
		const last = read(LAST_KEY);
		const ids = last === "next" || last === "previous" ? [last] : ["next", "previous"];
		ids.forEach(function(id) {
			const link = target(id);
			if(link && link.dataset.image) new Image().src = link.dataset.image;
		});
	}

	function image() {
		return document.getElementById("image");
	}

	function place(img, x, ms) {
		if(!img) return;
		img.style.transition = ms ? "transform " + ms + "ms ease-out" : "none";
		img.style.transform = x ? "translateX(" + x + "px)" : "";
	}

	function reset() {
		const img = image();
		if(img) img.style.transition = img.style.transform = "";
	}

	function go(link, dx) {
		leaving = true;
		store(LAST_KEY, link.id); // pageswap also does this, where supported
		const img = image();
		if(!img || reducedMotion()) {
			document.location.href = link.href;
			return;
		}
		place(img, Math.sign(dx) * window.innerWidth, SLIDE_MS);
		setTimeout(function() { document.location.href = link.href; }, SLIDE_MS);
	}

	document.addEventListener("touchstart", function(e) {
		start = null;
		if(leaving || e.touches.length !== 1) return;
		preload();
		const t = e.touches[0];
		if(t.clientX < EDGE_ZONE || t.clientX > window.innerWidth - EDGE_ZONE) return;
		if(e.target.closest && e.target.closest(IGNORE)) return;
		if(zoomed() || scrollsHorizontally(e.target)) return;
		start = { x: t.clientX, y: t.clientY, locked: false, recent: [{ x: t.clientX, time: e.timeStamp }] };
	}, { passive: true });

	// not passive: once a swipe is locked horizontal it must stop the page scrolling
	document.addEventListener("touchmove", function(e) {
		if(!start) return;
		if(e.touches.length !== 1) { // pinch
			start = null;
			place(image(), 0, SLIDE_MS);
			return;
		}
		const t = e.touches[0];
		const dx = t.clientX - start.x;
		const dy = t.clientY - start.y;
		if(!start.locked) {
			if(Math.max(Math.abs(dx), Math.abs(dy)) < LOCK_DISTANCE) return;
			if(Math.abs(dy) >= Math.abs(dx)) { // a scroll; let it be
				start = null;
				return;
			}
			start.locked = true;
		}
		// uncancelable when the touch interrupts a momentum scroll; the swipe still works
		if(e.cancelable) e.preventDefault();
		start.recent.push({ x: t.clientX, time: e.timeStamp });
		// keep one sample at least FLICK_WINDOW old to measure speed against
		while(start.recent.length > 1 && e.timeStamp - start.recent[1].time >= FLICK_WINDOW) start.recent.shift();
		const link = target(dx < 0 ? "previous" : "next");
		place(image(), link ? dx : dx * RESISTANCE, 0);
	}, { passive: false });

	document.addEventListener("touchcancel", function() {
		if(start && start.locked) place(image(), 0, SLIDE_MS);
		start = null;
	}, { passive: true });

	document.addEventListener("touchend", function(e) {
		if(!start || e.touches.length !== 0) return;
		const s = start;
		start = null;
		if(!s.locked) return;
		const x = e.changedTouches[0].clientX;
		const dx = x - s.x;
		const from = s.recent[0];
		const velocity = (x - from.x) / Math.max(1, e.timeStamp - from.time);
		const far = Math.abs(dx) >= Math.max(MIN_DISTANCE, window.innerWidth * COMMIT_FRACTION);
		// a flick back toward the start cancels, however far the drag went
		const back = Math.sign(velocity) === -Math.sign(dx) && Math.abs(velocity) >= FLICK_SPEED;
		const flick = Math.abs(dx) >= MIN_DISTANCE && Math.sign(velocity) === Math.sign(dx) && Math.abs(velocity) >= FLICK_SPEED;
		const link = target(dx < 0 ? "previous" : "next");
		if(link && !back && (far || flick)) {
			go(link, dx);
		} else {
			place(image(), 0, SLIDE_MS);
		}
	}, { passive: true });

	// coming back via the back button restores the page with the image slid off
	window.addEventListener("pageshow", function(e) {
		if(e.persisted) {
			leaving = false;
			reset();
		}
	});

	// Tag pic-to-pic navigations (swipe, arrow keys, or clicking newer/older)
	// with a direction so pic.css can slide the new page in from that side.
	window.addEventListener("pageswap", function(e) {
		if(!e.activation || !e.activation.entry) return;
		const url = e.activation.entry.url;
		const dir = ["next", "previous"].find(function(id) {
			const link = target(id);
			return link && link.href === url;
		});
		store(NAV_KEY, dir);
		if(dir) store(LAST_KEY, dir);
	});

	window.addEventListener("pagereveal", function(e) {
		const dir = read(NAV_KEY);
		store(NAV_KEY, null);
		if(e.viewTransition && dir) {
			e.viewTransition.types.add(dir === "next" ? "newer" : "older");
		}
	});
})();
