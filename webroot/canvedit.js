// file canvedit.js - Javascript & Jquery for canvedit.html.

/**   Copyright (C)  2015 Basile Starynkevitch, later FSF
      MONIMELT is a monitor for MELT - see http://gcc-melt.org/
      This file is part of GCC.
      
      GCC is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 3, or (at your option)
      any later version.
      
      GCC is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
      You should have received a copy of the GNU General Public License
      along with GCC; see the file COPYING3.   If not see
      <http://www.gnu.org/licenses/>.
**/

var edicanvas;
var editlog;
var cleareditbut;

// from http://stackoverflow.com/a/1219983/841108
function htmlEncode(value){
  //create a in-memory div, set it's inner text(which jQuery automatically encodes)
  //then grab the encoded contents back out.  The div never exists on the page.
  return $('<div/>').text(value).html();
}

function htmlDecode(value){
  return $('<div/>').html(value).text();
}


function addupdatehtml(txt) {
    console.debug("addupdatehtml txt=", txt);
    editlog.append(txt);
}

////////

function ajaxcanvascript(data) {
    console.debug("ajaxcanvascript data=", data);
}

////////


$(document).ready(function(){
    console.debug("document ready");
    edicanvas = $("#canvedit_id");
    editlog = $("#editlog_id");
    cleareditbut = $("#cleareditbut_id");
    console.debug("edicanvas=", edicanvas);
    cleareditbut.click(function(evt){
	console.debug("clearedit evt=", evt);
	editlog.html("");
    });
    $.ajax
    ({url: "/canvedit",
      method: "POST",
      data: {"do_fillcanvas": true},
      dataType: "script",
      success: ajaxcanvascript
     });
});
