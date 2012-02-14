// on page load, call this function
$(document).ready(function() {

	// vote links
	$("#votelinks a").click(function(e) {
		handle_vote($(this),e);
	});

	// subscribe link
	$("#subscribeLink").click(function(e) {
		handle_subscribe($(this),e,$("#good").attr("name"));
	});
	$("#unsubscribeLink").click(function(e) {
		handle_subscribe($(this),e,$("#good").attr("name"));
	});
	

	// keyboard events
	$(document).keydown(function(e) {
		handle_keypress($(this),e);
	});

	// toggle instructions
	$("#instruction_link a").click(function () {
		$("#instructions").toggle();
	});

	// quick comment
	handle_quickcomment();
	
	// image dimensions
	image_dimensions(theimage(), function(width, height) {
	  $("span#dimensions").append(", "+width+"x"+height+' <span id="scaled"></span>');
	});
	
	// scaling factor
	theimage().resize(function() {
	  var current_width = $(this).width(), current_height = $(this).height();
	  image_dimensions(this, function(actual_width, actual_height) {
	    if(actual_width != current_width || actual_height != current_height) {
	      $("span#scaled").text("(shown: "+current_width+"x"+current_height+")");
	    } else {
	      $("span#scaled").text("");
	    }
	  })
  }).resize();
});

// prevent sites from hosting this page in a frame;
if( window != top ) {
	top.location.href = window.location.href;
}

// handle a vote that was clicked on
function handle_vote(o,e)
{
	e.preventDefault();		// prevent the link to continue
	do_vote(o);
}

// perform a vote. The object here is the div that belongs to this vote link
function do_vote(o) {
  if(!o.parent().hasClass('on')) {
    return;
  }
  
	// make sure we can't vote again
	$("#votelinks a").unbind();			// remove click event

	vote = o.attr("id");				
	imageid = o.attr("name");			// we've added the image id as an attribute to the votelinks div

	disable_voting();
	if(vote == "good") {
		handle_comment_post(imageid, "", "this is good", "", "0", "0", "0");
		increase_count("#count_good");
	} else {
		handle_comment_post(imageid, "", "this is bad", "", "0", "0", "0");
		increase_count("#count_bad");
	}
}

form_text = "";
function handle_quickcomment()
{
	fileid = $("#good").attr("name");
	// make the #dialog div into a popup window
	$('#dialog').jqm({
		onHide: function(hash) {
			// save form text
			form_text = $("#qc_form textarea").val();
			// keyboard events
			$(document).keydown(function(e) {
				handle_keypress($(this),e);
			});
			if($.browser.opera)
				$("body div:first").removeClass('jqmOverlay');
			hash.w.fadeOut('fast',function() {
				hash.o.remove();
			});
		},
		onShow: function(hash) {
			// remove all key events
			$(document).unbind('keydown');
			$(document).keydown(function(e) {
				handle_qc_keypress($(this),e);
			});

			// get the comments
			get_comments(hash);

			// set a focus function
			hash.w.focus(function() {
				$("#qc_comment").focus();
			});

			// show the window
			hash.w.fadeIn('fast', function () {
				hash.o.show();
				hash.w.focus();
			});
			
		}
	});		

	// draggable
	$("#dialog").jqDrag('.blackbar');
}

// get the comment box content through ajax
function get_comments(hash) {
	fileid = $("#good").attr("name");
	$.get("/offensive/ui/api.php/getquickcommentbox.php", {
		fileid: fileid },
		function(data) {
			// insert the html into #qc_bluebox
			$("#qc_bluebox").html(data);
			
			// restore text
			$("#qc_form textarea").val(form_text);

			// create different colors for each row of data
			$(".qc_comment:even").css("background-color", "#bbbbee");

			// show the dialog box 
			$("#dialog").css("display", "block");

			// add a handler for the submit button
			// we can only do that here because before this time the
			// form didnt exist
			$("form#qc_form").submit(function(e){
				// prevent the default html submit
				e.preventDefault();

				// hide the dialog box
				$("#dialog").jqmHide();
				vote = $('#dialog input[name=vote]:checked').val();
				comment = $("#qc_comment").val();
				tmbo = $("#tmbo").attr("checked") ? "1" : "0";
				repost = $("#repost").attr("checked") ? "1" : "0";
				subscribe = $("#subscribe").attr("checked") ? "1" : "0";

				if(vote == "this is good" || vote == "this is bad") {
					disable_voting();
					if(vote == "this is good") increase_count("#count_good");
					else if(vote == "this is bad") increase_count("#count_bad");
				}
				if(comment != "") increase_count("#count_comment");

				handle_comment_post(fileid, comment, vote, tmbo, repost, subscribe);
				form_text = "";
			});
		});
}

function handle_comment_post(fileid, comment, vote, tmbo, repost, subscribe) {
	if(comment == undefined) comment = "";
	if(vote == undefined) vote = "";
	if(tmbo == undefined) tmbo = "0";
	if(repost == undefined) repost = "0";
	if(subscribe == undefined) subscribe = "0";
	
	// XXX: not really convinced of this solution.  this fixes the bug, but not the behaviour.
	// we just clicked the 'go' button without selecting anything
	if(comment == "" && (vote == "novote" || vote == "") && tmbo == 0 && repost == 0 && subscribe == 0)
		return;
	
	// post the submit data using ajax
	$.post("/offensive/api.php/postcomment.php", {
		  fileid: fileid,
		  comment: comment,
		  vote: vote,
		  offensive: tmbo,
		  repost: repost,
		  subscribe: subscribe
		}, function(data) {
			if(vote == "this is good" || vote == "this is bad") {
				greyout_voting();
			}

			// if you made a comment or you voted 'this is bad', you have auto-subscribed to the thread
			if(comment != '' || vote == "this is bad" || subscribe != "0") {
				toggle_subscribe("subscribe", fileid, $("#subscribeLink"));
			}
		}
	);
}

function disable_voting() {
	o = $("#good");
        o.parent().find("a").removeAttr("href");        // remove all hrefs from the a links inside that div
}

function greyout_voting() {
	o = $("#good");
        o.parent().removeClass("on");                   // switch the class from on to off
        o.parent().addClass("off");
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