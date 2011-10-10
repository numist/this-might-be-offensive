var lazy_interval = 0;
var lazy_interval_bottom;
var lazy_interval_top;
var lazy_load = false;

var lazy_timeout = 15000;	// how often to check for new data
var lazy_limit = 40;		// how many thumbs to fetch on the bottom of the page
var update_thumbs = 200;	// how many thumbs to update if comment/vote counts change

$(document).ready(function() {
  $('#commands').bind('inview', function (event, visible) {
    if (visible == true) {
      clearInterval(lazy_interval_top);
      //lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
    } else {
      clearInterval(lazy_interval_top);
    }
  });

  $('#bottom').bind('inview', function (event, visible) {
    if(visible == true && lazy_load == true) {
      lazyload_bottom();
      lazy_interval_bottom = setInterval(lazyload_bottom,lazy_timeout);
    } else {
      clearInterval(lazy_interval_bottom);
    }
  });
  $(window).scroll();

  $('#bottom a').click(function(e) {
    e.preventDefault();
    lazy_load = true;
    lazyload_bottom();
    $('#bottom p').css('display','none');
    $(window).scroll();
  });

});

function lazyload_top() {
  var latest_upload = parseInt($('#grid-container ul li:first').attr('fileid'));
  var last_on_page = parseInt($('#grid-container ul li:last').attr('fileid'))-1;

  $.getJSON('/offensive/api.php/getuploads.json', {
    'type': "image",
    'since': latest_upload - update_thumbs,
    'sort': 'date_asc'
  }, function(data) {
    clearInterval(lazy_interval_top);

    $.each(data, function(i, upload) {
      // make sure this element is visible on this page
      if(upload.id > last_on_page) {
        // find the element on the page
        var $score = $("li[fileid='"+upload.id+"'] .score");

        // if the element already exists, replace the score field
        // checking to see if something changed is not any faster. just replace.
        if($score.length) {
          $score.html(create_score(upload));
        } else {
          // create a new element
          var html = get_element(upload);
          $('#thumbnails ul').prepend(html);
          $("#thumbnails ul li:first").show("slow");
        }
      }
    });

    lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
  });
}

function lazyload_bottom() {
  var last_on_page = parseInt($('#grid-container ul li:last').attr('fileid'))-1;
  var opts = {
     'type': "image",
     'max': last_on_page,
     'sort': 'date_desc',
     'limit': lazy_limit
  };
  var userid_match = window.location.search.match(/userid=(\d+)/);
  if (userid_match) {
    opts.userid = userid_match[1];
  }
  $.getJSON('/offensive/api.php/getuploads.json', opts, function(data) {
    $.each(data, function(i, upload) {
      html = get_element(upload);
      $('#thumbnails ul').append(html);
      //$("#thumbnails ul li:last").fadeIn();
      $("#thumbnails ul li:last").css('display', 'block');
    });

    $(window).scroll();
  });
}

function get_element(upload) {
  // we cant find a thumbnail for this image.
  if(typeof(upload.link_thumb) == "undefined")  upload.link_thumb = "/offensive/graphics/previewNotAvailable.gif";

  // image is filtered
  if(upload.filtered == true) {
    orig_src = upload.link_thumb;
    upload.link_thumb = "/offensive/graphics/th-filtered.gif";
    mouseout = "onmouseout='changesrc(\"th"+upload.id+"\",\"/offensive/graphics/th-filtered.gif\")' onmouseover='changesrc(\"th"+upload.id+"\",\""+orig_src+"\")'";
  } else {
    mouseout = "";
  }

  html = '<li style="display: none;" fileid="'+upload.id+'"><div class="thumbcontainer"><div><a '+mouseout+' href="pages/pic.php?id='+upload.id+'"><img title="uploaded by '+upload.username+'" src="'+upload.link_thumb+'" name="th'+upload.id+'"/></a></div><div class="score">'+create_score(upload)+'</div></div></li>';
  return(html);
}

function create_score(upload) {
  // adjust
  if (upload.comments == "0") {
	  upload.comments = "no comments";
  }
  else if (upload.comments == "1") {
    upload.comments = "1 comment";
  } else {
    upload.comments = upload.comments+" comments";
  }


  html = '<a href="./?c=comments&fileid='+upload.id+'">'+upload.comments+' (+'+upload.vote_good+' -'+upload.vote_bad+')</a>';
  return(html);
}
