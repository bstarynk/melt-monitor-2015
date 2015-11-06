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
};

function htmlDecode(value){
  return $('<div/>').html(value).text();
};


function addupdatehtml(txt) {
    console.log("addupdatehtml txt=", txt);
    editlog.append(txt);
};


console.warn("the constructors could be really wrong. FIXME");
var MomcScalar = function (text) {
    this.str = text;
    momc_count = momc_count+1;
    this.num = momc_count;
};
MomcScalar.prototype = {
    name: "momcscalar",
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
    name: "momcnilref",
    font_size: 13,
    font_family: "Verdana, sans-serif",
    font_style: 'italics',
    prototype: MomcScalar
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
    name: "momcnilval",
    font_size: 13,
    font_family: "Verdana, sans-serif",
    font_style: 'bold',
    prototype: MomcScalar
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
    name: "momcitemval",
    font_size: 13,
    font_family: "Arial, sans-serif",
    font_style: 'oblique',
    prototype: MomcScalar
};
function momc_item_val(nam) {
    var res= new MomcItemVal(nam);
    return res;
};

////////////////
var MomcItemRef = function (nam)
{
    MomcItemVal(nam);
};
MomcItemRef.prototype = {
    name: "momcitemref",
    font_size: 13,
    font_family: "Arial, sans-serif",
    font_style: 'oblique',
    prototype: MomcScalar
};
function momc_item_ref(nam) {
    var res = new MomcItemRef(nam);
    return res;
};

////////////////
var MomcInt = function (num) {
    MomcScalar(num.toString());
};
MomcInt.prototype =  {
    name: "momcint",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    prototype: MomcScalar
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
    name: "momcdouble",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    prototype: MomcScalar
};
function momc_double(str) {
    var res= new MomcDouble(str);
    return res;
};

////////////////

var MomcString = function (str) {
    MomcScalar(str);
};
MomcString.prototype = {
    name: "momcstring",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    prototype: MomcScalar
};
function momc_string(str) {
    var res = new MomcString(str);
    return res;
};

////////////////

var MomcSequence = function (arr) {
    this.seq = arr;
    momc_count = momc_count+1;
    this.num = momc_count;
};
MomcSequence.prototype = {
    name: "momcsequence",
    do_layout: function() {
	console.log("csequence-do_layout this=", this);
    }
};

////////////////
var MomcTuple = function (arr) {
    MomcSequence(arr);
};
MomcTuple.prototype = {
    name: "momctuple",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    prototype: MomcSequence
};
function momc_tuple(arr) {
    var res = new MomcTuple(arr);
    return res;
};

////////////////
var MomcSet = function (arr) {
    MomcSequence(arr);
};
MomcSet.prototype = {
    name: "momcset", 
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    prototype: MomcSequence
};

function momc_set(arr) {
    var res= new MomcSet(arr);
    return res;
};

////////////////
var MomcNode = function (conn, arr) {
    MomcSequence(arr);
    this.conn = conn;
};
MomcNode.prototype = {
    name: "momcnode",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain',
    prototype: MomcSequence
};  
function momc_node(conn,sons) {
    var res= new MomcNode(conn,sons);
    return res;
};

////////////////
var MomcTopEntry = function (eattr, eval) {
    this.entattr = eattr;
    this.entval = eval;
    momc_count = momc_count+1;
    this.num = momc_count;
};
MomcTopEntry.prototype = {
    name: "momctopentry",
    font_size: 12,
    font_family: "Arial, sans-serif",
    font_style: 'plain',
    do_layout: function () {
	console.log ("top_entry-do_layout this=", this, " edicanvas=", edicanvas);
	console.trace();
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
    var res =  new MomcTopEntry(attr, val);
    console.log("top_entry res=", res);
    return res;
};

////////////////
function momc_display_canvas(msg,arr) {
    console.log("display_canvas msg=", msg, " arr=", arr, " canvedit=", canvedit);
    canvedit.clearCanvas();
    console.log("display_canvas cleared canvedit=", canvedit);
    canvedit.drawText({
	x: 333, y: 585, fontSize: 10, fontFamily: "Arial",
	text: msg,
	fillStyle: '#BBAAEB'
    });
    console.log("display_canvas drawn msg=", msg);
    canvarr = arr;
    var l = arr.length;
    if (false) for (var i=0; i<l; i++) {
	ob = arr[i];
	console.log ("display_canvas ob=", ob, " i#", i);
	var dim = ob.do_layout();
	console.log ("display_canvas dim=", dim);
    }
};
////////

function ajaxcanvascript(data) {
    console.log("ajaxcanvascript data=", data);
};

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
