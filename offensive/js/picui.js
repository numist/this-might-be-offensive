/******************************************************************************
 * quick comments
 *****************************************************************************/

function qc_dialog_init() {

		function qc_headers(self) {
		  if($("#qc_comments").hasAttr("loading")) {
				return;
			}
			
			if($("#qc_commentrows").children().length == 0) {
				if($("#qc_form:visible").length > 0) {
					self.dialog("option", "title", "first post!");
				} else {
					self.dialog("option", "title", "nothing to see here, move along");
				}
			} else {
				self.dialog("option", "title", "let's hear it");
				if($("#qc_comments").children("b").length == 0) {
					$("#qc_comments").prepend("<b>the dorks who came before you said: </b>");
				}
			}
			
			qc_autosize(self);
		}

		function qc_autosize(self) {
			// if dialog is not open, do not mess with it—its height is zero
			if(!self.dialog("isOpen")) { return; }

			// if dialog has been manipulated by the user, do not mess with the size
			if(self.hasAttr("modified")) {
				// Note: the contents of the box may still need a re-fit (eg: disable_voting).
				qc_fit(self);
				return;
			}

			// scale to fit content
			var commentRows = self.find("#qc_commentrows");
			if(commentRows.filter(":visible").length > 0) {
				self.height(commentRows.position().top + commentRows.get(0).scrollHeight);
				// limit the maximum height
				// Note: using .dialog("option", "maxHeight") would restrict the user from embiggening it further
				if(self.height() > $(window).height() - 150) {
					self.height($(window).height() - 150);
				}
			} else {
				// no comments to fit, no need to get fancy.
				self.height("");
			}
			// re-center the qc box since it's now not properly centered
			self.dialog("option", "position", "center");

			// since we may have changed the height, re-fit the contents
			qc_fit(self);
		}

		function qc_fit(self) {
			// since we're resizing elements in JS anyway, make the textarea fit correctly
			var textarea = self.find("textarea#qc_comment");
			textarea.width(self.width() - (textarea.outerWidth(true) - textarea.width()));

			// commentrows height = bottom of dialog(top of dialog + height of dialog) - top of comments
			var commentRows = self.find("#qc_commentrows:visible");
			if(commentRows.children().length > 0) {
				commentRows.height(self.height() - commentRows.position().top)
			}
		}

		// handle form submission from within comments box
		$("form#qc_form").on("submit", function(e) {
			e.preventDefault();
			
			var dialog = $("#qc_dialog");
			
			vote = $('#qc_form input[name=vote]:checked').val();
			comment = $("#qc_comment").val();
			tmbo = $("#qc_tmbo").attr("checked") ? "1" : "0";
			repost = $("#qc_repost").attr("checked") ? "1" : "0";
			subscribe = $("#qc_subscribe").attr("checked") ? "1" : "0";

			dialog.dialog("close");

			handle_comment_post(comment, vote, tmbo, repost, subscribe);
		});

		// remember the scroll height
		$("#qc_commentrows").on("scroll", function(){
			self = $(this);
			if(self.scrollTop() > 0) {
				self.attr("scrollTop", self.scrollTop());
			} else {
				self.removeAttr("scrollTop");
			}
		});
		
		// remember location of the caret
		function qc_save_caret(e) {
			var self = $(this);
			// Note: Capture the caret's position after the event has actually moved it
			window.setTimeout(function(){
				if(self.getCaretPosition() != undefined) {
					self.attr("caret", self.getCaretPosition());
				}
			}, 0);
		}
		$("#qc_comment").on("keydown", qc_save_caret).on("click", qc_save_caret);

		function qc_start_manipulation(self) {
			// fade out
			self.dialog("widget").fadeTo("fast", 0.7);
			
			// disable automatic size/position after the box has been manipulated
			self.attr("modified", "");
			
			// this has to be done every time the box is moved/resized due to an issue with Internet Explorer
			$(window).off("clickoutside.qc");
		}
		
		function qc_done_manipulation(self) {
			// fade in
			self.dialog("widget").fadeTo("fast", 1);
		  
			// reflow contents/quick box
			qc_autosize(self);

			// this has to be done every time the box is moved/resized due to an issue with Internet Explorer
			qc_bind_clickoutside(self);
		}
		
		function qc_bind_clickoutside(self) {
			window.setTimeout(
				function(){
					self.dialog("widget").on("clickoutside.qc", function(){
						self.dialog("close");
					});
				}, 0);
		}

		return {
			autoOpen: false,
			title: "please stand by",
			width: "500px",
			open: function(event, ui) {
				var self = $(this);

				// restore scroll state
				var commentRows = $("#qc_commentrows");
				if(commentRows.hasAttr("scrollTop")) {
					commentRows.scrollTop(commentRows.attr("scrollTop"));
				}
				
				// restore caret position in comment box
				var comment = $("#qc_comment");
				var caret = comment.hasAttr("caret") ? comment.attr("caret") : 0;
				comment.focus().setCaretPosition(caret);

				// disable normal events
				unbind_default_events()
									
				// window resize should trigger a reflow.
				$(window).on("resize.qc", function(e,o){qc_autosize(self);});

				// clicking outside the box closes it
				qc_bind_clickoutside(self);

				// load comments into the quick window
				var comments = $("#qc_comments");
				if(!comments.hasAttr("loading")) {
					// get comments
					$.ajax({
       	  	type: 'GET',
       	  	url: "/offensive/ui/api.php/getcomments.html?fileid="+getURLParam("id"),
       	  	dataType: "html",
						beforeSend: function() {
							if(comments.filter(":visible").length == 0) {
								comments.show();
							}
							comments.attr("loading", "");
							if(commentRows.children().length == 0) {
								// user-facing loading feedback
								commentRows.text("loading…");
							}
							qc_autosize(self);
						},
       	  	success: function(data) {
							// call was not unsuccessful
							if($(data).filter("div#comments").length != 1) {
								return;
							}
          	
							comments.removeAttr("loading");
          	
							// remember how many comments we were displaying before
							var thecount = commentRows.children().length;
							var atBottom = self.height() > 0
							            && commentRows.scrollTop() > 0
							            && commentRows.scrollTop() == commentRows.get(0).scrollHeight - commentRows.height();
							
							// remove loading feedback
							if(thecount == 0) {
								commentRows.text("");
							}
       				
							// get comments from result
       	  	  var filteredData = $(data).find("div.entry");
	      		  if(filteredData.length > 0) {
								comments.show();
								if(thecount > 0) {
									commentRows.children().remove();
								}
								commentRows.append(filteredData);
								if(thecount == 0) {
									// put some padding between the form and the comments
									$("#qc_form").css("padding-bottom", "10px");
								}
								// scroll down if user had already scrolled to bottom
								if(atBottom) {
									// don't scroll more than one window height
									var scrollto = Math.min(commentRows.scrollTop() + commentRows.height(),
									                        commentRows.get(0).scrollHeight - commentRows.height());
									commentRows.animate({scrollTop : scrollto}, 500);
								}
	      		  } else if(commentRows.children().length == 0) {
								// there should never be a case where there were comments and then the API returns none, but plan for failure!
								comments.hide();
							}
							// update headers
							qc_headers(self);
       	  	},
       	  	complete: function(jqXHR, textStatus) {
							// failed API query
							if(comments.hasAttr("loading")) {
								commentRows.text("fuck. try again?");
								comments.removeAttr("loading");
								qc_autosize(self);
							}
       	  	}
       		});
				}
			},
			close: function(event, ui) {
				// re-enable normal behaviour
				bind_default_events();
				
				// clean up all qc bindings.
				$(this).dialog("widget").off(".qc").find("*").off(".qc");
				$(document).off(".qc");
				$(window).off(".qc");
			},
			dragStart: function(event, ui) {
				qc_start_manipulation($(this));
			},
			dragStop: function(event, ui) {
				var self = $(this), widget = self.dialog("widget");
				
				qc_done_manipulation(self);
				
				// remember the location of the box so it doesn't move itself next time it's opened
				self.dialog("option", "position", [widget.offset().left, widget.offset().top]);
			},
			resizeStart: function(event, ui) {
				qc_start_manipulation($(this));
			},
			resize: function(event, ui) {
				qc_fit($(this));
			},
			resizeStop: function(event, ui) {
				qc_done_manipulation($(this));
			}
		};
};

