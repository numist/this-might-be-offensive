function toggle_subscribe(sub,imageid,o) {
	if(sub == "subscribe") {
		o.html("unsubscribe");
		o.attr({
			"title": "take this file off my 'unread comments' watch list.",
			"href":  "/offensive/subscribe.php?fileid=" + imageid + "&un=1"
		});
	
	} else {
		o.html("subscribe");
		o.attr({
			"title": "watch this thread for new comments.",
			"href":  "/offensive/subscribe.php?fileid=" + imageid
		});
	}
}

function handle_subscribe(o,e, imageid) {
	e.preventDefault();
	var sub = o.html();

	toggle_subscribe(sub,imageid,o);
	if(sub == "subscribe") {
		$.get("/offensive/api.php/subscribe.php", { threadid: imageid, subscribe: 1 } );
	} else {
		$.get("/offensive/api.php/subscribe.php", { threadid: imageid, subscribe: 0 } );
	}
}