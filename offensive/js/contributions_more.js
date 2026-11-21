// infinite scroll for ?c=contributions. the page lists a user's uploads as
// year headings (h3), each followed by its own ul (a ul.imagelist, or a
// list of thumbnails in the thumbnail view), so the shared
// lazyload_bottom.js, which appends every row to every list on the page,
// can't be used. this fetches the "more…" link's page when it nears the
// viewport and appends its headings and lists, folding a list into the
// last one on the page when it continues the same year. without
// javascript, the "more…" link still pages normally. it also applies the
// viewer's thumbnail filters (tmbolib.js) to the page and to what it adds.
(function() {
	function lastList(box) {
		var lists = box.querySelectorAll(":scope > ul");
		return lists.length ? lists[lists.length - 1] : null;
	}

	// the year heading that a list sits under, or null for the votes sort.
	function headingFor(list) {
		var prev = list.previousElementSibling;
		return prev && prev.tagName == "H3" ? prev.textContent : null;
	}

	function init() {
		hideFilteredThumbnails();

		// the lists' box, which the "more…" link ends
		var more = document.getElementById("morelink");
		if(!more || !("IntersectionObserver" in window)) {
			return;
		}
		var box = more.parentElement;
		var link = more.querySelector("a");
		var loading = false;

		function load() {
			if(loading) {
				return;
			}
			loading = true;
			link.textContent = "loading…";
			fetch(link.href, { credentials: "same-origin" })
				.then(function(response) {
					if(!response.ok) {
						throw new Error(response.status);
					}
					return response.text();
				})
				.then(function(html) {
					var doc = new DOMParser().parseFromString(html, "text/html");
					var newList = doc.querySelector(".bluebox > ul");
					if(!newList) {
						throw new Error("no list in the response");
					}
					var newBox = newList.parentElement;
					var heading = null;
					newBox.querySelectorAll(":scope > h3, :scope > ul").forEach(function(node) {
						if(node.tagName == "H3") {
							heading = node;
							return;
						}
						var last = lastList(box);
						var lastHeading = last ? headingFor(last) : null;
						var sameYear = heading ? heading.textContent === lastHeading : lastHeading === null;
						if(last && sameYear) {
							// the page carried on where the last one stopped
							while(node.firstElementChild) {
								last.appendChild(document.adoptNode(node.firstElementChild));
							}
						} else {
							if(heading) {
								more.before(document.adoptNode(heading));
							}
							more.before(document.adoptNode(node));
						}
						heading = null;
					});
					hideFilteredThumbnails();
					var next = newBox.querySelector("#morelink a");
					if(next) {
						link.href = next.href;
						link.textContent = "more…";
						loading = false;
						// still in view after a short page: keep going
						observer.unobserve(more);
						observer.observe(more);
					} else {
						observer.disconnect();
						more.remove();
					}
				})
				.catch(function() {
					// leave the plain link to try again by hand
					link.textContent = "more…";
					loading = false;
					observer.disconnect();
				});
		}

		var observer = new IntersectionObserver(function(entries) {
			if(entries.some(function(entry) { return entry.isIntersecting; })) {
				load();
			}
		}, { rootMargin: "0px 0px 100% 0px" });
		observer.observe(more);
	}

	if(document.readyState == "loading") {
		document.addEventListener("DOMContentLoaded", init);
	} else {
		init();
	}
})();
