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
};

function htmlDecode(value){
  return $('<div/>').html(value).text();
};


function ajaxfillscript(script) {
    console.log("ajaxfillscript:\n", script, "\n### endajaxfillscript\n");
};

/// see http://stackoverflow.com/q/33540051/841108
////////

var momc_count=0;

function mom_numbered(obj) {
    momc_count = momc_count+1;
    obj.inum = momc_count;
    return momc_count;
};

function mome_begin_fill(statitmid) {
    console.log("mome_begin_fill statitmid=", statitmid);
};

function mome_mtime(timestr) {
    console.log("mome_mtime timestr=", timestr);
};

function mome_entries(entarr) {
    console.log("mome_entries entarr=", entarr);
};

function mome_generated(msg) {
    console.log("mome_generated msg=", msg);
};



////////////////
function MomeEntry(entitm,entval) {
    this.entryItem= entitm;
    this.entryVal= entval;
    mom_numbered(this);
};
MomeEntry.prototype = {
    name: "MomeEntry"
};
function mome_entry(entitm,entval) {
    console.log("mome_entry entitm=", entitm, " entval=", entval);
    var res = new MomeEntry(entitm, entval);
    console.log("mome_entry res=", res);
    return res;
};

var momp_value = {
};

var momp_item_ref = {
};

var momp_nil_item = {
    __proto__: momp_item_ref
};

var momp_nil_value = {
    __proto__: momp_value
};

function MomeNilValue() {
    mom_numbered(this);
};
MomeNilValue.prototype = momp_nil_value;

function mome_nil_val() { /// a nil value
    var res = new MomeNilValue();
    console.log ("mome_nil_val res=", res);
    return res;
};

function MomeNilRef() {
    mom_numbered(this);
};
MomeNilRef.prototype = momp_nil_item;

function mome_nil_ref() {	/// a nil item reference
    var res = new MomeNilRef();
    console.log ("mome_nil_ref res=", res);
    return res;
};

var momp_item_value = {
    __proto__: momp_value,
    val_kind: 'item'
};

function MomeItemVal(iname) {
    this.item_name = iname;
    mom_numbered(this);
};
MomeItemVal.prototype = momp_item_value;

function mome_item_val(itmname) {
    console.log ("mome_item_val itmname=", itmname);
    var res = new MomeItemVal(itmname);
    console.log ("mome_item_val res=", res);
    return res;
};

function MomeItemRef(iname) {
    this.item_name = iname;
    mom_numbered(this);
};
MomeItemRef.prototype = momp_item_ref;

function mome_item_ref(itmname) {
    var res = new MomeItemRef(itmname);
    console.log ("mome_item_ref res=", res);
    return res;
};


////////////////
var momp_int_value = {
    __proto__: momp_value,
    val_kind: 'int'
};

function MomeIntValue(ival) {
    mom_numbered(this);
    this.int_val = ival;
};
MomeIntValue.prototype = momp_int_value;

function mome_int (num) {
    var res = new MomeIntValue(num);
    console.log ("mome_int res=", res);
    return res;
};

////////////////
var momp_double_value = {
    __proto__: momp_value,
    val_kind: 'double'
};
function MomeDoubleValue(dval) {
    mom_numbered(this);
    this.dbl_val = dval;
}
MomeDoubleValue.prototype = momp_double_value;
function mome_double(dbl) {
    var res = new MomeDoubleValue(dbl);
    console.log ("mome_int res=", res);
    return res;
}

////////////////
var momp_string_value = {
    __proto__: momp_value,
    val_kind: 'string'
};
function MomeStringValue(str) {
    mom_numbered(this);
    this.str_val = str;
}
MomeStringValue.prototype = momp_string_value;
function mome_string(str) {
    var res = new MomeStringValue(str);
    console.log ("mome_string res=", res);
    return res;
}


////////////////
var momp_tuple_value = {
    __proto__: momp_value,
    val_kind: 'tuple'
};
function MomeTupleValue(arr) {
    mom_numbered(this);
    this.tup_val = arr;
}
MomeTupleValue.prototype = momp_tuple_value;
function mome_tuple(tuparr) {
    var res = new MomeTupleValue(tuparr);
    console.log ("mome_tuple res=", res);
    return res;
}

////////////////
var momp_set_value = {
    __proto__: momp_value,
    val_kind: 'set'
};
function MomeSetValue(arr) {
    mom_numbered(this);
    this.set_val = arr;
}
MomeSetValue.prototype = momp_set_value;
function mome_set(setarr) {
    var res = new MomeSetValue(setarr);
    console.log ("mome_set res=", res);
    return res;
}

////////////////
var momp_node_value = {
    __proto__: momp_value,
    val_kind: 'node'
};
function MomeNodeValue(conn,sons) {
    mom_numbered(this);
    this.conn_itm = conn;
    this.sons_arr = sons;
}
MomeNodeValue.prototype = momp_node_value;
function mome_node(connitm, sonsarr) {
    var res = new MomeNodeValue(connitm,sonsarr);
    console.log ("mome_node res=", res);
    return res;
}


$(document).ready(function(){
    console.log("microedit document ready");
    // see http://stackoverflow.com/a/10556743/841108
    window.onerror = function(msg,url,line,col,error) {
	console.error("window error ", url, ":", line, ": ", msg, " /", error);
    };
    $editdiv = $("#microedit_id");
    $editlog = $("#editlog_id");
    $cleareditbut = $("#cleareditbut_id");
    $cleareditbut.click(function(evt){
	console.log("clearedit evt=", evt);
	$editlog.html("");
    });
    console.log("microedit before ajax do_fillpage");
    $.ajax
    ({url: "/microedit",
      method: "POST",
      data: {"do_fillpage": true},
      dataType: "script",
      success: ajaxfillscript
     });
    console.log("microedit document done ready");
});

console.log("eof parsed script microedit.js");/// eof microedit.js
