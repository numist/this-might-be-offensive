/*****************************************************************************
  * jQuery doTimeout: Like setTimeout, but better! - v1.0 - 3/3/2010
  * http://benalman.com/projects/jquery-dotimeout-plugin/
  * 
  * Copyright (c) 2010 "Cowboy" Ben Alman
  * Dual licensed under the MIT and GPL licenses.
  * http://benalman.com/about/license/
  */
 (function($){var a={},c="doTimeout",d=Array.prototype.slice;$[c]=function(){return b.apply(window,[0].concat(d.call(arguments)))};$.fn[c]=function(){var f=d.call(arguments),e=b.apply(this,[c+f[0]].concat(f));return typeof f[0]==="number"||typeof f[1]==="number"?this:e};function b(l){var m=this,h,k={},g=l?$.fn:$,n=arguments,i=4,f=n[1],j=n[2],p=n[3];if(typeof f!=="string"){i--;f=l=0;j=n[1];p=n[2]}if(l){h=m.eq(0);h.data(l,k=h.data(l)||{})}else{if(f){k=a[f]||(a[f]={})}}k.id&&clearTimeout(k.id);delete k.id;function e(){if(l){h.removeData(l)}else{if(f){delete a[f]}}}function o(){k.id=setTimeout(function(){k.fn()},j)}if(p){k.fn=function(q){if(typeof p==="string"){p=g[p]}p.apply(m,d.call(n,i))===true&&!q?o():e()};o()}else{if(k.fn){j===undefined?e():k.fn(j===false);return true}else{e()}}}})(jQuery);
/*****************************************************************************
 * Original file: infScr.js at https://github.com/numist/jslib/
 * Released under the MIT License; see link above.
 */

// loading feedback used when XHR is active
var infScrLoadingFeedback = 'loading…';

(function(){
  // infScrStates = states of the InfScr system:
  var infScrStates = {
    idle: 0,    // (-> loading)
    loading: 1  // (-> idle)
  };
  
  // infScr = current state of infinite scroll
  var infScrState = infScrStates.idle;
  
  // first page in range of pages shown, set once.
  var infScrUrlBasePage = null;
  
  var hasMore = true;
  
  // worker function
  function infScrExecute() {
    /*
     * get more content if:
     * • not already loading new content
     * • viewport is less than one $(window).height() from bottom of document.
     *   see: http://www.tbray.org/ongoing/When/201x/2011/11/26/Misscrolling
     */
    
    var moreNode = $('p#morelink').last();
    if(moreNode.length == 0) {
      return;
    }
     
    if(infScrState == infScrStates.idle
    && ($(document).height() < $(document).scrollTop() + (2 * $(window).height())
       || moreNode.offset().top < $(window).scrollTop() + $(window).height()))
    {
      // block potentially concurrent requests
      infScrState = infScrStates.loading;
  
      // get next page's URL
      var moreURL = moreNode.find('a').last().attr("href");
  
      // make request if node was found, not hidden, and updatepath is supported
      if(moreURL.length > 0 && moreNode.css('display') != 'none')
      {
        $.ajax({
          type: 'GET',
          url: moreURL,
          dataType: "html",
          beforeSend: function() {
            // display loading feedback
            moreNode.clone().empty().insertBefore(moreNode).append(infScrLoadingFeedback);

            // hide 'more' browser
            moreNode.hide();
          },
          success: function(data) {
            // use nodetype to grab elements
            var filteredData = $(data).find("li[fileid]");

            if(filteredData.length > 0) {
              // append unique items (determined by fileid=xxxx)
              $.each(filteredData, function(i, item) {
                if($('li[fileid="'+$(item).attr("fileid")+'"]').length == 0) {
                  if(typeof prep_item == "function") {
                    // if needed, prep the item (for css)
                    item = prep_item(item, $('li[fileid]').last());
                  }
                  $('#grid-container ul').append(item);
                }
              });
              
              // update moreNode with url from data
              moreNode.find('a').last().attr("href", $(data).find('p#morelink').last().find('a').last().attr("href"));
            } else if($(data).find('#grid-container ul').length > 0) {
              // make sure the error exists before assuming no elements—this could be an error page
              hasMore = false;
            }
          },
          complete: function(jqXHR, textStatus) {
            // remove loading feedback
            moreNode.prev().remove();

            if(hasMore) {
              moreNode.show();
            } else {
              moreNode.remove();
            }
  
            infScrState = infScrStates.idle;
          }
        });
      }
    }
  }
  
  $(document).ready(function () {
    // http://unscriptable.com/index.php/2009/03/20/debouncing-javascript-methods/
    // http://paulirish.com/2009/throttled-smartresize-jquery-event-handler/
    $(window).scroll(function() {
      $.doTimeout('scroll', 200 /* milliseconds */, infScrExecute);
    });
  });
})();