var lazy_interval = 0;
var lazy_timeout  = 30000; //miliseconds
var lazy_interval_bottom;
var lazy_interval_top;
var lazy_load = false;
var color_class = "";
var lazy_limit = 100;

$(document).ready(function() {
  $('#commands').bind('inview', function (event, visible) {
    if (visible == true) {
      lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
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
  var latest_upload = $('#grid-container ul li:first');
  var latest_id = parseInt($(latest_upload).attr('fileid'));
  color_class = ($(latest_upload).find('.col1').hasClass('odd_row')) ? "odd_row" : "even_row";

  $.getJSON('/offensive/api.php/getuploads.json', {
    'type': "image",
    'since': latest_id,
    'sort': 'date_asc'
  }, function(data) {
    clearInterval(lazy_interval_top);
    $.each(data, function(i, upload) {
      html = get_element(upload);
      $('#grid-container ul').prepend(html);
      $("#grid-container ul li:first").fadeIn("slow");
    });
    lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
  });
}

function lazyload_bottom() {
  var last_on_page = $('#grid-container ul li:last');
  var last_id = parseInt($(last_on_page).attr('fileid'))-1;
  color_class = ($(last_on_page).find('.col1').hasClass('odd_row')) ? "odd_row" : "even_row";

  $.getJSON('/offensive/api.php/getuploads.json', {
    'type': "image",
    'max': last_id,
    'sort': 'date_desc',
    'limit': lazy_limit
  }, function(data) {
    $.each(data, function(i, upload) {
      html = get_element(upload);
      $('#grid-container ul').append(html);
      $("#grid-container ul li:last").fadeIn();
    });

    $(window).scroll();
  });
}

function get_element(upload) {
  if (upload.comments == "0") {
	upload.comments = "no comments";
  }
  else if (upload.comments == "1") {
    upload.comments = "1 comment";
  } else {
    upload.comments = upload.comments+" comments";
  }

  color_class = (color_class == "odd_row") ? "even_row" : "odd_row";

  html = '<li style="display: none;" fileid="'+upload.id+'"><div class="col col1 '+color_class+'"><a href="pages/pic.php?id='+upload.id+'" title="uploaded by '+upload.username+'">'+upload.filename.htmlescape()+'</a></div><div class="col col2 '+color_class+'"><a href="./?c=comments&fileid='+upload.id+'">'+upload.comments+' (+'+upload.vote_good+' -'+upload.vote_bad+')</a></div></li>';

  return(html);
}
