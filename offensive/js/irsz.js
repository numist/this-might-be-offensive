/*! ©/info: https://github.com/numist/jslib/blob/master/irsz.js */
(function($) {
  var self,
  methods = {
    init : function(opts) {
      self.data("irsz", $.extend({
        // do not act upon elements with images smaller than:
        min_height: 400,
        min_width: 400,

        // automatically resize images when viewport is resized/on page load
        auto: true,

        // resize image (x, y) smaller than viewport
        padding: [10, 10],

        // custom cursors for when the click action on the image is resize, not the default action.
        cursor_zoom_in: "auto",
        cursor_zoom_out: "auto",

        // keep images zoomed in when they were click-zoomed
        noresize_class: "irsz_noresize"
      }, opts || {}));

      // if images are already loaded, fit them
      apply_resize();

      // if images are not already loaded, attach a function to fire on arrival
      self.load(function() {
        apply_resize();
      });
      
      // bind function to resize as needed during viewport resize
      $(window).bind("resize.irsz", function() {
        apply_resize();
      });
      
      // attach click handler for manual zooming
      self.each(function() {
        $(this).bind("click.irsz", function(e) {
          var target = $(e.target);
          var prevWidth = target.width();
          var percentScrollY = e.offsetY / target.height();
          var percentScrollX = e.offsetX / target.width();
          image_toggle(target.get(0));
          if (prevWidth != target.width()) {
            e.preventDefault();
            if (prevWidth < target.width()) {
              $(window).scrollTop((target.height() * percentScrollY) + target.offset().top - ($(window).height() / 2));
              $(window).scrollLeft((target.width() * percentScrollX) + target.offset().left - ($(window).width() / 2));
            }
          }
          return true;
        });
      });

      return self;
    },
    toggle: function() {
      if(self.data("irsz").auto == undefined) {
        $.error("Must init plugin before calling members in jQuery.irsz");
        return self;
      }
      
      self.each(function() { image_toggle(this); });
      return self;
    },
    destroy: function() {
      if(self.data("irsz").auto == undefined) {
        $.error("Must init plugin before calling members in jQuery.irsz");
        return self;
      }
      
      // unbind event handlers
      $(window).unbind(".irsz");
      self.each(function() {
        $(this).unbind(".irsz");
      });

      // revert image, cursor, etc to normal state
      self.each(function() {
        var image = this;
        image_dimensions(image, function(width, height){
          image_resize(image, width, height);
        })
        // reset the cursor to its normal state
        resetcursor(image);
      });
      
      // destroy settings
      self.data("irsz", {});
      
      return self;
    }
  };
  
  $.fn.irsz = function(method) {
		if(this.length == 0) return this;

    self = this;
    if(methods[method]) {
      return methods[method].apply(self, Array.prototype.slice.call( arguments, 1 ));
    } else if(typeof method === 'object' || ! method) {
      return methods.init.apply(self, arguments);
    } else {
      $.error('Method ' +  method + ' does not exist on jQuery.irsz');
    }
  };

  // private utility functions:

  // get an image's actual dimensions
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
  
  // manipulate the user's cursor
  function resetcursor(image) { setcursor(image, "auto"); }
  function setcursor(image, style) { image.style.cursor = style; }
  
  // zoom image in/out
  function image_toggle(image) {
    image_dimensions(image, function(actual_width, actual_height) {
      // check both dimensions in case there's a bug elsewhere we're resetting
      if($(image).width() < actual_width || $(image).height < actual_height) {
        $(image).addClass(self.data("irsz").noresize_class);
        setcursor(image, self.data("irsz").cursor_zoom_out);
        image_resize(image, actual_width, actual_height);
      } else {
        $(image).removeClass(self.data("irsz").noresize_class);
        image_fit(image);
      }
    });
  }
  
  // fit image to the viewport
  function image_fit(image) {
    image_dimensions(image, function(actual_width, actual_height) {
      var aspect_ratio = actual_width > actual_height ? actual_width / actual_height : actual_height / actual_width,
          target_width = $(window).width() - self.data("irsz").padding[0],
          target_height = $(window).height() - self.data("irsz").padding[1],
          new_height = 0,
          new_width = 0,
          w_width, w_height, h_width, h_height;
      
      // do not bother with images that are already smaller than the minima
      if(actual_height < self.data("irsz").min_height) { return; }
      if(actual_width < self.data("irsz").min_width) { return; }
  
      function compute_width(height) {
        return Math.round(actual_width * height / actual_height);
      }
      function compute_height(width) {
        return Math.round(actual_height * width / actual_width);
      }
      
      // fit image entirely within viewport
      w_width = target_width > self.data("irsz").min_width ? target_width : self.data("irsz").min_width;
      w_height = compute_height(w_width);
      h_height = target_height > self.data("irsz").min_height ? target_height : self.data("irsz").min_height;
      h_width = compute_width(h_height);
      
      // do not enlarge image beyond its limits
      w_width = w_width < actual_width ? w_width : actual_width;
      w_height = w_height < actual_height ? w_height : actual_height;
      h_width = h_width < actual_width ? h_width : actual_width;
      h_height = h_height < actual_height ? h_height : actual_height;
      
      if(/scroll/.test($(image).attr("src")) || aspect_ratio > 3) {
        // if a scroller, check and fit one dimension
        // this assumes the image is intended to be scrolled in one dimension
        if(w_height < h_height) {
          new_width = h_width;
          new_height = h_height;
        } else {
          new_width = w_width;
          new_height = w_height;
        }
      } else {
        // fit image entirely in viewport
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
      if($(image).hasClass(self.data("irsz").noresize_class)) {
        if(new_height < actual_height && new_width < actual_width) {
          setcursor(image, self.data("irsz").cursor_zoom_out);
        } else {
          resetcursor(image);
        }
        return;
      }
      
      // resize image
      if(new_height != $(image).height()) {
        if(new_height < actual_height) {
          setcursor(image, self.data("irsz").cursor_zoom_in);
        } else {
          resetcursor(image);
        }
        image_resize(image, new_width, new_height);
      }
      return;
    });
  }
  
  function image_resize(image, new_width, new_height) {
    var image = $(image), old_width = image.width(), old_height = image.height();
    image.height(new_height+"px");
    image.width(new_width+"px");

    // propagate an event to support add-on features
    if(new_width != old_width || new_height != old_height) {
      image.resize();
    }
    return;
  }
  
  function apply_resize() {
    if(self.data("irsz").auto == true) {
      self.each(function() {
        image_fit(this);
      });
    }
  }
})(jQuery);