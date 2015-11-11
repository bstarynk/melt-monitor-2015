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

var rawcanvas;
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
    return $('<div/>').html(value).text();};


function addupdatehtml(txt) {
    console.log("addupdatehtml txt=", txt);
    editlog.append(txt);
};

/// nearly copied from https://github.com/danielearwicker/carota/blob/master/src/text.js
/// in Carota, which has an MIT license, by Daniel Earwicker.
var nbsp = String.fromCharCode(160);
/*  Returns width, height, ascent, descent in pixels for the specified text and font.
    The ascent and descent are measured from the baseline. Note that we add/remove
    all the DOM elements used for a measurement each time - this is not a significant
    part of the cost, and if we left the hidden measuring node in the DOM then it
    would affect the dimensions of the whole page.
*/
var dimproto = {
    toString: function ()
    { return "txtdim{h=" + this.height.toString() + ",w=" + this.width.toString()
      + ",asc=" + this.ascent.toString() + ",desc=" + this.descent.toString() + "}"; }
};
var measureText = function(text, styleprops)
{
    //console.log ("measureText text=", text, " styleprops=", styleprops);
    var span, block, div;

    span = document.createElement('span');
    block = document.createElement('div');
    div = document.createElement('div');

    block.style.display = 'inline-block';
    block.style.width = '1px';
    block.style.height = '0';

    div.style.visibility = 'hidden';
    div.style.position = 'absolute';
    div.style.top = '0';
    div.style.left = '0';
    div.style.width = '550px';
    div.style.height = '300px';

    div.appendChild(span);
    div.appendChild(block);
    document.body.appendChild(div);
    try {
	for (var p in styleprops) {
	    if (styleprops.hasOwnProperty(p) && styleprops[p]) {
		//console.log("measureText p=", p, " : ", styleprops[p]);
		span.style[p] = styleprops[p];
	    }
	}
	//console.log("measureText span=", span, " spanfont=", span.getAttribute("font"),
	//	    " spanstyle=", span.style);
        span.innerHTML = '';
        span.appendChild(document.createTextNode(text.replace(/\s/g, nbsp)));

        var result = {};
        block.style.verticalAlign = 'baseline';
        result.ascent = (block.offsetTop - span.offsetTop);
        block.style.verticalAlign = 'bottom';
        result.height = (block.offsetTop - span.offsetTop);
        result.descent = result.height - result.ascent;
        result.width = span.offsetWidth;
	result.__proto__ = dimproto;
    } catch (exc) {
	console.warn ("measureText error text=", text, " styleprops=", styleprops, " exc=", exc);
    } finally {
        div.parentNode.removeChild(div);
        div = null;
    }
    //console.log ("measureText text=", text, " result=", result);
    return result;
};

////////////////

