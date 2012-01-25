var lazy_interval_top;
var lazy_timeout = 15000;	// how often to check for new data

$(document).ready(function() {
  if (update_index != undefined) {
    lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
  }
});

function lazyload_top() {
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
        // find the element on the page
        var score = $("li[fileid='"+upload_id+"'] .score a");
        if(score.length) {
          var command_args = entry[3].split(":");
          
          // create a space for new tmbo votes, if needed
          if(score.find('.tmbos').text() == "" && parseInt(command_args[3]) > 0) {
            score.find('.bads').after('&nbsp;x<span class="tmbos">0</span>');
          }
        
          score.find('.goods').text(parseInt(command_args[0]) + parseInt(score.find('.goods').text()));
          score.find('.bads').text(parseInt(command_args[1]) + parseInt(score.find('.bads').text()));
          score.find('.tmbos').text(parseInt(command_args[3]) + parseInt(score.find('.tmbos').text()));

          var comments = parseInt(score.find('.comments').text());
          if(comments == 0) {
            score.find('.commentlabel').text("comment");
          } else if (comments == 1) {
            score.find('.commentlabel').text("comments");
          }
          score.find('.comments').text(parseInt(command_args[4]) + comments);
        }
      } else if (command == "upload" && upload_id > $('#grid-container ul li[fileid]').first().attr('fileid')) {
        var api = "/offensive/ui/api.php/";
        switch(getURLParam("c")) {
          case "main":
            api += "getfileli.php?type=image";
            break;
          case "audio":
            api += "getfileli.php?type=audio";
            break;
          case "discussions":
            api += "gettopicli.php?type=topic";
            break;
          case "thumbs":
            api += "getthumbli.php?type=image";
            break;
          default:
            return;
        }
        $.ajax({
          type: 'GET',
          url: api+"&fileid="+upload_id,
          dataType: "html",
          success: function(data) {
            var item = $(data).filter('li[fileid="'+upload_id+'"]');
            if(item.length == 1) {
              if(typeof prep_item == "function") {
                // if needed, prep the item (for css)
                item = prep_item(item, $('li[fileid]').first());
              }
              item.insertBefore($('#grid-container ul li[fileid]').first()).fadeIn("slow");
            }
          }
        });
      }
    });
    lazy_interval_top = setInterval(lazyload_top,lazy_timeout);
  });
}