function qc_form_reset() {
	// reset the quick form
	$("#qc_form textarea").val("");
	$("#qc_comment").removeAttr("caret");
	$("#qc_tmbo").removeAttr("checked");
	$("#qc_repost").removeAttr("checked");
	$("#qc_subscribe").removeAttr("checked");
}

/******************************************************************************
 * global actions
 *****************************************************************************/

function bind_default_events() {
	$(document).on("keydown.default", handle_keypress);
	$(document).focus();
}

function unbind_default_events() {
	$(document).off(".default");
}

// perform a vote. The object here is the div that belongs to this vote link
function do_vote(o) {
  if(!o.parent().hasClass('on')) {
    return;
  }
  
	// make sure we can't vote again
	$("#votelinks a").unbind();			// remove click event handler

	vote = o.attr("id");				
	imageid = getURLParam("id");

	if(vote == "good") {
		handle_comment_post("", "this is good", "", "0", "0", "0");
	} else {
		handle_comment_post("", "this is bad", "", "0", "0", "0");
	}
}

function handle_comment_post(comment, vote, tmbo, repost, subscribe) {
	var fileid = getURLParam("id");
	if(comment == undefined) comment = "";
	if(vote == undefined) vote = "";
	if(tmbo == undefined) tmbo = "0";
	if(repost == undefined) repost = "0";
	if(subscribe == undefined) subscribe = "0";
	
	// we just clicked the 'go' button without selecting anything
	if(comment == "" && (vote == "novote" || vote == "") && tmbo == 0 && repost == 0 && subscribe == 0)
		return;
		
	if(vote == "this is good" || vote == "this is bad") {
		disable_voting();
	}
	
	// post the submit data using ajax
	$.post("/offensive/api.php/postcomment.php", {
		  fileid: fileid,
		  comment: comment,
		  vote: vote,
		  offensive: tmbo,
		  repost: repost,
		  subscribe: subscribe
		}, function(data) {
			// increment counts
			if(vote == "this is good") increase_count("#count_good");
			else if(vote == "this is bad") increase_count("#count_bad");
			if(comment != "") increase_count("#count_comment");
			if(tmbo != 0 && $("#count_tmbo").length > 0) increase_count("#count_tmbo");
			
			// if you made a comment or you voted 'this is bad', you have auto-subscribed to the thread
			if(comment != '' || vote == "this is bad" || subscribe != "0") {
				toggle_subscribe("subscribe", fileid, $("#subscribeLink"));
			}
			
			qc_form_reset();

			// disable voting
			if(vote == "this is good" || vote == "this is bad") {
				greyout_voting();
			}
		}
	);
}

