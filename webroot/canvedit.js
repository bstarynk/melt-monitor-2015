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
var canvctxt;
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


console.warn("the constructors could be really wrong. FIXME");
var MomcScalar = function (text) {
    this.str = text;
    momc_count = momc_count+1;
    this.num = momc_count;
}
MomcScalar.prototype = {
    font_size: 13,
    font_family: "Verdana, sans-serif",
    font_style: 'plain',
    do_layout: function() {
	console.log("cscalar-do_layout this=", this);
	var dim = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: this.str
	 });
	console.log ("cscalar-do_layout dim=", dim);
	return dim;
    }
};

////////////////
var MomcNilRef = function () {
    MomcScalar("~");
};
MomcNilRef.prototype = {
    font_size: 13,
    font_family: "Verdana, sans-serif",
    font_style: 'italics',
};

function momc_nil_ref() {
    var res = new MomcNilRef();
    return res;
};


////////////////
var MomcNilVal = function () {
    MomcScalar("~");
};
MomcNilVal.prototype = {
    font_size: 13,
    font_family: "Verdana, sans-serif",
    font_style: 'bold',
};

function momc_nil_val() {
    var res = new MomcNilVal();
    return res;
};

////////////////
var MomcItemVal = function (nam)
{
    MomcScalar(nam);
};

MomcItemVal.prototype = {
    name: "momp_item_val",
    font_size: 13,
    font_family: "Arial, sans-serif",
    font_style: 'oblique'
};
function momc_item_val(nam) {
    var res= new MomcItemVal(nam);
    return res;
};

////////////////
var MomcItemRef = function (nam)
{
    MomcItemVal(nam);
}
MomcItemRef.prototype = {
    font_size: 13,
    font_family: "Arial, sans-serif",
    font_style: 'oblique'
};
function momc_item_ref(nam) {
    var res = new MomcItemRef(nam);
    return res;
};

////////////////
var MomcInt = function (num) {
    MomcScalar(num.toString());
}
MomcInt.prototype =  {
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
function momc_int(num) {
    var res = new MomcInt(num);
    return res;
};

////////////////
var MomcDouble = function (str) {
    MomcScalar(str);
    this.num = parseFloat(str);
};
MomcDouble.prototype = {
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
function momc_double(str) {
    var res= new MomcDouble(str);
    return res;
};

////////////////

var MomcString = function (str) {
    MomcScalar(str);
}
MomcString.prototype = {
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
function momc_string(str) {
    var res = new MomcString(str);
    return res;
};

////////////////


var momp_tuple = {
    name: "momp_tuple",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    do_layout: function () {
	console.log ("tuple-do_layout this=", this);
	var dimleft = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: "["
	 });
	console.log ("tuple-do_layout dimleft=", dimleft);
	var dimright = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: "]"
	 });
	console.log ("tuple-do_layout dimright=", dimright);
    }
};
function momc_tuple(arr) {
    momc_count = momc_count + 1;
    var res = Object.create(momp_tuple,
			    {num: momc_count,
			     kind: "tuple",
			     arr: arr});
    return res;
};

////////////////
var momp_set = {
    name: "momp_set",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    do_layout: function () {
	console.log ("set-do_layout this=", this);
	var dimleft = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: "["
	 });
	console.log ("set-do_layout dimleft=", dimleft);
	var dimright = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: "]"
	 });
	console.log ("set-do_layout dimright=", dimright);
    }
};

function momc_set(arr) {
    momc_count = momc_count + 1;
    var res= Object.create(momp_set,
			   {num: momc_count,
			    kind: "set",
			    arr: arr});
    return res;
};

////////////////
var momp_node = {
    name: "momp_node",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    do_layout: function () {
	console.log ("node-do_layout this=", this);
	var dimleft = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: "("
	 });
	console.log ("node-do_layout dimleft=", dimleft);
	var dimright = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: ")"
	 });
	console.log ("node-do_layout dimright=", dimright);
    }
};  
function momc_node(conn,sons) {
    momc_count = momc_count + 1;
    var res= Object.create(momp_node,
			   {num: momc_count,
			    kind: "node",
			    conn: conn,
			    sons: sons});
    return res;
};

////////////////
var momp_top_entry = {
    name: "momp_top_entry",
    font_size: 12,
    font_family: "Arial, sans-serif",
    font_style: 'plain',
    do_layout: function () {
	console.log ("top_entry-do_layout this=", this);
	var dimleft = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: "°"
	 });
	console.log ("top-entry-do_layout dimleft=", dimleft);
	var dimmiddle = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: ":"
	 });
	console.log ("node-do_layout dimmiddle=", dimmiddle);
	this.entattr.do_layout();
	console.log ("node-do_layout after entattr");
	this.entval.do_layout();
	console.log ("node-do_layout after entval");
    }
};  
function momc_top_entry(attr,val) {
    var res =  Object.create(momp_top_entry,
			     {num: momc_count,
			      kind: "top_entry",
			      entattr: attr,
			      entval: val});
    console.log("top_entry res=", res);
    return res;
};

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
