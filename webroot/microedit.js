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

// maybe use http://jakiestfu.github.io/Medium.js/ ??

// see also following links suggested by Alexandre Lissy
// https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes/contenteditable
// https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes/tabindex
// https://dxr.mozilla.org/mozilla-central/source/dom/html/nsGenericHTMLElement.h#193
// https://developer.mozilla.org/en-US/docs/Web/API/Selection
// https://bugzilla.mozilla.org/show_bug.cgi?id=1196479
// https://developer.mozilla.org/en-US/docs/Web/API/Document/activeElement

// and from SO: http://stackoverflow.com/a/33911936/841108

// from http://stackoverflow.com/a/1219983/841108
function htmlEncode(value){
    //create a in-memory div, set it's inner text(which jQuery automatically encodes)
    //then grab the encoded contents back out.  The div never exists on the page.
    return $('<div/>').text(value).html();
};

function htmlDecode(value){
    return $('<div/>').html(value).text();
};



/// this function is called by /microedit AJAX for do_fillpage at document loading
function ajaxfillscript(script) {
    console.log("ajaxfillscript:\n", script, "\n### endajaxfillscript\n");
    console.trace();
};

/// see http://stackoverflow.com/q/33540051/841108
////////

var momc_count=0;
var momv_entriesvar=null;
var mom_name_regexp = /^[A-Za-z0-9_]*$/;

var mom_name_cache = new Object();

// this utility function is assigning a unique number
function mom_numbered(obj) {
    momc_count = momc_count+1;
    obj.inum = momc_count;
    return momc_count;
};

/// check if a name is some known item, with memoization & AJAX do_knownitem
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
};                              // end mom_known_item

//// give the completion of some name string, using AJAX do_completename
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
      success: function (data) {
          console.log("mom_complete_name success data=", data);
          console.trace();
          res = data;
      },
      error: function (jq, stat, err) {
          console.warn("mom_complete_name error jq=", jq, " stat=", stat, " err=", err);
          res = false;
      }
     });
    console.log ("mom_complete_name name=", name, " res=", res);
    return res;
};                              // end mom_complete_name


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
        curent.realize_for2(function(del1, del2) {
            console.log ("mome_entries curent=", curent , " for del1=", del1, " del2=", del2);
            del1.appendTo(entdl);
            del2.appendTo(entdl);
        });
        console.log ("mome_entries done ixent#", ixent, " curent=", curent);
    };
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

};                              // end mome_generated