var momp_scalar ={ 
    name: "momcscalar",
    my_font_size: '13pt',
    my_font_family: "Verdana, sans-serif",
    //my_font_style: 'plain',
    my_color: '#0B372C',
    get_dim: function(hints) {
	if (this.dim) {
	    console.log("cscalar-get_dim already layout this=", this);
	    return this.dim;
	}
	console.log("cscalar-get_dim this=", this);
	var dim = false;
	var dimstr = measureText(this.str,
				 {fontFamily: this.my_font_family,
				  fontSize: this.my_font_size,
				  fontStyle: this.my_font_style});
	console.log ("cscalar-get_dim dimstr=", dimstr);
	var decostyleprop = {fontFamily: this.my_decofont_family || this.my_font_family,
			     fontSize: this.my_decofont_size || this.my_font_size,
			     fontStyle: this.my_decofont_style || this.my_font_style};
	var dimleft = {ascent:0, descent:0, height:0, width:0};
	if (this.my_decoleft)
	    dimleft = measureText(this.my_decoleft, decostyleprop);
	console.log ("cscalar-get_dim dimleft=", dimleft);
	var dimright = {ascent:0, descent:0, height:0, width:0};
	if (this.my_decoright)
	    dimright = measureText(this.my_decoright, decostyleprop);
	console.log ("cscalar-get_dim dimright=", dimleft);
	dim = { ascent: Math.max(dimstr.ascent,dimleft.ascent,dimright.ascent),
		descent: Math.max(dimstr.descent,dimleft.descent,dimright.descent),
		width: dimstr.width+dimleft.width+dimright.width,
		height: Math.max(dimstr.height,dimleft.height,dimright.height)
	      };
	dim.__proto__ = dimproto;
	console.log("cscalar-get_dim dim=", dim, " from dimstr=", dimstr,
		    " dimleft=", dimleft, " dimright=", dimright);
	this.dim = dim;
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
    str: "~",
    name: "momcnilref",
    my_font_size: '13pt',
    my_font_family: "Verdana, sans-serif",
    my_font_style: "italics",
    my_color: '#0B1E37'
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
    str: "~",
    name: "momcnilval",
    my_font_size: '13pt',
    my_font_family: "Verdana, sans-serif",
    my_font_style: 'bold',
    my_color: '#0D145B'
};
momp_nil_val.__proto__ = momp_scalar;
console.log ("momp_nil_val=", momp_nil_val);
var MomcNilVal = function () {
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
    my_font_size: '13pt',
    my_font_family: "Arial, sans-serif",
    my_font_style: 'oblique',
    my_color: '#330A4F'
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
    my_font_size: '13pt',
    my_font_family: "Arial, sans-serif",
    my_font_style: 'oblique',
    my_color: '#862769'
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
    my_font_size: '12pt',
    my_font_family: "Courier, Lucida",
    //my_font_style: 'plain',
    my_color: '#5B1D01'
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
    my_font_size: '12pt',
    my_font_family: "Courier, Lucida",
    //    my_font_style: 'plain',
    my_color: '#7C480B'
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
    my_font_size: '12pt',
    my_font_family: "Courier, Lucida",
    //my_font_style: 'plain',
    my_color: '#074723',
    my_decofont_family: 'Helvetica, sans-serif',
    my_decoleft:'\u201c', // “ U+201C LEFT DOUBLE QUOTATION MARK 
    my_decoright: '\u201d', // ” U+201D RIGHT DOUBLE QUOTATION MARK
    my_decocolor: '#093588'
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
var momp_horizgroup = {
    name: "momchorizgroup",
    hgap: 1,
    vgap: 0,
    initialx: 0,
    get_dim: function (hints) {
	var len = this.arr.length;
	var hgap = this.hgap;
	var dimarr = new Array(len);
	var w= 0, h=0; a=0, d=0;
	var curx=hints.initialx?hints.initialx:this.initialx;
	var vgap=hints.vgap?hints.vgap:this.vgap;
	this.offx = curx;
	for (var ix=0; ix<dimarr; ix++) {
	    var comp = this.arr[ix];
	    var curdim = comp.get_dim(hints);
	    w = w+hgap+curdim.width;
	    h = Math.max(h,curdim.height);
	    a = Math.max(a,curdim.ascent);
	    d = Math.max(d,curdim.descent);
	    comp.offx = curx;
	    curx += w+hgap;
	    dimarr[ix] = curdim;
	}
	for (var ix=0; ix<dimarr; ix++) {
	    var comp = this.arr[ix];
	    comp.offy = vgap+a;
	}
	var dim= {width: w, height: h, ascent: a, descent: d, __proto__: dimproto};
	this.dim = dim;
	return dim;
    }
};
var MomcHorizGroup = function (arr) {
    this.arr = arr;
    console.log ("MomcHorizGroup this=", this);
};
MomcHorizGroup.prototype = momp_horizgroup;

function momc_horizgroup(arr) {
    var res = new MomcHorizGroup(arr);
    return res;
};


////////////////
var momp_horizlayout = {
    name: "momchorizlayout",
    get_dim: function (hints) {
	console.log ("chorizlayout-get_dim this=", this, " hints=", hints);
	var arr = this.arr;
	var len = arr.length;
	var before = this.before;
	var after = this.after;
	var beforedim = before.get_dim(hints);
	console.log("chorizlayout-get_dim beforedim=", beforedim);
	var afterdim = after.get_dim(hints);
	console.log("chorizlayout-get_dim afterdim=", afterdim);
	console.warn("chorizlayout-get_dim @@unimplemented arr=", arr);
    }
};
var MomcHorizLayout = function (before,arr,after) {
    this.before = before;
    this.arr = arr;
    this.after = after;
};
MomcHorizLayout.prototype = momp_horizlayout;
function momc_horizlayout(before,arr,after) {
    var res = new MomcHorizLayout(before,arr,after);
    return res;
}

////////////////
var momp_sequence = {
    name: "momcsequence",
    get_dim: function (hints) {
	var len = this.arr.length;
	var putdimarr = new Array(len);
	var rowarr = new Array();
	var curow = null;
	if (len>0) {
	    curow = {rowix: 0, rowcomp: new Array()};
	    rowarr.push(curow);
	}
	
	for (var ix=0; ix<len; ix++) {
	    var curcomp = this.arr[ix];
	    console.log ("csequence-get_dim curcomp=", curcomp, " ix#", ix);
	    var curdim = curcomp.get_dim(hints);
	    console.log ("csequence-get_dim curdim=", curdim,
			 " for curcomp=", curcomp, " ix#", ix);
	    putdimarr[ix] = curdim;
	}
	console.trace("csequence-get_dim putdimarr=", putdimarr);
	console.warn("csequence-get_dim @@unimplemented this=", this, " hints=", hints);
    }
};
console.log ("momp_sequence=", momp_sequence);

////////////////
var momp_tuple = {
    name: "momctuple",
    my_font_size: '12pt',
    my_font_family: "Courier, Lucida",
    my_deco_before_str: "[",
    my_deco_after_str: "]", 
    my_decofont_family: "Verdana, sans-serif",
    my_decofont_size: '13pt',
    my_hgap: 7,
    my_vgap: 3
    //my_font_style: 'plain'
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
    my_font_size: '12pt',
    my_font_family: "Courier, Lucida",
    my_decofont_size: '13pt',
    my_deco_before_str: "{",
    my_deco_after_str: "}", 
    my_decofont_family: "Verdana, sans-serif",
    my_hgap: 5,
    my_vgap: 3
    //my_font_style: 'plain'
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
    my_font_size: '12pt',
    my_font_family: "Courier, Lucida",
    my_decofont_family: "Verdana, sans-serif",
    my_deco_beforeconn_str: "*",
    my_deco_beforesons_str: "(",
    my_deco_aftersons_str: ")",
    //my_font_style: "plain",
    get_dim: function(hints) {
	if (this.dim) {
	    console.log ("cnode-get_dim with dim this=",this);
	    return this.dim;
	}
	console.log ("cnode-get_dim this=", this, " hints=", hints);
	var decostyleprop = {fontFamily: this.my_decofont_family || this.my_font_family,
			     fontSize: this.my_decofont_size || this.my_font_size,
			     fontStyle: this.my_decofont_style || this.my_font_style || null};
	var dimbeforeconn = measureText(this.my_deco_beforeconn_str, decostyleprop);
	console.log ("cnode-get_dim dimbeforeconn=", dimbeforeconn);
	var oldhints = hints;
	var connhints = $.extend(oldhints,
				 {maxheight: oldhints.maxheight,
				  maxwidth: oldhints.maxwidth-dimbeforeconn.width});
	console.log ("cnode-get_dim connhints=", connhints);
	var dimconn = this.conn.get_dim(connhints);
	console.log ("cnode-get_dim dimconn=", dimconn);
	var dimbeforesons = measureText(this.my_deco_beforesons_str, decostyleprop);
	console.log ("cnode-get_dim dimbeforesons=", dimbeforesons);
	var dimaftersons = measureText(this.my_deco_aftersons_str, decostyleprop);
	console.log ("cnode-get_dim dimaftersons=", dimaftersons);
	var dimfullconn
	    = {ascent: Math.max(dimbeforeconn.ascent, dimconn.ascent+1, dimbeforesons.ascent),
	       descent: Math.max(dimbeforeconn.descent, dimconn.descent+1, dimbeforesons.descent),
	       height: Math.max(dimbeforeconn.height, dimconn.height+1, dimbeforesons.height),
	       width: dimbeforeconn.width + dimconn.width + dimbeforesons.width,
	       __proto__: dimproto};
	console.log ("cnode-get_dim dimfullconn=", dimfullconn);
	// beforeconn deco drawn at offx=1, offy=conn.offy
	this.conn.offx = dimbeforeconn.width;
	this.conn.offy = dimfullconn.ascent+1;
	// afterconn deco drawn at offx=1+dimfullconn.width, offy=conn.offy
	this.afterconndecoffx=1+dimfullconn.width;
	var len = this.arr.length;
	var putsondims= new Array(len); // putative dimensions
	for (var ix=0; ix<len; ix++) {
	    var curson = this.arr[ix];
	    console.log ("cnode-get_dim ix=", ix, " curson=", curson);
	    var curputdim = curson.get_dim(hints);
	    delete curson.dim;
	    delete curson.offx;
	    delete curson.offy;
	    putsondims[ix] = curputdim;
	    console.log ("cnode-get_dim curputdim=", curputdim, " for curson=", curson,
			 "\n.. ix=", ix, " this=", this);
	}
	console.log ("cnode-get_dim putsondims=", putsondims, "\n ... for this=", this);
	console.warn("cnode-get_dim @@@incomplete this=", this);
    }
};
console.log ("momp_node=", momp_node);
var MomcNode = function (conn, arr) {
    this.conn = conn;
    this.arr = arr;
    momc_count = momc_count+1;
    var myinum = momc_count;
    this.inum = myinum;
    var len = arr.length;
    for (var ix=0; ix<len; ix++) {
	var comp = arr[ix];
	if (typeof comp === "object") {
	    comp.insideinum = myinum;
	    comp.insideindex = ix;
	}
    }
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
    my_font_size: '12pt',
    my_font_family: "Arial, sans-serif",
    my_decofont_family: "Verdana, sans-serif",
    my_deco_left_attr_str: "\u2023", //  ‣ U+2023 TRIANGULAR BULLET
    my_deco_right_attr_str: " :",
    my_deco_left_val_str: " \u25b5", //  ▵ U+25B5 WHITE UP-POINTING SMALL TRIANGLE
    my_deco_right_val_str: ";",
    my_deco_val_horgap: 10,
    my_deco_val_vertgap: 3,
    //my_font_style: 'plain',
    get_dim: function (hints) {
	if (this.dim) {
	    console.log ("top_entry-get_dim with dim this=",this);
	    return this.dim;
	}
	console.log ("top_entry-get_dim this=", this, " hints=", hints,
		     " rawcanvas=", rawcanvas);
	var dimattr = this.entattr.get_dim(hints);
	console.log ("top_entry-get_dim dimattr=", dimattr);
	var decostyleprop= 
	    {fontFamily: this.my_decofont_family || this.my_font_family,
	     fontSize: this.my_decofont_size || this.my_font_size,
	     fontStyle: this.my_decofont_style || this.my_font_style
	    };
	var dimleftattrdeco =
	    measureText(this.my_deco_left_attr_str, decostyleprop);
	console.log ("top_entry-get_dim dimleftattrdeco=", dimleftattrdeco);
	var dimrightattrdeco =
	    measureText(this.my_deco_right_attr_str, decostyleprop);
	console.log ("top_entry-get_dim dimrightattrdeco=", dimrightattrdeco);
	var dimfullattr = {ascent: Math.max(dimattr.ascent+1,
					    dimleftattrdeco.ascent,
					    dimrightattrdeco.ascent),
			   descent: Math.max(dimattr.descent+1,
					     dimleftattrdeco.descent,
					     dimrightattrdeco.descent),
			   height:  Math.max(dimattr.height+2,
					     dimleftattrdeco.height,
					     dimrightattrdeco.height),
			   width: dimleftattrdeco.width + dimattr.width + dimrightattrdeco.width + 2,
			   __proto__: dimproto};
	/// left attr deco drawn at offx= 1, offy= entattr.offy;
	this.entattr.offx = dimleftattrdeco.width+1;
	this.entattr.offy = dimfullattr.ascent+1;
	/// right attr deco drawn at offx=dimleftattrdeco.width + dimattr.width + 1, offy= entattr.offy
	this.rightattrdecoffx = dimleftattrdeco.width + dimattr.width + 1;
	console.log ("top_entry-get_dim dimfullattr=", dimfullattr, " hints=", hints,
		     "\n.. this=", this);
	var oldhints = hints;
	var valhints = $.extend(oldhints,
				{maxheight: oldhints.maxheight - dimfullattr.height - this.my_deco_val_vertgap,
				 maxwidth: oldhints.maxwidth - this.my_deco_val_horgap});
	console.log ("top_entry-get_dim valhints=", valhints, "\n.. this=", this);
	var dimval = this.entval.get_dim(valhints);
	console.log ("top_entry-get_dim dimval=", dimval);
	var dimleftvaldeco =
	    measureText(this.my_deco_left_val_str, decostyleprop);
	console.log ("top_entry-get_dim dimleftvaldeco=", dimleftattrdeco);
	var dimrightvaldeco =
	    measureText(this.my_deco_right_val_str, decostyleprop);
	console.log ("top_entry-get_dim dimrightvaldeco=", dimrightattrdeco);
	/// left val deco drawn at offx= my_deco_val_horgap, ....
	/// offy= this.entval.offy
	this.entval.offx = my_deco_val_horgap+dimleftvaldeco.width+1;
	this.entval.offy = entattr.offy + dimval.ascent + my_deco_val_vertgap;
	/// right val deco drawn at offx= dimvalattrdeco.width + dimval.width + 1
	this.rightvaldecoffx = dimvalattrdeco.width + dimval.width + 1;
	var dimfullval = {ascent: Math.max(dimval.ascent+1,
					   dimleftvaldeco.ascent,
					   dimrighvaldeco.ascent),
			  descent: Math.max(dimval.descent+1,
					    dimleftvaldeco.descent,
					    dimrightvaldeco.descent),
			  height:  Math.max(dimval.height+2,
					    dimleftvaldeco.height,
					    dimrightvaldeco.height),
			  width: dimvalattrdeco.width + dimval.width + dimvalattrdeco.width + my_deco_val_horgap+ 2,
			  __proto__: dimproto};
	console.log ("top_entry-get_dim dimfullval=", dimfullval, " valhints=", valhints,
		     "\n.. this=", this);
	var diment = {ascent: this.entval.offx,
		      descent: dimfullval.descent+1,
		      height: dimfullattr.height + dimfullval.height + my_deco_val_horgap + 2,
		      width: Math.max (dimfullattr.width+2,
				       dimfullval.width),
		      __proto__: dimproto};
	this.dim = diment;
	console.debug("top_entry-get_dim result diment=", diment, " for this=", this);
	return diment;
    }
};				// end momp_top_entry
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


function test_other_display(msg) {
    var otherstr = msg + "!Mpf..jW";
    var otherstyle = {fontFamily: "Helvetica", fontSize: "45px"};
    var otherdim = measureText(otherstr, otherstyle);
    var otherx = 240;
    var othery = 600;
    console.log("test_other_display got otherdim=", otherdim, " otherdim2str=", otherdim.toString());
    canvctxt.clearRect(0,0,rawcanvas.width,rawcanvas.height);
    canvctxt.fillStyle = "rgba(255,165,0,0.6)";
    canvctxt.strokeStyle = "rgba(60,200,50,0.4)";
    canvctxt.font = otherstyle.fontSize + " " + otherstyle.fontFamily;
    canvctxt.fillText(otherstr, otherx, othery);
    canvctxt.beginPath();
    canvctxt.arc(otherx, othery, 4.0 /*radius*/, 2*Math.PI, false);
    canvctxt.fillStyle = 'rgba(125,17,81,0.25)';
    canvctxt.fill();
    canvctxt.fillStyle = "rgba(112,233,123,0.5)";
    canvctxt.strokeStyle = "rgba(12,73,53,0.5)";
    canvctxt.strokeRect(otherx, othery-otherdim.ascent, otherdim.width, otherdim.height);
    addupdatehtml("<p class='smallnote_cl'>otherdim="+otherdim.toString()+"</p>");
};				// end of test_other_display

////////////////
function momc_display_canvas(msg,arr) {
    console.group("display_canvas/%s", msg);
    console.log("display_canvas msg=", msg, " arr=", arr);
    console.log("display_canvas rawcanvas=", rawcanvas);
    console.trace();
    console.log("display_canvas before dim");
    var dim = measureText(msg, {fontFamily: "Arial", fontSize: "15pt"});
    console.log("display_canvas got dim=", dim);
    test_other_display(msg);
    canvarr = arr;
    var l = arr.length;
    var hints = {maxwidth: rawcanvas.width()-2, maxheight: rawcanvas.height()-2};
    console.log("display_canvas l=", l, " initial hints=", hints);
    var dimarr = new Array(l);
    for (var i=0; i<l; i++) {
	ob = arr[i];
	console.log ("display_canvas ob=", ob, " i#", i, " hints=", hints);
	var curdim = ob.get_dim(hints);
	console.log ("display_canvas curdim=", curdim, " i#", i);
	dimarr[i] = curdim;
	var prevmaxwidth = hints.maxwidth;
	var prevmaxheight = hints.maxwidth;
	if (curdim) {
	    hints= {maxwidth: prevmaxwidth, maxheight: prevmaxheight - curdim.height};
	    console.log ("display_canvas updated hints=", hints, " i#", i);
	}
	else
	    console.log ("display_canvas same hints=", hints, " i#", i);
    };
    console.log ("display_canvas dimarr=", dimarr);
    console.log("display_canvas end msg=", msg, " l=", l, " final hints=", hints);
    console.groupEnd();
};
////////

function ajaxcanvascript(data) {
    console.log("ajaxcanvascript data=", data);
};

////////


$(document).ready(function(){
    console.log("canvedit document ready");
    // see http://stackoverflow.com/a/10556743/841108
    window.onerror = function(msg,url,line,col,error) {
	console.trace("window error ", url, ":", line, ": ", msg, " /", error);
    }
    editlog = $("#editlog_id");
    cleareditbut = $("#cleareditbut_id");
    console.log("ready editlog=", editlog);
    rawcanvas = $("#canvedit_id");
    console.log ("ready rawcanvas=", rawcanvas);
    canvctxt = rawcanvas[0].getContext('2d');
    console.log("ready canvctxt=", canvctxt);
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
