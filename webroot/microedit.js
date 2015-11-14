// file microedit.js - Javascript & Jquery for microedit.html.

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

var $editdiv;
var $editlog;
var $cleareditbut;

// from http://stackoverflow.com/a/1219983/841108
function htmlEncode(value){
  //create a in-memory div, set it's inner text(which jQuery automatically encodes)
  //then grab the encoded contents back out.  The div never exists on the page.
  return $('<div/>').text(value).html();
}

function htmlDecode(value){
  return $('<div/>').html(value).text();
}


function ajaxfillscript(script) {
    console.log("ajaxfillscript:\n", script, "\n### endajaxfillscript\n");
}

/// see http://stackoverflow.com/q/33540051/841108
////////

function mome_begin_fill(statitmid) {
    console.log("mome_begin_fill statitmid=", statitmid);
};

function mome_nil_val() { /// a nil value
}

function mome_nil_ref() {	/// a nil item reference
}

function mome_item_val(itmname) {
}

function mome_item_ref(itmname) {
}

function mome_int (num) {
}

function mome_double(dbl) {
}

function mome_string(str) {
}

function mome_tuple(tuparr) {
}

function mome_set(setarr) {
}

function mome_node(connitm, sonsarr) {
}

$(document).ready(function(){
    console.log("document ready");
    $editdiv = $("#microedit_id");
    $editlog = $("#editlog_id");
    $cleareditbut = $("#cleareditbut_id");
    $cleareditbut.click(function(evt){
	console.log("clearedit evt=", evt);
	$editlog.html("");
    });
    $.ajax
    ({url: "/microedit",
      method: "POST",
      data: {"do_fillpage": true},
      dataType: "script",
      success: ajaxfillscript
     });
});
