// ©/info: https://github.com/numist/jslib/blob/master/irsz.js
(function() {
  $(document).ready(function() {
    // if images are not already loaded, attach a function to fire on arrival
    irsz_selector(document).load(function() {
      if(irsz_auto) {
        image_fit(this, false);
      }
    });
    
    // bind function to resize as needed during viewport resize
    $(window).resize(function() {
      if(irsz_auto) {
        irsz_selector(document).each(function(i, e) {
          image_fit(e, false);
        });
      }
    });
    
    irsz_selector(document).each(function(i, e) {
      // attach click handler for manual zooming
      $(e).click(function(){
        var prevwidth = $(this).width();
        image_toggle(this, false);
        return prevwidth == $(this).width();
      });
      
      // if iages are already loaded, fit them
      if(irsz_auto) {
        image_fit(this, false);
      }
    });
  });
  
  // keep images zoomed in when they were click-zoomed
  var noresize_class = "irsz_noresize";

  // get image's actual dimensions
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
  
  function resetcursor(image) { image.style.cursor = "auto"; }
  
  // zoom image in/out
  function image_toggle(image, animate) {
    if(!irsz_enabled) { resetcursor(image); return; }
    
    image_dimensions(image, function(actual_width, actual_height) {
      // check both dimensions in case there's a bug elsewhere we're resetting
      if($(image).width() < actual_width || $(image).height < actual_height) {
        $(image).addClass(noresize_class);
        image.style.cursor = "url(/offensive/graphics/zoom_out.cur),default";
        image_resize(image, actual_width, actual_height, animate);
      } else {
        $(image).removeClass(noresize_class);
        image_fit(image, animate);
      }
    });
  }
  
  function image_fit(image, animate) {
    if(!irsz_enabled) { resetcursor(image); return; }
    
    image_dimensions(image, function(actual_width, actual_height) {
      var target_width = $(window).width() - irsz_padding[0],
          target_height = $(window).height() - irsz_padding[1],
          new_height = 0,
          new_width = 0,
          w_width, w_height, h_width, h_height;
      
      // do not bother with images that are already smaller than the minima
      if(actual_height < irsz_min_height) { return; }
      if(actual_width < irsz_min_width) { return; }

      function compute_width(height) {
        return Math.round(actual_width * height / actual_height);
      }
      function compute_height(width) {
        return Math.round(actual_height * width / actual_width);
      }
      
      if(/scroll/.test($(image).attr("src"))) {
        // if a scroller, check and fit to *smaller* image dimension
        // this assumes the image is intended to be scrolled in one dimension
        if(actual_width < actual_height) {
          new_width = target_width > irsz_min_width ? target_width : irsz_min_width;
          new_height = compute_height(new_width);
        } else {
          new_height = target_height > irsz_min_height ? target_height : irsz_min_height;
          new_width = compute_width(new_height);
        }
        
        // correct for oversizing
        if(new_width > actual_width || new_height > actual_height) {
          new_width = actual_width;
          new_height = actual_height;
        }
      } else {
        // fit image entirely within viewport
        w_width = target_width > irsz_min_width ? target_width : irsz_min_width;
        w_height = compute_height(w_width);
        h_height = target_height > irsz_min_height ? target_height : irsz_min_height;
        h_width = compute_width(h_height);
        
        // do not enlarge image beyond its limits
        w_width = w_width < actual_width ? w_width : actual_width;
        w_height = w_height < actual_height ? w_height : actual_height;
        h_width = h_width < actual_width ? h_width : actual_width;
        h_height = h_height < actual_height ? h_height : actual_height;

        if(w_height > h_height) {
          // width-based dimensions are too tall
          new_width = h_width;
          new_height = h_height;
        } else {
          // height-based dimensions are too wide/just right
          new_width = w_width;
          new_height = w_height;
        }
      }
      
      // image toggle blocker
      if($(image).hasClass(noresize_class)) {
        if(new_height < actual_height && new_width < actual_width) {
          image.style.cursor = "url(/offensive/graphics/zoom_out.cur),default";
        } else {
          resetcursor(image);
        }
        return;
      }
      
      // resize image
      if(new_height != $(image).height()) {
        if(new_height < actual_height) {
          image.style.cursor = "url(/offensive/graphics/zoom_in.cur),default";
        } else {
          resetcursor(image);
        }
        image_resize(image, new_width, new_height, animate);
      }
    });
  }
  
  function image_resize(image, new_width, new_height, animate) {
    if(animate) {
      $(image).animate({
          width: new_width+"px",
          height: new_height+"px"
      }, 1500 );
    } else {
      image.style.height = new_height+"px";
      image.style.width = new_width+"px";
    }
  }
})();