////////////////
function MomeEntry(entitm,entval) {
    this.entryItem= entitm;
    this.entryVal= entval;
    mom_numbered(this);
};
var momp_entry = {
    name: "MomeEntry",
    realize_for2: function (rec2fun) {
        console.log("MomeEntry-realize_for2 start rec2fun=", rec2fun, " this=", this,
                    "\n"
                   );
        console.trace();
        console.assert("entryItem" in this, "MomeEntry no entryItem in this=", this);
        console.assert("entryVal" in this, "MomeEntry no entryVal in this=", this);
        var eattelem = $("<dt class='statattr_cl'></dt>");
        var evalelem = $("<dd class='statval_cl'></dd>");
        var self = this;
        console.log("MomeEntry-realize_for2 self=", self, " eattelem=", eattelem, " evalelem=", evalelem, " rec2fun=", rec2fun);
        rec2fun(eattelem, evalelem);
        console.log("MomeEntry-realize_for2 this=", this,
                    " done rec2fun=", rec2fun);
        this.entryItem.realize_for(function (del) {
            console.log ("MomeEntry-realize_for2 self=", self, " eattelem=", eattelem, " del=", del);
            console.trace();
            del.appendTo(eattelem);
	    del.mom_attr_entry = self;
        });
        this.entryVal.realize_for(function (del) {
            console.log ("MomeEntry-realize_for2 self=", self, " evalelem=", evalelem, " del=", del);
            console.trace();
            del.appendTo(evalelem);
	    del.mom_val_entry = self;
        });
        console.log("MomeEntry-realize_for2 done rec2fun=", rec2fun,
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
    realize_for: function (recfun) {
        console.error("MomeValue-realize_for bad recfun=", recfun, " this=", this);
    }
};

var momp_item_ref = {
    name: "MomeItemRef",
    realize_for: function (recfun) {
        console.log("MomeItemRef-realize_for start recfun=", recfun, " this=", this);
        console.assert(typeof (this.item_name) === "string", "MomeItemRef no item_name in this=", this);
        /// perhaps need contenteditable in the span below?
        var eitelem = $("<span class='momitem_bcl momitemref_cl' tabindex='0'>"+this.item_name+"</span>");
        console.log("MomeItemRef-realize_for on eitelem=", eitelem, " gotinput=", this.gotinput,
                    " gotfocusin=", this.gotfocusin, " gotfocusout=", this.gotfocusout);
        eitelem.on("input", this.gotinput);
        eitelem.on("focusin", this.gotfocusin);
        eitelem.on("focusout", this.gotfocusout);
        eitelem.on("keypress", this.gotkeypress);
        console.log("MomeItemRef-realize_for recfun=", recfun, " this=", this, " eitelem=", eitelem);
        recfun(eitelem);
        console.log("MomeItemRef-realize_for done recfun=", recfun,
                    " eitelem=", eitelem, "\n..this=", this);
    },
    gotinput: function (ev) {
        console.log ("MomeItemRef-gotinput ev=", ev, " $(this)=", $(this), " $(':focus')=", $(':focus'));
        var curtxt = $(this).text();
        console.log ("MomeItemRef-gotinput curtxt=", curtxt);
    },
    gotkeypress: function (ev) {
        var focusel = $(':focus');
        console.log ("MomeItemRef-gotkeypress ev=", ev, " .key=`", ev.key, "' .which=", ev.which,
		     "$(this)=", $(this), " focusel=", focusel);
        if (ev.which == 32 || ev.key === ' ') {
            console.log ("MomeItemRef-gotkeypress space ev=", ev);
            if (ev.ctrlKey && !ev.metaKey) {
            }
        };
        if (ev.ctrlKey || ev.metaKey
            || (typeof ev.key)!=="string"
            || !ev.key.match(mom_name_regexp)) {
            console.log ("MomeItemRef-gotkeypress ev=", ev, ".which=", ev.which, " reject strangekey");
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
    realize_for: function (recfun) {
        console.log("MomeNilValue-realize_for start recfun=", recfun, " this=", this);
        console.trace();
        var enilelem = $("<span class='momval_bcl momemptyval_cl' tabindex='0'>~</span>");
        enilelem.data("momfor", this);
        console.log("MomeNilValue-realize_for this=", this, " enilelem=", enilelem);
        recfun(enilelem);
        console.log("MomeNilValue-realize_for done recfun=", recfun,
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
    realize_for: function (recfun) {
        console.log("MomeNilRef-realize_for start recfun=", recfun, " this=", this);
        console.trace();
        var enilelem = $("<span class=' mom_item_bcl momrefnil_cl' tabindex='0'>~</span>");
        enilelem.data("momfor", this);
        console.log("MomeNilRef-realize_for enilelem=", enilelem, " this=", this);
        recfun(enilelem);
        console.log("MomeNilRef-realize_for done recfun=", recfun,
                    " enilelem=", enilelem, "\n..this=", this);
    },
    __proto__: momp_item_ref
};
function MomeNilRef() {
    mom_numbered(this);
};
MomeNilRef.prototype = momp_nil_ref;

function mome_nil_ref() {       /// a nil item reference
    var res = new MomeNilRef();
    console.log ("mome_nil_ref res=", res);
    return res;
};

var momp_item_input = {
    name: "MomeItemInput",
    install_input: function () {
        console.log ("MomeItemInput-install_input start this=", this, " $(this)=", $(this),
		     " activeElement=", document.activeElement);
        console.trace();
        var self = this;
        var orig = this.mom_orig;
        console.assert (orig, "MomeItemInput-install_input bad orig");
        var inp = $("<input type='text' width='16' class='mom_item_input_cl'/>");
        console.log ("MomeItemInput-install_input this=", this, " inp=", inp, " orig=", orig,
		     " activeElement=", document.activeElement);
	inp.mom_obj = this;
        orig.replaceWith(inp);
        console.log ("MomeItemInput-install_input this=", this, " inp=", inp, " replaced orig=", orig,
		     " activeElement=", document.activeElement);
        this.mom_input = inp;
	inp.focus();
        console.log ("MomeItemInput-install_input this=", this, " inp=", inp,
		     " activeElement=", document.activeElement);
        // don't navigate away from the field on tab when selecting an item
        inp.bind("keydown", function (ev) {
            console.log ("MomeItemInput keydown ev=", ev);
            if (ev.keyCode === $.ui.keyCode.TAB
                && inp.autocomplete("instance").menu.active) {
                console.log ("MomeItemInput keydown ignore tab-menu");
                ev.preventDefault();
            };      
        });
        inp.on("change", function (ev) {
            console.log ("MomeItemInput change ev=", ev, " this=", this, " self=", self);
        });
        inp.on("input", function (ev) {
            console.log ("MomeItemInput input ev=", ev, " this=", this, " self=", self);
        });
        inp.autocomplete({
            minLength: 2,
            source: this.item_autocomplete
        });
        console.log ("MomeItemInput-install_input this=", this, " give inp=", inp,
		     " activeElement=", document.activeElement);
        return inp;
    },
    item_autocomplete: function (requ, respfun) {
        console.log ("MomeItemInput-item_autocomplete start requ=", requ, " respfun=", respfun);
        var compl = mom_complete_name(requ.term);
        console.log ("MomeItemInput-item_autocomplete compl=", compl);
        if (compl) respfun(compl);
        else respfun("");
    },
    val_kind: "item"
};

function MomeItemInput(orig) {
    this.mom_orig = orig;
    console.log("MomeItemInput this=", this, " orig=", orig);
    console.assert(orig, "MomeItemInput bad orig");
};
MomeItemInput.prototype = momp_item_input;

function mome_replace_by_item_input_for(orig, recfun) {
    console.log ("mome_replace_by_item_input_for start this=", this,
		 " $(this)=", $(this), " orig=", orig, " recfun=", recfun,
		 " activeElement=", document.activeElement);
    console.trace();
    console.assert (orig, "mome_replace_by_item_input_for bad orig");
    var inp = new MomeItemInput(orig);
    inp.install_input();
    console.log ("mome_replace_by_item_input_for end this=", this, " inp=", inp,
		 " activeElement=", document.activeElement);
    return inp;
};

////////////////
var momp_item_value = {
    name: "MomeItemVal",
    __proto__: momp_value,
    realize_for: function (recfun) {
        console.log("MomeItemVal-realize_for start recfun=", recfun, " this=", this, " $(this)=", $(this));
        console.trace();
        console.assert(typeof (this.item_name) === "string",
                       "MomeItemVal no item_name in this=", this);
        /// perhaps need contenteditable in the span below?
        var eitmelem = $("<span class='mom_item_bcl momitemval_cl' tabindex='0'>"+ this.item_name +"</span>");
        eitmelem.data("mom_item_val", this.item_name);
        this.mom_span = eitmelem;
        console.log("MomeItemVal-realize_for eitmelem=", eitmelem, " this=", this,  " $(this)=", $(this), " recfun=", recfun);
        eitmelem.data("momfor", this);
        eitmelem.on("input", this.gotinput);
        eitmelem.on("focusin", this.gotfocusin);
        eitmelem.on("focusout", this.gotfocusout);
        eitmelem.on("keypress", this.gotkeypress);
        console.log("MomeItemVal-realize_for this=", this, " before recfun=", recfun, " eitmelem=", eitmelem);
        recfun(eitmelem);
        console.log("MomeItemVal-realize_for done recfun=", recfun,
                    " eitmelem=", eitmelem, "\n..this=", this);
    },
    gotinput: function (ev) {
        console.log ("MomeItemVal-gotinput ev=", ev, " $(this)=", $(this), " $(':focus')=", $(':focus'));
        var curtxt = $(this).text();
        console.log ("MomeItemVal-gotinput curtxt=", curtxt);
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
        // don't use .key in jquery keypress event, but only .which
        // see https://api.jquery.com/keypress/
        var self = $(this);
        var curtxt = self.text();
        console.log ("MomeItemVal-gotkeypress ev=", ev, ".key='", ev.key,
                     "' .keyCode=", ev.keyCode, " .which=", ev.which,
                     " curtxt=", curtxt,
                     " self=", self, " this=", this, " $(':focus')=", $(':focus'),
		     " activeElement=", document.activeElement);
        if (ev.keyCode === $.ui.keyCode.SPACE
	    || ev.which === 32
            || ev.key === ' ') {
            console.log ("MomeItemVal-gotkeypress space ev=", ev, " this=", this,
                         " self=", self,
                         " before replace_by_item_input_for",
			 " activeElement=", document.activeElement);
            console.trace();
	    ////@@ self.blur();
            ////@@ console.log ("MomeItemVal-gotkeypress space ev=", ev, " blurred self=", self,
	    ////@@	 " activeElement=", document.activeElement);
            var inp = mome_replace_by_item_input_for($(this), function (del) {
                console.log ("MomeItemVal gotkeypress replacing self=", self, " this=", this, " del=", del);
                self.replaceWith(del);
                //del.focus();
                console.log ("MomeItemVal gotkeypress done self=", self, " replaced by del=", del,
			     " activeElement=", document.activeElement);
            });
            console.log ("MomeItemVal-gotkeypress space this=", this, " inp=", inp);
        }
        else if (ev.ctrlKey || ev.metaKey
                 || (typeof ev.key) !== "string"
                 || !ev.key.match(mom_name_regexp)) {
            console.log ("MomeItemVal-gotkeypress ev=", ev, " reject strangekey .which=", ev.which);
            return false;
        };
        console.log ("MomeItemVal-gotkeypress done ev=", ev, " now focus=", $(':focus'),
		     " activeElement=", document.activeElement);
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
    realize_for: function (recfun) {
        console.log("MomeIntValue-realize_for start recfun=", recfun, " this=", this);
        console.trace();
        console.assert(typeof (this.int_val) === "number",
                       "MomeIntValue no int_val in this=", this);
        var eintelem = $("<span class='mom_value_bcl momnumber_cl' tabindex='0'>"+ this.int_val.toString() +"</span>");
        eintelem.data("momfor", this);
        console.log("MomeIntValue-realize_for this=", this, " before recfun=", recfun, " eintelem=", eintelem);
        recfun(eintelem);
        console.log("MomeIntValue-realize_for done recfun=", recfun,
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
    realize_for: function (recfun) {
        console.log("MomeDoubleValue-realize_for start recfun=", recfun, " this=", this);
        console.trace();
        console.assert(typeof (this.dbl_val) === "number",
                       "MomeDoubleValue no dbl_val in this=", this);
        var edblelem = $("<span class='mom_value_bcl momnumber_cl' tabindex='0'>"+ this.dbl_val.toString() +"</span>");
        edblelem.data("momfor", this);
        console.log("MomeDoubleValue-realize_for before recfun=", recfun, " this=", this, " edblelem=", edblelem);
        recfun(edblelem);
        console.log("MomeDoubleValue-realize_for done recfun=", recfun,
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
    realize_for: function (recfun) {
        console.log("MomeStringValue-realize_for start recfun=", recfun, " this=", this);
        console.assert(typeof (this.str_val) === "string",
                       "MomeStringValue no str_val in this=", this);
        var estrelem = $("<q class='momstrquote_cl'><span class='mom_value_bcl momstring_cl' tabindex='0'>"+ htmlEncode(this.str_val) +"</span></q>");
        estrelem.data("momfor", this);
        console.log("MomeStringValue-realize_for estrelem=", estrelem, " this=", this, " before recfun=", recfun);
        recfun(estrelem);
        console.log("MomeStringValue-realize_for done recfun=", recfun,
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
    realize_for: function (recfun) {
        console.log("MomeTupleValue-realize_for start recfun=", recfun, " this=", this);
        console.trace();
        console.assert(Array.isArray (this.tup_val),
                       "MomeTupleValue no tup_val in this=", this);
        var self = this;
        var etupelem = $("<span class='mom_value_bcl momtuple_cl' tabindex='0'></span>");
        var et = null;
        et = document.createTextNode("[");
        $(et).appendTo(etupelem);
        et = null;
        var nbcomp = this.tup_val.length;
        for (var compix=0; compix<nbcomp; compix++) {
            var curcomp = this.tup_val[compix];
            console.log("MomeTupleValue-realize_for compix=", compix,
                        " curcomp=", curcomp,
                        " this=", this, " etupelem=", etupelem);
            if (compix > 0) {
                et = document.createTextNode(" ");
                $(et).appendTo(etupelem);
                et = null;
            };
            curcomp.realize_for(function (del) {
                console.log("MomeTupleValue curcomp=", curcomp, " del=", del, " etupelem=", etupelem);
                console.trace();
                del.appendTo(etupelem);
            });
            console.log("MomeTupleValue-realize_for did compix=", compix,
                        " curcomp=", curcomp, " etupelem=", etupelem);
        }
        et = document.createTextNode("]");
        $(et).appendTo(etupelem);
        et = null;
        etupelem.data("momfor", this);
        console.log("MomeTupleValue-realize_for etupelem=", etupelem, " this=", this,
                    " before recfun=", recfun);
        recfun(etupelem);
        console.log("MomeTupleValue-realize_for done recfun=", recfun,
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
    realize_for: function (recfun) {
        console.log("MomeSetValue-realize_for start recfun=", recfun, " this=", this);
        console.trace();
        console.assert(Array.isArray (this.set_val),
                       "MomeSetValue no set_val in this=", this);
        var esetelem = $("<span class='mom_value_bcl momset_cl' tabindex='0'></span>");
        console.log("MomeSetValue-realize_for esetelem=", esetelem);
        var et = null;
        et = document.createTextNode("{");
        $(et).appendTo(esetelem);
        et = null;
        var nbelem = this.set_val.length;
        for (var elemix=0; elemix<nbelem; elemix++) {
            var curelem = this.set_val[elemix];
            console.log("MomeSetValue-realize_for elemix=", elemix,
                        " curelem=", curelem,
                        " this=", this);
            if (elemix > 0) {
                et = document.createTextNode(" ");
                $(et).appendTo(esetelem);
                et = null;
            };
            curelem.realize_for(function (del) {
                console.log ("MomeSetValue curelem=", curelem, " del=", del, " esetelem=", esetelem)
                del.appendTo(esetelem);
            });
            console.log("MomeSetValue-realize_for did elemix=", elemix,
                        " curelem=", curelem, " esetelem=", esetelem);
        }
        et = document.createTextNode("}");
        $(et).appendTo(esetelem);
        et = null;
        esetelem.data("momfor", this);
        console.log("MomeSetValue-realize_for this=", this, " esetelem=", esetelem, " before recfun=", recfun);
        recfun(esetelem);
        console.log("MomeSetValue-realize_for done recfun=", recfun,
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
    realize_for: function (recfun) {
        var self = this;
        console.log("MomeNodeValue-realize_for start recfun=", recfun, " this=", this);
        console.trace();
        console.assert(typeof (this.conn_itm) === "object"
                       && Array.isArray(this.sons_arr),
                       "MomeSetValue no conn_itm&sons_arr in this=", this);
        var enodelem = $("<span class='mom_value_bcl momnode_cl' tabindex='0'></span>");
        console.log("MomeNodeValue-realize_for enodelem=", enodelem);
        var et = null;
        var econnelem = $("<span class='momconn_cl' tabindex='0'>*</span>");
        econnelem.appendTo(enodelem);
        this.conn_itm.realize_for(function (del) {
            console.log("MomeNodeValue connitm self=", self, " del=", del);
            del.appendTo(econnelem);
        });
        console.log("MomeNodeValue-realize_for after connitm enodelem=", enodelem, " this=", this);
        et = document.createTextNode("(");
        $(et).appendTo(enodelem);
        var nbsons = this.sons_arr.length;
        console.log("MomeNodeValue-realize_for nbsons=", nbsons, " this=", this, " enodelem=", enodelem);
        for (var sonix=0; sonix<nbsons; sonix++) {
            var curson = this.sons_arr[sonix];
            console.log("MomeNodeValue-realize_for sonix=", sonix, " curson=", curson, " this=", this);
            if (sonix>0) {
                et = document.createTextNode(" ");
                $(et).appendTo(enodelem);
                et = null;
            };
            curson.realize_for(function (del) {
                console.log ("MomeNodeValue curson=", curson, " self=", self, " del=", del);
                del.appendTo(enodelem);
            });
            console.log("MomeNodeValue-realize_for enodelem=", enodelem, " after sonix=", sonix);
        }
        et = document.createTextNode(")");
        $(et).appendTo(enodelem);
        et = null;
        enodelem.data("momfor", this);
        console.log("MomeNodeValue-realize_for self=", self, " enodelem=", enodelem, " before recfun=", recfun);
        recfun (enodelem);
        console.log("MomeNodeValue-realize_for done recfun=", recfun,
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

function editdivinput(ev) {
    console.log ("editdivinput ev=", ev, " this=", this,
                 " $(this)=", $(this),
                 " $(':focus')=", $(':focus'),
		 " activeElement=", document.activeElement);
};

function editdivkeypress(ev) {
    console.log ("editdivkeypress ev=", ev, " .key=`", ev.key,
		 "' .which=", ev.which,
		 " this=", this,
                 " $(this)=", $(this),
                 " $(':focus')=", $(':focus'),
		 " activeElement=", document.activeElement);
};

function editdivfocusin(ev) {
    console.log ("editdivfocusin ev=", ev, " this=", this,
                 " $(this)=", $(this),
                 " $(':focus')=", $(':focus'),
		 " activeElement=", document.activeElement);
};

function editdivfocusout(ev) {
    console.log ("editdivfocusout ev=", ev, " this=", this,
                 " $(this)=", $(this),
		 " activeElement=", document.activeElement);
};

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
    $editdiv.on("input", editdivinput);
    $editdiv.on("keypress", editdivkeypress);
    $editdiv.on("focusin", editdivfocusin);
    $editdiv.on("focusout", editdivfocusout);
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
