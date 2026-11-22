/* Keeps the unread comments lists current: the sidebar (#unread) and the nav
	popover (#unreadMenu-unread). Both are rendered by unread() in
	offensive/assets/unread.inc, and this refetches that markup from
	offensive/ui/unread.php instead of rebuilding it here, so the squelch
	filtering, striping and .empty state stay in one place.

	Refreshes when:
	- the page comes back after being hidden for STALE_MS or more (tab switch,
	  phone app resume, screen unlock),
	- the page is restored from the back/forward cache,
	- the realtime service reports a subscription change or reconnects
	  (wired up in offensive/index.php). */
var refreshUnread = (function() {
	var STALE_MS = 30000;
	var DEBOUNCE_MS = 500;
	var hiddenAt = document.hidden ? Date.now() : 0;
	var timer = null;
	var inflight = false;
	var again = false;

	function load() {
		if(inflight) {
			again = true;
			return;
		}
		inflight = true;
		fetch("/offensive/ui/unread.php", { cache: "no-store" })
			.then(function(response) {
				// a lapsed session redirects to the login page; leave the lists alone.
				if(!response.ok || response.redirected) return null;
				return response.text();
			})
			.then(function(html) {
				if(html === null) return;
				var doc = new DOMParser().parseFromString(html, "text/html");
				doc.querySelectorAll(".contentbox[id]").forEach(function(fresh) {
					var current = document.getElementById(fresh.id);
					if(current) current.replaceWith(fresh);
				});
			})
			.catch(function() {
				// offline or mid-resume; the next trigger tries again.
			})
			.finally(function() {
				inflight = false;
				if(again) {
					again = false;
					load();
				}
			});
	}

	// several events can arrive together (a busy thread, a reconnect), so fetch once.
	function refresh() {
		clearTimeout(timer);
		timer = setTimeout(load, DEBOUNCE_MS);
	}

	document.addEventListener("visibilitychange", function() {
		if(document.hidden) {
			hiddenAt = Date.now();
			return;
		}
		if(hiddenAt && Date.now() - hiddenAt >= STALE_MS) refresh();
		hiddenAt = 0;
	});

	window.addEventListener("pageshow", function(e) {
		if(e.persisted) refresh();
	});

	return refresh;
})();
