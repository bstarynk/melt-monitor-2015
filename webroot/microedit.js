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

/// see http://stackoverflow.com/q/33540051/841108
function ignore_keypressev(evt) {
    console.log("ignore_keypressev evt=", evt, " evt.target=", evt.target);
    //evt.stopImmediatePropagation();
    //evt.preventDefault();
    return false;
}

function ignore_keydownev(evt) {
    console.log("ignore_keydownev evt=", evt);
    return false;
}

function changev(evt) {
    console.log("changev evt=", evt);
}

function focusev(evt) {
    console.log("focusev evt=", evt);
}

function blurev(evt) {
    console.log("blurev evt=", evt);
}

function ignore_pastev(evt) {
    console.log("ignore_pastev evt=", evt);
    return false;
}

function ignore_cutev(evt) {
    console.log("ignore_cutev evt=", evt);
    return false;
}

function name_keypressev(evt) {
    console.log("name_keypressev evt=", evt);
}

function name_focusev(evt) {
    console.log("name_focusev evt=", evt);
}

function name_blurev(evt) {
    console.log("name_blurev evt=", evt);
}


////////

function ajaxload(data) {
    console.log("ajaxload data=",data);
    editdiv.html(data);
    console.log("ajaxload editdiv=",editdiv);
    $("#microedit_id .momitemref_cl").each(function() {
	console.log ("each itemref this=", $(this).html(), ": $(this)=", $(this), " this=", this);
	//$(this).on("keypress",name_keypressev);
	//$(this).on("focus",name_focusev);
    });
    $("#microedit_id .momitemval_cl").each(function() {
	console.log ("each itemval this=", $(this).html(), ": $(this)=", $(this), " this=", this);
	//$(this).on("keypress",name_keypressev);
	//$(this).on("focus",name_focusev);
    });
    editdiv.on("keypress",".momname_bcl",name_keypressev);
    editdiv.on("focus",".momname_bcl",name_focusev);
    editdiv.on("focus",".momname_bcl",name_blurev);
    editdiv.on("keypress",":not(.momname_bcl)",ignore_keypressev);
    editdiv.on("keydown",":not(.momname_bcl)",ignore_keydownev);
    console.log("ajaxload done editdiv=", editdiv);
}

////////


$(document).ready(function(){
    console.log("document ready");
    editdiv = $("#microedit_id");
    editlog = $("#editlog_id");
    cleareditbut = $("#cleareditbut_id");
    editdiv.on("change",changev);
    editdiv.on("focus",focusev);
    editdiv.on("blur",blurev);
    editdiv.on("cut",ignore_cutev);
    editdiv.on("paste",ignore_pastev);
    cleareditbut.click(function(evt){
	console.log("clearedit evt=", evt);
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
