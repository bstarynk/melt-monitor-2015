// file nanoedit.js - Javascript & Jquery for nanoedit.html.

/**   Copyright (C)  2016 Basile Starynkevitch, later FSF
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

var $editdiv;
var $editlog;
var $cleareditbut;
var $rawmodebox;

////////////////////////////////////////////////////////////////
///// utility functions

// from http://stackoverflow.com/a/1219983/841108
function htmlEncode(value){
    //create a in-memory div, set it's inner text(which jQuery automatically encodes)
    //then grab the encoded contents back out.  The div never exists on the page.
    return $('<div/>').text(value).html();
}

function htmlDecode(value){
    return $('<div/>').html(value).text();
}

var mom_name_regexp = /^[A-Za-z0-9_]*$/;


var mom_name_cache = {};

/// see http://mikemurko.com/general/jquery-keycode-cheatsheet/


/// check if a name is some known item, with memoization & AJAX do_knownitem
function mom_known_item(name) {
    if (!name.match(mom_name_regexp))
        return false;
    if (mom_name_cache[name])
        return true;
    var res = null;
    console.log("mom_known_item name=", name, " before ajaxing");
    $.ajax
    ({url: "/nanoedit",
      method: "POST",
      async: false,
      data: {"do_knownitem": name},
      dataType: "json",
      success: function (data, stat, jh) {
          console.log("mom_known_item success data=", data, " stat=", stat, " jh=", jh);
          if (data) {
              mom_known_item[name] = data;
              res = true;
          }
          else res = false;
      },
      error: function (jq, stat, err) {
          console.warn("mom_known_item error jq=", jq, " stat=", stat, " err=", err);
          res = false;
      }
     });
    console.log ("mom_known_item name=", name, " res=", res);
    return res;
}                              // end mom_known_item

//// give the completion of some name string, using AJAX do_completename
function mom_complete_name(name) {
    var res = null;
    if (!name.match(mom_name_regexp)) {
        console.log ("mom_complete_name bad name=", name);
        return false;
    }
    else 
        console.log("mom_complete_name name=", name, " before ajaxing");
    $.ajax
    ({url: "/nanoedit",
      method: "POST",
      async: false,
      data: {"do_completename": name},
      dataType: "json",
      success: function (data) {
          console.log("mom_complete_name success data=", data);
          console.trace();
          res = data;
      },
      error: function (jq, stat, err) {
          console.warn("mom_complete_name error jq=", jq, " stat=", stat, " err=", err);
          res = false;
      }
     });
    console.log ("mom_complete_name name=", name, " res=", res);
    return res;
}                              // end mom_complete_name


/// this function is called by /microedit AJAX for do_fillpage at document loading
function mom_ajaxfill(htmlc) {
    console.log("mom_ajaxfill:\n", htmlc, "\n### endajaxfill\n");
    console.trace();
    $editdiv.html(htmlc);
    $rawmodebox = $("#momrawbox_id");
    $rawmodebox.on("change", function() {
	var valmode = $rawmodebox.val();
	var valchecked = $rawmodebox.prop("checked");
	console.log("mom_ajaxfill valmode=", valmode,
		    " valchecked=", valchecked, " $rawmodebox=", $rawmodebox);
	$.ajax
	({url: "/nanoedit",
	  method: "POST",
	  data: {"do_fillpage": true, "rawmode", valchecked},
	  dataType: "html",
	  success: mom_ajaxfill
	 });
    });
}


$(document).ready(function(){
    console.log("nanoedit document ready");
    // see http://stackoverflow.com/a/10556743/841108
    window.onerror = function(msg,url,line,col,error) {
        console.error("window error ", url, ":", line, ": ", msg, " /", error);
    };
    $editdiv = $("#nanoedit_id");
    $editlog = $("#editlog_id");
    $cleareditbut = $("#cleareditbut_id");
    console.log ("nanoedit readying $editdiv=", $editdiv, " $editlog=", $editlog, " $cleareditbut=", $cleareditbut);
    $cleareditbut.click(function(evt){
        console.log("clearedit evt=", evt);
        $editlog.html("");
    });
    console.log("nanoedit before ajax do_fillpage");
    $.ajax
    ({url: "/nanoedit",
      method: "POST",
      data: {"do_fillpage": true, "rawmode": true},
      dataType: "html",
      success: mom_ajaxfill
     });
    console.log("nanoedit document done ready");
});


// end of file nanoedit.js