function disable_voting() {
	// remove all hrefs from the a links inside that div
  $("#good").parent().find("a").removeAttr("href");
}

function greyout_voting() {
	$("#good").parent().removeClass("on").addClass("off");
	$("#qc_vote").remove();
}

/* image rollover stuff */
function changesrc(a,im)
{
	x = eval("document."+a);
	x.src=im;
}

/******************************************************************************
 * bound actions
 *****************************************************************************/

// handle a vote that was clicked on
function handle_vote(o,e)
{
	e.preventDefault();		// prevent the link to continue
	do_vote(o);
}

function increase_count(id) {
	count = parseInt($(id).html()) + 1;
	$(id).html(count);
}

// from: https://github.com/numist/jslib/blob/master/irsz.js
function image_dimensions(image, func) {
  var attr_width = "max-width", attr_height = "max-height", units = "px", image_width, image_height;
  image = $(image);
  if(image.length != 1 || image.attr("src") == undefined) { return; }
  
  if(image.filter("["+attr_width+"]["+attr_height+"]").length == 1) {
    // found cached/supplied image dimensions
    var pixels_width, pixels_height;
    image_width = image.attr(attr_width);
    pixels_width = parseInt(image_width.endsWith(units)
                          ? image_width.substr(0, image_width.lastIndexOf(units))
                          : image_width);
    image_height = image.attr(attr_height);
    pixels_height = parseInt(image_height.endsWith(units)
                           ? image_height.substr(0, image_height.lastIndexOf(units))
                           : image_height);
    func(pixels_width, pixels_height);
  } else {
    // get dimensions from image. make a copy in memory to avoid css issues.
    $("<img/>")
      .attr("src", image.attr("src"))
      .load(function() {
        image_width = this.width, image_height = this.height;
        image.attr(attr_width, image_width+units).attr(attr_height, image_height+units);
        func(image_width, image_height);
      });
  }
}


/******************************************************************************
 * Global stuff
 *****************************************************************************/

// prevent sites from hosting this page in a frame;
if( window != top ) {
	top.location.href = window.location.href;
}

// get your image here
function theimage() { return $(document).find("a#imageLink img").last(); };

/******************************************************************************
 * Events: document ready
 *****************************************************************************/

$(document).ready(function() {
  // init quick comment box
  $("#qc_dialog").dialog(qc_dialog_init());

	// bind vote links
	$("#votelinks a").click(function(e) {
		handle_vote($(this),e);
	});

	// bind subscribe link
	$("#subscribeLink").click(function(e) {
		handle_subscribe($(this),e,$("#good").attr("name"));
	});
	$("#unsubscribeLink").click(function(e) {
		handle_subscribe($(this),e,$("#good").attr("name"));
	});
	
	// bind quick link
	$("#quickcomment").bind("click", function(e){ $("#qc_dialog").dialog("open"); e.preventDefault(); });

	// start reacting to events normally
	bind_default_events();

	// bind instructions link
	$("#instruction_link a").click(function () {
		$("#instructions").toggle();
	});

	// show image dimensions
	image_dimensions(theimage(), function(width, height) {
	  $("span#dimensions").append(", "+width+"x"+height+' <span id="scaled"></span>');
	});
	
	// set up image resizer
	var ypad = $("body").height() - theimage().outerHeight(true);
	var xpad = $("div#content").outerWidth(true) - $("div#content").width();
	theimage().irsz({
		min_height: 40, min_width: 40,
		padding: [xpad, ypad],
		cursor_zoom_in: "url(/offensive/graphics/zoom_in.cur),default", cursor_zoom_out: "url(/offensive/graphics/zoom_out.cur),default"
	})
	.resize(	function() {
		if($("span#scaled").length == 0) { return; }
    if(theimage().length == 0) { return; }

		var image = theimage();
		var current_width = image.width(), current_height = image.height();
    
		image_dimensions(image, function(actual_width, actual_height) {
		  if(actual_width != current_width || actual_height != current_height) {
		    $("span#scaled").text("(shown: "+current_width+"x"+current_height+")");
		  } else {
		    $("span#scaled").text("");
		  }
		})
	})
	.resize();
});