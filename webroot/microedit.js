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

var editdiv;
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

function inputev(evt) {
    console.debug("inputev evt=", evt);
}

function ignore_keypressev(evt) {
    console.debug("ignore_keypressev evt=", evt);
    //evt.stopImmediatePropagation();
    //evt.preventDefault();
    return false;
}

function changev(evt) {
    console.debug("changev evt=", evt);
}

function focusev(evt) {
    console.debug("focusev evt=", evt);
}

function ignore_pastev(evt) {
    console.debug("ignore_pastev evt=", evt);
    return false;
}

function ignore_cutev(evt) {
    console.debug("ignore_cutev evt=", evt);
    return false;
}

function ajaxload(data) {
    console.debug("ajaxload data=",data);
    editdiv.html(data);
}

$(document).ready(function(){
    console.debug("document ready");
    editdiv = $("#microedit_id");
    editlog = $("#editlog_id");
    cleareditbut = $("#cleareditbut_id");
    editdiv.on("input",inputev);
    editdiv.keypress(ignore_keypressev);
    editdiv.change(changev);
    editdiv.focus(focusev);
    editdiv.on("cut",ignore_cutev);
    editdiv.on("paste",ignore_pastev);
    cleareditbut.click(function(evt){
	console.debug("clearedit evt=", evt);
	editlog.html("");
    });
    $.ajax
    ({url: "/microedit",
      method: "POST",
      data: {"do_loadpage": true},
      dataType: "html",
      success: ajaxload
     });
});
