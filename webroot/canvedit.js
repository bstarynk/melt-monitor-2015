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
var canvarr;
var editlog;
var cleareditbut;
var momc_count=0;

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
    console.log("addupdatehtml txt=", txt);
    editlog.append(txt);
}

////////////////
var momp_empty_val = {
    name: "momp_empty_val"
};

function momc_empty_val() {
    momc_count = momc_count + 1;
    var res = {num: momc_count,
		kind: "empty_val",
		str: "",
	       x: false};
    res.__proto__ = momp_empty_val;
    return res;
}

////////////////
var momp_nil_ref = {
    name: "momp_nil_ref"
};

function momc_nil_ref() {
    momc_count = momc_count + 1;
    var res = {num: momc_count,
	       kind: "nil_ref",
	       str: "~"};
    res.__proto__ = momp_nil_ref;
    return res;
};


////////////////
var momp_nil_val = {
    name: "momp_nil_val"
};

function momc_nil_val() {
    momc_count = momc_count + 1;
    var res = {num: momc_count,
		kind: "nil_val",
	       str: "~"};
    res.__proto__ = momp_nil_val;
};

////////////////
var momp_item_val = {
    name: "momp_item_val"
};
function momc_item_val(nam) {
    momc_count = momc_count + 1;
    var res= {num: momc_count,
		kind: "item_val",
	      str: nam};
    res.__proto__ = momp_item_val;
    return res;
};

////////////////
var momp_item_ref = {
    name: "momp_item_ref"
};
function momc_item_ref(nam) {
    momc_count = momc_count + 1;
    var res= {num: momc_count,
	      kind: "item_ref",
	      str: nam};
    res.__proto__ = momp_item_ref;
    return res;
};

////////////////
var momp_int = {
    name: "momp_int"
};
function momc_int(num) {
    momc_count = momc_count + 1;
    var res= {num: momc_count,
		kind: "int",
		str: num.toString(),
	      num: num};
    res.__proto__ = momp_int;
    return res;
};

////////////////
var momp_double = {
    name: "momp_double"
};
function momc_double(str) {
    momc_count = momc_count + 1;
    var res= {num: momc_count,
		kind: "dbl",
		str: str,
		dbl: parseFloat(str)};
    res.__proto__ = momp_int;
    return res;
};

////////////////
var momp_string = {
    name: "momp_string"
};
function momc_string(str) {
    momc_count = momc_count + 1;
    var res = {num: momc_count,
		kind: "str",
		str: str};
    res.__proto__ = momp_int;
    return res;
};

////////////////
var momp_tuple = {
    name: "momp_tuple"
};
function momc_tuple(arr) {
    momc_count = momc_count + 1;
    var res = {num: momc_count,
		kind: "tuple",
		arr: arr};
    res.__proto__ = momp_tuple;
    return res;
};

////////////////
var momp_set = {
    name: "momp_set"
};

function momc_set(arr) {
    momc_count = momc_count + 1;
    var res= {num: momc_count,
		kind: "set",
		arr: arr};
    res.__proto__ = momp_set;
    return res;
};

////////////////
var momp_node = {
    name: "momp_node"
};  
function momc_node(conn,sons) {
    momc_count = momc_count + 1;
    var res= {num: momc_count,
		kind: "node",
		conn: conn,
		sons: sons};
    res.__proto__ = momp_node;
    return res;
};

////////////////
var momp_top_entry = {
    name: "momp_top_entry"
};  
function momc_top_entry(attr,val) {
    var res =  {num: momc_count,
		    kind: "top_entry",
		    eattr: attr,
		eval: val};
    res.__proto__ = momp_top_entry;
    console.log("top_entry res=", res);
    return res;
}

////////////////
function momc_display_canvas(arr) {
    console.log("display_canvas arr=", arr);
    canvarr = arr;
}
////////

function ajaxcanvascript(data) {
    console.log("ajaxcanvascript data=", data);
}

////////


$(document).ready(function(){
    console.log("canvedit document ready");
    edicanvas = $("#canvedit_id");
    editlog = $("#editlog_id");
    cleareditbut = $("#cleareditbut_id");
    console.log("edicanvas=", edicanvas);
    cleareditbut.click(function(evt){
	console.log("clearedit evt=", evt);
	editlog.html("");
    });
    $.ajax
    ({url: "/canvasedit",
      method: "POST",
      data: {"do_fillcanvas": true},
      dataType: "script",
      success: ajaxcanvascript
     });
});

console.log("canvedit.js parsed");
