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
var momv_entriesvar=null;
var mom_name_regexp = /^[A-Za-z0-9_]*$/;

var mom_name_cache = new Object();

function mom_numbered(obj) {
    momc_count = momc_count+1;
    obj.inum = momc_count;
    return momc_count;
};

function mom_known_item(name) {
    if (!name.match(mom_name_regexp))
	return false;
    if (mom_name_cache[name])
	return true;
    var res = null;
    console.log("mom_known_item name=", name, " before ajaxing");
    $.ajax
    ({url: "/microedit",
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
};				// end mom_known_item

function mom_complete_name(name) {
    var res = null;
    if (!name.match(mom_name_regexp)) {
	console.log ("mom_complete_name bad name=", name);
	return false;
    }
    else 
    console.log("mom_complete_name name=", name, " before ajaxing");
    $.ajax
    ({url: "/microedit",
      method: "POST",
      async: false,
      data: {"do_completename": name},
      dataType: "json",
      success: function (data, stat, jh) {
	  console.log("mom_complete_name success data=", data, " stat=", stat, " jh=", jh);
	  res = data;
      },
      error: function (jq, stat, err) {
	  console.warn("mom_complete_name error jq=", jq, " stat=", stat, " err=", err);
	  res = false;
      }
     });
    console.log ("mom_complete_name name=", name, " res=", res);
    return res;
};				// end mom_complete_name


function mome_begin_fill(statitmid) {
    console.log("mome_begin_fill statitmid=", statitmid, " $editdiv=", $editdiv);
    $editdiv.empty();
    $("<p class='mombeginfill_cl'>" + statitmid + "</p>").appendTo($editdiv);
    console.log("mome_begin_fill $editdiv=", $editdiv, " this=", this);
    mom_name_cache = new Object();
};

function mome_mtime(timestr) {
    console.log("mome_mtime timestr=", timestr);
    $("<p class='mommtime_cl'>" + htmlEncode(timestr) + "</p>").appendTo($editdiv);
    console.log("mome_mtime $editdiv=", $editdiv, " this=", this);
};

function mome_entries(entarr) {
    console.log("mome_entries entarr=", entarr, " $editdiv=", $editdiv,
		" this=", this);
    momv_entriesvar = entarr;
    console.assert(Array.isArray(entarr),
		   "mome_entries bad entarr=", entarr);
    var nbent = entarr.length;
    var entdl = $("<dl class='momentrieslist_cl'></dl>");
    entdl.appendTo($editdiv);
    console.log("mome_entries entdl=", entdl, " $editdiv=", $editdiv);
    for (var ixent = 0; ixent<nbent; ixent++) {
	var curent = entarr[ixent];
	console.log ("mome_entries ixent#", ixent, " curent=", curent);
	curent.realize(entdl);
	console.log ("mome_entries done ixent#", ixent, " curent=", curent);
    }
    console.log("mome_entries done entarr=", entarr, " $editdiv=", $editdiv);
};

function mome_generated(msg) {
    console.log("mome_generated start msg=", msg, "; $editdiv=", $editdiv, " this=", this);
    var nbitem=0, nbvalue=0;
    // see http://stackoverflow.com/a/33541885/841108 the git tag
    // many-handler-debug, git commit 51c73df483c2281fa4a6 had lot of
    // console debugging messages to handle events.
    console.log("mome_generated ending $editlog=", $editlog, " msg=", msg);
    $editlog.append(msg);

};				// end mome_generated


////////////////
function MomeEntry(entitm,entval) {
    this.entryItem= entitm;
    this.entryVal= entval;
    mom_numbered(this);
};
var momp_entry = {
    name: "MomeEntry",
    realize: function (cont) {
	console.log("MomeEntry-realize start cont=", cont, " this=", this,
		    " $(cont).is('dl')=", $(cont).is('dl'),
		    " cont.is('dl')=", cont.is('dl'),
		    "\n"
		   );
	console.assert($(cont).is("dl"), "MomeEntry invalid container cont=", cont);
	console.assert("entryItem" in this, "MomeEntry no entryItem in this=", this);
	console.assert("entryVal" in this, "MomeEntry no entryVal in this=", this);
	var eattelem = $("<dt class='statattr_cl'></dt>");
	this.entryItem.realize(eattelem);
	var evalelem = $("<dd class='statval_cl'></dd>");
	this.entryVal.realize(evalelem);
	eattelem.appendTo(cont);
	evalelem.appendTo(cont);
	console.log("MomeEntry-realize done cont=", cont,
		    " eattelem=", eattelem, " evalelem=", evalelem, "\n..this=", this);
    }
};
MomeEntry.prototype = momp_entry;
function mome_entry(entitm,entval) {
    console.log("mome_entry entitm=", entitm, " entval=", entval);
    var res = new MomeEntry(entitm, entval);
    console.log("mome_entry res=", res);
    return res;
};

var momp_value = {
    realize: function (cont) {
	console.error("MomeValue-realize bad cont=", cont, " this=", this);
    }
};

var momp_item_ref = {
    name: "MomeItemRef",
    gotinput: function (ev) {
	console.log ("MomeItemRef-gotinput ev=", ev, " $(this)=", $(this));
	var curtxt = $(this).text();
	var curmatch = curtxt.match(mom_name_regexp);
	console.log ("MomeItemRef-gotinput curtxt=", curtxt, " curmatch=", curmatch);
	if (!curmatch) {
	    $(this).text($(this).old_text);
	}
    },
    gotkeypress: function (ev) {
	var curtxt = $(this).text();
	var curmatch = curtxt.match(mom_name_regexp);
	console.log ("MomeItemRef-gotkeypress ev=", ev, " .key=`", ev.key, "' $(this)=", $(this),
		     " curtxt=", curtxt, " curmatch=", curmatch);
	if (ev.key === ' ') {
	    console.log ("MomeItemRef-gotkeypress space ev=", ev);
	    if (ev.ctrlKey && !ev.metaKey) {
	    }
	};
	if (ev.ctrlKey || ev.metaKey
	    || (typeof ev.key)!=="string"
	    || !ev.key.match(mom_name_regexp)) {
	    console.log ("MomeItemRef-gotkeypress ev=", ev, " reject strangekey");
	    return false;
	};
    },
    gotfocusin: function (ev) {
	//$(this).prop("contenteditable",true);
	var oldtxt = $(this).text();
	var focusel = $(':focus');
	console.log ("MomeItemRef-gotfocusin ev=", ev, " $(this)=", $(this), " $editdiv=", $editdiv, " oldtxt=", oldtxt,
		     " focusel=", focusel);
	$(this).old_text = oldtxt;
    },
    gotfocusout: function (ev) {
	//$(this).prop("contenteditable",false);
	console.log ("MomeItemRef-gotfocusout ev=", ev, " $(this)=", $(this), " $editdiv=", $editdiv);
	delete $(this).old_text;
    },
    realize: function (cont) {
	console.log("MomeItemRef-realize start cont=", cont, " this=", this);
	console.assert(typeof (this.item_name) === "string", "MomeItemRef no item_name in this=", this);
	/// perhaps need contenteditable in the span below?
	var eitelem = $("<span class='momitem_bcl momitemref_cl'>"+this.item_name+"</span>");
	eitelem.appendTo(cont);
	console.log("MomeItemRef-realize on eitelem=", eitelem, " gotinput=", this.gotinput,
		    " gotfocusin=", this.gotfocusin, " gotfocusout=", this.gotfocusout);
	eitelem.on("input", this.gotinput);
	eitelem.on("focusin", this.gotfocusin);
	eitelem.on("focusout", this.gotfocusout);
	eitelem.on("keypress", this.gotkeypress);
	console.log("MomeItemRef-realize done cont=", cont,
		    " eitelem=", eitelem, "\n..this=", this);
    }
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

var momp_nil_value = {
    name: "MomeNilValue",
    realize: function (cont) {
	console.log("MomeNilValue-realize start cont=", cont, " this=", this);
	var enilelem = $("<span class=' momval_bcl momemptyval_cl'>~</span>");
	enilelem.appendTo(cont);
	enilelem.data("momfor", this);
	console.log("MomeNilValue-realize done cont=", cont,
		    " enilelem=", enilelem, "\n..this=", this);
    },
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

////////////////
var momp_nil_ref = {
    name: "MomeNilRef",
    realize: function (cont) {
	console.log("MomeNilRef-realize start cont=", cont, " this=", this);
	var enilelem = $("<span class=' mom_item_bcl momrefnil_cl'>~</span>");
	enilelem.appendTo(cont);
	enilelem.data("momfor", this);
	console.log("MomeNilRef-realize done cont=", cont,
		    " enilelem=", enilelem, "\n..this=", this);
    },
    __proto__: momp_item_ref
};
function MomeNilRef() {
    mom_numbered(this);
};
MomeNilRef.prototype = momp_nil_ref;

function mome_nil_ref() {	/// a nil item reference
    var res = new MomeNilRef();
    console.log ("mome_nil_ref res=", res);
    return res;
};

////////////////
var momp_item_value = {
    name: "MomeItemVal",
    __proto__: momp_value,
    gotinput: function (ev) {
	console.log ("MomeItemVal-gotinput ev=", ev, " $(this)=", $(this));
	var curtxt = $(this).text();
	var curmatch = curtxt.match(mom_name_regexp);
	console.log ("MomeItemVal-gotinput curtxt=", curtxt, " curmatch=", curmatch);
	if (!curmatch) {
	    $(this).text($(this).old_text);
	    return false;
	}
	if (mom_known_item(curtxt)) {
	    console.log ("MomeItemVal-gotinput knownitem ", curtxt);
	    $(this).addClass('momitemval_cl');
	    $(this).removeClass('momname_cl');
	}
	else {
	    console.log ("MomeItemVal-gotinput unknownitem ", curtxt);
	    $(this).removeClass('momitemval_cl');
	    $(this).addClass('momname_cl');
	}
    },
    gotkeypress: function (ev) {
	var curtxt = $(this).text();
	var curmatch = curtxt.match(mom_name_regexp);
	var compl = null;
	console.log ("MomeItemVal-gotkeypress ev=", ev, ".key='", ev.key, "' curtxt=", curtxt,
		     " $(this)=", $(this), " curmatch=", curmatch);
	if (ev.key === ' ') {
	    console.log ("MomeItemVal-gotkeypress space ev=", ev);
	    if (ev.ctrlKey && !ev.metaKey) {
		console.log ("MomeItemVal-gotkeypress ctrlspace ev=", ev, " should complete curtxt=", curtxt);
		comp = mom_complete_name(curtxt);
		console.log ("MomeItemVal-gotkeypress completed curtxt=", curtxt, " got comp=", comp);
	    }
	    return false;
	};
	if (ev.ctrlKey || ev.metaKey
	    || (typeof ev.key) !== "string"
	    || !ev.key.match(mom_name_regexp)) {
	    console.log ("MomeItemVal-gotkeypress ev=", ev, " reject strangekey");
	    return false;
	};
    },
    gotfocusin: function (ev) {
	var focusel = $(':focus');
	var oldtxt = $(this).text();
	console.log ("MomeItemVal-gotfocusin ev=", ev, " oldtxt=", oldtxt,
		     " $(this)=", $(this), " $editdiv=", $editdiv,
		     " focusel=", focusel);
	$(this).old_text = oldtxt;
    },
    gotfocusout: function (ev) {
	console.log ("MomeItemVal-gotfocusout ev=", ev, " $(this)=", $(this), " $editdiv=", $editdiv);
	delete $(this).old_text;
    },
    realize: function (cont) {
	console.log("MomeItemVal-realize start cont=", cont, " this=", this);
	console.assert(typeof (this.item_name) === "string",
		       "MomeItemVal no item_name in this=", this);
	/// perhaps need contenteditable in the span below?
	var eitmelem = $("<span class='mom_item_bcl momitemval_cl'>"+ this.item_name +"</span>");
	$(this).mom_span = eitmelem;
	console.log("MomeItemVal-realize eitmelem=", eitmelem, " this=", this, " cont=", cont);
	eitmelem.appendTo(cont);
	console.log("MomeItemVal-realize updated cont=", cont, " this=", this, " eitmelem=", eitmelem);	
	eitmelem.data("momfor", this);
	console.log("MomeItemVal-realize updated cont=", cont, " this=", this, " gotinput=", this.gotinput);
	eitmelem.on("input", this.gotinput);
	console.log("MomeItemVal-realize updated cont=", cont, " this=", this, " gotfocusin=", this.gotfocusin);
	eitmelem.on("focusin", this.gotfocusin);
	console.log("MomeItemVal-realize updated cont=", cont, " this=", this, " gotfocusout=", this.gotfocusout);
	eitmelem.on("focusout", this.gotfocusout);
	eitmelem.on("keypress", this.gotkeypress);
	console.log("MomeItemVal-realize done cont=", cont,
		    " eitmelem=", eitmelem, "\n..this=", this);
    },
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


////////////////
var momp_int_value = {
    name: "MomeIntValue",
    __proto__: momp_value,
    realize: function (cont) {
	console.log("MomeIntValue-realize start cont=", cont, " this=", this);
	console.assert(typeof (this.int_val) === "number",
		       "MomeIntValue no int_val in this=", this);
	var eintelem = $("<span class='mom_value_bcl momnumber_cl'>"+ this.int_val.toString() +"</span>");
	eintelem.appendTo(cont);
	eintelem.data("momfor", this);
	console.log("MomeIntValue-realize done cont=", cont,
		    " eintelem=", eintelem, "\n..this=", this);
    },
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
    name: "MomeDoubleValue",
    __proto__: momp_value,
    realize: function (cont) {
	console.log("MomeDoubleValue-realize start cont=", cont, " this=", this);
	console.assert(typeof (this.dbl_val) === "number",
		       "MomeDoubleValue no dbl_val in this=", this);
	var edblelem = $("<span class='mom_value_bcl momnumber_cl'>"+ this.dbl_val.toString() +"</span>");
	edblelem.appendTo(cont);
	edblelem.data("momfor", this);
	console.log("MomeDoubleValue-realize done cont=", cont,
		    " edblelem=", edblelem, "\n..this=", this);
    },
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
    name: 'MomeStringValue',
    __proto__: momp_value,
    realize: function (cont) {
	console.log("MomeStringValue-realize start cont=", cont, " this=", this);
	console.assert(typeof (this.str_val) === "string",
		       "MomeStringValue no str_val in this=", this);
	var estrelem = $("<q class='momstrquote_cl'><span class='mom_value_bcl momstring_cl'>"+ htmlEncode(this.str_val) +"</span></q>");
	estrelem.appendTo(cont);
	estrelem.data("momfor", this);
	console.log("MomeStringValue-realize done cont=", cont,
		    " estrelem=", estrelem, "\n..this=", this);
    },
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
    name: 'MomeTupleValue',
    __proto__: momp_value,
    realize: function (cont) {
	console.log("MomeTupleValue-realize start cont=", cont, " this=", this);
	console.assert(Array.isArray (this.tup_val),
		       "MomeTupleValue no tup_val in this=", this);
	var etupelem = $("<span class='mom_value_bcl momtuple_cl'></span>");
	etupelem.appendTo(cont);
	var et = null;
	et = document.createTextNode("[");
	$(et).appendTo(etupelem);
	et = null;
	var nbcomp = this.tup_val.length;
	for (var compix=0; compix<nbcomp; compix++) {
	    var curcomp = this.tup_val[compix];
	    console.log("MomeTupleValue-realize compix=", compix,
			" curcomp=", curcomp,
			" this=", this, " cont=", cont);
	    if (compix > 0) {
		et = document.createTextNode(" ");
		$(et).appendTo(etupelem);
		et = null;
	    };
	    curcomp.realize(etupelem);
	    console.log("MomeTupleValue-realize did compix=", compix,
			" curcomp=", curcomp, " etupelem=", etupelem);
	}
	et = document.createTextNode("]");
	$(et).appendTo(etupelem);
	et = null;
	etupelem.data("momfor", this);
	console.log("MomeTupleValue-realize done cont=", cont,
		    " etupelem=", etupelem, "\n..this=", this);
    },
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
    name: "MomeSetValue",
    realize: function (cont) {
	console.log("MomeSetValue-realize start cont=", cont, " this=", this);
	console.assert(Array.isArray (this.set_val),
		       "MomeSetValue no set_val in this=", this);
	var esetelem = $("<span class='mom_value_bcl momset_cl'></span>");
	esetelem.appendTo(cont);
	var et = null;
	et = document.createTextNode("{");
	$(et).appendTo(esetelem);
	et = null;
	var nbelem = this.set_val.length;
	for (var elemix=0; elemix<nbelem; elemix++) {
	    var curelem = this.set_val[elemix];
	    console.log("MomeSetValue-realize elemix=", elemix,
			" curelem=", curelem,
			" this=", this, " cont=", cont);
	    if (elemix > 0) {
		et = document.createTextNode(" ");
		$(et).appendTo(esetelem);
		et = null;
	    };
	    curelem.realize(esetelem);
	    console.log("MomeSetValue-realize did elemix=", elemix,
			" curelem=", curelem, " esetelem=", esetelem);
	}
	et = document.createTextNode("}");
	$(et).appendTo(esetelem);
	et = null;
	esetelem.data("momfor", this);
	console.log("MomeSetValue-realize done cont=", cont,
		    " esetelem=", esetelem, "\n..this=", this);
    },
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
    name: "MomeNodeValue",
    realize: function (cont) {
	console.log("MomeNodeValue-realize start cont=", cont, " this=", this);
	console.assert(typeof (this.conn_itm) === "object"
		       && Array.isArray(this.sons_arr),
		       "MomeSetValue no conn_itm&sons_arr in this=", this);
	var enodelem = $("<span class='mom_value_bcl momnode_cl'></span>");
	console.log("MomeNodeValue-realize enodelem=", enodelem);
	enodelem.appendTo(cont);
	var et = null;
	var econnelem = $("<span class='momconn_cl'>*</span>");
	econnelem.appendTo(enodelem);
	this.conn_itm.realize(econnelem);
	console.log("MomeNodeValue-realize after connitm enodelem=", enodelem, " this=", this);
	et = document.createTextNode("(");
	$(et).appendTo(enodelem);
	var nbsons = this.sons_arr.length;
	console.log("MomeNodeValue-realize nbsons=", nbsons, " this=", this);
	for (var sonix=0; sonix<nbsons; sonix++) {
	    var curson = this.sons_arr[sonix];
	    console.log("MomeNodeValue-realize sonix=", sonix, " curson=", curson, " this=", this);
	    if (sonix>0) {
		et = document.createTextNode(" ");
		$(et).appendTo(enodelem);
		et = null;
	    };
	    curson.realize(enodelem);
	    console.log("MomeNodeValue-realize enodelem=", enodelem, " after sonix=", sonix);
	}
	et = document.createTextNode(")");
	$(et).appendTo(enodelem);
	et = null;
	enodelem.data("momfor", this);
	console.log("MomeNodeValue-realize done cont=", cont,
		    " enodelem=", enodelem, "\n..this=", this);
    },
    val_kind: 'node'
};
function MomeNodeValue(conn,sons) {
    mom_numbered(this);
    this.conn_itm = conn;
    this.sons_arr = sons;
};
MomeNodeValue.prototype = momp_node_value;
function mome_node(connitm, sonsarr) {
    var res = new MomeNodeValue(connitm,sonsarr);
    console.log ("mome_node res=", res);
    return res;
};

//function editdivinput(ev) {
//    console.log ("editdivinput ev=", ev, " this=", this);
//}

//function editdivbeforeinput(ev) {
//    console.log ("editdivbeforeinput ev=", ev, " this=", this);
//}

$(document).ready(function(){
    console.log("microedit document ready");
    // see http://stackoverflow.com/a/10556743/841108
    window.onerror = function(msg,url,line,col,error) {
	console.error("window error ", url, ":", line, ": ", msg, " /", error);
    };
    $editdiv = $("#microedit_id");
    $editlog = $("#editlog_id");
    $cleareditbut = $("#cleareditbut_id");
    console.log ("microedit readying $editdiv=", $editdiv, " $editlog=", $editlog, " $cleareditbut=", $cleareditbut);
    $cleareditbut.click(function(evt){
	console.log("clearedit evt=", evt);
	$editlog.html("");
    });
    console.log("microedit before ajax do_fillpage");
//    $editdiv.on("input", editdivinput);
//    $editdiv.on("beforeinput", editdivbeforeinput);
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
