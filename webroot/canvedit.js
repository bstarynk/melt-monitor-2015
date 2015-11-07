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


var momp_scalar ={ 
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
console.log("momp_scalar=", momp_scalar);
var MomcScalar = function (text) {
    this.str = text;
    momc_count = momc_count+1;
    this.inum = momc_count;
    console.log ("MomcScalar this=", this);
};
MomcScalar.prototype = momp_scalar;

////////////////
var momp_nil_ref = {
    name: "momcnilref",
    font_size: 13,
    font_family: "Verdana, sans-serif",
    font_style: "italics",
};
momp_nil_ref.prototype= momp_scalar;
console.log("momp_nil_ref=", momp_nil_ref);
var MomcNilRef = function () {
    this.str = "~";
    momc_count = momc_count+1;
    this.inum = momc_count;
    console.log ("MomcNilRef this=", this);
};
MomcNilRef.prototype = momp_nil_ref;

function momc_nil_ref() {
    var res = new MomcNilRef();
    console.log ("momc_nil_ref res=", res); 
    return res;
};


////////////////
var momp_nil_val = {
    name: "momcnilval",
    font_size: 13,
    font_family: "Verdana, sans-serif",
    font_style: 'bold'
};
momp_nil_val.__proto__ = momp_scalar;
console.log ("momp_nil_val=", momp_nil_val);
var MomcNilVal = function () {
    this.str = "~";
    momc_count = momc_count+1;
    this.inum = momc_count;
    //this.__proto__ = momp_nil_val;
    console.log ("MomcNilVal this=", this);
};
MomcNilVal.prototype = momp_nil_val;

function momc_nil_val() {
    var res = new MomcNilVal();
    console.log ("momc_nil_val res=", res);    
    return res;
};

////////////////
var momp_item_val = {
    name: "momcitemval",
    font_size: 13,
    font_family: "Arial, sans-serif",
    font_style: 'oblique'
};
momp_item_val.prototype = momp_scalar;
console.log ("momp_item_val=", momp_item_val);
var MomcItemVal = function (nam)
{
    this.str = "~";
    momc_count = momc_count+1;
    this.inum = momc_count;
    console.log ("MomcNilVal this=", this);
};
MomcItemVal.prototype = momp_item_val;
function momc_item_val(nam) {
    var res= new MomcItemVal(nam);
    console.log ("momc_item_val res=", res);
    return res;
};

////////////////
var momp_item_ref = {
    name: "momcitemref",
    font_size: 13,
    font_family: "Arial, sans-serif",
    font_style: 'oblique'
};
momp_item_ref.__proto__ = momp_scalar;
console.log ("momp_item_ref=", momp_item_ref);
var MomcItemRef = function (nam)
{
    this.str = nam;
    momc_count = momc_count+1;
    this.inum = momc_count;
    console.log ("MomcItemRef this=", this);
};
MomcItemRef.prototype = momp_item_ref;
function momc_item_ref(nam) {
    var res = new MomcItemRef(nam);
    console.log ("momc_item_ref res=", res);
    return res;
};

////////////////
var momp_int = {
    name: "momcint",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
momp_int.__proto__ = momp_scalar;
console.log ("momp_int=", momp_int);
var MomcInt = function (num) {
    this.str = num.toString();
    momc_count = momc_count+1;
    this.inum = momc_count;
    this.number = num;
    console.log ("MomcInt this=", this);
};
MomcInt.prototype = momp_int;
function momc_int(num) {
    var res = new MomcInt(num);
    console.log ("momc_int res=", res);
    return res;
};

////////////////
var momp_double = {
    name: "momcdouble",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
momp_double.__proto__ = momp_scalar;
console.log ("momp_double=", momp_double);
var MomcDouble = function (str) {
    this.str = str;
    momc_count = momc_count+1;
    this.inum = momc_count;
    this.number = str.toDouble();
    console.log ("MomcDouble this=", this);
};
MomcDouble.prototype = momp_double;
function momc_double(str) {
    var res= new MomcDouble(str);
    console.log ("momc_double res=", res);
    return res;
};

////////////////
var momp_string = {
    name: "momcstring",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
momp_string.__proto__ = momp_scalar;
console.log ("momp_string=", momp_string);
var MomcString = function (str) {
    this.str = str;
    momc_count = momc_count+1;
    this.inum = momc_count;
    console.log ("MomcString this=", this);
};
MomcString.prototype = momp_string;

function momc_string(str) {
    var res = new MomcString(str);
    console.log ("momc_string res=", res);
    return res;
};

////////////////
var momp_sequence = {
    name: "momcsequence",
    do_layout: function () {
	console.log("csequence-do_layout this=", this);
    }
};
console.log ("momp_sequence=", momp_sequence);

////////////////
var momp_tuple = {
    name: "momctuple",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
momp_tuple.__proto__= momp_sequence;
console.log ("momp_tuple=", momp_tuple);
var MomcTuple = function (arr) {
    this.arr = arr;
    momc_count = momc_count+1;
    this.inum = momc_count;
    console.log ("MomcTuple this=", this);
};
MomcTuple.prototype = momp_tuple;
function momc_tuple(arr) {
    var res = new MomcTuple(arr);
    console.log ("momc_tuple res=", res);
    return res;
};

////////////////
var momp_set = {
    name: "momcset",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: 'plain'
};
momp_set.__proto__= momp_sequence;
console.log ("momp_set=", momp_set);
var MomcSet = function (arr) {
    this.arr = arr;
    momc_count = momc_count+1;
    this.inum = momc_count;
    console.log ("MomcSet this=", this);
};
MomcSet.prototype = momp_set;
function momc_set(arr) {
    var res= new MomcSet(arr);
    console.log ("momc_set res=", res);
    return res;
};

////////////////
var momp_node = {
    name: "momcnode",
    font_size: 12,
    font_family: "Courier, Lucida",
    font_style: "plain",
    do_layout: function() {
	console.log("cnode-do_layout this=", this);
    }
};
console.log ("momp_node=", momp_node);
var MomcNode = function (conn, arr) {
    this.conn = conn;
    this.arr = arr;
    this.inum = momc_count;
    console.log ("MomcNode this=", this);
};
MomcNode.prototype = momp_node;
function momc_node(conn,sons) {
    var res= new MomcNode(conn,sons);
    console.log ("momc_node res=", res);
    return res;
};

////////////////
var momp_top_entry = {
    name: "momctopentry",
    font_size: 12,
    font_family: "Arial, sans-serif",
    font_style: 'plain',
    do_layout: function () {
	console.log ("top_entry-do_layout this=", this,
		     " edicanvas=", edicanvas);
	var dimdegree = edicanvas.measureText
	({fontSize: font_size,
	  fontFamily: font_family,
	  fontStyle: font_style,
	  text: "°"
	 });
	console.log ("top_entry-do_layout dimdegree=", dimdegree);
    }
};
console.log ("momp_top_entry=", momp_top_entry);
var MomcTopEntry = function (eattr, eval) {
    this.entattr = eattr;
    this.entval = eval;
    momc_count = momc_count+1;
    this.inum = momc_count;
//    this.__proto__ = momp_top_entry;
    console.log ("MomcTopEntry this=", this);
};
MomcTopEntry.prototype = momp_top_entry;

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
    if (false)
	for (var i=0; i<l; i++) {
	    ob = arr[i];
	    console.log ("display_canvas ob=", ob, " i#", i);
	    var dim = ob.do_layout();
	    console.log ("display_canvas dim=", dim);
	};
    console.log("display_canvas end msg=", msg);    
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
