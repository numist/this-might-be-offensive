// Swipe left/right on the pic page to follow the "older"/"newer" links.
// Mirrors the arrow keys: swiping left brings in the item to the right (older),
// swiping right brings in the one to the left (newer).
(function() {
	"use strict";

	const MIN_DISTANCE = 60;  // px of horizontal travel
	const MAX_SLOPE = 0.5;    // |dy| / |dx|; anything steeper is a scroll
	const MAX_DURATION = 600; // ms; slower drags are panning/reading, not swiping
	const EDGE_ZONE = 24;     // px; leave screen edges to the OS back/forward gesture

	const IGNORE = "input, textarea, select, [contenteditable], .ui-dialog, .ui-widget-overlay";

	let start = null;

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

	function follow(id) {
		const link = document.getElementById(id);
		// with nothing newer/older the link is a hidden placeholder pointing at the index
		if(!link || getComputedStyle(link).visibility === "hidden") return;
		document.location.href = link.href;
	}

	document.addEventListener("touchstart", function(e) {
		start = null;
		if(e.touches.length !== 1) return;
		const t = e.touches[0];
		if(t.clientX < EDGE_ZONE || t.clientX > window.innerWidth - EDGE_ZONE) return;
		if(e.target.closest && e.target.closest(IGNORE)) return;
		if(zoomed() || scrollsHorizontally(e.target)) return;
		start = { x: t.clientX, y: t.clientY, time: e.timeStamp, scrollX: window.scrollX };
	}, { passive: true });

	document.addEventListener("touchmove", function(e) {
		if(e.touches.length !== 1) start = null; // pinch
	}, { passive: true });

	document.addEventListener("touchcancel", function() {
		start = null;
	}, { passive: true });

	document.addEventListener("touchend", function(e) {
		if(!start || e.touches.length !== 0) return;
		const s = start;
		start = null;
		const t = e.changedTouches[0];
		const dx = t.clientX - s.x;
		const dy = t.clientY - s.y;
		if(Math.abs(dx) < MIN_DISTANCE) return;
		if(Math.abs(dy) > Math.abs(dx) * MAX_SLOPE) return;
		if(e.timeStamp - s.time > MAX_DURATION) return;
		if(window.scrollX !== s.scrollX || zoomed()) return;
		const selection = window.getSelection && window.getSelection();
		if(selection && !selection.isCollapsed) return;
		follow(dx < 0 ? "previous" : "next");
	}, { passive: true });
})();
