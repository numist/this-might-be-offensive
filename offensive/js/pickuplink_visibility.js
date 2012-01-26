function pickuplink_set_visibility() {
  var pickuplink = $("div#pickuplink[fileid]");
  var firstli = $('#grid-container ul li[fileid]').first();
  
  if(pickuplink.length != 1 || firstli.length == 0) {
    return;
  }
  
  if(pickuplink.attr("fileid") == firstli.attr('fileid')) {
    pickuplink.hide();
  } else {
    pickuplink.show();
  }
}

$(document).ready(function() {
  pickuplink_set_visibility();
});