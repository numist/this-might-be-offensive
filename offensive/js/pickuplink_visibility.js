function pickuplink_set_visibility() {
  var pickuplink = $("div#pickuplink[fileid]");
  var firstli = $('#grid-container ul li[fileid]').first();
  
  // one pickuplink
  if(pickuplink.length == 1 && firstli.length == 1) {
    // show/hide pickuplink as appropriate
    if(pickuplink.attr("fileid") == firstli.attr('fileid')) {
      pickuplink.hide();
    } else {
      pickuplink.show();
    }
    
    // hilight appropriate rows
    var pickupli = $("#grid-container ul li[fileid='"+pickuplink.attr("fileid")+"']");
    if(pickupli.attr("fileid") != firstli.attr("fileid")) {
      pickupli.find("div.col").addClass("hilight_row");
    }
    return;
  }

  // pickuplinks differ between cookie and db
  if($("a#pickUp").length > 0) {
    // hilight appropriate rows
    $("a#pickUp").each(function(i, e) {
      var pickupli = $("#grid-container ul li[fileid='"+$(e).attr("fileid")+"']");
      if(pickupli.attr("fileid") != firstli.attr("fileid")) {
        pickupli.find("div.col").addClass("hilight_row");
      }
    });
  }
}

$(document).ready(function() {
  pickuplink_set_visibility();
});