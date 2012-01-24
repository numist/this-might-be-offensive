var lazy_interval_top;
var lazy_timeout = 15000;	// how often to check for new data
var lazy_limit = 40;		// how many thumbs to fetch on the bottom of the page
var update_thumbs = 200;	// how many thumbs to update if comment/vote counts change

$(document).ready(function() {
  if (update_index != undefined) {
    lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
  }
});

function lazyload_top() {
  var latest_upload = $('#grid-container ul li:first');
  var latest_id = parseInt($(latest_upload).attr('fileid'));
  var last_on_page = parseInt($('#grid-container ul li:last').attr('fileid'))-1;
  color_class = ($(latest_upload).find('.col1').hasClass('odd_row')) ? "odd_row" : "even_row";

  $.getJSON('/offensive/api.php/getchanges.json', {
    'since': update_index
  }, function(data) {
    clearInterval(lazy_interval_top);
    $.each(data, function(i, entry) {
      i = parseInt(i);
      if (i > update_index)
        update_index = i; 
      entry = entry.match(/([^:]+):([^:]+):(.*)/);
      var command = entry[1];
      var upload_id = entry[2];

      if (command == "comment") {
        var command_args = entry[3].split(":");
        
        // make sure this element is visible on this page
        if(upload_id > last_on_page) {
          // find the element on the page
          var $score = $("li[fileid='"+upload_id+"'] .score a");
          // if the element already exists, replace the score field
          // checking to see if something changed is not any faster. just replace.
          if($score.length) {
            var score_parts = $score.html().match(/^([\w]+) comment(?:s)? \(\+(\d+) -(\d+)(?: x(\d+))?\)/);
            var comments = parseInt(score_parts[1] == "no" ? "0" : score_parts[1]);
            var good = parseInt(score_parts[2]);
            var bad = parseInt(score_parts[3]);
            var offensive = score_parts[4] == undefined ? 0 : parseInt(score_parts[4]);
            comments += parseInt(command_args[4]);
            good += parseInt(command_args[0]);
            bad += parseInt(command_args[1]);
            offensive += parseInt(command_args[3]);
            $score.html(create_score_text(comments, good, bad, offensive));
          } 
        }
      } else if (command == "upload") {
        $.getJSON('/offensive/api.php/getupload.json', {'fileid' : upload_id}, function(data) {
          var html = html_upload(data);
          if(html.length > 0) {
            $('#grid-container ul').prepend(html);
  				  $("#grid-container ul li:first").fadeIn("slow");
				  }
        });
      }
    });
    lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
  });
}

function uploadobject(id, username, filename, comments, good, bad) {
  this.id = id;
  this.username = username;
  this.filename = filename;
  this.vote_good = good ? good : 0;
  this.vote_bad = bad ? bad : 0;
  this.comments = comments ? comments : 0;
}

function create_score(upload) {
  html = '<a href="./?c=comments&fileid='+upload.id+'">'+ create_score_text(upload.comments, upload.vote_good, upload.vote_bad)+'</a>';
  return(html);
}

function create_score_text(comments, good, bad, offensive) {
  // adjust
  if (comments == 0) {
	comments = "no comments";
  }
  else if (comments == 1) {
    comments = "1 comment";
  } else {
    comments = comments+" comments";
  }
  if (offensive > 0) {
    offensive = " x" + offensive;
  } else {
    offensive = "";
  }
  return comments + " (+" + good + " -" + bad + offensive + ")";
}
